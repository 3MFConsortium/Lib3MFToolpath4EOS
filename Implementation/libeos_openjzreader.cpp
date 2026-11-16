/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Converts an OpenJZ job archive into a Lib3MF Model.

*/

#include "libeos_openjzreader.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_3mfjobloader.hpp"
#include "libeos_eosto3mfconverter.hpp"

#include "../Common/common_importstream_native.hpp"

#include "Libraries/libzip/zip.h"
#include "Libraries/PugiXML/pugixml.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace Lib3MF4EOS {
namespace Impl {

static void throwOpenJZError(const std::string& sMessage)
{
    throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJZ: " + sMessage);
}

static std::string toLower(std::string sValue)
{
    std::transform(sValue.begin(), sValue.end(), sValue.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return sValue;
}

static std::string normalizeEntryName(const std::string& sName)
{
    std::string sResult = sName;
    std::replace(sResult.begin(), sResult.end(), '\\', '/');
    while (!sResult.empty() && (sResult[0] == '/'))
        sResult.erase(0, 1);
    return sResult;
}

// ---------------------------------------------------------------------------
// ZIP archive
// ---------------------------------------------------------------------------

class COpenJZArchive {
private:
    std::vector<uint8_t> m_Buffer;
    zip_error_t m_ZIPError;
    zip_source_t* m_pSource;
    zip_t* m_pArchive;
    // Keyed by the lower case, forward slash entry name.
    std::map<std::string, uint64_t> m_Entries;
    std::vector<std::string> m_EntryNames;

public:
    COpenJZArchive(const std::string& sFileName)
        : m_pSource(nullptr), m_pArchive(nullptr)
    {
        AMCCommon::CImportStream_Native importStream(sFileName);
        uint64_t nSize = importStream.retrieveSize();
        if (nSize == 0)
            throwOpenJZError("file is empty: " + sFileName);
        m_Buffer.resize((size_t)nSize);
        importStream.readBuffer(m_Buffer.data(), nSize, true);

        zip_error_init(&m_ZIPError);

        m_pSource = zip_source_buffer_create(m_Buffer.data(), m_Buffer.size(), 0, &m_ZIPError);
        if (m_pSource == nullptr)
            throwOpenJZError("could not read ZIP file: " + sFileName);

        m_pArchive = zip_open_from_source(m_pSource, ZIP_RDONLY | ZIP_CHECKCONS, &m_ZIPError);
        if (m_pArchive == nullptr)
            throwOpenJZError("could not open ZIP file: " + sFileName);

        int64_t nEntryCount = zip_get_num_entries(m_pArchive, ZIP_FL_UNCHANGED);
        if (nEntryCount < 0)
            throwOpenJZError("could not get ZIP entries: " + sFileName);

        for (int64_t nIndex = 0; nIndex < nEntryCount; nIndex++) {
            const char* pszName = zip_get_name(m_pArchive, (uint64_t)nIndex, ZIP_FL_ENC_GUESS);
            if (pszName == nullptr)
                continue;
            std::string sName = normalizeEntryName(pszName);
            m_Entries.insert(std::make_pair(toLower(sName), (uint64_t)nIndex));
            m_EntryNames.push_back(sName);
        }
    }

    ~COpenJZArchive()
    {
        if (m_pArchive != nullptr)
            zip_close(m_pArchive);
        else if (m_pSource != nullptr)
            zip_source_free(m_pSource);

        zip_error_fini(&m_ZIPError);
    }

    const std::vector<std::string>& getEntryNames()
    {
        return m_EntryNames;
    }

    bool hasEntry(const std::string& sName)
    {
        return m_Entries.find(toLower(normalizeEntryName(sName))) != m_Entries.end();
    }

    std::vector<uint8_t> readEntry(const std::string& sName)
    {
        auto iIter = m_Entries.find(toLower(normalizeEntryName(sName)));
        if (iIter == m_Entries.end())
            throwOpenJZError("archive entry not found: " + sName);

        zip_stat_t stat;
        if (zip_stat_index(m_pArchive, iIter->second, ZIP_FL_UNCHANGED, &stat) != 0)
            throwOpenJZError("could not stat archive entry: " + sName);

        std::vector<uint8_t> buffer((size_t)stat.size);

        zip_file_t* pFile = zip_fopen_index(m_pArchive, iIter->second, ZIP_FL_UNCHANGED);
        if (pFile == nullptr)
            throwOpenJZError("could not open archive entry: " + sName);

        uint64_t nTotalRead = 0;
        while (nTotalRead < stat.size) {
            zip_int64_t nRead = zip_fread(pFile, buffer.data() + nTotalRead, stat.size - nTotalRead);
            if (nRead <= 0)
                break;
            nTotalRead += (uint64_t)nRead;
        }
        zip_fclose(pFile);

        if (nTotalRead != stat.size)
            throwOpenJZError("could not read archive entry: " + sName);

        return buffer;
    }
};

// ---------------------------------------------------------------------------
// STL reader
// ---------------------------------------------------------------------------

struct SSTLMesh {
    std::vector<std::array<double, 3>> vertices;
    std::vector<Lib3MF::sTriangle> triangles;
};

class CSTLMeshBuilder {
private:
    SSTLMesh& m_Mesh;
    std::map<std::array<float, 3>, uint32_t> m_VertexMap;

    uint32_t addVertex(float x, float y, float z)
    {
        std::array<float, 3> key = { x, y, z };
        auto iIter = m_VertexMap.find(key);
        if (iIter != m_VertexMap.end())
            return iIter->second;

        uint32_t nIndex = (uint32_t)m_Mesh.vertices.size();
        m_Mesh.vertices.push_back({ (double)x, (double)y, (double)z });
        m_VertexMap.insert(std::make_pair(key, nIndex));
        return nIndex;
    }

public:
    CSTLMeshBuilder(SSTLMesh& mesh) : m_Mesh(mesh) {}

    void addTriangle(const float* pCoordinates)
    {
        uint32_t n0 = addVertex(pCoordinates[0], pCoordinates[1], pCoordinates[2]);
        uint32_t n1 = addVertex(pCoordinates[3], pCoordinates[4], pCoordinates[5]);
        uint32_t n2 = addVertex(pCoordinates[6], pCoordinates[7], pCoordinates[8]);

        // 3MF does not allow triangles that reference the same vertex twice.
        if ((n0 == n1) || (n1 == n2) || (n0 == n2))
            return;

        Lib3MF::sTriangle triangle;
        triangle.m_Indices[0] = n0;
        triangle.m_Indices[1] = n1;
        triangle.m_Indices[2] = n2;
        m_Mesh.triangles.push_back(triangle);
    }
};

static SSTLMesh readSTL(const std::vector<uint8_t>& buffer, const std::string& sDebugName)
{
    SSTLMesh mesh;
    CSTLMeshBuilder builder(mesh);

    bool bIsBinary = false;
    if (buffer.size() >= 84) {
        uint32_t nTriangleCount;
        memcpy(&nTriangleCount, &buffer[80], 4);
        bIsBinary = ((uint64_t)buffer.size() == 84 + (uint64_t)nTriangleCount * 50);

        if (bIsBinary) {
            for (uint32_t nIndex = 0; nIndex < nTriangleCount; nIndex++) {
                float coordinates[9];
                memcpy(coordinates, &buffer[84 + (size_t)nIndex * 50 + 12], sizeof(coordinates));
                builder.addTriangle(coordinates);
            }
        }
    }

    if (!bIsBinary) {
        std::string sContent(buffer.begin(), buffer.end());
        if (toLower(sContent.substr(0, 5)) != "solid")
            throwOpenJZError("invalid STL file: " + sDebugName);

        std::istringstream stream(sContent);
        std::string sToken;
        float coordinates[9];
        uint32_t nVertexIndex = 0;
        while (stream >> sToken) {
            if (toLower(sToken) != "vertex")
                continue;

            if (!(stream >> coordinates[nVertexIndex * 3] >> coordinates[nVertexIndex * 3 + 1] >> coordinates[nVertexIndex * 3 + 2]))
                throwOpenJZError("invalid vertex in ASCII STL file: " + sDebugName);

            nVertexIndex++;
            if (nVertexIndex == 3) {
                builder.addTriangle(coordinates);
                nVertexIndex = 0;
            }
        }
    }

    if (mesh.triangles.empty())
        throwOpenJZError("STL file contains no triangles: " + sDebugName);

    return mesh;
}

// ---------------------------------------------------------------------------
// OpenJob parsing
// ---------------------------------------------------------------------------

struct SOpenJobResource {
    std::string fileName;
    bool isSupport;
};

static double readDoubleChild(const pugi::xml_node& node, const char* pszChildName, double dDefault)
{
    auto child = node.child(pszChildName);
    if (child.empty())
        return dDefault;
    return child.text().as_double(dDefault);
}

static void collectDescendants(const pugi::xml_node& node, const char* pszName, std::vector<pugi::xml_node>& result)
{
    for (auto child : node.children()) {
        if (child.type() != pugi::node_element)
            continue;
        if (strcmp(child.name(), pszName) == 0)
            result.push_back(child);
        else
            collectDescendants(child, pszName, result);
    }
}

static std::string readChildText(const pugi::xml_node& node, const char* pszChildName)
{
    return node.child(pszChildName).text().as_string();
}

static std::string fileNameWithoutExtension(const std::string& sFileName)
{
    std::string sName = normalizeEntryName(sFileName);
    auto nSlash = sName.find_last_of('/');
    if (nSlash != std::string::npos)
        sName = sName.substr(nSlash + 1);
    auto nDot = sName.find_last_of('.');
    if (nDot != std::string::npos)
        sName = sName.substr(0, nDot);
    return sName;
}

static void attachMaterialSetBuffer(Lib3MF::PModel pModel, const std::vector<uint8_t>& buffer)
{
    removeMaterialSetFromModel(pModel);
    auto pAttachment = pModel->AddAttachment(EOS3MF_MATERIALSET_PATH_STR, LIBEOS_NAMESPACE_MATERIALSETRELATIONSHIP);
    pAttachment->ReadFromBuffer(buffer);
}

void convertOpenJZToModel(const std::string& sOpenJZFileName, Lib3MF::PModel pModel)
{
    if (pModel.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model is null");

    COpenJZArchive archive(sOpenJZFileName);

    std::string sOpenJobEntry = "openjob.openjob";
    if (!archive.hasEntry(sOpenJobEntry)) {
        sOpenJobEntry.clear();
        for (auto& sName : archive.getEntryNames()) {
            std::string sLower = toLower(sName);
            if ((sLower.find('/') == std::string::npos) && (sLower.size() > 8) && (sLower.substr(sLower.size() - 8) == ".openjob")) {
                sOpenJobEntry = sName;
                break;
            }
        }
        if (sOpenJobEntry.empty())
            throwOpenJZError("archive contains no openjob document: " + sOpenJZFileName);
    }

    auto openJobBuffer = archive.readEntry(sOpenJobEntry);
    pugi::xml_document doc;
    pugi::xml_parse_result parseResult = doc.load_buffer(openJobBuffer.data(), openJobBuffer.size());
    if (!parseResult)
        throwOpenJZError(std::string("could not parse openjob document: ") + parseResult.description());

    auto openJobNode = doc.child("openjob");
    if (openJobNode.empty())
        throwOpenJZError("missing openjob root element");

    SEOS3MFMetadata meta;
    meta.machineTarget = readChildText(openJobNode, "target");
    auto paramSetNode = openJobNode.child("paramset");
    meta.paramSetName = paramSetNode.attribute("name").as_string();
    meta.paramSetVersion = paramSetNode.attribute("version").as_string();
    meta.materialName = readChildText(paramSetNode, "materialname");
    meta.materialThickness = readChildText(paramSetNode, "materialthickness");

    std::map<std::string, SOpenJobResource> resources;
    std::vector<pugi::xml_node> resourceNodes;
    collectDescendants(openJobNode.child("resources"), "resource", resourceNodes);
    for (auto& resourceNode : resourceNodes) {
        SOpenJobResource resource;
        resource.fileName = resourceNode.attribute("file").as_string();
        resource.isSupport = resourceNode.attribute("is_support").as_bool(false);
        resources[resourceNode.attribute("id").as_string()] = resource;
    }

    double dUnitInMM = getModelUnitInMM(pModel);

    struct SPlacedPart {
        uint32_t nPartID;
        std::string sName;
        SEOS3MFPartMeta meta;
        std::vector<Lib3MF::sPosition> vertices;
        std::vector<Lib3MF::sTriangle> triangles;
    };
    std::vector<SPlacedPart> placedParts;
    std::set<uint32_t> usedPartIDs;
    std::vector<pugi::xml_node> partsWithoutID;

    std::vector<pugi::xml_node> partNodes;
    collectDescendants(openJobNode.child("parts"), "part", partNodes);
    if (partNodes.empty())
        throwOpenJZError("openjob document contains no parts");

    for (auto& partNode : partNodes) {

        if (!partNode.child("segments").empty())
            throwOpenJZError("Z-segmented parts are not supported");

        std::string sFileName;
        bool bIsSupport = false;
        auto resourceIDAttribute = partNode.attribute("resourceid");
        if (!resourceIDAttribute.empty()) {
            auto iResource = resources.find(resourceIDAttribute.as_string());
            if (iResource == resources.end())
                throwOpenJZError(std::string("part references unknown resource: ") + resourceIDAttribute.as_string());
            sFileName = iResource->second.fileName;
            bIsSupport = iResource->second.isSupport;
        }
        else {
            sFileName = partNode.attribute("file").as_string();
        }

        if (sFileName.empty())
            throwOpenJZError("part has no geometry file");
        std::string sLowerFileName = toLower(sFileName);
        if ((sLowerFileName.size() < 4) || (sLowerFileName.substr(sLowerFileName.size() - 4) != ".stl"))
            throwOpenJZError("only STL part geometries are supported: " + sFileName);

        SSTLMesh mesh = readSTL(archive.readEntry(sFileName), sFileName);

        double dScaleX = 1.0 + readDoubleChild(partNode.child("scaling"), "x", 0.0) / 100.0;
        double dScaleY = 1.0 + readDoubleChild(partNode.child("scaling"), "y", 0.0) / 100.0;
        for (auto& v : mesh.vertices) {
            v[0] *= dScaleX;
            v[1] *= dScaleY;
        }

        double referencePoint[3] = { 0.0, 0.0, 0.0 };
        auto originNode = partNode.child("coordinateorigin");
        if (!originNode.empty()) {
            std::string sFormat = originNode.attribute("format").as_string();
            double values[3] = { readDoubleChild(originNode, "x", 0.0), readDoubleChild(originNode, "y", 0.0), readDoubleChild(originNode, "z", 0.0) };

            if (sFormat == "coord") {
                for (int j = 0; j < 3; j++)
                    referencePoint[j] = values[j];
            }
            else if ((sFormat == "bbox_rel") || (sFormat == "bboxrel")) {
                double bboxMin[3] = { (std::numeric_limits<double>::max)(), (std::numeric_limits<double>::max)(), (std::numeric_limits<double>::max)() };
                double bboxMax[3] = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() };
                for (auto& v : mesh.vertices) {
                    for (int j = 0; j < 3; j++) {
                        bboxMin[j] = (std::min)(bboxMin[j], v[j]);
                        bboxMax[j] = (std::max)(bboxMax[j], v[j]);
                    }
                }
                for (int j = 0; j < 3; j++)
                    referencePoint[j] = bboxMin[j] + values[j] * (bboxMax[j] - bboxMin[j]);
            }
            else {
                throwOpenJZError("unknown coordinateorigin format: " + sFormat);
            }
        }

        auto translationNode = partNode.child("translation");
        double translation[3] = { readDoubleChild(translationNode, "x", 0.0), readDoubleChild(translationNode, "y", 0.0), readDoubleChild(translationNode, "z", 0.0) };

        const double dPi = 3.14159265358979323846;
        double dAngle = readDoubleChild(partNode.child("rotation"), "z", 0.0) * dPi / 180.0;
        double dCos = cos(dAngle);
        double dSin = sin(dAngle);

        SPlacedPart placedPart;
        placedPart.vertices.reserve(mesh.vertices.size());
        for (auto& v : mesh.vertices) {
            double x = v[0] - referencePoint[0];
            double y = v[1] - referencePoint[1];
            double z = v[2] - referencePoint[2];

            double worldX = x * dCos - y * dSin + translation[0];
            double worldY = x * dSin + y * dCos + translation[1];
            double worldZ = z + translation[2];

            Lib3MF::sPosition position;
            position.m_Coordinates[0] = (float)(worldX / dUnitInMM);
            position.m_Coordinates[1] = (float)(worldY / dUnitInMM);
            position.m_Coordinates[2] = (float)(worldZ / dUnitInMM);
            placedPart.vertices.push_back(position);
        }
        placedPart.triangles = std::move(mesh.triangles);

        placedPart.nPartID = partNode.attribute("id").as_uint(0);
        if (placedPart.nPartID != 0)
            usedPartIDs.insert(placedPart.nPartID);

        placedPart.sName = partNode.attribute("name").as_string();
        if (placedPart.sName.empty())
            placedPart.sName = fileNameWithoutExtension(sFileName);

        placedPart.meta.partParameter = readChildText(partNode, "parameter");
        placedPart.meta.isSupport = bIsSupport;

        placedParts.push_back(std::move(placedPart));
    }

    // Parts without an ID receive the smallest free ID, as EOSPRINT does.
    uint32_t nNextFreeID = 1;
    for (auto& placedPart : placedParts) {
        if (placedPart.nPartID != 0)
            continue;
        while (usedPartIDs.count(nNextFreeID) > 0)
            nNextFreeID++;
        placedPart.nPartID = nNextFreeID;
        usedPartIDs.insert(nNextFreeID);
    }

    // Build items are added in part ID order, so that the enumeration order of the 3MF
    // matches the part IDs of the OpenJZ whenever those are consecutive.
    std::stable_sort(placedParts.begin(), placedParts.end(), [](const SPlacedPart& a, const SPlacedPart& b) { return a.nPartID < b.nPartID; });

    Lib3MF::sTransform identityTransform = {};
    identityTransform.m_Fields[0][0] = 1.0f;
    identityTransform.m_Fields[1][1] = 1.0f;
    identityTransform.m_Fields[2][2] = 1.0f;

    for (auto& placedPart : placedParts) {
        auto pMeshObject = pModel->AddMeshObject();
        pMeshObject->SetName(placedPart.sName);
        pMeshObject->SetGeometry(placedPart.vertices, placedPart.triangles);
        writeObjectMetadata(pMeshObject, placedPart.meta);
        pModel->AddBuildItem(pMeshObject.get(), identityTransform);
    }

    writeModelMetadata(pModel, meta);

    for (auto& sName : archive.getEntryNames()) {
        std::string sLower = toLower(sName);
        if ((sLower.rfind("materialset/", 0) == 0) && (sLower.size() > 7) && (sLower.substr(sLower.size() - 7) == ".eospar")) {
            attachMaterialSetBuffer(pModel, archive.readEntry(sName));
            break;
        }
    }
}

} // namespace Impl
} // namespace Lib3MF4EOS
