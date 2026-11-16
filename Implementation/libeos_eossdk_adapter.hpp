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


Abstract: Abstract EOS SDK version adapter and the registry of known SDK versions.

*/


#ifndef __LIBEOS_EOSSDK_ADAPTER
#define __LIBEOS_EOSSDK_ADAPTER

#include "libeos_eossdk_types.hpp"

// The SDK 2.25 adapter is built once its declarations exist: put the SDK 2.25 headers in
// Libraries/EOSSDK/2.25/include/EosprintApi and run Scripts/eossdk_headers.py generate 2.25.
#if __has_include("SDK/libeos_eossdk_decl_2_25.hpp")
#define LIBEOS_SDK_2_25
#endif

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Lib3MF4EOS {
    namespace Impl {

        class CEOSSDKAdapter {
        private:
            std::mutex m_CallbackMutex;
            PEOSBuildTimeInfoCallback m_pBuildTimeInfoCallback;
            void* m_pBuildTimeInfoUserData;
            PEOSEventCallback m_pEventCallback;
            void* m_pEventUserData;

        protected:
            // Called by the version trampolines on the SDK's thread.
            void dispatchBuildTimeInfo(const sEOSBuildTimeInfo& buildTimeInfo);
            void dispatchEvent(const sEOSEvent& event);

            void storeBuildTimeInfoCallback(PEOSBuildTimeInfoCallback pCallback, void* pUserData);
            void storeEventCallback(PEOSEventCallback pCallback, void* pUserData);

        public:
            CEOSSDKAdapter();
            virtual ~CEOSSDKAdapter();

            virtual uint32_t getVersionMajor() const = 0;
            virtual uint32_t getVersionMinor() const = 0;
            std::string getVersionString() const;

            // OpenJob schema version written for this SDK version.
            virtual std::string getOpenJobVersion() const = 0;

            virtual bool supportsProductVersion() const = 0;
            // The value Eos_GetApiVersion returns for the SDK version of this adapter.
            virtual uint32_t getApiVersion() const = 0;
            // Whether the API of this SDK version has the beam shape record.
            virtual bool versionHasBeamShape() const = 0;
            // Whether the loaded library exports the beam shape record.
            virtual bool supportsBeamShape() const = 0;

            // Resolves the version-dependent entry points. A null library leaves all of them unresolved.
            virtual void loadEntryPoints(void* hLibrary) = 0;

            // Names of required version-dependent entry points that are not exported by the library.
            virtual std::vector<std::string> getMissingRequiredEntryPoints() const = 0;

            virtual EOS_ERROR loadMaterialSetInfo(const std::wstring& sPathToEOSPar, sEOSMaterialSetInfo& materialSetInfo) = 0;
            virtual EOS_ERROR loadMachineInfo(const std::wstring& sPathToConfigDirectory, sEOSMachineInfo& machineInfo) = 0;
            virtual EOS_ERROR getMachineInfo(sEOSMachineInfo& machineInfo) = 0;

            // Both need a loaded job and a loaded machine configuration.
            virtual EOS_ERROR getLayers(std::vector<sEOSLayerInfo>& layers) = 0;
            virtual EOS_ERROR getProcessHeights(sEOSProcessHeights& processHeights) = 0;

            virtual EOS_ERROR beginTaskCreation(const sEOSTaskMetaData& metaData, const std::wstring& sTargetDirectory) = 0;
            virtual bool supportsTaskCreationFromToolpath() const = 0;
            // The result of the last task creation; EosTaskGen_WaitForTaskCreation only says that it stopped.
            virtual EOS_ERROR getTaskCreationResult() = 0;
            virtual EOS_ERROR beginTaskCreationFromToolpath(EOS_FILE_HANDLE hFileHandle, const sEOSTaskMetaData& metaData, const std::wstring& sTargetDirectory) = 0;

            // Passing a null callback removes it.
            virtual EOS_ERROR setBuildTimeInfoCallback(PEOSBuildTimeInfoCallback pCallback, void* pUserData) = 0;
            virtual EOS_ERROR setEventCallback(PEOSEventCallback pCallback, void* pUserData) = 0;

            // Returns EOS_ERR_UNSUPPORTED if the SDK version has no beam shape support.
            virtual EOS_ERROR writeSetBeamShape(EOS_FILE_HANDLE hFileHandle, EOS_BEAMSHAPE beamShape, EOS_INT nProfileID) = 0;

        };

        typedef std::shared_ptr<CEOSSDKAdapter> PEOSSDKAdapter;


        struct sEOSSDKAdapterRegistration {
            uint32_t m_nMajor;
            uint32_t m_nMinor;
            std::function<PEOSSDKAdapter()> m_Factory;
        };

        class CEOSSDKAdapterRegistry {
        public:
            // Sorted ascending by version.
            static const std::vector<sEOSSDKAdapterRegistration>& getRegistrations();

            // Exact match, otherwise the nearest lower known version, which adds a message.
            // Returns null if the version is lower than the lowest known version.
            static PEOSSDKAdapter createAdapter(uint32_t nMajor, uint32_t nMinor, std::vector<std::string>& messages);

            static PEOSSDKAdapter createLowestAdapter();

            static bool isKnownVersion(uint32_t nMajor, uint32_t nMinor);

            // For example "2.18,2.26".
            static std::string getSupportedVersionsString();
        };

    } // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_EOSSDK_ADAPTER
