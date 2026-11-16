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


Abstract: This is a stub class definition of CEOSRawTask

*/

#include "libeos_eosrawtask.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_eosrawtasklayer.hpp"
#include "libeos_powerramp.hpp"

// Include custom headers here.
#include "../Common/common_utils.hpp"
#include "libeos_rawtasklayerreader.hpp"

using namespace Lib3MF4EOS::Impl;

CRawTaskLayerReader::CRawTaskLayerReader()
    : m_nErrorCount(0),
    m_nLastErrorCode(eRawTaskReaderErrorCode::treNoError),
    m_LayerState(eRawTaskReaderLayerState::eLayerState_Undefined),
    m_nCurrentLayerHeightInMicrometers(0),
    m_nCurrentPartID (-1),
    m_nCurrentLaserScannerIndex (0),
    m_dCurrentLaserPower (0.0),
    m_dCurrentScanSpeed (0.0),
    m_dCurrentFocus (EOS_FOCUS_NOMINAL),
    m_bHasCurrentFocus (false),
    m_bCurrentSkywritingEnabled (false),
    m_dNextSegmentPreDelayInMicroseconds (0.0),
    m_dLastTimestampInMicroseconds (0.0),
    m_nStripeCount (0),
    m_nCurrentStripeIndex (0),
	m_CurrentExposureType(EOS_EXPOSURE_TYPE::EOS_EXPOSURE_TYPE_HATCH),
    m_nCurrentSetIndex (0),
    m_nCurrentStepIndex (0),
    m_nCurrentBlockStartPointIndex (0),
    m_dPulsedWavePeriodTimeInMicroseconds (0.0),
    m_dPulsedWavePowerOnTimeInMicroseconds (0.0),
	m_nExposedDepthInMicrometers(0),
    m_BlockState (eRawTaskReaderBlockState::eBlockState_NoBlock)

{

}

CRawTaskLayerReader::~CRawTaskLayerReader()
{

}

uint32_t CRawTaskLayerReader::addPoint(double dXInMM, double dYInMM, double dLaserPowerInWatts, double dTimestampInMicroseconds) noexcept
{
    m_dLastTimestampInMicroseconds = dTimestampInMicroseconds;
    uint32_t nIndex = (uint32_t)m_Coordinates.size();
    try {
        Lib3MF4EOS::sEOSTaskPoint point;
        memset((void*)&point, 0, sizeof(point));
        point.m_X = dXInMM;
        point.m_Y = dYInMM;
        point.m_LaserPower = dLaserPowerInWatts;
        point.m_ScanSpeed = m_dCurrentScanSpeed;
        point.m_LaserFocus = m_dCurrentFocus;
        point.m_HasLaserFocus = m_bHasCurrentFocus;
        point.m_Timestamp = dTimestampInMicroseconds;
        m_Coordinates.push_back(point);

    } catch (...) {
        throwError (eRawTaskReaderErrorCode::treCouldNotAddPoint);
        return UINT32_MAX;
	}

    return nIndex;
}

void CRawTaskLayerReader::throwError(eRawTaskReaderErrorCode nErrorCode) noexcept
{
    m_nErrorCount++;
	m_nLastErrorCode = nErrorCode;
}

void CRawTaskLayerReader::startHatchBlock() noexcept
{
    if (m_BlockState == eRawTaskReaderBlockState::eBlockState_NoBlock) {
		m_BlockState = eRawTaskReaderBlockState::eLayerState_HatchBlock;
        m_nCurrentBlockStartPointIndex = (uint32_t)m_Coordinates.size();
    }
    else {
		throwError(eRawTaskReaderErrorCode::treBlockAlreadyOpen);
    }
}

