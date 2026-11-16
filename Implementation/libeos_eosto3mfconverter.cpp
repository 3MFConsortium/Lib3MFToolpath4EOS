/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Converts EOS raw task layers into 3MF toolpath data.
          Also provides helpers for reading/writing EOS 3MF metadata.

*/

#include "libeos_eosto3mfconverter.hpp"
#include "libeos_powerramp.hpp"
#include "../Common/common_importstream_native.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <functional>

using namespace Lib3MF4EOS::Impl;

// Writing always uses the current namespaces.
static const std::string EOS3MF_NAMESPACE = LIBEOS_NAMESPACE_JOBMETADATA;
static const std::string EOS3MF_MATERIALSET_RELTYPE = LIBEOS_NAMESPACE_MATERIALSETRELATIONSHIP;
static const std::string EOS3MF_MATERIALSET_PATH = EOS3MF_MATERIALSET_PATH_STR;
static const std::string EOS_NAMESPACE = LIBEOS_NAMESPACE_TOOLPATHPROFILE;
static const std::string EOS_SEGMENTATTRIBUTE_TIMESTAMP = "timestamp";
static const std::string EOS_SEGMENTATTRIBUTE_STRIPE = "stripe";

void Lib3MF4EOS::Impl::writeModelMetadata(Lib3MF::PModel pModel, const SEOS3MFMetadata& meta) {
	auto pMetaGroup = pModel->GetMetaDataGroup();

	auto addMeta = [&](const std::string& key, const std::string& value) {
		if (!value.empty())
			pMetaGroup->AddMetaData(EOS3MF_NAMESPACE, key, value, "xs:string", true);
	};

	addMeta("machinetarget", meta.machineTarget);
	addMeta("materialname", meta.materialName);
	addMeta("materialthickness", meta.materialThickness);
	addMeta("paramsetname", meta.paramSetName);
	addMeta("paramsetversion", meta.paramSetVersion);
}

void Lib3MF4EOS::Impl::writeObjectMetadata(Lib3MF::PObject pObject, const SEOS3MFPartMeta& pm) {
	auto pMetaGroup = pObject->GetMetaDataGroup();
	if (!pm.partParameter.empty())
		pMetaGroup->AddMetaData(EOS3MF_NAMESPACE, "partparameter", pm.partParameter, "xs:string", true);
	pMetaGroup->AddMetaData(EOS3MF_NAMESPACE, "issupport", pm.isSupport ? "true" : "false", "xs:string", true);
}

void Lib3MF4EOS::Impl::removeMaterialSetFromModel(Lib3MF::PModel pModel) {
	for (uint32_t nIndex = pModel->GetAttachmentCount(); nIndex > 0; nIndex--) {
		auto pAttachment = pModel->GetAttachment(nIndex - 1);
		if ((pAttachment->GetPath() == EOS3MF_MATERIALSET_PATH) || (pAttachment->GetRelationShipType() == EOS3MF_MATERIALSET_RELTYPE))
			pModel->RemoveAttachment(pAttachment.get());
	}
}

void Lib3MF4EOS::Impl::attachMaterialSetToModel(Lib3MF::PModel pModel, const std::string& sEosparPath) {
	removeMaterialSetFromModel(pModel);
	// Lib3MF cannot write an attachment read with ReadFromFile back out (WriteToFile is not implemented for it),
	// so the material set could not be extracted again before the model is saved.
	AMCCommon::CImportStream_Native importStream(sEosparPath);
	uint64_t nSize = importStream.retrieveSize();
	std::vector<uint8_t> buffer((size_t)nSize);
	if (nSize > 0)
		importStream.readBuffer(buffer.data(), nSize, true);

	auto pAttachment = pModel->AddAttachment(EOS3MF_MATERIALSET_PATH, EOS3MF_MATERIALSET_RELTYPE);
	pAttachment->ReadFromBuffer(buffer);
}

// =====================================================================================
// CEOSTo3MFConverterPart
// =====================================================================================

CEOSTo3MFConverterPart::CEOSTo3MFConverterPart(uint32_t nPartID, Lib3MF::PMeshObject pMeshObject, Lib3MF::PBuildItem pBuildItem)
	: m_nPartID(nPartID), m_pMeshObject(pMeshObject), m_pBuildItem(pBuildItem),
	m_dMaxLayerZ(0.0), m_dMinLayerZ(0.0), m_bHasLayer(false),
	m_dMinX(0.0), m_dMinY(0.0), m_dMaxX(0.0), m_dMaxY(0.0),
	m_bHasCoordinate(false), m_bHasOriginalMesh(false)
{
	if (pMeshObject.get() == nullptr)
		throw std::runtime_error("invalid meshobject parameter");
	if (pBuildItem.get() == nullptr)
		throw std::runtime_error("invalid builditem parameter");
}

