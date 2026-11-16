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


Abstract: Tests of the conversion settings, their validation and the exposure sets per layer thickness.

*/

#include "libeos_unittests.hpp"
#include "libeos_eostoolpathconversionsettings.hpp"

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

	class CConversionSettingsTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "ConversionSettings";
		}

		void registerTests() override
		{
			registerTest("Defaults", "the defaults are valid and strict", [this]() {
				SConversionSettings settings;
				settings.validate();
				assertTrue(settings.m_bStrictMode);
				assertTrue(settings.m_ePowerRampMode == Lib3MF4EOS::eEOSPowerRampMode::Polyline);
				assertTrue(settings.m_eHatchWriteMode == Lib3MF4EOS::eEOSHatchWriteMode::JoinTouching);
				assertTrue(settings.m_eLayerZConvention == Lib3MF4EOS::eEOSLayerZConvention::Top);
				assertFalse(settings.m_bAllowDefaultExposureSets);
				assertTrue(settings.m_ExposureSetsForThickness.empty());
			});

			registerTest("SetterValidation", "setters refuse values outside their range", [this]() {
				CEOSToolpathConversionSettings settings;
				assertThrowsError([&]() { settings.SetPowerDelay(-1.0); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "power delay");
				assertThrowsError([&]() { settings.SetMaxPowerStep(0.0); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "power step");
				assertThrowsError([&]() { settings.SetJoinTolerance(-0.1); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "join tolerance");
				assertThrowsError([&]() { settings.SetDefaultHatchExposureType(1000); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "exposure type");
				assertThrowsError([&]() { settings.SetPowerRampMode((Lib3MF4EOS::eEOSPowerRampMode)99); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "ramp mode");
				assertThrowsError([&]() { settings.SetLayerZConvention((Lib3MF4EOS::eEOSLayerZConvention)99); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "Z convention");

				settings.SetJoinTolerance(0.01);
				assertNear(settings.GetJoinTolerance(), 0.01, 0.0);
				settings.SetPowerRampMode(Lib3MF4EOS::eEOSPowerRampMode::Flatten);
				assertTrue(settings.getSettings().m_ePowerRampMode == Lib3MF4EOS::eEOSPowerRampMode::Flatten);
				settings.ResetToDefaults();
				assertTrue(settings.GetPowerRampMode() == Lib3MF4EOS::eEOSPowerRampMode::Polyline);
			});

			registerTest("StructValidation", "validate refuses invalid values set directly", [this]() {
				SConversionSettings settings;
				settings.m_dMinSubVectorLength = 0.0;
				assertThrowsError([&]() { settings.validate(); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS);

				SConversionSettings assignment;
				assignment.m_ExposureSetsForThickness[std::make_pair(40u, false)] = "";
				assertThrowsError([&]() { assignment.validate(); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS);
			});

			registerTest("ExposureSetsForThickness", "exposure sets are kept per thickness and support flag", [this]() {
				CEOSToolpathConversionSettings settings;
				settings.SetExposureSetForThickness(40, false, "Part 40");
				settings.SetExposureSetForThickness(40, true, "Support 40");
				settings.SetExposureSetForThickness(80, false, "Part 80");

				assertEqualString(settings.GetExposureSetForThickness(40, false), "Part 40");
				assertEqualString(settings.GetExposureSetForThickness(40, true), "Support 40");
				assertEqualString(settings.GetExposureSetForThickness(80, false), "Part 80");
				assertEqualString(settings.GetExposureSetForThickness(80, true), "");
				assertEqualString(settings.GetExposureSetForThickness(60, false), "");

				settings.SetExposureSetForThickness(80, false, "");
				assertEqualString(settings.GetExposureSetForThickness(80, false), "", "an empty name removes the assignment");
				assertThrowsError([&]() { settings.SetExposureSetForThickness(0, false, "Zero"); }, LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS);

				std::string sJSON = settings.ToJSON();
				assertContains(sJSON, "\"exposureSetsForThickness\":[{\"thickness\":40,\"isSupport\":false,\"exposureSet\":\"Part 40\"},{\"thickness\":40,\"isSupport\":true,\"exposureSet\":\"Support 40\"}]");

				settings.ClearExposureSetsForThickness();
				assertContains(settings.ToJSON(), "\"exposureSetsForThickness\":[]");
			});

			registerTest("JSON", "the settings JSON lists every setting", [this]() {
				std::string sJSON = SConversionSettings().toJSON();
				for (auto sKey : { "strictMode", "powerDelay", "maxPowerStep", "maxSpeedStep", "maxFocusStep", "minSubVectorLength", "powerRampMode",
					"hatchWriteMode", "joinTolerance", "hatchPowerDeadband", "joinContours", "refuseUnequalConstantHatch", "defaultSkywriting",
					"defaultHatchExposureType", "defaultContourExposureType", "layerOpening", "layerOpeningDelay", "dwellTimeUnit", "layerZConvention",
					"allowDefaultExposureSets", "exposureSetsForThickness" })
					assertContains(sJSON, std::string("\"") + sKey + "\":");
				assertContains(sJSON, "\"powerRampMode\":\"Polyline\"");
				assertContains(sJSON, "\"layerZConvention\":\"Top\"");
			});
		}
	};

	PUnitTestGroup createConversionSettingsTests()
	{
		return std::make_shared<CConversionSettingsTests>();
	}

}
