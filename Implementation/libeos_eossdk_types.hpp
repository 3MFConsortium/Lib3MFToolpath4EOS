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


Abstract: Version-neutral model of the EOS SDK. Scalars, handles and the structures of the
invariant entry points are binary identical in all supported SDK versions. Enums are supersets
of all supported versions. Structures that differ between versions are modelled as plain
C++ structures, which the version adapters convert from and to.

*/


#ifndef __LIBEOS_EOSSDK_TYPES
#define __LIBEOS_EOSSDK_TYPES

#include <cstdint>
#include <string>
#include <vector>

namespace Lib3MF4EOS {
    namespace Impl {

        typedef int32_t EOS_INT;
        typedef uint32_t EOS_UINT;
        typedef const wchar_t* EOS_CWSTR;
        typedef int32_t EOS_MICROMETER;
        typedef uint64_t EOS_LAYERINDEX;
        typedef uint64_t EOS_MILLISECS;
        typedef double EOS_MICROSECS;
        typedef uint64_t EOS_TIMESTAMP;
        typedef uint8_t EOS_BYTE;
        typedef uint64_t EOS_SIZET;
        typedef uint64_t EOS_ULONG;
        typedef uint32_t EOS_LICENSABLE_FEATURES;
        typedef uint64_t EOS_HANDLE;
        typedef uint64_t EOS_DATA_HANDLE;
        typedef uint64_t EOS_SESSION_HANDLE;
        typedef uint64_t EOS_FILE_HANDLE;
        typedef uint64_t EOS_FILETRANSFER_HANDLE;
        typedef double EOS_MILLIMETER;
        typedef double EOS_DOUBLE;
        typedef int32_t EOS_MODULE_ID;
        typedef int32_t EOS_EVENT_ID;

