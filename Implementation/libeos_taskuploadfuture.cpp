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


Abstract: This is a stub class definition of CTaskUploadFuture

*/

#include "libeos_taskuploadfuture.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.
#include "../Common/common_utils.hpp"

using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CTaskUploadFuture 
**************************************************************************************************************************/

CTaskUploadFuture::CTaskUploadFuture(PMachineSessionHandle pSessionHandle)
    : m_pSessionHandle(pSessionHandle), m_pState(std::make_shared<sTaskUploadState>()), m_TaskUploadState (eFutureState::Unknown)
{
    if (pSessionHandle.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
}

CTaskUploadFuture::~CTaskUploadFuture()
{
    try {
        updateTaskUploadState();
        if (m_TaskUploadState == eFutureState::Running)
            requestAbort();
    }
    catch (...) {
    }
}

bool CTaskUploadFuture::IsRunning()
{
    updateTaskUploadState();
    return m_TaskUploadState == eFutureState::Running;
}

void CTaskUploadFuture::Abort()
{
    updateTaskUploadState();
    if (m_TaskUploadState != eFutureState::Running)
        return;

    requestAbort();
    m_TaskUploadFinished = std::future<bool>();
    m_TaskUploadState = eFutureState::Aborted;
}

void CTaskUploadFuture::requestAbort()
{
    std::lock_guard<std::mutex> lockGuard(m_pState->m_Mutex);
    m_pState->m_bAbort.store(true);
    if (m_pState->m_hCurrentTransfer != 0)
        m_pSessionHandle->getEOSSDK()->EosMachineSession_AbortFileTransfer(m_pState->m_hCurrentTransfer);
}

bool CTaskUploadFuture::HasFinished()
{
    updateTaskUploadState();
    return m_TaskUploadState == eFutureState::FinishedSuccessfully;
}

bool CTaskUploadFuture::HasFailed()
{
    updateTaskUploadState();
    return m_TaskUploadState == eFutureState::ExceptionOccured;
}

bool CTaskUploadFuture::HasBeenAborted()
{
    updateTaskUploadState();
    return m_TaskUploadState == eFutureState::Aborted;
}

Lib3MF4EOS_double CTaskUploadFuture::GetProgress()
{
    std::lock_guard<std::mutex> lockGuard(m_pState->m_Mutex);
    return m_pState->m_dProgress;
}

void sTaskUploadState::setProgress(double dProgress)
{
    std::lock_guard<std::mutex> lockGuard(m_Mutex);
    m_dProgress = dProgress;
}



void CTaskUploadFuture::addUpload(const std::string& sFileName)
{
    m_filesToUpload.push_back(sFileName);
}

void CTaskUploadFuture::updateTaskUploadState()
{
    if (m_TaskUploadFinished.valid()) {

        if (m_TaskUploadFinished.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            try {
                if (m_TaskUploadFinished.get())
                    m_TaskUploadState = eFutureState::FinishedSuccessfully;
                else
                    m_TaskUploadState = eFutureState::Aborted;
            }
            catch (...) {
                setErrorMessageFromException(std::current_exception());
                m_TaskUploadState = eFutureState::ExceptionOccured;
            }
        }
    }
}

void CTaskUploadFuture::simulateUploading()
{
    if (m_TaskUploadState != eFutureState::Unknown)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_ALREADYSTARTEDUPLOADING);

    m_TaskUploadState = eFutureState::Running;

    std::promise<bool> taskUploadPromise;
    m_TaskUploadFinished = taskUploadPromise.get_future();

    auto pState = m_pState;
    std::thread taskUploadThread([pState](std::promise<bool>&& promise) {
        try {
            for (uint32_t nIndex = 0; (nIndex < 100) && !pState->m_bAbort.load(); nIndex++) {
                std::this_thread::sleep_for(std::chrono::milliseconds (100));
                pState->setProgress(nIndex * 0.01);
            }

            promise.set_value(!pState->m_bAbort.load());
        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }

    }, std::move(taskUploadPromise));

    taskUploadThread.detach();
}