void CRawTaskLayerReader::startPolylineBlock() noexcept
{
    if (m_BlockState == eRawTaskReaderBlockState::eBlockState_NoBlock) {
        m_BlockState = eRawTaskReaderBlockState::eLayerState_PolylineBlock;
		m_nCurrentBlockStartPointIndex = (uint32_t)m_Coordinates.size();
    }
    else {
        throwError(eRawTaskReaderErrorCode::treBlockAlreadyOpen);
    }

}


void CRawTaskLayerReader::finishCurrentBlock() noexcept
{
    if (m_BlockState != eRawTaskReaderBlockState::eBlockState_NoBlock) {
        // Finalize block here

		uint32_t nEndPointIndex = (uint32_t)m_Coordinates.size();

        Lib3MF4EOS::sEOSTaskBlock newBlock;
		memset((void*) & newBlock, 0, sizeof(Lib3MF4EOS::sEOSTaskBlock));

        switch (m_BlockState) {
            case eRawTaskReaderBlockState::eLayerState_HatchBlock:
                newBlock.m_Type = Lib3MF4EOS::eEOSTaskBlockType::HatchBlock;
                break;
            case eRawTaskReaderBlockState::eLayerState_PolylineBlock:
                newBlock.m_Type = Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock;
                break;
            default:
				throwError(eRawTaskReaderErrorCode::treInvalidBlockType);
                return;
        }

		newBlock.m_PartID = m_nCurrentPartID;
		newBlock.m_StartPointIndex = m_nCurrentBlockStartPointIndex;
        newBlock.m_EndPointIndex = nEndPointIndex;
		newBlock.m_LaserScannerIndex = m_nCurrentLaserScannerIndex;
		// Laser power changes, and speed or focus changes inside a polyline, do not start a new block; every point keeps its own values.
		if (m_nCurrentBlockStartPointIndex < nEndPointIndex) {
			auto& firstPoint = m_Coordinates[m_nCurrentBlockStartPointIndex];
			newBlock.m_DefaultLaserPower = firstPoint.m_LaserPower;
			newBlock.m_ScanSpeed = firstPoint.m_ScanSpeed;
			newBlock.m_LaserFocus = firstPoint.m_LaserFocus;
			newBlock.m_HasLaserFocus = firstPoint.m_HasLaserFocus;
		}
		else {
			newBlock.m_DefaultLaserPower = m_dCurrentLaserPower;
			newBlock.m_ScanSpeed = m_dCurrentScanSpeed;
			newBlock.m_LaserFocus = m_dCurrentFocus;
			newBlock.m_HasLaserFocus = m_bHasCurrentFocus;
		}
		newBlock.m_SkywritingEnabled = m_bCurrentSkywritingEnabled;
		newBlock.m_PreDelay = m_dNextSegmentPreDelayInMicroseconds;
		newBlock.m_ExposureType = (uint32_t) m_CurrentExposureType;
		newBlock.m_ExposureSetIndex = (uint32_t) m_nCurrentSetIndex;
		newBlock.m_ExposureStepIndex = (uint32_t)m_nCurrentStepIndex;
		newBlock.m_PulsedWavePeriodTime = m_dPulsedWavePeriodTimeInMicroseconds;
		newBlock.m_PulsedWavePowerOnTime = m_dPulsedWavePowerOnTimeInMicroseconds;
		newBlock.m_ExposedDepth = (uint32_t) m_nExposedDepthInMicrometers;
		newBlock.m_BeamShapeProfile = -1;
		newBlock.m_StripeIndex = m_nCurrentStripeIndex;
		m_Blocks.push_back(newBlock);
        
        m_dNextSegmentPreDelayInMicroseconds = 0.0;

	}

	m_BlockState = eRawTaskReaderBlockState::eBlockState_NoBlock;
}

uint32_t CRawTaskLayerReader::getErrorCount() noexcept
{
    return m_nErrorCount;
}

std::string CRawTaskLayerReader::getErrorMessage()
{
	return std::to_string(m_nErrorCount) + " layer read errors occurred. Last error code: " + std::to_string((uint32_t)m_nLastErrorCode);
}


