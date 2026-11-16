/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Helpers for building an OpenJZ from a Lib3MF Model.

*/

#include "libeos_3mfjobloader.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "../Common/common_portablezipwriter.hpp"
#include "../Common/common_exportstream_native.hpp"
#include "../Common/common_importstream_native.hpp"
#include "../Common/common_utils.hpp"

#include "Libraries/PugiXML/pugixml.hpp"

#include <sstream>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <functional>
#include <algorithm>
#include <limits>

namespace Lib3MF4EOS {
namespace Impl {

// ---------------------------------------------------------------------------
// Metadata
// ---------------------------------------------------------------------------

SEOS3MFMetadata readModelMetadata(Lib3MF::PModel pModel, const CNamespaceRegistry& registry)
{
    SEOS3MFMetadata meta;
    auto pMetaGroup = pModel->GetMetaDataGroup();
    uint32_t nCount = pMetaGroup->GetMetaDataCount();

    for (uint32_t i = 0; i < nCount; i++) {
        auto pMD = pMetaGroup->GetMetaData(i);
        if (!registry.isAccepted(Lib3MF4EOS::eEOSNamespaceKind::JobMetadata, pMD->GetNameSpace()))
            continue;

        std::string localKey = pMD->GetName();
        std::string val = pMD->GetValue();

        if (localKey == "machinetarget") meta.machineTarget = val;
        else if (localKey == "materialname") meta.materialName = val;
        else if (localKey == "materialthickness") meta.materialThickness = val;
        else if (localKey == "paramsetname") meta.paramSetName = val;
        else if (localKey == "paramsetversion") meta.paramSetVersion = val;
    }
    return meta;
}

SEOS3MFPartMeta readObjectMetadata(Lib3MF::PObject pObject, const CNamespaceRegistry& registry)
{
    SEOS3MFPartMeta pm;
    pm.isSupport = false;

    auto pMetaGroup = pObject->GetMetaDataGroup();
    uint32_t nCount = pMetaGroup->GetMetaDataCount();

    for (uint32_t i = 0; i < nCount; i++) {
        auto pMD = pMetaGroup->GetMetaData(i);
        if (!registry.isAccepted(Lib3MF4EOS::eEOSNamespaceKind::JobMetadata, pMD->GetNameSpace()))
            continue;

        std::string localKey = pMD->GetName();
        std::string val = pMD->GetValue();

        if (localKey == "partparameter") pm.partParameter = val;
        else if (localKey == "issupport") pm.isSupport = (val == "true");
    }
    return pm;
}

// ---------------------------------------------------------------------------
// Material extraction
// ---------------------------------------------------------------------------

std::string extractMaterialSetFromModel(Lib3MF::PModel pModel, const std::string& sOutputDir, const CNamespaceRegistry& registry)
{
    uint32_t nAttCount = pModel->GetAttachmentCount();
    for (uint32_t i = 0; i < nAttCount; i++) {
        auto pAttachment = pModel->GetAttachment(i);
        if ((pAttachment->GetPath() == EOS3MF_MATERIALSET_PATH_STR) ||
            registry.isAccepted(Lib3MF4EOS::eEOSNamespaceKind::MaterialSetRelationship, pAttachment->GetRelationShipType())) {
            std::string sOutputPath = sOutputDir + "/materialset.eospar";
            pAttachment->WriteToFile(sOutputPath);
            return sOutputPath;
        }
    }
    return "";
}

// ---------------------------------------------------------------------------
// Part enumeration
// ---------------------------------------------------------------------------

double getModelUnitInMM(Lib3MF::PModel pModel)
{
    switch (pModel->GetUnit()) {
    case Lib3MF::eModelUnit::MicroMeter: return 0.001;
    case Lib3MF::eModelUnit::MilliMeter: return 1.0;
    case Lib3MF::eModelUnit::CentiMeter: return 10.0;
    case Lib3MF::eModelUnit::Inch: return 25.4;
    case Lib3MF::eModelUnit::Foot: return 304.8;
    case Lib3MF::eModelUnit::Meter: return 1000.0;
    default:
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "Unknown 3MF model unit");
    }
}

// 3MF transforms use the row vector convention: p' = p * M, with row 3 being the translation.
struct SAffineTransform {
    double m[4][3];

