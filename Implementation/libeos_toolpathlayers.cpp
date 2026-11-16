/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Layer analysis of 3MF toolpaths.

*/

#include "libeos_toolpathlayers.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_jsonwriter.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace Lib3MF4EOS {
namespace Impl {

using Lib3MF4EOS::eEOSLogSeverity;

static std::string micrometersToMMString(int64_t nMicrometers)
{
	return jsonNumber((double)nMicrometers / 1000.0);
}

static std::string conventionName(Lib3MF4EOS::eEOSLayerZConvention eConvention)
{
	return (eConvention == Lib3MF4EOS::eEOSLayerZConvention::Base) ? "Base" : "Top";
}

std::string layerThicknessSourceToString(eLayerThicknessSource eSource)
{
	return (eSource == eLayerThicknessSource::PartLayers) ? "partLayers" : "toolpathStep";
}

uint32_t SToolpathZLayout::getPartLayerThickness(uint32_t nPartID) const
{
	auto iPart = m_Parts.find(nPartID);
	if (iPart == m_Parts.end())
		return 0;
	return iPart->second.m_nLayerThickness;
}

SToolpathLayerScan scanToolpathLayers(Lib3MF::PToolpath pToolpath, double dToolpathUnitInMM, const std::map<std::string, uint32_t>& buildItemPartIDs)
{
	SToolpathLayerScan scan;
	uint32_t nLayerCount = pToolpath->GetLayerCount();
	scan.m_LayerZ.reserve(nLayerCount);
	scan.m_LayerParts.resize(nLayerCount);

	for (uint32_t nLayerIndex = 0; nLayerIndex < nLayerCount; nLayerIndex++) {
		scan.m_LayerZ.push_back((int64_t)llround(pToolpath->GetLayerZMax(nLayerIndex) * dToolpathUnitInMM * 1000.0));

		Lib3MF::PToolpathLayerReader pLayerReader;
		try {
			pLayerReader = pToolpath->ReadLayerData(nLayerIndex);
		}
		catch (Lib3MF::ELib3MFException& e) {
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "could not read toolpath layer " + std::to_string(nLayerIndex) + " (" + e.what() +
				"); toolpath 3MFs must be read with Reader::ReadFromPersistentSource and the source must stay alive");
		}

		std::set<uint32_t> partIDs;
		uint32_t nSegmentCount = pLayerReader->GetSegmentCount();
		for (uint32_t nSegmentIndex = 0; nSegmentIndex < nSegmentCount; nSegmentIndex++) {
			// Sync and delay segments have no build item; an EOS task cannot express them.
			Lib3MF::eToolpathSegmentType eSegmentType;
			uint32_t nPointCount = 0;
			pLayerReader->GetSegmentInfo(nSegmentIndex, eSegmentType, nPointCount);
			if ((eSegmentType == Lib3MF::eToolpathSegmentType::Sync) || (eSegmentType == Lib3MF::eToolpathSegmentType::Delay))
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_LASERSYNCNOTSUPPORTED, "toolpath layer " + std::to_string(nLayerIndex) + " has a " +
					std::string((eSegmentType == Lib3MF::eToolpathSegmentType::Sync) ? "sync" : "delay") + " segment, which an EOS task cannot express");

			std::string sBuildItemUUID = pLayerReader->GetSegmentBuildItemUUID(nSegmentIndex);
			auto iPartID = buildItemPartIDs.find(sBuildItemUUID);
			if (iPartID == buildItemPartIDs.end())
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a segment that references a build item without mesh: " + sBuildItemUUID);
			partIDs.insert(iPartID->second);
		}
		scan.m_LayerParts[nLayerIndex].assign(partIDs.begin(), partIDs.end());
	}

	return scan;
}