uint32_t CRawTaskLayerReader::getBlockCount() noexcept
{
    return (uint32_t) m_Blocks.size ();
}

uint32_t CRawTaskLayerReader::getPointCount() noexcept
{
    return (uint32_t) m_Coordinates.size ();
}

int64_t CRawTaskLayerReader::getCurrentLayerHeightInMicrometers() noexcept
{
    return m_nCurrentLayerHeightInMicrometers;
}

void CRawTaskLayerReader::beginLayer(int64_t nMicrometers) noexcept
{
    switch (m_LayerState) {
        case eRawTaskReaderLayerState::eLayerState_Undefined:
        {
            m_LayerState = eRawTaskReaderLayerState::eLayerState_LayerOpen;
            m_nCurrentLayerHeightInMicrometers = nMicrometers;
            break;
		}

        case eRawTaskReaderLayerState::eLayerState_LayerOpen:
        {
			throwError(eRawTaskReaderErrorCode::treLayerAlreadyOpen);
            break;
        }

        case eRawTaskReaderLayerState::eLayerState_LayerClosed:
        {
            throwError(eRawTaskReaderErrorCode::treCanNotOpenLayerTwice);
            break;
        }

        default:
            throwError(eRawTaskReaderErrorCode::treInvalidLayerState);
    }

    
}

void CRawTaskLayerReader::endLayer(double nTimestampMicroseconds) noexcept
{
    finishCurrentBlock();

    switch (m_LayerState) {
    case eRawTaskReaderLayerState::eLayerState_Undefined:
    {
        throwError(eRawTaskReaderErrorCode::treNoLayerOpen);
        break;
    }

    case eRawTaskReaderLayerState::eLayerState_LayerOpen:
    {
        m_LayerState = eRawTaskReaderLayerState::eLayerState_LayerClosed;
        break;
    }

    case eRawTaskReaderLayerState::eLayerState_LayerClosed:
    {
        throwError(eRawTaskReaderErrorCode::treLayerAlreadyClosed);
        break;
    }

    default:
        throwError(eRawTaskReaderErrorCode::treInvalidLayerState);
    }

}

void CRawTaskLayerReader::beginPart(int32_t nPartID) noexcept
{
    finishCurrentBlock();
    m_nCurrentPartID = nPartID;
}

void CRawTaskLayerReader::selectLaserScannerUnit(uint8_t nLaserScannerIndex) noexcept
{
    finishCurrentBlock();
    m_nCurrentLaserScannerIndex = nLaserScannerIndex;
    auto iFocus = m_ScannerFocus.find(m_nCurrentLaserScannerIndex);
    m_bHasCurrentFocus = (iFocus != m_ScannerFocus.end());
    m_dCurrentFocus = m_bHasCurrentFocus ? iFocus->second : EOS_FOCUS_NOMINAL;
}

void CRawTaskLayerReader::setLaserPower(double dPower, double dDelayInMicroseconds) noexcept
{
    // The delay belongs to the power change and is written again from the conversion settings, so it is not a pre-delay.
    m_dCurrentLaserPower = dPower;
}

void CRawTaskLayerReader::setScanSpeed(double dSpeedInMillimetersPerSecond) noexcept
{
    // Inside a polyline, the change applies to the next vector and the polyline continues.
    if (m_BlockState != eRawTaskReaderBlockState::eLayerState_PolylineBlock)
        finishCurrentBlock();
	m_dCurrentScanSpeed = dSpeedInMillimetersPerSecond;
}

void CRawTaskLayerReader::setFocus(double dFocus) noexcept
{
    if (m_BlockState != eRawTaskReaderBlockState::eLayerState_PolylineBlock)
        finishCurrentBlock();
    m_dCurrentFocus = dFocus;
    m_bHasCurrentFocus = true;
    m_ScannerFocus[m_nCurrentLaserScannerIndex] = dFocus;
}