    static SAffineTransform identity()
    {
        SAffineTransform t = {};
        t.m[0][0] = 1.0;
        t.m[1][1] = 1.0;
        t.m[2][2] = 1.0;
        return t;
    }

    static SAffineTransform fromLib3MF(const Lib3MF::sTransform& transform)
    {
        SAffineTransform t;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 3; j++)
                t.m[i][j] = transform.m_Fields[i][j];
        return t;
    }

    // Returns the transform that applies this first, then other.
    SAffineTransform then(const SAffineTransform& other) const
    {
        SAffineTransform r;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                double val = (i == 3) ? other.m[3][j] : 0.0;
                for (int k = 0; k < 3; k++)
                    val += m[i][k] * other.m[k][j];
                r.m[i][j] = val;
            }
        }
        return r;
    }
};

std::vector<SEOS3MFPart> enumerateModelParts(Lib3MF::PModel pModel, const CNamespaceRegistry& registry)
{
    std::vector<SEOS3MFPart> parts;
    double dUnitInMM = getModelUnitInMM(pModel);

    std::string sBuildItemUUID;

    std::function<void(Lib3MF::PObject, const SAffineTransform&)> collectMeshes;
    collectMeshes = [&](Lib3MF::PObject pObj, const SAffineTransform& transform) {
        if (pObj->IsMeshObject()) {
            auto pMesh = pModel->GetMeshObjectByID(pObj->GetResourceID());

            SEOS3MFPart part;
            part.partId = (uint32_t)parts.size() + 1;
            part.buildItemUUID = sBuildItemUUID;
            part.name = pObj->GetName();
            if (part.name.empty())
                part.name = "Part " + std::to_string(part.partId);
            part.meta = readObjectMetadata(pObj, registry);

            pMesh->GetVertices(part.vertices);
            pMesh->GetTriangleIndices(part.triangles);

            for (auto& v : part.vertices) {
                double x = v.m_Coordinates[0];
                double y = v.m_Coordinates[1];
                double z = v.m_Coordinates[2];
                for (int j = 0; j < 3; j++) {
                    double val = x * transform.m[0][j] + y * transform.m[1][j] + z * transform.m[2][j] + transform.m[3][j];
                    v.m_Coordinates[j] = (float)(val * dUnitInMM);
                }
            }

            parts.push_back(std::move(part));
        }
        else if (pObj->IsComponentsObject()) {
            auto pCompObj = pModel->GetComponentsObjectByID(pObj->GetResourceID());
            uint32_t nCompCount = pCompObj->GetComponentCount();

            for (uint32_t ci = 0; ci < nCompCount; ci++) {
                auto pComp = pCompObj->GetComponent(ci);

                SAffineTransform compTransform = SAffineTransform::identity();
                if (pComp->HasTransform())
                    compTransform = SAffineTransform::fromLib3MF(pComp->GetTransform());

                collectMeshes(pComp->GetObjectResource(), compTransform.then(transform));
            }
        }
    };

    auto pBuildItems = pModel->GetBuildItems();
    while (pBuildItems->MoveNext()) {
        auto pItem = pBuildItems->GetCurrent();

        SAffineTransform itemTransform = SAffineTransform::identity();
        if (pItem->HasObjectTransform())
            itemTransform = SAffineTransform::fromLib3MF(pItem->GetObjectTransform());

        bool bHasUUID = false;
        sBuildItemUUID = pItem->GetUUID(bHasUUID);
        if (!bHasUUID)
            sBuildItemUUID.clear();

        collectMeshes(pItem->GetObjectResource(), itemTransform);
    }

    // The meshes keep their Z, because the toolpath layers are not moved with them.
    // The EOS SDK refuses parts below the platform with EOS_ERR_STL_BELOW_PLATFORM.
    return parts;
}

// ---------------------------------------------------------------------------
// Binary STL builder
// ---------------------------------------------------------------------------

