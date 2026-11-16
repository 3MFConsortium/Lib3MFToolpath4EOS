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


Abstract: Tests of the raw task writer: per-scanner state, parts, layer opening, block state, hatch and
contour joining, ramps, and the fingerprint, checked on the recorded command stream.

*/

#include "libeos_unittests.hpp"
#include "libeos_unittest_helpers.hpp"

using namespace Lib3MF4EOS::Impl;
using Lib3MF4EOS::eEOSTaskBlockType;

namespace LibEOSUnitTest {

	class CRawTaskWriterTests : public CUnitTestGroup {
	private:
		void assertLines(const std::vector<std::string>& actual, const std::vector<std::string>& expected)
		{
			std::string sActual;
			for (auto& sLine : actual)
				sActual += sLine + "; ";
			for (size_t nIndex = 0; nIndex < (std::min)(actual.size(), expected.size()); nIndex++) {
				if (actual[nIndex] != expected[nIndex])
					fail("command " + std::to_string(nIndex) + " is \"" + actual[nIndex] + "\", expected \"" + expected[nIndex] + "\"; recorded: " + sActual);
			}
			if (actual.size() != expected.size())
				fail(std::to_string(actual.size()) + " commands recorded, expected " + std::to_string(expected.size()) + "; recorded: " + sActual);
		}

		static bool hasMessage(PConversionLogData pLog, const std::string& sCode)
		{
			for (size_t nIndex = 0; nIndex < pLog->getMessageCount(); nIndex++) {
				if (pLog->getMessage(nIndex).m_sCode == sCode)
					return true;
			}
			return false;
		}

		static std::string fingerprintHash(PConversionLogData pLog)
		{
			std::string sJSON = pLog->toJSON();
			size_t nSection = sJSON.find("\"fingerprint\":{");
			if (nSection == std::string::npos)
				return "";
			size_t nHash = sJSON.find("\"hash\":\"", nSection);
			return (nHash == std::string::npos) ? "" : sJSON.substr(nHash + 8, 16);
		}

		// Two separate hatches, from (0,y) to (10,y).
		static std::vector<Lib3MF4EOS::sEOSTaskPoint> twoHatches(double dY1, double dY2)
		{
			return { taskPoint(0.0, dY1), taskPoint(10.0, dY1), taskPoint(0.0, dY2), taskPoint(10.0, dY2) };
		}

		const std::vector<std::string> m_DefaultState = { "SetExposureType 2", "SetSkywriting 0", "SetLaserPower 200 190", "SetScanSpeed 1000" };

		std::vector<std::string> concat(std::initializer_list<std::vector<std::string>> parts)
		{
			std::vector<std::string> result;
			for (auto& part : parts)
				result.insert(result.end(), part.begin(), part.end());
			return result;
		}

	public:
		std::string getName() override
		{
			return "RawTaskWriter";
		}

		void registerTests() override
		{
			registerTest("BlockStateWrittenOnce", "a block writes scanner, part and its state once before its vectors", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4) });
				writer.close();

				auto lines = writer.getLines();
				assertEqualString(lines.front(), "OpenFile");
				assertEqualString(lines[1], "BeginLayer 50");
				assertEqualString(lines.back(), "CloseFile");
				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 10 0", "Hatch 0 1 10 1" } }));
				assertEqual((int64_t)writer.getLog()->getSummaryValue("hatchesWritten"), 2);
			});

			registerTest("PerScannerState", "every scanner keeps its own state, and the part is begun again after each switch", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(10, 0), taskPoint(0, 1), taskPoint(10, 1), taskPoint(0, 2), taskPoint(10, 2) };
				writer.writeLayer(50, points, {
					taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 1, 0),
					taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4, 1, 1),
					taskBlock(eEOSTaskBlockType::HatchBlock, 4, 6, 1, 0) });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({
					{ "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 10 0", "Hatch 0 2 10 2" },
					{ "SelectLaserScanner 1", "BeginPart 1" }, m_DefaultState, { "Hatch 0 1 10 1" } }));
			});

			registerTest("AscendingScannerOrder", "every scanner is selected once per layer in ascending order, and its blocks keep their order", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(10, 0), taskPoint(0, 1), taskPoint(10, 1),
					taskPoint(0, 2), taskPoint(10, 2), taskPoint(0, 3), taskPoint(10, 3) };
				writer.writeLayer(50, points, {
					taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 2, 2),
					taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4, 1, 0),
					taskBlock(eEOSTaskBlockType::HatchBlock, 4, 6, 3, 2),
					taskBlock(eEOSTaskBlockType::HatchBlock, 6, 8, 1, 1) });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({
					{ "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 1 10 1" },
					{ "SelectLaserScanner 1", "BeginPart 1" }, m_DefaultState, { "Hatch 0 3 10 3" },
					{ "SelectLaserScanner 2", "BeginPart 2" }, m_DefaultState, { "Hatch 0 0 10 0", "BeginPart 3", "Hatch 0 2 10 2" } }));
				assertEqual((int64_t)writer.countCommands("SelectLaserScanner"), 3);
			});

			registerTest("PartChange", "a part change on the same scanner only begins the new part", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), {
					taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 1),
					taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4, 2) });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 10 0", "BeginPart 2", "Hatch 0 1 10 1" } }));
			});

			registerTest("NewLayerResetsState", "each layer selects the scanner, begins the part and writes the state again", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.writeLayer(100, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.close();

				auto layer = concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 10 0" } });
				assertLines(writer.getLayerContentLines(), concat({ layer, layer }));
				assertEqual((int64_t)writer.getRecorder()->getLayerCount(), 2);
			});

			registerTest("IdleScannersBeginPart", "a scanner without work in a layer gets a part begun at its place in the scanner order, and nothing else", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.setLaserScannerCount(4);
				writer.writeLayer(50, twoHatches(0.0, 1.0), {
					taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 1, 0),
					taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4, 2, 2) });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({
					{ "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 10 0" },
					{ "SelectLaserScanner 1", "BeginPart 2" },
					{ "SelectLaserScanner 2", "BeginPart 2" }, m_DefaultState, { "Hatch 0 1 10 1" },
					{ "SelectLaserScanner 3", "BeginPart 2" } }));
				assertEqual((int64_t)writer.getLog()->getSummaryValue("idleScannerParts"), 2);
			});

			registerTest("FilledLayer", "a filled layer begins the part on every scanner and exposes nothing", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.setLaserScannerCount(2);
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4, 1, 0) });
				writer.writeEmptyLayer(100, 4);
				writer.writeEmptyLayer(150, 4);
				assertThrowsError([&]() { writer.writeEmptyLayer(150, 4); }, LIB3MF4EOS_ERROR_INVALIDPARAM);
				writer.close();

				auto lines = writer.getLines();
				auto iSecondLayer = std::find(lines.begin(), lines.end(), "BeginLayer 100");
				assertTrue(iSecondLayer != lines.end(), "the filled layer is written at its Z");
				assertLines(std::vector<std::string>(iSecondLayer, lines.end()), {
					"BeginLayer 100", "SelectLaserScanner 0", "BeginPart 4", "SelectLaserScanner 1", "BeginPart 4",
					"BeginLayer 150", "SelectLaserScanner 0", "BeginPart 4", "SelectLaserScanner 1", "BeginPart 4", "CloseFile" });
				assertEqual((int64_t)writer.getLog()->getSummaryValue("layersFilled"), 2);
				assertEqual((int64_t)writer.getRecorder()->getLayerCount(), 3);
			});

			registerTest("ScannerNotAvailable", "a block on a scanner the machine does not have is refused", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.setLaserScannerCount(2);
				assertThrowsError([&]() { writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 1, 2) }); },
					LIB3MF4EOS_ERROR_LASERSCANNERNOTAVAILABLE);
			});

			registerTest("PreDelayOnItsScanner", "a pre-delay follows the layer opening on its own scanner, before the block state, and is never joined over", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(10, 0), taskPoint(10, 0), taskPoint(20, 0), taskPoint(0, 5), taskPoint(10, 5) };
				auto padded = taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4, 1, 1);
				padded.m_PreDelay = 120500.0;
				auto paddedAfterSwitch = taskBlock(eEOSTaskBlockType::HatchBlock, 4, 6, 1, 2);
				paddedAfterSwitch.m_PreDelay = 80250.0;

				CRecordingWriter writer(2, 26, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 1, 1), padded, paddedAfterSwitch });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({
					{ "SelectLaserScanner 1", "BeginPart 1", "SetBeamShape 0 0", "Delay 10" }, m_DefaultState, { "Hatch 0 0 10 0", "Delay 120500", "Hatch 10 0 20 0" },
					{ "SelectLaserScanner 2", "BeginPart 1", "SetBeamShape 0 0", "Delay 10", "Delay 80250" }, m_DefaultState, { "Hatch 0 5 10 5" } }));
			});

			registerTest("LayerOpening226", "with SDK 2.26, every scanner opens its layer with the standard beam shape and the opening delay", [this]() {
				CRecordingWriter writer(2, 26, SConversionSettings());
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(10, 0), taskPoint(0, 1), taskPoint(10, 1), taskPoint(0, 2), taskPoint(10, 2) };
				writer.writeLayer(50, points, {
					taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 1, 0),
					taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4, 2, 0),
					taskBlock(eEOSTaskBlockType::HatchBlock, 4, 6, 1, 1) });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({
					{ "SelectLaserScanner 0", "BeginPart 1", "SetBeamShape 0 0", "Delay 10" }, m_DefaultState, { "Hatch 0 0 10 0", "BeginPart 2", "Hatch 0 1 10 1" },
					{ "SelectLaserScanner 1", "BeginPart 1", "SetBeamShape 0 0", "Delay 10" }, m_DefaultState, { "Hatch 0 2 10 2" } }));
				assertFalse(hasMessage(writer.getLog(), "SDK_FEATURE_UNAVAILABLE"));
			});

			registerTest("LayerOpening218", "with SDK 2.18, the layer opening is left out with one warning", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.writeLayer(100, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.close();

				assertEqual((int64_t)writer.countCommands("SetBeamShape"), 0);
				assertTrue(hasMessage(writer.getLog(), "SDK_FEATURE_UNAVAILABLE"));
				assertEqual((int64_t)writer.getLog()->getCountBySeverity(Lib3MF4EOS::eEOSLogSeverity::Warning), 1);
			});

			registerTest("LayerOpeningDisabled", "without layer opening, SDK 2.26 writes no beam shape for plain blocks", [this]() {
				SConversionSettings settings;
				settings.m_bLayerOpening = false;
				CRecordingWriter writer(2, 26, settings);
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.close();
				assertEqual((int64_t)writer.countCommands("SetBeamShape"), 0);
				assertEqual((int64_t)writer.countCommands("Delay"), 0);
			});

			registerTest("BeamShapeProfile", "a beam shape profile is written on 2.26, refused on 2.18 in strict mode and ignored with a warning otherwise", [this]() {
				auto block = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2);
				block.m_BeamShapeProfile = 3;

				CRecordingWriter writer226(2, 26, SConversionSettings());
				writer226.writeLayer(50, twoHatches(0.0, 1.0), { block });
				writer226.close();
				assertLines(writer226.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1", "SetBeamShape 0 0", "Delay 10", "SetBeamShape 0 3" }, m_DefaultState, { "Hatch 0 0 10 0" } }));

				CRecordingWriter strictWriter(2, 18, SConversionSettings());
				assertThrowsError([&]() { strictWriter.writeLayer(50, twoHatches(0.0, 1.0), { block }); }, LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);

				SConversionSettings lenient;
				lenient.m_bStrictMode = false;
				CRecordingWriter lenientWriter(2, 18, lenient);
				lenientWriter.writeLayer(50, twoHatches(0.0, 1.0), { block });
				lenientWriter.close();
				assertEqual((int64_t)lenientWriter.countCommands("SetBeamShape"), 0);
				assertEqual((int64_t)lenientWriter.countCommands("Hatch"), 1);
			});

			registerTest("BlockStateRecords", "delay, exposure type, skywriting, exposure step, laser pulse and depth are written in this order", [this]() {
				auto block = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4);
				block.m_PreDelay = 5.0;
				block.m_ExposureType = 5;
				block.m_SkywritingEnabled = true;
				block.m_ExposureSetIndex = 1;
				block.m_ExposureStepIndex = 2;
				block.m_PulsedWavePeriodTime = 100.0;
				block.m_PulsedWavePowerOnTime = 50.0;
				block.m_ExposedDepth = 30;
				block.m_PostDelay = 7.0;

				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), { block });
				writer.close();

				assertLines(writer.getLayerContentLines(), {
					"SelectLaserScanner 0", "BeginPart 1", "Delay 5", "SetExposureType 5", "SetSkywriting 1", "SetExposureStep 1 2", "SetLaserPulse 100 50", "SetDepth 30",
					"SetLaserPower 200 190", "SetScanSpeed 1000", "Hatch 0 0 10 0", "Delay 7", "Hatch 0 1 10 1", "Delay 7" });
			});

			registerTest("ExposureTypeChange", "a block with another exposure type writes only that change", [this]() {
				auto block2 = taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4);
				block2.m_ExposureType = 5;
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2), block2 });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 10 0", "SetExposureType 5", "Hatch 0 1 10 1" } }));
			});

			registerTest("JoinTouchingHatches", "collinear touching hatches become one polyline, also across blocks", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 0), taskPoint(10, 0) };
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2), taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4) });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "PolyStart 0 0", "PolyCoord 5 0 0", "PolyEnd 10 0" } }));
				assertEqual((int64_t)writer.getLog()->getSummaryValue("hatchesJoined"), 1);
				assertEqual((int64_t)writer.getLog()->getSummaryValue("laserBreaksAvoided"), 1);
			});

			registerTest("DwellEndsRun", "a hatch with a dwell joins the touching hatch before it, and the dwell follows the run", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 0), taskPoint(10, 0), taskPoint(10, 0), taskPoint(15, 0) };
				auto dwellBlock = taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4);
				dwellBlock.m_PostDelay = 7.0;
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2), dwellBlock, taskBlock(eEOSTaskBlockType::HatchBlock, 4, 6) });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState,
					{ "PolyStart 0 0", "PolyCoord 5 0 0", "PolyEnd 10 0", "Delay 7", "Hatch 10 0 15 0" } }));
			});

			registerTest("DwellPerHatch", "touching collinear hatches with a dwell each keep both dwells", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 0), taskPoint(10, 0) };
				auto dwellBlock = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4);
				dwellBlock.m_PostDelay = 7.0;
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { dwellBlock });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState,
					{ "Hatch 0 0 5 0", "Delay 7", "Hatch 5 0 10 0", "Delay 7" } }));
			});

			registerTest("JoinTolerance", "hatches closer than the join tolerance are joined with a short connecting vector", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5.0005, 0), taskPoint(10, 0) };

				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4) });
				writer.close();
				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "PolyStart 0 0", "PolyCoord 5 0 0", "PolyCoord 5.0005 0 0", "PolyEnd 10 0" } }));

				SConversionSettings tight;
				tight.m_dJoinTolerance = 0.0001;
				CRecordingWriter tightWriter(2, 18, tight);
				tightWriter.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4) });
				tightWriter.close();
				assertLines(tightWriter.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 5 0", "Hatch 5.0005 0 10 0" } }));
			});

			registerTest("JoinCorner", "touching hatches at an angle are joined without a connecting vector", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 0), taskPoint(5, 5) };
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4) });
				writer.close();
				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "PolyStart 0 0", "PolyCoord 5 0 0", "PolyEnd 5 5" } }));
			});

			registerTest("NoJoinAcrossStates", "hatch blocks with different states are not joined", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 0), taskPoint(10, 0) };
				auto block2 = taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4);
				block2.m_ExposureType = 5;
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2), block2 });
				writer.close();
				assertEqual((int64_t)writer.countCommands("Hatch"), 2);
				assertEqual((int64_t)writer.countCommands("PolyStart"), 0);
			});

			registerTest("SplitHatchesNeverJoin", "in SplitHatches mode, touching hatches stay single hatches", [this]() {
				SConversionSettings settings;
				settings.m_ePowerRampMode = Lib3MF4EOS::eEOSPowerRampMode::SplitHatches;
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 0), taskPoint(10, 0) };
				CRecordingWriter writer(2, 18, settings);
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4) });
				writer.close();
				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 0 5 0", "Hatch 5 0 10 0" } }));
			});

			registerTest("AllPolylines", "in AllPolylines mode, a single hatch is written as a polyline", [this]() {
				SConversionSettings settings;
				settings.m_eHatchWriteMode = Lib3MF4EOS::eEOSHatchWriteMode::AllPolylines;
				CRecordingWriter writer(2, 18, settings);
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.close();
				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "PolyStart 0 0", "PolyEnd 10 0" } }));
			});

			registerTest("PowerRampPolyline", "a linear power ramp becomes a polyline of sub-vectors with their mean power", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0, 100.0), taskPoint(10, 0, 110.0) };
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.close();

				auto lines = writer.getLayerContentLines();
				assertEqualString(lines[2], "SetExposureType 2");
				assertEqualString(lines[4], "SetLaserPower 100.5 190");
				assertEqualString(lines[6], "PolyStart 0 0");
				assertEqualString(lines[7], "PolyCoord 1 0 0");
				assertEqualString(lines[8], "SetLaserPower 101.5 190");
				assertEqualString(lines.back(), "PolyEnd 10 0");
				assertEqual((int64_t)writer.countCommands("SetLaserPower"), 10);
				assertEqual((int64_t)writer.countCommands("PolyCoord"), 9);
				assertEqual((int64_t)writer.getLog()->getSummaryValue("rampedHatches"), 1);
				assertEqual((int64_t)writer.getLog()->getSummaryValue("rampSubVectors"), 10);
			});

			registerTest("PowerDeadband", "within a polyline, power changes below the deadband are not written", [this]() {
				SConversionSettings settings;
				settings.m_dHatchPowerDeadband = 2.5;
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0, 100.0), taskPoint(10, 0, 110.0) };
				CRecordingWriter writer(2, 18, settings);
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.close();

				std::vector<std::string> powers;
				for (auto& sLine : writer.getLines()) {
					if (sLine.rfind("SetLaserPower", 0) == 0)
						powers.push_back(sLine);
				}
				assertLines(powers, { "SetLaserPower 100.5 190", "SetLaserPower 103.5 190", "SetLaserPower 106.5 190", "SetLaserPower 109.5 190" });
				assertEqual((int64_t)writer.countCommands("PolyCoord"), 9);
			});

			registerTest("PowerRampFlatten", "in Flatten mode, a ramp is written at its mean with a warning", [this]() {
				SConversionSettings settings;
				settings.m_ePowerRampMode = Lib3MF4EOS::eEOSPowerRampMode::Flatten;
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0, 100.0), taskPoint(10, 0, 110.0) };
				CRecordingWriter writer(2, 18, settings);
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				writer.close();
				assertLines(writer.getLayerContentLines(), { "SelectLaserScanner 0", "BeginPart 1", "SetExposureType 2", "SetSkywriting 0", "SetLaserPower 105 190", "SetScanSpeed 1000", "Hatch 0 0 10 0" });
				assertTrue(hasMessage(writer.getLog(), "POWER_RAMP_FLATTENED"));
			});

			registerTest("PowerRampReject", "in Reject mode, a ramp fails the layer", [this]() {
				SConversionSettings settings;
				settings.m_ePowerRampMode = Lib3MF4EOS::eEOSPowerRampMode::Reject;
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0, 100.0), taskPoint(10, 0, 110.0) };
				CRecordingWriter writer(2, 18, settings);
				assertThrowsError([&]() { writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) }); }, LIB3MF4EOS_ERROR_POWERRAMPREJECTED);
			});

			registerTest("FocusNominalByDefault", "blocks without focus are exposed fully focused and write no focus command", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4) });
				writer.close();
				assertEqual((int64_t)writer.countCommands("SetFocus"), 0);
			});

			registerTest("FocusWritten", "a focus is written once, again in the next layer, and back to 1.0 when a later block has none", [this]() {
				auto focused = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2);
				focused.m_LaserFocus = 0.5;
				focused.m_HasLaserFocus = true;
				auto focused2 = focused;
				focused2.m_StartPointIndex = 2;
				focused2.m_EndPointIndex = 4;
				auto nominal = taskBlock(eEOSTaskBlockType::HatchBlock, 2, 4);

				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, twoHatches(0.0, 1.0), { focused, focused2 });
				writer.writeLayer(100, twoHatches(0.0, 1.0), { nominal });
				writer.writeLayer(150, twoHatches(0.0, 1.0), { nominal });
				writer.close();

				assertLines(writer.getLayerContentLines(), concat({
					{ "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "SetFocus 0.5", "Hatch 0 0 10 0", "Hatch 0 1 10 1" },
					{ "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "SetFocus 1", "Hatch 0 1 10 1" },
					{ "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "Hatch 0 1 10 1" } }));
			});

			registerTest("FocusOutOfRange", "focus values outside 0 to 1 are refused", [this]() {
				for (double dFocus : { -0.1, 1.5 }) {
					auto block = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2);
					block.m_LaserFocus = dFocus;
					block.m_HasLaserFocus = true;
					CRecordingWriter writer(2, 18, SConversionSettings());
					assertThrowsError([&]() { writer.writeLayer(50, twoHatches(0.0, 1.0), { block }); }, LIB3MF4EOS_ERROR_INVALIDFOCUS);
				}
			});

			registerTest("FocusScannerCapabilities", "the focus is checked against the scanner capabilities when they are known", [this]() {
				auto block = [](double dFocus) {
					auto result = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2);
					result.m_LaserFocus = dFocus;
					result.m_HasLaserFocus = true;
					return result;
				};

				CRecordingWriter strictWriter(2, 26, SConversionSettings());
				strictWriter.setLaserScannerCapabilities({ 0 });
				assertThrowsError([&]() { strictWriter.writeLayer(50, twoHatches(0.0, 1.0), { block(0.0) }); }, LIB3MF4EOS_ERROR_FOCUSNOTSUPPORTED);

				SConversionSettings lenient;
				lenient.m_bStrictMode = false;
				CRecordingWriter lenientWriter(2, 26, lenient);
				lenientWriter.setLaserScannerCapabilities({ 0 });
				lenientWriter.writeLayer(50, twoHatches(0.0, 1.0), { block(0.0) });
				lenientWriter.writeLayer(100, twoHatches(0.0, 1.0), { block(0.0) });
				lenientWriter.close();
				assertEqual((int64_t)lenientWriter.countCommands("SetFocus"), 0);
				assertEqual((int64_t)lenientWriter.countCommands("Hatch"), 2);
				assertEqual((int64_t)lenientWriter.getLog()->getCountBySeverity(Lib3MF4EOS::eEOSLogSeverity::Warning), 1);
				assertTrue(hasMessage(lenientWriter.getLog(), "FOCUS_UNSUPPORTED"));

				CRecordingWriter binaryWriter(2, 26, SConversionSettings());
				binaryWriter.setLaserScannerCapabilities({ EOS_LSC_FOCUS_DEFOCUS });
				binaryWriter.writeLayer(50, twoHatches(0.0, 1.0), { block(0.0) });
				assertEqual((int64_t)binaryWriter.countCommands("SetFocus"), 1);
				assertThrowsError([&]() { binaryWriter.writeLayer(100, twoHatches(0.0, 1.0), { block(0.5) }); }, LIB3MF4EOS_ERROR_INVALIDFOCUS);

				CRecordingWriter variableWriter(2, 26, SConversionSettings());
				variableWriter.setLaserScannerCapabilities({ EOS_LSC_VARIABLE_FOCUS });
				variableWriter.writeLayer(50, twoHatches(0.0, 1.0), { block(0.5) });
				variableWriter.close();
				assertEqual((int64_t)variableWriter.countCommands("SetFocus"), 1);
			});

			registerTest("ContourJoin", "touching contours with the same state are written as one polyline", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 0), taskPoint(10, 0), taskPoint(10, 5) };
				std::vector<Lib3MF4EOS::sEOSTaskBlock> blocks = { taskBlock(eEOSTaskBlockType::PolylineBlock, 0, 2), taskBlock(eEOSTaskBlockType::PolylineBlock, 2, 5) };

				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, blocks);
				writer.close();
				assertLines(writer.getLayerContentLines(), concat({ { "SelectLaserScanner 0", "BeginPart 1" }, m_DefaultState, { "PolyStart 0 0", "PolyCoord 5 0 0", "PolyCoord 10 0 0", "PolyEnd 10 5" } }));
				assertEqual((int64_t)writer.getLog()->getSummaryValue("contoursJoined"), 1);

				SConversionSettings noJoin;
				noJoin.m_bJoinContours = false;
				CRecordingWriter separateWriter(2, 18, noJoin);
				separateWriter.writeLayer(50, points, blocks);
				separateWriter.close();
				assertEqual((int64_t)separateWriter.countCommands("PolyStart"), 2);
				assertEqual((int64_t)separateWriter.countCommands("PolyEnd"), 2);
			});

			registerTest("ClosedContourNotExtended", "a closed contour is not joined with the next one", [this]() {
				std::vector<Lib3MF4EOS::sEOSTaskPoint> points = { taskPoint(0, 0), taskPoint(5, 0), taskPoint(5, 5), taskPoint(0, 0), taskPoint(0, 0), taskPoint(0, 5) };
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, points, { taskBlock(eEOSTaskBlockType::PolylineBlock, 0, 4), taskBlock(eEOSTaskBlockType::PolylineBlock, 4, 6) });
				writer.close();
				assertEqual((int64_t)writer.countCommands("PolyStart"), 2);
				assertEqual((int64_t)writer.getLog()->getSummaryValue("contoursJoined"), 0);
			});

			registerTest("EmptyLayerIsRefused", "writing a layer without vectors breaks the EMPTY_LAYER rule at the next layer", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(50, {}, {});
				std::string sMessage = assertThrowsError([&]() { writer.writeLayer(100, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) }); },
					LIB3MF4EOS_ERROR_TOOLPATHORDERVIOLATION);
				assertContains(sMessage, "EMPTY_LAYER");
			});

			registerTest("LayerOrderChecked", "layer index and Z must increase", [this]() {
				CRecordingWriter writer(2, 18, SConversionSettings());
				writer.writeLayer(100, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
				assertThrowsError([&]() { writer.writeLayer(100, twoHatches(0.0, 1.0), { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) }); }, LIB3MF4EOS_ERROR_INVALIDPARAM);
			});

			registerTest("Fingerprint", "the fingerprint is stored in the log and does not depend on the run", [this]() {
				auto run = [](double dEndX) {
					auto pWriter = std::make_shared<CRecordingWriter>(2, 26, SConversionSettings());
					pWriter->writeLayer(50, { taskPoint(0, 0), taskPoint(dEndX, 0) }, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
					pWriter->writeLayer(100, { taskPoint(0, 0), taskPoint(10, 0) }, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2) });
					pWriter->close();
					return pWriter;
				};

				auto pFirst = run(10.0);
				auto pSecond = run(10.0);
				auto pChanged = run(11.0);

				std::string sHash = fingerprintHash(pFirst->getLog());
				assertEqual((int64_t)sHash.size(), 16, "hash length");
				assertEqualString(fingerprintHash(pSecond->getLog()), sHash);
				assertTrue(fingerprintHash(pChanged->getLog()) != sHash, "a changed vector changes the hash");
				assertContains(pFirst->getLog()->toJSON(), "\"layerHashes\":[{\"layerIndex\":1,\"z\":0.05");
			});
		}
	};

	PUnitTestGroup createRawTaskWriterTests()
	{
		return std::make_shared<CRawTaskWriterTests>();
	}

}