CEOSTo3MFConverterPart::~CEOSTo3MFConverterPart() {}

uint32_t CEOSTo3MFConverterPart::getPartID() { return m_nPartID; }
double CEOSTo3MFConverterPart::getMinLayerZ() { return m_dMinLayerZ; }
double CEOSTo3MFConverterPart::getMaxLayerZ() { return m_dMaxLayerZ; }
Lib3MF::PMeshObject CEOSTo3MFConverterPart::getMeshObject() { return m_pMeshObject; }
Lib3MF::PBuildItem CEOSTo3MFConverterPart::getBuildItem() { return m_pBuildItem; }

void CEOSTo3MFConverterPart::setOriginalMesh(const std::vector<Lib3MF::sPosition>& vertices, const std::vector<Lib3MF::sTriangle>& triangles) {
	m_pMeshObject->SetGeometry(vertices, triangles);
	m_bHasOriginalMesh = true;
}

void CEOSTo3MFConverterPart::addLayerZ(double dLayerZinMM) {
	if (m_bHasLayer) {
		if (dLayerZinMM < m_dMinLayerZ) m_dMinLayerZ = dLayerZinMM;
		if (dLayerZinMM > m_dMaxLayerZ) m_dMaxLayerZ = dLayerZinMM;
	}
	else {
		m_dMinLayerZ = dLayerZinMM;
		m_dMaxLayerZ = dLayerZinMM;
		m_bHasLayer = true;
	}
}

void CEOSTo3MFConverterPart::addXYCoordinate(double dXinMM, double dYinMM) {
	if (m_bHasCoordinate) {
		if (dXinMM < m_dMinX) m_dMinX = dXinMM;
		if (dXinMM > m_dMaxX) m_dMaxX = dXinMM;
		if (dYinMM < m_dMinY) m_dMinY = dYinMM;
		if (dYinMM > m_dMaxY) m_dMaxY = dYinMM;
	}
	else {
		m_dMinX = dXinMM;
		m_dMaxX = dXinMM;
		m_dMinY = dYinMM;
		m_dMaxY = dYinMM;
		m_bHasCoordinate = true;
	}
}

void CEOSTo3MFConverterPart::createBoxMesh() {
	if (m_bHasOriginalMesh)
		return;

	std::vector<Lib3MF::sPosition> Vertices;
	std::vector<Lib3MF::sTriangle> Triangles;

	Vertices.push_back({ (float)m_dMinX, (float)m_dMinY, (float)m_dMinLayerZ });
	Vertices.push_back({ (float)m_dMaxX, (float)m_dMinY, (float)m_dMinLayerZ });
	Vertices.push_back({ (float)m_dMaxX, (float)m_dMaxY, (float)m_dMinLayerZ });
	Vertices.push_back({ (float)m_dMinX, (float)m_dMaxY, (float)m_dMinLayerZ });
	Vertices.push_back({ (float)m_dMinX, (float)m_dMinY, (float)m_dMaxLayerZ });
	Vertices.push_back({ (float)m_dMaxX, (float)m_dMinY, (float)m_dMaxLayerZ });
	Vertices.push_back({ (float)m_dMaxX, (float)m_dMaxY, (float)m_dMaxLayerZ });
	Vertices.push_back({ (float)m_dMinX, (float)m_dMaxY, (float)m_dMaxLayerZ });

	Triangles.push_back({ 2, 1, 0 });
	Triangles.push_back({ 0, 3, 2 });
	Triangles.push_back({ 4, 5, 6 });
	Triangles.push_back({ 6, 7, 4 });
	Triangles.push_back({ 0, 1, 5 });
	Triangles.push_back({ 5, 4, 0 });
	Triangles.push_back({ 1, 2, 6 });
	Triangles.push_back({ 6, 5, 1 });
	Triangles.push_back({ 2, 3, 7 });
	Triangles.push_back({ 7, 6, 2 });
	Triangles.push_back({ 3, 0, 4 });
	Triangles.push_back({ 4, 7, 3 });

	m_pMeshObject->SetGeometry(Vertices, Triangles);
}


// =====================================================================================
// CEOSTo3MFConverter
// =====================================================================================

