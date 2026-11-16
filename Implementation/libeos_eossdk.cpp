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


Abstract: This is a stub class definition of CTaskCreation

*/

#include "libeos_eossdk.hpp"
#include "Common/common_utils.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

#include <Windows.h>
#include <stdexcept>
#include <vector>
#include <cstring>


using namespace Lib3MF4EOS::Impl;


std::string Lib3MF4EOS::Impl::eosErrorToString(EOS_ERROR errorCode) {

    switch (errorCode)
    {
    case EOS_ERR_NO_ERROR: return "Everything went well";
    case EOS_ERR_ARGUMENT_INVALID: return "An invalid argument was passed, e.g., a null pointer where not allowed";
    case EOS_ERR_ENCODING_CONVERSION_ERROR: return "A string could not be converted to another encoding";
    case EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS: return "A string used to create a task contains non-Windows-1252 characters";
    case EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS: return "A path contains non-Windows-1252 characters";
    case EOS_ERR_MISSING_LICENSE: return "A required license is missing";
    case EOS_ERR_EMPTY: return "Empty, nothing to get";
    case EOS_ERR_NO_MATCH: return "Nothing found, no match";
    case EOS_ERR_NOT_A_DIRECTORY: return "The given path is not a directory (e.g., because it does not exist or because it's a file)";
    case EOS_ERR_FILE_NOT_FOUND: return "The specified file was not found";
    case EOS_ERR_ILLEGAL_FILE_EXTENSION: return "The extension is invalid; e.g. extension does not fit to the requirements (not: file not found)";
    case EOS_ERR_FILESYSTEM_IO_ERROR: return "A file system level error occurred, e.g., because of missing access rights (not: file not found), disk out of space";
    case EOS_ERR_NETWORK_IO_ERROR: return "A network level error occurred, e.g., because missing network, missing server access rights, wrong network address";
    case EOS_ERR_OUT_OF_MEMORY: return "An operation could not be performed because memory allocating failed";
    case EOS_ERR_MACHINE_NOT_SUPPORTED: return "A machine type is not supported";
    case EOS_ERR_IDLE: return "An operation could not be performed, because the subject is (already) in idle state; e.g. aborting something which is not running";
    case EOS_ERR_NOT_IDLE: return "An operation could not be performed, because a competing operation is already running";
    case EOS_ERR_FILETRANSFER_INVALID: return "An invalid file transfer handle was passed";
    case EOS_ERR_FILETRANSFER_RUNNING: return "A file transfer is running or pending (queued)";
    case EOS_ERR_FILETRANSFER_NOT_RUNNING: return "No running or queued file transfer (nothing to abort)";
    case EOS_ERR_FILETRANSFER_ABORTED: return "The file transfer was aborted by the user";
    case EOS_ERR_FILETRANSFER_FAILED: return "The file transfer failed, e.g., caused by transmission failure, protocol failure, non-gracefully disconnection";
    case EOS_ERR_SESSION_INVALID: return "An invalid session handle was passed";
    case EOS_ERR_TOO_MANY_SESSIONS: return "The maximum of simultaneously allowed sessions was already reached";
    case EOS_ERR_BUILDTIME_ESTIMATION_RUNNING: return "A build time estimation is running or pending";
    case EOS_ERR_BUILDTIME_ESTIMATION_NOT_RUNNING: return "No running build time estimation (nothing to abort)";
    case EOS_ERR_BUILDTIME_ESTIMATION_ABORTED: return "The build time estimation was aborted by the user";
    case EOS_ERR_PREVIEW_RUNNING: return "A preview is running or pending";
    case EOS_ERR_PREVIEW_NOT_RUNNING: return "No running preview (nothing to abort)";
    case EOS_ERR_PREVIEW_ABORTED: return "The preview was aborted by the user";
    case EOS_ERR_PREVIEW_LAYER_INVALID: return "A non-existant layer index was specified";
    case EOS_ERR_PREVIEW_ZHEIGHT_INVALID: return "An invalid z-height was specified";
    case EOS_ERR_TASK_CREATION_RUNNING: return "A task creation is running or pending (queued)";
    case EOS_ERR_TASK_CREATION_NOT_RUNNING: return "No running task creation (nothing to abort)";
    case EOS_ERR_TASK_CREATION_ABORTED: return "The task creation was aborted by the user";
    case EOS_ERR_TASK_WRITE_FAILED: return "Some information could not be written to the task";
    case EOS_ERR_TASK_INVALID: return "The task is invalid, e.g., because it contains invalid/inconsistent data";
    case EOS_ERR_TASK_CORRUPT: return "The task is corrupt, i.e. it cannot be read, because its format is completely unrecognized";
    case EOS_ERR_JOB_NOT_LOADED: return "No job loaded; operations which need a loaded job could not be performed";
    case EOS_ERR_CONFIG_NOT_LOADED: return "No machine configuration loaded; operations which need a loaded machine configuration could not be performed";
    case EOS_ERR_CONFIG_INVALID: return "An operation could not be performed, e.g., because machine configuration is invalid: serial number differs from machine or outdated or contains inconsistent data";
    case EOS_ERR_CONFIG_CORRUPT: return "The machine configuration is corrupt, e.g., because files cannot be opened or because their format is not recognized";
    case EOS_ERR_ILLEGAL_CONFIG_DIR: return "One of the required files in the configuration directory is missing";
    case EOS_ERR_OPENJOB_NOT_FOUND: return "The OpenJob was not found";
    case EOS_ERR_OPENJOB_NOT_SUPPORTED: return "The OpenJob is of a not supported version";
    case EOS_ERR_OPENJOB_INVALID: return "The OpenJob contains invalid data, e.g., a reference to an undefined positioning point";
    case EOS_ERR_OPENJOB_CORRUPT: return "The OpenJob is corrupt, i.e. its format is completely unrecognized (e.g., not an XML or not well-formed)";
    case EOS_ERR_OPENJZ_INVALID: return "The OpenJz is invalid, e.g., because it can be opened and extracted, but its internal file structure is unrecognized";
    case EOS_ERR_OPENJZ_CORRUPT: return "The OpenJz is corrupt, i.e. it cannot be extracted";
    case EOS_ERR_EOSPAR_NOT_FOUND: return "The .eospar file was not found";
    case EOS_ERR_EOSPAR_UNSUPPORTED: return "The .eospar file is of a not supported version";
    case EOS_ERR_EOSPAR_INVALID: return "The .eospar file contains invalid/inconsistent data";
    case EOS_ERR_EOSPAR_CORRUPT: return "The .eospar file cannot be read, because its format is completely unrecognized";
    case EOS_ERR_DIRECTORY_NOT_EMPTY: return "A directory that was expected to be empty was not";
    case EOS_ERR_WAIT_TIMED_OUT: return "Waiting timed out";
    case EOS_ERR_INVALID_CODE_PAGE: return "The provided code page is invalid";
    case EOS_ERR_INCORRECT_ENCODING: return "A string was not correctly encoded as unicode";
    case EOS_ERR_PART_FILE_NOT_FOUND: return "A specified part file was not found; e.g., a referenced part file in an openjob";
    case EOS_ERR_MACHINE_VERSION_NOT_SUPPORTED: return "The machine software version is higher than the expected version";
    case EOS_ERR_FILECREATION_INVALID: return "An invalid file transfer handle was passed";
    case EOS_ERR_FILECREATION_NOT_RUNNING: return "No running or queued file creation (nothing to abort)";
    case EOS_ERR_FILECREATION_ABORTED: return "The file creation was aborted by the user";
    case EOS_ERR_STL_INVALID: return "The given stl file cannot be loaded";
    case EOS_ERR_DEFORMATION_ERROR: return "The requested deformation operation on a mesh could not be performed";
    case EOS_ERR_DOMAIN_ERROR: return "A numerical input value was outside the domain of an operation";
    case EOS_ERR_OVERFLOW_ERROR: return "An overflow occured while processing the result of an operation";
    case EOS_ERR_STL_BELOW_PLATFORM: return "The given stl file is (partially) below the platform, i.e. it contains negative z coordinates";
    case EOS_ERR_STL_OUTSIDE_BUILD_VOLUME: return "The given stl file is (partially) outside the build volume of the machine";
    case EOS_ERR_STL_ABOVE_JOB_HEIGHT: return "The given stl file is (partially) above the given job height";
    case EOS_ERR_STL_CONTAINS_NEGATIVE_COORDINATES: return "The given stl file contains negative coordinates, which is not supported by the requested action";
    case EOS_ERR_INVALID_PART_ID: return "An invalid part id was supplied";
    case EOS_ERR_INVALID_EXPOSURE_SET_NAME: return "An invalid Exposure Set name was supplied";
    case EOS_ERR_FEATURE_NOT_SUPPORTED: return "A specific feature is used by the job, but the target machine does not support it";
    case EOS_ERR_TASK_UNSUPPORTED_VERSION: return "The version of the task or one of its dependencies is not supported for this feature";
    case EOS_ERR_ADJUSTED_VALUE_VIOLATES_THRESHOLD: return "After applying one or several adjustments (job- or machine-specific) to an adjustable building process parameter its value now violates the specified min / max threshold.";
    case EOS_ERR_ADJUSTMENT_ID_INVALID: return "The currently loaded material set does not contain an adjustable process parameter with the specified id.";
    case EOS_ERR_ADJUSTMENT_TYPE_NOT_ENABLED: return "The specified adjustment type is not enabled for the specified adjustable process parameter. (e.g., when trying to get a job specific adjustment for an adjustable process parameter that only allows machine specific adjustments)";
    case EOS_ERR_ADJUSTMENT_NOT_SUPPORTED_BY_MACHINE: return "The specified adjustment was valid, but it is not supported by the machine to which it applies, because the machine is running an old version of EOSYSTEM.";
    case EOS_ERR_ADJUSTMENT_HAS_NO_EFFECT_ON_EXPOSURE: return "The specfied adjustment cannot be applied, because the EOSPRINT API only allows adjustment of process parameters that have an effect on exposure.";
    case EOS_ERR_NOT_IMPLEMENTED: return "The function is not implemented by this SDK";
    case EOS_ERR_LOADING_DLL_FAILED: return "A library required by the SDK could not be loaded";
    case EOS_ERR_CONSTRAINT_VIOLATED: return "A constraint of the operation was violated";
    case EOS_ERR_DEFOCUS_NOT_SUPPORTED: return "Defocus is not supported";
    case EOS_ERR_PROTOCOL_ERROR: return "A protocol error occurred";
    case EOS_ERR_FILECREATION_RUNNING: return "A file creation is running";
    case EOS_ERR_FILE_HANDLE_INVALID: return "The file handle is invalid";
    case EOS_ERR_INVALID_LAYER: return "The layer is invalid";
    case EOS_ERR_FAULTY_TRIANGLE_MESH: return "The triangle mesh is faulty";
    case EOS_ERR_CANNOT_WRITE_TO_INPUT_FILE: return "A file opened for reading cannot be written";
    case EOS_ERR_NO_LAYER: return "The command requires an open layer";
    case EOS_ERR_NO_SCANNER: return "The command requires a selected laser scanner";
    case EOS_ERR_NO_PART: return "The command requires an open part";
    case EOS_ERR_LAYER: return "The layer command is not valid at this point";
    case EOS_ERR_PART: return "The part command is not valid at this point";
    case EOS_ERR_LSU_INVALID: return "The laser scanner index is invalid";
    case EOS_ERR_UNSUPPORTED: return "The operation is not supported";
    case EOS_ERR_BAD_COORDINATE: return "A coordinate is invalid";
    case EOS_ERR_COMMAND_NOT_POSSIBLE: return "The command is not possible at this point";
    case EOS_ERR_CANNOT_UNLOAD: return "The object cannot be unloaded";
    case EOS_ERR_INSUFFICIENT_SIZE: return "A buffer or value is too small";
    case EOS_ERR_NOT_ALLOWED: return "The operation is not allowed";
    case EOS_ERR_CLIENT_NOT_AUTHENTICATED: return "The client is not authenticated for the toolpath API";
    case EOS_ERR_INVALID_EXPOSURE_PARAMETERS: return "The exposure parameters are invalid";
    case EOS_ERR_RESOURCECACHE_EXTERNAL_IMPORT_ERROR: return "The resource cache could not import an external file";
    case EOS_ERR_RESOURCECACHE_EXTERNAL_MESH_ERROR: return "The resource cache could not process an external mesh";
    case EOS_ERR_RESOURCECACHE_EXTERNAL_CONTOUR_ERROR: return "The resource cache could not process an external contour";
    case EOS_ERR_RESOURCECACHE_EXTERNAL_IMPORT_WARNING: return "The resource cache reported a warning while importing an external file";
    case EOS_ERR_RESOURCECACHE_EXTERNAL_MESH_WARNING: return "The resource cache reported a warning while processing an external mesh";
    case EOS_ERR_RESOURCECACHE_EXTERNAL_CONTOUR_WARNING: return "The resource cache reported a warning while processing an external contour";
    case EOS_ERR_FILE_MALFORMED: return "The file is malformed";
    case EOS_ERR_PARAMETER_DOES_NOT_EXIST: return "The parameter does not exist";
    case EOS_ERR_PARAMETER_INACCESSIBLE: return "The parameter is not accessible";
    case EOS_ERR_INVALID_PARAMETER_VALUE: return "The parameter value is invalid";
    case EOS_ERR_ID_CONFLICT: return "An identifier is used more than once";
    case EOS_ERR_FILE_CORRUPT: return "The file is corrupt";
    case EOS_ERR_INSUFFICIENT_DISK_SPACE: return "There is not enough disk space";
    case EOS_ERR_DATA_FORMAT_UNSUPPORTED: return "The data format is not supported";
    case EOS_ERR_EOSSIM_INVALID: return "The .eossim file is invalid";
    case EOS_ERR_EOSSIM_CORRUPT: return "The .eossim file is corrupt";
    case EOS_ERR_INTERNAL_ERROR: return "An internal error state was reached, please consider reporting this to EOS support.";
    case EOS_ERR_UNEXPECTED_ERROR: return "An unknown exception was caught, please consider reporting this to EOS support";
    default: return "An unknown error was returned.";
    }
}

