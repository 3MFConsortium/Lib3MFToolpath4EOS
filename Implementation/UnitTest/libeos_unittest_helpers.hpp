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


Abstract: Raw task data builders and a raw task writer that records into a simulated EOS SDK.

*/

#ifndef __LIBEOS_UNITTEST_HELPERS
#define __LIBEOS_UNITTEST_HELPERS

#include "libeos_eosrawtaskwriter.hpp"
#include "libeos_eosrawtasklayer.hpp"
#include "libeos_eossdk.hpp"
#include "libeos_eostoolpathconversionsettings.hpp"

#include <memory>
#include <string>
#include <vector>

namespace LibEOSUnitTest {

	// Values of 0 for power and speed mean "use the block value", as in the raw task API.
	Lib3MF4EOS::sEOSTaskPoint taskPoint(double dX, double dY, double dLaserPower = 0.0, double dScanSpeed = 0.0);

	// A block with 200 W, 1000 mm/s, no focus, infill exposure type and no delays.
	Lib3MF4EOS::sEOSTaskBlock taskBlock(Lib3MF4EOS::eEOSTaskBlockType eType, uint32_t nStartPointIndex, uint32_t nEndPointIndex, uint32_t nPartID = 1, uint32_t nLaserScannerIndex = 0);

	std::vector<std::string> splitLines(const std::string& sText);

	// A raw task writer on an SDK without library in recording mode.
	class CRecordingWriter {
	private:
		Lib3MF4EOS::Impl::PEOSSDK m_pSDK;
		Lib3MF4EOS::Impl::PToolpathRecorder m_pRecorder;
		Lib3MF4EOS::Impl::PConversionLogData m_pLog;
		std::unique_ptr<Lib3MF4EOS::Impl::CEOSRawTaskWriter> m_pWriter;
		uint32_t m_nNextLayerIndex;

	public:
		CRecordingWriter(uint32_t nSDKMajor, uint32_t nSDKMinor, const Lib3MF4EOS::Impl::SConversionSettings& settings);
		~CRecordingWriter();

		// The EOS_LSC_* flags of each scanner.
		void setLaserScannerCapabilities(const std::vector<uint64_t>& capabilities);
		// The scanners of the machine, for the writer and the recorder.
		void setLaserScannerCount(uint32_t nCount);
		void writeLayer(int64_t nZInMicrometers, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points, const std::vector<Lib3MF4EOS::sEOSTaskBlock>& blocks);
		void writeEmptyLayer(int64_t nZInMicrometers, uint32_t nPartID);
		void close();

		Lib3MF4EOS::Impl::PConversionLogData getLog();
		Lib3MF4EOS::Impl::PToolpathRecorder getRecorder();

		// The recorded commands, one text line each.
		std::vector<std::string> getLines();
		// The recorded commands without OpenFile, BeginLayer, CloseFile.
		std::vector<std::string> getLayerContentLines();
		size_t countCommands(const std::string& sCommandName);
	};

}

#endif // __LIBEOS_UNITTEST_HELPERS