std::string CEOSTo3MFConverter::exposureTypeName(uint32_t nType) {
	switch (nType) {
		case 0: return "partboundary";
		case 1: return "downskin";
		case 2: return "infill";
		case 3: return "upskin";
		case 4: return "downskin_contour";
		case 5: return "contour";
		case 6: return "upskin_contour";
		case 7: return "edge";
		case 8: return "support";
		case 9: return "hatch_gap";
		case 10: return "contour_simple";
		case 11: return "jump";
		case 12: return "support_exposed";
		case 13: return "updowncontour";
		case 14: return "core";
		case 15: return "hatch";
		default: return "type" + std::to_string(nType);
	}
}

bool CEOSTo3MFConverter::exposureTypeFromName(const std::string& sName, uint32_t& nType) {
	for (uint32_t nCandidate = 0; nCandidate <= 15; nCandidate++) {
		if (exposureTypeName(nCandidate) == sName) {
			nType = nCandidate;
			return true;
		}
	}

	if ((sName.size() > 4) && (sName.compare(0, 4, "type") == 0) &&
		(sName.find_first_not_of("0123456789", 4) == std::string::npos) && (sName.size() <= 13)) {
		nType = (uint32_t)std::stoul(sName.substr(4));
		return true;
	}

	return false;
}

sEOSTo3MFConverterProfile CEOSTo3MFConverter::getProfileFromBlock(const Lib3MF4EOS::sEOSTaskBlock& block, const sEOSTo3MFConverterPowerModifier& powerModifier) {
	sEOSTo3MFConverterProfile sProfile;
	sProfile.m_LaserIndex = (int32_t)block.m_LaserScannerIndex;
	sProfile.m_LaserPowerInWatts = block.m_DefaultLaserPower;
	sProfile.m_ScanSpeedInMMPerS = block.m_ScanSpeed;
	sProfile.m_Focus = block.m_HasLaserFocus ? block.m_LaserFocus : EOS_FOCUS_NOMINAL;
	sProfile.m_ExposureType = (int32_t)block.m_ExposureType;
	sProfile.m_SkywritingEnabled = (int32_t)block.m_SkywritingEnabled;
	sProfile.m_ExposureSetIndex = block.m_ExposureSetIndex;
	sProfile.m_ExposureStepIndex = block.m_ExposureStepIndex;
	sProfile.m_PulsedWavePeriodTime = block.m_PulsedWavePeriodTime;
	sProfile.m_PulsedWavePowerOnTime = block.m_PulsedWavePowerOnTime;
	sProfile.m_ExposedDepth = block.m_ExposedDepth;
	sProfile.m_PowerModificationType = (int32_t)powerModifier.m_eType;
	sProfile.m_PowerModifierMaxValue = powerModifier.m_dMaxValue;
	sProfile.m_PreDelayInMicroseconds = (block.m_PreDelay > 0.0) ? block.m_PreDelay : 0.0;
	return sProfile;
}

static double effectiveLaserPower(const Lib3MF4EOS::sEOSTaskPoint& point, const Lib3MF4EOS::sEOSTaskBlock& block) {
	return (point.m_LaserPower > 0.0) ? point.m_LaserPower : block.m_DefaultLaserPower;
}

static SVectorValues effectiveSpeedAndFocus(const Lib3MF4EOS::sEOSTaskPoint& point, const Lib3MF4EOS::sEOSTaskBlock& block) {
	SVectorValues values;
	values.m_dLaserPower = 0.0;
	values.m_dScanSpeed = (point.m_ScanSpeed > 0.0) ? point.m_ScanSpeed : block.m_ScanSpeed;
	if (point.m_HasLaserFocus)
		values.m_dLaserFocus = point.m_LaserFocus;
	else
		values.m_dLaserFocus = block.m_HasLaserFocus ? block.m_LaserFocus : EOS_FOCUS_NOMINAL;
	return values;
}

