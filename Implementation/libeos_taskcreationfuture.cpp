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


Abstract: This is a stub class definition of CTaskCreationFuture

*/

#include "libeos_taskcreationfuture.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_eosrawtask.hpp"

// Include custom headers here.
#include "libeos_workingdirectory.hpp"

#include "Common/common_utils.hpp"

#include <filesystem>
#include <set>

#define MAXLAYERHEIGHTINMICRON 1000000000 /*(1km)*/
#define MAXLAYERCOUNT 100000000


using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CTaskCreationFuture 
**************************************************************************************************************************/

// The SDK runs one task creation per process and reports its progress here. The callback finds the progress of the
// running task creation through a weak pointer, so it never touches a future that has been released.
static std::mutex g_ActiveProgressMutex;
static std::weak_ptr<sTaskCreationProgress> g_pActiveProgress;

void taskCreationProgressCallback(EOS_DATA_HANDLE customDataHandle, EOS_LAYERINDEX finishedIndex, EOS_MICROMETER finishedHeight_um)
{
    PTaskCreationProgress pProgress;
    {
        std::lock_guard<std::mutex> lockGuard(g_ActiveProgressMutex);
        pProgress = g_pActiveProgress.lock();
    }

    if (pProgress.get() != nullptr)
        pProgress->setProgress(finishedIndex, finishedHeight_um);
}

void sTaskCreationProgress::setProgress(uint64_t nFinishedLayerIndex, uint64_t nFinishedLayerHeightInMicron)
{
    std::lock_guard<std::mutex> lockGuard(m_Mutex);
    m_nCurrentLayerIndex = nFinishedLayerIndex;
    m_nCurrentLayerHeightInMicron = nFinishedLayerHeightInMicron;
}

CToolpathFileHandle::CToolpathFileHandle(PEOSSDK pSDK, EOS_FILE_HANDLE hFile)
    : m_pSDK(pSDK), m_hFile(hFile)
{
}

CToolpathFileHandle::~CToolpathFileHandle()
{
    if ((m_hFile != 0) && (m_pSDK.get() != nullptr) && (m_pSDK->EosToolpath_CloseFile != nullptr))
        m_pSDK->EosToolpath_CloseFile(m_hFile);
}


CTaskCreationFuture::CTaskCreationFuture(PEOSSDK pSDK, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion, bool bCreateRawTask)
    : m_pSDK(pSDK), m_sName(sName), m_sComment(sComment), m_sGeneratorVersion(sGeneratorVersion), m_sGeneratorName(sGeneratorName),
    m_pProgress(std::make_shared<sTaskCreationProgress>()), m_TaskCreationState (eFutureState::Unknown),
    m_bCreateRawTask (bCreateRawTask), m_pLog(std::make_shared<CConversionLogData>())
{
    if (pSDK.get() == nullptr)
        throw std::runtime_error("invalid sdk parameter");

    m_sNameW = AMCCommon::CUtils::UTF8toUTF16(m_sName);
    m_sCommentW = AMCCommon::CUtils::UTF8toUTF16(m_sComment);
    m_sGeneratorVersionW = AMCCommon::CUtils::UTF8toUTF16(m_sGeneratorVersion);
    m_sGeneratorNameW = AMCCommon::CUtils::UTF8toUTF16(m_sGeneratorName);

    m_pTaskDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_td");

    if (!pSDK->simulateFullSDK()) {

        sEOSTaskMetaData metaData;
        metaData.m_sName = m_sNameW;
        metaData.m_sComment = m_sCommentW;
        metaData.m_nNumberOfAllowedBuilds = 0;
        metaData.m_sGeneratorName = m_sGeneratorNameW;
        metaData.m_sGeneratorVersion = m_sGeneratorVersionW;
        metaData.m_TaskMode = EOS_TASKMODE::EOS_TM_REGULAR;
        if (bCreateRawTask) {
            metaData.m_TaskFormat = EOS_TASKFORMAT::EOS_TF_RAW;
        }
        else {
            metaData.m_TaskFormat = EOS_TASKFORMAT::EOS_TF_REGULAR;
        }

        CCurrentDirectoryOverride directoryOverride(pSDK->getSDKDirectory());

        registerProgressCallback();

        EOS_ERROR error = m_pSDK->getAdapter().beginTaskCreation(metaData, m_pTaskDirectory->getPathW());
        if (error != EOS_ERR_NO_ERROR)
            throw std::runtime_error("could not begin task creation: " + std::to_string(error));

        EOS_LAYERINDEX targetLayerIndex = 0;
        error = m_pSDK->EosTaskGen_GetTargetLayerIndex(&targetLayerIndex);
        if (error != EOS_ERR_NO_ERROR)
            throw std::runtime_error("could not get target layer index.");

        EOS_MICROMETER targetZHeight = 0;
        error = m_pSDK->EosTaskGen_GetTargetZHeight(&targetZHeight);
        if (error != EOS_ERR_NO_ERROR)
            throw std::runtime_error("could not get target z height.");

        m_TaskCreationState = eFutureState::Running;
        std::lock_guard<std::mutex> lockGuard(m_pProgress->m_Mutex);
        m_pProgress->m_nTargetLayerIndex = targetLayerIndex;
        m_pProgress->m_nTargetLayerHeightInMicron = targetZHeight;

    }
    else {
        m_TaskCreationState = eFutureState::Running;
        std::lock_guard<std::mutex> lockGuard(m_pProgress->m_Mutex);
        m_pProgress->m_nTargetLayerIndex = 1000;
        m_pProgress->m_nTargetLayerHeightInMicron = m_pProgress->m_nTargetLayerIndex * 100;

    }

    startTaskCreationThread();
}

