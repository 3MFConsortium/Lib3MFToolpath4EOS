/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the 3MF Consortium nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE 3MF CONSORTIUM BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Abstract: This is the class definition of CEOSRawTaskWriter

*/

#include "libeos_eosrawtaskwriter.hpp"
#include "libeos_taskcreationfuture.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_jsonwriter.hpp"
#include "../Common/common_utils.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CEOSRawTaskWriter 
**************************************************************************************************************************/

#define LIBEOS_HATCHCHAIN_DISTANCEEPSILON 1.0e-6
#define LIBEOS_HATCHCHAIN_ANGLEEPSILON 1.0e-9

static SVectorValues effectiveValues(const Lib3MF4EOS::sEOSTaskPoint& point, const Lib3MF4EOS::sEOSTaskBlock& block)
{
	SVectorValues values;
	values.m_dLaserPower = (point.m_LaserPower > 0.0) ? point.m_LaserPower : block.m_DefaultLaserPower;
	values.m_dScanSpeed = (point.m_ScanSpeed > 0.0) ? point.m_ScanSpeed : block.m_ScanSpeed;
	if (point.m_HasLaserFocus)
		values.m_dLaserFocus = point.m_LaserFocus;
	else
		values.m_dLaserFocus = block.m_HasLaserFocus ? block.m_LaserFocus : EOS_FOCUS_NOMINAL;
	return values;
}

static double pointDistance(double dX1, double dY1, double dX2, double dY2)
{
	return std::sqrt((dX2 - dX1) * (dX2 - dX1) + (dY2 - dY1) * (dY2 - dY1));
}

// Whether hatch 2 starts where hatch 1 ends and continues in the same direction.
static bool hatchContinues(const Lib3MF4EOS::sEOSTaskPoint& start1, const Lib3MF4EOS::sEOSTaskPoint& end1,
	const Lib3MF4EOS::sEOSTaskPoint& start2, const Lib3MF4EOS::sEOSTaskPoint& end2)
{
	if ((std::fabs(start2.m_X - end1.m_X) > LIBEOS_HATCHCHAIN_DISTANCEEPSILON) || (std::fabs(start2.m_Y - end1.m_Y) > LIBEOS_HATCHCHAIN_DISTANCEEPSILON))
		return false;

	double dX1 = end1.m_X - start1.m_X;
	double dY1 = end1.m_Y - start1.m_Y;
	double dX2 = end2.m_X - start2.m_X;
	double dY2 = end2.m_Y - start2.m_Y;
	double dLength1 = std::sqrt(dX1 * dX1 + dY1 * dY1);
	double dLength2 = std::sqrt(dX2 * dX2 + dY2 * dY2);
	if ((dLength1 <= 0.0) || (dLength2 <= 0.0))
		return false;

	double dCross = dX1 * dY2 - dY1 * dX2;
	double dDot = dX1 * dX2 + dY1 * dY2;
	return (dDot > 0.0) && (std::fabs(dCross) <= LIBEOS_HATCHCHAIN_ANGLEEPSILON * dLength1 * dLength2 + LIBEOS_HATCHCHAIN_DISTANCEEPSILON * (dLength1 + dLength2));
}