        typedef enum
        {
            EOS_ERR_NO_ERROR = 1,
            EOS_ERR_ARGUMENT_INVALID = 2,
            EOS_ERR_ENCODING_CONVERSION_ERROR = 3,
            EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS = 4,
            EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS = 5,
            EOS_ERR_MISSING_LICENSE = 6,
            EOS_ERR_EMPTY = 7,
            EOS_ERR_NO_MATCH = 8,
            EOS_ERR_NOT_A_DIRECTORY = 9,
            EOS_ERR_FILE_NOT_FOUND = 10,
            EOS_ERR_ILLEGAL_FILE_EXTENSION = 11,
            EOS_ERR_FILESYSTEM_IO_ERROR = 12,
            EOS_ERR_NETWORK_IO_ERROR = 13,
            EOS_ERR_OUT_OF_MEMORY = 14,
            EOS_ERR_MACHINE_NOT_SUPPORTED = 15,
            EOS_ERR_IDLE = 16,
            EOS_ERR_NOT_IDLE = 17,
            EOS_ERR_FILETRANSFER_INVALID = 18,
            EOS_ERR_FILETRANSFER_RUNNING = 19,
            EOS_ERR_FILETRANSFER_NOT_RUNNING = 20,
            EOS_ERR_FILETRANSFER_ABORTED = 21,
            EOS_ERR_FILETRANSFER_FAILED = 22,
            EOS_ERR_SESSION_INVALID = 23,
            EOS_ERR_TOO_MANY_SESSIONS = 24,
            EOS_ERR_BUILDTIME_ESTIMATION_RUNNING = 25,
            EOS_ERR_BUILDTIME_ESTIMATION_NOT_RUNNING = 26,
            EOS_ERR_BUILDTIME_ESTIMATION_ABORTED = 27,
            EOS_ERR_PREVIEW_RUNNING = 28,
            EOS_ERR_PREVIEW_NOT_RUNNING = 29,
            EOS_ERR_PREVIEW_ABORTED = 30,
            EOS_ERR_LAYER_INVALID = 31,
            EOS_ERR_PREVIEW_LAYER_INVALID = EOS_ERR_LAYER_INVALID,
            EOS_ERR_PREVIEW_ZHEIGHT_INVALID = 32,
            EOS_ERR_TASK_CREATION_RUNNING = 33,
            EOS_ERR_TASK_CREATION_NOT_RUNNING = 34,
            EOS_ERR_TASK_CREATION_ABORTED = 35,
            EOS_ERR_TASK_WRITE_FAILED = 36,
            EOS_ERR_TASK_INVALID = 37,
            EOS_ERR_TASK_CORRUPT = 38,
            EOS_ERR_JOB_NOT_LOADED = 39,
            EOS_ERR_CONFIG_NOT_LOADED = 40,
            EOS_ERR_CONFIG_INVALID = 41,
            EOS_ERR_CONFIG_CORRUPT = 42,
            EOS_ERR_ILLEGAL_CONFIG_DIR = 43,
            EOS_ERR_OPENJOB_NOT_FOUND = 44,
            EOS_ERR_OPENJOB_NOT_SUPPORTED = 45,
            EOS_ERR_OPENJOB_INVALID = 46,
            EOS_ERR_OPENJOB_CORRUPT = 47,
            EOS_ERR_OPENJZ_INVALID = 48,
            EOS_ERR_OPENJZ_CORRUPT = 49,
            EOS_ERR_EOSPAR_NOT_FOUND = 50,
            EOS_ERR_EOSPAR_UNSUPPORTED = 51,
            EOS_ERR_EOSPAR_INVALID = 52,
            EOS_ERR_EOSPAR_CORRUPT = 53,
            EOS_ERR_DIRECTORY_NOT_EMPTY = 54,
            EOS_ERR_WAIT_TIMED_OUT = 55,
            EOS_ERR_INVALID_CODE_PAGE = 56,
            EOS_ERR_INCORRECT_ENCODING = 57,
            EOS_ERR_PART_FILE_NOT_FOUND = 58,
            EOS_ERR_MACHINE_VERSION_NOT_SUPPORTED = 59,
            EOS_ERR_FILECREATION_INVALID = 60,
            EOS_ERR_FILECREATION_NOT_RUNNING = 61,
            EOS_ERR_FILECREATION_ABORTED = 62,
            EOS_ERR_STL_INVALID = 63,
            EOS_ERR_DEFORMATION_ERROR = 64,
            EOS_ERR_DOMAIN_ERROR = 65,
            EOS_ERR_OVERFLOW_ERROR = 66,
            EOS_ERR_STL_BELOW_PLATFORM = 67,
            EOS_ERR_STL_OUTSIDE_BUILD_VOLUME = 68,
            EOS_ERR_STL_ABOVE_JOB_HEIGHT = 69,
            EOS_ERR_STL_CONTAINS_NEGATIVE_COORDINATES = 70,
            EOS_ERR_INVALID_PART_ID = 71,
            EOS_ERR_INVALID_EXPOSURE_SET_NAME = 72,
            EOS_ERR_FEATURE_NOT_SUPPORTED = 73,
            EOS_ERR_TASK_UNSUPPORTED_VERSION = 74,
            EOS_ERR_ADJUSTED_VALUE_VIOLATES_THRESHOLD = 75,
            EOS_ERR_ADJUSTMENT_ID_INVALID = 76,
            EOS_ERR_ADJUSTMENT_TYPE_NOT_ENABLED = 77,
            EOS_ERR_ADJUSTMENT_NOT_SUPPORTED_BY_MACHINE = 78,
            EOS_ERR_ADJUSTMENT_HAS_NO_EFFECT_ON_EXPOSURE = 79,
            EOS_ERR_NOT_IMPLEMENTED = 80,
            EOS_ERR_LOADING_DLL_FAILED = 81,
            EOS_ERR_CONSTRAINT_VIOLATED = 82,
            EOS_ERR_DEFOCUS_NOT_SUPPORTED = 83,
            EOS_ERR_PROTOCOL_ERROR = 84,
            EOS_ERR_FILECREATION_RUNNING = 85,
            EOS_ERR_FILE_HANDLE_INVALID = 86,
            EOS_ERR_INVALID_LAYER = 87,
            EOS_ERR_FAULTY_TRIANGLE_MESH = 88,
            EOS_ERR_CANNOT_WRITE_TO_INPUT_FILE = 89,
            EOS_ERR_NO_LAYER = 90,
            EOS_ERR_NO_SCANNER = 91,
            EOS_ERR_NO_PART = 92,
            EOS_ERR_LAYER = 93,
            EOS_ERR_PART = 94,
            EOS_ERR_LSU_INVALID = 95,
            EOS_ERR_UNSUPPORTED = 96,
            EOS_ERR_BAD_COORDINATE = 97,
            EOS_ERR_COMMAND_NOT_POSSIBLE = 98,
            EOS_ERR_CANNOT_UNLOAD = 99,
            EOS_ERR_INSUFFICIENT_SIZE = 100,
            EOS_ERR_NOT_ALLOWED = 101,
            EOS_ERR_CLIENT_NOT_AUTHENTICATED = 102,
            EOS_ERR_INVALID_EXPOSURE_PARAMETERS = 103,
            EOS_ERR_RESOURCECACHE_EXTERNAL_IMPORT_ERROR = 104,
            EOS_ERR_RESOURCECACHE_EXTERNAL_MESH_ERROR = 105,
            EOS_ERR_RESOURCECACHE_EXTERNAL_CONTOUR_ERROR = 106,
            EOS_ERR_RESOURCECACHE_EXTERNAL_IMPORT_WARNING = 107,
            EOS_ERR_RESOURCECACHE_EXTERNAL_MESH_WARNING = 108,
            EOS_ERR_RESOURCECACHE_EXTERNAL_CONTOUR_WARNING = 109,
            EOS_ERR_FILE_MALFORMED = 110,
            EOS_ERR_PARAMETER_DOES_NOT_EXIST = 111,
            EOS_ERR_PARAMETER_INACCESSIBLE = 112,
            EOS_ERR_INVALID_PARAMETER_VALUE = 113,
            EOS_ERR_ID_CONFLICT = 114,
            EOS_ERR_FILE_CORRUPT = 115,
            EOS_ERR_INSUFFICIENT_DISK_SPACE = 116,
            EOS_ERR_DATA_FORMAT_UNSUPPORTED = 117,
            EOS_ERR_EOSSIM_INVALID = 118,
            EOS_ERR_EOSSIM_CORRUPT = 119,
            EOS_ERR_INTERNAL_ERROR = 32766,
            EOS_ERR_UNEXPECTED_ERROR = 32767

        } EOS_ERROR;