CTaskCreationFuture::CTaskCreationFuture(PEOSSDK pSDK, const std::string& sErrorMessage, PConversionLogData pLog)
    : m_pSDK(pSDK), m_pProgress(std::make_shared<sTaskCreationProgress>()),
    m_TaskCreationState(eFutureState::ExceptionOccured), m_bCreateRawTask(false), m_pLog(pLog)
{
    if (pLog.get() == nullptr)
        throw std::runtime_error("invalid log parameter");

    m_pTaskDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_td");
    setErrorMessage(sErrorMessage);
}

CTaskCreationFuture::CTaskCreationFuture(PEOSSDK pSDK, AMCCommon::PWorkingDirectory pInputDirectory, PConversionLogData pLog, uint64_t nLastLayerIndex, int64_t nLastZHeightInMicrometers)
    : m_pSDK(pSDK), m_pProgress(std::make_shared<sTaskCreationProgress>()),
    m_TaskCreationState(eFutureState::FinishedSuccessfully), m_bCreateRawTask(false), m_pInputDirectory(pInputDirectory), m_pLog(pLog)
{
    if (pSDK.get() == nullptr)
        throw std::runtime_error("invalid sdk parameter");
    if (pLog.get() == nullptr)
        throw std::runtime_error("invalid log parameter");

    uint64_t nLastZHeight = (nLastZHeightInMicrometers > 0) ? (uint64_t)nLastZHeightInMicrometers : 0;
    m_pProgress->m_nCurrentLayerIndex = nLastLayerIndex;
    m_pProgress->m_nCurrentLayerHeightInMicron = nLastZHeight;
    m_pProgress->m_nTargetLayerIndex = nLastLayerIndex;
    m_pProgress->m_nTargetLayerHeightInMicron = nLastZHeight;

    m_pTaskDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_td");
}