CEOSRawTaskWriter::CEOSRawTaskWriter(PEOSSDK pSDK, AMCCommon::PWorkingDirectory pWorkingDirectory, const SConversionSettings& settings, PConversionLogData pLog)
	: m_pSDK(pSDK), m_pWorkingDirectory(pWorkingDirectory), m_bRecording(false),
	m_bHasWrittenLayer(false), m_nLastLayerIndex(0), m_nLastZHeightInMicrometers(0),
	m_Settings(settings), m_pLog(pLog), m_nLogLayerIndex(LIBEOS_CONVERSIONLOG_NOLAYER), m_pScannerState(nullptr),
	m_nCurrentScanner(0), m_nCurrentPartID(LIBEOS_CONVERSIONLOG_NOPART),
	m_bLayerOpeningUnavailableLogged(false), m_bBeamShapeUnavailableLogged(false), m_nLaserScannerCount(0), m_bHasLaserScannerCapabilities(false)
{
	if (pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
	if (pWorkingDirectory.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
	if (pLog.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
	m_Settings.validate();

	// Listed with 0, so that the log shows every counter.
	for (auto sKey : { "layersWritten", "layersSkipped", "layersFilled", "hatchesWritten", "polylinesWritten", "polylineVectorsWritten",
		"rampedHatches", "rampSubVectors", "hatchesJoined", "contoursJoined", "laserBreaksAvoided", "factorsClamped", "idleScannerParts", "profileParametersIgnored" })
		m_pLog->addSummaryValue(sKey, 0);

	m_sAbsoluteFileName = pWorkingDirectory->getPath() + "/output.rawtask";

	auto pRecorder = m_pSDK->getToolpathRecorder();
	if (pRecorder.get() != nullptr) {
		m_bRecording = true;
		m_pOutput = std::make_unique<CToolpathOutputRecording>(pRecorder);
	}
	else {
		m_pOutput = std::make_unique<CToolpathOutputSDK>(m_pSDK, m_sAbsoluteFileName);
	}
}

CEOSRawTaskWriter::~CEOSRawTaskWriter()
{
	try {
		Close();
	}
	catch (...) {
	}
}

std::string CEOSRawTaskWriter::GetAbsoluteFilename()
{
	return m_sAbsoluteFileName;
}

std::string CEOSRawTaskWriter::GetTaskDirectory()
{
	return m_pWorkingDirectory->getPath();
}

void CEOSRawTaskWriter::Close()
{
	if (m_pOutput.get() == nullptr)
		return;

	auto pOutput = std::move(m_pOutput);
	if (!m_Fingerprint.isEmpty())
		m_pLog->setSection("fingerprint", m_Fingerprint.toJSON());
	if (!m_ScannerStatistics.empty()) {
		CJSONArray scanners;
		for (auto& statistics : m_ScannerStatistics) {
			CJSONObject scanner;
			scanner.addUnsigned("scannerIndex", statistics.first);
			scanner.addUnsigned("exposedLayers", statistics.second.m_nExposedLayers);
			scanner.addUnsigned("idleLayers", statistics.second.m_nIdleLayers);
			scanner.addUnsigned("blocks", statistics.second.m_nBlocks);
			scanner.addUnsigned("preDelays", statistics.second.m_nPreDelays);
			scanner.addNumber("preDelayMicroseconds", statistics.second.m_dPreDelayMicroseconds);
			scanners.addRaw(scanner.toString());
		}
		m_pLog->setSection("scanners", scanners.toString());
	}
	pOutput->close();
}

void CEOSRawTaskWriter::emit(const SToolpathCommand& command)
{
	if (m_pOutput.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKHASBEENRELEASED);
	m_Fingerprint.add(command, m_nLogLayerIndex);
	m_pOutput->write(command);
}

void CEOSRawTaskWriter::WriteLayer(const Lib3MF4EOS_uint32 nLayerIndex, IEOSRawTaskLayer* pLayer)
{
	writeLayer(nLayerIndex, pLayer, (int32_t)nLayerIndex);
}

void CEOSRawTaskWriter::setLaserScannerCapabilities(const std::vector<uint64_t>& capabilities)
{
	m_LaserScannerCapabilities = capabilities;
	m_bHasLaserScannerCapabilities = true;
}

void CEOSRawTaskWriter::setLaserScannerCount(uint32_t nCount)
{
	m_nLaserScannerCount = nCount;
}

void CEOSRawTaskWriter::selectScannerState(uint32_t nScannerIndex)
{
	auto iState = m_ScannerStates.find(nScannerIndex);
	if (iState == m_ScannerStates.end()) {
		SScannerState state;
		// The focus of an earlier layer may still apply, so it is written again.
		if (m_FocusChangedScanners.count(nScannerIndex) > 0)
			state.m_dLaserFocus = -1.0;
		iState = m_ScannerStates.insert(std::make_pair(nScannerIndex, state)).first;
	}
	m_nCurrentScanner = nScannerIndex;
	m_pScannerState = &iState->second;
}

void CEOSRawTaskWriter::beginLayer(uint32_t nLayerIndex, int64_t nZHeightInMicrometers)
{
	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);
	if (m_pOutput.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKHASBEENRELEASED);

	if (m_bHasWrittenLayer) {
		if (nLayerIndex <= m_nLastLayerIndex)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "layer index " + std::to_string(nLayerIndex) + " is not greater than the previous layer index " + std::to_string(m_nLastLayerIndex));
		if (nZHeightInMicrometers <= m_nLastZHeightInMicrometers)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "layer Z height " + std::to_string(nZHeightInMicrometers) + "um is not greater than the previous layer Z height " + std::to_string(m_nLastZHeightInMicrometers) + "um");
	}

	emit(SToolpathCommand(eToolpathCommand::BeginLayer, { (double)nZHeightInMicrometers }));
	m_bHasWrittenLayer = true;
	m_nLastLayerIndex = nLayerIndex;
	m_nLastZHeightInMicrometers = nZHeightInMicrometers;
}

void CEOSRawTaskWriter::writeEmptyLayer(uint32_t nLayerIndex, int64_t nZHeightInMicrometers, uint32_t nPartID)
{
	beginLayer(nLayerIndex, nZHeightInMicrometers);

	uint32_t nScannerCount = (m_nLaserScannerCount > 0) ? m_nLaserScannerCount : 1;
	for (uint32_t nScanner = 0; nScanner < nScannerCount; nScanner++) {
		emit(SToolpathCommand(eToolpathCommand::SelectLaserScanner, { (double)nScanner }));
		emit(SToolpathCommand(eToolpathCommand::BeginPart, { (double)nPartID }));
		m_ScannerStatistics[nScanner].m_nIdleLayers++;
	}

	m_pLog->addSummaryValue("layersFilled", 1);
	m_pLog->addCountedMessage(eEOSLogSeverity::Info, "LAYER_FILLED", LIBEOS_CONVERSIONLOG_NOLAYER, nPartID,
		"a layer of the job's layer stack without toolpath was written without exposure; every laser scanner begins the part in it");
}

void CEOSRawTaskWriter::writeLayer(uint32_t nLayerIndex, IEOSRawTaskLayer* pLayer, int32_t nLogLayerIndex)
{
	if (pLayer == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

	std::vector<Lib3MF4EOS::sEOSTaskPoint> points;
	std::vector<Lib3MF4EOS::sEOSTaskBlock> blocks;

	Lib3MF4EOS_uint64 nPointCount = 0;
	pLayer->GetPointData(0, &nPointCount, nullptr);
	if (nPointCount > 0) {
		points.resize(nPointCount);
		pLayer->GetPointData(nPointCount, nullptr, points.data());
	}

	Lib3MF4EOS_uint64 nBlockCount = 0;
	pLayer->GetBlockData(0, &nBlockCount, nullptr);
	if (nBlockCount > 0) {
		blocks.resize(nBlockCount);
		pLayer->GetBlockData(nBlockCount, nullptr, blocks.data());
	}

	m_nLogLayerIndex = nLogLayerIndex;
	beginLayer(nLayerIndex, pLayer->GetZHeight());
	m_pLog->addSummaryValue("layersWritten", 1);

	// Contour joining looks at the next block, so all blocks are checked first.
	std::set<uint32_t> exposingScanners;
	for (auto& block : blocks) {
		if (block.m_EndPointIndex < block.m_StartPointIndex)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_STARTINDEXLARGERTHANENDINDEX);
		if (block.m_EndPointIndex > points.size())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPOINTINDEXINBLOCK);
		if ((m_nLaserScannerCount > 0) && (block.m_LaserScannerIndex >= m_nLaserScannerCount))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_LASERSCANNERNOTAVAILABLE, "layer " + std::to_string(nLogLayerIndex) + " uses laser scanner " +
				std::to_string(block.m_LaserScannerIndex) + ", but the machine configuration has " + std::to_string(m_nLaserScannerCount) + " laser scanners");
		m_ScannerStatistics[block.m_LaserScannerIndex].m_nBlocks++;
		exposingScanners.insert(block.m_LaserScannerIndex);
	}
	for (auto nScanner : exposingScanners)
		m_ScannerStatistics[nScanner].m_nExposedLayers++;

	// Every scanner is selected once, in ascending order. The scanners run their commands independently,
	// so only the order within a scanner matters.
	std::stable_sort(blocks.begin(), blocks.end(), [](const Lib3MF4EOS::sEOSTaskBlock& block1, const Lib3MF4EOS::sEOSTaskBlock& block2) {
		return block1.m_LaserScannerIndex < block2.m_LaserScannerIndex;
	});

	// The EOS task generator refuses a layer in which a laser scanner has no begun part (status 87, LayerDataUnavailable).
	// A scanner without work in this layer therefore gets a part begun at its place in the order, and nothing else: no exposure, no delay.
	uint32_t nNextIdleScanner = 0;
	auto beginIdleScannersBelow = [&](uint32_t nScannerLimit, uint32_t nPartID) {
		for (; nNextIdleScanner < (std::min)(nScannerLimit, m_nLaserScannerCount); nNextIdleScanner++) {
			if (exposingScanners.count(nNextIdleScanner) > 0)
				continue;
			emit(SToolpathCommand(eToolpathCommand::SelectLaserScanner, { (double)nNextIdleScanner }));
			emit(SToolpathCommand(eToolpathCommand::BeginPart, { (double)nPartID }));
			m_ScannerStatistics[nNextIdleScanner].m_nIdleLayers++;
			m_pLog->addSummaryValue("idleScannerParts", 1);
		}
	};

	m_ScannerStates.clear();
	m_pScannerState = nullptr;
	int32_t nLastPartID = -1;
	uint32_t nLastLaserScanner = UINT32_MAX;
	const Lib3MF4EOS::sEOSTaskBlock* pOpenPolylineBlock = nullptr;

	for (size_t nBlockIndex = 0; nBlockIndex < blocks.size(); nBlockIndex++) {
		auto& block = blocks[nBlockIndex];

		// A part is begun per scanner: after a scanner switch, the current part must be begun again.
		bool bScannerChanged = (block.m_LaserScannerIndex != nLastLaserScanner);
		if (bScannerChanged) {
			beginIdleScannersBelow(block.m_LaserScannerIndex, block.m_PartID);
			emit(SToolpathCommand(eToolpathCommand::SelectLaserScanner, { (double)block.m_LaserScannerIndex }));
			nLastLaserScanner = block.m_LaserScannerIndex;
			selectScannerState(block.m_LaserScannerIndex);
		}

		if (bScannerChanged || ((int32_t)block.m_PartID != nLastPartID)) {
			emit(SToolpathCommand(eToolpathCommand::BeginPart, { (double)block.m_PartID }));
			nLastPartID = (int32_t)block.m_PartID;
		}
		m_nCurrentPartID = block.m_PartID;

		writeLayerOpening(block.m_PartID);
		writeBlockState(block);

		if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::HatchBlock) {
			// Hatch blocks with the same state form one sequence of hatches, so touching hatches join across them.
			size_t nGroupEnd = nBlockIndex + 1;
			while ((nGroupEnd < blocks.size()) && canJoinHatchBlocks(blocks[nGroupEnd - 1], blocks[nGroupEnd]))
				nGroupEnd++;
			writeHatchBlocks(blocks, nBlockIndex, nGroupEnd - nBlockIndex, points);
			nBlockIndex = nGroupEnd - 1;
		}
		else if (block.m_Type == Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock) {
			bool bJoinsNext = (nBlockIndex + 1 < blocks.size()) && canJoinContours(block, blocks[nBlockIndex + 1], points);
			writePolylineBlock(block, points, pOpenPolylineBlock, bJoinsNext);
			pOpenPolylineBlock = bJoinsNext ? &block : nullptr;
		}
	}

	if (nLastPartID >= 0)
		beginIdleScannersBelow(m_nLaserScannerCount, (uint32_t)nLastPartID);

	m_ScannerStates.clear();
	m_pScannerState = nullptr;
	m_nCurrentPartID = LIBEOS_CONVERSIONLOG_NOPART;
	m_nLogLayerIndex = LIBEOS_CONVERSIONLOG_NOLAYER;
}

