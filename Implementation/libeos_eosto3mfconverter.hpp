/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Converts EOS raw task layers into 3MF toolpath data.
          Also provides helpers for reading/writing EOS 3MF metadata.

*/

#ifndef __LIBEOS_EOSTO3MFCONVERTER
#define __LIBEOS_EOSTO3MFCONVERTER

#include "lib3mftoolpath4eos_interfaces.hpp"
#include "libeos_3mfjobloader.hpp"

#include <map>
#include <cmath>
#include <string>
#include <memory>
#include <tuple>
#include <vector>
#include <functional>

namespace Lib3MF4EOS {
namespace Impl {

void writeModelMetadata(Lib3MF::PModel pModel, const SEOS3MFMetadata& meta);
void writeObjectMetadata(Lib3MF::PObject pObject, const SEOS3MFPartMeta& pm);
void removeMaterialSetFromModel(Lib3MF::PModel pModel);
void attachMaterialSetToModel(Lib3MF::PModel pModel, const std::string& sEosparPath);

// How the per-point laser power of a block is stored in 3MF. Blocks with varying power get a
// laserpower modifier on factor F with range [0, MaxValue].
struct sEOSTo3MFConverterPowerModifier {
	Lib3MF::eToolpathProfileModificationType m_eType;
	double m_dMaxValue;
};

struct sEOSTo3MFConverterProfile {
	int32_t m_LaserIndex;
	double m_LaserPowerInWatts;
	double m_ScanSpeedInMMPerS;
	// The EOS focus value, from 0.0 (fully defocused) to 1.0 (fully focused).
	double m_Focus;
	int32_t m_ExposureType;
	int32_t m_SkywritingEnabled;
	uint32_t m_ExposureSetIndex;
	uint32_t m_ExposureStepIndex;
	double m_PulsedWavePeriodTime;
	double m_PulsedWavePowerOnTime;
	uint32_t m_ExposedDepth;
	int32_t m_PowerModificationType;
	double m_PowerModifierMaxValue;
	double m_PreDelayInMicroseconds;

	friend bool operator<(sEOSTo3MFConverterProfile const& x, sEOSTo3MFConverterProfile const& y) noexcept {
		return std::tie(x.m_LaserIndex, x.m_LaserPowerInWatts, x.m_ScanSpeedInMMPerS, x.m_Focus, x.m_ExposureType, x.m_SkywritingEnabled,
				x.m_ExposureSetIndex, x.m_ExposureStepIndex, x.m_PulsedWavePeriodTime, x.m_PulsedWavePowerOnTime, x.m_ExposedDepth,
				x.m_PowerModificationType, x.m_PowerModifierMaxValue, x.m_PreDelayInMicroseconds)
			< std::tie(y.m_LaserIndex, y.m_LaserPowerInWatts, y.m_ScanSpeedInMMPerS, y.m_Focus, y.m_ExposureType, y.m_SkywritingEnabled,
				y.m_ExposureSetIndex, y.m_ExposureStepIndex, y.m_PulsedWavePeriodTime, y.m_PulsedWavePowerOnTime, y.m_ExposedDepth,
				y.m_PowerModificationType, y.m_PowerModifierMaxValue, y.m_PreDelayInMicroseconds);
	}
};


class CEOSTo3MFConverterPart {
private:
	uint32_t m_nPartID;
	double m_dMinLayerZ;
	double m_dMaxLayerZ;
	bool m_bHasLayer;

	double m_dMinX;
	double m_dMinY;
	double m_dMaxX;
	double m_dMaxY;
	bool m_bHasCoordinate;
	bool m_bHasOriginalMesh;

	Lib3MF::PMeshObject m_pMeshObject;
	Lib3MF::PBuildItem m_pBuildItem;

public:

	CEOSTo3MFConverterPart(uint32_t nPartID, Lib3MF::PMeshObject pMeshObject, Lib3MF::PBuildItem pBuildItem);
	virtual ~CEOSTo3MFConverterPart();

	uint32_t getPartID();
	double getMinLayerZ();
	double getMaxLayerZ();
	Lib3MF::PMeshObject getMeshObject();
	Lib3MF::PBuildItem getBuildItem();

	void setOriginalMesh(const std::vector<Lib3MF::sPosition>& vertices, const std::vector<Lib3MF::sTriangle>& triangles);
	void addLayerZ(double dLayerZinMM);
	void addXYCoordinate(double dXinMM, double dYinMM);
	void createBoxMesh();
};

typedef std::shared_ptr<CEOSTo3MFConverterPart> PEOSTo3MFConverterPart;


class CEOSTo3MFConverter {
private:
	Lib3MF::PWrapper m_pLib3MFWrapper;
	Lib3MF::PModel m_pModel;
	Lib3MF::PWriter m_pModelWriter;
	Lib3MF::PToolpath m_pToolpath;

	double m_dUnits;
	bool m_bHasLayer;

	std::map<uint32_t, PEOSTo3MFConverterPart> m_Parts;
	std::map<sEOSTo3MFConverterProfile, Lib3MF::PToolpathProfile> m_Profiles;
	// Laser sources 0..N-1 are declared for the scanners, so that their ascending order maps them back to the same scanners.
	uint32_t m_nDeclaredLaserSourceCount;

	void declareLaserSources(uint32_t nScannerCount);

	sEOSTo3MFConverterProfile getProfileFromBlock(const Lib3MF4EOS::sEOSTaskBlock& block, const sEOSTo3MFConverterPowerModifier& powerModifier);
	static sEOSTo3MFConverterPowerModifier getPowerModifierOfBlock(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points);

public:

	// Cuts a block wherever the per-point scan speed or focus changes, because a 3MF segment has one profile
	// and the only modifier factor lib3mf writes carries the power.
	static void splitBlockAtValueChanges(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points, std::vector<Lib3MF4EOS::sEOSTaskBlock>& outBlocks);

	static std::string exposureTypeName(uint32_t nType);
	static bool exposureTypeFromName(const std::string& sName, uint32_t& nType);

	// The Z below the first layer, one layer thickness under it. The thickness is the distance of the first two layers,
	// or for a single layer its Z divided by its EOS layer index. A task that starts at the first EOS layer starts at 0.
	static double computeBottomZInMM(uint32_t nFirstLayerIndex, uint32_t nLayerCount, int64_t nFirstLayerZInMicrometers, int64_t nSecondLayerZInMicrometers);

	static std::string formatTimestamp(double dTimestampInMicroseconds);

	CEOSTo3MFConverter(Lib3MF::PWrapper pWrapper, Lib3MF::PModel pModel, Lib3MF::PWriter pModelWriter);
	virtual ~CEOSTo3MFConverter();

	// Must be called before the first layer is converted.
	void setBottomZ(double dBottomZInMM);

	void convertLayer(IEOSRawTaskLayer* pEOSLayer);
	PEOSTo3MFConverterPart findOrCreatePart(uint32_t nPartID);
	Lib3MF::PToolpathProfile findOrCreateProfile(const Lib3MF4EOS::sEOSTaskBlock& block, const sEOSTo3MFConverterPowerModifier& powerModifier);
	void setPartMesh(const SEOS3MFPart& part);
	void finish();
};

void collectAndRegisterMeshes(
	Lib3MF::PModel pInputModel,
	CEOSTo3MFConverter& converter,
	const CNamespaceRegistry& registry);

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_EOSTO3MFCONVERTER
