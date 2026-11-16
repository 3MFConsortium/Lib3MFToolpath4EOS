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


Abstract: Tests of the EOS to 3MF conversion: cutting blocks where speed or focus change, and exposure type names.

*/

#include "libeos_unittests.hpp"
#include "libeos_unittest_helpers.hpp"
#include "libeos_eosto3mfconverter.hpp"

using namespace Lib3MF4EOS::Impl;
using Lib3MF4EOS::eEOSTaskBlockType;

namespace LibEOSUnitTest {

	class CEOSTo3MFTests : public CUnitTestGroup {
	private:
		static std::vector<Lib3MF4EOS::sEOSTaskBlock> split(const Lib3MF4EOS::sEOSTaskBlock& block, const std::vector<Lib3MF4EOS::sEOSTaskPoint>& points)
		{
			std::vector<Lib3MF4EOS::sEOSTaskBlock> blocks;
			CEOSTo3MFConverter::splitBlockAtValueChanges(block, points, blocks);
			return blocks;
		}

	public:
		std::string getName() override
		{
			return "EOSTo3MF";
		}

		void registerTests() override
		{
			registerTest("ConstantBlockStays", "a block with constant speed and focus is not cut", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(10, 0), taskPoint(0, 1), taskPoint(10, 1) };
				auto blocks = split(taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4), points);
				assertEqual((int64_t)blocks.size(), 1);
				assertEqual(blocks[0].m_StartPointIndex, 0);
				assertEqual(blocks[0].m_EndPointIndex, 4);
			});

			registerTest("HatchSpeedChange", "hatches are grouped into runs of equal speed, using the mean of each hatch", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = {
					taskPoint(0, 0, 0, 800), taskPoint(10, 0, 0, 800),
					taskPoint(0, 1, 0, 800), taskPoint(10, 1, 0, 800),
					taskPoint(0, 2, 0, 600), taskPoint(10, 2, 0, 800) };
				auto block = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 6);
				block.m_PreDelay = 12.0;
				auto blocks = split(block, points);

				assertEqual((int64_t)blocks.size(), 2);
				assertEqual(blocks[0].m_StartPointIndex, 0);
				assertEqual(blocks[0].m_EndPointIndex, 4);
				assertNear(blocks[0].m_ScanSpeed, 800.0, 1e-9);
				assertNear(blocks[0].m_PreDelay, 12.0, 0.0, "the first part keeps the pre-delay");
				assertEqual(blocks[1].m_StartPointIndex, 4);
				assertEqual(blocks[1].m_EndPointIndex, 6);
				assertNear(blocks[1].m_ScanSpeed, 700.0, 1e-9, "mean of the ramp from 600 to 800");
				assertNear(blocks[1].m_PreDelay, 0.0, 0.0, "later parts have no pre-delay");
			});

			registerTest("PolylineSpeedChange", "a polyline is cut where the speed changes, and the parts share the joint", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(1, 0, 0, 900), taskPoint(2, 0, 0, 900), taskPoint(3, 0, 0, 700), taskPoint(4, 0, 0, 700) };
				auto blocks = split(taskBlock(eEOSTaskBlockType::PolylineBlock, 0, 5), points);

				assertEqual((int64_t)blocks.size(), 2);
				assertEqual(blocks[0].m_StartPointIndex, 0);
				assertEqual(blocks[0].m_EndPointIndex, 3);
				assertNear(blocks[0].m_ScanSpeed, 900.0, 0.0);
				assertEqual(blocks[1].m_StartPointIndex, 2);
				assertEqual(blocks[1].m_EndPointIndex, 5);
				assertNear(blocks[1].m_ScanSpeed, 700.0, 0.0);
			});

			registerTest("InvalidRange", "a block outside the points fails", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(1, 0) };
				std::vector<Lib3MF4EOS::sEOSTaskBlock> blocks;
				bool bThrown = false;
				try {
					CEOSTo3MFConverter::splitBlockAtValueChanges(taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4), points, blocks);
				}
				catch (std::exception&) {
					bThrown = true;
				}
				assertTrue(bThrown);
			});

			registerTest("ExposureTypeNames", "exposure type names map both ways, unknown types as typeN", [this]() {
				assertEqualString(CEOSTo3MFConverter::exposureTypeName(2), "infill");
				assertEqualString(CEOSTo3MFConverter::exposureTypeName(10), "contour_simple");
				assertEqualString(CEOSTo3MFConverter::exposureTypeName(42), "type42");
				for (uint32_t nType = 0; nType <= 20; nType++) {
					uint32_t nParsed = 9999;
					assertTrue(CEOSTo3MFConverter::exposureTypeFromName(CEOSTo3MFConverter::exposureTypeName(nType), nParsed), "type " + std::to_string(nType));
					assertEqual(nParsed, nType);
				}
				uint32_t nParsed = 0;
				assertFalse(CEOSTo3MFConverter::exposureTypeFromName("unknown", nParsed));
				assertFalse(CEOSTo3MFConverter::exposureTypeFromName("type", nParsed));
				assertFalse(CEOSTo3MFConverter::exposureTypeFromName("type1x", nParsed));
			});
		}
	};

	PUnitTestGroup createEOSTo3MFTests()
	{
		return std::make_shared<CEOSTo3MFTests>();
	}

}