CTaskCreationFuture::CTaskCreationFuture(PEOSSDK pSDK, EOS_FILE_HANDLE hToolpathFile, AMCCommon::PWorkingDirectory pInputDirectory, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion, PConversionLogData pLog)
    : m_pSDK(pSDK), m_sName(sName), m_sComment(sComment), m_sGeneratorVersion(sGeneratorVersion), m_sGeneratorName(sGeneratorName),
    m_pProgress(std::make_shared<sTaskCreationProgress>()), m_TaskCreationState(eFutureState::Unknown),
    m_bCreateRawTask(false), m_pInputFile(std::make_shared<CToolpathFileHandle>(pSDK, hToolpathFile)), m_pInputDirectory(pInputDirectory), m_pLog(pLog)
{
    if (pSDK.get() == nullptr)
        throw std::runtime_error("invalid sdk parameter");
    if (pLog.get() == nullptr)
        throw std::runtime_error("invalid log parameter");
    if (hToolpathFile == 0)
        throw std::runtime_error("invalid toolpath file handle");

    m_sNameW = AMCCommon::CUtils::UTF8toUTF16(m_sName);
    m_sCommentW = AMCCommon::CUtils::UTF8toUTF16(m_sComment);
    m_sGeneratorVersionW = AMCCommon::CUtils::UTF8toUTF16(m_sGeneratorVersion);
    m_sGeneratorNameW = AMCCommon::CUtils::UTF8toUTF16(m_sGeneratorName);

    m_pTaskDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_td");

    if (!pSDK->simulateFullSDK()) {

        auto& adapter = m_pSDK->getAdapter();
        if (!adapter.supportsTaskCreationFromToolpath())
            throw std::runtime_error("EosTaskGen_BeginTaskCreationFromToolpath is not available");

        sEOSTaskMetaData metaData;
        metaData.m_sName = m_sNameW;
        metaData.m_sComment = m_sCommentW;
        metaData.m_nNumberOfAllowedBuilds = 0;
        metaData.m_sGeneratorName = m_sGeneratorNameW;
        metaData.m_sGeneratorVersion = m_sGeneratorVersionW;
        metaData.m_TaskMode = EOS_TASKMODE::EOS_TM_REGULAR;
        metaData.m_TaskFormat = EOS_TASKFORMAT::EOS_TF_REGULAR;

        CCurrentDirectoryOverride directoryOverride(pSDK->getSDKDirectory());

        registerProgressCallback();

        EOS_ERROR error = adapter.beginTaskCreationFromToolpath(hToolpathFile, metaData, m_pTaskDirectory->getPathW());
        if (error != EOS_ERR_NO_ERROR)
            throw std::runtime_error("could not begin task creation from toolpath: " + std::to_string(error));

        EOS_LAYERINDEX targetLayerIndex = 0;
        error = m_pSDK->EosTaskGen_GetTargetLayerIndex(&targetLayerIndex);
        if (error != EOS_ERR_NO_ERROR)
            throw std::runtime_error("could not get target layer index.");

        EOS_MICROMETER targetZHeight = 0;
        error = m_pSDK->EosTaskGen_GetTargetZHeight(&targetZHeight);
        if (error != EOS_ERR_NO_ERROR)
            throw std::runtime_error("could not get target z height.");

        m_TaskCreationState = eFutureState::Running;
        std::lock_guard<std::mutex> lockGuard(m_pProgress->m_Mutex);
        m_pProgress->m_nTargetLayerIndex = targetLayerIndex;
        m_pProgress->m_nTargetLayerHeightInMicron = targetZHeight;

    }
    else {
        m_TaskCreationState = eFutureState::Running;
        std::lock_guard<std::mutex> lockGuard(m_pProgress->m_Mutex);
        m_pProgress->m_nTargetLayerIndex = 1000;
        m_pProgress->m_nTargetLayerHeightInMicron = m_pProgress->m_nTargetLayerIndex * 100;

    }

    startTaskCreationThread();
}

CTaskCreationFuture::~CTaskCreationFuture()
{
    try {
        updateTaskCreationState();
        if (m_TaskCreationState == eFutureState::Running)
            requestAbort();
    }
    catch (...) {
    }

    // The SDK keeps the callback; it does nothing once no progress is active.
    std::lock_guard<std::mutex> lockGuard(g_ActiveProgressMutex);
    if (g_pActiveProgress.lock() == m_pProgress)
        g_pActiveProgress.reset();
}

void CTaskCreationFuture::registerProgressCallback()
{
    {
        std::lock_guard<std::mutex> lockGuard(g_ActiveProgressMutex);
        g_pActiveProgress = m_pProgress;
    }
    m_pSDK->EosTaskGen_SetProgressCallback(taskCreationProgressCallback, (uintptr_t)m_pProgress.get());
}

