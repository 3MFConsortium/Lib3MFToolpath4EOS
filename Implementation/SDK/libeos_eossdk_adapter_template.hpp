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


Abstract: Adapter implementation shared by all SDK versions. The version structures are
accessed by member name, which is the same in all versions; members that only exist in
some versions are converted by the version adapters in the Extras hooks.

*/


#ifndef __LIBEOS_EOSSDK_ADAPTER_TEMPLATE
#define __LIBEOS_EOSSDK_ADAPTER_TEMPLATE

#include "../libeos_eossdk_adapter.hpp"

#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

namespace Lib3MF4EOS {
    namespace Impl {

        namespace AdapterConversion {

            // Upper bound for array sizes reported by the SDK, to reject corrupt structures.
            constexpr uint64_t MAXARRAYSIZE = 1000000;

            inline std::wstring toWString(const wchar_t* pString)
            {
                if (pString == nullptr)
                    return std::wstring();
                return std::wstring(pString);
            }

            inline const wchar_t* toCWSTR(const std::wstring& sString)
            {
                if (sString.empty())
                    return nullptr;
                return sString.c_str();
            }

            template <typename TVersion> sEOSSDKVersion convertVersion(const TVersion& version)
            {
                sEOSSDKVersion result;
                result.m_nMajor = (uint32_t)version.major;
                result.m_nMinor = (uint32_t)version.minor;
                result.m_nBuild = (uint32_t)version.build;
                result.m_nRevision = (uint32_t)version.revision;
                return result;
            }

            template <typename TLaserScannerInfo> sEOSLaserScannerInfo convertLaserScannerInfo(const TLaserScannerInfo& info)
            {
                sEOSLaserScannerInfo result;
                result.m_dLaserPowerNominal = info.laserPowerNominal_W;
                result.m_dLaserPowerMax = info.laserPowerMax_W;
                result.m_dMaxScanSpeed = info.maxScanSpeed_mm_per_s;
                result.m_dMinX = info.minPoint2D.x_mm;
                result.m_dMinY = info.minPoint2D.y_mm;
                result.m_dMaxX = info.maxPoint2D.x_mm;
                result.m_dMaxY = info.maxPoint2D.y_mm;
                result.m_nCapabilities = (uint64_t)info.capabilities;
                return result;
            }

            template <typename TMachineInfo> void convertMachineInfo(const TMachineInfo& info, sEOSMachineInfo& result)
            {
                result.m_VolumeType = (EOS_VOLUME_TYPE)info.volumeType;
                result.m_dDimX = info.dimX;
                result.m_dDimY = info.dimY;
                result.m_dDimZ = info.dimZ;
                result.m_dDimR = info.dimR;
                result.m_ModelType = (EOS_MODEL_TYPE)info.modelType;
                result.m_MachineSoftwareVersion = convertVersion(info.machineSoftwareVersion);
                result.m_MachineInterfaceVersion = convertVersion(info.machineInterfaceVersion);
                result.m_sSerialNumber = toWString(info.serialNumber);
                result.m_sPrintDomain = toWString(info.printDomain);

                result.m_nNumberOfLaserScanners = (info.numberOfLaserScanners > 0) ? (uint32_t)info.numberOfLaserScanners : 0;
                result.m_LaserScanners.clear();
                if ((info.laserScannerInfo != nullptr) && (result.m_nNumberOfLaserScanners < MAXARRAYSIZE)) {
                    for (uint32_t nIndex = 0; nIndex < result.m_nNumberOfLaserScanners; nIndex++)
                        result.m_LaserScanners.push_back(convertLaserScannerInfo(info.laserScannerInfo[nIndex]));
                }
            }