void CEOSRawTaskWriter::writeLayerOpening(uint32_t nPartID)
{
	auto& state = *m_pScannerState;
	if (state.m_bLayerOpened)
		return;
	state.m_bLayerOpened = true;

	if (!m_Settings.m_bLayerOpening)
		return;

	if (!m_pSDK->canWriteBeamShape()) {
		if (!m_bLayerOpeningUnavailableLogged) {
			m_pLog->addMessage(Lib3MF4EOS::eEOSLogSeverity::Warning, "SDK_FEATURE_UNAVAILABLE", m_nLogLayerIndex, nPartID,
				"the layer opening was not written, because the loaded EOS SDK cannot set the beam shape (EOS SDK 2.26 or later is needed)");
			m_bLayerOpeningUnavailableLogged = true;
		}
		return;
	}

	emit(SToolpathCommand(eToolpathCommand::SetBeamShape, { (double)EOS_BEAMSHAPE_STANDARD, 0.0 }));
	state.m_nBeamShapeProfile = 0;
	if (m_Settings.m_dLayerOpeningDelay > 0.0)
		writeDelay(m_Settings.m_dLayerOpeningDelay, "the layer opening delay");
}

void CEOSRawTaskWriter::writeBlockState(const Lib3MF4EOS::sEOSTaskBlock& block)
{
	auto& state = *m_pScannerState;

	if (block.m_PreDelay > 0.0) {
		writeDelay(block.m_PreDelay, "the pre-delay of a block");
		auto& statistics = m_ScannerStatistics[m_nCurrentScanner];
		statistics.m_nPreDelays++;
		statistics.m_dPreDelayMicroseconds += block.m_PreDelay;
	}

	if ((block.m_BeamShapeProfile >= 0) && (block.m_BeamShapeProfile != state.m_nBeamShapeProfile)) {
		if (m_pSDK->canWriteBeamShape()) {
			emit(SToolpathCommand(eToolpathCommand::SetBeamShape, { (double)EOS_BEAMSHAPE_STANDARD, (double)block.m_BeamShapeProfile }));
		}
		else if (block.m_BeamShapeProfile > 0) {
			std::string sReason = "beam shape profile " + std::to_string(block.m_BeamShapeProfile) +
				" was requested, but the loaded EOS SDK cannot set the beam shape (EOS SDK 2.26 or later is needed)";
			if (m_Settings.m_bStrictMode)
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED, "layer " + std::to_string(m_nLogLayerIndex) + ": " + sReason);
			if (!m_bBeamShapeUnavailableLogged) {
				m_pLog->addMessage(Lib3MF4EOS::eEOSLogSeverity::Warning, "SDK_FEATURE_UNAVAILABLE", m_nLogLayerIndex, block.m_PartID,
					sReason + "; the standard beam shape is used for all such blocks");
				m_bBeamShapeUnavailableLogged = true;
			}
		}
		state.m_nBeamShapeProfile = block.m_BeamShapeProfile;
	}

	if (block.m_ExposureType != state.m_nExposureType) {
		emit(SToolpathCommand(eToolpathCommand::SetExposureType, { (double)block.m_ExposureType }));
		state.m_nExposureType = block.m_ExposureType;
	}

	int32_t nSkywriting = block.m_SkywritingEnabled ? 1 : 0;
	if (nSkywriting != state.m_nSkywriting) {
		emit(SToolpathCommand(eToolpathCommand::SetSkywriting, { (double)nSkywriting }));
		state.m_nSkywriting = nSkywriting;
	}

	if ((block.m_ExposureSetIndex != state.m_nExposureSetIndex) || (block.m_ExposureStepIndex != state.m_nExposureStepIndex)) {
		emit(SToolpathCommand(eToolpathCommand::SetExposureStep, { (double)block.m_ExposureSetIndex, (double)block.m_ExposureStepIndex }));
		state.m_nExposureSetIndex = block.m_ExposureSetIndex;
		state.m_nExposureStepIndex = block.m_ExposureStepIndex;
	}

	if ((block.m_PulsedWavePeriodTime != state.m_dPulsedWavePeriodTime) || (block.m_PulsedWavePowerOnTime != state.m_dPulsedWavePowerOnTime)) {
		emit(SToolpathCommand(eToolpathCommand::SetLaserPulse, { block.m_PulsedWavePeriodTime, block.m_PulsedWavePowerOnTime }));
		state.m_dPulsedWavePeriodTime = block.m_PulsedWavePeriodTime;
		state.m_dPulsedWavePowerOnTime = block.m_PulsedWavePowerOnTime;
	}

	if (block.m_ExposedDepth != state.m_nExposedDepth) {
		emit(SToolpathCommand(eToolpathCommand::SetDepth, { (double)block.m_ExposedDepth }));
		state.m_nExposedDepth = block.m_ExposedDepth;
	}
}

void CEOSRawTaskWriter::writeDelay(double dDelayInMicroseconds, const std::string& sPurpose)
{
	try {
		emit(SToolpathCommand(eToolpathCommand::Delay, { dDelayInMicroseconds }));
	}
	catch (ELib3MF4EOSInterfaceException& e) {
		if (e.getErrorCode() != LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED)
			throw;
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED, "the loaded EOS SDK cannot write delays, so " + sPurpose + " in layer " +
			std::to_string(m_nLogLayerIndex) + " cannot be written");
	}
}

