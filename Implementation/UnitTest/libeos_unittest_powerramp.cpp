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


Abstract: Tests of the power ramp splitting and the modifier evaluation.

*/

#include "libeos_unittests.hpp"
#include "libeos_powerramp.hpp"
#include "libeos_eostoolpathconversionsettings.hpp"

#include <cmath>

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

	class CPowerRampTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "PowerRamp";
		}

		void registerTests() override
		{
			registerTest("RampDetection", "differences of 1e-6 or less are no ramp", [this]() {
				assertFalse(CPowerRamp::isRamp(100.0, 100.0000005));
				assertTrue(CPowerRamp::isRamp(100.0, 100.1));
				assertFalse(CPowerRamp::isRamp(SVectorValues{ 100.0, 1000.0, 0.0 }, SVectorValues{ 100.0, 1000.0, 0.0 }));
				assertTrue(CPowerRamp::isRamp(SVectorValues{ 100.0, 1000.0, 0.0 }, SVectorValues{ 100.0, 1001.0, 0.0 }), "speed ramp");
				assertTrue(CPowerRamp::isRamp(SVectorValues{ 100.0, 1000.0, 0.0 }, SVectorValues{ 100.0, 1000.0, 0.1 }), "focus ramp");
				assertTrue(CPowerRamp::equalValues(SVectorValues{ 1.0, 2.0, 3.0 }, SVectorValues{ 1.0, 2.0, 3.0 }));
			});

			registerTest("Interpolate", "values are interpolated linearly", [this]() {
				auto values = CPowerRamp::interpolate(SVectorValues{ 100.0, 1000.0, 0.0 }, SVectorValues{ 200.0, 2000.0, 1.0 }, 0.25);
				assertNear(values.m_dLaserPower, 125.0, 1e-12);
				assertNear(values.m_dScanSpeed, 1250.0, 1e-12);
				assertNear(values.m_dLaserFocus, 0.25, 1e-12);
			});

			registerTest("SubVectorCount", "the value needing the most steps sets the count, limited by the shortest sub-vector", [this]() {
				SConversionSettings settings;
				SVectorValues base = { 100.0, 1000.0, 0.0 };
				assertEqual(CPowerRamp::subVectorCount(settings, base, base, 10.0), 1, "no ramp");
				assertEqual(CPowerRamp::subVectorCount(settings, base, SVectorValues{ 110.0, 1000.0, 0.0 }, 10.0), 10, "power steps of 1 W");
				assertEqual(CPowerRamp::subVectorCount(settings, base, SVectorValues{ 100.0, 1100.0, 0.0 }, 10.0), 10, "speed steps of 10 mm/s");
				assertEqual(CPowerRamp::subVectorCount(settings, base, SVectorValues{ 100.0, 1000.0, 0.2 }, 10.0), 4, "focus steps of 0.05");
				assertEqual(CPowerRamp::subVectorCount(settings, base, SVectorValues{ 110.0, 1000.0, 0.0 }, 0.2), 4, "limited by 0.05 mm sub-vectors");
				assertEqual(CPowerRamp::subVectorCount(settings, base, SVectorValues{ 110.0, 1000.0, 0.0 }, 0.01), 1, "shorter than one sub-vector");

				settings.m_dMaxPowerStep = 0.5;
				assertEqual(CPowerRamp::subVectorCount(settings, base, SVectorValues{ 110.0, 1000.0, 0.0 }, 10.0), 20, "smaller power step");
			});

			registerTest("SubVectorsKeepEnergy", "sub-vectors take the mean value of their interval and end at the vector end", [this]() {
				SConversionSettings settings;
				std::vector<SPowerRampSubVector> subVectors;
				CPowerRamp::appendSubVectors(settings, 0.0, 0.0, SVectorValues{ 100.0, 1000.0, 0.0 }, 3.0, 4.0, SVectorValues{ 107.0, 1000.0, 0.0 }, subVectors);

				assertEqual((int64_t)subVectors.size(), 7);
				double dEnergy = 0.0;
				double dPreviousX = 0.0;
				double dPreviousY = 0.0;
				for (auto& subVector : subVectors) {
					double dLength = std::sqrt((subVector.m_dEndX - dPreviousX) * (subVector.m_dEndX - dPreviousX) + (subVector.m_dEndY - dPreviousY) * (subVector.m_dEndY - dPreviousY));
					dEnergy += subVector.m_Values.m_dLaserPower * dLength;
					dPreviousX = subVector.m_dEndX;
					dPreviousY = subVector.m_dEndY;
				}
				assertNear(dEnergy, 103.5 * 5.0, 1e-9, "power times length");
				assertNear(subVectors.front().m_Values.m_dLaserPower, 100.5, 1e-9);
				assertNear(subVectors.back().m_Values.m_dLaserPower, 106.5, 1e-9);
				assertTrue((subVectors.back().m_dEndX == 3.0) && (subVectors.back().m_dEndY == 4.0), "the last sub-vector ends exactly at the vector end");
			});

			registerTest("ZeroLengthVector", "a vector without length adds no sub-vector", [this]() {
				SConversionSettings settings;
				std::vector<SPowerRampSubVector> subVectors;
				CPowerRamp::appendSubVectors(settings, 1.0, 1.0, SVectorValues{ 100.0, 1000.0, 0.0 }, 1.0, 1.0, SVectorValues{ 150.0, 1000.0, 0.0 }, subVectors);
				assertEqual((int64_t)subVectors.size(), 0);
			});

			registerTest("ModifierValue", "a modifier factor maps linearly onto its range and is clamped to [0, 1]", [this]() {
				assertNear(CPowerRamp::modifiedValue(100.0, 200.0, 0.25), 125.0, 1e-12);
				assertNear(CPowerRamp::modifiedValue(100.0, 200.0, -1.0), 100.0, 1e-12);
				assertNear(CPowerRamp::modifiedValue(100.0, 200.0, 2.0), 200.0, 1e-12);
				assertNear(CPowerRamp::modifiedValue(0.0, 300.0, 1.0), 300.0, 1e-12);
			});
		}
	};

	PUnitTestGroup createPowerRampTests()
	{
		return std::make_shared<CPowerRampTests>();
	}

}