            template <typename TExposureSetInfo> sEOSExposureSetInfo convertExposureSetInfo(const TExposureSetInfo& info)
            {
                sEOSExposureSetInfo result;
                result.m_sName = toWString(info.name);
                result.m_nLayerThickness = info.layerThickness;
                result.m_bIsEditable = (info.isEditable != 0);
                result.m_bIsSupport = (info.isSupport != 0);
                result.m_bUsesSmartFusion = (info.usesSmartFusion != 0);
                return result;
            }

            template <typename TMaterialSetInfo> void convertMaterialSetInfo(const TMaterialSetInfo& info, sEOSMaterialSetInfo& result)
            {
                result.m_sName = toWString(info.name);
                result.m_Version = convertVersion(info.version);
                result.m_sUUID = toWString(info.uuid);
                result.m_sParentUUID = toWString(info.parentUuid);
                result.m_ModelType = (EOS_MODEL_TYPE)info.modelType;
                result.m_sMaterial = toWString(info.material);
                result.m_nBaseLayerThickness = info.baseLayerThickness;
                result.m_InertGas = (EOS_INERT_GAS)info.inertGas;
                result.m_bIsAuthoredByEos = (info.isAuthoredByEos != 0);
                result.m_bIsFinal = (info.isFinal != 0);

                result.m_ExposureSets.clear();
                if ((info.exposureSets != nullptr) && (info.exposureSetsSize < MAXARRAYSIZE)) {
                    for (uint64_t nIndex = 0; nIndex < info.exposureSetsSize; nIndex++)
                        result.m_ExposureSets.push_back(convertExposureSetInfo(info.exposureSets[nIndex]));
                }
            }

            // The returned structure points into metaData, which must outlive it.
            template <typename TTaskMetaData> TTaskMetaData createTaskMetaData(const sEOSTaskMetaData& metaData)
            {
                TTaskMetaData result;
                memset((void*)&result, 0, sizeof(result));

                result.name = toCWSTR(metaData.m_sName);
                result.comment = toCWSTR(metaData.m_sComment);
                result.numberOfAllowedBuilds = metaData.m_nNumberOfAllowedBuilds;
                result.generatorName = toCWSTR(metaData.m_sGeneratorName);
                result.generatorVersion = toCWSTR(metaData.m_sGeneratorVersion);
                result.taskMode = (decltype(result.taskMode))metaData.m_TaskMode;
                result.partBaseDirectory = toCWSTR(metaData.m_sPartBaseDirectory);
                result.taskFormat = (decltype(result.taskFormat))metaData.m_TaskFormat;
                result.compression = (decltype(result.compression))metaData.m_Compression;
                return result;
            }

            template <typename TBuildTimeInfo> sEOSBuildTimeInfo convertBuildTimeInfo(const TBuildTimeInfo& info)
            {
                sEOSBuildTimeInfo result;
                result.m_dBuildTimeMs = info.buildTimesMs;
                result.m_nNumberOfCalculatedLayers = info.numberOfCalculatedLayers;
                result.m_nNumberOfAllLayers = info.numberOfAllLayers;
                result.m_dExposureTimeMs = info.exposureTimesMs;
                result.m_dNonExposureTimeMs = info.nonExposureTimesMs;
                result.m_nCurrentLayerNumber = info.currentLayerNumber;
                if ((info.exposureTimePerLaserScannerMs != nullptr) && (info.numberOfLaserScanners < MAXARRAYSIZE)) {
                    for (uint64_t nIndex = 0; nIndex < info.numberOfLaserScanners; nIndex++)
                        result.m_ExposureTimePerLaserScannerMs.push_back(info.exposureTimePerLaserScannerMs[nIndex]);
                }
                return result;
            }

            template <typename TInfoItem> sEOSEventInfoItem convertEventInfoItem(const TInfoItem& item)
            {
                sEOSEventInfoItem result;
                result.m_sKey = toWString(item.key);
                switch ((EOS_VARIANT_TYPE)item.type) {
                case EOS_VAR_INT: result.m_sValue = std::to_wstring(item.value.intVariant); break;
                case EOS_VAR_DOUBLE: result.m_sValue = std::to_wstring(item.value.doubleVariant); break;
                case EOS_VAR_CWSTR: result.m_sValue = toWString(item.value.cWStrVariant); break;
                default: break;
                }
                return result;
            }

