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


Abstract: Tests of the toolpath command recorder, its ordering rules and the toolpath fingerprint.

*/

#include "libeos_unittests.hpp"
#include "libeos_toolpathcommands.hpp"
#include "lib3mftoolpath4eos_types.hpp"

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

	typedef eToolpathCommand C;

	class CToolpathRecorderTests : public CUnitTestGroup {
	private:
		// File, layer at 50 um, scanner 0 and part 7.
		static void startPart(CToolpathRecorder& recorder)
		{
			recorder.record(SToolpathCommand(C::OpenFile, {}));
			recorder.record(SToolpathCommand(C::BeginLayer, { 50.0 }));
			recorder.record(SToolpathCommand(C::SelectLaserScanner, { 0.0 }));
			recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
		}

		void expectRule(CToolpathRecorder& recorder, const SToolpathCommand& command, const std::string& sRule)
		{
			uint64_t nCountBefore = recorder.getCommandCount();
			std::string sMessage = assertThrowsError([&]() { recorder.record(command); }, LIB3MF4EOS_ERROR_TOOLPATHORDERVIOLATION, sRule);
			assertContains(sMessage, "rule " + sRule, "violation message");
			assertEqual((int64_t)recorder.getCommandCount(), (int64_t)nCountBefore, "a violating command is not kept");
		}

	public:
		std::string getName() override
		{
			return "ToolpathRecorder";
		}

		void registerTests() override
		{
			registerTest("NumberText", "numbers are written with at most six decimals and no trailing zeros", [this]() {
				assertEqualString(toolpathNumberToString(2.0), "2");
				assertEqualString(toolpathNumberToString(1.5), "1.5");
				assertEqualString(toolpathNumberToString(0.1234567), "0.123457");
				assertEqualString(toolpathNumberToString(-0.0000001), "0");
				assertEqualString(toolpathNumberToString(-3.25), "-3.25");
				assertEqualString(toolpathCommandToText(SToolpathCommand(C::Hatch, { 1.5, 2.0, 11.5, 2.0 })), "Hatch 1.5 2 11.5 2");
				assertEqualString(toolpathCommandToText(SToolpathCommand(C::PolyEnd, {})), "PolyEnd");
			});

			registerTest("TooManyArguments", "a command takes at most four arguments", [this]() {
				assertThrowsError([]() { SToolpathCommand(C::Hatch, { 1.0, 2.0, 3.0, 4.0, 5.0 }); }, LIB3MF4EOS_ERROR_INVALIDPARAM);
			});

			registerTest("ValidSequence", "a well-formed file is recorded in order", [this]() {
				CToolpathRecorder recorder;
				startPart(recorder);
				recorder.record(SToolpathCommand(C::SetExposureType, { 2.0 }));
				recorder.record(SToolpathCommand(C::SetLaserPower, { 200.0, 190.0 }));
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 10.0, 0.0 }));
				recorder.record(SToolpathCommand(C::PolyStart, { 0.0, 1.0 }));
				recorder.record(SToolpathCommand(C::SetLaserPower, { 150.0, 190.0 }));
				recorder.record(SToolpathCommand(C::PolyCoord, { 5.0, 1.0, 0.0 }));
				recorder.record(SToolpathCommand(C::PolyEnd, { 10.0, 1.0 }));
				recorder.record(SToolpathCommand(C::BeginLayer, { 100.0 }));
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 1.0 }));
				recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 10.0, 0.0 }));
				recorder.record(SToolpathCommand(C::CloseFile, {}));

				assertEqual((int64_t)recorder.getCommandCount(), 16);
				assertEqual((int64_t)recorder.getLayerCount(), 2);
				assertEqualString(toolpathCommandToText(recorder.getCommand(1)), "BeginLayer 50");
				auto lines = recorder.toText();
				assertContains(lines, "SetLaserPower 150 190\nPolyCoord 5 1 0\n");
				assertThrowsError([&]() { recorder.getCommand(16); }, LIB3MF4EOS_ERROR_INVALIDPARAM);

				recorder.clear();
				assertEqual((int64_t)recorder.getCommandCount(), 0);
				assertEqual((int64_t)recorder.getLayerCount(), 0);
			});

			registerTest("NoFile", "commands need an open file", [this]() {
				CToolpathRecorder recorder;
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 50.0 }), "NO_FILE");
				startPart(recorder);
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
				recorder.record(SToolpathCommand(C::CloseFile, {}));
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 100.0 }), "NO_FILE");
			});

			registerTest("NoLayer", "commands other than BeginLayer need a layer", [this]() {
				CToolpathRecorder recorder;
				recorder.record(SToolpathCommand(C::OpenFile, {}));
				expectRule(recorder, SToolpathCommand(C::SelectLaserScanner, { 0.0 }), "NO_LAYER");
			});

			registerTest("NoScanner", "parts and settings need a selected scanner", [this]() {
				CToolpathRecorder recorder;
				recorder.record(SToolpathCommand(C::OpenFile, {}));
				recorder.record(SToolpathCommand(C::BeginLayer, { 50.0 }));
				expectRule(recorder, SToolpathCommand(C::BeginPart, { 1.0 }), "NO_SCANNER");
				expectRule(recorder, SToolpathCommand(C::SetScanSpeed, { 1000.0 }), "NO_SCANNER");
				expectRule(recorder, SToolpathCommand(C::Delay, { 10.0 }), "NO_SCANNER");
			});

			registerTest("NoPartAfterScannerSwitch", "an exposure after a scanner switch needs the part again (SDK status 92)", [this]() {
				CToolpathRecorder recorder;
				startPart(recorder);
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 1.0 }));
				expectRule(recorder, SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }), "NO_PART");
				expectRule(recorder, SToolpathCommand(C::PolyStart, { 0.0, 0.0 }), "NO_PART");
				recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
			});

			registerTest("NoPartInNewLayer", "a new layer needs the scanner and the part again", [this]() {
				CToolpathRecorder recorder;
				startPart(recorder);
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
				recorder.record(SToolpathCommand(C::BeginLayer, { 100.0 }));
				expectRule(recorder, SToolpathCommand(C::BeginPart, { 7.0 }), "NO_SCANNER");
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 0.0 }));
				expectRule(recorder, SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }), "NO_PART");
			});

			registerTest("EmptyLayer", "a layer with neither exposure nor a begun part is refused", [this]() {
				CToolpathRecorder recorder;
				recorder.record(SToolpathCommand(C::OpenFile, {}));
				recorder.record(SToolpathCommand(C::BeginLayer, { 50.0 }));
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 0.0 }));
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 100.0 }), "EMPTY_LAYER");
				expectRule(recorder, SToolpathCommand(C::CloseFile, {}), "EMPTY_LAYER");
			});

			registerTest("FilledLayer", "a layer without exposure is accepted if every scanner begins a part, as in a filled layer", [this]() {
				CToolpathRecorder recorder;
				recorder.setLaserScannerCount(2);
				startPart(recorder);
				recorder.record(SToolpathCommand(C::SetLaserPower, { 200.0, 190.0 }));
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 100.0 }), "SCANNER_WITHOUT_PART");
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 1.0 }));
				recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
				recorder.record(SToolpathCommand(C::BeginLayer, { 100.0 }));
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 0.0 }));
				recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 1.0 }));
				recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
				recorder.record(SToolpathCommand(C::CloseFile, {}));
				assertEqual((int64_t)recorder.getLayerCount(), 2);
			});

			registerTest("ScannerWithoutPart", "with a known scanner count, every scanner needs a begun part in every layer (SDK status 87)", [this]() {
				CToolpathRecorder recorder;
				recorder.setLaserScannerCount(2);
				startPart(recorder);
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 100.0 }), "SCANNER_WITHOUT_PART");
				expectRule(recorder, SToolpathCommand(C::CloseFile, {}), "SCANNER_WITHOUT_PART");
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 1.0 }));
				recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
				recorder.record(SToolpathCommand(C::CloseFile, {}));
			});

			registerTest("ScannerRange", "with a known scanner count, a scanner the machine does not have is refused (SDK status 95)", [this]() {
				CToolpathRecorder recorder;
				recorder.setLaserScannerCount(2);
				startPart(recorder);
				expectRule(recorder, SToolpathCommand(C::SelectLaserScanner, { 2.0 }), "SCANNER_RANGE");
			});

			registerTest("FocusRules", "a focus must be between 0 and 1 and needs a part", [this]() {
				CToolpathRecorder recorder;
				recorder.record(SToolpathCommand(C::OpenFile, {}));
				recorder.record(SToolpathCommand(C::BeginLayer, { 50.0 }));
				recorder.record(SToolpathCommand(C::SelectLaserScanner, { 0.0 }));
				expectRule(recorder, SToolpathCommand(C::SetFocus, { 1.0 }), "NO_PART");
				recorder.record(SToolpathCommand(C::BeginPart, { 7.0 }));
				expectRule(recorder, SToolpathCommand(C::SetFocus, { 1.5 }), "FOCUS_RANGE");
				expectRule(recorder, SToolpathCommand(C::SetFocus, { -0.1 }), "FOCUS_RANGE");
				recorder.record(SToolpathCommand(C::SetFocus, { 0.0 }));
				recorder.record(SToolpathCommand(C::SetFocus, { 1.0 }));
			});

			registerTest("LayerOrder", "layer Z must increase", [this]() {
				CToolpathRecorder recorder;
				startPart(recorder);
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 50.0 }), "LAYER_ORDER");
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 40.0 }), "LAYER_ORDER");
			});

			registerTest("PolylineRules", "polyline points need a start, and only value changes may interrupt a polyline", [this]() {
				CToolpathRecorder recorder;
				startPart(recorder);
				expectRule(recorder, SToolpathCommand(C::PolyCoord, { 1.0, 0.0, 0.0 }), "NO_POLYLINE");
				expectRule(recorder, SToolpathCommand(C::PolyEnd, { 1.0, 0.0 }), "NO_POLYLINE");
				recorder.record(SToolpathCommand(C::PolyStart, { 0.0, 0.0 }));
				recorder.record(SToolpathCommand(C::SetScanSpeed, { 800.0 }));
				recorder.record(SToolpathCommand(C::SetFocus, { 0.5 }));
				expectRule(recorder, SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }), "POLYLINE_OPEN");
				expectRule(recorder, SToolpathCommand(C::SetExposureType, { 5.0 }), "POLYLINE_OPEN");
				expectRule(recorder, SToolpathCommand(C::BeginLayer, { 100.0 }), "POLYLINE_OPEN");
				recorder.record(SToolpathCommand(C::PolyEnd, { 1.0, 0.0 }));
				recorder.record(SToolpathCommand(C::SetExposureType, { 5.0 }));
			});

			registerTest("RecordingAcrossFiles", "a new file resets the file state, the commands are kept", [this]() {
				CToolpathRecorder recorder;
				startPart(recorder);
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
				recorder.record(SToolpathCommand(C::CloseFile, {}));
				startPart(recorder);
				recorder.record(SToolpathCommand(C::Hatch, { 0.0, 0.0, 1.0, 0.0 }));
				assertEqual((int64_t)recorder.getCommandCount(), 11);
				assertEqual((int64_t)recorder.getLayerCount(), 2);
			});

			registerTest("FingerprintPerLayer", "the fingerprint hashes every layer and ignores the file commands", [this]() {
				auto build = [](double dHatchEnd, bool bWithFileCommands) {
					CToolpathFingerprint fingerprint;
					if (bWithFileCommands)
						fingerprint.add(SToolpathCommand(C::OpenFile, {}), -1);
					fingerprint.add(SToolpathCommand(C::BeginLayer, { 50.0 }), 0);
					fingerprint.add(SToolpathCommand(C::Hatch, { 0.0, 0.0, 10.0, 0.0 }), 0);
					fingerprint.add(SToolpathCommand(C::BeginLayer, { 100.0 }), 1);
					fingerprint.add(SToolpathCommand(C::Hatch, { 0.0, 0.0, dHatchEnd, 0.0 }), 1);
					if (bWithFileCommands)
						fingerprint.add(SToolpathCommand(C::CloseFile, {}), -1);
					return fingerprint;
				};

				CToolpathFingerprint empty;
				assertTrue(empty.isEmpty());

				auto reference = build(10.0, false);
				auto withFile = build(10.0, true);
				auto changed = build(10.5, false);
				auto belowResolution = build(10.0 + 1.0e-8, false);

				assertFalse(reference.isEmpty());
				assertEqual((int64_t)reference.getLayerCount(), 2);
				assertTrue(reference.getTotalHash() == withFile.getTotalHash(), "file commands are not hashed");
				assertTrue(reference.getTotalHash() == belowResolution.getTotalHash(), "differences below 1e-6 are not hashed");
				assertTrue(reference.getTotalHash() != changed.getTotalHash(), "a changed coordinate changes the hash");
				assertTrue(reference.getLayerHash(0) == changed.getLayerHash(0), "the unchanged layer keeps its hash");
				assertTrue(reference.getLayerHash(1) != changed.getLayerHash(1), "the changed layer gets another hash");

				assertEqualString(CToolpathFingerprint::hashToString(0x1234), "0000000000001234");
				std::string sJSON = reference.toJSON();
				assertContains(sJSON, "\"algorithm\":\"FNV-1a 64\"");
				assertContains(sJSON, "\"layers\":2");
				assertContains(sJSON, "\"layerIndex\":1");
				assertContains(sJSON, "\"z\":0.1");
				assertContains(sJSON, "\"commands\":2");
				assertContains(sJSON, "\"hash\":\"" + CToolpathFingerprint::hashToString(reference.getTotalHash()) + "\"");
			});
		}
	};

	PUnitTestGroup createToolpathRecorderTests()
	{
		return std::make_shared<CToolpathRecorderTests>();
	}

}