        typedef struct
        {
            EOS_INT major;
            EOS_INT minor;
            EOS_INT build;
            EOS_INT revision;
        } EOS_VERSION;

        typedef enum
        {
            EOS_FALSE = 0,
            EOS_TRUE = 1
        } EOS_BOOL;

        typedef enum
        {
            EOS_LS_CODEMETER = 1

        } EOS_LICENSING_SYSTEM;

        typedef enum
        {
            EOS_TG_IDLE = 0,
            EOS_TG_RUNNING = 1,
            EOS_TG_SUSPENDED = 2,
            EOS_TG_COMPLETED = 3,
            EOS_TG_FAILED = 4,
            EOS_TG_ABORTED = 5

        } EOS_TASK_STATE;

        typedef enum
        {
            EOS_TS_INVALID_HANDLE = 0,
            EOS_TS_RUNNING = 1,
            EOS_TS_QUEUED = 2,
            EOS_TS_COMPLETED = 3,
            EOS_TS_FAILED = 4,
            EOS_TS_ABORTED = 5
        } EOS_TRANSFER_STATUS;

        typedef enum
        {
            EOS_MT_M290 = 1,
            EOS_MT_M400 = 2,
            EOS_MT_M404 = 3,
            EOS_MT_P500 = 4,
            EOS_MT_M304 = 5,
            EOS_MT_P900 = 6,
            EOS_MT_P396 = 7,
            EOS_MT_P420 = 8,
            EOS_MT_P711 = 9,
            EOS_MT_M100 = 10,
            EOS_MT_P120 = 11,
            EOS_MT_M292 = 12,
            EOS_MT_P520 = 13,
            EOS_MT_M293 = 14,
            EOS_MT_P110 = 15,
            EOS_MT_M294 = 17,
            EOS_MT_M314 = 18,
            EOS_MT_P397 = 20,
            EOS_MT_M808 = 21,
            EOS_MT_P501 = 22,
            EOS_MT_P521 = 23,
            EOS_MT_M450 = 25,
            EOS_MT_M451 = 27,
            EOS_MT_M405 = 28
        } EOS_MODEL_TYPE;

