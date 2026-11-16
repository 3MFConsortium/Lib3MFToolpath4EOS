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


Abstract: This is the class declaration of CMachineConfigFuture

*/


#ifndef __LIBEOS_MACHINECONFIGFUTURE
#define __LIBEOS_MACHINECONFIGFUTURE

#include "lib3mftoolpath4eos_interfaces.hpp"
#include "libeos_eossdk.hpp"
#include "libeos_machinesessionhandle.hpp"
#include "libeos_namespaceregistry.hpp"

#include "../Common/common_workingdirectory.hpp"
#include "../Common/common_importstream.hpp"

// Parent classes
#include "libeos_future.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include <atomic>
#include <chrono>
#include <future>

namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CMachineConfigFuture 
**************************************************************************************************************************/

class CMachineConfigFuture : public virtual IMachineConfigFuture, public virtual CFuture {

protected:

    std::vector<uint8_t> m_StreamData;


    AMCCommon::PWorkingDirectory m_pMachineConfigTempDirectory;
    PMachineSessionHandle m_pSessionHandle;
    PMachineFileTransferHandle m_pDownloadConfigTransferHandle;
    
    std::chrono::steady_clock m_Clock;
    std::chrono::steady_clock::time_point m_StartTime;

    PEOSSDK m_pSDK;

    // Namespaces accepted when reading the package index. Writing always uses the current namespace.
    CNamespaceRegistry m_NamespaceRegistry;

    eFutureState m_ConfigState;
    // Delivers the package data. The download thread must not access this object, which can be released while it runs.
    std::future<std::vector<uint8_t>> m_ConfigDownloadFuture;
    std::shared_ptr<std::atomic<bool>> m_pAbortFlag;
    void updateConfigState();
    void requestAbort();

    static std::vector<uint8_t> buildPackageFromPath(const std::string& sDirectoryPath);
    static std::vector<uint8_t> buildSimulationPackage();

public:

    CMachineConfigFuture(PEOSSDK pEOSSDK, const CNamespaceRegistry& registry = CNamespaceRegistry());

    virtual ~CMachineConfigFuture();

    bool IsRunning() override;

    void Abort() override;

    bool HasFinished() override;

    bool HasFailed() override;

    bool HasBeenAborted() override;

	void StoreInDirectory(const std::string & sPath) override;

	void RetrievePackageData(Lib3MF4EOS_uint64 nStreamDataBufferSize, Lib3MF4EOS_uint64* pStreamDataNeededCount, Lib3MF4EOS_uint8 * pStreamDataBuffer) override;

	void StorePackageInFile(const std::string & sFileName) override;

    void loadFromArray(uint64_t nArraySize, const uint8_t* pData);

    void loadFromStream(AMCCommon::CImportStream& importStream);

    void loadFromEOSSession(PMachineSessionHandle pSessionHandle);

    void loadFromDirectory(const std::string& sDirectoryPath);

    void storeInWorkingDirectory(AMCCommon::PWorkingDirectory pDirectory);

    IEOSMachineInfo* ReadMachineInformation() override;

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_MACHINECONFIGFUTURE