            template <typename TEvent> sEOSEvent convertEvent(const TEvent& event)
            {
                sEOSEvent result;
                result.m_nModuleID = event.moduleId;
                result.m_nEventID = event.eventId;
                result.m_ErrorCode = (EOS_ERROR)event.errorCode;
                result.m_Severity = (EOS_SEVERITY)event.severity;
                result.m_DisplayOptions = (EOS_DISPLAY_OPTIONS)event.displayOptions;
                result.m_sName = toWString(event.name);
                if ((event.infoItems != nullptr) && (event.infoItemCount < MAXARRAYSIZE)) {
                    for (uint32_t nIndex = 0; nIndex < event.infoItemCount; nIndex++)
                        result.m_InfoItems.push_back(convertEventInfoItem(event.infoItems[nIndex]));
                }
                return result;
            }

            template <typename TLayerInfo> sEOSLayerInfo convertLayerInfo(const TLayerInfo& info)
            {
                sEOSLayerInfo result;
                result.m_nLayerIndex = (uint64_t)info.layerIndex;
                result.m_nDeltaInMicrometers = (int32_t)info.delta_um;
                result.m_nZPositionInMicrometers = (int32_t)info.zPosition_um;
                result.m_bIsExposed = (info.isExposed != 0);
                if ((info.exposedPartIds != nullptr) && ((uint64_t)info.exposedPartsCount < MAXARRAYSIZE)) {
                    for (uint64_t nIndex = 0; nIndex < (uint64_t)info.exposedPartsCount; nIndex++)
                        result.m_ExposedPartIDs.push_back((int32_t)info.exposedPartIds[nIndex]);
                }
                return result;
            }

            template <typename TProcessHeights> sEOSProcessHeights convertProcessHeights(const TProcessHeights& heights)
            {
                sEOSProcessHeights result;
                result.m_dPreProcessHeight = heights.preProcessHeight;
                result.m_dBottomOffsetHeight = heights.bottomOffsetHeight;
                result.m_dPayloadHeight = heights.payloadHeight;
                result.m_dTopOffsetHeight = heights.topOffsetHeight;
                result.m_dPostProcessHeight = heights.postProcessHeight;
                result.m_dAccumulatedShrinkageCompensation = heights.accumulatedShrinkageCompensation;
                result.m_dTotalZTravel = heights.totalZTravel;
                result.m_dMaxZTravel = heights.maxZTravel;
                return result;
            }

            // The SDK owns the returned layer array.
            template <typename TError, typename TLayerInfo, typename TSize>
            EOS_ERROR callGetLayers(TError(*pGetLayers)(const TLayerInfo**, TSize*), std::vector<sEOSLayerInfo>& layers)
            {
                layers.clear();
                if (pGetLayers == nullptr)
                    return EOS_ERR_UNSUPPORTED;

                const TLayerInfo* pLayerInfos = nullptr;
                TSize nLayerCount = 0;
                auto error = (EOS_ERROR)pGetLayers(&pLayerInfos, &nLayerCount);
                if (error != EOS_ERR_NO_ERROR)
                    return error;
                if (((uint64_t)nLayerCount > 0) && (pLayerInfos == nullptr))
                    return EOS_ERR_EMPTY;
                if ((uint64_t)nLayerCount >= MAXARRAYSIZE)
                    return EOS_ERR_EMPTY;

                for (uint64_t nIndex = 0; nIndex < (uint64_t)nLayerCount; nIndex++)
                    layers.push_back(convertLayerInfo(pLayerInfos[nIndex]));
                return EOS_ERR_NO_ERROR;
            }