SToolpathZLayout computeToolpathZLayout(const SToolpathLayerScan& scan, Lib3MF4EOS::eEOSLayerZConvention eConvention, uint32_t nFallbackThickness,
	const std::vector<uint32_t>& partIDs, PConversionLogData pLog)
{
	const int64_t nTolerance = TOOLPATHLAYERS_TOLERANCE_UM;
	auto& layerZ = scan.m_LayerZ;
	size_t nLayerCount = layerZ.size();

	SToolpathZLayout layout;
	layout.m_eConvention = eConvention;

	auto fail = [&](int32_t nLayerIndex, uint32_t nPartID, const std::string& sCode, Lib3MF4EOSResult nErrorCode, const std::string& sText) {
		pLog->addMessage(eEOSLogSeverity::Error, sCode, nLayerIndex, nPartID, sText);
		throw ELib3MF4EOSInterfaceException(nErrorCode, sText);
	};

	for (size_t nIndex = 1; nIndex < nLayerCount; nIndex++) {
		int64_t nDistance = layerZ[nIndex] - layerZ[nIndex - 1];
		if (nDistance <= 0)
			fail((int32_t)nIndex, LIBEOS_CONVERSIONLOG_NOPART, "LAYER_Z_INVALID", LIB3MF4EOS_ERROR_LAYERZINVALID, "toolpath layer " + std::to_string(nIndex) + " is at Z " +
				micrometersToMMString(layerZ[nIndex]) + " mm, which is not above the previous layer at Z " + micrometersToMMString(layerZ[nIndex - 1]) + " mm");
		if ((layout.m_nStep == 0) || (nDistance < layout.m_nStep))
			layout.m_nStep = nDistance;
	}
	if (nLayerCount < 2)
		layout.m_nStep = nFallbackThickness;

	std::map<uint32_t, std::vector<int64_t>> partLayerZ;
	for (size_t nIndex = 0; nIndex < nLayerCount; nIndex++)
		for (auto nPartID : scan.m_LayerParts[nIndex])
			partLayerZ[nPartID].push_back(layerZ[nIndex]);

	for (auto nPartID : partIDs) {
		SPartLayers& part = layout.m_Parts[nPartID];
		auto iLayerZ = partLayerZ.find(nPartID);
		if (iLayerZ == partLayerZ.end()) {
			part.m_nLayerThickness = (uint32_t)(std::max)(layout.m_nStep, (int64_t)0);
			continue;
		}

		auto& zValues = iLayerZ->second;
		part.m_nLayerCount = (uint32_t)zValues.size();
		part.m_nFirstZ = zValues.front();
		part.m_nLastZ = zValues.back();

		if (zValues.size() < 2) {
			part.m_nLayerThickness = (uint32_t)(std::max)(layout.m_nStep, (int64_t)0);
			continue;
		}

		int64_t nThickness = (std::numeric_limits<int64_t>::max)();
		for (size_t nIndex = 1; nIndex < zValues.size(); nIndex++)
			nThickness = (std::min)(nThickness, zValues[nIndex] - zValues[nIndex - 1]);

		// Rounding to micrometers may shift every layer Z by up to one micrometer.
		for (size_t nIndex = 1; nIndex < zValues.size(); nIndex++) {
			int64_t nDistance = zValues[nIndex] - zValues[nIndex - 1];
			int64_t nLayers = (nDistance + nThickness / 2) / nThickness;
			if (std::llabs(nDistance - nLayers * nThickness) > nLayers * nTolerance)
				fail(LIBEOS_CONVERSIONLOG_NOLAYER, nPartID, "PART_THICKNESS_INCONSISTENT", LIB3MF4EOS_ERROR_LAYERTHICKNESSINCONSISTENT, "part " + std::to_string(nPartID) +
					" has toolpath layers " + std::to_string(nThickness) + " and " + std::to_string(nDistance) + " micrometers apart, which are not multiples of one layer thickness");
		}

		part.m_nLayerThickness = (uint32_t)nThickness;
		part.m_eThicknessSource = eLayerThicknessSource::PartLayers;
	}

	if (nLayerCount > 0) {
		int64_t nFirstZ = layerZ.front();

		if (eConvention == Lib3MF4EOS::eEOSLayerZConvention::Top) {
			if (nFirstZ <= nTolerance)
				fail(0, LIBEOS_CONVERSIONLOG_NOPART, "LAYER_Z_INVALID", LIB3MF4EOS_ERROR_LAYERZINVALID, "the first toolpath layer is at Z " + micrometersToMMString(nFirstZ) +
					" mm; as the top of a layer, it would have no thickness. If the toolpath gives the base of every layer, set the layer Z convention to Base");
			layout.m_nOffset = 0;
		}
		else {
			// The EOS layer Z of a layer base is base plus thickness, and a toolpath layer has one EOS layer Z.
			uint32_t nCommonThickness = 0;
			uint32_t nCommonPartID = 0;
			for (auto& part : layout.m_Parts) {
				if ((part.second.m_nLayerCount == 0) || (part.second.m_nLayerThickness == 0))
					continue;
				if (nCommonThickness == 0) {
					nCommonThickness = part.second.m_nLayerThickness;
					nCommonPartID = part.first;
				}
				else if (std::llabs((int64_t)part.second.m_nLayerThickness - (int64_t)nCommonThickness) > nTolerance) {
					fail(LIBEOS_CONVERSIONLOG_NOLAYER, part.first, "LAYER_Z_INVALID", LIB3MF4EOS_ERROR_LAYERZINVALID, "with the layer Z convention Base, all parts need the same layer thickness, but part " +
						std::to_string(nCommonPartID) + " has " + std::to_string(nCommonThickness) + " micrometers and part " + std::to_string(part.first) + " has " +
						std::to_string(part.second.m_nLayerThickness) + " micrometers");
				}
			}

			layout.m_nOffset = (nCommonThickness > 0) ? (int64_t)nCommonThickness : layout.m_nStep;
			if (layout.m_nOffset <= 0)
				fail(0, LIBEOS_CONVERSIONLOG_NOPART, "LAYER_Z_INVALID", LIB3MF4EOS_ERROR_LAYERZINVALID,
					"with the layer Z convention Base, the layer thickness is needed, but the toolpath has a single layer and the job's material thickness is unknown");
		}

		if ((layout.m_nStep > 0) && (std::llabs(nFirstZ - layout.m_nStep) <= nTolerance)) {
			pLog->addMessage(eEOSLogSeverity::Info, "LAYER_Z_AMBIGUOUS", 0, LIBEOS_CONVERSIONLOG_NOPART, "the first toolpath layer is at Z " + micrometersToMMString(nFirstZ) +
				" mm, which equals the layer distance: as top, it is the first layer of the job; as base, the job starts one layer higher. The layer Z convention " +
				conventionName(eConvention) + " is used");
		}
	}

	for (auto& zValue : layerZ)
		layout.m_EOSLayerZ.push_back(zValue + layout.m_nOffset);
	for (auto& part : layout.m_Parts) {
		if (part.second.m_nLayerCount > 0) {
			part.second.m_nFirstZ += layout.m_nOffset;
			part.second.m_nLastZ += layout.m_nOffset;
		}
	}

	if (eConvention == Lib3MF4EOS::eEOSLayerZConvention::Base)
		pLog->addMessage(eEOSLogSeverity::Info, "LAYER_Z_CONVENTION", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, "the toolpath layer Z values are taken as layer bases; " +
			std::to_string(layout.m_nOffset) + " micrometers are added to get the EOS layer Z, which is the top of the layer");
	else
		pLog->addMessage(eEOSLogSeverity::Info, "LAYER_Z_CONVENTION", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"the toolpath layer Z values are taken as layer tops, as EOS layers are given");

	for (auto& part : layout.m_Parts) {
		auto& partLayers = part.second;
		if (partLayers.m_nLayerCount == 0) {
			pLog->addMessage(eEOSLogSeverity::Warning, "PART_WITHOUT_TOOLPATH", LIBEOS_CONVERSIONLOG_NOLAYER, part.first,
				"the part has no toolpath; it is in the job, but it is not exposed");
			continue;
		}

		std::string sThickness = (partLayers.m_nLayerThickness > 0) ? (std::to_string(partLayers.m_nLayerThickness) + " micrometers") : std::string("unknown");
		std::string sSource = (partLayers.m_eThicknessSource == eLayerThicknessSource::PartLayers) ? "the distance of its layers" :
			"the smallest layer distance of the toolpath, as the part has a single layer";
		pLog->addMessage(eEOSLogSeverity::Info, "PART_LAYERS", LIBEOS_CONVERSIONLOG_NOLAYER, part.first, std::to_string(partLayers.m_nLayerCount) +
			" toolpath layers from EOS Z " + micrometersToMMString(partLayers.m_nFirstZ) + " to " + micrometersToMMString(partLayers.m_nLastZ) +
			" mm; layer thickness " + sThickness + ", from " + sSource);
	}

	return layout;
}