void CRawTaskLayerReader::setSkywriting(int32_t bSkywritingEnabled) noexcept
{
    finishCurrentBlock();
	m_bCurrentSkywritingEnabled = (bSkywritingEnabled != 0);
}

void CRawTaskLayerReader::customData(const std::wstring& sCustomData) noexcept
{	
}

void CRawTaskLayerReader::hatch(double nTimestampMicroseconds, const EOS_POINT2& start, const EOS_POINT2& end) noexcept
{
    if (m_BlockState != eRawTaskReaderBlockState::eLayerState_HatchBlock) {
        finishCurrentBlock();
        startHatchBlock();
	}

	addPoint(start.m_dX, start.m_dY, m_dCurrentLaserPower, nTimestampMicroseconds);
	addPoint(end.m_dX, end.m_dY, m_dCurrentLaserPower, nTimestampMicroseconds);
}

void CRawTaskLayerReader::polyStart(double nTimestampMicroseconds, const EOS_POINT2& startCoord) noexcept
{
    // Every polyline is a block of its own, so that consecutive polylines are not joined.
    finishCurrentBlock();
    startPolylineBlock();

    addPoint(startCoord.m_dX, startCoord.m_dY, m_dCurrentLaserPower, nTimestampMicroseconds);
}

void CRawTaskLayerReader::polyCoord(double nTimestampMicroseconds, const EOS_POINT2& intermediateCoord) noexcept
{
    if (m_BlockState != eRawTaskReaderBlockState::eLayerState_PolylineBlock) {
		throwError(eRawTaskReaderErrorCode::trePolylineNotStarted);
    }
    else {
        addPoint(intermediateCoord.m_dX, intermediateCoord.m_dY, m_dCurrentLaserPower, nTimestampMicroseconds);
    }
}

void CRawTaskLayerReader::polyEnd(const EOS_POINT2& endCoord) noexcept
{
    if (m_BlockState != eRawTaskReaderBlockState::eLayerState_PolylineBlock) {
        throwError(eRawTaskReaderErrorCode::trePolylineNotStarted);
    }
    else {
        addPoint(endCoord.m_dX, endCoord.m_dY, m_dCurrentLaserPower, m_dLastTimestampInMicroseconds);
    }
}

void CRawTaskLayerReader::delay(double nTimestampMicroseconds, double dDelayInMicroseconds) noexcept
{
    if (dDelayInMicroseconds > 0.0) {
        finishCurrentBlock();
		m_dNextSegmentPreDelayInMicroseconds += dDelayInMicroseconds;
	}

}

void CRawTaskLayerReader::setExposureType(EOS_EXPOSURE_TYPE exposureType) noexcept
{
    finishCurrentBlock();
    m_CurrentExposureType = exposureType;
}

void CRawTaskLayerReader::setExposureStep(uint32_t nExposureSetIndex, uint32_t nExposureStepIndex) noexcept
{
    finishCurrentBlock();
	m_nCurrentSetIndex = nExposureSetIndex;
    m_nCurrentStepIndex = nExposureStepIndex;
}

void CRawTaskLayerReader::setDepth(uint64_t nExposedDepthInMicrometers) noexcept
{
    finishCurrentBlock();
	m_nExposedDepthInMicrometers = nExposedDepthInMicrometers;
}

void CRawTaskLayerReader::startStripeSegment(const EOS_POINT2& stripeBegin, const EOS_POINT2& hatchDistVector) noexcept
{
    finishCurrentBlock();
    m_nStripeCount++;
    m_nCurrentStripeIndex = m_nStripeCount;
}

void CRawTaskLayerReader::startGeneralSegment(const EOS_POINT2& hatchDistVector) noexcept
{
    finishCurrentBlock();
    m_nCurrentStripeIndex = 0;
}

void CRawTaskLayerReader::endSegment() noexcept
{
    finishCurrentBlock();
    m_nCurrentStripeIndex = 0;
}

