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


Abstract: This is a stub class definition of CMachineConfigFuture

*/

#include "libeos_machineconfigfuture.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_workingdirectory.hpp"
#include "libeos_eosmachineinfo.hpp"

using namespace Lib3MF4EOS::Impl;

#include "../Common/common_exportstream_native.hpp"
#include "../Common/common_importstream_native.hpp"
#include "../Common/common_importstream_shared_memory.hpp"
#include "../Common/common_resourcepackage.hpp"
#include "../Common/common_utils.hpp"
#include "../Libraries/cppcodec/base64_rfc4648.hpp"

#define MACHINECONFIGPACKAGE_MINSIZE 16
#define MACHINECONFIGPACKAGE_MAXSIZE (128 * 1024 * 1024)

/*************************************************************************************************************************
 Class definition of CMachineConfigFuture 
**************************************************************************************************************************/

CMachineConfigFuture::CMachineConfigFuture(PEOSSDK pEOSSDK, const CNamespaceRegistry& registry)
    : m_pSDK(pEOSSDK), m_NamespaceRegistry(registry), m_ConfigState(eFutureState::Unknown), m_pAbortFlag(std::make_shared<std::atomic<bool>>(false))
{
    if (pEOSSDK.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

    m_StartTime = m_Clock.now();
}

CMachineConfigFuture::~CMachineConfigFuture()
{
    try {
        if (m_ConfigDownloadFuture.valid())
            requestAbort();
    }
    catch (...) {
    }
}

bool CMachineConfigFuture::IsRunning()
{
    updateConfigState();
    return m_ConfigState == eFutureState::Running;

    /*auto pEOSSDK = m_pSessionHandle->getEOSSDK();

    if (!pEOSSDK->isSimulationMode()) {

        if (m_bIsRunning) {
            if (m_pDownloadConfigTransferHandle.get() != nullptr) {

                EOS_ERROR error = pEOSSDK->EosMachineSession_WaitForFileTransfer(m_pDownloadConfigTransferHandle->getHandle(), 1);
                if (error == EOS_ERR_NO_ERROR) {
                    m_bIsRunning = false;
                    m_bIsSuccessful = true;
                    return false;
                }
            }
        }
    }
    else {

        auto duration = m_Clock.now() - m_StartTime;
        if (duration >= std::chrono::milliseconds(1000)) {
            m_bIsRunning = false;
            m_bIsSuccessful = true;
        }
    }

    return m_bIsRunning; */
}

void CMachineConfigFuture::Abort()
{
    updateConfigState();
    if (m_ConfigState != eFutureState::Running)
        return;

    requestAbort();
    m_ConfigDownloadFuture = std::future<std::vector<uint8_t>>();
    m_ConfigState = eFutureState::Aborted;
}

void CMachineConfigFuture::requestAbort()
{
    m_pAbortFlag->store(true);

    if ((m_pSessionHandle.get() != nullptr) && (m_pDownloadConfigTransferHandle.get() != nullptr)) {
        auto pEOSSDK = m_pSessionHandle->getEOSSDK();
        // Fails harmlessly if the transfer has just ended; the download thread stops on the abort flag either way.
        if (!pEOSSDK->simulateConnectivity())
            pEOSSDK->EosMachineSession_AbortFileTransfer(m_pDownloadConfigTransferHandle->getHandle());
    }
}

bool CMachineConfigFuture::HasFinished()
{
    updateConfigState();
    return m_ConfigState == eFutureState::FinishedSuccessfully;
}

bool CMachineConfigFuture::HasFailed()
{
    updateConfigState();
    return m_ConfigState == eFutureState::ExceptionOccured;
}

bool CMachineConfigFuture::HasBeenAborted()
{
    updateConfigState();
    return m_ConfigState == eFutureState::Aborted;
}

void CMachineConfigFuture::StoreInDirectory(const std::string & sPath)
{
    updateConfigState();
    AMCCommon::CImportStream_Shared_Memory importStream(m_StreamData.data(), m_StreamData.size());
    AMCCommon::CResourcePackage resourcePackage(&importStream, "package", m_NamespaceRegistry.acceptedNamespaces(Lib3MF4EOS::eEOSNamespaceKind::MachinePackage));

    auto entryCount = resourcePackage.getEntryCount();
    for (uint64_t entryIndex = 0; entryIndex < entryCount; entryIndex++) {
        auto pPackageEntry = resourcePackage.getEntry(entryIndex);
        auto sName = pPackageEntry->getName();

        auto sFilePath = AMCCommon::CUtils::includeTrailingPathDelimiter(sPath) + sName + "." + pPackageEntry->getExtension();

        std::vector<uint8_t> Buffer;        
        resourcePackage.readEntry (sName, Buffer);

        AMCCommon::CExportStream_Native exportStream (sFilePath);
        if (Buffer.size() > 0) {
            exportStream.writeBuffer (Buffer.data (), Buffer.size ());
        }
    }

}


void CMachineConfigFuture::storeInWorkingDirectory(AMCCommon::PWorkingDirectory pDirectory)
{
    updateConfigState();

    if (m_StreamData.size() == 0)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EMPTYMACHINECONFIGSTREAM);

    AMCCommon::CImportStream_Shared_Memory importStream(m_StreamData.data(), m_StreamData.size());
    AMCCommon::CResourcePackage resourcePackage(&importStream, "package", m_NamespaceRegistry.acceptedNamespaces(Lib3MF4EOS::eEOSNamespaceKind::MachinePackage));

    auto entryCount = resourcePackage.getEntryCount();
    for (uint64_t entryIndex = 0; entryIndex < entryCount; entryIndex++) {
        auto pPackageEntry = resourcePackage.getEntry(entryIndex);
        auto sName = pPackageEntry->getName();

        auto sFileName = sName + "." + pPackageEntry->getExtension();

        std::vector<uint8_t> Buffer;
        resourcePackage.readEntry(sName, Buffer);
        pDirectory->storeData(sFileName, Buffer);
    }
}

