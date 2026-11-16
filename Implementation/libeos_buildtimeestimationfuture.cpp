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


Abstract: This is a stub class definition of CBuildTimeEstimationFuture

*/

#include "libeos_buildtimeestimationfuture.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.
#define EOSBUILDTIME_MAXLAYERCOUNT 100000000

using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CBuildTimeEstimationFuture 
**************************************************************************************************************************/

void buildTimeInfoCallback(void* pUserData, const sEOSBuildTimeInfo& info)
{
    if (pUserData == nullptr)
        return;

    CBuildTimeEstimationFuture* pBuildTimeEstimation = (CBuildTimeEstimationFuture*)pUserData;
    pBuildTimeEstimation->setBuildTimeInfo(info);


} 


CBuildTimeEstimationFuture::CBuildTimeEstimationFuture(PEOSSDK pSDK)
    : m_pSDK(pSDK), m_bSimulationIsRunning (false), m_bSimulationIsAborted (false), m_nSimulatedBuildTimeInMs(0), m_nSimulatedExposureTimeInMs(0), m_nSimulatedLayerCount (1), m_nSimulationCalculationTimeInMs (3800)
{
    if (pSDK.get() == nullptr)
        throw std::runtime_error("invalid sdk parameter");

    if (!pSDK->simulateFullSDK()) {

        m_pSDK->checkEOSError(m_pSDK->getAdapter().setBuildTimeInfoCallback(buildTimeInfoCallback, this));

        EOS_ERROR error = m_pSDK->EosTaskGen_BeginBuildTimeEstimation();
        if (error != EOS_ERR_NO_ERROR)
            throw std::runtime_error("could not begin build time estimation: " + std::to_string(error));
    }
    else {
        m_StartTime = m_HighResolutionClock.now();
        m_bSimulationIsRunning = true;
        m_nSimulatedBuildTimeInMs = 1600000;
        m_nSimulatedExposureTimeInMs = 1200000;
        m_nSimulatedLayerCount = 1000;        
    }

}


CBuildTimeEstimationFuture::~CBuildTimeEstimationFuture()
{
    if (m_pSDK.get() != nullptr) {
        if (!m_pSDK->simulateFullSDK()) {
            m_pSDK->getAdapter().setBuildTimeInfoCallback(nullptr, nullptr);
        }
    }
}

bool CBuildTimeEstimationFuture::IsRunning()
{
    if (!m_pSDK->simulateFullSDK()) {
        auto state = m_pSDK->EosTaskGen_GetBuildTimeEstimationState();
        return ((state == EOS_TG_RUNNING) || (state == EOS_TG_SUSPENDED) || (state == EOS_TG_IDLE));
    }
    else {
        if (m_bSimulationIsRunning) {
            auto currentDuration = m_HighResolutionClock.now() - m_StartTime;
            m_bSimulationIsRunning = (currentDuration < std::chrono::milliseconds(m_nSimulationCalculationTimeInMs));
            return m_bSimulationIsRunning;
        }
        else {
            return false;
        }
    }
}

void CBuildTimeEstimationFuture::Abort()
{
    if (!m_pSDK->simulateFullSDK()) {
        m_pSDK->EosTaskGen_AbortBuildTimeEstimation();
    }
    else {
        m_bSimulationIsRunning = false;
        m_bSimulationIsAborted = true;
    }
}

bool CBuildTimeEstimationFuture::HasFinished()
{
    if (!m_pSDK->simulateFullSDK()) {
        auto state = m_pSDK->EosTaskGen_GetBuildTimeEstimationState();
        return (state == EOS_TG_COMPLETED);
    }
    else {
        return ((!m_bSimulationIsRunning) && (!m_bSimulationIsAborted));
    }
}

bool CBuildTimeEstimationFuture::HasFailed()
{
    if (!m_pSDK->simulateFullSDK()) {
        auto state = m_pSDK->EosTaskGen_GetBuildTimeEstimationState();
        return (state == EOS_TG_FAILED);
    }
    else {
        return false;
    }
}