        typedef enum
        {
            EOS_VOL_CUBOID = 1,
            EOS_VOL_CYLINDER = 2
        } EOS_VOLUME_TYPE;

        typedef enum
        {
            EOS_SEV_WARNING = 0,
            EOS_SEV_ERROR = 1,
            EOS_SEV_FATAL = 2
        } EOS_SEVERITY;

        typedef enum
        {
            EOS_VAR_INT = 0,
            EOS_VAR_DOUBLE = 1,
            EOS_VAR_CWSTR = 2,
        } EOS_VARIANT_TYPE;

        typedef enum
        {
            EOS_IG_NONE = 0,
            EOS_IG_NITROGEN = 1,
            EOS_IG_ARGON = 2
        } EOS_INERT_GAS;

        typedef enum
        {
            EOS_DSP_NONE = 0,
            EOS_DSP_MODAL_WINDOW = 1,
            EOS_DSP_BALLOON = 2
        } EOS_DISPLAY_OPTIONS;

        typedef enum
        {
            EOS_EXPOSURE_TYPE_PARTBOUNDARY = 0,
            EOS_EXPOSURE_TYPE_DOWNSKIN = 1,
            EOS_EXPOSURE_TYPE_INFILL = 2,
            EOS_EXPOSURE_TYPE_UPSKIN = 3,
            EOS_EXPOSURE_TYPE_DOWNSKIN_CONTOUR = 4,
            EOS_EXPOSURE_TYPE_CONTOUR = 5,
            EOS_EXPOSURE_TYPE_UPSKIN_CONTOUR = 6,
            EOS_EXPOSURE_TYPE_EDGE = 7,
            EOS_EXPOSURE_TYPE_SUPPORT = 8,
            EOS_EXPOSURE_TYPE_HATCH_GAP = 9,
            EOS_EXPOSURE_TYPE_CONTOUR_SIMPLE = 10,
            EOS_EXPOSURE_TYPE_JUMP = 11,
            EOS_EXPOSURE_TYPE_SUPPORT_EXPOSED = 12,
            EOS_EXPOSURE_TYPE_UPDOWNCONTOUR = 13,
            EOS_EXPOSURE_TYPE_CORE = 14,
            EOS_EXPOSURE_TYPE_HATCH = 15,
            EOS_EXPOSURE_TYPE_HIDDEN = 16

        } EOS_EXPOSURE_TYPE;

        typedef enum
        {
            EOS_TRO_BY_LASER_THEN_BY_TIME_INDEX = 0,
            EOS_TRO_BY_TIME_INDEX = 1
        } EOS_TOOLPATH_READING_ORDER;

        typedef enum
        {
            EOS_TM_REGULAR = 0,
            EOS_TM_SINGLELAYER = 1
        } EOS_TASKMODE;

        typedef enum
        {
            EOS_TF_REGULAR = 0,
            EOS_TF_RAW = 1
        } EOS_TASKFORMAT;

        typedef enum
        {
            EOS_CL_DEFAULT_COMPRESSION = 0,
            EOS_CL_BEST_SPEED = 1,
            EOS_CL_HIGH_SPEED = 2,
            EOS_CL_NORMAL = 3,
            EOS_CL_HIGH_COMPRESSION = 4,
            EOS_CL_BEST_COMPRESSION = 5,
            EOS_CL_NO_COMPRESSION = 10
        } EOS_COMPRESSION_LEVEL;

        typedef enum
        {
            EOS_BEAMSHAPE_STANDARD = 0
        } EOS_BEAMSHAPE;

        typedef struct
        {
            EOS_MILLIMETER m_dX;
            EOS_MILLIMETER m_dY;
        } EOS_POINT2;

        typedef void(*EOS_FILETRANSFER_PROGRESS_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_FILETRANSFER_HANDLE handle, EOS_DOUBLE progress);
        typedef void(*EOS_STATE_CHANGE_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_TASK_STATE state);
        typedef void(*EOS_GENERATION_PROGRESS_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_LAYERINDEX finishedIndex, EOS_MICROMETER finishedHeight_um);

