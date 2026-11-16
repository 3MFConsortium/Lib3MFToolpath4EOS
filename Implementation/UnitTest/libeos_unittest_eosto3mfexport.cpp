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


Abstract: Tests of what the EOS to 3MF export carries over from the EOS task: laser sources, the Z of the first layer,
pre-delays, EOS timestamps and stripe numbers. The raw layers are written to a 3MF with lib3mf and read back.

*/

#include "libeos_unittests.hpp"
#include "libeos_unittest_helpers.hpp"
#include "libeos_eosto3mfconverter.hpp"
#include "libeos_rawtasklayerreader.hpp"
#include "libeos_namespaceregistry.hpp"
#include "libeos_toolpathprofileparameters.hpp"

#include <deque>
#include <memory>

using namespace Lib3MF4EOS::Impl;
using Lib3MF4EOS::eEOSTaskBlockType;

namespace LibEOSUnitTest {

	struct SRawTaskLayerDeleter {
		void operator()(CEOSRawTaskLayer* pLayer) const
		{
			if (pLayer != nullptr)
				pLayer->DecRefCount();
		}
	};
	typedef std::unique_ptr<CEOSRawTaskLayer, SRawTaskLayerDeleter> PRawTaskLayer;

	static const std::string EOS_TOOLPATH_NAMESPACE = LIBEOS_NAMESPACE_TOOLPATHPROFILE;

	class CEOSTo3MFExportTests : public CUnitTestGroup {
	private:
		static EOS_POINT2 point2(double dX, double dY)
		{
			EOS_POINT2 point;
			point.m_dX = dX;
			point.m_dY = dY;
			return point;
		}

		static Lib3MF4EOS::sEOSTaskPoint stampedPoint(double dX, double dY, double dTimestamp)
		{
			auto point = taskPoint(dX, dY, 0.0, 0.0);
			point.m_Timestamp = dTimestamp;
			return point;
		}

		static PRawTaskLayer createLayer(int64_t nZInMicrometers, const std::deque<Lib3MF4EOS::sEOSTaskPoint>& points, const std::deque<Lib3MF4EOS::sEOSTaskBlock>& blocks)
		{
			PRawTaskLayer pLayer(new CEOSRawTaskLayer(nZInMicrometers));
			pLayer->initializeData(points, blocks);
			return pLayer;
		}

		static std::vector<Lib3MF4EOS::sEOSTaskPoint> layerPoints(CEOSRawTaskLayer* pLayer)
		{
			Lib3MF4EOS_uint64 nCount = 0;
			pLayer->GetPointData(0, &nCount, nullptr);
			std::vector<Lib3MF4EOS::sEOSTaskPoint> points((size_t)nCount);
			pLayer->GetPointData(nCount, &nCount, points.data());
			return points;
		}

		static std::vector<Lib3MF4EOS::sEOSTaskBlock> layerBlocks(CEOSRawTaskLayer* pLayer)
		{
			Lib3MF4EOS_uint64 nCount = 0;
			pLayer->GetBlockData(0, &nCount, nullptr);
			std::vector<Lib3MF4EOS::sEOSTaskBlock> blocks((size_t)nCount);
			pLayer->GetBlockData(nCount, &nCount, blocks.data());
			return blocks;
		}

	public:
		std::string getName() override
		{
			return "EOSTo3MFExport";
		}