void CTaskCreationFuture::startTaskCreationThread()
{
    std::promise<bool> taskCreationPromise;
    m_TaskCreationFinished = taskCreationPromise.get_future();

    // The thread may outlive the future, so it only holds shared state. The input file stays open until the SDK has stopped.
    auto pSDK = m_pSDK;
    auto pProgress = m_pProgress;
    auto pInputFile = m_pInputFile;
    std::thread taskCreationThread([pSDK, pProgress, pInputFile](std::promise<bool>&& promise) {
        try {

            if (!pSDK->simulateFullSDK()) {

                bool bIsFinished = false;
                while (!bIsFinished) {

                    auto state = pSDK->EosTaskGen_WaitForTaskCreation(10);

                    if (state == EOS_ERR_TASK_CREATION_NOT_RUNNING) {
                        if (pProgress->m_bAbort.load()) {
                            promise.set_value(false);
                            return;
                        }

                        uint64_t nStoppedLayer, nStoppedHeight, nTgtLayer, nTgtHeight;
                        {
                            std::lock_guard<std::mutex> lock(pProgress->m_Mutex);
                            nStoppedLayer = pProgress->m_nCurrentLayerIndex;
                            nStoppedHeight = pProgress->m_nCurrentLayerHeightInMicron;
                            nTgtLayer = pProgress->m_nTargetLayerIndex;
                            nTgtHeight = pProgress->m_nTargetLayerHeightInMicron;
                        }
                        throw std::runtime_error("task creation is not running (stopped at layer "
                            + std::to_string(nStoppedLayer) + " of " + std::to_string(nTgtLayer)
                            + ", Z: " + std::to_string(nStoppedHeight) + " / " + std::to_string(nTgtHeight) + " um)");
                    }

                    if (state == EOS_ERR_NO_ERROR) {
                        bIsFinished = true;
                    }

                }

                if (pProgress->m_bAbort.load()) {
                    promise.set_value(false);
                    return;
                }

                // Task creation has stopped, successfully or not.
                pSDK->checkEOSError(pSDK->getAdapter().getTaskCreationResult());
            }
            else {
                uint64_t nTargetLayerIndex;
                {
                    std::lock_guard<std::mutex> lock(pProgress->m_Mutex);
                    nTargetLayerIndex = pProgress->m_nTargetLayerIndex;
                }

                for (uint64_t nLayerIndex = 1; (nLayerIndex <= nTargetLayerIndex) && !pProgress->m_bAbort.load(); nLayerIndex++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    pProgress->setProgress(nLayerIndex, nLayerIndex * 100);
                }

                if (pProgress->m_bAbort.load()) {
                    promise.set_value(false);
                    return;
                }
            }

            promise.set_value(true);
        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }

    }, std::move(taskCreationPromise));

    taskCreationThread.detach();
}

void CTaskCreationFuture::requestAbort()
{
    m_pProgress->m_bAbort.store(true);
    if (!m_pSDK->simulateFullSDK())
        m_pSDK->EosTaskGen_AbortTaskCreation();
}

bool CTaskCreationFuture::IsRunning()
{
    updateTaskCreationState();
    return m_TaskCreationState == eFutureState::Running;
}

void CTaskCreationFuture::Abort()
{
    if (!IsRunning())
        return;

    requestAbort();
    m_TaskCreationFinished = std::future<bool>();
    m_TaskCreationState = eFutureState::Aborted;
}

bool CTaskCreationFuture::HasFinished()
{
    updateTaskCreationState();
    return m_TaskCreationState == eFutureState::FinishedSuccessfully;
}

bool CTaskCreationFuture::HasFailed()
{
    updateTaskCreationState();
    return m_TaskCreationState == eFutureState::ExceptionOccured;
}

bool CTaskCreationFuture::HasBeenAborted()
{
    updateTaskCreationState();
    return m_TaskCreationState == eFutureState::Aborted;
}



void CTaskCreationFuture::GetProgress(Lib3MF4EOS_uint32 & nCurrentLayer, Lib3MF4EOS_uint32 & nNumberOfLayers, Lib3MF4EOS_uint32 & nCurrentZHeight, Lib3MF4EOS_uint32 & nTargetZHeight)
{
    std::lock_guard<std::mutex> lockGuard(m_pProgress->m_Mutex);
    auto& progress = *m_pProgress;

    if (progress.m_nCurrentLayerHeightInMicron >= MAXLAYERHEIGHTINMICRON)
        throw std::runtime_error("invalid layer height: " + std::to_string(progress.m_nCurrentLayerHeightInMicron));
    if (progress.m_nTargetLayerHeightInMicron >= MAXLAYERHEIGHTINMICRON)
        throw std::runtime_error("invalid target layer height: " + std::to_string(progress.m_nTargetLayerHeightInMicron));
    if (progress.m_nCurrentLayerIndex >= MAXLAYERCOUNT)
        throw std::runtime_error("invalid layer index: " + std::to_string(progress.m_nCurrentLayerIndex));
    if (progress.m_nTargetLayerIndex >= MAXLAYERCOUNT)
        throw std::runtime_error("invalid target layer index: " + std::to_string(progress.m_nTargetLayerIndex));

    nCurrentLayer = (uint32_t)progress.m_nCurrentLayerIndex;
    nNumberOfLayers = (uint32_t)progress.m_nTargetLayerIndex;
    nCurrentZHeight = (uint32_t)progress.m_nCurrentLayerHeightInMicron;
    nTargetZHeight = (uint32_t)progress.m_nTargetLayerHeightInMicron;
}