void CMachineConfigFuture::RetrievePackageData(Lib3MF4EOS_uint64 nStreamDataBufferSize, Lib3MF4EOS_uint64* pStreamDataNeededCount, Lib3MF4EOS_uint8 * pStreamDataBuffer)
{
    updateConfigState();
    if (pStreamDataNeededCount != nullptr)
        *pStreamDataNeededCount = m_StreamData.size();

    if (pStreamDataBuffer != nullptr) {
        if (nStreamDataBufferSize < m_StreamData.size())
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUFFERTOOSMALL);

        for (size_t nIndex = 0; nIndex < m_StreamData.size(); nIndex++)
            pStreamDataBuffer[nIndex] = m_StreamData.at(nIndex);
    }
}

void CMachineConfigFuture::StorePackageInFile(const std::string & sFileName)
{
    updateConfigState();

    AMCCommon::CExportStream_Native exportStream(sFileName);
    if (m_StreamData.size() > 0) {
        exportStream.writeBuffer(m_StreamData.data(), m_StreamData.size());
    }
}


void CMachineConfigFuture::loadFromEOSSession(PMachineSessionHandle pSessionHandle)
{

    if (pSessionHandle.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

    m_pSessionHandle = pSessionHandle;

    auto pEOSSDK = pSessionHandle->getEOSSDK();

    if (!pEOSSDK->simulateConnectivity()) {

        EOS_FILETRANSFER_HANDLE nFileTransferHandle = 0;

        m_pMachineConfigTempDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_mctemp");

        pEOSSDK->checkEOSError(pEOSSDK->EosMachineSession_BeginConfigDownload(pSessionHandle->getSessionHandle(), m_pMachineConfigTempDirectory->getPathW().c_str(), &nFileTransferHandle));

        if (nFileTransferHandle == 0)
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_RETRIEVEDINVALIDFILETRANSFERHANDLE);

        m_pDownloadConfigTransferHandle = m_pSessionHandle->registerFileTransfer(nFileTransferHandle);


        //
    }
    else {

    }

    std::promise<std::vector<uint8_t>> downloadPromise;
    m_ConfigDownloadFuture = downloadPromise.get_future();

    m_ConfigState = eFutureState::Running;

    auto pDownloadHandle = m_pDownloadConfigTransferHandle;
    auto pTempDirectory = m_pMachineConfigTempDirectory;
    auto pAbortFlag = m_pAbortFlag;

    std::thread downloadThread([pEOSSDK, pSessionHandle, pDownloadHandle, pTempDirectory, pAbortFlag](std::promise<std::vector<uint8_t>>&& promise) {
        try {

            if (!pEOSSDK->simulateConnectivity()) {

                bool bStopped = false;
                while (!bStopped && !pAbortFlag->load()) {
                    EOS_ERROR error = pEOSSDK->EosMachineSession_WaitForFileTransfer(pDownloadHandle->getHandle(), 100);
                    if (error == EOS_ERR_NO_ERROR) {
                        bStopped = true;
                    }
                    else {
                        if (error != EOS_ERR_WAIT_TIMED_OUT)
                            pEOSSDK->checkEOSError(error);
                    }
                }

                if (pAbortFlag->load()) {
                    promise.set_value(std::vector<uint8_t>());
                    return;
                }

                // The transfer has stopped, successfully or not.
                EOS_ERROR transferResult = pEOSSDK->EosMachineSession_GetFileTransferResult(pDownloadHandle->getHandle());
                try {
                    pEOSSDK->checkEOSError(transferResult);
                }
                catch (ELib3MF4EOSInterfaceException& e) {
                    throw ELib3MF4EOSInterfaceException(e.getErrorCode(), std::string("machine configuration download failed: ") + e.what());
                }

                promise.set_value(buildPackageFromPath(pTempDirectory->getPath()));
            }
            else
            {
                for (uint32_t nStep = 0; (nStep < 20) && !pAbortFlag->load(); nStep++)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if (pAbortFlag->load())
                    promise.set_value(std::vector<uint8_t>());
                else
                    promise.set_value(buildSimulationPackage());
            }

        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }

    }, std::move(downloadPromise));

    downloadThread.detach();

}


