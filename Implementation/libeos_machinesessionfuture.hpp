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


Abstract: This is the class declaration of CMachineSessionFuture

*/


#ifndef __LIBEOS_MACHINESESSIONFUTURE
#define __LIBEOS_MACHINESESSIONFUTURE

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_future.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include "libeos_eossdk.hpp"
#include "libeos_machinesessionhandle.hpp"
#include "../Common/common_workingdirectory.hpp"

#include <atomic>
#include <future>
#include <thread>

namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CMachineSessionFuture 
**************************************************************************************************************************/

class CMachineSessionFuture : public virtual IMachineSessionFuture, public virtual CFuture {
protected:
    PEOSSDK m_pEOSSDK;
    std::string m_sNetworkAddress;

    AMCCommon::PWorkingDirectory m_pCertificateStoreDirectory;
    AMCCommon::PWorkingDirectory m_pTempStoreDirectory;

    std::future<PMachineSessionHandle> m_pSessionHandleFuture;
    PMachineSessionHandle m_pSessionHandle;
    // Set by Abort, Disconnect and the destructor. A connection that completes afterwards is closed by the connection thread.
    std::shared_ptr<std::atomic<bool>> m_pAbortFlag;

    eFutureState m_SessionState;
    void updateSessionState();

public:
    CMachineSessionFuture(PEOSSDK pEOSSDK, const std::string sNetworkAddress);

    virtual ~CMachineSessionFuture();

    bool IsRunning() override;

    void Abort() override;

    bool HasFinished() override;

    bool HasFailed() override;

    bool HasBeenAborted() override;

    void Disconnect() override;

	bool IsConnected() override;

	std::string GetNetworkAddress() override;

	IMachineConfigFuture * DownloadMachineConfig() override;

	ITaskUploadFuture * UploadTask(ITaskCreationFuture* pTaskCreationInstance) override;

	ITaskUploadFuture * UploadTaskFile(const std::string & sTaskFileName) override;

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_MACHINESESSIONFUTURE
