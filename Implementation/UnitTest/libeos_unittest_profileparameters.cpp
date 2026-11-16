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


Abstract: Tests of the toolpath profile parameter check: the parameters LibEOS writes to EOS, jumpspeed,
unknown parameters, and parameters in unknown namespaces.

*/

#include "libeos_unittests.hpp"
#include "libeos_unittest_helpers.hpp"
#include "libeos_toolpathprofileparameters.hpp"
#include "libeos_namespaceregistry.hpp"

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

	static const std::string EOS_PROFILE_NAMESPACE = LIBEOS_NAMESPACE_TOOLPATHPROFILE;
	static const std::string LEGACY_PROFILE_NAMESPACE = "http://example.com/eos/legacy";

	class CProfileParameterTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "ProfileParameters";
		}

		void registerTests() override
		{
			registerTest("KnownParameters", "the parameters LibEOS writes pass, in the current and in registered legacy namespaces", [this]() {
				std::vector<std::string> accepted = { EOS_PROFILE_NAMESPACE, LEGACY_PROFILE_NAMESPACE };
				for (auto sName : { "laserpower", "laserspeed", "laserindex", "laserfocus", "prewaittime", "postwaittime" })
					assertEqualString(unsupportedProfileParameterReason("", sName, accepted), "", sName);
				for (auto sName : { "focus", "exposuretype", "skywriting", "exposuresetindex", "exposurestepindex", "pulsedwaveperiodtime",
					"pulsedwavepowerontime", "exposeddepth", "predelay", "postdelay", "beamshapeprofile" }) {
					assertEqualString(unsupportedProfileParameterReason(EOS_PROFILE_NAMESPACE, sName, accepted), "", sName);
					assertEqualString(unsupportedProfileParameterReason(LEGACY_PROFILE_NAMESPACE, sName, accepted), "", sName);
				}
			});

			registerTest("UnsupportedParameters", "jumpspeed, unknown names and unknown namespaces are reported with their reason", [this]() {
				std::vector<std::string> accepted = { EOS_PROFILE_NAMESPACE };
				assertContains(unsupportedProfileParameterReason("", "jumpspeed", accepted), "jump speed");
				assertContains(unsupportedProfileParameterReason("", "laserdiameter", accepted), "unknown");
				assertContains(unsupportedProfileParameterReason(EOS_PROFILE_NAMESPACE, "laserpower", accepted), "EOS parameter is unknown");
				assertContains(unsupportedProfileParameterReason(LEGACY_PROFILE_NAMESPACE, "focus", accepted), "namespace is unknown");
			});

			registerTest("ProfileScan", "all parameters of a lib3mf profile are checked, in profile order", [this]() {
				auto pWrapper = Lib3MF::CWrapper::loadLibrary(LIBEOS_UNITTEST_LIB3MF_LIBRARY);
				auto pModel = pWrapper->CreateModel();
				auto pProfile = pModel->AddToolpath(0.001)->AddProfile("hatch");
				pProfile->SetParameterDoubleValue("", "laserpower", 200.0);
				pProfile->SetParameterDoubleValue("", "laserspeed", 1000.0);
				pProfile->SetParameterDoubleValue("", "prewaittime", 5.0);
				pProfile->SetParameterDoubleValue(EOS_PROFILE_NAMESPACE, "focus", 1.0);
				pProfile->SetParameterDoubleValue("", "jumpspeed", 5000.0);
				pProfile->SetParameterValue("http://example.com/vendor", "gas", "argon");

				auto parameters = findUnsupportedProfileParameters(pProfile, { EOS_PROFILE_NAMESPACE });
				assertEqual((int64_t)parameters.size(), 2);
				assertEqualString(parameters[0].m_sName, "jumpspeed");
				assertEqualString(parameters[1].m_sName, "http://example.com/vendor:gas");
				assertContains(parameters[1].m_sReason, "namespace is unknown");
			});
		}
	};

	PUnitTestGroup createProfileParameterTests()
	{
		return std::make_shared<CProfileParameterTests>();
	}

}