void CMachineConfigFuture::loadFromArray(uint64_t nArraySize, const uint8_t* pData)
{   
    if (pData == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

    if ((nArraySize < MACHINECONFIGPACKAGE_MINSIZE) || (nArraySize > MACHINECONFIGPACKAGE_MAXSIZE))
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDMACHINECONFIGPACKAGESIZE);

    m_StreamData.resize(nArraySize);
    for (size_t nIndex = 0; nIndex < nArraySize; nIndex++)
        m_StreamData[nIndex] = pData[nIndex];

    m_ConfigState = eFutureState::FinishedSuccessfully;
}

void CMachineConfigFuture::loadFromStream(AMCCommon::CImportStream& importStream)
{
    auto nStreamSize = importStream.retrieveSize();
    if ((nStreamSize < MACHINECONFIGPACKAGE_MINSIZE) || (nStreamSize > MACHINECONFIGPACKAGE_MAXSIZE))
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDMACHINECONFIGPACKAGESIZE);

    m_StreamData.resize(nStreamSize);
    importStream.seekPosition(0, true);
    importStream.readBuffer(m_StreamData.data(), nStreamSize, true);

    m_ConfigState = eFutureState::FinishedSuccessfully;
}

void CMachineConfigFuture::loadFromDirectory(const std::string& sDirectoryPath)
{
    if (!AMCCommon::CUtils::fileOrPathExistsOnDisk(sDirectoryPath))
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MACHINEPACKAGEFILENOTFOUND, "machine config directory not found: " + sDirectoryPath);
    if (!AMCCommon::CUtils::pathIsDirectory(sDirectoryPath))
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "machine config path is not a directory: " + sDirectoryPath);

    m_StreamData = buildPackageFromPath(sDirectoryPath);

    if (m_StreamData.size() == 0)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EMPTYMACHINECONFIGSTREAM, "machine config directory is empty: " + sDirectoryPath);

    m_ConfigState = eFutureState::FinishedSuccessfully;
}

std::vector<uint8_t> CMachineConfigFuture::buildPackageFromPath(const std::string& sDirectoryPath)
{
    std::vector<uint8_t> packageData;

    std::string sPath = AMCCommon::CUtils::includeTrailingPathDelimiter(sDirectoryPath);

    auto sTempFileName = AMCCommon::CUtils::findTemporaryFileName(AMCCommon::CUtils::getTempFolder(), "machineconfig_", ".zip", 16);

    auto pExportStream = std::make_shared<AMCCommon::CExportStream_Native>(sTempFileName);

    {
        AMCCommon::CResourcePackageBuilder resourceBuilder(pExportStream, LIBEOS_NAMESPACE_MACHINEPACKAGE);

        auto filesInDirectory = AMCCommon::CUtils::findContentOfDirectory(sDirectoryPath, true, false);
        for (auto sFileName : filesInDirectory) {

            auto pFileStream = std::make_shared<AMCCommon::CImportStream_Native>(sPath + sFileName);
            auto nSize = pFileStream->retrieveSize();
            std::vector<uint8_t> buffer;
            if (nSize > 0) {
                buffer.resize(nSize);
                pFileStream->readBuffer(buffer.data(), nSize, true);
            }

            std::string sName, sExtension;

            auto nPointPos = sFileName.find(".");
            if (nPointPos != std::string::npos) {
                sName = sFileName.substr(0, nPointPos);
                sExtension = sFileName.substr(nPointPos + 1);
            }
            else {
                sName = sFileName;
                sExtension = "";
            }

            resourceBuilder.addEntry(sName, sFileName, sExtension, "application/binary", buffer);
        }

        resourceBuilder.finalize();
        pExportStream = nullptr;

    }

    {
        AMCCommon::CImportStream_Native importStream(sTempFileName);
        auto nSize = importStream.retrieveSize();
        packageData.resize(nSize);
        if (nSize > 0)
            importStream.readBuffer(packageData.data(), nSize, true);
    }

    AMCCommon::CUtils::deleteFileFromDisk(sTempFileName, false);

    return packageData;
}