void CEOSRawTaskWriter::writeLaserPower(double dLaserPower, double dDeadband)
{
	auto& state = *m_pScannerState;
	if (dLaserPower == state.m_dLaserPower)
		return;
	if ((dDeadband > 0.0) && (state.m_dLaserPower >= 0.0) && (std::fabs(dLaserPower - state.m_dLaserPower) < dDeadband))
		return;
	emit(SToolpathCommand(eToolpathCommand::SetLaserPower, { dLaserPower, m_Settings.m_dPowerDelay }));
	state.m_dLaserPower = dLaserPower;
}

void CEOSRawTaskWriter::writeVectorValues(const SVectorValues& values, double dPowerDeadband)
{
	auto& state = *m_pScannerState;
	writeLaserPower(values.m_dLaserPower, dPowerDeadband);

	if (values.m_dScanSpeed != state.m_dScanSpeed) {
		emit(SToolpathCommand(eToolpathCommand::SetScanSpeed, { values.m_dScanSpeed }));
		state.m_dScanSpeed = values.m_dScanSpeed;
	}

	if (values.m_dLaserFocus != state.m_dLaserFocus)
		writeFocus(values.m_dLaserFocus);
}

void CEOSRawTaskWriter::writeFocus(double dFocus)
{
	auto& state = *m_pScannerState;
	std::string sWhere = "layer " + std::to_string(m_nLogLayerIndex) + ", laser scanner " + std::to_string(m_nCurrentScanner);

	if (!((dFocus >= 0.0) && (dFocus <= 1.0)))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDFOCUS, sWhere + ": the focus value " + std::to_string(dFocus) +
			" is outside 0.0 (fully defocused) to 1.0 (fully focused)");

	if (m_bHasLaserScannerCapabilities && (m_nCurrentScanner < m_LaserScannerCapabilities.size())) {
		uint64_t nCapabilities = m_LaserScannerCapabilities[m_nCurrentScanner];
		if ((nCapabilities & (EOS_LSC_FOCUS_DEFOCUS | EOS_LSC_VARIABLE_FOCUS)) == 0) {
			std::string sReason = sWhere + " cannot change the focus, but the toolpath requests a focus of " + std::to_string(dFocus);
			if (m_Settings.m_bStrictMode)
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_FOCUSNOTSUPPORTED, sReason);
			if (m_FocusUnavailableLogged.insert(m_nCurrentScanner).second)
				m_pLog->addMessage(Lib3MF4EOS::eEOSLogSeverity::Warning, "FOCUS_UNSUPPORTED", m_nLogLayerIndex, m_nCurrentPartID,
					sReason + "; the focus is left out, so the scanner exposes fully focused");
			return;
		}
		if (((nCapabilities & EOS_LSC_VARIABLE_FOCUS) == 0) && (dFocus != 0.0) && (dFocus != 1.0))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDFOCUS, sWhere + " can only focus fully (1.0) or defocus fully (0.0), so it cannot expose with a focus of " +
				std::to_string(dFocus));
	}

	emit(SToolpathCommand(eToolpathCommand::SetFocus, { dFocus }));
	state.m_dLaserFocus = dFocus;
	if (dFocus == EOS_FOCUS_NOMINAL)
		m_FocusChangedScanners.erase(m_nCurrentScanner);
	else
		m_FocusChangedScanners.insert(m_nCurrentScanner);
}

