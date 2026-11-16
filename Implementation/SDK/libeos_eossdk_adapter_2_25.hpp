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


Abstract: EOS SDK adapter for version 2.25.

*/


#ifndef __LIBEOS_EOSSDK_ADAPTER_2_25
#define __LIBEOS_EOSSDK_ADAPTER_2_25

#if __has_include("libeos_eossdk_decl_2_25.hpp")

#include "libeos_eossdk_adapter_template.hpp"
#include "libeos_eossdk_decl_2_25.hpp"

namespace Lib3MF4EOS {
    namespace Impl {

        struct sEOSSDKAdapterTraits_2_25 {
            typedef SDK_2_25::sEntryPoints EntryPoints;
            typedef SDK_2_25::EOS_MACHINE_INFO MachineInfo;
            typedef SDK_2_25::EOS_MATERIALSET_INFO MaterialSetInfo;
            typedef SDK_2_25::EOS_TASK_METADATA TaskMetaData;
            typedef SDK_2_25::EOS_DATA_HANDLE DataHandle;
            typedef const SDK_2_25::EOS_BUILDTIMEINFO* BuildTimeCallbackArgument;
            typedef const SDK_2_25::EOS_EVENT* EventCallbackArgument;
        };

        class CEOSSDKAdapter_2_25 : public CEOSSDKAdapterTemplate<sEOSSDKAdapterTraits_2_25> {

        public:

            uint32_t getVersionMajor() const override;
            uint32_t getVersionMinor() const override;
            std::string getOpenJobVersion() const override;

            bool supportsProductVersion() const override;
            uint32_t getApiVersion() const override;
            bool versionHasBeamShape() const override;
            bool supportsBeamShape() const override;

            EOS_ERROR writeSetBeamShape(EOS_FILE_HANDLE hFileHandle, EOS_BEAMSHAPE beamShape, EOS_INT nProfileID) override;

        };

    } // namespace Impl
} // namespace Lib3MF4EOS

#endif // __has_include("libeos_eossdk_decl_2_25.hpp")

#endif // __LIBEOS_EOSSDK_ADAPTER_2_25