            template <typename TError, typename TProcessHeights>
            EOS_ERROR callGetProcessHeights(TError(*pGetProcessHeights)(TProcessHeights*), sEOSProcessHeights& processHeights)
            {
                if (pGetProcessHeights == nullptr)
                    return EOS_ERR_UNSUPPORTED;

                TProcessHeights heights;
                memset((void*)&heights, 0, sizeof(heights));
                auto error = (EOS_ERROR)pGetProcessHeights(&heights);
                if (error != EOS_ERR_NO_ERROR)
                    return error;

                processHeights = convertProcessHeights(heights);
                return EOS_ERR_NO_ERROR;
            }

            // Callback arguments are passed by value in some versions and by pointer in others.
            template <typename T> const T* argumentPointer(const T& argument) { return &argument; }
            template <typename T> const T* argumentPointer(const T* pArgument) { return pArgument; }

        }


        /*************************************************************************************************************************
         TTraits must provide: EntryPoints, MachineInfo, MaterialSetInfo, TaskMetaData, DataHandle,
         BuildTimeCallbackArgument and EventCallbackArgument (the second parameter types of the version's
         EOS_BUILD_TIME_PROGRESS_CALLBACK and EOS_EVENT_CALLBACK).
        **************************************************************************************************************************/

        template <typename TTraits> class CEOSSDKAdapterTemplate : public CEOSSDKAdapter {

        protected:
            typename TTraits::EntryPoints m_EntryPoints;

            virtual void convertMachineInfoExtras(const typename TTraits::MachineInfo& info, sEOSMachineInfo& machineInfo)
            {
            }

            virtual void convertMaterialSetInfoExtras(const typename TTraits::MaterialSetInfo& info, sEOSMaterialSetInfo& materialSetInfo)
            {
            }

            static void buildTimeInfoTrampoline(typename TTraits::DataHandle hCustomDataHandle, typename TTraits::BuildTimeCallbackArgument argument)
            {
                auto pAdapter = reinterpret_cast<CEOSSDKAdapterTemplate<TTraits>*> ((uintptr_t)hCustomDataHandle);
                auto pInfo = AdapterConversion::argumentPointer(argument);
                if ((pAdapter != nullptr) && (pInfo != nullptr))
                    pAdapter->dispatchBuildTimeInfo(AdapterConversion::convertBuildTimeInfo(*pInfo));
            }

            static void eventTrampoline(typename TTraits::DataHandle hCustomDataHandle, typename TTraits::EventCallbackArgument argument)
            {
                auto pAdapter = reinterpret_cast<CEOSSDKAdapterTemplate<TTraits>*> ((uintptr_t)hCustomDataHandle);
                auto pEvent = AdapterConversion::argumentPointer(argument);
                if ((pAdapter != nullptr) && (pEvent != nullptr))
                    pAdapter->dispatchEvent(AdapterConversion::convertEvent(*pEvent));
            }

            EOS_ERROR convertMachineInfoAndFree(const typename TTraits::MachineInfo* pInfo, sEOSMachineInfo& machineInfo, bool bFree)
            {
                if (pInfo == nullptr)
                    return EOS_ERR_EMPTY;

                try {
                    AdapterConversion::convertMachineInfo(*pInfo, machineInfo);
                    convertMachineInfoExtras(*pInfo, machineInfo);
                }
                catch (...) {
                    if (bFree)
                        m_EntryPoints.EosFileProperties_FreeMachineInfo(pInfo);
                    throw;
                }

                if (bFree)
                    m_EntryPoints.EosFileProperties_FreeMachineInfo(pInfo);

                return EOS_ERR_NO_ERROR;
            }

        public:

            void loadEntryPoints(void* hLibrary) override
            {
                m_EntryPoints = typename TTraits::EntryPoints();
                if (hLibrary != nullptr)
                    m_EntryPoints.load(hLibrary);
            }

            std::vector<std::string> getMissingRequiredEntryPoints() const override
            {
                std::vector<std::string> missing;
                if (m_EntryPoints.EosFileProperties_LoadMaterialSetInfo == nullptr) missing.push_back("EosFileProperties_LoadMaterialSetInfo");
                if (m_EntryPoints.EosFileProperties_FreeMaterialSetInfo == nullptr) missing.push_back("EosFileProperties_FreeMaterialSetInfo");
                if (m_EntryPoints.EosFileProperties_LoadMachineInfo == nullptr) missing.push_back("EosFileProperties_LoadMachineInfo");
                if (m_EntryPoints.EosFileProperties_FreeMachineInfo == nullptr) missing.push_back("EosFileProperties_FreeMachineInfo");
                if (m_EntryPoints.EosTaskGen_GetMachineInfo == nullptr) missing.push_back("EosTaskGen_GetMachineInfo");
                if (m_EntryPoints.EosTaskGen_BeginTaskCreation == nullptr) missing.push_back("EosTaskGen_BeginTaskCreation");
                if (m_EntryPoints.EosTaskGen_SetNewBuildTimeEstimateCallback == nullptr) missing.push_back("EosTaskGen_SetNewBuildTimeEstimateCallback");
                if (m_EntryPoints.Eos_SetEventCallback == nullptr) missing.push_back("Eos_SetEventCallback");
                return missing;
            }

            EOS_ERROR loadMaterialSetInfo(const std::wstring& sPathToEOSPar, sEOSMaterialSetInfo& materialSetInfo) override
            {
                if ((m_EntryPoints.EosFileProperties_LoadMaterialSetInfo == nullptr) || (m_EntryPoints.EosFileProperties_FreeMaterialSetInfo == nullptr))
                    return EOS_ERR_UNSUPPORTED;

                const typename TTraits::MaterialSetInfo* pInfo = nullptr;
                auto error = (EOS_ERROR)m_EntryPoints.EosFileProperties_LoadMaterialSetInfo(sPathToEOSPar.c_str(), &pInfo);
                if (error != EOS_ERR_NO_ERROR)
                    return error;
                if (pInfo == nullptr)
                    return EOS_ERR_EMPTY;

                try {
                    AdapterConversion::convertMaterialSetInfo(*pInfo, materialSetInfo);
                    convertMaterialSetInfoExtras(*pInfo, materialSetInfo);
                }
                catch (...) {
                    m_EntryPoints.EosFileProperties_FreeMaterialSetInfo(pInfo);
                    throw;
                }

                m_EntryPoints.EosFileProperties_FreeMaterialSetInfo(pInfo);
                return EOS_ERR_NO_ERROR;
            }

            EOS_ERROR loadMachineInfo(const std::wstring& sPathToConfigDirectory, sEOSMachineInfo& machineInfo) override
            {
                if ((m_EntryPoints.EosFileProperties_LoadMachineInfo == nullptr) || (m_EntryPoints.EosFileProperties_FreeMachineInfo == nullptr))
                    return EOS_ERR_UNSUPPORTED;

                const typename TTraits::MachineInfo* pInfo = nullptr;
                auto error = (EOS_ERROR)m_EntryPoints.EosFileProperties_LoadMachineInfo(sPathToConfigDirectory.c_str(), &pInfo);
                if (error != EOS_ERR_NO_ERROR)
                    return error;

                return convertMachineInfoAndFree(pInfo, machineInfo, true);
            }

            EOS_ERROR getMachineInfo(sEOSMachineInfo& machineInfo) override
            {
                if (m_EntryPoints.EosTaskGen_GetMachineInfo == nullptr)
                    return EOS_ERR_UNSUPPORTED;

                const typename TTraits::MachineInfo* pInfo = nullptr;
                auto error = (EOS_ERROR)m_EntryPoints.EosTaskGen_GetMachineInfo(&pInfo);
                if (error != EOS_ERR_NO_ERROR)
                    return error;

                // The SDK owns the structure returned by EosTaskGen_GetMachineInfo.
                return convertMachineInfoAndFree(pInfo, machineInfo, false);
            }

            EOS_ERROR getLayers(std::vector<sEOSLayerInfo>& layers) override
            {
                return AdapterConversion::callGetLayers(m_EntryPoints.EosTaskGen_GetLayers, layers);
            }

            EOS_ERROR getProcessHeights(sEOSProcessHeights& processHeights) override
            {
                return AdapterConversion::callGetProcessHeights(m_EntryPoints.EosTaskGen_GetProcessHeights, processHeights);
            }

            EOS_ERROR beginTaskCreation(const sEOSTaskMetaData& metaData, const std::wstring& sTargetDirectory) override
            {
                if (m_EntryPoints.EosTaskGen_BeginTaskCreation == nullptr)
                    return EOS_ERR_UNSUPPORTED;

                auto sdkMetaData = AdapterConversion::createTaskMetaData<typename TTraits::TaskMetaData>(metaData);
                return (EOS_ERROR)m_EntryPoints.EosTaskGen_BeginTaskCreation(&sdkMetaData, sTargetDirectory.c_str());
            }

            EOS_ERROR getTaskCreationResult() override
            {
                if (m_EntryPoints.EosTaskGen_GetTaskCreationResult == nullptr)
                    return EOS_ERR_UNSUPPORTED;
                return (EOS_ERROR)m_EntryPoints.EosTaskGen_GetTaskCreationResult();
            }

            bool supportsTaskCreationFromToolpath() const override
            {
                return (m_EntryPoints.EosTaskGen_BeginTaskCreationFromToolpath != nullptr);
            }

            EOS_ERROR beginTaskCreationFromToolpath(EOS_FILE_HANDLE hFileHandle, const sEOSTaskMetaData& metaData, const std::wstring& sTargetDirectory) override
            {
                if (m_EntryPoints.EosTaskGen_BeginTaskCreationFromToolpath == nullptr)
                    return EOS_ERR_UNSUPPORTED;

                auto sdkMetaData = AdapterConversion::createTaskMetaData<typename TTraits::TaskMetaData>(metaData);
                return (EOS_ERROR)m_EntryPoints.EosTaskGen_BeginTaskCreationFromToolpath(hFileHandle, &sdkMetaData, sTargetDirectory.c_str());
            }

            EOS_ERROR setBuildTimeInfoCallback(PEOSBuildTimeInfoCallback pCallback, void* pUserData) override
            {
                if (m_EntryPoints.EosTaskGen_SetNewBuildTimeEstimateCallback == nullptr)
                    return EOS_ERR_UNSUPPORTED;

                storeBuildTimeInfoCallback(pCallback, pUserData);

                if (pCallback != nullptr)
                    return (EOS_ERROR)m_EntryPoints.EosTaskGen_SetNewBuildTimeEstimateCallback(&buildTimeInfoTrampoline, (typename TTraits::DataHandle) (uintptr_t)this);
                else
                    return (EOS_ERROR)m_EntryPoints.EosTaskGen_SetNewBuildTimeEstimateCallback(nullptr, 0);
            }

            EOS_ERROR setEventCallback(PEOSEventCallback pCallback, void* pUserData) override
            {
                if (m_EntryPoints.Eos_SetEventCallback == nullptr)
                    return EOS_ERR_UNSUPPORTED;

                storeEventCallback(pCallback, pUserData);

                if (pCallback != nullptr)
                    return (EOS_ERROR)m_EntryPoints.Eos_SetEventCallback(&eventTrampoline, (typename TTraits::DataHandle) (uintptr_t)this);
                else
                    return (EOS_ERROR)m_EntryPoints.Eos_SetEventCallback(nullptr, 0);
            }

        };

    } // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_EOSSDK_ADAPTER_TEMPLATE