void CRawTaskLayerReader::endLine() noexcept
{	
}

CEOSRawTaskLayer* CRawTaskLayerReader::createRawTaskLayer()
{
	std::unique_ptr<CEOSRawTaskLayer> pNewLayer(new CEOSRawTaskLayer (m_nCurrentLayerHeightInMicrometers));
	pNewLayer->initializeData(m_Coordinates, m_Blocks);
	return pNewLayer.release();

}


void CRawTaskLayerReader::setLaserPulse(double dPulsedWavePeriodTimeInMicroseconds, double dPulsedWavePowerOnTimeInMicroseconds) noexcept
{
    finishCurrentBlock();
    m_dPulsedWavePeriodTimeInMicroseconds = dPulsedWavePeriodTimeInMicroseconds;
    m_dPulsedWavePowerOnTimeInMicroseconds = dPulsedWavePowerOnTimeInMicroseconds;
}

void eosRawTask_BeginLayerCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROMETER z_um)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->beginLayer(z_um);
}

void eosRawTask_EndLayerCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
	pRawTaskReader->endLayer(timestamp_us);
           
}

void eosRawTask_BeginPartCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_INT partId)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->beginPart(partId);
}

void eosRawTask_SelectLaserScannerUnitCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_BYTE laserScannerIndex)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->selectLaserScannerUnit(laserScannerIndex);
}

void eosRawTask_SetLaserPowerCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE power, EOS_MICROSECS delay_us)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setLaserPower(power, delay_us);
}

void eosRawTask_SetScanSpeedCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE speed_mm_per_s)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setScanSpeed(speed_mm_per_s);
}

void eosRawTask_SetFocusCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE focus)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setFocus(focus);
}

void eosRawTask_SetSkywritingCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_INT skywritingEnabled)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setSkywriting(skywritingEnabled);
}

void eosRawTask_CustomDataCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, const EOS_CWSTR customData)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->customData(std::wstring(customData));
}

void eosRawTask_HatchCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 start, EOS_POINT2 end)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->hatch(timestamp_us, start, end);
}

void eosRawTask_PolyStartCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 startCoord)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->polyStart(timestamp_us, startCoord);
}

void eosRawTask_PolyCoordCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 intermediateCoord)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->polyCoord(timestamp_us, intermediateCoord);
}

void eosRawTask_PolyEndCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 endCoord)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->polyEnd(endCoord);
}

void eosRawTask_DelayCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_MICROSECS delay_us)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->delay(timestamp_us, delay_us);
}

void eosRawTask_SetExposureTypeCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_EXPOSURE_TYPE exposureType)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setExposureType(exposureType);
}

void eosRawTask_SetExposureStepCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_UINT exposureSetIndex, EOS_UINT exposureStepIndex)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setExposureStep(exposureSetIndex, exposureStepIndex);
}

void eosRawTask_SetDepthCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROMETER exposedDepth_um)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setDepth(exposedDepth_um);
}

void eosRawTask_StartStripeSegmentCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 stripeBegin, EOS_POINT2 hatchDistVector)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->startStripeSegment(stripeBegin, hatchDistVector);
}

void eosRawTask_StartGeneralSegmentCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 hatchDistVector)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->startGeneralSegment(hatchDistVector);
}

void eosRawTask_EndSegmentCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->endSegment();
}

void eosRawTask_EndLineCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle)
{
    if (customReadDataHandle == 0) 
        return;   
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->endLine();
}

void eosRawTask_SetLaserPulseCallback(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS wavePeriod_us, EOS_MICROSECS powerOnTime_us)
{
    if (customReadDataHandle == 0)
        return;
    CRawTaskLayerReader* pRawTaskReader = (CRawTaskLayerReader*)((void*)customReadDataHandle);
    pRawTaskReader->setLaserPulse(wavePeriod_us, powerOnTime_us);
}