void CTaskUploadFuture::startUploading()
{
    if (m_TaskUploadState != eFutureState::Unknown)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_ALREADYSTARTEDUPLOADING);

    m_TaskUploadState = eFutureState::Running;

    auto filesToUpload = m_filesToUpload;
    if (filesToUpload.size() == 0)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_NOFILESTOUPLOAD);

    auto pSDK = m_pSessionHandle->getEOSSDK();
    auto pSessionHandle = m_pSessionHandle;

    std::promise<bool> taskUploadPromise;
    m_TaskUploadFinished = taskUploadPromise.get_future();

    auto pState = m_pState;
    std::thread taskUploadThread([pSDK, filesToUpload, pSessionHandle, pState](std::promise<bool>&& promise) {
        try {

            auto setCurrentTransfer = [pState](EOS_FILETRANSFER_HANDLE fileTransferHandle) {
                std::lock_guard<std::mutex> lockGuard(pState->m_Mutex);
                pState->m_hCurrentTransfer = fileTransferHandle;
            };

            size_t nCurrentUploadIndex = 0;
            while (nCurrentUploadIndex < filesToUpload.size()) {

                if (pState->m_bAbort.load()) {
                    promise.set_value(false);
                    return;
                }

                std::string sFilePath = filesToUpload.at(nCurrentUploadIndex);

                std::wstring sFilePathW = AMCCommon::CUtils::UTF8toUTF16(sFilePath);
                EOS_FILETRANSFER_HANDLE fileTransferHandle = 0;
                pSDK->checkEOSError(pSDK->EosMachineSession_BeginTaskUpload(pSessionHandle->getSessionHandle(), sFilePathW.c_str(), &fileTransferHandle));

                if (fileTransferHandle == 0)
                    throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_RETRIEVEDINVALIDFILETRANSFERHANDLE);
                pSessionHandle->registerFileTransfer(fileTransferHandle);

                // An Abort that came before the handle was published cannot have cancelled this transfer.
                bool bAbortedBeforeStart = false;
                {
                    std::lock_guard<std::mutex> lockGuard(pState->m_Mutex);
                    pState->m_hCurrentTransfer = fileTransferHandle;
                    bAbortedBeforeStart = pState->m_bAbort.load();
                }
                if (bAbortedBeforeStart)
                    pSDK->EosMachineSession_AbortFileTransfer(fileTransferHandle);

                bool bIsFinished = false;
                while (!bIsFinished && !pState->m_bAbort.load()) {

                    double dSingleTransferProgress = pSessionHandle->getProgress(fileTransferHandle);
                    double dTotalProgress = ((double)nCurrentUploadIndex + dSingleTransferProgress) / ((double)filesToUpload.size());
                    pState->setProgress(dTotalProgress);

                    auto state = pSDK->EosMachineSession_WaitForFileTransfer(fileTransferHandle, 10);

                    if (state == EOS_ERR_FILETRANSFER_INVALID) {
                        setCurrentTransfer(0);
                        pSessionHandle->releaseFileTransfer(fileTransferHandle);
                        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_FILETRANSFERHANDLEBECAMEINVALID);
                    }

                    if (state == EOS_ERR_NO_ERROR) {
                        bIsFinished = true;
                    }

                }

                setCurrentTransfer(0);
                if (pState->m_bAbort.load()) {
                    pSessionHandle->releaseFileTransfer(fileTransferHandle);
                    promise.set_value(false);
                    return;
                }

                // The transfer has stopped, successfully or not.
                EOS_ERROR transferResult = pSDK->EosMachineSession_GetFileTransferResult(fileTransferHandle);
                pSessionHandle->releaseFileTransfer(fileTransferHandle);
                try {
                    pSDK->checkEOSError(transferResult);
                }
                catch (ELib3MF4EOSInterfaceException& e) {
                    throw ELib3MF4EOSInterfaceException(e.getErrorCode(), "upload of " + sFilePath + " failed: " + e.what());
                }

                nCurrentUploadIndex++;

                pState->setProgress((double)nCurrentUploadIndex / ((double)filesToUpload.size()));

            }

            promise.set_value(true);
        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }

    }, std::move(taskUploadPromise));

    taskUploadThread.detach();

   
}

