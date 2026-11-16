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


Abstract: This is a stub class definition of CMachineSessionFuture

*/

#include "libeos_machinesessionfuture.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.
#include "libeos_machineconfigfuture.hpp"
#include "libeos_taskuploadfuture.hpp"
#include "libeos_taskcreationfuture.hpp"

#include "../Common/common_utils.hpp"

#include <algorithm>
#include <filesystem>

using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CMachineSessionFuture 
**************************************************************************************************************************/
CMachineSessionFuture::CMachineSessionFuture(PEOSSDK pEOSSDK, const std::string sNetworkAddress)
    : m_sNetworkAddress(sNetworkAddress), m_pEOSSDK(pEOSSDK), m_SessionState (eFutureState::Unknown), m_pAbortFlag(std::make_shared<std::atomic<bool>>(false))
{
    if (pEOSSDK.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

    if (sNetworkAddress.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EMPTYNETWORKADDRESS);

    m_pCertificateStoreDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_certstore");
    m_pTempStoreDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_tempstore");

    auto pCertificateStoreDirectory = m_pCertificateStoreDirectory;
    auto pTempStoreDirectory = m_pTempStoreDirectory;
    auto pAbortFlag = m_pAbortFlag;

    std::promise<PMachineSessionHandle> connectionPromise;
    m_pSessionHandleFuture = connectionPromise.get_future();

    m_SessionState = eFutureState::Running;

    // EosMachineSession_Create cannot be cancelled. After an abort the thread runs on and closes the session it gets.
    std::thread connectionThread([pEOSSDK, sNetworkAddress, pCertificateStoreDirectory, pTempStoreDirectory, pAbortFlag](std::promise<PMachineSessionHandle>&& promise) {
        try {

            EOS_SESSION_HANDLE nSessionHandle = 0;

            if (!pEOSSDK->simulateConnectivity()) {

                auto sNetworkAddressW = AMCCommon::CUtils::UTF8toUTF16(sNetworkAddress);
                pEOSSDK->checkEOSError(pEOSSDK->EosMachineSession_Create(sNetworkAddressW.c_str(), pCertificateStoreDirectory->getPathW().c_str(), pTempStoreDirectory->getPathW().c_str(), &nSessionHandle));

                if (nSessionHandle == 0)
                    throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_RETRIEVEDINVALIDSESSIONHANDLE);
            }
            else
            {
                for (uint32_t nStep = 0; (nStep < 20) && !pAbortFlag->load(); nStep++)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            auto pSessionHandle = std::make_shared<CMachineSessionHandle>(pEOSSDK, nSessionHandle);
            if (pAbortFlag->load())
                pSessionHandle = nullptr;

            promise.set_value(pSessionHandle);

        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }

    }, std::move (connectionPromise));

    connectionThread.detach();

}

CMachineSessionFuture::~CMachineSessionFuture()
{
    Disconnect();
}


void CMachineSessionFuture::Disconnect()
{
    m_pAbortFlag->store(true);
    m_pSessionHandleFuture = std::future<PMachineSessionHandle>();
    m_pSessionHandle = nullptr;
}

bool CMachineSessionFuture::IsConnected()
{
    updateSessionState();
    return m_pSessionHandle.get () != nullptr;

}

std::string CMachineSessionFuture::GetNetworkAddress()
{
    return m_sNetworkAddress;
}

IMachineConfigFuture * CMachineSessionFuture::DownloadMachineConfig()
{
    if (!IsConnected())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MACHINESESSIONNOTCONNECTED);

    auto pMachineConfig = std::make_unique<CMachineConfigFuture>(m_pEOSSDK);
    pMachineConfig->loadFromEOSSession(m_pSessionHandle);

    return pMachineConfig.release();
}

ITaskUploadFuture * CMachineSessionFuture::UploadTask(ITaskCreationFuture* pTaskCreation)
{
    if (!IsConnected())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MACHINESESSIONNOTCONNECTED);

    if (pTaskCreation == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

    auto pTaskCreationInstance = dynamic_cast<CTaskCreationFuture*> (pTaskCreation);
    if (pTaskCreationInstance == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

    auto pTaskUpload = std::make_unique<CTaskUploadFuture>(m_pSessionHandle);

    if (!m_pEOSSDK->simulateConnectivity()) {

        auto dataFiles = pTaskCreationInstance->getDataFilesToUpload();
        for (auto dataFile : dataFiles) {
            pTaskUpload->addUpload(dataFile);
        }

        auto taskFile = pTaskCreationInstance->getTaskFileToUpload();
        pTaskUpload->addUpload(taskFile);

        pTaskUpload->startUploading();

    }
    else {
        pTaskUpload->simulateUploading();
    }

    return pTaskUpload.release();
}

ITaskUploadFuture * CMachineSessionFuture::UploadTaskFile(const std::string & sTaskFileName)
{
    if (!IsConnected())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MACHINESESSIONNOTCONNECTED);

    if (sTaskFileName.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "empty task file name");

    std::filesystem::path taskFilePath = std::filesystem::u8path(AMCCommon::CUtils::getFullPathName(sTaskFileName, false));
    if (!std::filesystem::is_regular_file(taskFilePath))
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSTASKFILE_NOTEXISTING, "task file not existing: " + sTaskFileName);

    auto pTaskUpload = std::make_unique<CTaskUploadFuture>(m_pSessionHandle);

    if (!m_pEOSSDK->simulateConnectivity()) {

        std::vector<std::string> dataFiles;
        for (auto& entry : std::filesystem::directory_iterator(taskFilePath.parent_path())) {
            if (entry.is_regular_file() && (entry.path().extension() == ".data"))
                dataFiles.push_back(entry.path().u8string());
        }
        std::sort(dataFiles.begin(), dataFiles.end());

        for (auto& sDataFile : dataFiles)
            pTaskUpload->addUpload(sDataFile);

        pTaskUpload->addUpload(taskFilePath.u8string());

        pTaskUpload->startUploading();
    }
    else {
        pTaskUpload->simulateUploading();
    }

    return pTaskUpload.release();
}

bool CMachineSessionFuture::IsRunning()
{
    updateSessionState();
    return m_SessionState == eFutureState::Running;
}


void CMachineSessionFuture::Abort()
{
    updateSessionState();
    if (m_SessionState != eFutureState::Running)
        return;

    m_pAbortFlag->store(true);
    m_pSessionHandleFuture = std::future<PMachineSessionHandle>();
    m_SessionState = eFutureState::Aborted;
}

bool CMachineSessionFuture::HasFinished()
{
    return IsConnected ();
}

bool CMachineSessionFuture::HasFailed()
{
    updateSessionState();
    return (m_SessionState == eFutureState::ExceptionOccured);
}

bool CMachineSessionFuture::HasBeenAborted()
{
    updateSessionState();
    return (m_SessionState == eFutureState::Aborted);
}

void CMachineSessionFuture::updateSessionState()
{

    if (m_pSessionHandle.get() == nullptr) {

        if (m_pSessionHandleFuture.valid()) {
            
            if (m_pSessionHandleFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                try {
                    m_pSessionHandle = m_pSessionHandleFuture.get();
                    m_SessionState = eFutureState::FinishedSuccessfully;
                }
                catch (...) {
                    setErrorMessageFromException(std::current_exception());
                    m_SessionState = eFutureState::ExceptionOccured;
                }
            }
        }
    }
    
}
