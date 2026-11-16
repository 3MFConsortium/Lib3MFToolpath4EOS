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


Abstract: This is the class declaration of CTaskUploadFuture

*/


#ifndef __LIBEOS_TASKUPLOADFUTURE
#define __LIBEOS_TASKUPLOADFUTURE

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_future.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

#include "libeos_machinesessionhandle.hpp"
#include <vector>
#include <mutex>
#include <future>
#include <atomic>

namespace Lib3MF4EOS {
namespace Impl {

// Shared by the future and its worker thread, so that the thread never touches a released future.
struct sTaskUploadState {
    std::mutex m_Mutex;
    double m_dProgress = 0.0;
    // The running transfer, 0 between transfers. Abort cancels it under the mutex, so it is never cancelled after its release.
    EOS_FILETRANSFER_HANDLE m_hCurrentTransfer = 0;
    std::atomic<bool> m_bAbort { false };

    void setProgress(double dProgress);
};

typedef std::shared_ptr<sTaskUploadState> PTaskUploadState;


/*************************************************************************************************************************
 Class declaration of CTaskUploadFuture 
**************************************************************************************************************************/

class CTaskUploadFuture : public virtual ITaskUploadFuture, public virtual CFuture {
protected:
    PMachineSessionHandle m_pSessionHandle;
    std::vector <std::string> m_filesToUpload;

    PTaskUploadState m_pState;

    std::future<bool> m_TaskUploadFinished;
    eFutureState m_TaskUploadState;

    void requestAbort();

public:

    CTaskUploadFuture(PMachineSessionHandle pSessionHandle);

    virtual ~CTaskUploadFuture();

    bool IsRunning() override;

    void Abort() override;

    bool HasFinished() override;

    bool HasFailed() override;

    bool HasBeenAborted() override;

	Lib3MF4EOS_double GetProgress() override;

    void addUpload(const std::string& sFileName);

    void startUploading();

    void updateTaskUploadState();

    void simulateUploading();

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_TASKUPLOADFUTURE
