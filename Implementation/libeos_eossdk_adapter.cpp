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


Abstract: Abstract EOS SDK version adapter and the registry of known SDK versions.

*/

#include "libeos_eossdk_adapter.hpp"
#include "SDK/libeos_eossdk_adapter_2_18.hpp"
#ifdef LIBEOS_SDK_2_25
#include "SDK/libeos_eossdk_adapter_2_25.hpp"
#endif
#include "SDK/libeos_eossdk_adapter_2_26.hpp"

using namespace Lib3MF4EOS::Impl;


std::string sEOSSDKVersion::toString() const
{
    return std::to_string(m_nMajor) + "." + std::to_string(m_nMinor) + "." + std::to_string(m_nBuild) + "." + std::to_string(m_nRevision);
}

std::string sEOSSDKVersion::toShortString() const
{
    return std::to_string(m_nMajor) + "." + std::to_string(m_nMinor);
}

std::string Lib3MF4EOS::Impl::eosModelTypeToIdentifier(EOS_MODEL_TYPE modelType)
{
    switch (modelType) {
    case EOS_MT_M290: return "M290";
    case EOS_MT_M400: return "M400";
    case EOS_MT_M404: return "M404";
    case EOS_MT_P500: return "P500";
    case EOS_MT_M304: return "M304";
    case EOS_MT_P900: return "P900";
    case EOS_MT_P396: return "P396";
    case EOS_MT_P420: return "P420";
    case EOS_MT_P711: return "P711";
    case EOS_MT_M100: return "M100";
    case EOS_MT_P120: return "P120";
    case EOS_MT_M292: return "M292";
    case EOS_MT_P520: return "P520";
    case EOS_MT_M293: return "M293";
    case EOS_MT_P110: return "P110";
    case EOS_MT_M294: return "M294";
    case EOS_MT_M314: return "M314";
    case EOS_MT_P397: return "P397";
    case EOS_MT_M808: return "M808";
    case EOS_MT_P501: return "P501";
    case EOS_MT_P521: return "P521";
    case EOS_MT_M450: return "M450";
    case EOS_MT_M451: return "M451";
    case EOS_MT_M405: return "M405";
    default: return "";
    }
}

std::string Lib3MF4EOS::Impl::eosModelTypeToMachineTarget(EOS_MODEL_TYPE modelType)
{
    // OpenJob files and EOSPRINT identify the M 290 as M291.
    if (modelType == EOS_MT_M290)
        return "M291";

    return eosModelTypeToIdentifier(modelType);
}


/*************************************************************************************************************************
 CEOSSDKAdapter
**************************************************************************************************************************/

CEOSSDKAdapter::CEOSSDKAdapter()
    : m_pBuildTimeInfoCallback(nullptr), m_pBuildTimeInfoUserData(nullptr), m_pEventCallback(nullptr), m_pEventUserData(nullptr)
{
}

CEOSSDKAdapter::~CEOSSDKAdapter()
{
}

std::string CEOSSDKAdapter::getVersionString() const
{
    return std::to_string(getVersionMajor()) + "." + std::to_string(getVersionMinor());
}

void CEOSSDKAdapter::storeBuildTimeInfoCallback(PEOSBuildTimeInfoCallback pCallback, void* pUserData)
{
    std::lock_guard<std::mutex> lockGuard(m_CallbackMutex);
    m_pBuildTimeInfoCallback = pCallback;
    m_pBuildTimeInfoUserData = pUserData;
}

void CEOSSDKAdapter::storeEventCallback(PEOSEventCallback pCallback, void* pUserData)
{
    std::lock_guard<std::mutex> lockGuard(m_CallbackMutex);
    m_pEventCallback = pCallback;
    m_pEventUserData = pUserData;
}

void CEOSSDKAdapter::dispatchBuildTimeInfo(const sEOSBuildTimeInfo& buildTimeInfo)
{
    PEOSBuildTimeInfoCallback pCallback;
    void* pUserData;
    {
        std::lock_guard<std::mutex> lockGuard(m_CallbackMutex);
        pCallback = m_pBuildTimeInfoCallback;
        pUserData = m_pBuildTimeInfoUserData;
    }

    if (pCallback != nullptr)
        pCallback(pUserData, buildTimeInfo);
}

void CEOSSDKAdapter::dispatchEvent(const sEOSEvent& event)
{
    PEOSEventCallback pCallback;
    void* pUserData;
    {
        std::lock_guard<std::mutex> lockGuard(m_CallbackMutex);
        pCallback = m_pEventCallback;
        pUserData = m_pEventUserData;
    }

    if (pCallback != nullptr)
        pCallback(pUserData, event);
}


/*************************************************************************************************************************
 CEOSSDKAdapterRegistry
**************************************************************************************************************************/

const std::vector<sEOSSDKAdapterRegistration>& CEOSSDKAdapterRegistry::getRegistrations()
{
    static const std::vector<sEOSSDKAdapterRegistration> registrations = {
        { SDK_2_18::VERSION_MAJOR, SDK_2_18::VERSION_MINOR, []() -> PEOSSDKAdapter { return std::make_shared<CEOSSDKAdapter_2_18>(); } },
#ifdef LIBEOS_SDK_2_25
        { SDK_2_25::VERSION_MAJOR, SDK_2_25::VERSION_MINOR, []() -> PEOSSDKAdapter { return std::make_shared<CEOSSDKAdapter_2_25>(); } },
#endif
        { SDK_2_26::VERSION_MAJOR, SDK_2_26::VERSION_MINOR, []() -> PEOSSDKAdapter { return std::make_shared<CEOSSDKAdapter_2_26>(); } },
    };

    return registrations;
}

static bool versionIsLower(uint32_t nMajorA, uint32_t nMinorA, uint32_t nMajorB, uint32_t nMinorB)
{
    if (nMajorA != nMajorB)
        return nMajorA < nMajorB;
    return nMinorA < nMinorB;
}

PEOSSDKAdapter CEOSSDKAdapterRegistry::createAdapter(uint32_t nMajor, uint32_t nMinor, std::vector<std::string>& messages)
{
    auto& registrations = getRegistrations();

    const sEOSSDKAdapterRegistration* pBestMatch = nullptr;
    for (auto& registration : registrations) {
        if ((registration.m_nMajor == nMajor) && (registration.m_nMinor == nMinor))
            return registration.m_Factory();

        if (versionIsLower(registration.m_nMajor, registration.m_nMinor, nMajor, nMinor))
            pBestMatch = &registration;
    }

    if (pBestMatch == nullptr)
        return nullptr;

    messages.push_back("EOS SDK version " + std::to_string(nMajor) + "." + std::to_string(nMinor) + " is not known to LibEOS; using the structures of version "
        + std::to_string(pBestMatch->m_nMajor) + "." + std::to_string(pBestMatch->m_nMinor) + ".");

    return pBestMatch->m_Factory();
}

PEOSSDKAdapter CEOSSDKAdapterRegistry::createLowestAdapter()
{
    return getRegistrations().front().m_Factory();
}

bool CEOSSDKAdapterRegistry::isKnownVersion(uint32_t nMajor, uint32_t nMinor)
{
    for (auto& registration : getRegistrations()) {
        if ((registration.m_nMajor == nMajor) && (registration.m_nMinor == nMinor))
            return true;
    }
    return false;
}

std::string CEOSSDKAdapterRegistry::getSupportedVersionsString()
{
    std::string sResult;
    for (auto& registration : getRegistrations()) {
        if (!sResult.empty())
            sResult += ",";
        sResult += std::to_string(registration.m_nMajor) + "." + std::to_string(registration.m_nMinor);
    }
    return sResult;
}