void CEOSTo3MFConverter::splitBlockAtValueChanges(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points, std::vector<Lib3MF4EOS::sEOSTaskBlock>& outBlocks) {
	if ((block.m_StartPointIndex > block.m_EndPointIndex) || (block.m_EndPointIndex > points.size()))
		throw std::runtime_error("invalid point range in raw task block");

	auto addPart = [&](uint32_t nStartPointIndex, uint32_t nEndPointIndex, const SVectorValues& values) {
		Lib3MF4EOS::sEOSTaskBlock part = block;
		part.m_StartPointIndex = nStartPointIndex;
		part.m_EndPointIndex = nEndPointIndex;
		part.m_ScanSpeed = values.m_dScanSpeed;
		part.m_LaserFocus = values.m_dLaserFocus;
		part.m_HasLaserFocus = true;
		if (nStartPointIndex != block.m_StartPointIndex)
			part.m_PreDelay = 0.0;
		outBlocks.push_back(part);
	};

	if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::HatchBlock) {
		// A speed or focus ramp inside one hatch is stored as its mean.
		auto hatchValues = [&](uint32_t nHatchIndex) {
			auto values1 = effectiveSpeedAndFocus(points.at(block.m_StartPointIndex + nHatchIndex * 2), block);
			auto values2 = effectiveSpeedAndFocus(points.at(block.m_StartPointIndex + nHatchIndex * 2 + 1), block);
			return CPowerRamp::interpolate(values1, values2, 0.5);
		};

		uint32_t nNumHatches = (block.m_EndPointIndex - block.m_StartPointIndex) / 2;
		if (nNumHatches == 0) {
			outBlocks.push_back(block);
			return;
		}

		uint32_t nRunStart = 0;
		SVectorValues runValues = hatchValues(0);
		for (uint32_t nHatchIndex = 1; nHatchIndex <= nNumHatches; nHatchIndex++) {
			SVectorValues values = (nHatchIndex < nNumHatches) ? hatchValues(nHatchIndex) : runValues;
			if ((nHatchIndex == nNumHatches) || !CPowerRamp::equalValues(values, runValues)) {
				addPart(block.m_StartPointIndex + nRunStart * 2, block.m_StartPointIndex + nHatchIndex * 2, runValues);
				nRunStart = nHatchIndex;
				runValues = values;
			}
		}
		return;
	}

	if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock) {
		// 3MF polylines carry one profile, so the polyline is cut where speed or focus change.
		// The parts share their joints and are joined again when written to EOS.
		uint32_t nNumPoints = block.m_EndPointIndex - block.m_StartPointIndex;
		if (nNumPoints < 3) {
			if (nNumPoints == 2)
				addPart(block.m_StartPointIndex, block.m_EndPointIndex, effectiveSpeedAndFocus(points.at(block.m_StartPointIndex + 1), block));
			else
				outBlocks.push_back(block);
			return;
		}

		uint32_t nPartStart = 0;
		SVectorValues partValues = effectiveSpeedAndFocus(points.at(block.m_StartPointIndex + 1), block);
		for (uint32_t nPointIndex = 2; nPointIndex < nNumPoints; nPointIndex++) {
			SVectorValues values = effectiveSpeedAndFocus(points.at(block.m_StartPointIndex + nPointIndex), block);
			if (!CPowerRamp::equalValues(values, partValues)) {
				addPart(block.m_StartPointIndex + nPartStart, block.m_StartPointIndex + nPointIndex, partValues);
				nPartStart = nPointIndex - 1;
				partValues = values;
			}
		}
		addPart(block.m_StartPointIndex + nPartStart, block.m_EndPointIndex, partValues);
		return;
	}

	outBlocks.push_back(block);
}

sEOSTo3MFConverterPowerModifier CEOSTo3MFConverter::getPowerModifierOfBlock(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points) {
	sEOSTo3MFConverterPowerModifier modifier;
	modifier.m_eType = Lib3MF::eToolpathProfileModificationType::NoModification;
	modifier.m_dMaxValue = 0.0;

	bool bVaries = false;
	bool bHasRamp = false;
	double dMaxPower = (std::max)(block.m_DefaultLaserPower, 0.0);

	if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::HatchBlock) {
		uint32_t nNumHatches = (block.m_EndPointIndex - block.m_StartPointIndex) / 2;
		for (uint32_t nHatchIndex = 0; nHatchIndex < nNumHatches; nHatchIndex++) {
			double dPower1 = effectiveLaserPower(points.at(block.m_StartPointIndex + nHatchIndex * 2), block);
			double dPower2 = effectiveLaserPower(points.at(block.m_StartPointIndex + nHatchIndex * 2 + 1), block);
			bVaries = bVaries || CPowerRamp::isRamp(block.m_DefaultLaserPower, dPower1) || CPowerRamp::isRamp(block.m_DefaultLaserPower, dPower2);
			bHasRamp = bHasRamp || CPowerRamp::isRamp(dPower1, dPower2);
			dMaxPower = (std::max)(dMaxPower, (std::max)(dPower1, dPower2));
		}
	}
	else if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock) {
		// The first point only carries the power of the jump.
		for (uint32_t nPointIndex = block.m_StartPointIndex + 1; nPointIndex < block.m_EndPointIndex; nPointIndex++) {
			double dPower = effectiveLaserPower(points.at(nPointIndex), block);
			bVaries = bVaries || CPowerRamp::isRamp(block.m_DefaultLaserPower, dPower);
			dMaxPower = (std::max)(dMaxPower, dPower);
		}
		bHasRamp = bVaries;
	}

	if (bVaries) {
		modifier.m_eType = bHasRamp ? Lib3MF::eToolpathProfileModificationType::LinearModification : Lib3MF::eToolpathProfileModificationType::ConstantModification;
		// Rounded up to 100 W, so that blocks of similar power share a profile.
		modifier.m_dMaxValue = (std::max)(100.0, std::ceil(dMaxPower / 100.0) * 100.0);
	}

	return modifier;
}


