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

*/

#include "libeos_unittest_helpers.hpp"
#include "Common/common_workingdirectory.hpp"

#include <deque>
#include <sstream>

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

Lib3MF4EOS::sEOSTaskPoint taskPoint(double dX, double dY, double dLaserPower, double dScanSpeed)
{
	Lib3MF4EOS::sEOSTaskPoint point;
	point.m_X = dX;
	point.m_Y = dY;
	point.m_LaserPower = dLaserPower;
	point.m_ScanSpeed = dScanSpeed;
	point.m_LaserFocus = 0.0;
	point.m_HasLaserFocus = false;
	point.m_Timestamp = 0.0;
	return point;
}

Lib3MF4EOS::sEOSTaskBlock taskBlock(Lib3MF4EOS::eEOSTaskBlockType eType, uint32_t nStartPointIndex, uint32_t nEndPointIndex, uint32_t nPartID, uint32_t nLaserScannerIndex)
{
	Lib3MF4EOS::sEOSTaskBlock block;
	block.m_Type = eType;
	block.m_StartPointIndex = nStartPointIndex;
	block.m_EndPointIndex = nEndPointIndex;
	block.m_PartID = nPartID;
	block.m_LaserScannerIndex = nLaserScannerIndex;
	block.m_DefaultLaserPower = 200.0;
	block.m_ScanSpeed = 1000.0;
	block.m_LaserFocus = 0.0;
	block.m_HasLaserFocus = false;
	block.m_PreDelay = 0.0;
	block.m_ExposureSetIndex = 0;
	block.m_ExposureStepIndex = 0;
	block.m_ExposureType = 2;
	block.m_SkywritingEnabled = false;
	block.m_PulsedWavePeriodTime = 0.0;
	block.m_PulsedWavePowerOnTime = 0.0;
	block.m_ExposedDepth = 0;
	block.m_PostDelay = 0.0;
	block.m_BeamShapeProfile = -1;
	block.m_StripeIndex = 0;
	return block;
}

std::vector<std::string> splitLines(const std::string& sText)
{
	std::vector<std::string> lines;
	std::istringstream stream(sText);
	std::string sLine;
	while (std::getline(stream, sLine)) {
		if (!sLine.empty())
			lines.push_back(sLine);
	}
	return lines;
}


CRecordingWriter::CRecordingWriter(uint32_t nSDKMajor, uint32_t nSDKMinor, const SConversionSettings& settings)
	: m_nNextLayerIndex(1)
{
	m_pSDK = std::make_shared<CEOSSDK>("", "", nSDKMajor, nSDKMinor);
	m_pSDK->setToSimulationMode(eSimulationMode::smRecording);
	m_pRecorder = m_pSDK->getToolpathRecorder();
	if (m_pRecorder.get() == nullptr)
		throw std::runtime_error("the simulated SDK does not record");

	m_pLog = std::make_shared<CConversionLogData>();
	auto pWorkingDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_ut");
	m_pWriter.reset(new CEOSRawTaskWriter(m_pSDK, pWorkingDirectory, settings, m_pLog));
}

CRecordingWriter::~CRecordingWriter()
{
}

void CRecordingWriter::writeLayer(int64_t nZInMicrometers, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points, const std::vector<Lib3MF4EOS::sEOSTaskBlock>& blocks)
{
	CEOSRawTaskLayer layer(nZInMicrometers);
	layer.initializeData(std::deque<Lib3MF4EOS::sEOSTaskPoint>(points.begin(), points.end()), std::deque<Lib3MF4EOS::sEOSTaskBlock>(blocks.begin(), blocks.end()));
	uint32_t nLayerIndex = m_nNextLayerIndex++;
	m_pWriter->writeLayer(nLayerIndex, &layer, (int32_t)nLayerIndex);
}

void CRecordingWriter::writeEmptyLayer(int64_t nZInMicrometers, uint32_t nPartID)
{
	m_pWriter->writeEmptyLayer(m_nNextLayerIndex++, nZInMicrometers, nPartID);
}

void CRecordingWriter::setLaserScannerCapabilities(const std::vector<uint64_t>& capabilities)
{
	m_pWriter->setLaserScannerCapabilities(capabilities);
}

void CRecordingWriter::setLaserScannerCount(uint32_t nCount)
{
	m_pWriter->setLaserScannerCount(nCount);
	m_pRecorder->setLaserScannerCount(nCount);
}

void CRecordingWriter::close()
{
	m_pWriter->Close();
}

PConversionLogData CRecordingWriter::getLog()
{
	return m_pLog;
}

PToolpathRecorder CRecordingWriter::getRecorder()
{
	return m_pRecorder;
}

std::vector<std::string> CRecordingWriter::getLines()
{
	return splitLines(m_pRecorder->toText());
}

std::vector<std::string> CRecordingWriter::getLayerContentLines()
{
	std::vector<std::string> lines;
	for (auto& sLine : getLines()) {
		if ((sLine.rfind("OpenFile", 0) == 0) || (sLine.rfind("BeginLayer", 0) == 0) || (sLine.rfind("CloseFile", 0) == 0))
			continue;
		lines.push_back(sLine);
	}
	return lines;
}

size_t CRecordingWriter::countCommands(const std::string& sCommandName)
{
	size_t nCount = 0;
	for (auto& sLine : getLines()) {
		if ((sLine == sCommandName) || (sLine.rfind(sCommandName + " ", 0) == 0))
			nCount++;
	}
	return nCount;
}

}