std::string eosErrorToStringVerbose(EOS_ERROR errorCode) {
    return eosErrorToString(errorCode) + " (#" + std::to_string(errorCode) + ")";
}

CEOSSDK::CEOSSDK(const std::string& sSDKDirectory, const std::string& sEOSPrintAPIDLLName, uint32_t nOverrideMajor, uint32_t nOverrideMinor)
    : m_hLibraryHandle (nullptr), m_sSDKDirectory (sSDKDirectory), m_ProductVersionSource (eEOSSDKVersionSource::None)
{
    resetPtrs();

    if (!sEOSPrintAPIDLLName.empty()) {
        loadSDK(sEOSPrintAPIDLLName);
        detectProductVersion();
    }

    selectAdapter(nOverrideMajor, nOverrideMinor);

    m_pAdapter->loadEntryPoints(m_hLibraryHandle);
    if (m_hLibraryHandle != nullptr) {
        auto missingEntryPoints = m_pAdapter->getMissingRequiredEntryPoints();
        if (!missingEntryPoints.empty())
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTFINDLIBRARYEXPORT, "API export not found: " + missingEntryPoints.front());
    }
}

void CEOSSDK::detectProductVersion()
{
    m_ProductVersion = sEOSSDKVersion();
    m_ProductVersionSource = eEOSSDKVersionSource::None;

    if (Eos_GetProductVersion != nullptr) {
        EOS_VERSION productVersion;
        memset((void*)&productVersion, 0, sizeof(productVersion));
        if ((Eos_GetProductVersion(&productVersion) == EOS_ERR_NO_ERROR) && (productVersion.major > 0)) {
            m_ProductVersion.m_nMajor = (uint32_t)productVersion.major;
            m_ProductVersion.m_nMinor = (uint32_t)productVersion.minor;
            m_ProductVersion.m_nBuild = (uint32_t)productVersion.build;
            m_ProductVersion.m_nRevision = (uint32_t)productVersion.revision;
            m_ProductVersionSource = eEOSSDKVersionSource::ProductVersionFunction;
            return;
        }
    }

    if (m_hLibraryHandle == nullptr)
        return;

    std::vector<wchar_t> modulePath(32768, 0);
    DWORD nPathLength = GetModuleFileNameW((HMODULE)m_hLibraryHandle, modulePath.data(), (DWORD)modulePath.size());
    if ((nPathLength == 0) || (nPathLength >= modulePath.size()))
        return;

    DWORD hDummy = 0;
    DWORD nVersionInfoSize = GetFileVersionInfoSizeW(modulePath.data(), &hDummy);
    if (nVersionInfoSize == 0)
        return;

    std::vector<uint8_t> versionInfo(nVersionInfoSize, 0);
    if (!GetFileVersionInfoW(modulePath.data(), 0, nVersionInfoSize, versionInfo.data()))
        return;

    VS_FIXEDFILEINFO* pFixedFileInfo = nullptr;
    UINT nFixedFileInfoSize = 0;
    if (!VerQueryValueW(versionInfo.data(), L"\\", (LPVOID*)&pFixedFileInfo, &nFixedFileInfoSize))
        return;
    if ((pFixedFileInfo == nullptr) || (nFixedFileInfoSize < sizeof(VS_FIXEDFILEINFO)))
        return;

    m_ProductVersion.m_nMajor = HIWORD(pFixedFileInfo->dwProductVersionMS);
    m_ProductVersion.m_nMinor = LOWORD(pFixedFileInfo->dwProductVersionMS);
    m_ProductVersion.m_nBuild = HIWORD(pFixedFileInfo->dwProductVersionLS);
    m_ProductVersion.m_nRevision = LOWORD(pFixedFileInfo->dwProductVersionLS);

    if (m_ProductVersion.m_nMajor > 0)
        m_ProductVersionSource = eEOSSDKVersionSource::VersionResource;
    else
        m_ProductVersion = sEOSSDKVersion();
}