SEOSLayerStack simulateLayerStack(const std::vector<SEOS3MFPart>& parts, const SToolpathZLayout& layout, double dMaxZTravelInMM)
{
	std::map<int64_t, std::set<int32_t>> layerParts;
	for (auto& part : parts) {
		int64_t nThickness = layout.getPartLayerThickness(part.partId);
		if (part.vertices.empty() || (nThickness <= 0))
			continue;

		float fMinZ = (std::numeric_limits<float>::max)();
		float fMaxZ = -(std::numeric_limits<float>::max)();
		for (auto& vertex : part.vertices) {
			fMinZ = (std::min)(fMinZ, vertex.m_Coordinates[2]);
			fMaxZ = (std::max)(fMaxZ, vertex.m_Coordinates[2]);
		}
		int64_t nMinZ = (std::max)((int64_t)llround(fMinZ * 1000.0), (int64_t)0);
		int64_t nMaxZ = (int64_t)llround(fMaxZ * 1000.0);

		// The layers whose span from top minus thickness to top overlaps the mesh.
		int64_t nFirstLayer = nMinZ / nThickness + 1;
		int64_t nLastLayer = (std::max)((nMaxZ + nThickness - 1) / nThickness, nFirstLayer);
		for (int64_t nLayer = nFirstLayer; nLayer <= nLastLayer; nLayer++)
			layerParts[nLayer * nThickness].insert((int32_t)part.partId);
	}

	SEOSLayerStack stack;
	stack.m_bSimulated = true;

	int64_t nPreviousZ = 0;
	for (auto& layer : layerParts) {
		sEOSLayerInfo info;
		info.m_nLayerIndex = stack.m_Layers.size() + 1;
		info.m_nDeltaInMicrometers = (int32_t)(layer.first - nPreviousZ);
		info.m_nZPositionInMicrometers = (int32_t)layer.first;
		info.m_bIsExposed = true;
		info.m_ExposedPartIDs.assign(layer.second.begin(), layer.second.end());
		stack.m_Layers.push_back(info);
		nPreviousZ = layer.first;
	}

	stack.m_ProcessHeights.m_dPayloadHeight = (double)nPreviousZ / 1000.0;
	stack.m_ProcessHeights.m_dTotalZTravel = stack.m_ProcessHeights.m_dPayloadHeight;
	stack.m_ProcessHeights.m_dMaxZTravel = dMaxZTravelInMM;
	return stack;
}