static std::vector<uint8_t> buildBinarySTL(const std::vector<Lib3MF::sPosition>& vertices, const std::vector<Lib3MF::sTriangle>& triangles)
{
    uint32_t nTriCount = (uint32_t)triangles.size();
    size_t totalSize = 80 + 4 + (size_t)nTriCount * 50;
    std::vector<uint8_t> buf(totalSize, 0);
    size_t pos = 80;

    memcpy(&buf[pos], &nTriCount, 4);
    pos += 4;

    for (auto& tri : triangles) {
        auto& v0 = vertices.at(tri.m_Indices[0]);
        auto& v1 = vertices.at(tri.m_Indices[1]);
        auto& v2 = vertices.at(tri.m_Indices[2]);

        float ex = v1.m_Coordinates[0] - v0.m_Coordinates[0];
        float ey = v1.m_Coordinates[1] - v0.m_Coordinates[1];
        float ez = v1.m_Coordinates[2] - v0.m_Coordinates[2];
        float fx = v2.m_Coordinates[0] - v0.m_Coordinates[0];
        float fy = v2.m_Coordinates[1] - v0.m_Coordinates[1];
        float fz = v2.m_Coordinates[2] - v0.m_Coordinates[2];

        float nx = ey * fz - ez * fy;
        float ny = ez * fx - ex * fz;
        float nz = ex * fy - ey * fx;
        float len = sqrtf(nx * nx + ny * ny + nz * nz);
        if (len > 0.0f) { nx /= len; ny /= len; nz /= len; }

        memcpy(&buf[pos], &nx, 4); pos += 4;
        memcpy(&buf[pos], &ny, 4); pos += 4;
        memcpy(&buf[pos], &nz, 4); pos += 4;

        for (int vi = 0; vi < 3; vi++) {
            auto& v = vertices.at(tri.m_Indices[vi]);
            memcpy(&buf[pos], v.m_Coordinates, 12); pos += 12;
        }

        uint16_t attrByteCount = 0;
        memcpy(&buf[pos], &attrByteCount, 2); pos += 2;
    }

    return buf;
}

// ---------------------------------------------------------------------------
// OpenJZ builder
// ---------------------------------------------------------------------------

static std::string escapeXML(const std::string& sValue)
{
    std::string sResult;
    sResult.reserve(sValue.size());
    for (char c : sValue) {
        switch (c) {
        case '&': sResult += "&amp;"; break;
        case '<': sResult += "&lt;"; break;
        case '>': sResult += "&gt;"; break;
        case '"': sResult += "&quot;"; break;
        case '\'': sResult += "&apos;"; break;
        default: sResult += c;
        }
    }
    return sResult;
}

static std::string formatMM(double dValue)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.6f", dValue);
    return buffer;
}

static void writeZipEntry(AMCCommon::CPortableZIPWriter& zipWriter, const std::string& sName, const void* pData, size_t nSize)
{
    auto pStream = zipWriter.createEntry(sName, 0);
    pStream->writeBuffer(pData, nSize);
}

static std::vector<uint8_t> readFileToBuffer(const std::string& sPath)
{
    AMCCommon::CImportStream_Native importStream(sPath);
    uint64_t nSize = importStream.retrieveSize();
    std::vector<uint8_t> buf((size_t)nSize);
    if (nSize > 0)
        importStream.readBuffer(buf.data(), nSize, true);
    return buf;
}

std::string getDefaultPartParameter(bool bIsSupport)
{
    return bIsSupport ? "_Default_ExternalSupport" : "_Default_DirectPart";
}