bool CBuildTimeEstimationFuture::HasBeenAborted()
{
    if (!m_pSDK->simulateFullSDK()) {
        auto state = m_pSDK->EosTaskGen_GetBuildTimeEstimationState();
        return (state == EOS_TG_ABORTED);
    }
    else {
        return m_bSimulationIsAborted;
    }
}


Lib3MF4EOS_uint64 CBuildTimeEstimationFuture::GetBuildTime()
{
    if (!HasFinished ())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUILDTIMEESTIMATIONDIDNOTFINISH);

    if (!m_pSDK->simulateFullSDK()) {
        return m_pSDK->EosTaskGen_GetBuildTimeEstimation();
    }
    else {
        return m_nSimulatedBuildTimeInMs;
    }
}

void CBuildTimeEstimationFuture::GetProgress(Lib3MF4EOS_uint32 & nCurrentLayer, Lib3MF4EOS_uint32 & nNumberOfLayers, Lib3MF4EOS_uint64 & nCurrentEstimationInMs, Lib3MF4EOS_uint64 & nExposureTimeInMs, Lib3MF4EOS_uint64 & nNonExposureTimeInMs)
{
    if (!m_pSDK->simulateFullSDK()) {
        sEOSBuildTimeInfo buildTimeInfo;
        {
            std::lock_guard<std::mutex> lockGuard(m_BuildTimeInfoMutex);
            buildTimeInfo = m_BuildTimeInfo;
        }

        nCurrentLayer = 0;
        nNumberOfLayers = 0;
        nCurrentEstimationInMs = 0;
        nExposureTimeInMs = 0;
        nNonExposureTimeInMs = 0;

        if (buildTimeInfo.m_nNumberOfCalculatedLayers < EOSBUILDTIME_MAXLAYERCOUNT)
            nCurrentLayer = (uint32_t)buildTimeInfo.m_nNumberOfCalculatedLayers;

        if (buildTimeInfo.m_nNumberOfAllLayers < EOSBUILDTIME_MAXLAYERCOUNT)
            nNumberOfLayers = (uint32_t)buildTimeInfo.m_nNumberOfAllLayers;

        if (buildTimeInfo.m_dBuildTimeMs > 0.0)
            nCurrentEstimationInMs = (uint64_t)(buildTimeInfo.m_dBuildTimeMs);
        if (buildTimeInfo.m_dExposureTimeMs > 0.0)
            nExposureTimeInMs = (uint64_t)(buildTimeInfo.m_dExposureTimeMs);
        if (buildTimeInfo.m_dNonExposureTimeMs > 0.0)
            nNonExposureTimeInMs = (uint64_t)(buildTimeInfo.m_dNonExposureTimeMs);

    }
    else {
        auto currentDuration = m_HighResolutionClock.now() - m_StartTime;
        uint32_t currentDurationInMs = (uint32_t) std::chrono::duration_cast<std::chrono::milliseconds> (currentDuration).count();
        double dFactor = (double)currentDurationInMs / (double) m_nSimulationCalculationTimeInMs;
        if (dFactor >= 1.0)
            dFactor = 1.0;

        nCurrentLayer = (uint32_t) (m_nSimulatedLayerCount * dFactor);
        nNumberOfLayers = m_nSimulatedLayerCount;
        nCurrentEstimationInMs = m_nSimulatedBuildTimeInMs;
        nExposureTimeInMs = m_nSimulatedExposureTimeInMs;
        nNonExposureTimeInMs = m_nSimulatedBuildTimeInMs - m_nSimulatedExposureTimeInMs;
    }

}

void CBuildTimeEstimationFuture::setBuildTimeInfo(const sEOSBuildTimeInfo& buildTimeInfo)
{
    std::lock_guard<std::mutex> lockGuard(m_BuildTimeInfoMutex);
    m_BuildTimeInfo = buildTimeInfo;
}