uint64_t checkLayerCoverage(const SToolpathLayerScan& scan, const SToolpathZLayout& layout, const SEOSLayerStack& stack, bool bStrictMode, bool bThrowOnError, PConversionLogData pLog)
{
	const int64_t nTolerance = TOOLPATHLAYERS_TOLERANCE_UM;

	std::map<int64_t, size_t> stackIndexByZ;
	for (size_t nIndex = 0; nIndex < stack.m_Layers.size(); nIndex++)
		stackIndexByZ[stack.m_Layers[nIndex].m_nZPositionInMicrometers] = nIndex;

	std::map<int64_t, std::set<uint32_t>> toolpathPartsByZ;
	for (size_t nIndex = 0; nIndex < scan.m_LayerParts.size(); nIndex++) {
		if (!scan.m_LayerParts[nIndex].empty())
			toolpathPartsByZ[layout.m_EOSLayerZ[nIndex]].insert(scan.m_LayerParts[nIndex].begin(), scan.m_LayerParts[nIndex].end());
	}

	auto findNear = [nTolerance](auto& map, int64_t nZ) {
		auto iEntry = map.lower_bound(nZ - nTolerance);
		if ((iEntry != map.end()) && (iEntry->first <= nZ + nTolerance))
			return iEntry;
		return map.end();
	};

	struct SPartMismatch {
		uint64_t m_nCount = 0;
		int64_t m_nFirstZ = 0;
	};

	uint64_t nToolpathLayers = 0;
	uint64_t nLayersNotInStack = 0;
	int32_t nFirstMissingLayer = LIBEOS_CONVERSIONLOG_NOLAYER;
	int64_t nFirstMissingZ = 0;
	std::map<uint32_t, SPartMismatch> partsNotExposedInJob;

	for (size_t nIndex = 0; nIndex < scan.m_LayerParts.size(); nIndex++) {
		auto& layerParts = scan.m_LayerParts[nIndex];
		if (layerParts.empty())
			continue;
		nToolpathLayers++;

		int64_t nZ = layout.m_EOSLayerZ[nIndex];
		auto iStackLayer = findNear(stackIndexByZ, nZ);
		if (iStackLayer == stackIndexByZ.end()) {
			if (nLayersNotInStack == 0) {
				nFirstMissingLayer = (int32_t)nIndex;
				nFirstMissingZ = nZ;
			}
			nLayersNotInStack++;
			continue;
		}

		auto& exposedPartIDs = stack.m_Layers[iStackLayer->second].m_ExposedPartIDs;
		for (auto nPartID : layerParts) {
			if (std::find(exposedPartIDs.begin(), exposedPartIDs.end(), (int32_t)nPartID) == exposedPartIDs.end()) {
				auto& mismatch = partsNotExposedInJob[nPartID];
				if (mismatch.m_nCount == 0)
					mismatch.m_nFirstZ = nZ;
				mismatch.m_nCount++;
			}
		}
	}

	std::map<uint32_t, SPartMismatch> partsWithoutToolpath;
	for (auto& stackLayer : stack.m_Layers) {
		if (!stackLayer.m_bIsExposed)
			continue;
		auto iToolpathLayer = findNear(toolpathPartsByZ, stackLayer.m_nZPositionInMicrometers);
		for (auto nPartID : stackLayer.m_ExposedPartIDs) {
			if ((iToolpathLayer != toolpathPartsByZ.end()) && (iToolpathLayer->second.count((uint32_t)nPartID) > 0))
				continue;
			auto& mismatch = partsWithoutToolpath[(uint32_t)nPartID];
			if (mismatch.m_nCount == 0)
				mismatch.m_nFirstZ = stackLayer.m_nZPositionInMicrometers;
			mismatch.m_nCount++;
		}
	}

	uint64_t nPartLayersNotExposed = 0;
	for (auto& mismatch : partsNotExposedInJob) {
		nPartLayersNotExposed += mismatch.second.m_nCount;
		pLog->addMessage(eEOSLogSeverity::Warning, "PART_NOT_EXPOSED_IN_JOB", LIBEOS_CONVERSIONLOG_NOLAYER, mismatch.first, "the toolpath has " +
			std::to_string(mismatch.second.m_nCount) + " layers of the part where the job does not expose it, the first at EOS Z " + micrometersToMMString(mismatch.second.m_nFirstZ) + " mm");
	}

	uint64_t nPartLayersWithoutToolpath = 0;
	for (auto& mismatch : partsWithoutToolpath) {
		nPartLayersWithoutToolpath += mismatch.second.m_nCount;
		pLog->addMessage(eEOSLogSeverity::Warning, "PART_LAYERS_WITHOUT_TOOLPATH", LIBEOS_CONVERSIONLOG_NOLAYER, mismatch.first, "the job exposes the part in " +
			std::to_string(mismatch.second.m_nCount) + " layers without toolpath for it, the first at EOS Z " + micrometersToMMString(mismatch.second.m_nFirstZ) + " mm");
	}

	if (nLayersNotInStack > 0) {
		pLog->addMessage(bStrictMode ? eEOSLogSeverity::Error : eEOSLogSeverity::Warning, "LAYER_NOT_IN_STACK", nFirstMissingLayer, LIBEOS_CONVERSIONLOG_NOPART,
			std::to_string(nLayersNotInStack) + " of " + std::to_string(nToolpathLayers) + " toolpath layers have no layer at their Z in the layer stack of the job, the first at EOS Z " +
			micrometersToMMString(nFirstMissingZ) + " mm");
	}
	else if ((nPartLayersNotExposed == 0) && (nPartLayersWithoutToolpath == 0)) {
		pLog->addMessage(eEOSLogSeverity::Info, "LAYER_COVERAGE_COMPLETE", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"all " + std::to_string(nToolpathLayers) + " toolpath layers with vectors match the layer stack of the job");
	}

	pLog->setSummaryValue("stackLayers", stack.m_Layers.size());
	pLog->setSummaryValue("layersNotInStack", nLayersNotInStack);
	pLog->setSummaryValue("partLayersNotExposedInJob", nPartLayersNotExposed);
	pLog->setSummaryValue("partLayersWithoutToolpath", nPartLayersWithoutToolpath);

	auto& heights = stack.m_ProcessHeights;
	CJSONObject processHeights;
	processHeights.addNumber("preProcessHeight", heights.m_dPreProcessHeight);
	processHeights.addNumber("bottomOffsetHeight", heights.m_dBottomOffsetHeight);
	processHeights.addNumber("payloadHeight", heights.m_dPayloadHeight);
	processHeights.addNumber("topOffsetHeight", heights.m_dTopOffsetHeight);
	processHeights.addNumber("postProcessHeight", heights.m_dPostProcessHeight);
	processHeights.addNumber("accumulatedShrinkageCompensation", heights.m_dAccumulatedShrinkageCompensation);
	processHeights.addNumber("totalZTravel", heights.m_dTotalZTravel);
	processHeights.addNumber("maxZTravel", heights.m_dMaxZTravel);

	CJSONObject layerStack;
	layerStack.addBool("simulated", stack.m_bSimulated);
	layerStack.addUnsigned("layers", stack.m_Layers.size());
	if (!stack.m_Layers.empty()) {
		layerStack.addNumber("firstLayerZ", stack.m_Layers.front().m_nZPositionInMicrometers / 1000.0);
		layerStack.addNumber("lastLayerZ", stack.m_Layers.back().m_nZPositionInMicrometers / 1000.0);
	}
	else {
		layerStack.addNull("firstLayerZ");
		layerStack.addNull("lastLayerZ");
	}
	layerStack.addRaw("processHeights", processHeights.toString());
	pLog->setSection("layerStack", layerStack.toString());

	bool bZTravelExceeded = (heights.m_dMaxZTravel > 0.0) && (heights.m_dTotalZTravel > heights.m_dMaxZTravel);
	std::string sZTravel = "the job needs " + jsonNumber(heights.m_dTotalZTravel) + " mm of Z travel, the machine allows " + jsonNumber(heights.m_dMaxZTravel) + " mm";
	if (bZTravelExceeded)
		pLog->addMessage(eEOSLogSeverity::Error, "PROCESS_HEIGHT_EXCEEDED", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, sZTravel);
	else
		pLog->addMessage(eEOSLogSeverity::Info, "PROCESS_HEIGHTS", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, sZTravel);

	// The EOS SDK reports the compensation, but has no call to apply it to a toolpath.
	bool bHasShrinkageCompensation = (std::fabs(heights.m_dAccumulatedShrinkageCompensation) >= 0.0005);
	std::string sShrinkage = "the job compensates Z shrinkage with " + jsonNumber(heights.m_dAccumulatedShrinkageCompensation) +
		" mm, which cannot be applied to the toolpath; the toolpath layers are written at their own Z";
	if (bHasShrinkageCompensation) {
		if (bStrictMode)
			pLog->addMessage(eEOSLogSeverity::Error, "SHRINKAGE_COMPENSATION_REFUSED", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, sShrinkage);
		else
			pLog->addMessage(eEOSLogSeverity::Warning, "SHRINKAGE_COMPENSATION_IGNORED", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
				sShrinkage + "; include the compensation in the toolpath, or turn it off in the job");
	}

	if (!bThrowOnError)
		return nLayersNotInStack;
	if (bZTravelExceeded)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_PROCESSHEIGHTEXCEEDED, sZTravel);
	if (bStrictMode && (nLayersNotInStack > 0))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_LAYERSTACKMISMATCH, std::to_string(nLayersNotInStack) + " toolpath layers have no layer at their Z in the layer stack of the job, the first at EOS Z " +
			micrometersToMMString(nFirstMissingZ) + " mm");
	if (bStrictMode && bHasShrinkageCompensation)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_SHRINKAGECOMPENSATIONNOTSUPPORTED, sShrinkage);
	return nLayersNotInStack;
}

