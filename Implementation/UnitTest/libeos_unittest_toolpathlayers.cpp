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


Abstract: Tests of the layer Z layout, the layer thickness per part and the layer stack check.

*/

#include "libeos_unittests.hpp"
#include "libeos_toolpathlayers.hpp"

using namespace Lib3MF4EOS::Impl;
using Lib3MF4EOS::eEOSLayerZConvention;

namespace LibEOSUnitTest {

	class CToolpathLayersTests : public CUnitTestGroup {
	private:
		static SToolpathLayerScan makeScan(const std::vector<int64_t>& layerZ, const std::vector<std::vector<uint32_t>>& layerParts)
		{
			SToolpathLayerScan scan;
			scan.m_LayerZ = layerZ;
			scan.m_LayerParts = layerParts;
			return scan;
		}

		static bool hasMessage(PConversionLogData pLog, const std::string& sCode, Lib3MF4EOS::eEOSLogSeverity eSeverity)
		{
			for (size_t nIndex = 0; nIndex < pLog->getMessageCount(); nIndex++) {
				auto message = pLog->getMessage(nIndex);
				if ((message.m_sCode == sCode) && (message.m_eSeverity == eSeverity))
					return true;
			}
			return false;
		}

		// A stack with exposed layers of part 1 at the given Z values.
		static SEOSLayerStack makeStack(const std::vector<int32_t>& layerZ, double dTotalZTravel, double dMaxZTravel)
		{
			SEOSLayerStack stack;
			int32_t nPreviousZ = 0;
			for (auto nZ : layerZ) {
				sEOSLayerInfo info;
				info.m_nLayerIndex = stack.m_Layers.size() + 1;
				info.m_nDeltaInMicrometers = nZ - nPreviousZ;
				info.m_nZPositionInMicrometers = nZ;
				info.m_bIsExposed = true;
				info.m_ExposedPartIDs = { 1 };
				stack.m_Layers.push_back(info);
				nPreviousZ = nZ;
			}
			stack.m_ProcessHeights.m_dTotalZTravel = dTotalZTravel;
			stack.m_ProcessHeights.m_dMaxZTravel = dMaxZTravel;
			return stack;
		}

	public:
		std::string getName() override
		{
			return "ToolpathLayers";
		}

		void registerTests() override
		{
			registerTest("TopConvention", "with Top, toolpath Z values are the EOS layer Z values", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				auto layout = computeToolpathZLayout(makeScan({ 40, 80, 120 }, { { 1 }, { 1 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1 }, pLog);
				assertEqual(layout.m_nStep, 40);
				assertEqual(layout.m_nOffset, 0);
				assertEqual((int64_t)layout.m_EOSLayerZ.size(), 3);
				assertEqual(layout.m_EOSLayerZ[0], 40);
				assertEqual(layout.getPartLayerThickness(1), 40);
				assertEqual(layout.getPartLayerThickness(99), 0);
				assertTrue(hasMessage(pLog, "LAYER_Z_AMBIGUOUS", Lib3MF4EOS::eEOSLogSeverity::Info));
			});

			registerTest("BaseConvention", "with Base, the layer thickness is added to every toolpath Z", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				auto layout = computeToolpathZLayout(makeScan({ 0, 40, 80 }, { { 1 }, { 1 }, { 1 } }), eEOSLayerZConvention::Base, 0, { 1 }, pLog);
				assertEqual(layout.m_nOffset, 40);
				assertEqual(layout.m_EOSLayerZ[0], 40);
				assertEqual(layout.m_EOSLayerZ[2], 120);
				assertEqual(layout.m_Parts.at(1).m_nFirstZ, 40);
				assertEqual(layout.m_Parts.at(1).m_nLastZ, 120);
			});

			registerTest("TopNeedsPositiveFirstLayer", "with Top, a first layer at Z 0 is refused", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				assertThrowsError([&]() { computeToolpathZLayout(makeScan({ 0, 40 }, { { 1 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1 }, pLog); }, LIB3MF4EOS_ERROR_LAYERZINVALID);
				assertTrue(pLog->hasErrors());
			});

			registerTest("IncreasingZ", "layer Z values must increase", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				assertThrowsError([&]() { computeToolpathZLayout(makeScan({ 40, 80, 80 }, { { 1 }, { 1 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1 }, pLog); }, LIB3MF4EOS_ERROR_LAYERZINVALID);
			});

			registerTest("PartThicknesses", "every part gets the distance of its own layers", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				// Part 1 every 40 um, part 2 every 80 um, part 3 in a single layer.
				auto layout = computeToolpathZLayout(makeScan({ 40, 80, 120, 160 }, { { 1, 2 }, { 1 }, { 1, 2, 3 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1, 2, 3, 4 }, pLog);
				assertEqual(layout.getPartLayerThickness(1), 40);
				assertEqual(layout.getPartLayerThickness(2), 80);
				assertEqual(layout.getPartLayerThickness(3), 40, "single layer uses the toolpath step");
				assertTrue(layout.m_Parts.at(3).m_eThicknessSource == eLayerThicknessSource::ToolpathStep);
				assertEqual(layout.m_Parts.at(4).m_nLayerCount, 0);
				assertTrue(hasMessage(pLog, "PART_WITHOUT_TOOLPATH", Lib3MF4EOS::eEOSLogSeverity::Warning));
			});

			registerTest("InconsistentThickness", "a part whose layer distances are no multiples of one thickness is refused", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				assertThrowsError([&]() { computeToolpathZLayout(makeScan({ 40, 80, 140 }, { { 1 }, { 1 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1 }, pLog); }, LIB3MF4EOS_ERROR_LAYERTHICKNESSINCONSISTENT);
			});

			registerTest("RoundingTolerance", "layer distances may differ from the thickness by one micrometer per layer", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				auto layout = computeToolpathZLayout(makeScan({ 30, 60, 91, 121 }, { { 1 }, { 1 }, { 1 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1 }, pLog);
				assertEqual(layout.getPartLayerThickness(1), 30);

				assertThrowsError([&]() { computeToolpathZLayout(makeScan({ 30, 60, 92, 122 }, { { 1 }, { 1 }, { 1 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1 }, pLog); },
					LIB3MF4EOS_ERROR_LAYERTHICKNESSINCONSISTENT, "two micrometers off");
			});

			registerTest("BaseNeedsCommonThickness", "with Base, parts with different thicknesses are refused", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				assertThrowsError([&]() { computeToolpathZLayout(makeScan({ 0, 40, 80 }, { { 1, 2 }, { 1 }, { 1, 2 } }), eEOSLayerZConvention::Base, 0, { 1, 2 }, pLog); }, LIB3MF4EOS_ERROR_LAYERZINVALID);
			});

			registerTest("SingleLayerFallback", "a single toolpath layer uses the fallback thickness", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				auto layout = computeToolpathZLayout(makeScan({ 0 }, { { 1 } }), eEOSLayerZConvention::Base, 60, { 1 }, pLog);
				assertEqual(layout.m_nStep, 60);
				assertEqual(layout.m_EOSLayerZ[0], 60);

				assertThrowsError([&]() { computeToolpathZLayout(makeScan({ 0 }, { { 1 } }), eEOSLayerZConvention::Base, 0, { 1 }, pLog); }, LIB3MF4EOS_ERROR_LAYERZINVALID);
			});

			registerTest("CoverageComplete", "a stack with all toolpath layers is reported as complete", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				auto scan = makeScan({ 40, 80, 120 }, { { 1 }, { 1 }, { 1 } });
				auto layout = computeToolpathZLayout(scan, eEOSLayerZConvention::Top, 0, { 1 }, pLog);
				checkLayerCoverage(scan, layout, makeStack({ 40, 80, 120 }, 0.12, 400.0), true, true, pLog);
				assertTrue(hasMessage(pLog, "LAYER_COVERAGE_COMPLETE", Lib3MF4EOS::eEOSLogSeverity::Info));
				assertEqual((int64_t)pLog->getSummaryValue("stackLayers"), 3);
				assertEqual((int64_t)pLog->getSummaryValue("layersNotInStack"), 0);
				assertContains(pLog->toJSON(), "\"layerStack\":{\"simulated\":false,\"layers\":3,\"firstLayerZ\":0.04,\"lastLayerZ\":0.12");
			});

			registerTest("StackMismatch", "toolpath layers missing in the stack are an error only in strict mode", [this]() {
				auto scan = makeScan({ 40, 85, 130 }, { { 1 }, { 1 }, { 1 } });
				auto stack = makeStack({ 40, 80, 120 }, 0.12, 400.0);

				auto pStrictLog = std::make_shared<CConversionLogData>();
				auto layout = computeToolpathZLayout(scan, eEOSLayerZConvention::Top, 0, { 1 }, pStrictLog);
				assertThrowsError([&]() { checkLayerCoverage(scan, layout, stack, true, true, pStrictLog); }, LIB3MF4EOS_ERROR_LAYERSTACKMISMATCH);
				assertTrue(hasMessage(pStrictLog, "LAYER_NOT_IN_STACK", Lib3MF4EOS::eEOSLogSeverity::Error));
				assertEqual((int64_t)pStrictLog->getSummaryValue("layersNotInStack"), 2);

				auto pLenientLog = std::make_shared<CConversionLogData>();
				checkLayerCoverage(scan, layout, stack, false, true, pLenientLog);
				assertTrue(hasMessage(pLenientLog, "LAYER_NOT_IN_STACK", Lib3MF4EOS::eEOSLogSeverity::Warning));
				assertFalse(pLenientLog->hasErrors());
				assertTrue(hasMessage(pLenientLog, "PART_LAYERS_WITHOUT_TOOLPATH", Lib3MF4EOS::eEOSLogSeverity::Warning));
			});

			registerTest("ProcessHeightExceeded", "a Z travel above the machine maximum is an error", [this]() {
				auto pLog = std::make_shared<CConversionLogData>();
				auto scan = makeScan({ 40 }, { { 1 } });
				auto layout = computeToolpathZLayout(scan, eEOSLayerZConvention::Top, 40, { 1 }, pLog);
				assertThrowsError([&]() { checkLayerCoverage(scan, layout, makeStack({ 40 }, 500.0, 400.0), false, true, pLog); }, LIB3MF4EOS_ERROR_PROCESSHEIGHTEXCEEDED);
				assertTrue(hasMessage(pLog, "PROCESS_HEIGHT_EXCEEDED", Lib3MF4EOS::eEOSLogSeverity::Error));
			});

			registerTest("ShrinkageCompensation", "a Z shrinkage compensation of the job is refused in strict mode and a warning otherwise", [this]() {
				auto scan = makeScan({ 40, 80 }, { { 1 }, { 1 } });
				auto stack = makeStack({ 40, 80 }, 0.2, 400.0);
				stack.m_ProcessHeights.m_dAccumulatedShrinkageCompensation = 0.12;

				auto pStrictLog = std::make_shared<CConversionLogData>();
				auto layout = computeToolpathZLayout(scan, eEOSLayerZConvention::Top, 0, { 1 }, pStrictLog);
				assertThrowsError([&]() { checkLayerCoverage(scan, layout, stack, true, true, pStrictLog); }, LIB3MF4EOS_ERROR_SHRINKAGECOMPENSATIONNOTSUPPORTED);
				assertTrue(hasMessage(pStrictLog, "SHRINKAGE_COMPENSATION_REFUSED", Lib3MF4EOS::eEOSLogSeverity::Error));

				auto pLenientLog = std::make_shared<CConversionLogData>();
				assertEqual((int64_t)checkLayerCoverage(scan, layout, stack, false, true, pLenientLog), 0);
				assertTrue(hasMessage(pLenientLog, "SHRINKAGE_COMPENSATION_IGNORED", Lib3MF4EOS::eEOSLogSeverity::Warning));
				assertFalse(pLenientLog->hasErrors());

				stack.m_ProcessHeights.m_dAccumulatedShrinkageCompensation = 0.0;
				auto pPlainLog = std::make_shared<CConversionLogData>();
				checkLayerCoverage(scan, layout, stack, true, true, pPlainLog);
				assertFalse(hasMessage(pPlainLog, "SHRINKAGE_COMPENSATION_IGNORED", Lib3MF4EOS::eEOSLogSeverity::Warning));
			});

			registerTest("StackLayersToFill", "the cursor finds the exposed stack layers that the written toolpath layers leave out", [this]() {
				auto stack = makeStack({ 40, 80, 120, 160, 200, 240 }, 0.24, 400.0);
				stack.m_Layers[3].m_bIsExposed = false;
				stack.m_Layers[3].m_ExposedPartIDs.clear();
				stack.m_Layers[1].m_ExposedPartIDs = { 5, 1 };

				CStackLayerCursor cursor(stack);
				assertEqual((int64_t)cursor.passLayersUpTo(40).size(), 0, "the first layer has toolpath");

				auto layers = cursor.passLayersUpTo(201);
				assertEqual((int64_t)layers.size(), 2, "80 and 120 have no toolpath, 160 has no part, 200 is within the tolerance");
				assertEqual(layers[0].m_nZPositionInMicrometers, 80);
				assertEqual(layers[0].m_ExposedPartIDs.front(), 5);
				assertEqual(layers[1].m_nZPositionInMicrometers, 120);

				layers = cursor.passRemainingLayers();
				assertEqual((int64_t)layers.size(), 1);
				assertEqual(layers[0].m_nZPositionInMicrometers, 240);
				assertEqual((int64_t)cursor.passRemainingLayers().size(), 0);

				SEOSLayerStack emptyStack;
				CStackLayerCursor emptyCursor(emptyStack);
				assertEqual((int64_t)emptyCursor.passLayersUpTo(1000).size(), 0, "without a stack nothing is filled");
			});

			registerTest("SimulatedStack", "the simulated stack has the layers of every part that overlap its mesh", [this]() {
				SEOS3MFPart part;
				part.partId = 1;
				part.vertices = { Lib3MF::sPosition{ { 0.0f, 0.0f, 0.0f } }, Lib3MF::sPosition{ { 1.0f, 1.0f, 0.2f } } };

				auto pLog = std::make_shared<CConversionLogData>();
				auto layout = computeToolpathZLayout(makeScan({ 40, 80 }, { { 1 }, { 1 } }), eEOSLayerZConvention::Top, 0, { 1 }, pLog);
				auto stack = simulateLayerStack({ part }, layout, 400.0);
				assertTrue(stack.m_bSimulated);
				assertEqual((int64_t)stack.m_Layers.size(), 5);
				assertEqual(stack.m_Layers.front().m_nZPositionInMicrometers, 40);
				assertEqual(stack.m_Layers.back().m_nZPositionInMicrometers, 200);
				assertNear(stack.m_ProcessHeights.m_dTotalZTravel, 0.2, 1e-12);
			});
		}
	};

	PUnitTestGroup createToolpathLayersTests()
	{
		return std::make_shared<CToolpathLayersTests>();
	}

}
