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


Abstract: This is the class declaration of CTaskCreationFuture

*/


#ifndef __LIBEOS_TASKCREATIONFUTURE
#define __LIBEOS_TASKCREATIONFUTURE

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_future.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include "libeos_eossdk.hpp"
#include "libeos_eosconversionlog.hpp"
#include <mutex>
#include <atomic>

#include "../Common/common_workingdirectory.hpp"
#include <future>

namespace Lib3MF4EOS {
namespace Impl {

// Shared by the future, its worker thread and the SDK progress callback, so that it outlives whichever ends last.
struct sTaskCreationProgress {
    std::mutex m_Mutex;
    uint64_t m_nCurrentLayerIndex = 0;
    uint64_t m_nCurrentLayerHeightInMicron = 0;
    uint64_t m_nTargetLayerIndex = 0;
    uint64_t m_nTargetLayerHeightInMicron = 0;
    std::atomic<bool> m_bAbort { false };

    void setProgress(uint64_t nFinishedLayerIndex, uint64_t nFinishedLayerHeightInMicron);
};

typedef std::shared_ptr<sTaskCreationProgress> PTaskCreationProgress;

// Closes the EOS toolpath file when the last owner releases it, which can be the worker thread after the future is gone.
class CToolpathFileHandle {
private:
    PEOSSDK m_pSDK;
    EOS_FILE_HANDLE m_hFile;
public:
    CToolpathFileHandle(PEOSSDK pSDK, EOS_FILE_HANDLE hFile);
    ~CToolpathFileHandle();
};

typedef std::shared_ptr<CToolpathFileHandle> PToolpathFileHandle;


/*************************************************************************************************************************
 Class declaration of CTaskCreationFuture 
**************************************************************************************************************************/

class CTaskCreationFuture : public virtual ITaskCreationFuture, public virtual CFuture {
protected:
    PEOSSDK m_pSDK;

    std::string m_sName;
    std::string m_sComment;
    std::string m_sGeneratorName;
    std::string m_sGeneratorVersion;

    std::wstring m_sNameW;
    std::wstring m_sCommentW;
    std::wstring m_sGeneratorNameW;
    std::wstring m_sGeneratorVersionW;

    AMCCommon::PWorkingDirectory m_pTaskDirectory;

    PTaskCreationProgress m_pProgress;

    std::future<bool> m_TaskCreationFinished;
    eFutureState m_TaskCreationState;

    bool m_bCreateRawTask;

    PToolpathFileHandle m_pInputFile;

    AMCCommon::PWorkingDirectory m_pInputDirectory;

    PConversionLogData m_pLog;

    void updateTaskCreationState();

    void registerProgressCallback();

    void startTaskCreationThread();

    void requestAbort();

public:

    CTaskCreationFuture (PEOSSDK pSDK, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion, bool bCreateRawTask);

    CTaskCreationFuture (PEOSSDK pSDK, EOS_FILE_HANDLE hToolpathFile, AMCCommon::PWorkingDirectory pInputDirectory, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion, PConversionLogData pLog);

    // A future whose conversion failed before the SDK started the task creation. pSDK may be null.
    CTaskCreationFuture (PEOSSDK pSDK, const std::string& sErrorMessage, PConversionLogData pLog);

    // A future of a toolpath written in recording mode. It has finished at once and has no task file.
    CTaskCreationFuture (PEOSSDK pSDK, AMCCommon::PWorkingDirectory pInputDirectory, PConversionLogData pLog, uint64_t nLastLayerIndex, int64_t nLastZHeightInMicrometers);

    virtual ~CTaskCreationFuture ();

    bool IsRunning() override;

    void Abort() override;

    bool HasFinished() override;

    bool HasFailed() override;

    bool HasBeenAborted() override;

	void GetProgress(Lib3MF4EOS_uint32 & nCurrentLayer, Lib3MF4EOS_uint32 & nNumberOfLayers, Lib3MF4EOS_uint32 & nCurrentZHeight, Lib3MF4EOS_uint32 & nTargetZHeight) override;

	void StoreInDirectory(const std::string & sPath) override;

    std::vector<std::string> getDataFilesToUpload();

    std::string getTaskFileToUpload();

    std::string FindTaskFileName() override;

    std::string GetWorkingDirectory() override;

    bool IsRawTask() override;

    IEOSRawTask* LoadRawTask() override;

    IEOSConversionLog* GetConversionLog() override;

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_TASKCREATIONFUTURE