void CEOSSDK::selectAdapter(uint32_t nOverrideMajor, uint32_t nOverrideMinor)
{
    m_pAdapter = nullptr;

    bool bHasOverride = (nOverrideMajor != 0) || (nOverrideMinor != 0);
    bool bHasProductVersion = (m_ProductVersionSource != eEOSSDKVersionSource::None);

    if (bHasOverride) {
        std::string sMessage = "EOS SDK version override " + std::to_string(nOverrideMajor) + "." + std::to_string(nOverrideMinor) + " is used";
        if (bHasProductVersion)
            sMessage += " instead of the detected version " + m_ProductVersion.toString();
        m_VersionMessages.push_back(sMessage + ".");

        m_pAdapter = CEOSSDKAdapterRegistry::createAdapter(nOverrideMajor, nOverrideMinor, m_VersionMessages);
        if (m_pAdapter.get() == nullptr)
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_DETECTEDINCOMPATIBLEEOSSDKVERSION, "EOS SDK version override " + std::to_string(nOverrideMajor) + "." + std::to_string(nOverrideMinor)
                + " is lower than the lowest supported version (supported: " + CEOSSDKAdapterRegistry::getSupportedVersionsString() + ")");
    }
    else if (bHasProductVersion) {
        m_pAdapter = CEOSSDKAdapterRegistry::createAdapter(m_ProductVersion.m_nMajor, m_ProductVersion.m_nMinor, m_VersionMessages);
        if (m_pAdapter.get() == nullptr)
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_DETECTEDINCOMPATIBLEEOSSDKVERSION, "EOS SDK version " + m_ProductVersion.toString()
                + " is lower than the lowest supported version (supported: " + CEOSSDKAdapterRegistry::getSupportedVersionsString() + ")");
    }
    else {
        m_pAdapter = CEOSSDKAdapterRegistry::createLowestAdapter();
        if (m_hLibraryHandle != nullptr)
            m_VersionMessages.push_back("The EOS SDK product version could not be detected; assuming version " + m_pAdapter->getVersionString() + ".");
        else
            m_VersionMessages.push_back("No EOS SDK library is loaded; simulating version " + m_pAdapter->getVersionString() + ".");
    }
}