void CTaskCreationFuture::StoreInDirectory(const std::string & sPath)
{
    if (sPath.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "empty target directory");

    if (!HasFinished())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKISNOTFINISHED);

    std::filesystem::path targetPath = std::filesystem::u8path(sPath);
    std::filesystem::create_directories(targetPath);

    std::filesystem::path taskFilePath = std::filesystem::u8path(FindTaskFileName());

    std::set<std::filesystem::path> filesToCopy;
    filesToCopy.insert(taskFilePath);

    for (auto& sDataFile : m_pTaskDirectory->findFilesInDirectory("*.data", true))
        filesToCopy.insert(std::filesystem::u8path(sDataFile));

    std::filesystem::path taskFileDirectory = taskFilePath.parent_path();
    for (auto& entry : std::filesystem::directory_iterator(taskFileDirectory)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".data"))
            filesToCopy.insert(entry.path());
    }

    for (auto& sourcePath : filesToCopy) {
        std::filesystem::copy_file(sourcePath, targetPath / sourcePath.filename(), std::filesystem::copy_options::overwrite_existing);
    }
}

std::vector<std::string> CTaskCreationFuture::getDataFilesToUpload()
{
    return m_pTaskDirectory->findFilesInDirectory("*.data", true);
}

std::string CTaskCreationFuture::getTaskFileToUpload()
{
    auto taskFiles = m_pTaskDirectory->findFilesInDirectory("*.task", true);
    if (taskFiles.size() == 0)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSTASKFILE_NOTEXISTING);
    if (taskFiles.size() > 1)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSTASKFILE_AMBIGUOUS);

    return taskFiles.at(0);
}

void CTaskCreationFuture::updateTaskCreationState()
{
    if (m_TaskCreationFinished.valid()) {

        if (m_TaskCreationFinished.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            try {
                if (m_TaskCreationFinished.get())
                    m_TaskCreationState = eFutureState::FinishedSuccessfully;
                else
                    m_TaskCreationState = eFutureState::Aborted;
            }
            catch (...) {
                setErrorMessageFromException(std::current_exception());
                m_TaskCreationState = eFutureState::ExceptionOccured;
                m_pLog->addMessage(Lib3MF4EOS::eEOSLogSeverity::Error, "TASK_CREATION_FAILED", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, GetErrorMessage());
            }
        }
    }
}

std::string CTaskCreationFuture::FindTaskFileName()
{
    std::string sFilter;
    if (m_bCreateRawTask) {
		sFilter = "*.rawtask";
    }
    else {
		sFilter = "*.task";
    }

	try {
		auto taskFileNames = m_pTaskDirectory->findFilesInDirectory(sFilter, true);

		if (taskFileNames.size() == 1)
			return taskFileNames.at(0);
		if (taskFileNames.size() > 1)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSTASKFILE_NOTUNIQUE, "eos task file not unique in " + m_pTaskDirectory->getPath());
	}
	catch (const ELib3MF4EOSInterfaceException&) {
		throw;
	}
	catch (...) {
	}

	if (m_pInputDirectory.get() != nullptr) {
		try {
			auto taskFileNames = m_pInputDirectory->findFilesInDirectory(sFilter, true);

			if (taskFileNames.size() == 1)
				return taskFileNames.at(0);
			if (taskFileNames.size() > 1)
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSTASKFILE_NOTUNIQUE, "eos task file not unique in " + m_pInputDirectory->getPath());
		}
		catch (const ELib3MF4EOSInterfaceException&) {
			throw;
		}
		catch (...) {
		}
	}

	throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSTASKFILE_NOTEXISTING, "eos task file not existing in " + m_pTaskDirectory->getPath());
}

std::string CTaskCreationFuture::GetWorkingDirectory()
{
	return m_pTaskDirectory->getPath();
}

bool CTaskCreationFuture::IsRawTask()
{
    return m_bCreateRawTask;
}

IEOSRawTask* CTaskCreationFuture::LoadRawTask()
{
    if (!m_bCreateRawTask)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKISNOTRAWTASK);

    if (!HasFinished ())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKISNOTFINISHED);

    std::string sAbsoluteTaskFileName = FindTaskFileName();

    return new CEOSRawTask(m_pSDK, m_pTaskDirectory, sAbsoluteTaskFileName);
}

IEOSConversionLog* CTaskCreationFuture::GetConversionLog()
{
    return new CEOSConversionLog(m_pLog);
}

