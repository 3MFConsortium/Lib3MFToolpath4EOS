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


Abstract: This is the class declaration of CMachineSessionHandle

*/


#ifndef __LIBEOS_MACHINESESSIONHANDLE
#define __LIBEOS_MACHINESESSIONHANDLE

#include "libeos_eossdk.hpp"

#include <mutex>
#include <map>

namespace Lib3MF4EOS {
    namespace Impl {


        /*************************************************************************************************************************
         Class declaration of CMachineConfig
        **************************************************************************************************************************/

        class CMachineFileTransferHandle {
        private:
            EOS_FILETRANSFER_HANDLE m_fileTransferHandle;
        public:
            CMachineFileTransferHandle(EOS_FILETRANSFER_HANDLE fileTransferHandle);
            virtual ~CMachineFileTransferHandle();

            EOS_FILETRANSFER_HANDLE getHandle();

        };

        typedef std::shared_ptr<CMachineFileTransferHandle> PMachineFileTransferHandle;

        class CMachineSessionHandle {
        private:
            PEOSSDK m_pEOSSDK;
            EOS_SESSION_HANDLE m_sessionHandle;

            std::map<EOS_FILETRANSFER_HANDLE, PMachineFileTransferHandle> m_HandleMap;
            std::map<EOS_FILETRANSFER_HANDLE, double> m_ProgressMap;
            std::mutex m_ProgressMutex;

        public:

            CMachineSessionHandle(PEOSSDK pEOSSDK, EOS_SESSION_HANDLE sessionHandle);
            ~CMachineSessionHandle();

            PEOSSDK getEOSSDK();
            EOS_SESSION_HANDLE getSessionHandle();

            void onProgress (EOS_FILETRANSFER_HANDLE handle, double dProgress);

            double getProgress(EOS_FILETRANSFER_HANDLE handle);

            PMachineFileTransferHandle registerFileTransfer(EOS_FILETRANSFER_HANDLE handle);

            void releaseFileTransfer(EOS_FILETRANSFER_HANDLE handle);
        };

        typedef std::shared_ptr<CMachineSessionHandle> PMachineSessionHandle;

    }
}

#endif // __LIBEOS_MACHINESESSIONHANDLE