CEOSRawTaskWriter::SHatchLine CEOSRawTaskWriter::buildHatchLine(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points,
	uint32_t nFirstHatch, uint32_t nHatchCount, bool bHasRamp)
{
	auto& lineStart = points.at(block.m_StartPointIndex + nFirstHatch * 2);
	auto& lineEnd = points.at(block.m_StartPointIndex + (nFirstHatch + nHatchCount) * 2 - 1);

	SHatchLine line;
	line.m_dStartX = lineStart.m_X;
	line.m_dStartY = lineStart.m_Y;
	line.m_dEndX = lineEnd.m_X;
	line.m_dEndY = lineEnd.m_Y;
	line.m_nHatchCount = nHatchCount;

	if (!bHasRamp) {
		for (uint32_t nIndex = nFirstHatch; nIndex < nFirstHatch + nHatchCount; nIndex++) {
			auto& start = points.at(block.m_StartPointIndex + nIndex * 2);
			auto& end = points.at(block.m_StartPointIndex + nIndex * 2 + 1);
			line.m_Vectors.push_back(SPowerRampSubVector{ end.m_X, end.m_Y, effectiveValues(start, block) });
		}
		return line;
	}

	m_pLog->addSummaryValue("rampedHatches", nHatchCount);

	switch (m_Settings.m_ePowerRampMode) {
	case Lib3MF4EOS::eEOSPowerRampMode::Reject:
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_POWERRAMPREJECTED, "layer " + std::to_string(m_nLogLayerIndex) + " contains a ramp of laser power, scan speed or focus from (" +
			std::to_string(lineStart.m_X) + ", " + std::to_string(lineStart.m_Y) + ") to (" + std::to_string(lineEnd.m_X) + ", " + std::to_string(lineEnd.m_Y) + ")");

	case Lib3MF4EOS::eEOSPowerRampMode::Flatten: {
		// The mean of every value, weighted by length.
		SVectorValues sum = { 0.0, 0.0, 0.0 };
		SVectorValues unweightedSum = { 0.0, 0.0, 0.0 };
		double dLength = 0.0;
		for (uint32_t nIndex = nFirstHatch; nIndex < nFirstHatch + nHatchCount; nIndex++) {
			auto& start = points.at(block.m_StartPointIndex + nIndex * 2);
			auto& end = points.at(block.m_StartPointIndex + nIndex * 2 + 1);
			SVectorValues mean = CPowerRamp::interpolate(effectiveValues(start, block), effectiveValues(end, block), 0.5);
			double dHatchLength = pointDistance(start.m_X, start.m_Y, end.m_X, end.m_Y);
			sum.m_dLaserPower += mean.m_dLaserPower * dHatchLength;
			sum.m_dScanSpeed += mean.m_dScanSpeed * dHatchLength;
			sum.m_dLaserFocus += mean.m_dLaserFocus * dHatchLength;
			unweightedSum.m_dLaserPower += mean.m_dLaserPower;
			unweightedSum.m_dScanSpeed += mean.m_dScanSpeed;
			unweightedSum.m_dLaserFocus += mean.m_dLaserFocus;
			dLength += dHatchLength;
		}
		double dDivisor = (dLength > 0.0) ? dLength : (double)nHatchCount;
		SVectorValues& total = (dLength > 0.0) ? sum : unweightedSum;
		SVectorValues flattened = { total.m_dLaserPower / dDivisor, total.m_dScanSpeed / dDivisor, total.m_dLaserFocus / dDivisor };
		line.m_Vectors.push_back(SPowerRampSubVector{ line.m_dEndX, line.m_dEndY, flattened });
		m_pLog->addCountedMessage(Lib3MF4EOS::eEOSLogSeverity::Warning, "POWER_RAMP_FLATTENED", m_nLogLayerIndex, block.m_PartID,
			"a ramp of laser power, scan speed or focus was written at its mean values");
		return line;
	}

	default:
		break;
	}

	for (uint32_t nIndex = nFirstHatch; nIndex < nFirstHatch + nHatchCount; nIndex++) {
		auto& start = points.at(block.m_StartPointIndex + nIndex * 2);
		auto& end = points.at(block.m_StartPointIndex + nIndex * 2 + 1);
		CPowerRamp::appendSubVectors(m_Settings, start.m_X, start.m_Y, effectiveValues(start, block),
			end.m_X, end.m_Y, effectiveValues(end, block), line.m_Vectors);
	}
	m_pLog->addSummaryValue("rampSubVectors", line.m_Vectors.size());
	return line;
}

void CEOSRawTaskWriter::writeHatchBlocks(const std::vector<Lib3MF4EOS::sEOSTaskBlock>& blocks, size_t nFirstBlock, size_t nBlockCount, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points)
{
	auto& block = blocks[nFirstBlock];

	// Touching collinear hatches (for example the pieces of a nonlinear ramp) form one line.
	std::vector<SHatchLine> lines;
	for (size_t nBlockIndex = nFirstBlock; nBlockIndex < nFirstBlock + nBlockCount; nBlockIndex++) {
		auto& lineBlock = blocks[nBlockIndex];
		bool bDwellBlock = (lineBlock.m_PostDelay > 0.0);
		uint32_t nNumHatches = (lineBlock.m_EndPointIndex - lineBlock.m_StartPointIndex) / 2;
		uint32_t nHatchIndex = 0;
		while (nHatchIndex < nNumHatches) {
			bool bHasRamp = false;
			uint32_t nChainEnd = nHatchIndex;
			while (nChainEnd < nNumHatches) {
				auto& start = points.at(lineBlock.m_StartPointIndex + nChainEnd * 2);
				auto& end = points.at(lineBlock.m_StartPointIndex + nChainEnd * 2 + 1);
				if (nChainEnd > nHatchIndex) {
					// Every hatch of a block with a dwell is followed by that dwell, so it is a line of its own.
					if (bDwellBlock)
						break;
					auto& previousStart = points.at(lineBlock.m_StartPointIndex + nChainEnd * 2 - 2);
					auto& previousEnd = points.at(lineBlock.m_StartPointIndex + nChainEnd * 2 - 1);
					if (!hatchContinues(previousStart, previousEnd, start, end))
						break;
				}
				if (CPowerRamp::isRamp(effectiveValues(start, lineBlock), effectiveValues(end, lineBlock)))
					bHasRamp = true;
				nChainEnd++;
			}

			lines.push_back(buildHatchLine(lineBlock, points, nHatchIndex, nChainEnd - nHatchIndex, bHasRamp));
			lines.back().m_dPostDelay = lineBlock.m_PostDelay;
			nHatchIndex = nChainEnd;
		}
	}

	// A line with a dwell can end a run but not continue one, so the laser stays on up to the dwell. Split hatches are never joined.
	bool bCanJoin = (m_Settings.m_ePowerRampMode != Lib3MF4EOS::eEOSPowerRampMode::SplitHatches);

	size_t nLineIndex = 0;
	while (nLineIndex < lines.size()) {
		if (lines[nLineIndex].m_Vectors.empty()) {
			nLineIndex++;
			continue;
		}

		size_t nRunEnd = nLineIndex + 1;
		if (bCanJoin) {
			while ((nRunEnd < lines.size()) && (lines[nRunEnd - 1].m_dPostDelay <= 0.0) && !lines[nRunEnd].m_Vectors.empty() &&
				(pointDistance(lines[nRunEnd - 1].m_dEndX, lines[nRunEnd - 1].m_dEndY, lines[nRunEnd].m_dStartX, lines[nRunEnd].m_dStartY) <= m_Settings.m_dJoinTolerance))
				nRunEnd++;
		}

		writeHatchRun(block, lines, nLineIndex, nRunEnd - nLineIndex);
		if (lines[nRunEnd - 1].m_dPostDelay > 0.0)
			writeDelay(lines[nRunEnd - 1].m_dPostDelay, "the post-delay of a hatch");
		nLineIndex = nRunEnd;
	}
}