        typedef void(*EOS_BEGINLAYER_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROMETER z_um);
        typedef void(*EOS_ENDLAYER_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us);
        typedef void(*EOS_BEGINPART_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_INT partId);
        typedef void(*EOS_SELECTLASERSCANNERUNIT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_BYTE laserScannerIndex);
        typedef void(*EOS_SETLASERPOWER_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE power_W, EOS_MICROSECS delay_us);
        typedef void(*EOS_SETSCANSPEED_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE speed_mm_per_s);
        typedef void(*EOS_SETFOCUS_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE focus);
        typedef void(*EOS_SETSKYWRITING_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_INT skywritingEnabled);
        typedef void(*EOS_CUSTOMDATA_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_CWSTR customData);
        typedef void(*EOS_HATCH_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 start, EOS_POINT2 end);
        typedef void(*EOS_POLYSTART_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 startCoord);
        typedef void(*EOS_POLYCOORD_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 intermediateCoord);
        typedef void(*EOS_POLYEND_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 endCoord);
        typedef void(*EOS_DELAY_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_MICROSECS delay_us);
        typedef void(*EOS_SETEXPOSURETYPE_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_EXPOSURE_TYPE exposureType);
        typedef void(*EOS_SETEXPOSURESTEP_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_UINT exposureSetIndex, EOS_UINT exposureStepIndex);
        typedef void(*EOS_SETDEPTH_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROMETER exposedDepthInMicrometers);
        typedef void(*EOS_STARTSTRIPESEGMENT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 stripeBegin, EOS_POINT2 hatchDistVector);
        typedef void(*EOS_STARTGENERALSEGMENT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 hatchDistVector);
        typedef void(*EOS_ENDSEGMENT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle);
        typedef void(*EOS_ENDLINE_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle);
        typedef void(*EOS_SETLASERPULSE_CALLBACK)(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS pulsedWavePeriodTime_us, EOS_MICROSECS pulsedWavePowerOnTime_us);


        /*************************************************************************************************************************
         Neutral structures for the version-dependent entry points
        **************************************************************************************************************************/

        struct sEOSSDKVersion {
            uint32_t m_nMajor = 0;
            uint32_t m_nMinor = 0;
            uint32_t m_nBuild = 0;
            uint32_t m_nRevision = 0;

            bool isEmpty() const { return (m_nMajor == 0) && (m_nMinor == 0) && (m_nBuild == 0) && (m_nRevision == 0); }
            std::string toString() const;
            std::string toShortString() const;
        };

        // Bits of sEOSLaserScannerInfo::m_nCapabilities. Beam shaping is only reported by SDK 2.26 and later.
        constexpr uint64_t EOS_LSC_FOCUS_DEFOCUS = 0x1;
        constexpr uint64_t EOS_LSC_VARIABLE_FOCUS = 0x2;
        constexpr uint64_t EOS_LSC_BEAMSHAPING = 0x4;

        struct sEOSLaserScannerInfo {
            double m_dLaserPowerNominal = 0.0;
            double m_dLaserPowerMax = 0.0;
            double m_dMaxScanSpeed = 0.0;
            double m_dMinX = 0.0;
            double m_dMinY = 0.0;
            double m_dMaxX = 0.0;
            double m_dMaxY = 0.0;
            uint64_t m_nCapabilities = 0;
        };

        struct sEOSMachineInfo {
            EOS_VOLUME_TYPE m_VolumeType = EOS_VOL_CUBOID;
            double m_dDimX = 0.0;
            double m_dDimY = 0.0;
            double m_dDimZ = 0.0;
            double m_dDimR = 0.0;
            EOS_MODEL_TYPE m_ModelType = EOS_MT_M290;
            sEOSSDKVersion m_MachineSoftwareVersion;
            sEOSSDKVersion m_MachineInterfaceVersion;
            std::wstring m_sSerialNumber;
            std::wstring m_sPrintDomain;
            uint32_t m_nNumberOfLaserScanners = 0;
            std::vector<sEOSLaserScannerInfo> m_LaserScanners;
            std::wstring m_sModelName;
            bool m_bIsCustomizedMachine = false;
        };