double CEOSTo3MFConverter::computeBottomZInMM(uint32_t nFirstLayerIndex, uint32_t nLayerCount, int64_t nFirstLayerZInMicrometers, int64_t nSecondLayerZInMicrometers) {
	if ((nFirstLayerIndex <= 1) || (nLayerCount == 0))
		return 0.0;

	double dThicknessInMicrometers;
	if (nLayerCount > 1)
		dThicknessInMicrometers = (double)(nSecondLayerZInMicrometers - nFirstLayerZInMicrometers);
	else
		dThicknessInMicrometers = (double)nFirstLayerZInMicrometers / (double)nFirstLayerIndex;

	if (dThicknessInMicrometers <= 0.0)
		return 0.0;

	return (std::max)(0.0, ((double)nFirstLayerZInMicrometers - dThicknessInMicrometers) / 1000.0);
}

std::string CEOSTo3MFConverter::formatTimestamp(double dTimestampInMicroseconds) {
	if (!std::isfinite(dTimestampInMicroseconds))
		throw std::runtime_error("invalid EOS timestamp");

	// The shortest text that reads back as the same double.
	char buffer[64];
	auto result = std::to_chars(buffer, buffer + sizeof(buffer), dTimestampInMicroseconds);
	if (result.ec != std::errc())
		throw std::runtime_error("could not format EOS timestamp");
	return std::string(buffer, result.ptr);
}

CEOSTo3MFConverter::CEOSTo3MFConverter(Lib3MF::PWrapper pWrapper, Lib3MF::PModel pModel, Lib3MF::PWriter pModelWriter)
	: m_pLib3MFWrapper(pWrapper), m_pModel(pModel), m_pModelWriter(pModelWriter), m_bHasLayer(false), m_nDeclaredLaserSourceCount(0)
{
	m_dUnits = 0.001;
	m_pToolpath = m_pModel->AddToolpath(m_dUnits);
	m_pModel->SetBuildToolpath(m_pToolpath.get());
	m_pToolpath->RegisterCustomDoubleSegmentAttribute(EOS_NAMESPACE, EOS_SEGMENTATTRIBUTE_TIMESTAMP);
	m_pToolpath->RegisterCustomIntegerSegmentAttribute(EOS_NAMESPACE, EOS_SEGMENTATTRIBUTE_STRIPE);
}

CEOSTo3MFConverter::~CEOSTo3MFConverter() {}

void CEOSTo3MFConverter::setBottomZ(double dBottomZInMM) {
	if (m_bHasLayer)
		throw std::runtime_error("the bottom Z must be set before the first layer");
	if (!std::isfinite(dBottomZInMM) || (dBottomZInMM < 0.0))
		throw std::runtime_error("invalid bottom Z");

	m_pToolpath->SetBottomZ((uint32_t)round(dBottomZInMM / m_dUnits));
}

void CEOSTo3MFConverter::declareLaserSources(uint32_t nScannerCount) {
	for (; m_nDeclaredLaserSourceCount < nScannerCount; m_nDeclaredLaserSourceCount++)
		m_pToolpath->AddLaserSource(m_nDeclaredLaserSourceCount, "EOS scanner " + std::to_string(m_nDeclaredLaserSourceCount + 1));
}

