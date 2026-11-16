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


Abstract: Tests of the SDK version selection, the adapter registry and the simulation modes.
The structure layouts of every SDK version are checked at compile time.

*/

#include "libeos_unittests.hpp"
#include "libeos_eossdk.hpp"
#include "libeos_eossdk_adapter.hpp"

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

#ifdef LIBEOS_SDK_2_25
	static const char* SUPPORTED_VERSIONS = "2.18,2.25,2.26";
#else
	static const char* SUPPORTED_VERSIONS = "2.18,2.26";
#endif

	class CSDKVersionTests : public CUnitTestGroup {
	private:
		static bool anyMessageContains(const std::vector<std::string>& messages, const std::string& sPart)
		{
			for (auto& sMessage : messages) {
				if (sMessage.find(sPart) != std::string::npos)
					return true;
			}
			return false;
		}

	public:
		std::string getName() override
		{
			return "SDKVersions";
		}

		void registerTests() override
		{
			registerTest("KnownVersions", "the registry knows exactly the supported versions", [this]() {
				assertEqualString(CEOSSDKAdapterRegistry::getSupportedVersionsString(), SUPPORTED_VERSIONS);
				assertTrue(CEOSSDKAdapterRegistry::isKnownVersion(2, 18));
				assertTrue(CEOSSDKAdapterRegistry::isKnownVersion(2, 26));
#ifdef LIBEOS_SDK_2_25
				assertTrue(CEOSSDKAdapterRegistry::isKnownVersion(2, 25));
#else
				assertFalse(CEOSSDKAdapterRegistry::isKnownVersion(2, 25));
#endif
				assertFalse(CEOSSDKAdapterRegistry::isKnownVersion(2, 20));
				assertEqual(CEOSSDKAdapterRegistry::createLowestAdapter()->getVersionMinor(), 18);
			});

			registerTest("ExactVersion", "a known version gets its own adapter without message", [this]() {
				std::vector<std::string> messages;
				auto pAdapter = CEOSSDKAdapterRegistry::createAdapter(2, 26, messages);
				assertTrue(pAdapter.get() != nullptr);
				assertEqualString(pAdapter->getVersionString(), "2.26");
				assertEqual((int64_t)messages.size(), 0);
			});

#ifdef LIBEOS_SDK_2_25
			registerTest("Version225", "2.25 gets its own adapter, which has the beam shape record", [this]() {
				std::vector<std::string> messages;
				auto pAdapter = CEOSSDKAdapterRegistry::createAdapter(2, 25, messages);
				assertEqualString(pAdapter->getVersionString(), "2.25");
				assertEqual((int64_t)messages.size(), 0);
				assertTrue(pAdapter->versionHasBeamShape(), "beam shape");
			});
#endif

			registerTest("NearestLowerVersion", "an unknown version uses the nearest lower known version and says so", [this]() {
				std::vector<std::string> messages;
				auto pAdapter = CEOSSDKAdapterRegistry::createAdapter(2, 22, messages);
				assertEqualString(pAdapter->getVersionString(), "2.18");
				assertEqual((int64_t)messages.size(), 1);
				assertContains(messages[0], "2.22");

				messages.clear();
				pAdapter = CEOSSDKAdapterRegistry::createAdapter(3, 1, messages);
				assertEqualString(pAdapter->getVersionString(), "2.26");
				assertContains(messages[0], "3.1");
			});

			registerTest("TooOldVersion", "a version below the lowest known one gets no adapter", [this]() {
				std::vector<std::string> messages;
				assertTrue(CEOSSDKAdapterRegistry::createAdapter(2, 10, messages).get() == nullptr);
				assertTrue(CEOSSDKAdapterRegistry::createAdapter(1, 99, messages).get() == nullptr);
			});

			registerTest("AdapterFeatures", "the beam shape is in the API from SDK 2.26, and usable once the library exports it", [this]() {
				std::vector<std::string> messages;
				auto pAdapter218 = CEOSSDKAdapterRegistry::createAdapter(2, 18, messages);
				auto pAdapter226 = CEOSSDKAdapterRegistry::createAdapter(2, 26, messages);
				assertFalse(pAdapter218->versionHasBeamShape());
				assertTrue(pAdapter226->versionHasBeamShape());
				assertFalse(pAdapter218->supportsBeamShape());
				assertFalse(pAdapter226->supportsBeamShape(), "no library is loaded");
				assertFalse(pAdapter218->getOpenJobVersion().empty());
				assertFalse(pAdapter226->getOpenJobVersion().empty());
			});

			registerTest("BeamShapeInRecording", "a recording writes the beam shape if the SDK version has it", [this]() {
				CEOSSDK sdk226("", "", 2, 26);
				assertFalse(sdk226.canWriteBeamShape(), "no library and no recording");
				sdk226.setToSimulationMode(eSimulationMode::smRecording);
				assertTrue(sdk226.canWriteBeamShape());

				CEOSSDK sdk218("", "", 2, 18);
				sdk218.setToSimulationMode(eSimulationMode::smRecording);
				assertFalse(sdk218.canWriteBeamShape());
			});

			registerTest("Override", "an override selects the adapter of an SDK without library", [this]() {
				CEOSSDK sdk("", "", 2, 26);
				assertFalse(sdk.hasLibrary());
				assertEqualString(sdk.getAdapter().getVersionString(), "2.26");
				assertTrue(anyMessageContains(sdk.getVersionMessages(), "override 2.26"));

				CEOSSDK unknownSDK("", "", 2, 30);
				assertEqualString(unknownSDK.getAdapter().getVersionString(), "2.26");
				assertTrue(anyMessageContains(unknownSDK.getVersionMessages(), "2.30"));
			});

			registerTest("OverrideTooOld", "an override below the lowest known version fails", [this]() {
				std::string sMessage = assertThrowsError([]() { CEOSSDK sdk("", "", 1, 5); }, LIB3MF4EOS_ERROR_DETECTEDINCOMPATIBLEEOSSDKVERSION);
				assertContains(sMessage, SUPPORTED_VERSIONS);
			});

			registerTest("NoOverride", "without library and override, the lowest version is simulated", [this]() {
				CEOSSDK sdk("", "", 0, 0);
				assertEqualString(sdk.getAdapter().getVersionString(), "2.18");
				assertTrue(anyMessageContains(sdk.getVersionMessages(), "simulating version 2.18"));
			});

			registerTest("RecordingMode", "only the recording mode provides a toolpath recorder", [this]() {
				CEOSSDK sdk("", "", 2, 26);
				assertTrue(sdk.getToolpathRecorder().get() == nullptr, "no simulation");
				sdk.setToSimulationMode(eSimulationMode::smSimulateFullSDK);
				assertTrue(sdk.getToolpathRecorder().get() == nullptr, "full simulation");
				sdk.setToSimulationMode(eSimulationMode::smRecording);
				assertTrue(sdk.getToolpathRecorder().get() != nullptr, "recording");
				assertTrue(sdk.simulateFullSDK());
				assertTrue(sdk.simulateConnectivity());
			});

			registerTest("MachineTarget", "the M 290 is called M291 in OpenJob files", [this]() {
				assertEqualString(eosModelTypeToIdentifier(EOS_MT_M290), "M290");
				assertEqualString(eosModelTypeToMachineTarget(EOS_MT_M290), "M291");
				assertEqualString(eosModelTypeToMachineTarget(EOS_MT_M400), "M400");
			});
		}
	};

	PUnitTestGroup createSDKVersionTests()
	{
		return std::make_shared<CSDKVersionTests>();
	}

}