CStackLayerCursor::CStackLayerCursor(const SEOSLayerStack& stack)
	: m_Stack(stack), m_nNextLayer(0)
{
}

bool CStackLayerCursor::needsFilling(const sEOSLayerInfo& layer) const
{
	// A layer without parts has no part that could be begun.
	return layer.m_bIsExposed && !layer.m_ExposedPartIDs.empty();
}

std::vector<sEOSLayerInfo> CStackLayerCursor::passLayersUpTo(int64_t nZ)
{
	std::vector<sEOSLayerInfo> layers;
	while ((m_nNextLayer < m_Stack.m_Layers.size()) && (m_Stack.m_Layers[m_nNextLayer].m_nZPositionInMicrometers < nZ - TOOLPATHLAYERS_TOLERANCE_UM)) {
		if (needsFilling(m_Stack.m_Layers[m_nNextLayer]))
			layers.push_back(m_Stack.m_Layers[m_nNextLayer]);
		m_nNextLayer++;
	}
	while ((m_nNextLayer < m_Stack.m_Layers.size()) && (m_Stack.m_Layers[m_nNextLayer].m_nZPositionInMicrometers <= nZ + TOOLPATHLAYERS_TOLERANCE_UM))
		m_nNextLayer++;
	return layers;
}

std::vector<sEOSLayerInfo> CStackLayerCursor::passRemainingLayers()
{
	std::vector<sEOSLayerInfo> layers;
	for (; m_nNextLayer < m_Stack.m_Layers.size(); m_nNextLayer++) {
		if (needsFilling(m_Stack.m_Layers[m_nNextLayer]))
			layers.push_back(m_Stack.m_Layers[m_nNextLayer]);
	}
	return layers;
}

} // namespace Impl
} // namespace Lib3MF4EOS