        struct sEOSExposureSetInfo {
            std::wstring m_sName;
            EOS_MICROMETER m_nLayerThickness = 0;
            bool m_bIsEditable = false;
            bool m_bIsSupport = false;
            bool m_bUsesSmartFusion = false;
        };

        struct sEOSMaterialSetInfo {
            std::wstring m_sName;
            sEOSSDKVersion m_Version;
            std::wstring m_sUUID;
            std::wstring m_sParentUUID;
            EOS_MODEL_TYPE m_ModelType = EOS_MT_M290;
            std::wstring m_sMaterial;
            EOS_MICROMETER m_nBaseLayerThickness = 0;
            EOS_INERT_GAS m_InertGas = EOS_IG_NONE;
            bool m_bIsAuthoredByEos = false;
            bool m_bIsFinal = false;
            std::vector<sEOSExposureSetInfo> m_ExposureSets;
            std::wstring m_sModelName;
            bool m_bTargetsCustomizedMachine = false;
        };

        struct sEOSTaskMetaData {
            std::wstring m_sName;
            std::wstring m_sComment;
            uint32_t m_nNumberOfAllowedBuilds = 0;
            std::wstring m_sGeneratorName;
            std::wstring m_sGeneratorVersion;
            EOS_TASKMODE m_TaskMode = EOS_TM_REGULAR;
            // Empty means no part base directory.
            std::wstring m_sPartBaseDirectory;
            EOS_TASKFORMAT m_TaskFormat = EOS_TF_REGULAR;
            EOS_COMPRESSION_LEVEL m_Compression = EOS_CL_DEFAULT_COMPRESSION;
        };

        struct sEOSBuildTimeInfo {
            double m_dBuildTimeMs = 0.0;
            uint64_t m_nNumberOfCalculatedLayers = 0;
            uint64_t m_nNumberOfAllLayers = 0;
            double m_dExposureTimeMs = 0.0;
            double m_dNonExposureTimeMs = 0.0;
            uint64_t m_nCurrentLayerNumber = 0;
            std::vector<double> m_ExposureTimePerLaserScannerMs;
        };

        struct sEOSEventInfoItem {
            std::wstring m_sKey;
            std::wstring m_sValue;
        };

        struct sEOSEvent {
            EOS_MODULE_ID m_nModuleID = 0;
            EOS_EVENT_ID m_nEventID = 0;
            EOS_ERROR m_ErrorCode = EOS_ERR_NO_ERROR;
            EOS_SEVERITY m_Severity = EOS_SEV_WARNING;
            EOS_DISPLAY_OPTIONS m_DisplayOptions = EOS_DSP_NONE;
            std::wstring m_sName;
            std::vector<sEOSEventInfoItem> m_InfoItems;
        };

        struct sEOSLayerInfo {
            uint64_t m_nLayerIndex = 0;
            int32_t m_nDeltaInMicrometers = 0;
            // Upper edge of the layer, measured from the start of the job.
            int32_t m_nZPositionInMicrometers = 0;
            bool m_bIsExposed = false;
            std::vector<int32_t> m_ExposedPartIDs;
        };

        struct sEOSProcessHeights {
            double m_dPreProcessHeight = 0.0;
            double m_dBottomOffsetHeight = 0.0;
            double m_dPayloadHeight = 0.0;
            double m_dTopOffsetHeight = 0.0;
            double m_dPostProcessHeight = 0.0;
            double m_dAccumulatedShrinkageCompensation = 0.0;
            double m_dTotalZTravel = 0.0;
            double m_dMaxZTravel = 0.0;
        };

        typedef void (*PEOSBuildTimeInfoCallback) (void* pUserData, const sEOSBuildTimeInfo& buildTimeInfo);
        typedef void (*PEOSEventCallback) (void* pUserData, const sEOSEvent& event);

        std::string eosModelTypeToIdentifier(EOS_MODEL_TYPE modelType);
        std::string eosModelTypeToMachineTarget(EOS_MODEL_TYPE modelType);

    } // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_EOSSDK_TYPES