void CEOSRawTaskWriter::writeHatchRun(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<SHatchLine>& lines, size_t nFirstLine, size_t nLineCount)
{
	auto& firstLine = lines[nFirstLine];
	bool bSplitHatches = (m_Settings.m_ePowerRampMode == Lib3MF4EOS::eEOSPowerRampMode::SplitHatches);
	bool bAllPolylines = (m_Settings.m_eHatchWriteMode == Lib3MF4EOS::eEOSHatchWriteMode::AllPolylines) && !bSplitHatches;

	if ((nLineCount == 1) && !bAllPolylines && (bSplitHatches || (firstLine.m_Vectors.size() == 1))) {
		double dStartX = firstLine.m_dStartX;
		double dStartY = firstLine.m_dStartY;
		for (auto& vector : firstLine.m_Vectors) {
			writeVectorValues(vector.m_Values, 0.0);
			emit(SToolpathCommand(eToolpathCommand::Hatch, { dStartX, dStartY, vector.m_dEndX, vector.m_dEndY }));
			dStartX = vector.m_dEndX;
			dStartY = vector.m_dEndY;
		}
		m_pLog->addSummaryValue("hatchesWritten", firstLine.m_Vectors.size());
		return;
	}

	// The vectors of the polyline. A gap within the join tolerance becomes a short vector with the values of the next line.
	std::vector<SPowerRampSubVector> vectors;
	uint32_t nHatchCount = 0;
	for (size_t nLineIndex = nFirstLine; nLineIndex < nFirstLine + nLineCount; nLineIndex++) {
		auto& line = lines[nLineIndex];
		if (nLineIndex > nFirstLine) {
			auto& previousLine = lines[nLineIndex - 1];
			if ((line.m_dStartX != previousLine.m_dEndX) || (line.m_dStartY != previousLine.m_dEndY))
				vectors.push_back(SPowerRampSubVector{ line.m_dStartX, line.m_dStartY, line.m_Vectors.front().m_Values });
		}
		vectors.insert(vectors.end(), line.m_Vectors.begin(), line.m_Vectors.end());
		nHatchCount += line.m_nHatchCount;
	}

	m_pLog->addSummaryValue("polylinesWritten", 1);
	m_pLog->addSummaryValue("polylineVectorsWritten", vectors.size());
	m_pLog->addSummaryValue("hatchesJoined", nHatchCount - 1);
	m_pLog->addSummaryValue("laserBreaksAvoided", vectors.size() - 1);

	// A value change between polyline coordinates applies to the next vector, the laser stays on.
	writeVectorValues(vectors.front().m_Values, 0.0);
	emit(SToolpathCommand(eToolpathCommand::PolyStart, { firstLine.m_dStartX, firstLine.m_dStartY }));
	for (size_t nIndex = 0; nIndex < vectors.size(); nIndex++) {
		auto& vector = vectors[nIndex];
		writeVectorValues(vector.m_Values, (nIndex > 0) ? m_Settings.m_dHatchPowerDeadband : 0.0);
		if (nIndex + 1 < vectors.size())
			emit(SToolpathCommand(eToolpathCommand::PolyCoord, { vector.m_dEndX, vector.m_dEndY, 0.0 }));
		else
			emit(SToolpathCommand(eToolpathCommand::PolyEnd, { vector.m_dEndX, vector.m_dEndY }));
	}
}

bool CEOSRawTaskWriter::blockStatesMatch(const Lib3MF4EOS::sEOSTaskBlock& block1, const Lib3MF4EOS::sEOSTaskBlock& block2)
{
	// Nothing may be written between the two blocks.
	if ((block1.m_PartID != block2.m_PartID) || (block1.m_LaserScannerIndex != block2.m_LaserScannerIndex) ||
		(block1.m_ExposureType != block2.m_ExposureType) || (block1.m_SkywritingEnabled != block2.m_SkywritingEnabled) ||
		(block1.m_ExposureSetIndex != block2.m_ExposureSetIndex) || (block1.m_ExposureStepIndex != block2.m_ExposureStepIndex) ||
		(block1.m_PulsedWavePeriodTime != block2.m_PulsedWavePeriodTime) || (block1.m_PulsedWavePowerOnTime != block2.m_PulsedWavePowerOnTime) ||
		(block1.m_ExposedDepth != block2.m_ExposedDepth))
		return false;
	if ((block2.m_BeamShapeProfile >= 0) && (block2.m_BeamShapeProfile != block1.m_BeamShapeProfile))
		return false;
	return (block1.m_PostDelay <= 0.0) && (block2.m_PreDelay <= 0.0);
}