void CEOSTo3MFConverter::convertLayer(IEOSRawTaskLayer* pEOSLayer) {
	std::vector<Lib3MF4EOS::sEOSTaskPoint> points;
	std::vector<Lib3MF4EOS::sEOSTaskBlock> blocks;
	std::vector<std::pair<uint32_t, uint32_t>> blockProfilePartIDs;

	uint32_t nLayerHeightInMicron = (uint32_t)pEOSLayer->GetZHeight();
	double dLayerHeightInMM = static_cast<double>(nLayerHeightInMicron) / 1000.0;
	uint32_t nLayerZMaxInUnits = (uint32_t)round(dLayerHeightInMM / m_dUnits);

	Lib3MF4EOS_uint64 nPointCount = 0;
	pEOSLayer->GetPointData(0, &nPointCount, nullptr);
	points.resize(nPointCount);
	pEOSLayer->GetPointData(nPointCount, &nPointCount, points.data());

	Lib3MF4EOS_uint64 nBlockCount = 0;
	pEOSLayer->GetBlockData(0, &nBlockCount, nullptr);
	std::vector<Lib3MF4EOS::sEOSTaskBlock> layerBlocks(nBlockCount);
	pEOSLayer->GetBlockData(nBlockCount, &nBlockCount, layerBlocks.data());
	blocks.reserve(layerBlocks.size());
	uint32_t nScannerCount = 0;
	for (auto& block : layerBlocks) {
		splitBlockAtValueChanges(block, points, blocks);
		nScannerCount = (std::max)(nScannerCount, block.m_LaserScannerIndex + 1);
	}
	declareLaserSources(nScannerCount);

	m_bHasLayer = true;
	auto p3MFLayer = m_pToolpath->AddLayer(nLayerZMaxInUnits, "/toolpath/layer_" + std::to_string(nLayerHeightInMicron) + ".xml", m_pModelWriter);

	std::map<std::string, uint32_t> partMapping;
	std::map<std::string, uint32_t> profileMapping;
	std::vector<sEOSTo3MFConverterPowerModifier> blockPowerModifiers;
	blockPowerModifiers.reserve(blocks.size());

	for (auto& block : blocks) {
		uint32_t nLocalPartID;
		uint32_t nLocalProfileID;

		auto pPart = findOrCreatePart(block.m_PartID);
		pPart->addLayerZ(dLayerHeightInMM);

		auto powerModifier = getPowerModifierOfBlock(block, points);
		blockPowerModifiers.push_back(powerModifier);
		auto pProfile = findOrCreateProfile(block, powerModifier);
		auto pBuildItem = pPart->getBuildItem();

		bool bHasUUID = false;
		std::string sBuildItemUUID = pBuildItem->GetUUID(bHasUUID);
		if (!bHasUUID)
			throw std::runtime_error("builditem has no uuid!");

		std::string sProfileUUID = pProfile->GetUUID();

		auto iPartIter = partMapping.find(sBuildItemUUID);
		if (iPartIter == partMapping.end()) {
			nLocalPartID = p3MFLayer->RegisterBuildItem(pBuildItem);
			partMapping.insert(std::make_pair(sBuildItemUUID, nLocalPartID));
		}
		else {
			nLocalPartID = iPartIter->second;
		}

		auto iProfileIter = profileMapping.find(sProfileUUID);
		if (iProfileIter == profileMapping.end()) {
			nLocalProfileID = p3MFLayer->RegisterProfile(pProfile);
			profileMapping.insert(std::make_pair(sProfileUUID, nLocalProfileID));
		}
		else {
			nLocalProfileID = iProfileIter->second;
		}

		blockProfilePartIDs.push_back(std::make_pair(nLocalPartID, nLocalProfileID));
	}

	auto iIDIterator = blockProfilePartIDs.begin();
	auto iPowerModifierIterator = blockPowerModifiers.begin();
	for (auto& block : blocks) {
		uint32_t nLocalPartID = iIDIterator->first;
		uint32_t nLocalProfileID = iIDIterator->second;
		iIDIterator++;
		auto& powerModifier = *iPowerModifierIterator;
		iPowerModifierIterator++;

		auto pPart = findOrCreatePart(block.m_PartID);
		auto powerFactor = [&](const Lib3MF4EOS::sEOSTaskPoint& point) {
			return effectiveLaserPower(point, block) / powerModifier.m_dMaxValue;
		};
		auto discretePosition = [&](const Lib3MF4EOS::sEOSTaskPoint& point) {
			return Lib3MF::sDiscretePosition2D{ (int32_t)round(point.m_X / m_dUnits), (int32_t)round(point.m_Y / m_dUnits) };
		};

		if (block.m_EndPointIndex > block.m_StartPointIndex) {
			p3MFLayer->SetSegmentAttribute(EOS_NAMESPACE, EOS_SEGMENTATTRIBUTE_TIMESTAMP, formatTimestamp(points.at(block.m_StartPointIndex).m_Timestamp));
			p3MFLayer->SetSegmentAttribute(EOS_NAMESPACE, EOS_SEGMENTATTRIBUTE_STRIPE, std::to_string(block.m_StripeIndex));
		}

		if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::HatchBlock) {
			uint32_t nNumPoints = block.m_EndPointIndex - block.m_StartPointIndex;
			uint32_t nNumHatches = nNumPoints / 2;

			if (nNumHatches > 0) {
				std::vector<Lib3MF::sDiscreteHatch2D> hatchData;
				std::vector<double> factors1;
				std::vector<double> factors2;
				hatchData.reserve(nNumHatches);

				for (uint32_t nHatchIndex = 0; nHatchIndex < nNumHatches; nHatchIndex++) {
					auto& eosPoint1 = points.at(block.m_StartPointIndex + nHatchIndex * 2);
					auto& eosPoint2 = points.at(block.m_StartPointIndex + nHatchIndex * 2 + 1);

					hatchData.push_back(Lib3MF::sDiscreteHatch2D{
						(int32_t)round(eosPoint1.m_X / m_dUnits),
						(int32_t)round(eosPoint1.m_Y / m_dUnits),
						(int32_t)round(eosPoint2.m_X / m_dUnits),
						(int32_t)round(eosPoint2.m_Y / m_dUnits), 0 });

					if (powerModifier.m_eType != Lib3MF::eToolpathProfileModificationType::NoModification) {
						factors1.push_back(powerFactor(eosPoint1));
						factors2.push_back(powerFactor(eosPoint2));
					}

					pPart->addXYCoordinate(eosPoint1.m_X, eosPoint1.m_Y);
					pPart->addXYCoordinate(eosPoint2.m_X, eosPoint2.m_Y);
				}

				switch (powerModifier.m_eType) {
				case Lib3MF::eToolpathProfileModificationType::ConstantModification:
					p3MFLayer->WriteHatchDataDiscreteWithConstantFactors(nLocalProfileID, nLocalPartID, hatchData, factors1);
					break;
				case Lib3MF::eToolpathProfileModificationType::LinearModification:
					p3MFLayer->WriteHatchDataDiscreteWithLinearFactors(nLocalProfileID, nLocalPartID, hatchData, factors1, factors2);
					break;
				default:
					p3MFLayer->WriteHatchDataDiscrete(nLocalProfileID, nLocalPartID, hatchData);
					break;
				}
			}
		}

		if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock) {
			std::vector<Lib3MF::sDiscretePosition2D> pointData;
			std::vector<double> pointFactors;
			uint32_t nNumPoints = block.m_EndPointIndex - block.m_StartPointIndex;
			if (nNumPoints > 0) {
				pointData.reserve(nNumPoints);

				for (uint32_t nPointIndex = 0; nPointIndex < nNumPoints; nPointIndex++) {
					auto& eosPoint = points.at(block.m_StartPointIndex + nPointIndex);
					pPart->addXYCoordinate(eosPoint.m_X, eosPoint.m_Y);

					if (powerModifier.m_eType == Lib3MF::eToolpathProfileModificationType::NoModification) {
						pointData.push_back(discretePosition(eosPoint));
						continue;
					}

					// EOS uses one power per vector, 3MF ramps linearly between point factors. A power change is
					// encoded as a zero-length vector at the joint, so that every vector keeps a constant factor.
					if (nPointIndex == 0) {
						pointData.push_back(discretePosition(eosPoint));
						pointFactors.push_back((nNumPoints > 1) ? powerFactor(points.at(block.m_StartPointIndex + 1)) : powerFactor(eosPoint));
						continue;
					}

					double dFactor = powerFactor(eosPoint);
					if (CPowerRamp::isRamp(pointFactors.back() * powerModifier.m_dMaxValue, dFactor * powerModifier.m_dMaxValue)) {
						Lib3MF::sDiscretePosition2D joint = pointData.back();
						pointData.push_back(joint);
						pointFactors.push_back(dFactor);
					}
					pointData.push_back(discretePosition(eosPoint));
					pointFactors.push_back(dFactor);
				}

				if (powerModifier.m_eType == Lib3MF::eToolpathProfileModificationType::NoModification)
					p3MFLayer->WritePolylineDiscrete(nLocalProfileID, nLocalPartID, pointData);
				else
					p3MFLayer->WritePolylineDiscreteWithFactors(nLocalProfileID, nLocalPartID, pointData, pointFactors);
			}
		}
	}

	p3MFLayer->Finish();
}

