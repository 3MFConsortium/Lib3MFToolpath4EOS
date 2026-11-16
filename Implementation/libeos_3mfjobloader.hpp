/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Helpers for building an OpenJZ from a Lib3MF Model.

*/

#ifndef __LIBEOS_3MFJOBLOADER
#define __LIBEOS_3MFJOBLOADER

#include "lib3mftoolpath4eos_interfaces.hpp"
#include "libeos_namespaceregistry.hpp"
#include "../Common/common_workingdirectory.hpp"

#include <functional>
#include <string>
#include <vector>

#define EOS3MF_MATERIALSET_PATH_STR "/Attachments/materialset.eospar"

namespace Lib3MF4EOS {
namespace Impl {

struct SEOS3MFMetadata {
    std::string machineTarget;
    std::string materialName;
    std::string materialThickness;
    std::string paramSetName;
    std::string paramSetVersion;
};

struct SEOS3MFPartMeta {
    std::string partParameter;
    bool isSupport;
};

// A mesh instance of the 3MF build, flattened into world coordinates in millimeters.
// Part IDs are assigned in build item order, depth first through component trees,
// and are identical to the OpenJZ part IDs written by buildOpenJZFromModel.
struct SEOS3MFPart {
    uint32_t partId;
    std::string buildItemUUID;
    std::string name;
    SEOS3MFPartMeta meta;
    std::vector<Lib3MF::sPosition> vertices;
    std::vector<Lib3MF::sTriangle> triangles;
};

SEOS3MFMetadata readModelMetadata(Lib3MF::PModel pModel, const CNamespaceRegistry& registry);

SEOS3MFPartMeta readObjectMetadata(Lib3MF::PObject pObject, const CNamespaceRegistry& registry);

// Returns an empty string if the model has no material set attachment.
std::string extractMaterialSetFromModel(Lib3MF::PModel pModel, const std::string& sOutputDir, const CNamespaceRegistry& registry);

double getModelUnitInMM(Lib3MF::PModel pModel);

std::vector<SEOS3MFPart> enumerateModelParts(Lib3MF::PModel pModel, const CNamespaceRegistry& registry);

#define EOS_OPENJOB_FALLBACK_VERSION "1.5.22"

struct SEOS3MFJobOptions {
    std::string openJobVersion;
    // Written as <height><end> in millimeters if not negative.
    double heightEndInMM;
    // Chooses the parameter of a part without eos3mf:partparameter. If empty, the _Default_ parameters are used.
    std::function<std::string(const SEOS3MFPart& part)> resolvePartParameter;

    SEOS3MFJobOptions() : openJobVersion(EOS_OPENJOB_FALLBACK_VERSION), heightEndInMM(-1.0) {}
};

// The parameter a part without eos3mf:partparameter gets if no exposure set is chosen for it.
std::string getDefaultPartParameter(bool bIsSupport);

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
    const CNamespaceRegistry& registry);

// Packs an unpacked OpenJob, the resource files it references (relative to its directory) and a material set into an OpenJZ.
void buildOpenJZFromOpenJob(const std::string& sOpenJobFileName, const std::string& sEosparFilePath, const std::string& sOpenJZFileName);

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_3MFJOBLOADER