		void registerTests() override
		{
			registerTest("BottomZ", "the toolpath starts one layer below the first EOS layer", [this]() {
				assertNear(CEOSTo3MFConverter::computeBottomZInMM(1, 3, 40, 80), 0.0, 0.0, "a task from the first layer starts at 0");
				assertNear(CEOSTo3MFConverter::computeBottomZInMM(50, 3, 2000, 2040), 1.96, 1e-12, "thickness from the first two layers");
				assertNear(CEOSTo3MFConverter::computeBottomZInMM(50, 1, 2000, 0), 1.96, 1e-12, "a single layer divides its Z by its index");
				assertNear(CEOSTo3MFConverter::computeBottomZInMM(5, 2, 200, 200), 0.0, 0.0, "no thickness gives 0");
				assertNear(CEOSTo3MFConverter::computeBottomZInMM(5, 0, 0, 0), 0.0, 0.0, "no layers give 0");
			});

			registerTest("TimestampText", "timestamps are written as the shortest text that reads back as the same value", [this]() {
				assertEqualString(CEOSTo3MFConverter::formatTimestamp(1234.5), "1234.5");
				assertEqualString(CEOSTo3MFConverter::formatTimestamp(0.0), "0");
				assertEqualString(CEOSTo3MFConverter::formatTimestamp(98765432.0), "98765432");
				double dValue = 1.0 / 3.0;
				assertTrue(std::stod(CEOSTo3MFConverter::formatTimestamp(dValue)) == dValue, "1/3 reads back exactly");
			});

			registerTest("ReaderTimestampsAndStripes", "the layer reader keeps the EOS timestamps of the points and numbers the stripes of the layer", [this]() {
				CRawTaskLayerReader reader;
				reader.beginLayer(2000);
				reader.beginPart(1);
				reader.selectLaserScannerUnit(0);
				reader.setLaserPower(200.0, 0.0);
				reader.setScanSpeed(1000.0);

				reader.startStripeSegment(point2(0, 0), point2(0, 0.1));
				reader.hatch(100.0, point2(0, 0), point2(10, 0));
				reader.hatch(110.0, point2(0, 0.1), point2(10, 0.1));
				reader.endSegment();
				reader.startStripeSegment(point2(0, 5), point2(0, 0.1));
				reader.hatch(200.0, point2(0, 5), point2(10, 5));
				reader.endSegment();

				reader.startGeneralSegment(point2(0, 0));
				reader.selectLaserScannerUnit(1);
				reader.polyStart(300.0, point2(0, 0));
				reader.polyCoord(310.0, point2(5, 0));
				reader.polyEnd(point2(5, 5));
				reader.endSegment();

				reader.delay(400.0, 25.0);
				reader.hatch(500.0, point2(0, 8), point2(10, 8));
				reader.endLayer(600.0);

				assertEqual(reader.getErrorCount(), 0, reader.getErrorMessage());
				PRawTaskLayer pLayer(reader.createRawTaskLayer());
				auto points = layerPoints(pLayer.get());
				auto blocks = layerBlocks(pLayer.get());

				std::vector<double> expectedTimestamps = { 100, 100, 110, 110, 200, 200, 300, 310, 310, 500, 500 };
				assertEqual((int64_t)points.size(), (int64_t)expectedTimestamps.size());
				for (size_t nIndex = 0; nIndex < points.size(); nIndex++)
					assertNear(points[nIndex].m_Timestamp, expectedTimestamps[nIndex], 0.0, "timestamp of point " + std::to_string(nIndex) + "; a polyline end repeats the previous one");

				assertEqual((int64_t)blocks.size(), 4);
				assertEqual(blocks[0].m_StripeIndex, 1, "first stripe");
				assertEqual(blocks[0].m_EndPointIndex, 4);
				assertEqual(blocks[1].m_StripeIndex, 2, "second stripe");
				assertEqual(blocks[2].m_StripeIndex, 0, "a general segment is no stripe");
				assertEqual((int64_t)blocks[2].m_Type, (int64_t)eEOSTaskBlockType::PolylineBlock);
				assertEqual(blocks[2].m_LaserScannerIndex, 1);
				assertEqual(blocks[3].m_StripeIndex, 0, "outside any segment");
				assertNear(blocks[3].m_PreDelay, 25.0, 0.0, "the delay goes to the next block");
				assertNear(blocks[0].m_PreDelay, 0.0, 0.0);
			});

			registerTest("RoundTrip", "laser sources, bottom Z, pre-delays, timestamps and stripes read back from the written 3MF", [this]() {
				auto pWrapper = Lib3MF::CWrapper::loadLibrary(LIBEOS_UNITTEST_LIB3MF_LIBRARY);
				auto pModel = pWrapper->CreateModel();
				auto pWriter = pModel->QueryWriter("3mf");
				pWriter->RegisterCustomNamespace("eos", EOS_TOOLPATH_NAMESPACE);

				{
					CEOSTo3MFConverter converter(pWrapper, pModel, pWriter);
					converter.setBottomZ(1.96);

					std::deque<Lib3MF4EOS::sEOSTaskPoint> points1 = {
						stampedPoint(0, 0, 100.5), stampedPoint(10, 0, 100.5), stampedPoint(0, 1, 110.0), stampedPoint(10, 1, 110.0),
						stampedPoint(0, 0, 300.0), stampedPoint(5, 0, 310.0), stampedPoint(5, 5, 310.0) };
					auto hatchBlock = taskBlock(eEOSTaskBlockType::HatchBlock, 0, 4, 1, 0);
					hatchBlock.m_StripeIndex = 3;
					auto polylineBlock = taskBlock(eEOSTaskBlockType::PolylineBlock, 4, 7, 1, 1);
					polylineBlock.m_PreDelay = 25.0;
					auto pLayer1 = createLayer(2000, points1, { hatchBlock, polylineBlock });
					converter.convertLayer(pLayer1.get());

					bool bRefused = false;
					try {
						converter.setBottomZ(0.0);
					}
					catch (std::exception&) {
						bRefused = true;
					}
					assertTrue(bRefused, "the bottom Z cannot change after the first layer");

					std::deque<Lib3MF4EOS::sEOSTaskPoint> points2 = { stampedPoint(0, 0, 1000.0), stampedPoint(10, 0, 1000.0) };
					auto pLayer2 = createLayer(2040, points2, { taskBlock(eEOSTaskBlockType::HatchBlock, 0, 2, 1, 2) });
					converter.convertLayer(pLayer2.get());

					converter.finish();
				}

				std::vector<Lib3MF_uint8> buffer;
				pWriter->WriteToBuffer(buffer);

				auto pReadModel = pWrapper->CreateModel();
				auto pSource = pReadModel->CreatePersistentSourceFromBuffer(buffer);
				pReadModel->QueryReader("3mf")->ReadFromPersistentSource(pSource.get());

				auto pToolpath = pReadModel->GetBuildToolpath();
				assertTrue(pToolpath.get() != nullptr, "the build toolpath is written");
				assertEqual(pToolpath->GetBottomZ(), 1960, "bottom Z in toolpath units of 1 um");
				assertEqual(pToolpath->GetLayerCount(), 2);

				assertEqual(pToolpath->GetLaserSourceCount(), 3, "scanners 0 to 2 are declared, the last one after the first layer");
				for (uint32_t nIndex = 0; nIndex < 3; nIndex++)
					assertEqual(pToolpath->GetLaserSource(nIndex)->GetLaserIndex(), nIndex, "laser sources keep the scanner order");

				pToolpath->RegisterCustomDoubleSegmentAttribute(EOS_TOOLPATH_NAMESPACE, "timestamp");
				pToolpath->RegisterCustomIntegerSegmentAttribute(EOS_TOOLPATH_NAMESPACE, "stripe");

				auto pLayer1 = pToolpath->ReadLayerData(0);
				assertEqual(pLayer1->GetSegmentCount(), 2);
				assertNear(pLayer1->GetSegmentDoubleAttributeByName(0, EOS_TOOLPATH_NAMESPACE, "timestamp"), 100.5, 0.0, "time of the first hatch");
				assertEqual(pLayer1->GetSegmentIntegerAttributeByName(0, EOS_TOOLPATH_NAMESPACE, "stripe"), 3);
				assertNear(pLayer1->GetSegmentDoubleAttributeByName(1, EOS_TOOLPATH_NAMESPACE, "timestamp"), 300.0, 0.0, "time of the polyline start");
				assertEqual(pLayer1->GetSegmentIntegerAttributeByName(1, EOS_TOOLPATH_NAMESPACE, "stripe"), 0);

				auto pHatchProfile = pLayer1->GetSegmentDefaultProfile(0);
				assertFalse(pHatchProfile->HasParameterValue(EOS_TOOLPATH_NAMESPACE, "predelay"), "no pre-delay without a delay");
				assertEqual(pHatchProfile->GetParameterIntegerValue("", "laserindex"), 0);
				auto pPolylineProfile = pLayer1->GetSegmentDefaultProfile(1);
				assertNear(pPolylineProfile->GetParameterDoubleValue(EOS_TOOLPATH_NAMESPACE, "predelay"), 25.0, 0.0, "pre-delay in us");
				assertEqual(pPolylineProfile->GetParameterIntegerValue("", "laserindex"), 1);
				assertEqual((int64_t)findUnsupportedProfileParameters(pHatchProfile, { EOS_TOOLPATH_NAMESPACE }).size(), 0, "the exported hatch profile converts back");
				assertEqual((int64_t)findUnsupportedProfileParameters(pPolylineProfile, { EOS_TOOLPATH_NAMESPACE }).size(), 0, "the exported polyline profile converts back");

				auto pLayer2 = pToolpath->ReadLayerData(1);
				assertNear(pLayer2->GetSegmentDoubleAttributeByName(0, EOS_TOOLPATH_NAMESPACE, "timestamp"), 1000.0, 0.0);
				assertEqual(pLayer2->GetSegmentDefaultProfile(0)->GetParameterIntegerValue("", "laserindex"), 2);
			});
		}
	};

	PUnitTestGroup createEOSTo3MFExportTests()
	{
		return std::make_shared<CEOSTo3MFExportTests>();
	}

}