/*************************************************************************************************************************
 Class definition of CEOSRawTask 
**************************************************************************************************************************/

CEOSRawTask::CEOSRawTask(PEOSSDK pSDK, AMCCommon::PWorkingDirectory pTaskDirectory, const std::string& sAbsoluteTaskFileName)
: m_hTaskFile (0)
{
    if (pSDK.get () == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
    
    m_pSDK = pSDK;
	m_pTaskDirectory = pTaskDirectory;
	m_sAbsoluteTaskFileName = sAbsoluteTaskFileName;

    if (m_pSDK->EosToolpath_OpenInputFile == nullptr)
        throw  ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);
    if (m_pSDK->EosToolpath_CloseFile == nullptr)
        throw  ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);    
    if (m_pSDK->EosToolpath_GetNumberOfLayers == nullptr)
        throw  ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);

    std::wstring sAbsoluteTaskFileNameW = AMCCommon::CUtils::UTF8toUTF16(sAbsoluteTaskFileName);

    m_pSDK->checkEOSError (m_pSDK->EosToolpath_OpenInputFile(sAbsoluteTaskFileNameW.c_str(), &m_hTaskFile));
    if (m_hTaskFile == 0)
        throw  ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTOPENTASKFILE, "could not open task file: " + sAbsoluteTaskFileName);

}

CEOSRawTask::~CEOSRawTask()
{
    try {
        Release();
    }
    catch (...) {
        // ignore errors in destructor
    }
}


std::string CEOSRawTask::GetAbsoluteFilename()
{
    return m_sAbsoluteTaskFileName;
}

std::string CEOSRawTask::GetTaskDirectory()
{
    return m_pTaskDirectory->getPath();
}

void CEOSRawTask::queryLayerRange(uint64_t& nLayerCount, uint64_t& nFirstLayerIndex)
{
    if (m_pSDK.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

    if (m_hTaskFile == 0)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKHASBEENRELEASED);

    if (m_pSDK->EosToolpath_GetNumberOfLayers == nullptr)
        throw  ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);

    nLayerCount = 0;
    nFirstLayerIndex = 0;
    m_pSDK->checkEOSError (m_pSDK->EosToolpath_GetNumberOfLayers(m_hTaskFile, &nLayerCount, &nFirstLayerIndex));
}

Lib3MF4EOS_uint32 CEOSRawTask::GetNumberOfLayers()
{
    uint64_t nLayerCount = 0;
    uint64_t nFirstLayerIndex = 0;
    queryLayerRange(nLayerCount, nFirstLayerIndex);
    return (uint32_t)nLayerCount;
}

Lib3MF4EOS_uint32 CEOSRawTask::GetFirstLayerIndex()
{
    uint64_t nLayerCount = 0;
    uint64_t nFirstLayerIndex = 0;
    queryLayerRange(nLayerCount, nFirstLayerIndex);
    return (uint32_t)nFirstLayerIndex;
}

void CEOSRawTask::Release()
{
    if (m_pSDK.get() != nullptr) {
        if (m_hTaskFile != 0) {

            if (m_pSDK->EosToolpath_CloseFile == nullptr)
                throw  ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);

            m_pSDK->checkEOSError (m_pSDK->EosToolpath_CloseFile(m_hTaskFile));

            m_hTaskFile = 0;
        }
    }
}

// Older SDK versions do not export every callback setter; optional ones are skipped when missing.
template <typename TSetter, typename TCallback>
static void registerRawTaskCallback(CEOSSDK* pSDK, TSetter pSetter, TCallback pCallback, bool bRequired, const std::string& sSetterName)
{
    if (pSetter == nullptr) {
        if (bRequired)
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED, "EOS SDK does not provide " + sSetterName);
        return;
    }
    pSDK->checkEOSError(pSetter(pCallback, 0));
}