PEOSTo3MFConverterPart CEOSTo3MFConverter::findOrCreatePart(uint32_t nPartID) {
	auto iPart = m_Parts.find(nPartID);
	if (iPart == m_Parts.end()) {
		auto pMeshObject = m_pModel->AddMeshObject();
		auto pBuildItem = m_pModel->AddBuildItem(pMeshObject.get(), m_pLib3MFWrapper->GetIdentityTransform());
		auto pPart = std::make_shared<CEOSTo3MFConverterPart>(nPartID, pMeshObject, pBuildItem);
		m_Parts.insert(std::make_pair(nPartID, pPart));
		return pPart;
	}
	else
		return iPart->second;
}

Lib3MF::PToolpathProfile CEOSTo3MFConverter::findOrCreateProfile(const Lib3MF4EOS::sEOSTaskBlock& block, const sEOSTo3MFConverterPowerModifier& powerModifier) {
	auto profileStruct = getProfileFromBlock(block, powerModifier);
	auto iIter = m_Profiles.find(profileStruct);

	if (iIter == m_Profiles.end()) {
		std::string sTypeName = exposureTypeName(block.m_ExposureType);
		std::string sName = sTypeName + std::to_string(m_Profiles.size() + 1);
		auto p3MFProfile = m_pToolpath->AddProfile(sName);

		p3MFProfile->SetParameterDoubleValue("", "laserpower", profileStruct.m_LaserPowerInWatts);
		p3MFProfile->SetParameterDoubleValue("", "laserspeed", profileStruct.m_ScanSpeedInMMPerS);
		// laserfocus is a focus offset in mm, so the EOS focus value is stored as eos:focus.
		if (profileStruct.m_Focus != EOS_FOCUS_NOMINAL)
			p3MFProfile->SetParameterDoubleValue(EOS_NAMESPACE, "focus", profileStruct.m_Focus);
		if (powerModifier.m_eType != Lib3MF::eToolpathProfileModificationType::NoModification)
			p3MFProfile->AddModifier("", "laserpower", powerModifier.m_eType, Lib3MF::eToolpathProfileModificationFactor::FactorF, 0.0, powerModifier.m_dMaxValue);
		p3MFProfile->SetParameterIntegerValue("", "laserindex", profileStruct.m_LaserIndex);
		p3MFProfile->SetParameterValue(EOS_NAMESPACE, "exposuretype", sTypeName);
		p3MFProfile->SetParameterBoolValue(EOS_NAMESPACE, "skywriting", profileStruct.m_SkywritingEnabled);

		if ((profileStruct.m_ExposureSetIndex != 0) || (profileStruct.m_ExposureStepIndex != 0)) {
			p3MFProfile->SetParameterIntegerValue(EOS_NAMESPACE, "exposuresetindex", profileStruct.m_ExposureSetIndex);
			p3MFProfile->SetParameterIntegerValue(EOS_NAMESPACE, "exposurestepindex", profileStruct.m_ExposureStepIndex);
		}
		if ((profileStruct.m_PulsedWavePeriodTime != 0.0) || (profileStruct.m_PulsedWavePowerOnTime != 0.0)) {
			p3MFProfile->SetParameterDoubleValue(EOS_NAMESPACE, "pulsedwaveperiodtime", profileStruct.m_PulsedWavePeriodTime);
			p3MFProfile->SetParameterDoubleValue(EOS_NAMESPACE, "pulsedwavepowerontime", profileStruct.m_PulsedWavePowerOnTime);
		}
		if (profileStruct.m_ExposedDepth != 0)
			p3MFProfile->SetParameterIntegerValue(EOS_NAMESPACE, "exposeddepth", profileStruct.m_ExposedDepth);
		if (profileStruct.m_PreDelayInMicroseconds > 0.0)
			p3MFProfile->SetParameterDoubleValue(EOS_NAMESPACE, "predelay", profileStruct.m_PreDelayInMicroseconds);

		m_Profiles.insert(std::make_pair(profileStruct, p3MFProfile));
		return p3MFProfile;
	}
	else {
		return iIter->second;
	}
}

void CEOSTo3MFConverter::setPartMesh(const SEOS3MFPart& part) {
	auto pPart = findOrCreatePart(part.partId);
	pPart->setOriginalMesh(part.vertices, part.triangles);

	auto pMeshObject = pPart->getMeshObject();
	pMeshObject->SetName(part.name);
	writeObjectMetadata(pMeshObject, part.meta);
}

void CEOSTo3MFConverter::finish() {
	for (auto iPart : m_Parts) {
		iPart.second->createBoxMesh();
	}
}


// =====================================================================================
// Mesh collection helper for copying input geometry to output
// =====================================================================================

void Lib3MF4EOS::Impl::collectAndRegisterMeshes(
	Lib3MF::PModel pInputModel,
	CEOSTo3MFConverter& converter,
	const CNamespaceRegistry& registry)
{
	for (auto& part : enumerateModelParts(pInputModel, registry))
		converter.setPartMesh(part);
}