void buildOpenJZFromModel(
    Lib3MF::PModel pModel,
    const std::string& sMachineTarget,
    const std::string& sParamSetName,
    const std::string& sParamSetVersion,
    const std::string& sMaterialName,
    const std::string& sMaterialThickness,
    const std::string& sEosparFilePath,
    const std::string& sOpenJZFileName,
    const SEOS3MFJobOptions& options,
    const CNamespaceRegistry& registry)
{
    if (options.openJobVersion.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJob version is empty");

    struct SPartData {
        std::string stlFileName;
        std::string partParameter;
        std::string name;
        bool isSupport;
        uint32_t partId;
        double translation[3];
        std::vector<uint8_t> stlData;
    };

    auto parts = enumerateModelParts(pModel, registry);
    if (parts.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "No mesh parts found in 3MF model");

    // Each STL is written relative to the minimum of its bounding box. The part's reference point
    // defaults to the geometry origin, so the translation places it at its original world position.
    std::vector<SPartData> partDataList;
    for (auto& part : parts) {
        if (part.vertices.empty() || part.triangles.empty())
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF part has an empty mesh: " + part.name);

        SPartData pd;
        pd.partId = part.partId;
        pd.name = part.name;
        pd.isSupport = part.meta.isSupport;
        pd.partParameter = part.meta.partParameter;
        if (pd.partParameter.empty())
            pd.partParameter = options.resolvePartParameter ? options.resolvePartParameter(part) : getDefaultPartParameter(part.meta.isSupport);
        pd.stlFileName = "part_" + std::to_string(part.partId) + ".stl";

        float fMin[3] = { (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)() };
        for (auto& v : part.vertices)
            for (int j = 0; j < 3; j++)
                fMin[j] = (std::min)(fMin[j], v.m_Coordinates[j]);

        for (auto& v : part.vertices)
            for (int j = 0; j < 3; j++)
                v.m_Coordinates[j] -= fMin[j];

        for (int j = 0; j < 3; j++)
            pd.translation[j] = fMin[j];

        pd.stlData = buildBinarySTL(part.vertices, part.triangles);
        partDataList.push_back(std::move(pd));
    }

    std::ostringstream xml;
    xml << "<openjob version=\"" << escapeXML(options.openJobVersion) << "\">\n";
    xml << "  <target>" << escapeXML(sMachineTarget) << "</target>\n";
    xml << "  <paramset name=\"" << escapeXML(sParamSetName) << "\" version=\"" << escapeXML(sParamSetVersion) << "\">\n";
    xml << "    <materialname>" << escapeXML(sMaterialName) << "</materialname>\n";
    xml << "    <materialthickness>" << escapeXML(sMaterialThickness) << "</materialthickness>\n";
    xml << "  </paramset>\n";
    if (options.heightEndInMM >= 0.0)
        xml << "  <height><end>" << formatMM(options.heightEndInMM) << "</end></height>\n";
    xml << "  <parts>\n";

    for (auto& pd : partDataList) {
        xml << "    <part resourceid=\"{" << pd.partId << "}\" id=\"" << pd.partId << "\" name=\"" << escapeXML(pd.name) << "\">\n";
        xml << "      <parameter>" << escapeXML(pd.partParameter) << "</parameter>\n";
        xml << "      <translation><x>" << formatMM(pd.translation[0]) << "</x><y>" << formatMM(pd.translation[1])
            << "</y><z>" << formatMM(pd.translation[2]) << "</z></translation>\n";
        xml << "    </part>\n";
    }

    xml << "  </parts>\n";
    xml << "  <resources>\n";
    for (auto& pd : partDataList) {
        xml << "    <resource file=\"parts\\" << pd.stlFileName
            << "\" id=\"{" << pd.partId << "}\" is_support=\""
            << (pd.isSupport ? "true" : "false") << "\" is_aux=\"false\" />\n";
    }
    xml << "  </resources>\n";
    xml << "</openjob>\n";

    std::string sXmlContent = xml.str();

    auto pFileStream = std::make_shared<AMCCommon::CExportStream_Native>(sOpenJZFileName);
    AMCCommon::CPortableZIPWriter zipWriter(pFileStream, false);

    writeZipEntry(zipWriter, "openjob.openjob", sXmlContent.data(), sXmlContent.size());

    if (!sEosparFilePath.empty()) {
        auto materialBuffer = readFileToBuffer(sEosparFilePath);

        std::string sMaterialFileName = sEosparFilePath;
        auto lastSlash = sMaterialFileName.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            sMaterialFileName = sMaterialFileName.substr(lastSlash + 1);

        writeZipEntry(zipWriter, "materialset/" + sMaterialFileName, materialBuffer.data(), materialBuffer.size());
    }

    for (auto& pd : partDataList)
        writeZipEntry(zipWriter, "parts/" + pd.stlFileName, pd.stlData.data(), pd.stlData.size());

    zipWriter.writeDirectory();
}

static std::string fileNameOfPath(const std::string& sPath)
{
    auto nLastSlash = sPath.find_last_of("/\\");
    return (nLastSlash != std::string::npos) ? sPath.substr(nLastSlash + 1) : sPath;
}

// Resource paths of an OpenJob must stay inside its directory.
static std::string normalizeOpenJobResourcePath(const std::string& sFileAttribute)
{
    std::string sPath = sFileAttribute;
    std::replace(sPath.begin(), sPath.end(), '\\', '/');
    if (sPath.empty() || (sPath[0] == '/') || (sPath.find(':') != std::string::npos))
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJob: resource file must be a relative path: " + sFileAttribute);

    std::istringstream stream(sPath);
    std::string sComponent;
    while (std::getline(stream, sComponent, '/')) {
        if (sComponent.empty() || (sComponent == ".") || (sComponent == ".."))
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJob: resource file must not contain empty, . or .. path components: " + sFileAttribute);
    }
    return sPath;
}

void buildOpenJZFromOpenJob(const std::string& sOpenJobFileName, const std::string& sEosparFilePath, const std::string& sOpenJZFileName)
{
    auto openJobBuffer = readFileToBuffer(sOpenJobFileName);

    pugi::xml_document document;
    auto parseResult = document.load_buffer(openJobBuffer.data(), openJobBuffer.size());
    if (!parseResult)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, std::string("OpenJob: could not parse ") + sOpenJobFileName + ": " + parseResult.description());

    auto openJobNode = document.child("openjob");
    if (openJobNode.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJob: " + sOpenJobFileName + " has no openjob element");

    std::string sDirectory = sOpenJobFileName;
    auto nLastSlash = sDirectory.find_last_of("/\\");
    sDirectory = (nLastSlash != std::string::npos) ? sDirectory.substr(0, nLastSlash + 1) : std::string();

    std::vector<std::string> resourcePaths;
    for (auto resourceNode : openJobNode.child("resources").children("resource")) {
        std::string sPath = normalizeOpenJobResourcePath(resourceNode.attribute("file").as_string());
        if (std::find(resourcePaths.begin(), resourcePaths.end(), sPath) == resourcePaths.end())
            resourcePaths.push_back(sPath);
    }
    if (resourcePaths.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJob: " + sOpenJobFileName + " references no resource files");

    std::vector<std::vector<uint8_t>> resourceBuffers;
    for (auto& sPath : resourcePaths) {
        std::string sResourceFileName = sDirectory + sPath;
        if (!AMCCommon::CUtils::fileOrPathExistsOnDisk(sResourceFileName) || AMCCommon::CUtils::pathIsDirectory(sResourceFileName))
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJob: resource file not found: " + sResourceFileName);
        resourceBuffers.push_back(readFileToBuffer(sResourceFileName));
    }

    auto materialBuffer = readFileToBuffer(sEosparFilePath);

    auto pFileStream = std::make_shared<AMCCommon::CExportStream_Native>(sOpenJZFileName);
    AMCCommon::CPortableZIPWriter zipWriter(pFileStream, false);

    writeZipEntry(zipWriter, "openjob.openjob", openJobBuffer.data(), openJobBuffer.size());
    writeZipEntry(zipWriter, "materialset/" + fileNameOfPath(sEosparFilePath), materialBuffer.data(), materialBuffer.size());
    for (size_t nIndex = 0; nIndex < resourcePaths.size(); nIndex++)
        writeZipEntry(zipWriter, resourcePaths[nIndex], resourceBuffers[nIndex].data(), resourceBuffers[nIndex].size());

    zipWriter.writeDirectory();
}

} // namespace Impl
} // namespace Lib3MF4EOS
