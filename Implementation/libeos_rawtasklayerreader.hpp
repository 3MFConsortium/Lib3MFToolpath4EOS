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


Abstract: Collects the callbacks of EosToolpath_ReadLayer into the points and blocks of a raw task layer.

*/


#ifndef __LIBEOS_RAWTASKLAYERREADER
#define __LIBEOS_RAWTASKLAYERREADER

#include "lib3mftoolpath4eos_types.hpp"
#include "libeos_eossdk.hpp"
#include "libeos_eosrawtasklayer.hpp"

#include <deque>
#include <map>
#include <string>

namespace Lib3MF4EOS {
namespace Impl {

enum class eRawTaskReaderLayerState {
    eLayerState_Undefined,
    eLayerState_LayerOpen,
    eLayerState_LayerClosed
};

enum class eRawTaskReaderBlockState {
    eBlockState_NoBlock,
    eLayerState_HatchBlock,
    eLayerState_PolylineBlock
};

enum class eRawTaskReaderErrorCode {
    treNoError = 0,
	treCouldNotAddPoint = 1,
	treLayerAlreadyOpen = 2,
	treNoLayerOpen = 3,
	treLayerAlreadyClosed = 4,
    treCanNotOpenLayerTwice = 5,
	treInvalidLayerState = 6,
    treBlockAlreadyOpen = 7,
    trePolylineNotStarted = 8,
    treInvalidBlockType = 9
};

class CRawTaskLayerReader
{
private:
    std::deque<Lib3MF4EOS::sEOSTaskPoint> m_Coordinates;
	std::deque<Lib3MF4EOS::sEOSTaskBlock> m_Blocks;

    int64_t m_nCurrentLayerHeightInMicrometers;
    uint32_t m_nErrorCount;
    eRawTaskReaderErrorCode m_nLastErrorCode;

    eRawTaskReaderLayerState m_LayerState;

    int32_t m_nCurrentPartID;
    uint32_t m_nCurrentLaserScannerIndex;
	double m_dCurrentLaserPower;
    double m_dCurrentScanSpeed;
	double m_dCurrentFocus;
	// Whether a focus was set on the selected scanner. Without one, the scanner is fully focused.
	bool m_bHasCurrentFocus;
	// The focus applies to the scanner it was set on.
	std::map<uint32_t, double> m_ScannerFocus;
    bool m_bCurrentSkywritingEnabled;
	double m_dNextSegmentPreDelayInMicroseconds;
	// The timestamp of the last point, which the end point of a polyline repeats.
	double m_dLastTimestampInMicroseconds;
	// Stripe segments are numbered from 1 within the layer; 0 outside a stripe segment.
	uint32_t m_nStripeCount;
	uint32_t m_nCurrentStripeIndex;

	EOS_EXPOSURE_TYPE m_CurrentExposureType;
	uint64_t m_nCurrentSetIndex;
	uint64_t m_nCurrentStepIndex;

    double m_dPulsedWavePeriodTimeInMicroseconds;
    double m_dPulsedWavePowerOnTimeInMicroseconds;
    uint64_t m_nExposedDepthInMicrometers;

    uint32_t m_nCurrentBlockStartPointIndex;
	eRawTaskReaderBlockState m_BlockState;

public:

    CRawTaskLayerReader();

    virtual ~CRawTaskLayerReader();

    uint32_t addPoint(double dXInMM, double dYInMM, double dLaserPowerInWatts, double dTimestampInMicroseconds) noexcept;

    uint32_t getErrorCount() noexcept;

    uint32_t getBlockCount() noexcept;

    uint32_t getPointCount() noexcept;

    std::string getErrorMessage();

    int64_t getCurrentLayerHeightInMicrometers() noexcept;

	void throwError(eRawTaskReaderErrorCode nErrorCode) noexcept;

    void startHatchBlock() noexcept;

    void startPolylineBlock() noexcept;

    void finishCurrentBlock() noexcept;

    void beginLayer (int64_t nMicrometers) noexcept;

	void endLayer(double nTimestampMicroseconds) noexcept;

    void beginPart(int32_t nPartID) noexcept;

	void selectLaserScannerUnit(uint8_t nLaserScannerIndex) noexcept;

    void setLaserPower(double dPower, double dDelayInMicroseconds) noexcept;

	void setScanSpeed(double dSpeedInMillimetersPerSecond) noexcept;

	void setFocus(double dFocus) noexcept;

	void setSkywriting(int32_t bSkywritingEnabled) noexcept;

	void customData(const std::wstring& sCustomData) noexcept;

	void hatch(double nTimestampMicroseconds, const EOS_POINT2& start, const EOS_POINT2& end) noexcept;

	void polyStart(double nTimestampMicroseconds, const EOS_POINT2& startCoord) noexcept;

	void polyCoord(double nTimestampMicroseconds, const EOS_POINT2& intermediateCoord) noexcept;

	void polyEnd(const EOS_POINT2& endCoord) noexcept;

	void delay(double nTimestampMicroseconds, double nDelayInMicroseconds) noexcept;

	void setExposureType(EOS_EXPOSURE_TYPE exposureType) noexcept;

	void setExposureStep(uint32_t nExposureSetIndex, uint32_t nExposureStepIndex) noexcept;

	void setDepth(uint64_t nExposedDepthInMicrometers) noexcept;

	void startStripeSegment(const EOS_POINT2& stripeBegin, const EOS_POINT2& hatchDistVector) noexcept;

	void startGeneralSegment(const EOS_POINT2& hatchDistVector) noexcept;

	void endSegment() noexcept;

	void endLine() noexcept;

	void setLaserPulse(double nPulsedWavePeriodTimeInMicroseconds, double nPulsedWavePowerOnTimeInMicroseconds) noexcept;

	CEOSRawTaskLayer* createRawTaskLayer();

};

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_RAWTASKLAYERREADER