bool CEOSSDK::hasLibrary()
{
    return (m_hLibraryHandle != nullptr);
}

CEOSSDKAdapter& CEOSSDK::getAdapter()
{
    if (m_pAdapter.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);
    return *m_pAdapter;
}

sEOSSDKVersion CEOSSDK::getProductVersion()
{
    return m_ProductVersion;
}

eEOSSDKVersionSource CEOSSDK::getProductVersionSource()
{
    return m_ProductVersionSource;
}

const std::vector<std::string>& CEOSSDK::getVersionMessages()
{
    return m_VersionMessages;
}

CEOSSDK::~CEOSSDK()
{

}

void CEOSSDK::UnloadSDK()
{
    resetPtrs();

	if (m_hLibraryHandle != nullptr)
		FreeLibrary((HMODULE)m_hLibraryHandle);
	m_hLibraryHandle = nullptr;
}


void CEOSSDK::loadSDK(const std::string& sEOSPrintAPIDLLName)
{

    UnloadSDK();

    std::wstring sEOSPrintAPIDLLNameW = AMCCommon::CUtils::UTF8toUTF16(sEOSPrintAPIDLLName);

	HMODULE hLibrary = LoadLibraryW(sEOSPrintAPIDLLNameW.c_str());
	if (hLibrary == 0)
		throw std::runtime_error("Could not load EOS API library.");

	m_hLibraryHandle = hLibrary;

    Eos_GetApiVersion = (PEos_GetAPIVersion)GetProcAddress(hLibrary, "Eos_GetApiVersion");
	if (Eos_GetApiVersion == nullptr)
		throw std::runtime_error("API export not found: Eos_GetApiVersion");

    // Only exported from version 2.26 on.
    Eos_GetProductVersion = (PEos_GetProductVersion)GetProcAddress(hLibrary, "Eos_GetProductVersion");

    Eos_InitializeApi = (PEos_InitializeApi)GetProcAddress(hLibrary, "Eos_InitializeApi");
	if (Eos_InitializeApi == nullptr)
		throw std::runtime_error("API export not found: Eos_InitializeApi");

    Eos_DeinitializeApi = (PEos_DeinitializeApi)GetProcAddress(hLibrary, "Eos_DeinitializeApi");
	if (Eos_DeinitializeApi == nullptr)
		throw std::runtime_error("API export not found: Eos_DeinitializeApi");

    EosTaskGen_LoadOpenJz = (PEosTaskGen_LoadOpenJz)GetProcAddress(hLibrary, "EosTaskGen_LoadOpenJz");
    if (EosTaskGen_LoadOpenJz == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_LoadOpenJz");

    EosTaskGen_UnloadJob = (PEosTaskGen_UnloadJob)GetProcAddress(hLibrary, "EosTaskGen_UnloadJob");
    if (EosTaskGen_UnloadJob == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_UnloadJob");

    EosTaskGen_IsJobLoaded = (PEosTaskGen_IsJobLoaded)GetProcAddress(hLibrary, "EosTaskGen_IsJobLoaded");
    if (EosTaskGen_IsJobLoaded == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_IsJobLoaded");

    EosTaskGen_BeginBuildTimeEstimation = (PEosTaskGen_BeginBuildTimeEstimation)GetProcAddress(hLibrary, "EosTaskGen_BeginBuildTimeEstimation");
    if (EosTaskGen_BeginBuildTimeEstimation == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_BeginBuildTimeEstimation");

    EosTaskGen_AbortBuildTimeEstimation = (PEosTaskGen_AbortBuildTimeEstimation)GetProcAddress(hLibrary, "EosTaskGen_AbortBuildTimeEstimation");
    if (EosTaskGen_AbortBuildTimeEstimation == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_AbortBuildTimeEstimation");

    EosTaskGen_GetBuildTimeEstimationState = (PEosTaskGen_GetBuildTimeEstimationState)GetProcAddress(hLibrary, "EosTaskGen_GetBuildTimeEstimationState");
    if (EosTaskGen_GetBuildTimeEstimationState == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetBuildTimeEstimationState");

    EosTaskGen_WaitForBuildTimeEstimation = (PEosTaskGen_WaitForBuildTimeEstimation)GetProcAddress(hLibrary, "EosTaskGen_WaitForBuildTimeEstimation");
    if (EosTaskGen_WaitForBuildTimeEstimation == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_WaitForBuildTimeEstimation");

    EosTaskGen_GetBuildTimeEstimation = (PEosTaskGen_GetBuildTimeEstimation)GetProcAddress(hLibrary, "EosTaskGen_GetBuildTimeEstimation");
    if (EosTaskGen_GetBuildTimeEstimation == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetBuildTimeEstimation");

    EosTaskGen_AbortTaskCreation = (PEosTaskGen_AbortTaskCreation)GetProcAddress(hLibrary, "EosTaskGen_AbortTaskCreation");
    if (EosTaskGen_AbortTaskCreation == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_AbortTaskCreation");

    EosTaskGen_GetTaskCreationState = (PEosTaskGen_GetTaskCreationState)GetProcAddress(hLibrary, "EosTaskGen_GetTaskCreationState");
    if (EosTaskGen_GetTaskCreationState == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetTaskCreationState");
    
    EosTaskGen_WaitForTaskCreation = (PEosTaskGen_WaitForTaskCreation)GetProcAddress(hLibrary, "EosTaskGen_WaitForTaskCreation");
    if (EosTaskGen_WaitForTaskCreation == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_WaitForTaskCreation");

    EosTaskGen_GetCurrentLayerIndex = (PEosTaskGen_GetCurrentLayerIndex)GetProcAddress(hLibrary, "EosTaskGen_GetCurrentLayerIndex");
    if (EosTaskGen_GetCurrentLayerIndex == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetCurrentLayerIndex");

    EosTaskGen_GetStartLayerIndex = (PEosTaskGen_GetStartLayerIndex)GetProcAddress(hLibrary, "EosTaskGen_GetStartLayerIndex");
    if (EosTaskGen_GetStartLayerIndex == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetStartLayerIndex");

    EosTaskGen_GetTargetLayerIndex = (PEosTaskGen_GetTargetLayerIndex)GetProcAddress(hLibrary, "EosTaskGen_GetTargetLayerIndex");
    if (EosTaskGen_GetTargetLayerIndex == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetTargetLayerIndex");

    EosTaskGen_GetCurrentZHeight = (PEosTaskGen_GetCurrentZHeight)GetProcAddress(hLibrary, "EosTaskGen_GetCurrentZHeight");
    if (EosTaskGen_GetCurrentZHeight == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetCurrentZHeight");

    EosTaskGen_GetStartZHeight = (PEosTaskGen_GetStartZHeight)GetProcAddress(hLibrary, "EosTaskGen_GetStartZHeight");
    if (EosTaskGen_GetStartZHeight == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetStartZHeight");

    EosTaskGen_GetTargetZHeight = (PEosTaskGen_GetTargetZHeight)GetProcAddress(hLibrary, "EosTaskGen_GetTargetZHeight");
    if (EosTaskGen_GetTargetZHeight == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_GetTargetZHeight");

    EosTaskGen_SetNumberOfTaskCreationThreads = (PEosTaskGen_SetNumberOfTaskCreationThreads)GetProcAddress(hLibrary, "EosTaskGen_SetNumberOfTaskCreationThreads");
    if (EosTaskGen_SetNumberOfTaskCreationThreads == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_SetNumberOfTaskCreationThreads");
    
    EosTaskGen_LoadMachineConfiguration = (PEosTaskGen_LoadMachineConfiguration)GetProcAddress(hLibrary, "EosTaskGen_LoadMachineConfiguration");
    if (EosTaskGen_LoadMachineConfiguration == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_LoadMachineConfiguration");

    EosThreads_GetNumberOfAvailableThreads = (PEosThreads_GetNumberOfAvailableThreads)GetProcAddress(hLibrary, "EosThreads_GetNumberOfAvailableThreads");
    if (EosThreads_GetNumberOfAvailableThreads == nullptr)
        throw std::runtime_error("API export not found: EosThreads_GetNumberOfAvailableThreads");

    Eos_ResetAllCallbacks = (PEos_ResetAllCallbacks)GetProcAddress(hLibrary, "Eos_ResetAllCallbacks");
    if (Eos_ResetAllCallbacks == nullptr)
        throw std::runtime_error("API export not found: Eos_ResetAllCallbacks");

    EosTaskGen_UnloadMachineConfiguration = (PEosTaskGen_UnloadMachineConfiguration)GetProcAddress(hLibrary, "EosTaskGen_UnloadMachineConfiguration");
    if (EosTaskGen_UnloadMachineConfiguration == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_UnloadMachineConfiguration");

    EosTaskGen_IsMachineConfigLoaded = (PEosTaskGen_IsMachineConfigLoaded)GetProcAddress(hLibrary, "EosTaskGen_IsMachineConfigLoaded");
    if (EosTaskGen_IsMachineConfigLoaded == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_IsMachineConfigLoaded");

    EosTaskGen_SetCacheDirectory = (PEosTaskGen_SetCacheDirectory)GetProcAddress(hLibrary, "EosTaskGen_SetCacheDirectory");
    if (EosTaskGen_SetCacheDirectory == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_SetCacheDirectory");
     
    EosTaskGen_SetStateChangeCallback = (PEosTaskGen_SetStateChangeCallback)GetProcAddress(hLibrary, "EosTaskGen_SetStateChangeCallback");
    if (EosTaskGen_SetStateChangeCallback == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_SetStateChangeCallback");

    EosTaskGen_SetProgressCallback = (PEosTaskGen_SetProgressCallback)GetProcAddress(hLibrary, "EosTaskGen_SetProgressCallback");
    if (EosTaskGen_SetProgressCallback == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_SetProgressCallback");
     
    EosTaskGen_SetBuildTimeStateChangedCallback = (PEosTaskGen_SetBuildTimeStateChangedCallback)GetProcAddress(hLibrary, "EosTaskGen_SetBuildTimeStateChangedCallback");
    if (EosTaskGen_SetBuildTimeStateChangedCallback == nullptr)
        throw std::runtime_error("API export not found: EosTaskGen_SetBuildTimeStateChangedCallback");

    EosMachineSession_Create = (PEosMachineSession_Create)GetProcAddress(hLibrary, "EosMachineSession_Create");
    if (EosMachineSession_Create == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_Create");

    EosMachineSession_CreateWithHint = (PEosMachineSession_CreateWithHint)GetProcAddress(hLibrary, "EosMachineSession_CreateWithHint");
    if (EosMachineSession_CreateWithHint == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_CreateWithHint");


    EosMachineSession_Destroy = (PEosMachineSession_Destroy)GetProcAddress(hLibrary, "EosMachineSession_Destroy");
    if (EosMachineSession_Destroy == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_Destroy");

    EosMachineSession_BeginConfigDownload = (PEosMachineSession_BeginConfigDownload)GetProcAddress(hLibrary, "EosMachineSession_BeginConfigDownload");
    if (EosMachineSession_BeginConfigDownload == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_BeginConfigDownload");

    EosMachineSession_BeginTaskUpload = (PEosMachineSession_BeginTaskUpload)GetProcAddress(hLibrary, "EosMachineSession_BeginTaskUpload");
    if (EosMachineSession_BeginTaskUpload == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_BeginTaskUpload");

    EosMachineSession_AbortFileTransfer = (PEosMachineSession_AbortFileTransfer)GetProcAddress(hLibrary, "EosMachineSession_AbortFileTransfer");
    if (EosMachineSession_AbortFileTransfer == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_AbortFileTransfer");

    EosMachineSession_AbortAllFileTransfers = (PEosMachineSession_AbortAllFileTransfers)GetProcAddress(hLibrary, "EosMachineSession_AbortAllFileTransfers");
    if (EosMachineSession_AbortAllFileTransfers == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_AbortAllFileTransfers");

    EosMachineSession_GetFileTransferState = (PEosMachineSession_GetFileTransferState)GetProcAddress(hLibrary, "EosMachineSession_GetFileTransferState");
    if (EosMachineSession_GetFileTransferState == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_GetFileTransferState");

    EosMachineSession_WaitForFileTransfer = (PEosMachineSession_WaitForFileTransfer)GetProcAddress(hLibrary, "EosMachineSession_WaitForFileTransfer");
    if (EosMachineSession_WaitForFileTransfer == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_WaitForFileTransfer");

    EosMachineSession_GetFileTransferResult = (PEosMachineSession_GetFileTransferResult)GetProcAddress(hLibrary, "EosMachineSession_GetFileTransferResult");
    if (EosMachineSession_GetFileTransferResult == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_GetFileTransferResult");

    EosMachineSession_WaitForAllFileTransfers = (PEosMachineSession_WaitForAllFileTransfers)GetProcAddress(hLibrary, "EosMachineSession_WaitForAllFileTransfers");
    if (EosMachineSession_WaitForAllFileTransfers == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_WaitForAllFileTransfers");

    EosMachineSession_ReleaseFileTransferHandle = (PEosMachineSession_ReleaseFileTransferHandle)GetProcAddress(hLibrary, "EosMachineSession_ReleaseFileTransferHandle");
    if (EosMachineSession_ReleaseFileTransferHandle == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_ReleaseFileTransferHandle");

    EosMachineSession_SetFileTransferProgressCallback = (PEosMachineSession_SetFileTransferProgressCallback)GetProcAddress(hLibrary, "EosMachineSession_SetFileTransferProgressCallback");
    if (EosMachineSession_SetFileTransferProgressCallback == nullptr)
        throw std::runtime_error("API export not found: EosMachineSession_SetFileTransferProgressCallback");

    // Toolpath API is optional!
    EosToolpath_GetAuthChallenge = (PEosToolpath_GetAuthChallenge)GetProcAddress(hLibrary, "EosToolpath_GetAuthChallenge");
    EosToolpath_SetAuthResponse = (PEosToolpath_SetAuthResponse)GetProcAddress(hLibrary, "EosToolpath_SetAuthResponse");
    EosToolpath_OpenOutputFile = (PEosToolpath_OpenOutputFile)GetProcAddress(hLibrary, "EosToolpath_OpenOutputFile");
	EosToolpath_ChopOutput = (PEosToolpath_ChopOutput)GetProcAddress(hLibrary, "EosToolpath_ChopOutput");
	EosToolpath_OpenInputFile = (PEosToolpath_OpenInputFile)GetProcAddress(hLibrary, "EosToolpath_OpenInputFile");
	EosToolpath_CloseFile = (PEosToolpath_CloseFile)GetProcAddress(hLibrary, "EosToolpath_CloseFile");
	EosToolpath_ReadLayer = (PEosToolpath_ReadLayer)GetProcAddress(hLibrary, "EosToolpath_ReadLayer");
	EosToolpath_GetNumberOfLayers = (PEosToolpath_GetNumberOfLayers)GetProcAddress(hLibrary, "EosToolpath_GetNumberOfLayers");
	EosToolpath_WriteBeginLayer = (PEosToolpath_WriteBeginLayer)GetProcAddress(hLibrary, "EosToolpath_WriteBeginLayer");
	EosToolpath_WriteBeginPart = (PEosToolpath_WriteBeginPart)GetProcAddress(hLibrary, "EosToolpath_WriteBeginPart");
	EosToolpath_WriteSelectLaserScanner = (PEosToolpath_WriteSelectLaserScanner)GetProcAddress(hLibrary, "EosToolpath_WriteSelectLaserScanner");
	EosToolpath_WriteSetLaserPower = (PEosToolpath_WriteSetLaserPower)GetProcAddress(hLibrary, "EosToolpath_WriteSetLaserPower");
	EosToolpath_WriteSetScanSpeed = (PEosToolpath_WriteSetScanSpeed)GetProcAddress(hLibrary, "EosToolpath_WriteSetScanSpeed");
	EosToolpath_WriteSetFocus = (PEosToolpath_WriteSetFocus)GetProcAddress(hLibrary, "EosToolpath_WriteSetFocus");
	EosToolpath_WriteSetSkywriting = (PEosToolpath_WriteSetSkywriting)GetProcAddress(hLibrary, "EosToolpath_WriteSetSkywriting");
	EosToolpath_WriteCustomData = (PEosToolpath_WriteCustomData)GetProcAddress(hLibrary, "EosToolpath_WriteCustomData");
	EosToolpath_WriteHatch = (PEosToolpath_WriteHatch)GetProcAddress(hLibrary, "EosToolpath_WriteHatch");
	EosToolpath_WritePolyStart = (PEosToolpath_WritePolyStart)GetProcAddress(hLibrary, "EosToolpath_WritePolyStart");
	EosToolpath_WritePolyCoord = (PEosToolpath_WritePolyCoord)GetProcAddress(hLibrary, "EosToolpath_WritePolyCoord");
	EosToolpath_WritePolyEnd = (PEosToolpath_WritePolyEnd)GetProcAddress(hLibrary, "EosToolpath_WritePolyEnd");
	EosToolpath_WriteDelay = (PEosToolpath_WriteDelay)GetProcAddress(hLibrary, "EosToolpath_WriteDelay");
	EosToolpath_WriteSetExposureType = (PEosToolpath_WriteSetExposureType)GetProcAddress(hLibrary, "EosToolpath_WriteSetExposureType");
	EosToolpath_WriteSetExposureStep = (PEosToolpath_WriteSetExposureStep)GetProcAddress(hLibrary, "EosToolpath_WriteSetExposureStep");
	EosToolpath_WriteSetDepth = (PEosToolpath_WriteSetDepth)GetProcAddress(hLibrary, "EosToolpath_WriteSetDepth");
	EosToolpath_WriteStartStripeSegment = (PEosToolpath_WriteStartStripeSegment)GetProcAddress(hLibrary, "EosToolpath_WriteStartStripeSegment");
	EosToolpath_WriteStartGeneralSegment = (PEosToolpath_WriteStartGeneralSegment)GetProcAddress(hLibrary, "EosToolpath_WriteStartGeneralSegment");
	EosToolpath_WriteEndSegment = (PEosToolpath_WriteEndSegment)GetProcAddress(hLibrary, "EosToolpath_WriteEndSegment");
	EosToolpath_WriteEndLine = (PEosToolpath_WriteEndLine)GetProcAddress(hLibrary, "EosToolpath_WriteEndLine");
	EosToolpath_WriteSetLaserPulse = (PEosToolpath_WriteSetLaserPulse)GetProcAddress(hLibrary, "EosToolpath_WriteSetLaserPulse");
	EosToolpath_SetBeginLayerCallback = (PEosToolpath_SetBeginLayerCallback)GetProcAddress(hLibrary, "EosToolpath_SetBeginLayerCallback");
	EosToolpath_SetEndLayerCallback = (PEosToolpath_SetEndLayerCallback)GetProcAddress(hLibrary, "EosToolpath_SetEndLayerCallback");
	EosToolpath_SetBeginPartCallback = (PEosToolpath_SetBeginPartCallback)GetProcAddress(hLibrary, "EosToolpath_SetBeginPartCallback");
	EosToolpath_SetSelectLaserScannerCallback = (PEosToolpath_SetSelectLaserScannerCallback)GetProcAddress(hLibrary, "EosToolpath_SetSelectLaserScannerCallback");
	EosToolpath_SetSetLaserPowerCallback = (PEosToolpath_SetSetLaserPowerCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetLaserPowerCallback");
	EosToolpath_SetSetScanSpeedCallback = (PEosToolpath_SetSetScanSpeedCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetScanSpeedCallback");
	EosToolpath_SetSetFocusCallback = (PEosToolpath_SetSetFocusCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetFocusCallback");
	EosToolpath_SetSetSkywritingCallback = (PEosToolpath_SetSetSkywritingCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetSkywritingCallback");
	EosToolpath_SetCustomDataCallback = (PEosToolpath_SetCustomDataCallback)GetProcAddress(hLibrary, "EosToolpath_SetCustomDataCallback");
	EosToolpath_SetHatchCallback = (PEosToolpath_SetHatchCallback)GetProcAddress(hLibrary, "EosToolpath_SetHatchCallback");
	EosToolpath_SetPolyStartCallback = (PEosToolpath_SetPolyStartCallback)GetProcAddress(hLibrary, "EosToolpath_SetPolyStartCallback");
	EosToolpath_SetPolyCoordCallback = (PEosToolpath_SetPolyCoordCallback)GetProcAddress(hLibrary, "EosToolpath_SetPolyCoordCallback");
	EosToolpath_SetPolyEndCallback = (PEosToolpath_SetPolyEndCallback)GetProcAddress(hLibrary, "EosToolpath_SetPolyEndCallback");
	EosToolpath_SetDelayCallback = (PEosToolpath_SetDelayCallback)GetProcAddress(hLibrary, "EosToolpath_SetDelayCallback");
	EosToolpath_SetSetExposureTypeCallback = (PEosToolpath_SetSetExposureTypeCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetExposureTypeCallback");
	EosToolpath_SetSetExposureStepCallback = (PEosToolpath_SetSetExposureStepCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetExposureStepCallback");
	EosToolpath_SetSetDepthCallback = (PEosToolpath_SetSetDepthCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetDepthCallback");
    EosToolpath_SetStartStripeSegmentCallback = (PEosToolpath_SetStartStripeSegmentCallback)GetProcAddress(hLibrary, "EosToolpath_SetStartStripeSegmentCallback");
	EosToolpath_SetStartGeneralSegmentCallback = (PEosToolpath_SetStartGeneralSegmentCallback)GetProcAddress(hLibrary, "EosToolpath_SetStartGeneralSegmentCallback");
	EosToolpath_SetEndSegmentCallback = (PEosToolpath_SetEndSegmentCallback)GetProcAddress(hLibrary, "EosToolpath_SetEndSegmentCallback");
	EosToolpath_SetEndLineCallback = (PEosToolpath_SetEndLineCallback)GetProcAddress(hLibrary, "EosToolpath_SetEndLineCallback");
	EosToolpath_SetSetLaserPulseCallback = (PEosToolpath_SetSetLaserPulseCallback)GetProcAddress(hLibrary, "EosToolpath_SetSetLaserPulseCallback");

}

void CEOSSDK::resetPtrs()
{
    m_pSimulatorInstance = nullptr;
    Eos_GetApiVersion = nullptr;
    Eos_GetProductVersion = nullptr;
    Eos_InitializeApi = nullptr;
    Eos_DeinitializeApi = nullptr;
    EosTaskGen_LoadOpenJz = nullptr;
    EosTaskGen_UnloadJob = nullptr;
    EosTaskGen_IsJobLoaded = nullptr;
    EosTaskGen_BeginBuildTimeEstimation = nullptr;
    EosTaskGen_AbortBuildTimeEstimation = nullptr;
    EosTaskGen_GetBuildTimeEstimationState = nullptr;
    EosTaskGen_WaitForBuildTimeEstimation = nullptr;
    EosTaskGen_GetBuildTimeEstimation = nullptr;
    EosTaskGen_AbortTaskCreation = nullptr;
    EosTaskGen_GetTaskCreationState = nullptr;
    EosTaskGen_WaitForTaskCreation = nullptr;
    EosTaskGen_GetCurrentLayerIndex = nullptr;
    EosTaskGen_GetStartLayerIndex = nullptr;
    EosTaskGen_GetTargetLayerIndex = nullptr;
    EosTaskGen_GetCurrentZHeight = nullptr;
    EosTaskGen_GetStartZHeight = nullptr;
    EosTaskGen_GetTargetZHeight = nullptr;
    EosTaskGen_SetNumberOfTaskCreationThreads = nullptr;
    EosTaskGen_LoadMachineConfiguration = nullptr;
    EosThreads_GetNumberOfAvailableThreads = nullptr;

    Eos_ResetAllCallbacks = nullptr;
    EosTaskGen_UnloadMachineConfiguration = nullptr;
    EosTaskGen_IsMachineConfigLoaded = nullptr;
    EosTaskGen_SetCacheDirectory = nullptr;
    EosTaskGen_SetStateChangeCallback = nullptr;
    EosTaskGen_SetProgressCallback = nullptr;
    EosTaskGen_SetBuildTimeStateChangedCallback = nullptr;

    EosMachineSession_Create = nullptr;
    EosMachineSession_CreateWithHint = nullptr;
    EosMachineSession_Destroy = nullptr;
    EosMachineSession_BeginConfigDownload = nullptr;
    EosMachineSession_BeginTaskUpload = nullptr;
    EosMachineSession_AbortFileTransfer = nullptr;
    EosMachineSession_AbortAllFileTransfers = nullptr;
    EosMachineSession_GetFileTransferState = nullptr;
    EosMachineSession_WaitForFileTransfer = nullptr;
    EosMachineSession_GetFileTransferResult = nullptr;
    EosMachineSession_WaitForAllFileTransfers = nullptr;
    EosMachineSession_ReleaseFileTransferHandle = nullptr;
    EosMachineSession_SetFileTransferProgressCallback = nullptr;

    EosToolpath_GetAuthChallenge = nullptr;
    EosToolpath_SetAuthResponse = nullptr;
    EosToolpath_OpenOutputFile = nullptr;
    EosToolpath_ChopOutput = nullptr;
    EosToolpath_OpenInputFile = nullptr;
    EosToolpath_CloseFile = nullptr;
    EosToolpath_ReadLayer = nullptr;
    EosToolpath_GetNumberOfLayers = nullptr;
    EosToolpath_WriteBeginLayer = nullptr;
    EosToolpath_WriteBeginPart = nullptr;
    EosToolpath_WriteSelectLaserScanner = nullptr;
    EosToolpath_WriteSetLaserPower = nullptr;
    EosToolpath_WriteSetScanSpeed = nullptr;
    EosToolpath_WriteSetFocus = nullptr;
    EosToolpath_WriteSetSkywriting = nullptr;
    EosToolpath_WriteCustomData = nullptr;
    EosToolpath_WriteHatch = nullptr;
    EosToolpath_WritePolyStart = nullptr;
    EosToolpath_WritePolyCoord = nullptr;
    EosToolpath_WritePolyEnd = nullptr;
    EosToolpath_WriteDelay = nullptr;
    EosToolpath_WriteSetExposureType = nullptr;
    EosToolpath_WriteSetExposureStep = nullptr;
    EosToolpath_WriteSetDepth = nullptr;
    EosToolpath_WriteStartStripeSegment = nullptr;
    EosToolpath_WriteStartGeneralSegment = nullptr;
    EosToolpath_WriteEndSegment = nullptr;
    EosToolpath_WriteEndLine = nullptr;
    EosToolpath_WriteSetLaserPulse = nullptr;
    EosToolpath_SetBeginLayerCallback = nullptr;
    EosToolpath_SetEndLayerCallback = nullptr;
    EosToolpath_SetBeginPartCallback = nullptr;
    EosToolpath_SetSelectLaserScannerCallback = nullptr;
    EosToolpath_SetSetLaserPowerCallback = nullptr;
    EosToolpath_SetSetScanSpeedCallback = nullptr;
    EosToolpath_SetSetFocusCallback = nullptr;
    EosToolpath_SetSetSkywritingCallback = nullptr;
    EosToolpath_SetCustomDataCallback = nullptr;
    EosToolpath_SetHatchCallback = nullptr;
    EosToolpath_SetPolyStartCallback = nullptr;
    EosToolpath_SetPolyCoordCallback = nullptr;
    EosToolpath_SetPolyEndCallback = nullptr;
    EosToolpath_SetDelayCallback = nullptr;
    EosToolpath_SetSetExposureTypeCallback = nullptr;
    EosToolpath_SetSetExposureStepCallback = nullptr;
    EosToolpath_SetSetDepthCallback = nullptr;
    EosToolpath_SetStartStripeSegmentCallback = nullptr;
    EosToolpath_SetStartGeneralSegmentCallback = nullptr;
    EosToolpath_SetEndSegmentCallback = nullptr;
    EosToolpath_SetEndLineCallback = nullptr;
    EosToolpath_SetSetLaserPulseCallback = nullptr;

    if (m_pAdapter.get() != nullptr)
        m_pAdapter->loadEntryPoints(nullptr);

}

std::string CEOSSDK::getSDKDirectory()
{
    return m_sSDKDirectory;
}


void CEOSSDK::checkEOSError(EOS_ERROR eosError)
{
    if (eosError != EOS_ERROR::EOS_ERR_NO_ERROR)
        throw ELib3MF4EOSInterfaceException (LIB3MF4EOS_ERROR_EOSSDKERROR, "EOS SDK Error: " + eosErrorToStringVerbose (eosError));
}

void CEOSSDK::setToSimulationMode(eSimulationMode simulationMode)
{
    m_pSimulatorInstance = std::make_shared<CEOSSDKSimulatorInstance>(getAdapter().getVersionString(), simulationMode);
}

bool CEOSSDK::simulateConnectivity()
{
    if (m_pSimulatorInstance.get() != nullptr) {

        auto simulationMode = m_pSimulatorInstance->getSimulationMode();
        return (simulationMode == eSimulationMode::smSimulateConnectivity) || (simulationMode == eSimulationMode::smSimulateFullSDK) ||
            (simulationMode == eSimulationMode::smSimulatePlasticsConnectivity) || (simulationMode == eSimulationMode::smSimulatePlasticsFullSDK) ||
            (simulationMode == eSimulationMode::smRecording);

    }
    else
        return false;

}

bool CEOSSDK::simulateFullSDK()
{
    if (m_pSimulatorInstance.get() != nullptr) {

        auto simulationMode = m_pSimulatorInstance->getSimulationMode();
        return (simulationMode == eSimulationMode::smSimulateFullSDK) || (simulationMode == eSimulationMode::smSimulatePlasticsFullSDK) ||
            (simulationMode == eSimulationMode::smRecording);

    }
    else
        return false;

}

PToolpathRecorder CEOSSDK::getToolpathRecorder()
{
    if (m_pSimulatorInstance.get() == nullptr)
        return nullptr;
    return m_pSimulatorInstance->getToolpathRecorder();
}

bool CEOSSDK::canWriteBeamShape()
{
    auto& adapter = getAdapter();
    if (getToolpathRecorder().get() != nullptr)
        return adapter.versionHasBeamShape();
    return adapter.supportsBeamShape();
}

bool CEOSSDK::simulatePlastics()
{
    if (m_pSimulatorInstance.get() != nullptr) {

        auto simulationMode = m_pSimulatorInstance->getSimulationMode();
        return (simulationMode == eSimulationMode::smSimulatePlasticsConnectivity) || (simulationMode == eSimulationMode::smSimulatePlasticsFullSDK);

    }
    else
        return false;

}


CEOSSDKSimulatorInstance* CEOSSDK::getSimulator()
{
    if (m_pSimulatorInstance.get () == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKISNOTINSIMULATIONMODE);

    return m_pSimulatorInstance.get();
}

