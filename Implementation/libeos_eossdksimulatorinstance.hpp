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


#ifndef __LIBEOS_EOSSDKSIMULATORINSTANCE
#define __LIBEOS_EOSSDKSIMULATORINSTANCE

#include <memory>
#include <string>

#include "libeos_toolpathcommands.hpp"

namespace Lib3MF4EOS {
	namespace Impl {

		enum class eSimulationMode {
			smNoSimulation = 0,
			smSimulateConnectivity = 1,
			smSimulateFullSDK = 2,
			smSimulatePlasticsConnectivity = 3,
			smSimulatePlasticsFullSDK = 4,
			// Full SDK simulation in which toolpath writers record their commands.
			smRecording = 5
		};

		constexpr uint32_t LIBEOS_SIMULATOR_MAXLASERSCANNERS = 16;

		class CEOSSDKSimulatorInstance {
		private:

			eSimulationMode m_SimulationMode;
			std::string m_sSDKVersion;
			PToolpathRecorder m_pToolpathRecorder;
			uint32_t m_nLaserScannerCount;

		protected:

		public:

			CEOSSDKSimulatorInstance(const std::string& sSDKVersion, eSimulationMode simulationMode);

			virtual ~CEOSSDKSimulatorInstance();

			std::string GetAPIVersion();

			eSimulationMode getSimulationMode();

			// Null unless the mode is smRecording.
			PToolpathRecorder getToolpathRecorder();

			// Laser scanners of the simulated machine. Once they are set, the recorder checks every layer against them.
			void setLaserScannerCount(uint32_t nCount);
			uint32_t getLaserScannerCount();

		};

		typedef std::shared_ptr<CEOSSDKSimulatorInstance> PEOSSDKSimulatorInstance;

	} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_EOSSDKSIMULATORINSTANCE