std::vector<uint8_t> CMachineConfigFuture::buildSimulationPackage()
{
    std::string sBase64Machine;
    sBase64Machine += "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";
    sBase64Machine += "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";
    sBase64Machine += "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";
    sBase64Machine += "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";
    sBase64Machine += "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";
    sBase64Machine += "AAgAAAAAAESLXnr//////////wwAHAAAAAAAAACAAAAA/////0ZpcnN0VXNlLmN0YgEAGABvQgAAAAAAADQsAAAAAAAAMQIAAAAAAABQSwECLQAtAAAACAAAAAAAEUBh7v//////////DAAcAAAAAAAAAIAAAAD/////Z2Fsdm9jb24uY2Z0AQAYAMU1AAAAAAAA6SwAAAAAAACjLgAAAAAAAFBLAQItAC0AAAAIAAAAAABjE6Sp//////////8QABwAAAAAAAAAgAAAAP////9tYWNoaW5lX2luZm8uY2Z0AQAYAEIBAAAAAAAAxgAAAAAAAADKWwAAAAAAAFBLAQItAC0AAAAIAAAAAACAEmgo//////////8HABwAAAAAAAAAgAAAAP////9tcGMuY2Z0AQAYAI0jAAAAAAAAFhkAAAAAAADSXAAAAAAAAFBLAQItAC0AAAAIAAAAAAA5rQYD//////////8LABwAAAAAAAAAgAAAAP////9wYWNrYWdlLnhtbAEAGADpAgAAAAAAAPUAAAAAAAAAIXYAAAAAAABQSwYGLAAAAAAAAAAtAC0AAAAAAAAAAAAHAAAAAAAAAAcAAAAAAAAAUAIAAAAAAABTdwAAAAAAAFBLBgcAAAAAo3kAAAAAAAABAAAAUEsFBgAAAAAHAAcAUAIAAP////8AAA==";

    return cppcodec::base64_rfc4648::decode(sBase64Machine);
}


void CMachineConfigFuture::updateConfigState()
{
    if (m_ConfigDownloadFuture.valid()) {

        if (m_ConfigDownloadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            try {
                m_StreamData = m_ConfigDownloadFuture.get();
                m_ConfigState = eFutureState::FinishedSuccessfully;
            }
            catch (...) {
                setErrorMessageFromException(std::current_exception());
                m_ConfigState = eFutureState::ExceptionOccured;
            }
        }
    }
}


IEOSMachineInfo* CMachineConfigFuture::ReadMachineInformation()
{

    std::unique_ptr<CEOSMachineInfo> pMachineInformationResult(new CEOSMachineInfo());

    if (m_pSDK.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

    if (!m_pSDK->simulateFullSDK()) {

        CCurrentDirectoryOverride directoryOverride(m_pSDK->getSDKDirectory ());

        auto pWorkingDirectory = std::make_shared<AMCCommon::CWorkingDirectory> ("eosmc_");
        storeInWorkingDirectory(pWorkingDirectory);        

        sEOSMachineInfo machineInfo;
        EOS_ERROR error = m_pSDK->getAdapter().loadMachineInfo(pWorkingDirectory->getPathW(), machineInfo);
        if (error == EOS_ERR_EMPTY)
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTGETMACHINEINFORMATION);
        m_pSDK->checkEOSError(error);

        pMachineInformationResult->loadFromMachineInfo(machineInfo);
        
    }
    else {
        pMachineInformationResult->loadSimulatedMachineInfo(m_pSDK->getSimulator()->getLaserScannerCount());

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    return pMachineInformationResult.release();
}