bool CEOSRawTaskWriter::canJoinHatchBlocks(const Lib3MF4EOS::sEOSTaskBlock& block1, const Lib3MF4EOS::sEOSTaskBlock& block2) const
{
	if ((block1.m_Type != Lib3MF4EOS::eEOSTaskBlockType::HatchBlock) || (block2.m_Type != Lib3MF4EOS::eEOSTaskBlockType::HatchBlock))
		return false;
	if (m_Settings.m_ePowerRampMode == Lib3MF4EOS::eEOSPowerRampMode::SplitHatches)
		return false;
	return blockStatesMatch(block1, block2);
}

bool CEOSRawTaskWriter::canJoinContours(const Lib3MF4EOS::sEOSTaskBlock& block1, const Lib3MF4EOS::sEOSTaskBlock& block2, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points) const
{
	if (!m_Settings.m_bJoinContours)
		return false;
	if ((block1.m_Type != Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock) || (block2.m_Type != Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock))
		return false;

	uint32_t nPointCount1 = block1.m_EndPointIndex - block1.m_StartPointIndex;
	uint32_t nPointCount2 = block2.m_EndPointIndex - block2.m_StartPointIndex;
	if ((nPointCount1 < 2) || (nPointCount2 < 2))
		return false;
	if (!blockStatesMatch(block1, block2))
		return false;

	auto& first1 = points.at(block1.m_StartPointIndex);
	auto& last1 = points.at(block1.m_EndPointIndex - 1);
	auto& first2 = points.at(block2.m_StartPointIndex);

	// A closed loop is never extended.
	if ((nPointCount1 >= 3) && (pointDistance(first1.m_X, first1.m_Y, last1.m_X, last1.m_Y) <= m_Settings.m_dJoinTolerance))
		return false;

	return pointDistance(last1.m_X, last1.m_Y, first2.m_X, first2.m_Y) <= m_Settings.m_dJoinTolerance;
}

void CEOSRawTaskWriter::writePolylineBlock(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points, const Lib3MF4EOS::sEOSTaskBlock* pContinuedBlock, bool bJoinsNext)
{
	uint32_t nNumPoints = block.m_EndPointIndex - block.m_StartPointIndex;
	if (nNumPoints < 2)
		return;

	auto& ptStart = points.at(block.m_StartPointIndex);
	SVectorValues firstValues = effectiveValues(points.at(block.m_StartPointIndex + 1), block);

	if (pContinuedBlock != nullptr) {
		m_pLog->addSummaryValue("contoursJoined", 1);
		m_pLog->addSummaryValue("laserBreaksAvoided", 1);
		auto& ptPreviousEnd = points.at(pContinuedBlock->m_EndPointIndex - 1);
		if ((ptStart.m_X != ptPreviousEnd.m_X) || (ptStart.m_Y != ptPreviousEnd.m_Y)) {
			writeVectorValues(firstValues, 0.0);
			emit(SToolpathCommand(eToolpathCommand::PolyCoord, { ptStart.m_X, ptStart.m_Y, 0.0 }));
			m_pLog->addSummaryValue("polylineVectorsWritten", 1);
		}
	}
	else {
		m_pLog->addSummaryValue("polylinesWritten", 1);
		writeVectorValues(firstValues, 0.0);
		emit(SToolpathCommand(eToolpathCommand::PolyStart, { ptStart.m_X, ptStart.m_Y }));
	}

	m_pLog->addSummaryValue("polylineVectorsWritten", nNumPoints - 1);
	for (uint32_t nPtIndex = 1; nPtIndex < nNumPoints; nPtIndex++) {
		auto& point = points.at(block.m_StartPointIndex + nPtIndex);
		writeVectorValues(effectiveValues(point, block), 0.0);
		if ((nPtIndex + 1 < nNumPoints) || bJoinsNext)
			emit(SToolpathCommand(eToolpathCommand::PolyCoord, { point.m_X, point.m_Y, 0.0 }));
		else
			emit(SToolpathCommand(eToolpathCommand::PolyEnd, { point.m_X, point.m_Y }));
	}

	if (!bJoinsNext && (block.m_PostDelay > 0.0))
		writeDelay(block.m_PostDelay, "the post-delay of a polyline");
}

ITaskCreationFuture* CEOSRawTaskWriter::ConvertToTask(const std::string& sName, const std::string& sComment,
	const std::string& sGeneratorName, const std::string& sGeneratorVersion)
{
	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	Close();

	if (m_bRecording)
		return new CTaskCreationFuture(m_pSDK, m_pWorkingDirectory, m_pLog, m_nLastLayerIndex, m_nLastZHeightInMicrometers);

	if (m_pSDK->EosToolpath_OpenInputFile == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);

	EOS_FILE_HANDLE hInputFile = 0;
	std::wstring sFileNameW = AMCCommon::CUtils::UTF8toUTF16(m_sAbsoluteFileName);
	m_pSDK->checkEOSError(m_pSDK->EosToolpath_OpenInputFile(sFileNameW.c_str(), &hInputFile));

	if (hInputFile == 0)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTOPENTASKFILE,
			"could not open input task file: " + m_sAbsoluteFileName);

	return new CTaskCreationFuture(m_pSDK, hInputFile, m_pWorkingDirectory,
		sName, sComment, sGeneratorName, sGeneratorVersion, m_pLog);
}

IEOSConversionLog* CEOSRawTaskWriter::GetConversionLog()
{
	return new CEOSConversionLog(m_pLog);
}

PConversionLogData CEOSRawTaskWriter::getLog()
{
	return m_pLog;
}