IEOSRawTaskLayer* CEOSRawTask::ReadLayer(const Lib3MF4EOS_uint32 nLayerIndex)
{
    if (m_pSDK.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);
    if (m_hTaskFile == 0)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKHASBEENRELEASED);
    if (m_pSDK->EosToolpath_ReadLayer == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED, "EOS SDK does not provide EosToolpath_ReadLayer");

	std::shared_ptr<CRawTaskLayerReader> pRawTaskReader = std::make_shared<CRawTaskLayerReader>();
    EOS_DATA_HANDLE pDataHandle = (EOS_DATA_HANDLE) ((void*) pRawTaskReader.get());

    CEOSSDK* pSDK = m_pSDK.get();
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetBeginLayerCallback, eosRawTask_BeginLayerCallback, true, "EosToolpath_SetBeginLayerCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetEndLayerCallback, eosRawTask_EndLayerCallback, true, "EosToolpath_SetEndLayerCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetBeginPartCallback, eosRawTask_BeginPartCallback, true, "EosToolpath_SetBeginPartCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSelectLaserScannerCallback, eosRawTask_SelectLaserScannerUnitCallback, true, "EosToolpath_SetSelectLaserScannerCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetLaserPowerCallback, eosRawTask_SetLaserPowerCallback, true, "EosToolpath_SetSetLaserPowerCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetScanSpeedCallback, eosRawTask_SetScanSpeedCallback, true, "EosToolpath_SetSetScanSpeedCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetFocusCallback, eosRawTask_SetFocusCallback, true, "EosToolpath_SetSetFocusCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetSkywritingCallback, eosRawTask_SetSkywritingCallback, true, "EosToolpath_SetSetSkywritingCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetCustomDataCallback, eosRawTask_CustomDataCallback, false, "EosToolpath_SetCustomDataCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetHatchCallback, eosRawTask_HatchCallback, true, "EosToolpath_SetHatchCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetPolyStartCallback, eosRawTask_PolyStartCallback, true, "EosToolpath_SetPolyStartCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetPolyCoordCallback, eosRawTask_PolyCoordCallback, true, "EosToolpath_SetPolyCoordCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetPolyEndCallback, eosRawTask_PolyEndCallback, true, "EosToolpath_SetPolyEndCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetDelayCallback, eosRawTask_DelayCallback, false, "EosToolpath_SetDelayCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetExposureTypeCallback, eosRawTask_SetExposureTypeCallback, true, "EosToolpath_SetSetExposureTypeCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetExposureStepCallback, eosRawTask_SetExposureStepCallback, false, "EosToolpath_SetSetExposureStepCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetDepthCallback, eosRawTask_SetDepthCallback, false, "EosToolpath_SetSetDepthCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetStartStripeSegmentCallback, eosRawTask_StartStripeSegmentCallback, false, "EosToolpath_SetStartStripeSegmentCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetStartGeneralSegmentCallback, eosRawTask_StartGeneralSegmentCallback, false, "EosToolpath_SetStartGeneralSegmentCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetEndSegmentCallback, eosRawTask_EndSegmentCallback, false, "EosToolpath_SetEndSegmentCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetEndLineCallback, eosRawTask_EndLineCallback, false, "EosToolpath_SetEndLineCallback");
    registerRawTaskCallback(pSDK, pSDK->EosToolpath_SetSetLaserPulseCallback, eosRawTask_SetLaserPulseCallback, false, "EosToolpath_SetSetLaserPulseCallback");

	m_pSDK->checkEOSError(m_pSDK->EosToolpath_ReadLayer(m_hTaskFile, pDataHandle, EOS_TOOLPATH_READING_ORDER::EOS_TRO_BY_LASER_THEN_BY_TIME_INDEX, nLayerIndex));

    pRawTaskReader->finishCurrentBlock();

    if (pRawTaskReader->getErrorCount() > 0) {
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_ERRORSDURINGTASKREADING, pRawTaskReader->getErrorMessage ());
	}

    return pRawTaskReader->createRawTaskLayer();



}
