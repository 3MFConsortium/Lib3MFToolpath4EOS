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


Abstract: This is the class declaration of CEOSRawTaskWriter

*/


#ifndef __LIBEOS_EOSRAWTASKWRITER
#define __LIBEOS_EOSRAWTASKWRITER

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

#include "libeos_eossdk.hpp"
#include "libeos_toolpathoutput.hpp"
#include "libeos_powerramp.hpp"
#include "libeos_eosconversionlog.hpp"
#include "../Common/common_workingdirectory.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CEOSRawTaskWriter 
**************************************************************************************************************************/

class CEOSRawTaskWriter : public virtual IEOSRawTaskWriter, public virtual CBase {
private:
	PEOSSDK m_pSDK;
	AMCCommon::PWorkingDirectory m_pWorkingDirectory;
	std::string m_sAbsoluteFileName;
	PToolpathOutput m_pOutput;
	bool m_bRecording;
	CToolpathFingerprint m_Fingerprint;

	bool m_bHasWrittenLayer;
	uint32_t m_nLastLayerIndex;
	int64_t m_nLastZHeightInMicrometers;

	// What was last written to a scanner in the current layer. The SDK applies every setting
	// to the selected scanner only.
	struct SScannerState {
		double m_dLaserPower = -1.0;
		double m_dScanSpeed = -1.0;
		double m_dLaserFocus = EOS_FOCUS_NOMINAL;
		int32_t m_nSkywriting = -1;
		uint32_t m_nExposureType = UINT32_MAX;
		// Exposure step, laser pulse and depth start at the values a reader assumes,
		// so that plain toolpaths do not carry these records.
		uint32_t m_nExposureSetIndex = 0;
		uint32_t m_nExposureStepIndex = 0;
		double m_dPulsedWavePeriodTime = 0.0;
		double m_dPulsedWavePowerOnTime = 0.0;
		uint32_t m_nExposedDepth = 0;
		int32_t m_nBeamShapeProfile = -1;
		bool m_bLayerOpened = false;
	};

	// A chain of touching collinear hatches, already split into vectors of constant values.
	struct SHatchLine {
		double m_dStartX;
		double m_dStartY;
		double m_dEndX;
		double m_dEndY;
		uint32_t m_nHatchCount;
		std::vector<SPowerRampSubVector> m_Vectors;
		// The dwell after the line, in microseconds.
		double m_dPostDelay = 0.0;
	};

	SConversionSettings m_Settings;
	PConversionLogData m_pLog;
	int32_t m_nLogLayerIndex;
	std::map<uint32_t, SScannerState> m_ScannerStates;
	SScannerState* m_pScannerState;
	uint32_t m_nCurrentScanner;
	uint32_t m_nCurrentPartID;
	bool m_bLayerOpeningUnavailableLogged;
	bool m_bBeamShapeUnavailableLogged;

	// Laser scanners of the machine; 0 if the machine configuration does not give them, and then the scanner indices
	// are only checked by the EOS SDK and scanners without work in a layer get no part.
	uint32_t m_nLaserScannerCount;

	struct SScannerStatistics {
		uint64_t m_nExposedLayers = 0;
		uint64_t m_nIdleLayers = 0;
		uint64_t m_nBlocks = 0;
		uint64_t m_nPreDelays = 0;
		double m_dPreDelayMicroseconds = 0.0;
	};
	std::map<uint32_t, SScannerStatistics> m_ScannerStatistics;

	// Capability bits (EOS_LSC_*) of every scanner, if the machine configuration gives them.
	bool m_bHasLaserScannerCapabilities;
	std::vector<uint64_t> m_LaserScannerCapabilities;
	// Scanners whose last focus was not the nominal focus, so their focus is not known at the start of the next layer.
	std::set<uint32_t> m_FocusChangedScanners;
	std::set<uint32_t> m_FocusUnavailableLogged;

	void emit(const SToolpathCommand& command);
	void beginLayer(uint32_t nLayerIndex, int64_t nZHeightInMicrometers);
	void selectScannerState(uint32_t nScannerIndex);
	void writeFocus(double dFocus);
	void writeLayerOpening(uint32_t nPartID);
	void writeBlockState(const Lib3MF4EOS::sEOSTaskBlock& block);
	void writeDelay(double dDelayInMicroseconds, const std::string& sPurpose);
	void writeLaserPower(double dLaserPower, double dDeadband);
	void writeVectorValues(const SVectorValues& values, double dPowerDeadband);
	SHatchLine buildHatchLine(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points, uint32_t nFirstHatch, uint32_t nHatchCount, bool bHasRamp);
	void writeHatchBlocks(const std::vector<Lib3MF4EOS::sEOSTaskBlock>& blocks, size_t nFirstBlock, size_t nBlockCount, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points);
	void writeHatchRun(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<SHatchLine>& lines, size_t nFirstLine, size_t nLineCount);
	static bool blockStatesMatch(const Lib3MF4EOS::sEOSTaskBlock& block1, const Lib3MF4EOS::sEOSTaskBlock& block2);
	bool canJoinHatchBlocks(const Lib3MF4EOS::sEOSTaskBlock& block1, const Lib3MF4EOS::sEOSTaskBlock& block2) const;
	bool canJoinContours(const Lib3MF4EOS::sEOSTaskBlock& block1, const Lib3MF4EOS::sEOSTaskBlock& block2, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points) const;
	void writePolylineBlock(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points, const Lib3MF4EOS::sEOSTaskBlock* pContinuedBlock, bool bJoinsNext);

public:
	CEOSRawTaskWriter(PEOSSDK pSDK, AMCCommon::PWorkingDirectory pWorkingDirectory, const SConversionSettings& settings, PConversionLogData pLog);

	virtual ~CEOSRawTaskWriter();

	std::string GetAbsoluteFilename() override;

	std::string GetTaskDirectory() override;

	void WriteLayer(const Lib3MF4EOS_uint32 nLayerIndex, IEOSRawTaskLayer* pLayer) override;

	// nLogLayerIndex is the layer index used in log messages. The blocks are written per scanner, in ascending
	// scanner order, and the blocks of one scanner keep their order.
	void writeLayer(uint32_t nLayerIndex, IEOSRawTaskLayer* pLayer, int32_t nLogLayerIndex);

	// Writes a layer without exposure, in which every scanner begins the part, for a layer of the job that has no toolpath.
	void writeEmptyLayer(uint32_t nLayerIndex, int64_t nZHeightInMicrometers, uint32_t nPartID);

	// Focus commands are checked against these capabilities. Without them, only the SDK checks them.
	void setLaserScannerCapabilities(const std::vector<uint64_t>& capabilities);

	// Blocks must use one of these scanners, and every scanner without work in a layer gets an empty part.
	void setLaserScannerCount(uint32_t nCount);

	ITaskCreationFuture* ConvertToTask(const std::string& sName, const std::string& sComment,
		const std::string& sGeneratorName, const std::string& sGeneratorVersion) override;

	IEOSConversionLog* GetConversionLog() override;

	PConversionLogData getLog();

	void Close();
};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSRAWTASKWRITER
