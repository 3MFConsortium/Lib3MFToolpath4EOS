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


Abstract: This is the class declaration of CEOSSDK. It loads the EOS SDK, detects its version,
selects the version adapter and keeps the entry points that are the same in all known versions.

*/


#ifndef __LIBEOS_EOSSDK
#define __LIBEOS_EOSSDK

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include "libeos_eossdksimulatorinstance.hpp"
#include "libeos_eossdk_types.hpp"
#include "libeos_eossdk_adapter.hpp"

namespace Lib3MF4EOS {
    namespace Impl {

        /*************************************************************************************************************************
         Entry points that are the same in all known SDK versions
        **************************************************************************************************************************/

        typedef EOS_ERROR(*PEos_GetAPIVersion) (EOS_UINT* apiVersion);
        typedef EOS_ERROR(*PEos_GetProductVersion) (EOS_VERSION* productVersion);
        typedef EOS_ERROR(*PEos_InitializeApi) (EOS_CWSTR pathToLoggingProperties);
        typedef EOS_ERROR(*PEos_DeinitializeApi)();


        typedef EOS_ERROR(*PEosTaskGen_LoadOpenJz) (EOS_CWSTR pathToOpenJz, EOS_CWSTR pathToTemporaryDirectory);
        typedef EOS_ERROR(*PEosTaskGen_UnloadJob) ();
        typedef EOS_BOOL(*PEosTaskGen_IsJobLoaded) ();
        typedef EOS_ERROR(*PEosTaskGen_BeginBuildTimeEstimation) ();
        typedef EOS_ERROR(*PEosTaskGen_AbortBuildTimeEstimation) ();
        typedef EOS_TASK_STATE(*PEosTaskGen_GetBuildTimeEstimationState) ();
        typedef EOS_ERROR(*PEosTaskGen_WaitForBuildTimeEstimation) (EOS_MILLISECS timeout);
        typedef EOS_MILLISECS(*PEosTaskGen_GetBuildTimeEstimation) ();

        typedef EOS_ERROR(*PEosTaskGen_AbortTaskCreation) ();
        typedef EOS_TASK_STATE(*PEosTaskGen_GetTaskCreationState) ();
        typedef EOS_ERROR(*PEosTaskGen_WaitForTaskCreation) (EOS_MILLISECS timeout);
        typedef EOS_ERROR(*PEosTaskGen_GetCurrentLayerIndex) (EOS_LAYERINDEX* layerIndex);
        typedef EOS_ERROR(*PEosTaskGen_GetStartLayerIndex) (EOS_LAYERINDEX* layerIndex);
        typedef EOS_ERROR(*PEosTaskGen_GetTargetLayerIndex) (EOS_LAYERINDEX* layerIndex);
        typedef EOS_ERROR(*PEosTaskGen_GetCurrentZHeight) (EOS_MICROMETER* zHeight);
        typedef EOS_ERROR(*PEosTaskGen_GetStartZHeight) (EOS_MICROMETER* zHeight);
        typedef EOS_ERROR(*PEosTaskGen_GetTargetZHeight) (EOS_MICROMETER* zHeight);

        typedef EOS_ERROR(*PEosTaskGen_LoadMachineConfiguration) (EOS_CWSTR pathToConfigDirectory);

        typedef EOS_ERROR(*PEosTaskGen_SetNumberOfTaskCreationThreads) (EOS_SIZET numberOfThreads);
        typedef EOS_ERROR(*PEosThreads_GetNumberOfAvailableThreads) (EOS_SIZET* numberOfThreads);

        typedef EOS_ERROR(*PEos_ResetAllCallbacks) ();
        typedef EOS_ERROR(*PEosTaskGen_UnloadMachineConfiguration) ();
        typedef EOS_BOOL(*PEosTaskGen_IsMachineConfigLoaded) ();
        typedef EOS_ERROR(*PEosTaskGen_SetCacheDirectory) (EOS_CWSTR pathToCacheDirectory);
        typedef EOS_ERROR(*PEosTaskGen_SetStateChangeCallback) (EOS_STATE_CHANGE_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);
        typedef EOS_ERROR(*PEosTaskGen_SetProgressCallback) (EOS_GENERATION_PROGRESS_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);
        typedef EOS_ERROR(*PEosTaskGen_SetBuildTimeStateChangedCallback) (EOS_STATE_CHANGE_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);


        typedef EOS_ERROR(*PEosMachineSession_Create) (EOS_CWSTR networkAddress, EOS_CWSTR pathToCertificateDirectory, EOS_CWSTR pathToTemporaryDirectory, EOS_SESSION_HANDLE* sessionHandle);

        typedef EOS_ERROR(*PEosMachineSession_CreateWithHint) (EOS_CWSTR networkAddress, EOS_CWSTR pathToCertificateDirectory, EOS_CWSTR pathToTemporaryDirectory, EOS_MODEL_TYPE modelTypeHint, EOS_SESSION_HANDLE* sessionHandle);

        typedef EOS_ERROR(*PEosMachineSession_Destroy) (EOS_SESSION_HANDLE sessionHandle);

        typedef EOS_ERROR(*PEosMachineSession_BeginConfigDownload) (EOS_SESSION_HANDLE sessionHandle, EOS_CWSTR pathToLocalConfigDirectory, EOS_FILETRANSFER_HANDLE* fileTransferHandle);

        typedef EOS_ERROR(*PEosMachineSession_BeginTaskUpload) (EOS_SESSION_HANDLE sessionHandle, EOS_CWSTR pathToLocalTaskOrDataFile, EOS_FILETRANSFER_HANDLE* fileTransferHandle);

        typedef EOS_ERROR(*PEosMachineSession_AbortFileTransfer) (EOS_FILETRANSFER_HANDLE fileTransferHandle);

        typedef EOS_ERROR(*PEosMachineSession_AbortAllFileTransfers) (EOS_SESSION_HANDLE sessionHandle);

        typedef EOS_TRANSFER_STATUS(*PEosMachineSession_GetFileTransferState) (EOS_FILETRANSFER_HANDLE fileTransferHandle);

        typedef EOS_ERROR(*PEosMachineSession_WaitForFileTransfer) (EOS_FILETRANSFER_HANDLE fileTransferHandle, EOS_MILLISECS timeout);

        typedef EOS_ERROR(*PEosMachineSession_GetFileTransferResult) (EOS_FILETRANSFER_HANDLE fileTransferHandle);

        typedef EOS_ERROR(*PEosMachineSession_WaitForAllFileTransfers) (EOS_SESSION_HANDLE sessionHandle, EOS_MILLISECS timeout);

        typedef EOS_ERROR(*PEosMachineSession_ReleaseFileTransferHandle) (EOS_FILETRANSFER_HANDLE fileTransferHandle);

        typedef EOS_ERROR(*PEosMachineSession_SetFileTransferProgressCallback) (EOS_SESSION_HANDLE sessionHandle, EOS_FILETRANSFER_PROGRESS_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);

        typedef EOS_ERROR(*PEosToolpath_GetAuthChallenge) (EOS_BYTE** challenge, EOS_ULONG* challengeLength);

        typedef EOS_ERROR(*PEosToolpath_SetAuthResponse) (EOS_UINT client, EOS_BYTE const* response, EOS_ULONG responseLength);

        typedef EOS_ERROR(*PEosToolpath_OpenOutputFile) (EOS_CWSTR filePath, EOS_FILE_HANDLE* fileHandle);

        typedef EOS_ERROR(*PEosToolpath_ChopOutput) (EOS_FILE_HANDLE fileHandle);

        typedef EOS_ERROR(*PEosToolpath_OpenInputFile) (EOS_CWSTR filePath, EOS_FILE_HANDLE* fileHandle);

        typedef EOS_ERROR(*PEosToolpath_CloseFile) (EOS_FILE_HANDLE fileHandle);

        typedef EOS_ERROR(*PEosToolpath_ReadLayer) (EOS_FILE_HANDLE fileHandle, EOS_DATA_HANDLE customDataHandle, EOS_TOOLPATH_READING_ORDER readingOrder, EOS_LAYERINDEX layerIndex);

        typedef EOS_ERROR(*PEosToolpath_GetNumberOfLayers) (EOS_FILE_HANDLE fileHandle, EOS_ULONG* layerCount, EOS_LAYERINDEX* firstLayer);

        typedef EOS_ERROR(*PEosToolpath_WriteBeginLayer) (EOS_FILE_HANDLE fileHandle, EOS_MICROMETER z_um);

        typedef EOS_ERROR(*PEosToolpath_WriteBeginPart) (EOS_FILE_HANDLE fileHandle, EOS_INT partId);

        typedef EOS_ERROR(*PEosToolpath_WriteSelectLaserScanner) (EOS_FILE_HANDLE fileHandle, EOS_BYTE laserScannerIndex);

        typedef EOS_ERROR(*PEosToolpath_WriteSetLaserPower) (EOS_FILE_HANDLE fileHandle, EOS_DOUBLE power_W, EOS_MICROSECS delay_us);

        typedef EOS_ERROR(*PEosToolpath_WriteSetScanSpeed) (EOS_FILE_HANDLE fileHandle, EOS_DOUBLE speed_mm_per_s);

        typedef EOS_ERROR(*PEosToolpath_WriteSetFocus) (EOS_FILE_HANDLE fileHandle, EOS_DOUBLE focus);

        typedef EOS_ERROR(*PEosToolpath_WriteSetSkywriting) (EOS_FILE_HANDLE fileHandle, EOS_BOOL skywritingEnabled);

        typedef EOS_ERROR(*PEosToolpath_WriteCustomData) (EOS_FILE_HANDLE fileHandle, EOS_CWSTR customData);

        typedef EOS_ERROR(*PEosToolpath_WriteHatch) (EOS_FILE_HANDLE fileHandle, EOS_POINT2 start, EOS_POINT2 end);

        typedef EOS_ERROR(*PEosToolpath_WritePolyStart) (EOS_FILE_HANDLE fileHandle, EOS_POINT2 startCoord);

        typedef EOS_ERROR(*PEosToolpath_WritePolyCoord) (EOS_FILE_HANDLE fileHandle, EOS_POINT2 intermediateCoord, EOS_MICROSECS trackingErrorCompensation);

        typedef EOS_ERROR(*PEosToolpath_WritePolyEnd) (EOS_FILE_HANDLE fileHandle, EOS_POINT2 endCoord);

        typedef EOS_ERROR(*PEosToolpath_WriteDelay) (EOS_FILE_HANDLE fileHandle, EOS_MICROSECS delay_us);

        typedef EOS_ERROR(*PEosToolpath_WriteSetExposureType) (EOS_FILE_HANDLE fileHandle, EOS_EXPOSURE_TYPE exposureType);

        typedef EOS_ERROR(*PEosToolpath_WriteSetExposureStep) (EOS_FILE_HANDLE fileHandle, EOS_UINT exposureSetIndex, EOS_UINT exposureStepIndex);

        typedef EOS_ERROR(*PEosToolpath_WriteSetDepth) (EOS_FILE_HANDLE fileHandle, EOS_MICROMETER exposedDepthInMicrometers);

        typedef EOS_ERROR(*PEosToolpath_WriteStartStripeSegment) (EOS_FILE_HANDLE fileHandle, EOS_POINT2 stripeBegin, EOS_POINT2 hatchDistVector);

        typedef EOS_ERROR(*PEosToolpath_WriteStartGeneralSegment) (EOS_FILE_HANDLE fileHandle, EOS_POINT2 hatchDistVector);

        typedef EOS_ERROR(*PEosToolpath_WriteEndSegment) (EOS_FILE_HANDLE fileHandle);

        typedef EOS_ERROR(*PEosToolpath_WriteEndLine) (EOS_FILE_HANDLE fileHandle);

        typedef EOS_ERROR(*PEosToolpath_WriteSetLaserPulse) (EOS_FILE_HANDLE fileHandle, EOS_MICROSECS pulsedWavePeriodTime_us, EOS_MICROSECS pulsedWavePowerOnTime_us);

        typedef EOS_ERROR(*PEosToolpath_SetBeginLayerCallback) (EOS_BEGINLAYER_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetEndLayerCallback) (EOS_ENDLAYER_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetBeginPartCallback) (EOS_BEGINPART_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSelectLaserScannerCallback) (EOS_SELECTLASERSCANNERUNIT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetLaserPowerCallback) (EOS_SETLASERPOWER_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetScanSpeedCallback) (EOS_SETSCANSPEED_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetFocusCallback) (EOS_SETFOCUS_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetSkywritingCallback) (EOS_SETSKYWRITING_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetCustomDataCallback) (EOS_CUSTOMDATA_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetHatchCallback) (EOS_HATCH_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetPolyStartCallback) (EOS_POLYSTART_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetPolyCoordCallback) (EOS_POLYCOORD_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetPolyEndCallback) (EOS_POLYEND_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetDelayCallback) (EOS_DELAY_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetExposureTypeCallback) (EOS_SETEXPOSURETYPE_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetExposureStepCallback) (EOS_SETEXPOSURESTEP_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetDepthCallback) (EOS_SETDEPTH_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetStartStripeSegmentCallback) (EOS_STARTSTRIPESEGMENT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetStartGeneralSegmentCallback) (EOS_STARTGENERALSEGMENT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetEndSegmentCallback) (EOS_ENDSEGMENT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetEndLineCallback) (EOS_ENDLINE_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

        typedef EOS_ERROR(*PEosToolpath_SetSetLaserPulseCallback) (EOS_SETLASERPULSE_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);


        enum class eEOSSDKVersionSource {
            None = 0,
            ProductVersionFunction = 1,
            VersionResource = 2
        };


        /*************************************************************************************************************************
         Class declaration of CEOSSDK
        **************************************************************************************************************************/

        class CEOSSDK {
        private:

        protected:
            std::string m_sSDKDirectory;
            void* m_hLibraryHandle;

            PEOSSDKSimulatorInstance m_pSimulatorInstance;

            PEOSSDKAdapter m_pAdapter;
            sEOSSDKVersion m_ProductVersion;
            eEOSSDKVersionSource m_ProductVersionSource;
            std::vector<std::string> m_VersionMessages;

            void resetPtrs();
            void loadSDK(const std::string& sEOSPrintAPIDLLName);
            void detectProductVersion();
            void selectAdapter(uint32_t nOverrideMajor, uint32_t nOverrideMinor);

        public:

            PEos_GetAPIVersion Eos_GetApiVersion;
            PEos_GetProductVersion Eos_GetProductVersion;
            PEos_InitializeApi Eos_InitializeApi;
            PEos_DeinitializeApi Eos_DeinitializeApi;

            PEosTaskGen_LoadOpenJz EosTaskGen_LoadOpenJz;
            PEosTaskGen_UnloadJob EosTaskGen_UnloadJob;
            PEosTaskGen_IsJobLoaded EosTaskGen_IsJobLoaded;
            PEosTaskGen_BeginBuildTimeEstimation EosTaskGen_BeginBuildTimeEstimation;
            PEosTaskGen_AbortBuildTimeEstimation EosTaskGen_AbortBuildTimeEstimation;
            PEosTaskGen_GetBuildTimeEstimationState EosTaskGen_GetBuildTimeEstimationState;
            PEosTaskGen_WaitForBuildTimeEstimation EosTaskGen_WaitForBuildTimeEstimation;
            PEosTaskGen_GetBuildTimeEstimation EosTaskGen_GetBuildTimeEstimation;
            PEosTaskGen_AbortTaskCreation EosTaskGen_AbortTaskCreation;
            PEosTaskGen_GetTaskCreationState EosTaskGen_GetTaskCreationState;
            PEosTaskGen_WaitForTaskCreation EosTaskGen_WaitForTaskCreation;
            PEosTaskGen_GetCurrentLayerIndex EosTaskGen_GetCurrentLayerIndex;
            PEosTaskGen_GetStartLayerIndex EosTaskGen_GetStartLayerIndex;
            PEosTaskGen_GetTargetLayerIndex EosTaskGen_GetTargetLayerIndex;
            PEosTaskGen_GetCurrentZHeight EosTaskGen_GetCurrentZHeight;
            PEosTaskGen_GetStartZHeight EosTaskGen_GetStartZHeight;
            PEosTaskGen_GetTargetZHeight EosTaskGen_GetTargetZHeight;
            PEosTaskGen_SetNumberOfTaskCreationThreads EosTaskGen_SetNumberOfTaskCreationThreads;
            PEosTaskGen_LoadMachineConfiguration EosTaskGen_LoadMachineConfiguration;

            PEosThreads_GetNumberOfAvailableThreads EosThreads_GetNumberOfAvailableThreads;

            PEos_ResetAllCallbacks Eos_ResetAllCallbacks;
            PEosTaskGen_UnloadMachineConfiguration EosTaskGen_UnloadMachineConfiguration;
            PEosTaskGen_IsMachineConfigLoaded EosTaskGen_IsMachineConfigLoaded;
            PEosTaskGen_SetCacheDirectory EosTaskGen_SetCacheDirectory;
            PEosTaskGen_SetStateChangeCallback EosTaskGen_SetStateChangeCallback;
            PEosTaskGen_SetProgressCallback EosTaskGen_SetProgressCallback;
            PEosTaskGen_SetBuildTimeStateChangedCallback EosTaskGen_SetBuildTimeStateChangedCallback;

            PEosMachineSession_Create EosMachineSession_Create;
            PEosMachineSession_CreateWithHint EosMachineSession_CreateWithHint;
            PEosMachineSession_Destroy EosMachineSession_Destroy;
            PEosMachineSession_BeginConfigDownload EosMachineSession_BeginConfigDownload;
            PEosMachineSession_BeginTaskUpload EosMachineSession_BeginTaskUpload;
            PEosMachineSession_AbortFileTransfer EosMachineSession_AbortFileTransfer;
            PEosMachineSession_AbortAllFileTransfers EosMachineSession_AbortAllFileTransfers;
            PEosMachineSession_GetFileTransferState EosMachineSession_GetFileTransferState;
            PEosMachineSession_WaitForFileTransfer EosMachineSession_WaitForFileTransfer;
            PEosMachineSession_GetFileTransferResult EosMachineSession_GetFileTransferResult;
            PEosMachineSession_WaitForAllFileTransfers EosMachineSession_WaitForAllFileTransfers;
            PEosMachineSession_ReleaseFileTransferHandle EosMachineSession_ReleaseFileTransferHandle;
            PEosMachineSession_SetFileTransferProgressCallback EosMachineSession_SetFileTransferProgressCallback;

            PEosToolpath_GetAuthChallenge EosToolpath_GetAuthChallenge;
            PEosToolpath_SetAuthResponse EosToolpath_SetAuthResponse;
            PEosToolpath_OpenOutputFile EosToolpath_OpenOutputFile;
            PEosToolpath_ChopOutput EosToolpath_ChopOutput;
            PEosToolpath_OpenInputFile EosToolpath_OpenInputFile;
            PEosToolpath_CloseFile EosToolpath_CloseFile;
            PEosToolpath_ReadLayer EosToolpath_ReadLayer;
            PEosToolpath_GetNumberOfLayers EosToolpath_GetNumberOfLayers;
            PEosToolpath_WriteBeginLayer EosToolpath_WriteBeginLayer;
            PEosToolpath_WriteBeginPart EosToolpath_WriteBeginPart;
            PEosToolpath_WriteSelectLaserScanner EosToolpath_WriteSelectLaserScanner;
            PEosToolpath_WriteSetLaserPower EosToolpath_WriteSetLaserPower;
            PEosToolpath_WriteSetScanSpeed EosToolpath_WriteSetScanSpeed;
            PEosToolpath_WriteSetFocus EosToolpath_WriteSetFocus;
            PEosToolpath_WriteSetSkywriting EosToolpath_WriteSetSkywriting;
            PEosToolpath_WriteCustomData EosToolpath_WriteCustomData;
            PEosToolpath_WriteHatch EosToolpath_WriteHatch;
            PEosToolpath_WritePolyStart EosToolpath_WritePolyStart;
            PEosToolpath_WritePolyCoord EosToolpath_WritePolyCoord;
            PEosToolpath_WritePolyEnd EosToolpath_WritePolyEnd;
            PEosToolpath_WriteDelay EosToolpath_WriteDelay;
            PEosToolpath_WriteSetExposureType EosToolpath_WriteSetExposureType;
            PEosToolpath_WriteSetExposureStep EosToolpath_WriteSetExposureStep;
            PEosToolpath_WriteSetDepth EosToolpath_WriteSetDepth;
            PEosToolpath_WriteStartStripeSegment EosToolpath_WriteStartStripeSegment;
            PEosToolpath_WriteStartGeneralSegment EosToolpath_WriteStartGeneralSegment;
            PEosToolpath_WriteEndSegment EosToolpath_WriteEndSegment;
            PEosToolpath_WriteEndLine EosToolpath_WriteEndLine;
            PEosToolpath_WriteSetLaserPulse EosToolpath_WriteSetLaserPulse;
            PEosToolpath_SetBeginLayerCallback EosToolpath_SetBeginLayerCallback;
            PEosToolpath_SetEndLayerCallback EosToolpath_SetEndLayerCallback;
            PEosToolpath_SetBeginPartCallback EosToolpath_SetBeginPartCallback;
            PEosToolpath_SetSelectLaserScannerCallback EosToolpath_SetSelectLaserScannerCallback;
            PEosToolpath_SetSetLaserPowerCallback EosToolpath_SetSetLaserPowerCallback;
            PEosToolpath_SetSetScanSpeedCallback EosToolpath_SetSetScanSpeedCallback;
            PEosToolpath_SetSetFocusCallback EosToolpath_SetSetFocusCallback;
            PEosToolpath_SetSetSkywritingCallback EosToolpath_SetSetSkywritingCallback;
            PEosToolpath_SetCustomDataCallback EosToolpath_SetCustomDataCallback;
            PEosToolpath_SetHatchCallback EosToolpath_SetHatchCallback;
            PEosToolpath_SetPolyStartCallback EosToolpath_SetPolyStartCallback;
            PEosToolpath_SetPolyCoordCallback EosToolpath_SetPolyCoordCallback;
            PEosToolpath_SetPolyEndCallback EosToolpath_SetPolyEndCallback;
            PEosToolpath_SetDelayCallback EosToolpath_SetDelayCallback;
            PEosToolpath_SetSetExposureTypeCallback EosToolpath_SetSetExposureTypeCallback;
            PEosToolpath_SetSetExposureStepCallback EosToolpath_SetSetExposureStepCallback;
            PEosToolpath_SetSetDepthCallback EosToolpath_SetSetDepthCallback;
            PEosToolpath_SetStartStripeSegmentCallback EosToolpath_SetStartStripeSegmentCallback;
            PEosToolpath_SetStartGeneralSegmentCallback EosToolpath_SetStartGeneralSegmentCallback;
            PEosToolpath_SetEndSegmentCallback EosToolpath_SetEndSegmentCallback;
            PEosToolpath_SetEndLineCallback EosToolpath_SetEndLineCallback;
            PEosToolpath_SetSetLaserPulseCallback EosToolpath_SetSetLaserPulseCallback;


            // An empty DLL name creates an SDK without a library, which is only usable in full SDK simulation.
            // An override of 0.0 uses the detected version.
            CEOSSDK(const std::string & sSDKDirectory, const std::string& sEOSPrintAPIDLLName, uint32_t nOverrideMajor, uint32_t nOverrideMinor);
            virtual ~CEOSSDK();

            void UnloadSDK();

            std::string getSDKDirectory();

            bool hasLibrary();

            CEOSSDKAdapter& getAdapter();

            // Empty if the product version could not be detected.
            sEOSSDKVersion getProductVersion();
            eEOSSDKVersionSource getProductVersionSource();

            const std::vector<std::string>& getVersionMessages();

            void checkEOSError(EOS_ERROR eosError);

            void setToSimulationMode(eSimulationMode simulationMode);

            bool simulateConnectivity();
            bool simulateFullSDK();
            bool simulatePlastics();

            // Null unless toolpath writers record their commands.
            PToolpathRecorder getToolpathRecorder();

            // Whether toolpath writers can write the beam shape. A recording follows the SDK version,
            // otherwise the loaded library must export it.
            bool canWriteBeamShape();

            CEOSSDKSimulatorInstance* getSimulator();


        };

        typedef std::shared_ptr<CEOSSDK> PEOSSDK;

        std::string eosErrorToString(EOS_ERROR errorCode);

    } // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSSDK
