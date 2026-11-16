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


Abstract: This is a stub class definition of CEOSContext

*/

#include "libeos_eoscontext.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_eosmaterialset.hpp"
#include "libeos_eosmachineinfo.hpp"
#include "libeos_sslcrypt.hpp"
#include "libeos_eosrawtaskwriter.hpp"
#include "libeos_eosrawtasklayer.hpp"
#include "libeos_eostoolpathconversionsettings.hpp"
#include "libeos_eosconversionlog.hpp"
#include "libeos_jsonwriter.hpp"

#include "libeos_taskcreationfuture.hpp"
#include "libeos_buildtimeestimationfuture.hpp"

#include "libeos_workingdirectory.hpp"
#include "libeos_3mfjobloader.hpp"
#include "libeos_machineconfigfuture.hpp"
#include "libeos_machinesessionfuture.hpp"
#include "libeos_eossdksimulator.hpp"
#include "libeos_toolpathcomputationfuture.hpp"
#include "libeos_eosto3mfconverter.hpp"
#include "libeos_openjzreader.hpp"
#include "libeos_toolpathprofileparameters.hpp"

#include "../Common/common_importstream_native.hpp"
#include "../Common/common_utils.hpp"

#include <stdexcept>

// Include custom headers here.
#include <Windows.h>
#include <Shlwapi.h>

#include <vector>
#include <iostream>
#include <string>
#include <locale>
#include <fstream>
#include <sstream>
#include <deque>
#include <map>
#include <set>
#include <cmath>
#include <cstring>

using namespace Lib3MF4EOS::Impl;





/*************************************************************************************************************************
 Class definition of CEOSContext 
**************************************************************************************************************************/

CEOSContext::CEOSContext(const std::string& sSDKDirectory, const std::string& sLogFileName)
	: m_pSDK(nullptr), m_sDefaultSDKDirectory (sSDKDirectory), m_nSDKVersionOverrideMajor (0), m_nSDKVersionOverrideMinor (0), m_bSDKAPIIsInitialized (false)
{
	if (!sLogFileName.empty())
		m_sDefaultLogFileName = AMCCommon::CUtils::getFullPathName(sLogFileName, false);
}

CEOSContext::~CEOSContext()
{
	if (!m_sLoggingConfigFile.empty())
		AMCCommon::CUtils::deleteFileFromDisk(m_sLoggingConfigFile, false);

	if (m_sDefaultLogFileName.empty() && !m_sLoggingLogFile.empty ())
		AMCCommon::CUtils::deleteFileFromDisk(m_sLoggingLogFile, false);
	UnloadSDK();
}


void CEOSContext::UnloadSDK()
{
	if ((m_pSDK.get() != nullptr) && m_bSDKAPIIsInitialized)
		m_pSDK->Eos_DeinitializeApi();

	m_bSDKAPIIsInitialized = false;
	m_pSDK = nullptr;
}

std::string CEOSContext::findSDKDirectory(const std::string& sEOSPrintAPIDLLName)
{
	std::vector<std::pair<uint32_t, uint32_t>> versions;
	if (CEOSSDKAdapterRegistry::isKnownVersion(m_nSDKVersionOverrideMajor, m_nSDKVersionOverrideMinor))
		versions.push_back(std::make_pair(m_nSDKVersionOverrideMajor, m_nSDKVersionOverrideMinor));

	auto& registrations = CEOSSDKAdapterRegistry::getRegistrations();
	for (auto iIter = registrations.rbegin(); iIter != registrations.rend(); iIter++)
		versions.push_back(std::make_pair(iIter->m_nMajor, iIter->m_nMinor));

	for (auto& version : versions) {
		std::string sVersionDirectory = std::to_string(version.first) + "_" + std::to_string(version.second);

		std::vector<std::string> candidateDirectories;
		candidateDirectories.push_back("C:/Program Files/EOS/EOSPRINT " + std::to_string(version.first) + "/" + sVersionDirectory);
		candidateDirectories.push_back("C:/Program Files/EOS/EOSPRINT " + std::to_string(version.first) + " SDK/" + sVersionDirectory + "/bin/x64/release");

		for (auto& sCandidateDirectory : candidateDirectories) {
			std::string sDLLFileName = sCandidateDirectory + "/" + sEOSPrintAPIDLLName;
			if (AMCCommon::CUtils::fileOrPathExistsOnDisk(sDLLFileName.c_str()))
				return sCandidateDirectory;
		}
	}

	return "";
}

std::string CEOSContext::getDetectedVersionString()
{
	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	if (m_pSDK->getProductVersionSource() != eEOSSDKVersionSource::None)
		return m_pSDK->getProductVersion().toString();

	return m_pSDK->getAdapter().getVersionString();
}

void CEOSContext::LoadSDKIfNotExisting(eSimulationMode simulationMode)
{
	if (m_pSDK.get() != nullptr)
		return;

	std::string sEOSPrintAPIDLLName = "EosprintApi.dll";

	m_sSDKDirectory = m_sDefaultSDKDirectory;
	if (m_sSDKDirectory.empty())
		m_sSDKDirectory = findSDKDirectory(sEOSPrintAPIDLLName);

	bool bFullSDKSimulation = (simulationMode == eSimulationMode::smSimulateFullSDK) || (simulationMode == eSimulationMode::smSimulatePlasticsFullSDK) ||
		(simulationMode == eSimulationMode::smRecording);

	std::unique_ptr<CCurrentDirectoryOverride> pDirectoryOverride;

	if (m_sSDKDirectory.empty()) {
		if (!bFullSDKSimulation)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTFINDEOSSDKDIRECTORY, "could not find an EOS SDK of a supported version (" + CEOSSDKAdapterRegistry::getSupportedVersionsString() + ")");

		m_pSDK = std::make_shared<CEOSSDK>("", "", m_nSDKVersionOverrideMajor, m_nSDKVersionOverrideMinor);
	}
	else {
		pDirectoryOverride = std::make_unique<CCurrentDirectoryOverride>(m_sSDKDirectory);

		auto pSDK = std::make_shared<CEOSSDK>(m_sSDKDirectory, sEOSPrintAPIDLLName, m_nSDKVersionOverrideMajor, m_nSDKVersionOverrideMinor);

		uint32_t apiVersion = 0;
		pSDK->checkEOSError(pSDK->Eos_GetApiVersion(&apiVersion));
		uint32_t nExpectedAPIVersion = pSDK->getAdapter().getApiVersion();
		if (apiVersion != nExpectedAPIVersion)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_DETECTEDINCOMPATIBLEEOSSDKVERSION, "EOS SDK API version " + std::to_string(apiVersion) + " is not supported by the declarations of EOS SDK "
				+ pSDK->getAdapter().getVersionString() + ", which expect API version " + std::to_string(nExpectedAPIVersion));

		m_pSDK = pSDK;
	}

	if (simulationMode != eSimulationMode::smNoSimulation) 
		m_pSDK->setToSimulationMode(simulationMode);

	if (!m_pSDK->simulateFullSDK()) {

		m_sLoggingConfigFile = AMCCommon::CUtils::findTemporaryFileName(AMCCommon::CUtils::getTempFolder(), "libeos", ".config", 32);

		if (!m_sDefaultLogFileName.empty()) {
			m_sLoggingLogFile = m_sDefaultLogFileName;
		}
		else {
			m_sLoggingLogFile = AMCCommon::CUtils::findTemporaryFileName(AMCCommon::CUtils::getTempFolder(), "libeos", ".log", 32);
		}

		std::wstring sLoggingConfigFileW = AMCCommon::CUtils::UTF8toUTF16(m_sLoggingConfigFile);

		std::fstream fStream;
		fStream.open(sLoggingConfigFileW.c_str(), std::ios::out);

		if (!fStream.is_open())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDWRITELOGCONFIG, "could not write log config: " + m_sLoggingConfigFile);

		fStream << "# Rolling File Appender" << std::endl;
		fStream << "log4cplus.appender.FileAppender=log4cplus::RollingFileAppender" << std::endl;
		fStream << "log4cplus.appender.FileAppender.File=" << m_sLoggingLogFile << std::endl;
		fStream << "log4cplus.appender.FileAppender.MaxFileSize=10MB" << std::endl;
		fStream << "log4cplus.appender.FileAppender.MaxBackupIndex=10" << std::endl;
		fStream << "log4cplus.appender.FileAppender.layout=log4cplus::PatternLayout" << std::endl;
		fStream << "log4cplus.appender.FileAppender.layout.ConversionPattern=%D{%Y-%m-%d %H:%M:%S,%q} [%t] %-5p %c - %m%n" << std::endl;
		fStream << "log4cplus.appender.FileAppender.CreateDirs=true" << std::endl;
		fStream << std::endl;
		fStream << "# Win32 Debug Appender" << std::endl;
		fStream << "log4cplus.appender.DebugAppender=log4cplus::Win32DebugAppender" << std::endl;
		fStream << "log4cplus.appender.DebugAppender.layout=log4cplus::PatternLayout" << std::endl;
		fStream << "log4cplus.appender.DebugAppender.layout.ConversionPattern=%D{%Y-%m-%d %H:%M:%S,%q} [%t] %-5p %c - %m%n" << std::endl;
		fStream << std::endl;
		fStream << "# Generally log with error level to reduce noise," << std::endl;
		fStream << "log4cplus.rootLogger=ERROR, FileAppender, DebugAppender" << std::endl;
		fStream << std::endl;

		fStream.close();

		m_pSDK->checkEOSError(m_pSDK->Eos_InitializeApi(sLoggingConfigFileW.c_str()));
		m_bSDKAPIIsInitialized = true;

	}
	

}

std::string CEOSContext::LoadSDK()
{
	if (m_pSDK.get() != nullptr) 
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASALDREADYBEENLOADED);

	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	return getDetectedVersionString();
}

std::string CEOSContext::GetSDKFilePath()
{
	return m_sSDKDirectory;
}


IEOSSDKSimulator* CEOSContext::EnableSDKSimulation(const Lib3MF4EOS::eEOSSimulationMode eSimulationMode)
{
	if (m_pSDK.get() != nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASALDREADYBEENLOADED);
	
	Lib3MF4EOS::Impl::eSimulationMode implSimulationMode;
	switch (eSimulationMode) {
	case Lib3MF4EOS::eEOSSimulationMode::SimulateConnectivity: implSimulationMode = Lib3MF4EOS::Impl::eSimulationMode::smSimulateConnectivity; break;
	case Lib3MF4EOS::eEOSSimulationMode::SimulateFullSDK: implSimulationMode = Lib3MF4EOS::Impl::eSimulationMode::smSimulateFullSDK; break;
	case Lib3MF4EOS::eEOSSimulationMode::SimulatePlasticConnectivity: implSimulationMode = Lib3MF4EOS::Impl::eSimulationMode::smSimulatePlasticsConnectivity; break;
	case Lib3MF4EOS::eEOSSimulationMode::SimulatePlasticFullSDK: implSimulationMode = Lib3MF4EOS::Impl::eSimulationMode::smSimulatePlasticsFullSDK; break;
	case Lib3MF4EOS::eEOSSimulationMode::Recording: implSimulationMode = Lib3MF4EOS::Impl::eSimulationMode::smRecording; break;
	default:
		implSimulationMode = Lib3MF4EOS::Impl::eSimulationMode::smNoSimulation;
	}

	LoadSDKIfNotExisting(implSimulationMode);

	return new CEOSSDKSimulator(m_pSDK);
}



std::string CEOSContext::GetDetectedAPIVersion()
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	return getDetectedVersionString();

}

void CEOSContext::SetSDKVersionOverride(const Lib3MF4EOS_uint32 nMajor, const Lib3MF4EOS_uint32 nMinor)
{
	if (m_pSDK.get() != nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASALDREADYBEENLOADED);

	m_nSDKVersionOverrideMajor = nMajor;
	m_nSDKVersionOverrideMinor = nMinor;
}

void CEOSContext::GetSDKProductVersion(Lib3MF4EOS_uint32& nMajor, Lib3MF4EOS_uint32& nMinor, Lib3MF4EOS_uint32& nBuild, Lib3MF4EOS_uint32& nRevision)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	auto productVersion = m_pSDK->getProductVersion();
	nMajor = productVersion.m_nMajor;
	nMinor = productVersion.m_nMinor;
	nBuild = productVersion.m_nBuild;
	nRevision = productVersion.m_nRevision;
}

void CEOSContext::GetSDKAdapterVersion(Lib3MF4EOS_uint32& nMajor, Lib3MF4EOS_uint32& nMinor)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	auto& adapter = m_pSDK->getAdapter();
	nMajor = adapter.getVersionMajor();
	nMinor = adapter.getVersionMinor();
}

std::string CEOSContext::GetSupportedSDKVersions()
{
	return CEOSSDKAdapterRegistry::getSupportedVersionsString();
}

std::string CEOSContext::GetSDKVersionMessages()
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	std::string sMessages;
	for (auto& sMessage : m_pSDK->getVersionMessages()) {
		if (!sMessages.empty())
			sMessages += "\n";
		sMessages += sMessage;
	}
	return sMessages;
}


IEOSMaterialSet* CEOSContext::ImportMaterialSet(const std::string& sEOSParFileName)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	std::unique_ptr<CEOSMaterialSet> pMaterialSet;

	if (m_pSDK.get () == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	if (!m_pSDK->simulateFullSDK()) {


		std::wstring sEOSParFileNameW = AMCCommon::CUtils::UTF8toUTF16(sEOSParFileName);

		sEOSMaterialSetInfo materialSetInfo;
		EOS_ERROR error = m_pSDK->getAdapter().loadMaterialSetInfo(sEOSParFileNameW, materialSetInfo);
		if (error == EOS_ERR_EMPTY)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDMATERIALSETINFO);
		m_pSDK->checkEOSError(error);

		std::string sName = AMCCommon::CUtils::UTF16toUTF8(materialSetInfo.m_sName);
		std::string sUUID = AMCCommon::CUtils::UTF16toUTF8(materialSetInfo.m_sUUID);

		// Empty for model types unknown to LibEOS; jobs then need the machine target from elsewhere.
		std::string sModelType = eosModelTypeToMachineTarget(materialSetInfo.m_ModelType);

		std::string sMaterial = AMCCommon::CUtils::UTF16toUTF8(materialSetInfo.m_sMaterial);
		uint32_t nBaseLayerThickness = materialSetInfo.m_nBaseLayerThickness;
		auto& version = materialSetInfo.m_Version;

		pMaterialSet.reset(new CEOSMaterialSet(sName, sUUID, sMaterial, sModelType, nBaseLayerThickness, version.m_nMajor, version.m_nMinor, version.m_nBuild, version.m_nRevision));

		for (auto& exposureSet : materialSetInfo.m_ExposureSets) {
			std::string sSetName = AMCCommon::CUtils::UTF16toUTF8(exposureSet.m_sName);
			pMaterialSet->addExposureSet(sSetName, exposureSet.m_nLayerThickness, exposureSet.m_bIsSupport);
		}

		auto& buffer = pMaterialSet->getEOSParBuffer();

		std::ifstream eosParStream(sEOSParFileNameW.c_str(), std::ios::in | std::ios::binary);
		eosParStream.seekg(0, std::ios_base::end);
		auto nStreamSize = eosParStream.tellg();

		if (nStreamSize > 0) {
			buffer.resize(nStreamSize);
			eosParStream.seekg(0, std::ios_base::beg);
			eosParStream.read((char*)buffer.data(), nStreamSize);
		}

	}
	else {

		pMaterialSet.reset(new CEOSMaterialSet("Test Materialset", "8082c99a-4c5b-44a6-8259-eca1a291af91", "IN710", "M291", 30, 1, 0, 0, 0));
		pMaterialSet->addExposureSet("Test_Part_030", 30, false);
		pMaterialSet->addExposureSet("Test_Support_030", 30, true);

	}

	return pMaterialSet.release();
}

void CEOSContext::LoadOpenJZFromFile(const std::string& sFileName)
{
	std::string sFullPath = AMCCommon::CUtils::getFullPathName(sFileName, true);

	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	if (!m_pSDK->simulateFullSDK()) {

		std::wstring sPathToOpenJZW = AMCCommon::CUtils::UTF8toUTF16(sFullPath);

		m_pOpenJZTempPath = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_ojz");

		CCurrentDirectoryOverride directoryOverride(m_sSDKDirectory);

		EOS_ERROR error = m_pSDK->EosTaskGen_LoadOpenJz(sPathToOpenJZW.c_str(), m_pOpenJZTempPath->getPathW().c_str());
		m_pSDK->checkEOSError(error);

		// TODO: Make async?

	}
	else {
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

}

void CEOSContext::LoadJobFrom3MF(Lib3MF::PModel pModel)
{
	loadJobFrom3MF(pModel, S3MFJobWriteOptions());
}

void CEOSContext::loadJobFrom3MF(Lib3MF::PModel pModel, const S3MFJobWriteOptions& options)
{
	if (pModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model is null");

	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	auto pWorkingDir = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_3mf");
	std::string sOpenJZPath = pWorkingDir->getPath() + "/job.openjz";

	write3MFAsOpenJZ(pModel, sOpenJZPath, pWorkingDir, options);

	LoadOpenJZFromFile(sOpenJZPath);
}

void CEOSContext::LoadOpenJob(const std::string& sOpenJobFileName, const std::string& sMaterialSetFileName)
{
	std::string sOpenJobPath = AMCCommon::CUtils::getFullPathName(sOpenJobFileName, false);
	if (!AMCCommon::CUtils::fileOrPathExistsOnDisk(sOpenJobPath) || AMCCommon::CUtils::pathIsDirectory(sOpenJobPath))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJob file not found: " + sOpenJobPath);

	std::string sMaterialSetPath = AMCCommon::CUtils::getFullPathName(sMaterialSetFileName, false);
	if (!AMCCommon::CUtils::fileOrPathExistsOnDisk(sMaterialSetPath) || AMCCommon::CUtils::pathIsDirectory(sMaterialSetPath))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "material set file not found: " + sMaterialSetPath);

	auto pWorkingDir = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_oj");
	std::string sOpenJZPath = pWorkingDir->getPath() + "/job.openjz";
	buildOpenJZFromOpenJob(sOpenJobPath, sMaterialSetPath, sOpenJZPath);

	LoadOpenJZFromFile(sOpenJZPath);
}

namespace {

	// The material thickness of eos3mf and OpenJob has no stated unit. Values below 1 are taken as millimeters, others as micrometers.
	uint32_t parseMaterialThicknessInMicrometers(const std::string& sValue)
	{
		std::istringstream stream(sValue);
		stream.imbue(std::locale::classic());
		double dValue = 0.0;
		stream >> dValue;
		if (stream.fail() || !(dValue > 0.0) || (dValue > 1.0e6))
			return 0;

		double dMicrometers = (dValue < 1.0) ? (dValue * 1000.0) : dValue;
		return (uint32_t)llround(dMicrometers);
	}

	// Chooses the exposure set of a part without eos3mf:partparameter: an assignment in the settings,
	// else the only exposure set of the material set with the job's thickness and the part's support flag,
	// else the _Default_ parameter if the settings allow it.
	std::string resolvePartExposureSet(const SEOS3MFPart& part, uint32_t nThickness, const std::vector<SEOSMaterialExposureSet>& exposureSets,
		const SConversionSettings& settings, PConversionLogData pLog)
	{
		bool bIsSupport = part.meta.isSupport;
		std::string sKind = bIsSupport ? "support" : "part";
		std::string sThickness = std::to_string(nThickness) + " micrometers";

		using Lib3MF4EOS::eEOSLogSeverity;

		auto logMessage = [&](eEOSLogSeverity eSeverity, const std::string& sCode, const std::string& sText) {
			if (pLog.get() != nullptr)
				pLog->addMessage(eSeverity, sCode, LIBEOS_CONVERSIONLOG_NOLAYER, part.partId, sText);
		};

		std::string sReason;
		if (nThickness > 0) {
			auto iAssignment = settings.m_ExposureSetsForThickness.find(std::make_pair(nThickness, bIsSupport));
			if (iAssignment != settings.m_ExposureSetsForThickness.end()) {
				logMessage(eEOSLogSeverity::Info, "EXPOSURESET_ASSIGNED", sKind + " \"" + part.name + "\" uses the exposure set \"" + iAssignment->second +
					"\", which the settings assign to " + sThickness);
				return iAssignment->second;
			}

			std::vector<std::string> matches;
			for (auto& exposureSet : exposureSets)
				if ((exposureSet.m_nLayerThickness == nThickness) && (exposureSet.m_bIsSupport == bIsSupport))
					matches.push_back(exposureSet.m_sName);

			if (matches.size() == 1) {
				logMessage(eEOSLogSeverity::Info, "EXPOSURESET_DERIVED", sKind + " \"" + part.name + "\" uses the exposure set \"" + matches.front() +
					"\", the only " + sKind + " exposure set of the material set for " + sThickness);
				return matches.front();
			}

			if (matches.empty())
				sReason = "the material set has no " + sKind + " exposure set for " + sThickness;
			else
				sReason = "the material set has " + std::to_string(matches.size()) + " " + sKind + " exposure sets for " + sThickness;
		}
		else {
			sReason = "the layer thickness of the part is unknown";
		}

		if (settings.m_bAllowDefaultExposureSets) {
			std::string sDefault = getDefaultPartParameter(bIsSupport);
			logMessage(eEOSLogSeverity::Warning, "EXPOSURESET_DEFAULT", sKind + " \"" + part.name + "\" uses " + sDefault + ", because " + sReason);
			return sDefault;
		}

		std::string sMessage = "no exposure set could be chosen for " + sKind + " \"" + part.name + "\": " + sReason +
			"; set eos3mf:partparameter, assign one with SetExposureSetForThickness, or allow the default exposure sets";
		logMessage(eEOSLogSeverity::Error, "EXPOSURESET_AMBIGUOUS", sMessage);
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EXPOSURESETAMBIGUOUS, sMessage);
	}

}

std::string CEOSContext::write3MFAsOpenJZ(Lib3MF::PModel pModel, const std::string& sOpenJZFileName, AMCCommon::PWorkingDirectory pWorkingDir, const S3MFJobWriteOptions& options)
{
	if (pModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model is null");

	std::string sMaterialPath = extractMaterialSetFromModel(pModel, pWorkingDir->getPath(), m_NamespaceRegistry);
	SEOS3MFMetadata meta = readModelMetadata(pModel, m_NamespaceRegistry);
	std::vector<SEOSMaterialExposureSet> exposureSets;
	std::string sMaterialSetTarget;

	auto logMessage = [&options](eEOSLogSeverity eSeverity, const std::string& sCode, const std::string& sText) {
		if (options.m_pLog.get() != nullptr)
			options.m_pLog->addMessage(eSeverity, sCode, LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, sText);
	};

	if (!meta.machineTarget.empty()) {
		logMessage(eEOSLogSeverity::Info, "MACHINETARGET", "the job targets " + meta.machineTarget + ", as given by eos3mf:machinetarget");
	}
	else if (options.m_pMachineConfiguration != nullptr) {
		std::unique_ptr<IEOSMachineInfo> pMachineInfo(options.m_pMachineConfiguration->ReadMachineInformation());
		if (pMachineInfo->GetModelType() != eEOSModelType::Unknown) {
			meta.machineTarget = pMachineInfo->GetModelTypeIdentifier();
			logMessage(eEOSLogSeverity::Info, "MACHINETARGET", "the job targets " + meta.machineTarget + ", the model type of the machine configuration, as the model has no eos3mf:machinetarget");
		}
		else {
			logMessage(eEOSLogSeverity::Warning, "MACHINETARGET_UNKNOWN", "the machine configuration has a model type unknown to LibEOS (" + pMachineInfo->GetModelTypeProductName() +
				"), so it gives no machine target");
		}
	}

	if (!sMaterialPath.empty()) {
		std::unique_ptr<IEOSMaterialSet> pMaterialSet(ImportMaterialSet(sMaterialPath));
		auto pMaterialSetInstance = dynamic_cast<CEOSMaterialSet*>(pMaterialSet.get());
		if (pMaterialSetInstance != nullptr)
			exposureSets = pMaterialSetInstance->getExposureSets();

		std::ostringstream ssThickness;
		ssThickness << (pMaterialSet->GetBaseLayerthickness() / 1000.0);

		sMaterialSetTarget = pMaterialSet->GetModelType();
		if (meta.machineTarget.empty() && !sMaterialSetTarget.empty()) {
			meta.machineTarget = sMaterialSetTarget;
			logMessage(eEOSLogSeverity::Info, "MACHINETARGET", "the job targets " + meta.machineTarget + ", the model type of the material set");
		}
		if (meta.paramSetName.empty()) meta.paramSetName = pMaterialSet->GetName();
		if (meta.paramSetVersion.empty()) meta.paramSetVersion = pMaterialSet->GetParameterSetVersion();
		if (meta.materialName.empty()) meta.materialName = pMaterialSet->GetMaterial();
		if (meta.materialThickness.empty()) meta.materialThickness = ssThickness.str();
	}
	else {
		if (meta.paramSetName.empty() || meta.paramSetVersion.empty() || meta.materialName.empty() || meta.materialThickness.empty())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model has no embedded EOS material set (attach one with AttachMaterialSetTo3MF), "
				"and its eos3mf job metadata (paramsetname, paramsetversion, materialname, materialthickness) is incomplete");
	}

	if (meta.machineTarget.empty())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_UNKNOWNMODELTYPE, "the machine target of the job is unknown: the model has no eos3mf:machinetarget, and neither the machine configuration "
			"nor the material set has a model type known to LibEOS");

	if (!sMaterialSetTarget.empty() && (sMaterialSetTarget != meta.machineTarget))
		logMessage(eEOSLogSeverity::Warning, "MACHINETARGET_MISMATCH", "the job targets " + meta.machineTarget + ", but the material set is made for " + sMaterialSetTarget);

	SEOS3MFJobOptions jobOptions;
	if (m_pSDK.get() != nullptr)
		jobOptions.openJobVersion = m_pSDK->getAdapter().getOpenJobVersion();
	jobOptions.heightEndInMM = options.m_dHeightEndInMM;

	uint32_t nThickness = parseMaterialThicknessInMicrometers(meta.materialThickness);
	if (options.m_pLayerThicknessOut != nullptr)
		*options.m_pLayerThicknessOut = nThickness;

	if (options.m_pSettings != nullptr) {
		const SConversionSettings& settings = *options.m_pSettings;
		PConversionLogData pLog = options.m_pLog;
		jobOptions.resolvePartParameter = [&settings, &options, pLog, nThickness, &exposureSets](const SEOS3MFPart& part) {
			uint32_t nPartThickness = nThickness;
			if (options.m_pPartLayerThickness != nullptr) {
				auto iThickness = options.m_pPartLayerThickness->find(part.partId);
				if ((iThickness != options.m_pPartLayerThickness->end()) && (iThickness->second > 0))
					nPartThickness = iThickness->second;
			}
			std::string sExposureSet = resolvePartExposureSet(part, nPartThickness, exposureSets, settings, pLog);
			if (options.m_pChosenExposureSetsOut != nullptr)
				(*options.m_pChosenExposureSetsOut)[part.partId] = sExposureSet;
			return sExposureSet;
		};
	}

	buildOpenJZFromModel(pModel, meta.machineTarget, meta.paramSetName, meta.paramSetVersion,
		meta.materialName, meta.materialThickness, sMaterialPath, sOpenJZFileName, jobOptions, m_NamespaceRegistry);

	return sMaterialPath;
}


IEOSMachineInfo * CEOSContext::ActivateMachineConfiguration(IMachineConfigFuture* pMachineConfiguration)
{

	if (pMachineConfiguration == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

	auto pMachineConfigurationInstance = dynamic_cast<CMachineConfigFuture*> (pMachineConfiguration);
	if (pMachineConfigurationInstance == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCAST);

	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	if (!m_pSDK->simulateFullSDK()) {

		auto pWorkingDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("mc");

		pMachineConfigurationInstance->storeInWorkingDirectory(pWorkingDirectory);

		CCurrentDirectoryOverride directoryOverride(m_sSDKDirectory);

		std::wstring sFilePathW = pWorkingDirectory->getPathW();

		m_pSDK->checkEOSError(m_pSDK->EosTaskGen_LoadMachineConfiguration(sFilePathW.c_str()));

		// TODO: Make async?

	}
	else {

		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	return RetrieveCurrentMachineInformation();

}

IEOSMachineInfo* CEOSContext::ActivateMachineConfigurationFromPath(const std::string& sFilePath)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	if (!m_pSDK->simulateFullSDK()) {

		CCurrentDirectoryOverride directoryOverride(m_sSDKDirectory);

		std::wstring sFilePathW = AMCCommon::CUtils::UTF8toUTF16(sFilePath);

		m_pSDK->checkEOSError (m_pSDK->EosTaskGen_LoadMachineConfiguration(sFilePathW.c_str ()));

		// TODO: Make async?

	}
	else {

		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	return RetrieveCurrentMachineInformation();

}


IEOSMachineInfo* CEOSContext::RetrieveCurrentMachineInformation()
{
	std::unique_ptr<CEOSMachineInfo> pMachineInformationResult(new CEOSMachineInfo());

	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	if (!m_pSDK->simulateFullSDK()) {

		sEOSMachineInfo machineInfo;
		EOS_ERROR error = m_pSDK->getAdapter().getMachineInfo(machineInfo);
		if (error == EOS_ERR_EMPTY)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTGETMACHINEINFORMATION);
		m_pSDK->checkEOSError(error);

		pMachineInformationResult->loadFromMachineInfo(machineInfo);

	}
	else {
		pMachineInformationResult->loadSimulatedMachineInfo(m_pSDK->getSimulator()->getLaserScannerCount());
	}

	return pMachineInformationResult.release();
}

void CEOSContext::UnloadJob()
{
	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	if (!m_pSDK->simulateFullSDK()) {

		m_pSDK->checkEOSError (m_pSDK->EosTaskGen_UnloadJob());

	}
	else {

		std::this_thread::sleep_for(std::chrono::milliseconds (300));
	}
}

void CEOSContext::SetCacheDirectory(const std::string& sDirectory)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

}

bool CEOSContext::JobIsLoaded()
{
	if (m_pSDK.get() == nullptr)
		return false;

	if (!m_pSDK->simulateFullSDK()) {
		return m_pSDK->EosTaskGen_IsJobLoaded();

	}
	else {
		// TODO: Store flag
		return true;
	}

}

IBuildTimeEstimationFuture* CEOSContext::EstimateBuildTime()
{
	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	return new CBuildTimeEstimationFuture(m_pSDK);

}

ITaskCreationFuture* CEOSContext::CreateTask(const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion)
{
	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	return new CTaskCreationFuture(m_pSDK, sName, sComment, sGeneratorName, sGeneratorVersion, false);

}

ITaskCreationFuture* CEOSContext::CreateRawTask(const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion)
{
	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	return new CTaskCreationFuture(m_pSDK, sName, sComment, sGeneratorName, sGeneratorVersion, true);

}

IMachineSessionFuture* CEOSContext::ConnectToMachine(const std::string& sNetworkAddress)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	return new CMachineSessionFuture(m_pSDK, sNetworkAddress);

}

IMachineConfigFuture* CEOSContext::LoadMachinePackage(const Lib3MF4EOS_uint64 nStreamDataBufferSize, const Lib3MF4EOS_uint8* pStreamDataBuffer)
{
	if ((nStreamDataBufferSize == 0) || (pStreamDataBuffer == nullptr))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDSTREAMDATA);

	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	auto pMachineConfig = std::make_unique<CMachineConfigFuture>(m_pSDK, m_NamespaceRegistry);
	pMachineConfig->loadFromArray(nStreamDataBufferSize, pStreamDataBuffer);

	return pMachineConfig.release();
}

IMachineConfigFuture* CEOSContext::LoadMachinePackageFromFile(const std::string& sFileName)
{
	if (!AMCCommon::CUtils::fileOrPathExistsOnDisk(sFileName))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MACHINEPACKAGEFILENOTFOUND, "machine package file not found: " + sFileName);
	if (AMCCommon::CUtils::pathIsDirectory(sFileName))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MACHINEPACKAGEFILEISDIRECTORY, "machine package file is directory: " + sFileName);

	AMCCommon::CImportStream_Native importStream(sFileName);
	uint64_t nSize = importStream.retrieveSize();
	if (nSize == 0)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MACHINEPACKAGEFILEISEMPTY, "machine package file is empty: " + sFileName);

	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	auto pMachineConfig = std::make_unique<CMachineConfigFuture>(m_pSDK, m_NamespaceRegistry);
	pMachineConfig->loadFromStream(importStream);
	 
	return pMachineConfig.release();
}

IMachineConfigFuture* CEOSContext::LoadMachineConfigFromDirectory(const std::string& sPath)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	auto pMachineConfig = std::make_unique<CMachineConfigFuture>(m_pSDK, m_NamespaceRegistry);
	pMachineConfig->loadFromDirectory(AMCCommon::CUtils::getFullPathName(sPath, false));

	return pMachineConfig.release();
}


void CEOSContext::AuthenticateToolpathAPI(const Lib3MF4EOS_uint32 nClientID, const std::string& sClientKey)
{
	if (m_pSDK.get() == nullptr)
		LoadSDK();

	if (m_pSDK->EosToolpath_GetAuthChallenge == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);
	if (m_pSDK->EosToolpath_SetAuthResponse == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED);

	EOS_BYTE* challenge = nullptr;
	EOS_ULONG challengeLength = 0;

	m_pSDK->checkEOSError(m_pSDK->EosToolpath_GetAuthChallenge(&challenge, &challengeLength));

	if (challenge == nullptr || challengeLength == 0)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHAUTHENTICATIONFAILED, "could not get authentication challenge");

	CSSLRSACryptoHelper cryptHelper (sClientKey);

	if (!cryptHelper.encryptChallenge (challenge, challengeLength))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHAUTHENTICATIONFAILED, "could not encrypt authentication challenge");


	std::vector<uint8_t>& responseBuffer = cryptHelper.getResponseBuffer();

	if (responseBuffer.empty())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHAUTHENTICATIONFAILED, "challenge response buffer empty!");

	m_pSDK->checkEOSError(m_pSDK->EosToolpath_SetAuthResponse(nClientID, responseBuffer.data (), responseBuffer.size()));


}

void CEOSContext::AuthenticateToolpathAPIFromCredentialStore(const Lib3MF4EOS_uint32 nClientID, const std::string& sCredentialName)
{
	std::string sClientKey = readToolpathClientKeyFromCredentialStore(sCredentialName);
	try {
		AuthenticateToolpathAPI(nClientID, sClientKey);
	}
	catch (...) {
		secureErase(sClientKey);
		throw;
	}
	secureErase(sClientKey);
}

IEOSToolpathConversionSettings* CEOSContext::CreateToolpathConversionSettings()
{
	return new CEOSToolpathConversionSettings();
}

IEOSRawTaskWriter* CEOSContext::CreateRawTaskWriter(IEOSToolpathConversionSettings* pSettings)
{
	SConversionSettings settings = getConversionSettings(pSettings);

	if (m_pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_EOSSDKHASNOTBEENLOADED);

	auto pLog = createConversionLog(settings);
	addSDKInfoToLog(pLog);

	// The writer checks scanner indices and begins parts on idle scanners against the active machine configuration.
	uint32_t nLaserScannerCount = 0;
	try {
		std::unique_ptr<IEOSMachineInfo> pMachineInfo(RetrieveCurrentMachineInformation());
		nLaserScannerCount = pMachineInfo->GetNumberOfLaserScanners();
	}
	catch (ELib3MF4EOSInterfaceException& e) {
		if (e.getErrorCode() != LIB3MF4EOS_ERROR_COULDNOTGETMACHINEINFORMATION)
			throw;
	}
	if (nLaserScannerCount == 0)
		pLog->addMessage(eEOSLogSeverity::Warning, "SCANNER_COUNT_UNAVAILABLE", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"no machine configuration is active, so laser scanners without work in a layer get no part, and the EOS SDK refuses such layers");

	auto pWorkingDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_rw");
	auto pWriter = std::make_unique<CEOSRawTaskWriter>(m_pSDK, pWorkingDirectory, settings, pLog);
	pWriter->setLaserScannerCount(nLaserScannerCount);
	return pWriter.release();
}

IEOSRawTaskLayer* CEOSContext::CreateEOSRawTaskLayer(const Lib3MF4EOS_int64 nZHeightInMicrometers)
{
	auto pLayer = std::make_unique<CEOSRawTaskLayer>(nZHeightInMicrometers);
	return pLayer.release();
}


Lib3MF::PWrapper CEOSContext::getLib3MFWrapper()
{
	if (CWrapper::sPLib3MFWrapper.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTLOADLIBRARY, "Lib3MF has not been injected with InjectComponent");
	return CWrapper::sPLib3MFWrapper;
}

static std::string getLibEOSVersionString()
{
	return std::to_string(LIB3MF4EOS_VERSION_MAJOR) + "." + std::to_string(LIB3MF4EOS_VERSION_MINOR) + "." + std::to_string(LIB3MF4EOS_VERSION_MICRO);
}

IToolpathComputationFuture* CEOSContext::startToolpathComputation(Lib3MF::PModel pInputModel, IMachineConfigFuture* pMachineConfiguration, const std::string& sTaskComment)
{
	if (pInputModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF input model is null");
	if (pMachineConfiguration == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "Machine configuration is null");

	auto pLib3MFWrapper = getLib3MFWrapper();

	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);

	auto pWorkingDir = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_3mf_compute");
	std::string sOpenJZPath = pWorkingDir->getPath() + "/job.openjz";

	S3MFJobWriteOptions jobOptions;
	jobOptions.m_pMachineConfiguration = pMachineConfiguration;
	std::string sMaterialPath = write3MFAsOpenJZ(pInputModel, sOpenJZPath, pWorkingDir, jobOptions);
	LoadOpenJZFromFile(sOpenJZPath);

	std::unique_ptr<IEOSMachineInfo> pMachineInfo(ActivateMachineConfiguration(pMachineConfiguration));

	std::unique_ptr<CTaskCreationFuture> pTaskCreationFuture(
		new CTaskCreationFuture(m_pSDK, "compute.task", sTaskComment, "LibEOS", getLibEOSVersionString(), true));

	return new CToolpathComputationFuture(pInputModel, pLib3MFWrapper, std::move(pTaskCreationFuture), sMaterialPath, m_NamespaceRegistry);
}

IToolpathComputationFuture* CEOSContext::ComputeToolpathFrom3MF(Lib3MF::PModel pInputModel, IMachineConfigFuture* pMachineConfiguration)
{
	return startToolpathComputation(pInputModel, pMachineConfiguration, "Generated by LibEOS ComputeToolpathFrom3MF");
}

IToolpathComputationFuture* CEOSContext::ComputeToolpathFromOpenJZ(const std::string& sOpenJZFileName, IMachineConfigFuture* pMachineConfiguration)
{
	if (pMachineConfiguration == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "Machine configuration is null");

	auto pModel = getLib3MFWrapper()->CreateModel();
	convertOpenJZToModel(AMCCommon::CUtils::getFullPathName(sOpenJZFileName, true), pModel);

	return startToolpathComputation(pModel, pMachineConfiguration, "Generated by LibEOS ComputeToolpathFromOpenJZ");
}

void CEOSContext::Convert3MFToOpenJZ(Lib3MF::PModel pModel, const std::string& sOpenJZFileName)
{
	if (sOpenJZFileName.empty())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "OpenJZ file name is empty");

	auto pWorkingDir = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_3mf");
	write3MFAsOpenJZ(pModel, AMCCommon::CUtils::getFullPathName(sOpenJZFileName, false), pWorkingDir);
}

void CEOSContext::ConvertOpenJZTo3MF(const std::string& sOpenJZFileName, Lib3MF::PModel pModel)
{
	convertOpenJZToModel(AMCCommon::CUtils::getFullPathName(sOpenJZFileName, true), pModel);
}

void CEOSContext::AttachMaterialSetTo3MF(Lib3MF::PModel pModel, const std::string& sEOSParFileName)
{
	if (pModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model is null");

	std::string sFullPath = AMCCommon::CUtils::getFullPathName(sEOSParFileName, false);
	if (!AMCCommon::CUtils::fileOrPathExistsOnDisk(sFullPath) || AMCCommon::CUtils::pathIsDirectory(sFullPath))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "material set file not found: " + sFullPath);

	attachMaterialSetToModel(pModel, sFullPath);
}

std::string CEOSContext::GetMaterialSetRelationshipType()
{
	return m_NamespaceRegistry.getNamespace(Lib3MF4EOS::eEOSNamespaceKind::MaterialSetRelationship);
}

std::string CEOSContext::GetNamespace(const Lib3MF4EOS::eEOSNamespaceKind eKind)
{
	return m_NamespaceRegistry.getNamespace(eKind);
}

void CEOSContext::RegisterLegacyNamespace(const Lib3MF4EOS::eEOSNamespaceKind eKind, const std::string& sNamespaceURI)
{
	m_NamespaceRegistry.registerLegacyNamespace(eKind, sNamespaceURI);
}

void CEOSContext::ClearLegacyNamespaces()
{
	m_NamespaceRegistry.clearLegacyNamespaces();
}

namespace {

	// True if sNameSpace is the current or a registered legacy toolpath profile namespace.
	bool isAcceptedToolpathNamespace(const std::vector<std::string>& acceptedNamespaces, const std::string& sNameSpace)
	{
		for (const auto& sAccepted : acceptedNamespaces)
			if (sNameSpace == sAccepted)
				return true;
		return false;
	}

	// Reads eos: profile parameters, preferring the current namespace over registered legacy ones.
	struct SEOSProfileReader {
		Lib3MF::PToolpathProfile m_pProfile;
		const std::vector<std::string>& m_AcceptedNamespaces;

		// The first accepted namespace that holds sValueName, or an empty string if none does.
		std::string namespaceOf(const std::string& sValueName) const
		{
			for (const auto& sNamespace : m_AcceptedNamespaces)
				if (m_pProfile->HasParameterValue(sNamespace, sValueName))
					return sNamespace;
			return std::string();
		}

		bool has(const std::string& sValueName) const
		{
			return !namespaceOf(sValueName).empty();
		}

		double getDouble(const std::string& sValueName) const
		{
			return m_pProfile->GetParameterDoubleValue(namespaceOf(sValueName), sValueName);
		}

		int64_t getInteger(const std::string& sValueName) const
		{
			return m_pProfile->GetParameterIntegerValue(namespaceOf(sValueName), sValueName);
		}

		std::string getValueDef(const std::string& sValueName, const std::string& sDefault) const
		{
			std::string sNamespace = namespaceOf(sValueName);
			return sNamespace.empty() ? sDefault : m_pProfile->GetParameterValueDef(sNamespace, sValueName, sDefault);
		}

		bool getBoolDef(const std::string& sValueName, bool bDefault) const
		{
			std::string sNamespace = namespaceOf(sValueName);
			return sNamespace.empty() ? bDefault : m_pProfile->GetParameterBoolValueDef(sNamespace, sValueName, bDefault);
		}

		double getDoubleDef(const std::string& sValueName, double dDefault) const
		{
			std::string sNamespace = namespaceOf(sValueName);
			return sNamespace.empty() ? dDefault : m_pProfile->GetParameterDoubleValueDef(sNamespace, sValueName, dDefault);
		}

		int64_t getIntegerDef(const std::string& sValueName, int64_t nDefault) const
		{
			std::string sNamespace = namespaceOf(sValueName);
			return sNamespace.empty() ? nDefault : m_pProfile->GetParameterIntegerValueDef(sNamespace, sValueName, nDefault);
		}
	};

	// Block parameters that EOS can only change between blocks.
	enum class eConstantModifierTarget {
		PulsedWavePeriodTime,
		PulsedWavePowerOnTime
	};

	// A modifier of a value that can change along a hatch and between the vectors of a polyline.
	struct SValueModifier {
		bool m_bActive = false;
		Lib3MF::eToolpathProfileModificationType m_eType = Lib3MF::eToolpathProfileModificationType::NoModification;
		Lib3MF::eToolpathProfileModificationFactor m_eFactor = Lib3MF::eToolpathProfileModificationFactor::Unknown;
		double m_dMinValue = 0.0;
		double m_dMaxValue = 0.0;
	};

	struct SConstantModifier {
		std::string m_sName;
		eConstantModifierTarget m_eTarget;
		Lib3MF::eToolpathProfileModificationFactor m_eFactor;
		double m_dMinValue;
		double m_dMaxValue;
	};

	struct SUnsupportedModifier {
		std::string m_sName;
		Lib3MF::eToolpathProfileModificationFactor m_eFactor;
		std::string m_sReason;
	};

	bool findConstantModifierTarget(const std::vector<std::string>& acceptedNamespaces, const std::string& sNameSpace, const std::string& sValueName, eConstantModifierTarget& eTarget)
	{
		if (isAcceptedToolpathNamespace(acceptedNamespaces, sNameSpace) && (sValueName == "pulsedwaveperiodtime"))
			eTarget = eConstantModifierTarget::PulsedWavePeriodTime;
		else if (isAcceptedToolpathNamespace(acceptedNamespaces, sNameSpace) && (sValueName == "pulsedwavepowerontime"))
			eTarget = eConstantModifierTarget::PulsedWavePowerOnTime;
		else
			return false;
		return true;
	}

	double getConstantModifierTarget(const Lib3MF4EOS::sEOSTaskBlock& block, eConstantModifierTarget eTarget)
	{
		switch (eTarget) {
		case eConstantModifierTarget::PulsedWavePeriodTime: return block.m_PulsedWavePeriodTime;
		default: return block.m_PulsedWavePowerOnTime;
		}
	}

	void setConstantModifierTarget(Lib3MF4EOS::sEOSTaskBlock& block, eConstantModifierTarget eTarget, double dValue)
	{
		switch (eTarget) {
		case eConstantModifierTarget::PulsedWavePeriodTime: block.m_PulsedWavePeriodTime = dValue; break;
		default: block.m_PulsedWavePowerOnTime = dValue; break;
		}
	}

	struct SToolpathProfileSettings {
		std::string m_sUUID;
		std::string m_sName;
		uint32_t m_nLaserIndex;
		double m_dLaserPower;
		double m_dScanSpeed;
		// eos:focus, the EOS focus value from 0.0 (fully defocused) to 1.0 (fully focused).
		bool m_bHasFocus;
		double m_dFocus;
		// laserfocus, the focus offset in mm of the 3MF toolpath extension.
		double m_dLaserFocusOffset;
		bool m_bHasExposureType;
		uint32_t m_nExposureType;
		bool m_bSkywriting;
		uint32_t m_nExposureSetIndex;
		uint32_t m_nExposureStepIndex;
		double m_dPulsedWavePeriodTime;
		double m_dPulsedWavePowerOnTime;
		uint32_t m_nExposedDepth;
		double m_dPreDelay;
		double m_dPostDelay;
		// prewaittime and postwaittime, in the dwell time unit of the settings.
		double m_dPreWaitTime;
		double m_dDwellTime;
		int32_t m_nBeamShapeProfile;
		std::vector<SUnsupportedProfileParameter> m_UnsupportedParameters;

		// value = MinValue + clamp(factor, 0, 1) * (MaxValue - MinValue)
		SValueModifier m_PowerModifier;
		SValueModifier m_SpeedModifier;
		SValueModifier m_FocusModifier;
		SValueModifier m_DwellModifier;

		std::vector<SConstantModifier> m_ConstantModifiers;
		std::vector<SUnsupportedModifier> m_UnsupportedModifiers;
	};

	void readToolpathProfileModifiers(Lib3MF::PToolpathProfile pProfile, const std::vector<std::string>& acceptedNamespaces, SToolpathProfileSettings& settings)
	{
		uint32_t nModifierCount = pProfile->GetModifierCount();
		for (uint32_t nModifierIndex = 0; nModifierIndex < nModifierCount; nModifierIndex++) {
			std::string sNameSpace;
			std::string sValueName;
			Lib3MF::eToolpathProfileModificationType eType;
			Lib3MF::eToolpathProfileModificationFactor eFactor;
			double dMinValue = 0.0;
			double dMaxValue = 0.0;
			pProfile->GetModifierInformationByIndex(nModifierIndex, sNameSpace, sValueName, eType, eFactor, dMinValue, dMaxValue);

			if ((eType == Lib3MF::eToolpathProfileModificationType::NoModification) || (eFactor == Lib3MF::eToolpathProfileModificationFactor::Unknown))
				continue;

			std::string sName = sNameSpace.empty() ? sValueName : (sNameSpace + ":" + sValueName);
			eConstantModifierTarget eTarget;

			SValueModifier modifier;
			modifier.m_bActive = true;
			modifier.m_eType = eType;
			modifier.m_eFactor = eFactor;
			modifier.m_dMinValue = dMinValue;
			modifier.m_dMaxValue = dMaxValue;

			if (sNameSpace.empty() && (sValueName == "laserpower")) {
				settings.m_PowerModifier = modifier;
			}
			else if (sNameSpace.empty() && (sValueName == "laserspeed")) {
				settings.m_SpeedModifier = modifier;
			}
			else if (isAcceptedToolpathNamespace(acceptedNamespaces, sNameSpace) && (sValueName == "focus")) {
				if (!((dMinValue >= 0.0) && (dMinValue <= 1.0) && (dMaxValue >= 0.0) && (dMaxValue <= 1.0)))
					throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDFOCUS, "toolpath profile " + pProfile->GetName() + " modifies eos:focus between " +
						std::to_string(dMinValue) + " and " + std::to_string(dMaxValue) + "; EOS focus values go from 0.0 (fully defocused) to 1.0 (fully focused)");
				settings.m_FocusModifier = modifier;
			}
			else if (sNameSpace.empty() && (sValueName == "laserfocus")) {
				settings.m_UnsupportedModifiers.push_back(SUnsupportedModifier{ sName, eFactor,
					"laserfocus is a focus offset in mm, which has no EOS equivalent; modify the EOS focus value eos:focus instead" });
			}
			else if (sNameSpace.empty() && (sValueName == "postwaittime")) {
				settings.m_DwellModifier = modifier;
			}
			else if (sNameSpace.empty() && (sValueName == "prewaittime")) {
				settings.m_UnsupportedModifiers.push_back(SUnsupportedModifier{ sName, eFactor,
					"LibEOS writes prewaittime only as a constant; modify postwaittime instead" });
			}
			else if (findConstantModifierTarget(acceptedNamespaces, sNameSpace, sValueName, eTarget)) {
				if (eType == Lib3MF::eToolpathProfileModificationType::ConstantModification)
					settings.m_ConstantModifiers.push_back(SConstantModifier{ sName, eTarget, eFactor, dMinValue, dMaxValue });
				else
					settings.m_UnsupportedModifiers.push_back(SUnsupportedModifier{ sName, eFactor, "pulse settings can only change between blocks, so they only allow constant modifiers" });
			}
			else {
				settings.m_UnsupportedModifiers.push_back(SUnsupportedModifier{ sName, eFactor, "EOS cannot change this parameter between vectors" });
			}
		}
	}

	// Reads the parameter names written by CEOSTo3MFConverter::findOrCreateProfile.
	SToolpathProfileSettings readToolpathProfileSettings(Lib3MF::PToolpathProfile pProfile, const std::vector<std::string>& acceptedNamespaces, bool bDefaultSkywriting, uint32_t nDefaultLaserIndex)
	{
		SToolpathProfileSettings settings;
		std::string sProfileName = pProfile->GetName();
		settings.m_sUUID = pProfile->GetUUID();
		settings.m_sName = sProfileName;
		readToolpathProfileModifiers(pProfile, acceptedNamespaces, settings);

		SEOSProfileReader eosReader{ pProfile, acceptedNamespaces };

		if (!pProfile->HasParameterValue("", "laserpower"))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath profile has no laserpower: " + sProfileName);
		if (!pProfile->HasParameterValue("", "laserspeed"))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath profile has no laserspeed: " + sProfileName);

		settings.m_dLaserPower = pProfile->GetParameterDoubleValue("", "laserpower");
		settings.m_dScanSpeed = pProfile->GetParameterDoubleValue("", "laserspeed");
		settings.m_bHasFocus = eosReader.has("focus");
		settings.m_dFocus = settings.m_bHasFocus ? eosReader.getDouble("focus") : EOS_FOCUS_NOMINAL;
		if (!((settings.m_dFocus >= 0.0) && (settings.m_dFocus <= 1.0)))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDFOCUS, "toolpath profile " + sProfileName + " has an eos:focus of " + std::to_string(settings.m_dFocus) +
				"; EOS focus values go from 0.0 (fully defocused) to 1.0 (fully focused)");
		settings.m_dLaserFocusOffset = pProfile->GetParameterDoubleValueDef("", "laserfocus", 0.0);
		settings.m_nLaserIndex = (uint32_t)pProfile->GetParameterIntegerValueDef("", "laserindex", nDefaultLaserIndex);

		std::string sExposureType = eosReader.getValueDef("exposuretype", "");
		settings.m_nExposureType = 0;
		settings.m_bHasExposureType = CEOSTo3MFConverter::exposureTypeFromName(sExposureType, settings.m_nExposureType);
		if (!sExposureType.empty() && !settings.m_bHasExposureType)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath profile has unknown exposure type '" + sExposureType + "': " + sProfileName);

		settings.m_bSkywriting = eosReader.getBoolDef("skywriting", bDefaultSkywriting);
		settings.m_nExposureSetIndex = (uint32_t)eosReader.getIntegerDef("exposuresetindex", 0);
		settings.m_nExposureStepIndex = (uint32_t)eosReader.getIntegerDef("exposurestepindex", 0);
		settings.m_dPulsedWavePeriodTime = eosReader.getDoubleDef("pulsedwaveperiodtime", 0.0);
		settings.m_dPulsedWavePowerOnTime = eosReader.getDoubleDef("pulsedwavepowerontime", 0.0);
		settings.m_nExposedDepth = (uint32_t)eosReader.getIntegerDef("exposeddepth", 0);
		settings.m_dPreDelay = eosReader.getDoubleDef("predelay", 0.0);
		settings.m_dPostDelay = eosReader.getDoubleDef("postdelay", 0.0);
		settings.m_dPreWaitTime = pProfile->GetParameterDoubleValueDef("", "prewaittime", 0.0);
		settings.m_dDwellTime = pProfile->GetParameterDoubleValueDef("", "postwaittime", 0.0);
		settings.m_nBeamShapeProfile = -1;
		if (eosReader.has("beamshapeprofile")) {
			int64_t nBeamShapeProfile = eosReader.getInteger("beamshapeprofile");
			if ((nBeamShapeProfile < 0) || (nBeamShapeProfile > INT32_MAX))
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath profile has an invalid beam shape profile " + std::to_string(nBeamShapeProfile) + ": " + sProfileName);
			settings.m_nBeamShapeProfile = (int32_t)nBeamShapeProfile;
		}

		if ((settings.m_dPreDelay < 0.0) || (settings.m_dPostDelay < 0.0) || (settings.m_dPreWaitTime < 0.0) || (settings.m_dDwellTime < 0.0))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath profile has a negative delay or dwell time: " + sProfileName);

		settings.m_UnsupportedParameters = findUnsupportedProfileParameters(pProfile, acceptedNamespaces);

		return settings;
	}

	// The value of a piecewise linear curve of (parameter, value) points at dT.
	double curveValueAt(const std::vector<std::pair<double, double>>& curve, double dT)
	{
		for (size_t nIndex = 0; nIndex + 1 < curve.size(); nIndex++) {
			double dT1 = curve[nIndex].first;
			double dT2 = curve[nIndex + 1].first;
			if (dT <= dT2) {
				if (dT2 <= dT1)
					return curve[nIndex + 1].second;
				double dLocalT = (std::max)(0.0, (dT - dT1) / (dT2 - dT1));
				return curve[nIndex].second + dLocalT * (curve[nIndex + 1].second - curve[nIndex].second);
			}
		}
		return curve.back().second;
	}

	double dwellTimeInMicroseconds(double dDwellTime, Lib3MF4EOS::eEOSDwellTimeUnit eUnit)
	{
		switch (eUnit) {
		case Lib3MF4EOS::eEOSDwellTimeUnit::Microseconds: return dDwellTime;
		case Lib3MF4EOS::eEOSDwellTimeUnit::Milliseconds: return dDwellTime * 1000.0;
		case Lib3MF4EOS::eEOSDwellTimeUnit::Seconds: return dDwellTime * 1000000.0;
		default:
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_DWELLTIMEUNITMISSING);
		}
	}

	std::string modificationTypeToJSONName(Lib3MF::eToolpathProfileModificationType eType)
	{
		switch (eType) {
		case Lib3MF::eToolpathProfileModificationType::ConstantModification: return "constant";
		case Lib3MF::eToolpathProfileModificationType::LinearModification: return "linear";
		case Lib3MF::eToolpathProfileModificationType::NonlinearModification: return "nonlinear";
		default: return "none";
		}
	}

	std::string modificationFactorToJSONName(Lib3MF::eToolpathProfileModificationFactor eFactor)
	{
		switch (eFactor) {
		case Lib3MF::eToolpathProfileModificationFactor::FactorF: return "F";
		case Lib3MF::eToolpathProfileModificationFactor::FactorG: return "G";
		case Lib3MF::eToolpathProfileModificationFactor::FactorH: return "H";
		default: return "unknown";
		}
	}

	std::string profileSettingsToJSON(const SToolpathProfileSettings& settings)
	{
		CJSONObject profile;
		profile.addString("uuid", settings.m_sUUID);
		profile.addString("name", settings.m_sName);
		profile.addUnsigned("laserIndex", settings.m_nLaserIndex);
		profile.addNumber("laserPower", settings.m_dLaserPower);
		profile.addNumber("scanSpeed", settings.m_dScanSpeed);
		if (settings.m_bHasFocus)
			profile.addNumber("focus", settings.m_dFocus);
		else
			profile.addNull("focus");
		profile.addNumber("laserFocusOffset", settings.m_dLaserFocusOffset);
		if (settings.m_bHasExposureType)
			profile.addUnsigned("exposureType", settings.m_nExposureType);
		else
			profile.addNull("exposureType");
		profile.addBool("skywriting", settings.m_bSkywriting);
		profile.addUnsigned("exposureSetIndex", settings.m_nExposureSetIndex);
		profile.addUnsigned("exposureStepIndex", settings.m_nExposureStepIndex);
		profile.addNumber("pulsedWavePeriodTime", settings.m_dPulsedWavePeriodTime);
		profile.addNumber("pulsedWavePowerOnTime", settings.m_dPulsedWavePowerOnTime);
		profile.addUnsigned("exposedDepth", settings.m_nExposedDepth);
		profile.addNumber("preDelay", settings.m_dPreDelay);
		profile.addNumber("postDelay", settings.m_dPostDelay);
		profile.addNumber("preWaitTime", settings.m_dPreWaitTime);
		profile.addNumber("dwellTime", settings.m_dDwellTime);
		if (settings.m_nBeamShapeProfile >= 0)
			profile.addUnsigned("beamShapeProfile", (uint32_t)settings.m_nBeamShapeProfile);
		else
			profile.addNull("beamShapeProfile");

		auto addValueModifier = [&profile](const std::string& sKey, const SValueModifier& modifier) {
			if (!modifier.m_bActive) {
				profile.addNull(sKey);
				return;
			}
			CJSONObject modifierJSON;
			modifierJSON.addString("type", modificationTypeToJSONName(modifier.m_eType));
			modifierJSON.addString("factor", modificationFactorToJSONName(modifier.m_eFactor));
			modifierJSON.addNumber("minValue", modifier.m_dMinValue);
			modifierJSON.addNumber("maxValue", modifier.m_dMaxValue);
			profile.addRaw(sKey, modifierJSON.toString());
		};
		addValueModifier("powerModifier", settings.m_PowerModifier);
		addValueModifier("speedModifier", settings.m_SpeedModifier);
		addValueModifier("focusModifier", settings.m_FocusModifier);
		addValueModifier("dwellModifier", settings.m_DwellModifier);

		CJSONArray constantModifiers;
		for (auto& modifier : settings.m_ConstantModifiers) {
			CJSONObject constantModifier;
			constantModifier.addString("name", modifier.m_sName);
			constantModifier.addString("factor", modificationFactorToJSONName(modifier.m_eFactor));
			constantModifier.addNumber("minValue", modifier.m_dMinValue);
			constantModifier.addNumber("maxValue", modifier.m_dMaxValue);
			constantModifiers.addRaw(constantModifier.toString());
		}
		profile.addRaw("constantModifiers", constantModifiers.toString());

		CJSONArray unsupportedModifiers;
		for (auto& modifier : settings.m_UnsupportedModifiers) {
			CJSONObject unsupportedModifier;
			unsupportedModifier.addString("name", modifier.m_sName);
			unsupportedModifier.addString("factor", modificationFactorToJSONName(modifier.m_eFactor));
			unsupportedModifier.addString("reason", modifier.m_sReason);
			unsupportedModifiers.addRaw(unsupportedModifier.toString());
		}
		profile.addRaw("unsupportedModifiers", unsupportedModifiers.toString());

		CJSONArray unsupportedParameters;
		for (auto& parameter : settings.m_UnsupportedParameters) {
			CJSONObject unsupportedParameter;
			unsupportedParameter.addString("name", parameter.m_sName);
			unsupportedParameter.addString("reason", parameter.m_sReason);
			unsupportedParameters.addRaw(unsupportedParameter.toString());
		}
		profile.addRaw("unsupportedParameters", unsupportedParameters.toString());

		return profile.toString();
	}

}

struct CEOSContext::S3MFToolpathConversionState {
	int32_t m_nCurrentLayerIndex = LIBEOS_CONVERSIONLOG_NOLAYER;
	std::map<std::string, SToolpathProfileSettings> m_ProfileSettings;
	// Scanner index of every declared laser index. Empty if the toolpath declares no laser sources.
	std::map<uint32_t, uint32_t> m_DeclaredLaserScanners;
	std::set<uint32_t> m_UsedLaserIndices;
};

void CEOSContext::addConversionStateToLog(const S3MFToolpathConversionState& state, PConversionLogData pLog)
{
	CJSONArray profiles;
	for (auto& profile : state.m_ProfileSettings)
		profiles.addRaw(profileSettingsToJSON(profile.second));
	pLog->setSection("profiles", profiles.toString());

	std::set<uint32_t> laserIndices = state.m_UsedLaserIndices;
	for (auto& declaredLaser : state.m_DeclaredLaserScanners)
		laserIndices.insert(declaredLaser.first);

	CJSONArray lasers;
	for (auto nLaserIndex : laserIndices) {
		CJSONObject laser;
		laser.addUnsigned("laserIndex", nLaserIndex);
		auto iScanner = state.m_DeclaredLaserScanners.find(nLaserIndex);
		if (state.m_DeclaredLaserScanners.empty())
			laser.addUnsigned("scannerIndex", nLaserIndex);
		else if (iScanner != state.m_DeclaredLaserScanners.end())
			laser.addUnsigned("scannerIndex", iScanner->second);
		else
			laser.addNull("scannerIndex");
		laser.addBool("declared", iScanner != state.m_DeclaredLaserScanners.end());
		laser.addBool("used", state.m_UsedLaserIndices.find(nLaserIndex) != state.m_UsedLaserIndices.end());
		lasers.addRaw(laser.toString());
	}
	pLog->setSection("lasers", lasers.toString());
}

SConversionSettings CEOSContext::getConversionSettings(IEOSToolpathConversionSettings* pSettings)
{
	if (pSettings == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "conversion settings are null");

	auto pSettingsInstance = dynamic_cast<CEOSToolpathConversionSettings*>(pSettings);
	if (pSettingsInstance == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCAST, "conversion settings were not created by LibEOS");

	SConversionSettings settings = pSettingsInstance->getSettings();
	settings.validate();
	return settings;
}

PConversionLogData CEOSContext::createConversionLog(const SConversionSettings& settings)
{
	auto pLog = std::make_shared<CConversionLogData>();

	CJSONObject libEOS;
	libEOS.addString("version", getLibEOSVersionString());
	pLog->setSection("libeos", libEOS.toString());
	pLog->setSection("sdk", jsonNull());
	pLog->setSection("settings", settings.toJSON());

	return pLog;
}

void CEOSContext::addSDKInfoToLog(PConversionLogData pLog)
{
	if (m_pSDK.get() == nullptr)
		return;

	auto& adapter = m_pSDK->getAdapter();
	auto productVersion = m_pSDK->getProductVersion();
	auto eVersionSource = m_pSDK->getProductVersionSource();

	CJSONObject sdk;
	sdk.addString("directory", m_sSDKDirectory);
	if (eVersionSource != eEOSSDKVersionSource::None)
		sdk.addString("productVersion", productVersion.toString());
	else
		sdk.addNull("productVersion");

	switch (eVersionSource) {
	case eEOSSDKVersionSource::ProductVersionFunction: sdk.addString("versionSource", "productVersionFunction"); break;
	case eEOSSDKVersionSource::VersionResource: sdk.addString("versionSource", "dllVersionResource"); break;
	default: sdk.addString("versionSource", "notDetected"); break;
	}

	sdk.addString("adapterVersion", adapter.getVersionString());
	sdk.addString("openJobVersion", adapter.getOpenJobVersion());
	sdk.addBool("fullSimulation", m_pSDK->simulateFullSDK());
	sdk.addBool("recording", m_pSDK->getToolpathRecorder().get() != nullptr);

	CJSONArray versionMessages;
	for (auto& sMessage : m_pSDK->getVersionMessages())
		versionMessages.addString(sMessage);
	sdk.addRaw("versionMessages", versionMessages.toString());

	pLog->setSection("sdk", sdk.toString());

	std::string sProductVersion = (eVersionSource != eEOSSDKVersionSource::None) ? productVersion.toString() : std::string("of unknown version");
	pLog->addMessage(eEOSLogSeverity::Info, "SDK_VERSION", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
		"EOS SDK " + sProductVersion + " is used with the " + adapter.getVersionString() + " declarations");
	for (auto& sMessage : m_pSDK->getVersionMessages())
		pLog->addMessage(eEOSLogSeverity::Warning, "SDK_VERSION_FALLBACK", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, sMessage);
}

std::string CEOSContext::describeCurrentException()
{
	try {
		throw;
	}
	catch (ELib3MF4EOSInterfaceException& e) {
		return std::string(e.what()) + " (LibEOS error " + std::to_string(e.getErrorCode()) + ")";
	}
	catch (std::exception& e) {
		return e.what();
	}
	catch (...) {
		return "unknown error";
	}
}

Lib3MF::PToolpath CEOSContext::getBuildToolpath(Lib3MF::PModel pModel)
{
	if (pModel->HasBuildToolpath())
		return pModel->GetBuildToolpath();

	auto pToolpathIterator = pModel->GetToolpaths();
	uint64_t nToolpathCount = pToolpathIterator->Count();
	if (nToolpathCount == 0)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model contains no toolpath");
	if (nToolpathCount > 1)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model contains " + std::to_string(nToolpathCount) +
			" toolpaths, but its build does not select one with tp:toolpathid");
	pToolpathIterator->MoveNext();
	return pToolpathIterator->GetCurrentToolpath();
}

uint32_t CEOSContext::readJobLayerThickness(Lib3MF::PModel pModel)
{
	SEOS3MFMetadata meta = readModelMetadata(pModel, m_NamespaceRegistry);
	if (!meta.materialThickness.empty())
		return parseMaterialThicknessInMicrometers(meta.materialThickness);

	auto pWorkingDir = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_ms");
	std::string sMaterialPath = extractMaterialSetFromModel(pModel, pWorkingDir->getPath(), m_NamespaceRegistry);
	if (sMaterialPath.empty())
		return 0;

	std::unique_ptr<IEOSMaterialSet> pMaterialSet(ImportMaterialSet(sMaterialPath));
	return pMaterialSet->GetBaseLayerthickness();
}

void CEOSContext::prepare3MFToolpathJob(Lib3MF::PModel pModel, Lib3MF::PToolpath pToolpath, IMachineConfigFuture* pMachineConfiguration, const SConversionSettings& conversionSettings,
	PConversionLogData pLog, S3MFToolpathJob& job)
{
	LoadSDKIfNotExisting(eSimulationMode::smNoSimulation);
	addSDKInfoToLog(pLog);

	job.m_dModelUnitInMM = getModelUnitInMM(pModel);
	job.m_dToolpathUnitInMM = pToolpath->GetUnits() * job.m_dModelUnitInMM;

	// The job is loaded from the model's meshes, so the OpenJZ part IDs are the ones of enumerateModelParts.
	job.m_Parts = enumerateModelParts(pModel, m_NamespaceRegistry);
	std::vector<uint32_t> partIDs;
	for (auto& part : job.m_Parts) {
		partIDs.push_back(part.partId);
		if (!part.buildItemUUID.empty() && (job.m_BuildItemPartIDs.find(part.buildItemUUID) == job.m_BuildItemPartIDs.end()))
			job.m_BuildItemPartIDs.insert(std::make_pair(part.buildItemUUID, part.partId));
	}

	std::map<uint32_t, std::string> chosenExposureSets;
	auto setPartsSection = [&]() {
		CJSONArray parts;
		for (auto& part : job.m_Parts) {
			CJSONObject partJSON;
			partJSON.addUnsigned("partId", part.partId);
			partJSON.addString("name", part.name);
			partJSON.addString("buildItemUUID", part.buildItemUUID);
			partJSON.addBool("isSupport", part.meta.isSupport);
			partJSON.addString("partParameter", part.meta.partParameter);

			auto iExposureSet = chosenExposureSets.find(part.partId);
			if (!part.meta.partParameter.empty())
				partJSON.addString("exposureSet", part.meta.partParameter);
			else if (iExposureSet != chosenExposureSets.end())
				partJSON.addString("exposureSet", iExposureSet->second);
			else
				partJSON.addNull("exposureSet");

			auto iLayers = job.m_Layout.m_Parts.find(part.partId);
			if (iLayers != job.m_Layout.m_Parts.end()) {
				auto& layers = iLayers->second;
				partJSON.addUnsigned("toolpathLayers", layers.m_nLayerCount);
				if (layers.m_nLayerThickness > 0)
					partJSON.addUnsigned("layerThickness", layers.m_nLayerThickness);
				else
					partJSON.addNull("layerThickness");
				partJSON.addString("layerThicknessSource", layerThicknessSourceToString(layers.m_eThicknessSource));
				if (layers.m_nLayerCount > 0) {
					partJSON.addNumber("firstLayerZ", layers.m_nFirstZ / 1000.0);
					partJSON.addNumber("lastLayerZ", layers.m_nLastZ / 1000.0);
				}
				else {
					partJSON.addNull("firstLayerZ");
					partJSON.addNull("lastLayerZ");
				}
			}
			parts.addRaw(partJSON.toString());
		}
		pLog->setSection("parts", parts.toString());
	};
	setPartsSection();

	job.m_Scan = scanToolpathLayers(pToolpath, job.m_dToolpathUnitInMM, job.m_BuildItemPartIDs);
	uint32_t nFallbackThickness = (job.m_Scan.m_LayerZ.size() < 2) ? readJobLayerThickness(pModel) : 0;
	job.m_Layout = computeToolpathZLayout(job.m_Scan, conversionSettings.m_eLayerZConvention, nFallbackThickness, partIDs, pLog);

	CJSONObject units;
	units.addNumber("modelUnitInMM", job.m_dModelUnitInMM);
	units.addNumber("toolpathUnitInMM", job.m_dToolpathUnitInMM);
	units.addString("layerZConvention", (conversionSettings.m_eLayerZConvention == eEOSLayerZConvention::Base) ? "Base" : "Top");
	units.addString("layerZSource", "toolpathLayerZMax");
	units.addInteger("layerDistance", job.m_Layout.m_nStep);
	units.addInteger("eosZOffset", job.m_Layout.m_nOffset);
	pLog->setSection("units", units.toString());
	setPartsSection();

	std::map<uint32_t, uint32_t> partLayerThickness;
	for (auto& part : job.m_Layout.m_Parts) {
		if (part.second.m_nLayerThickness > 0)
			partLayerThickness.insert(std::make_pair(part.first, part.second.m_nLayerThickness));
	}

	S3MFJobWriteOptions jobOptions;
	jobOptions.m_pSettings = &conversionSettings;
	jobOptions.m_pLog = pLog;
	jobOptions.m_pLayerThicknessOut = &job.m_nJobLayerThickness;
	jobOptions.m_pPartLayerThickness = &partLayerThickness;
	jobOptions.m_pChosenExposureSetsOut = &chosenExposureSets;
	jobOptions.m_pMachineConfiguration = pMachineConfiguration;
	if (!job.m_Layout.m_EOSLayerZ.empty())
		jobOptions.m_dHeightEndInMM = job.m_Layout.m_EOSLayerZ.back() / 1000.0;
	loadJobFrom3MF(pModel, jobOptions);
	setPartsSection();

	if (job.m_nJobLayerThickness > 0) {
		for (auto& part : job.m_Layout.m_Parts) {
			uint32_t nThickness = part.second.m_nLayerThickness;
			if ((part.second.m_nLayerCount > 0) && (nThickness > 0) && (nThickness % job.m_nJobLayerThickness != 0))
				pLog->addMessage(eEOSLogSeverity::Warning, "PART_THICKNESS_NOT_MULTIPLE", LIBEOS_CONVERSIONLOG_NOLAYER, part.first, "the part's layer thickness of " +
					std::to_string(nThickness) + " micrometers is not a multiple of the job's material thickness of " + std::to_string(job.m_nJobLayerThickness) + " micrometers");
		}
	}

	std::unique_ptr<IEOSMachineInfo> pMachineInfo(ActivateMachineConfiguration(pMachineConfiguration));
	if (pMachineInfo->GetVolumeType() == eEOSVolumeType::CuboidVolume) {
		double dDimensionX = 0.0, dDimensionY = 0.0, dDimensionZ = 0.0;
		pMachineInfo->GetDimensions(dDimensionX, dDimensionY, dDimensionZ);
		job.m_dMachineHeightInMM = dDimensionZ;
	}

	job.m_nLaserScannerCount = pMachineInfo->GetNumberOfLaserScanners();
	if (job.m_nLaserScannerCount == 0)
		pLog->addMessage(eEOSLogSeverity::Warning, "SCANNER_COUNT_UNAVAILABLE", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"the machine configuration gives no number of laser scanners, so laser scanners without work in a layer get no part, and the EOS SDK refuses such layers");

	try {
		uint32_t nScannerCount = job.m_nLaserScannerCount;
		for (uint32_t nScannerIndex = 0; nScannerIndex < nScannerCount; nScannerIndex++) {
			bool bFocusDefocus = false, bVariableFocus = false, bBeamShaping = false;
			pMachineInfo->GetLaserScannerCapabilities(nScannerIndex, bFocusDefocus, bVariableFocus, bBeamShaping);
			job.m_LaserScannerCapabilities.push_back((bFocusDefocus ? EOS_LSC_FOCUS_DEFOCUS : 0) | (bVariableFocus ? EOS_LSC_VARIABLE_FOCUS : 0) |
				(bBeamShaping ? EOS_LSC_BEAMSHAPING : 0));
		}
		job.m_bHasLaserScannerCapabilities = true;
	}
	catch (ELib3MF4EOSInterfaceException& e) {
		if (e.getErrorCode() != LIB3MF4EOS_ERROR_COULDNOTGETMACHINEINFORMATION)
			throw;
		job.m_LaserScannerCapabilities.clear();
		pLog->addMessage(eEOSLogSeverity::Info, "SCANNER_CAPABILITIES_UNAVAILABLE", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"the machine configuration gives no laser scanner details, so focus commands are only checked by the EOS SDK");
	}
}

bool CEOSContext::retrieveLayerStack(const S3MFToolpathJob& job, SEOSLayerStack& stack, PConversionLogData pLog)
{
	if (m_pSDK->simulateFullSDK()) {
		stack = simulateLayerStack(job.m_Parts, job.m_Layout, job.m_dMachineHeightInMM);
		return true;
	}

	auto& adapter = m_pSDK->getAdapter();
	EOS_ERROR layersError = adapter.getLayers(stack.m_Layers);
	EOS_ERROR heightsError = (layersError == EOS_ERR_NO_ERROR) ? adapter.getProcessHeights(stack.m_ProcessHeights) : layersError;
	if ((layersError == EOS_ERR_UNSUPPORTED) || (heightsError == EOS_ERR_UNSUPPORTED)) {
		pLog->addMessage(eEOSLogSeverity::Warning, "LAYER_STACK_UNAVAILABLE", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"the EOS SDK does not give the layer stack of the job, so the toolpath layers were not checked against it");
		return false;
	}
	m_pSDK->checkEOSError(layersError);
	m_pSDK->checkEOSError(heightsError);
	return true;
}

IEOSConversionLog* CEOSContext::CheckLayerCoverage(Lib3MF::PModel pModel, IMachineConfigFuture* pMachineConfiguration, IEOSToolpathConversionSettings* pSettings)
{
	if (pModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model is null");
	if (pMachineConfiguration == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "Machine configuration is null");

	SConversionSettings conversionSettings = getConversionSettings(pSettings);
	auto pToolpath = getBuildToolpath(pModel);
	auto pLog = createConversionLog(conversionSettings);

	try {
		S3MFToolpathJob job;
		prepare3MFToolpathJob(pModel, pToolpath, pMachineConfiguration, conversionSettings, pLog, job);

		SEOSLayerStack layerStack;
		if (retrieveLayerStack(job, layerStack, pLog))
			checkLayerCoverage(job.m_Scan, job.m_Layout, layerStack, conversionSettings.m_bStrictMode, false, pLog);
	}
	catch (...) {
		pLog->addMessage(eEOSLogSeverity::Error, "CHECK_FAILED", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, describeCurrentException());
	}

	return new CEOSConversionLog(pLog);
}

IEOSConversionLog* CEOSContext::CheckPartToolpathAlignment(Lib3MF::PModel pModel, const Lib3MF4EOS_double dTolerance)
{
	if (pModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model is null");
	if (!(dTolerance > 0.0))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "the alignment tolerance must be positive");

	auto pToolpath = getBuildToolpath(pModel);

	auto pLog = std::make_shared<CConversionLogData>();
	CJSONObject libEOS;
	libEOS.addString("version", getLibEOSVersionString());
	pLog->setSection("libeos", libEOS.toString());

	double dModelUnitInMM = getModelUnitInMM(pModel);
	double dToolpathUnitInMM = pToolpath->GetUnits() * dModelUnitInMM;

	struct SBounds {
		double m_dMin[3] = { 0.0, 0.0, 0.0 };
		double m_dMax[3] = { 0.0, 0.0, 0.0 };
		bool m_bEmpty = true;

		void add(double dX, double dY, double dZ)
		{
			double values[3] = { dX, dY, dZ };
			for (int nAxis = 0; nAxis < 3; nAxis++) {
				m_dMin[nAxis] = m_bEmpty ? values[nAxis] : (std::min)(m_dMin[nAxis], values[nAxis]);
				m_dMax[nAxis] = m_bEmpty ? values[nAxis] : (std::max)(m_dMax[nAxis], values[nAxis]);
			}
			m_bEmpty = false;
		}
	};

	auto parts = enumerateModelParts(pModel, m_NamespaceRegistry);
	std::map<std::string, uint32_t> buildItemPartIDs;
	std::map<uint32_t, SBounds> meshBounds;
	for (auto& part : parts) {
		if (!part.buildItemUUID.empty() && (buildItemPartIDs.find(part.buildItemUUID) == buildItemPartIDs.end()))
			buildItemPartIDs.insert(std::make_pair(part.buildItemUUID, part.partId));
		auto& bounds = meshBounds[part.partId];
		for (auto& vertex : part.vertices)
			bounds.add(vertex.m_Coordinates[0], vertex.m_Coordinates[1], vertex.m_Coordinates[2]);
	}

	// XY of the vectors, and the Z of the first and last layer of every part.
	std::map<uint32_t, SBounds> toolpathBounds;
	double dLayerDistance = 0.0;
	double dPreviousLayerZ = 0.0;
	uint32_t nLayerCount = pToolpath->GetLayerCount();
	for (uint32_t nLayerIndex = 0; nLayerIndex < nLayerCount; nLayerIndex++) {
		double dLayerZ = pToolpath->GetLayerZMax(nLayerIndex) * dToolpathUnitInMM;
		if ((nLayerIndex > 0) && ((dLayerDistance == 0.0) || (dLayerZ - dPreviousLayerZ < dLayerDistance)))
			dLayerDistance = dLayerZ - dPreviousLayerZ;
		dPreviousLayerZ = dLayerZ;

		Lib3MF::PToolpathLayerReader pLayerReader;
		try {
			pLayerReader = pToolpath->ReadLayerData(nLayerIndex);
		}
		catch (Lib3MF::ELib3MFException& e) {
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "could not read toolpath layer " + std::to_string(nLayerIndex) + " (" + e.what() +
				"); toolpath 3MFs must be read with Reader::ReadFromPersistentSource and the source must stay alive");
		}

		uint32_t nSegmentCount = pLayerReader->GetSegmentCount();
		for (uint32_t nSegmentIndex = 0; nSegmentIndex < nSegmentCount; nSegmentIndex++) {
			auto iPartID = buildItemPartIDs.find(pLayerReader->GetSegmentBuildItemUUID(nSegmentIndex));
			if (iPartID == buildItemPartIDs.end())
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a segment that references a build item without mesh");
			auto& bounds = toolpathBounds[iPartID->second];

			Lib3MF::eToolpathSegmentType eSegmentType;
			uint32_t nPointCount = 0;
			pLayerReader->GetSegmentInfo(nSegmentIndex, eSegmentType, nPointCount);
			if (eSegmentType == Lib3MF::eToolpathSegmentType::Hatch) {
				std::vector<Lib3MF::sHatch2D> hatches;
				pLayerReader->GetSegmentHatchDataInModelUnits(nSegmentIndex, hatches);
				for (auto& hatch : hatches) {
					bounds.add(hatch.m_Point1Coordinates[0] * dModelUnitInMM, hatch.m_Point1Coordinates[1] * dModelUnitInMM, dLayerZ);
					bounds.add(hatch.m_Point2Coordinates[0] * dModelUnitInMM, hatch.m_Point2Coordinates[1] * dModelUnitInMM, dLayerZ);
				}
			}
			else {
				std::vector<Lib3MF::sPosition2D> points;
				pLayerReader->GetSegmentPointDataInModelUnits(nSegmentIndex, points);
				for (auto& point : points)
					bounds.add(point.m_Coordinates[0] * dModelUnitInMM, point.m_Coordinates[1] * dModelUnitInMM, dLayerZ);
			}
		}
	}

	auto boundsToJSON = [](const SBounds& bounds, bool bWithZ) {
		CJSONObject json;
		json.addNumber("minX", bounds.m_dMin[0]);
		json.addNumber("minY", bounds.m_dMin[1]);
		if (bWithZ) json.addNumber("minZ", bounds.m_dMin[2]);
		json.addNumber("maxX", bounds.m_dMax[0]);
		json.addNumber("maxY", bounds.m_dMax[1]);
		if (bWithZ) json.addNumber("maxZ", bounds.m_dMax[2]);
		return json.toString();
	};

	uint64_t nMisalignedParts = 0;
	CJSONArray alignment;
	for (auto& part : parts) {
		auto& mesh = meshBounds[part.partId];
		CJSONObject partJSON;
		partJSON.addUnsigned("partId", part.partId);
		partJSON.addString("name", part.name);
		partJSON.addRaw("mesh", mesh.m_bEmpty ? jsonNull() : boundsToJSON(mesh, true));

		auto iToolpath = toolpathBounds.find(part.partId);
		if ((iToolpath == toolpathBounds.end()) || iToolpath->second.m_bEmpty || mesh.m_bEmpty) {
			partJSON.addNull("toolpath");
			partJSON.addNull("maxDeviation");
			alignment.addRaw(partJSON.toString());
			pLog->addMessage(eEOSLogSeverity::Warning, "PART_WITHOUT_TOOLPATH", LIBEOS_CONVERSIONLOG_NOLAYER, part.partId,
				mesh.m_bEmpty ? std::string("the part has an empty mesh") : std::string("the part has no toolpath"));
			continue;
		}

		auto& toolpath = iToolpath->second;
		CJSONObject toolpathJSON;
		toolpathJSON.addNumber("minX", toolpath.m_dMin[0]);
		toolpathJSON.addNumber("minY", toolpath.m_dMin[1]);
		toolpathJSON.addNumber("maxX", toolpath.m_dMax[0]);
		toolpathJSON.addNumber("maxY", toolpath.m_dMax[1]);
		toolpathJSON.addNumber("firstLayerZ", toolpath.m_dMin[2]);
		toolpathJSON.addNumber("lastLayerZ", toolpath.m_dMax[2]);
		partJSON.addRaw("toolpath", toolpathJSON.toString());

		// Positive differences are vectors inside the mesh bounds, negative ones vectors outside.
		double dDifferences[4] = { toolpath.m_dMin[0] - mesh.m_dMin[0], toolpath.m_dMin[1] - mesh.m_dMin[1],
			mesh.m_dMax[0] - toolpath.m_dMax[0], mesh.m_dMax[1] - toolpath.m_dMax[1] };
		double dMaxDeviation = 0.0;
		for (double dDifference : dDifferences)
			dMaxDeviation = (std::max)(dMaxDeviation, std::fabs(dDifference));
		partJSON.addNumber("maxDeviation", dMaxDeviation);
		alignment.addRaw(partJSON.toString());

		std::vector<std::string> problems;
		if (dMaxDeviation > dTolerance) {
			problems.push_back("the XY bounds of its vectors differ from those of its mesh by up to " + jsonNumber(dMaxDeviation) + " mm (min X " + jsonNumber(dDifferences[0]) +
				", min Y " + jsonNumber(dDifferences[1]) + ", max X " + jsonNumber(dDifferences[2]) + ", max Y " + jsonNumber(dDifferences[3]) + " mm inwards)");

			double dShiftX = ((toolpath.m_dMin[0] - mesh.m_dMin[0]) + (toolpath.m_dMax[0] - mesh.m_dMax[0])) / 2.0;
			double dShiftY = ((toolpath.m_dMin[1] - mesh.m_dMin[1]) + (toolpath.m_dMax[1] - mesh.m_dMax[1])) / 2.0;
			if ((std::fabs(dShiftX) > dTolerance) || (std::fabs(dShiftY) > dTolerance))
				problems.push_back("the vectors appear shifted by " + jsonNumber(dShiftX) + " mm in X and " + jsonNumber(dShiftY) + " mm in Y");
		}

		// A layer Z is the top or the base of the layer, so the layers may extend one layer distance above the mesh.
		if ((toolpath.m_dMin[2] < mesh.m_dMin[2] - dTolerance) || (toolpath.m_dMax[2] > mesh.m_dMax[2] + dLayerDistance + dTolerance))
			problems.push_back("its toolpath layers from Z " + jsonNumber(toolpath.m_dMin[2]) + " to " + jsonNumber(toolpath.m_dMax[2]) + " mm exceed its mesh from Z " +
				jsonNumber(mesh.m_dMin[2]) + " to " + jsonNumber(mesh.m_dMax[2]) + " mm");

		if (problems.empty()) {
			pLog->addMessage(eEOSLogSeverity::Info, "PART_ALIGNED", LIBEOS_CONVERSIONLOG_NOLAYER, part.partId, "the toolpath of the part lies within " +
				jsonNumber(dTolerance) + " mm of the bounds of its mesh");
		}
		else {
			nMisalignedParts++;
			std::string sText;
			for (auto& sProblem : problems)
				sText += (sText.empty() ? "" : "; ") + sProblem;
			pLog->addMessage(eEOSLogSeverity::Warning, "PART_MISALIGNED", LIBEOS_CONVERSIONLOG_NOLAYER, part.partId, sText);
		}
	}

	pLog->setSection("alignment", alignment.toString());
	pLog->setSummaryValue("partsChecked", parts.size());
	pLog->setSummaryValue("partsMisaligned", nMisalignedParts);

	return new CEOSConversionLog(pLog);
}

ITaskCreationFuture* CEOSContext::CreateTaskFrom3MFToolpath(Lib3MF::PModel pModel, IMachineConfigFuture* pMachineConfiguration, IEOSToolpathConversionSettings* pSettings, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion)
{
	if (pModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "3MF model is null");
	if (pMachineConfiguration == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "Machine configuration is null");

	SConversionSettings conversionSettings = getConversionSettings(pSettings);
	auto pToolpath = getBuildToolpath(pModel);
	auto pLog = createConversionLog(conversionSettings);

	// From here on, errors are returned as a failed future that keeps the log.
	S3MFToolpathConversionState state;
	try {
		return convert3MFToolpath(pModel, pToolpath, pMachineConfiguration, conversionSettings, pLog, state, sName, sComment, sGeneratorName, sGeneratorVersion);
	}
	catch (...) {
		std::string sErrorMessage = describeCurrentException();
		addConversionStateToLog(state, pLog);
		pLog->addMessage(eEOSLogSeverity::Error, "CONVERSION_FAILED", state.m_nCurrentLayerIndex, LIBEOS_CONVERSIONLOG_NOPART, sErrorMessage);
		return new CTaskCreationFuture(m_pSDK, sErrorMessage, pLog);
	}
}

ITaskCreationFuture* CEOSContext::convert3MFToolpath(Lib3MF::PModel pModel, Lib3MF::PToolpath pToolpath, IMachineConfigFuture* pMachineConfiguration, const SConversionSettings& conversionSettings,
	PConversionLogData pLog, S3MFToolpathConversionState& state, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion)
{
	auto& nCurrentLayerIndex = state.m_nCurrentLayerIndex;
	auto& profileSettings = state.m_ProfileSettings;
	auto& usedLaserIndices = state.m_UsedLaserIndices;

	// Toolpath profile values are read from the current namespace and any registered legacy namespace.
	std::vector<std::string> acceptedToolpathNamespaces = m_NamespaceRegistry.acceptedNamespaces(Lib3MF4EOS::eEOSNamespaceKind::ToolpathProfile);

	auto refuseLaserSync = [&](int32_t nLayerIndex, const std::string& sReason) {
		std::string sMessage = sReason + "; EOS toolpaths cannot synchronise lasers";
		pLog->addMessage(eEOSLogSeverity::Error, "LASER_SYNC_UNSUPPORTED", nLayerIndex, LIBEOS_CONVERSIONLOG_NOPART, sMessage);
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_LASERSYNCNOTSUPPORTED, sMessage);
	};

	uint32_t nSyncGroupCount = pToolpath->GetSyncGroupCount();
	if (nSyncGroupCount > 0)
		refuseLaserSync(LIBEOS_CONVERSIONLOG_NOLAYER, "the toolpath declares " + std::to_string(nSyncGroupCount) + " laser sync group(s)");

	S3MFToolpathJob job;
	prepare3MFToolpathJob(pModel, pToolpath, pMachineConfiguration, conversionSettings, pLog, job);

	// The task needs every exposed layer of the job's layer stack. A stack layer without toolpath is filled with a layer
	// in which every laser scanner begins a part and exposes nothing. A toolpath off the stack is not filled.
	SEOSLayerStack layerStack;
	if (retrieveLayerStack(job, layerStack, pLog)) {
		uint64_t nLayersNotInStack = checkLayerCoverage(job.m_Scan, job.m_Layout, layerStack, conversionSettings.m_bStrictMode, true, pLog);
		if (nLayersNotInStack > 0) {
			pLog->addMessage(eEOSLogSeverity::Warning, "LAYER_FILL_SKIPPED", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
				"the toolpath layers are not all on the layer stack of the job, so the stack layers without toolpath are not filled");
			layerStack.m_Layers.clear();
		}
	}
	CStackLayerCursor stackLayerCursor(layerStack);

	double dModelUnitInMM = job.m_dModelUnitInMM;
	auto& buildItemPartIDs = job.m_BuildItemPartIDs;
	uint32_t nJobLayerThickness = job.m_nJobLayerThickness;

	auto pWorkingDirectory = std::make_shared<AMCCommon::CWorkingDirectory>("libeos_rw");
	auto pWriter = std::make_unique<CEOSRawTaskWriter>(m_pSDK, pWorkingDirectory, conversionSettings, pLog);
	if (job.m_bHasLaserScannerCapabilities)
		pWriter->setLaserScannerCapabilities(job.m_LaserScannerCapabilities);
	pWriter->setLaserScannerCount(job.m_nLaserScannerCount);

	// Factors this close to [0, 1] are rounding noise and are clamped without a message.
	const double dFactorTolerance = 1.0e-9;

	auto checkFactor = [&](double dFactor, uint32_t nPartID, const std::string& sModifierName) -> double {
		if ((dFactor >= -dFactorTolerance) && (dFactor <= 1.0 + dFactorTolerance))
			return dFactor;
		if (conversionSettings.m_bStrictMode)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_MODIFICATIONFACTOROUTOFRANGE, "toolpath layer " + std::to_string(nCurrentLayerIndex) + " has a " + sModifierName +
				" factor of " + std::to_string(dFactor) + "; factors must be between 0 and 1 in strict mode");
		pLog->addSummaryValue("factorsClamped", 1);
		pLog->addCountedMessage(eEOSLogSeverity::Warning, "FACTOR_CLAMPED", nCurrentLayerIndex, nPartID,
			"a " + sModifierName + " factor outside 0 to 1 was clamped");
		return dFactor;
	};

	// A constant modifier gives a hatch one value, so both ends must carry the same factor.
	auto constantHatchFactor = [&](double dFactor1, double dFactor2, uint32_t nPartID, const std::string& sModifierName) -> double {
		if (std::fabs(dFactor2 - dFactor1) <= dFactorTolerance)
			return dFactor2;
		if (conversionSettings.m_bRefuseUnequalConstantHatch)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_UNEQUALCONSTANTHATCH, "toolpath layer " + std::to_string(nCurrentLayerIndex) + " has a hatch whose constant " +
				sModifierName + " modifier has the factors " + std::to_string(dFactor1) + " and " + std::to_string(dFactor2) + " at its two ends");
		pLog->addCountedMessage(eEOSLogSeverity::Warning, "CONSTANT_HATCH_UNEQUAL", nCurrentLayerIndex, nPartID,
			"a hatch with different constant " + sModifierName + " factors at its ends was written at the factor of its end");
		return dFactor2;
	};

	// The declared laser indices, in ascending order, become the scanners 0 to N-1.
	uint32_t nDefaultLaserIndex = pToolpath->HasDefaultLaserIndex() ? pToolpath->GetDefaultLaserIndex() : 0;
	std::set<uint32_t> declaredLaserIndices;
	uint32_t nLaserSourceCount = pToolpath->GetLaserSourceCount();
	for (uint32_t nSourceIndex = 0; nSourceIndex < nLaserSourceCount; nSourceIndex++)
		declaredLaserIndices.insert(pToolpath->GetLaserSource(nSourceIndex)->GetLaserIndex());
	for (auto nLaserIndex : declaredLaserIndices)
		state.m_DeclaredLaserScanners.insert(std::make_pair(nLaserIndex, (uint32_t)state.m_DeclaredLaserScanners.size()));

	if (state.m_DeclaredLaserScanners.empty()) {
		pLog->addMessage(eEOSLogSeverity::Info, "LASER_MAPPING", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"the toolpath declares no laser sources, so its laser indices are used as scanner indices");
	}
	else {
		std::string sMapping;
		for (auto& declaredLaser : state.m_DeclaredLaserScanners)
			sMapping += (sMapping.empty() ? "" : ", ") + std::string("laser ") + std::to_string(declaredLaser.first) + " to scanner " + std::to_string(declaredLaser.second);
		pLog->addMessage(eEOSLogSeverity::Info, "LASER_MAPPING", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART,
			"the declared laser sources are mapped in ascending order: " + sMapping);
	}

	auto scannerOfLaser = [&](uint32_t nLaserIndex, uint32_t nPartID) -> uint32_t {
		usedLaserIndices.insert(nLaserIndex);
		if (state.m_DeclaredLaserScanners.empty())
			return nLaserIndex;
		auto iScanner = state.m_DeclaredLaserScanners.find(nLaserIndex);
		if (iScanner != state.m_DeclaredLaserScanners.end())
			return iScanner->second;
		pLog->addMessage(eEOSLogSeverity::Error, "LASER_UNDECLARED", nCurrentLayerIndex, nPartID,
			"laser index " + std::to_string(nLaserIndex) + " is used, but the toolpath does not declare it as laser source");
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_LASERINDEXUNDECLARED, "toolpath layer " + std::to_string(nCurrentLayerIndex) + " uses laser index " +
			std::to_string(nLaserIndex) + ", which the toolpath does not declare as laser source");
	};

	// laserfocus gives a focus offset in mm. EOS takes a focus value from 0 to 1, and no conversion between them is defined.
	auto checkLaserFocusOffset = [&](const SToolpathProfileSettings& settings, uint32_t nPartID) {
		if (settings.m_dLaserFocusOffset == 0.0)
			return;
		std::string sOffset = "toolpath profile " + settings.m_sName + " has a laserfocus of " + jsonNumber(settings.m_dLaserFocusOffset) + " mm";
		if (settings.m_bHasFocus) {
			pLog->addMessage(eEOSLogSeverity::Info, "FOCUS_OFFSET_SUPERSEDED", nCurrentLayerIndex, nPartID,
				sOffset + "; its eos:focus of " + jsonNumber(settings.m_dFocus) + " is used instead");
			return;
		}
		std::string sReason = sOffset + ", a focus offset that has no EOS equivalent; give the EOS focus value from 0.0 (fully defocused) to 1.0 (fully focused) as eos:focus";
		if (conversionSettings.m_bStrictMode) {
			pLog->addMessage(eEOSLogSeverity::Error, "FOCUS_OFFSET_REFUSED", nCurrentLayerIndex, nPartID, sReason);
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_FOCUSNOTSUPPORTED, sReason);
		}
		pLog->addMessage(eEOSLogSeverity::Warning, "FOCUS_OFFSET_IGNORED", nCurrentLayerIndex, nPartID,
			sReason + "; the offset is left out, so the profile is exposed fully focused");
	};

	auto checkProfileParameters = [&](const SToolpathProfileSettings& settings, uint32_t nPartID) {
		for (auto& parameter : settings.m_UnsupportedParameters) {
			std::string sReason = "toolpath profile " + settings.m_sName + " has the parameter " + parameter.m_sName + ", which cannot be written to EOS: " + parameter.m_sReason;
			if (conversionSettings.m_bStrictMode) {
				pLog->addMessage(eEOSLogSeverity::Error, "PROFILE_PARAMETER_REFUSED", nCurrentLayerIndex, nPartID, sReason);
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_PROFILEPARAMETERNOTSUPPORTED, sReason);
			}
			pLog->addSummaryValue("profileParametersIgnored", 1);
			pLog->addMessage(eEOSLogSeverity::Warning, "PROFILE_PARAMETER_IGNORED", nCurrentLayerIndex, nPartID, sReason + "; the parameter is left out");
		}
	};

	auto dwellInMicroseconds = [&](double dDwellTime, uint32_t nPartID) -> double {
		if (dDwellTime <= 0.0)
			return 0.0;
		if (conversionSettings.m_eDwellTimeUnit == eEOSDwellTimeUnit::Unset) {
			pLog->addMessage(eEOSLogSeverity::Error, "DWELL_UNIT_MISSING", nCurrentLayerIndex, nPartID,
				"the toolpath has dwell times (prewaittime or postwaittime), but the conversion settings give no dwell time unit");
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_DWELLTIMEUNITMISSING, "toolpath layer " + std::to_string(nCurrentLayerIndex) +
				" has dwell times (prewaittime or postwaittime); set the dwell time unit in the conversion settings");
		}
		return dwellTimeInMicroseconds(dDwellTime, conversionSettings.m_eDwellTimeUnit);
	};

	uint32_t nWrittenLayerCount = 0;
	auto fillStackLayers = [&](const std::vector<sEOSLayerInfo>& stackLayers) {
		for (auto& stackLayer : stackLayers) {
			nWrittenLayerCount++;
			pWriter->writeEmptyLayer(nWrittenLayerCount, stackLayer.m_nZPositionInMicrometers, (uint32_t)stackLayer.m_ExposedPartIDs.front());
		}
	};

	uint32_t nLayerCount = pToolpath->GetLayerCount();
	pLog->setSummaryValue("toolpathLayers", nLayerCount);
	for (uint32_t nLayerIndex = 0; nLayerIndex < nLayerCount; nLayerIndex++) {
		nCurrentLayerIndex = (int32_t)nLayerIndex;
		int64_t nZHeightInMicrometers = job.m_Layout.m_EOSLayerZ[nLayerIndex];
		double dLayerZInMM = nZHeightInMicrometers / 1000.0;

		Lib3MF::PToolpathLayerReader pLayerReader;
		try {
			pLayerReader = pToolpath->ReadLayerData(nLayerIndex);
		}
		catch (Lib3MF::ELib3MFException& e) {
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "could not read toolpath layer " + std::to_string(nLayerIndex) + " (" + e.what() +
				"); toolpath 3MFs must be read with Reader::ReadFromPersistentSource and the source must stay alive");
		}

		std::deque<Lib3MF4EOS::sEOSTaskPoint> points;
		std::deque<Lib3MF4EOS::sEOSTaskBlock> blocks;

		uint32_t nSegmentCount = pLayerReader->GetSegmentCount();
		for (uint32_t nSegmentIndex = 0; nSegmentIndex < nSegmentCount; nSegmentIndex++) {
			Lib3MF::eToolpathSegmentType eSegmentType;
			uint32_t nPointCount = 0;
			pLayerReader->GetSegmentInfo(nSegmentIndex, eSegmentType, nPointCount);

			// EOS lasers run their commands independently; a task has no barrier between them.
			bool bHasLaserSync = false;
			pLayerReader->GetSegmentLaserSync(nSegmentIndex, bHasLaserSync);
			if (bHasLaserSync) {
				std::string sReason = "toolpath layer " + std::to_string(nCurrentLayerIndex) + " has a segment with lasersync, but EOS lasers cannot wait for each other";
				if (conversionSettings.m_bStrictMode) {
					pLog->addMessage(eEOSLogSeverity::Error, "LASER_SYNC_REFUSED", nCurrentLayerIndex, LIBEOS_CONVERSIONLOG_NOPART, sReason);
					throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_LASERSYNCNOTSUPPORTED, sReason);
				}
				pLog->addCountedMessage(eEOSLogSeverity::Warning, "LASER_SYNC_IGNORED", nCurrentLayerIndex, LIBEOS_CONVERSIONLOG_NOPART,
					"a segment with lasersync was written without waiting for the other lasers");
			}

			std::string sBuildItemUUID = pLayerReader->GetSegmentBuildItemUUID(nSegmentIndex);
			auto iPartID = buildItemPartIDs.find(sBuildItemUUID);
			if (iPartID == buildItemPartIDs.end())
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath segment references a build item without mesh: " + sBuildItemUUID);

			std::string sProfileUUID = pLayerReader->GetSegmentDefaultProfileUUID(nSegmentIndex);
			auto iSettings = profileSettings.find(sProfileUUID);
			if (iSettings == profileSettings.end()) {
				iSettings = profileSettings.insert(std::make_pair(sProfileUUID, readToolpathProfileSettings(pLayerReader->GetSegmentDefaultProfile(nSegmentIndex),
					acceptedToolpathNamespaces, conversionSettings.m_bDefaultSkywriting, nDefaultLaserIndex))).first;
				checkLaserFocusOffset(iSettings->second, iPartID->second);
				checkProfileParameters(iSettings->second, iPartID->second);
			}
			auto& settings = iSettings->second;

			Lib3MF4EOS::sEOSTaskBlock block;
			memset((void*)&block, 0, sizeof(block));
			block.m_StartPointIndex = (uint32_t)points.size();
			block.m_PartID = iPartID->second;
			block.m_DefaultLaserPower = settings.m_dLaserPower;
			block.m_ScanSpeed = settings.m_dScanSpeed;
			block.m_LaserFocus = settings.m_dFocus;
			block.m_HasLaserFocus = settings.m_bHasFocus;
			block.m_SkywritingEnabled = settings.m_bSkywriting;
			block.m_ExposureSetIndex = settings.m_nExposureSetIndex;
			block.m_ExposureStepIndex = settings.m_nExposureStepIndex;
			block.m_PulsedWavePeriodTime = settings.m_dPulsedWavePeriodTime;
			block.m_PulsedWavePowerOnTime = settings.m_dPulsedWavePowerOnTime;
			uint32_t nPartLayerThickness = job.m_Layout.getPartLayerThickness(block.m_PartID);
			if (settings.m_nExposedDepth > 0)
				block.m_ExposedDepth = settings.m_nExposedDepth;
			else
				block.m_ExposedDepth = (nPartLayerThickness > 0) ? nPartLayerThickness : nJobLayerThickness;
			block.m_PreDelay = settings.m_dPreDelay;
			block.m_PostDelay = 0.0;
			block.m_BeamShapeProfile = settings.m_nBeamShapeProfile;

			bool bHasLaserIndex = false;
			uint32_t nSegmentLaserIndex = pLayerReader->GetSegmentLaserIndex(nSegmentIndex, bHasLaserIndex);
			block.m_LaserScannerIndex = scannerOfLaser(bHasLaserIndex ? nSegmentLaserIndex : settings.m_nLaserIndex, block.m_PartID);

			const SVectorValues baseValues = { settings.m_dLaserPower, settings.m_dScanSpeed, settings.m_dFocus };

			auto addPoint = [&](double dXInModelUnits, double dYInModelUnits, const SVectorValues& values) {
				Lib3MF4EOS::sEOSTaskPoint point;
				memset((void*)&point, 0, sizeof(point));
				point.m_X = dXInModelUnits * dModelUnitInMM;
				point.m_Y = dYInModelUnits * dModelUnitInMM;
				point.m_LaserPower = values.m_dLaserPower;
				point.m_ScanSpeed = values.m_dScanSpeed;
				point.m_LaserFocus = values.m_dLaserFocus;
				point.m_HasLaserFocus = true;
				points.push_back(point);
			};

			// Ends the current block at the last point; the next block of the segment starts at the next point, without the pre-delay.
			auto splitBlock = [&]() {
				block.m_EndPointIndex = (uint32_t)points.size();
				if (block.m_EndPointIndex > block.m_StartPointIndex) {
					blocks.push_back(block);
					block.m_PreDelay = 0.0;
				}
				block.m_StartPointIndex = (uint32_t)points.size();
			};

			for (auto& unsupportedModifier : settings.m_UnsupportedModifiers) {
				if (pLayerReader->SegmentHasModificationFactors(nSegmentIndex, unsupportedModifier.m_eFactor))
					throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " modifies '" + unsupportedModifier.m_sName +
						"' of profile " + pLayerReader->GetSegmentDefaultProfile(nSegmentIndex)->GetName() + "; " + unsupportedModifier.m_sReason);
			}

			std::vector<const SConstantModifier*> activeConstantModifiers;
			for (auto& constantModifier : settings.m_ConstantModifiers) {
				if (pLayerReader->SegmentHasModificationFactors(nSegmentIndex, constantModifier.m_eFactor))
					activeConstantModifiers.push_back(&constantModifier);
			}

			// Laser power, scan speed and focus can change along a hatch and between the vectors of a polyline.
			struct SActiveValueModifier {
				const SValueModifier* m_pModifier;
				double SVectorValues::* m_pValue;
				std::string m_sName;
			};
			std::vector<SActiveValueModifier> activeValueModifiers;
			auto addActiveValueModifier = [&](const SValueModifier& modifier, double SVectorValues::* pValue, const std::string& sName) {
				if (modifier.m_bActive && pLayerReader->SegmentHasModificationFactors(nSegmentIndex, modifier.m_eFactor))
					activeValueModifiers.push_back(SActiveValueModifier{ &modifier, pValue, sName });
			};
			addActiveValueModifier(settings.m_PowerModifier, &SVectorValues::m_dLaserPower, "laserpower");
			addActiveValueModifier(settings.m_SpeedModifier, &SVectorValues::m_dScanSpeed, "laserspeed");
			addActiveValueModifier(settings.m_FocusModifier, &SVectorValues::m_dLaserFocus, "eos:focus");

			bool bHasDwellFactors = settings.m_DwellModifier.m_bActive && pLayerReader->SegmentHasModificationFactors(nSegmentIndex, settings.m_DwellModifier.m_eFactor);
			auto modifierValue = [](const SValueModifier& modifier, double dFactor) {
				return CPowerRamp::modifiedValue(modifier.m_dMinValue, modifier.m_dMaxValue, dFactor);
			};

			switch (eSegmentType) {
			case Lib3MF::eToolpathSegmentType::Hatch: {
				std::vector<Lib3MF::sHatch2D> hatches;
				pLayerReader->GetSegmentHatchDataInModelUnits(nSegmentIndex, hatches);
				block.m_Type = Lib3MF4EOS::eEOSTaskBlockType::HatchBlock;
				block.m_ExposureType = settings.m_bHasExposureType ? settings.m_nExposureType : conversionSettings.m_nDefaultHatchExposureType;

				// Values of the constant modifiers, one row per hatch.
				std::vector<std::vector<double>> constantValues;
				for (auto pConstantModifier : activeConstantModifiers) {
					std::vector<Lib3MF::sHatch2DFactors> hatchFactors;
					pLayerReader->GetLinearSegmentHatchModificationFactors(nSegmentIndex, pConstantModifier->m_eFactor, hatchFactors);
					if (hatchFactors.size() != hatches.size())
						throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a hatch segment with " +
							std::to_string(hatchFactors.size()) + " " + pConstantModifier->m_sName + " factors for " + std::to_string(hatches.size()) + " hatches");
					constantValues.resize(hatches.size());
					for (size_t nHatchIndex = 0; nHatchIndex < hatches.size(); nHatchIndex++) {
						const std::string& sName = pConstantModifier->m_sName;
						double dFactor1 = checkFactor(hatchFactors[nHatchIndex].m_Point1Factor, block.m_PartID, sName);
						double dFactor2 = checkFactor(hatchFactors[nHatchIndex].m_Point2Factor, block.m_PartID, sName);
						double dFactor = constantHatchFactor(dFactor1, dFactor2, block.m_PartID, sName);
						constantValues[nHatchIndex].push_back(CPowerRamp::modifiedValue(pConstantModifier->m_dMinValue, pConstantModifier->m_dMaxValue, dFactor));
					}
				}

				// The dwell after every hatch: postwaittime at the hatch end, plus eos:postdelay.
				std::vector<double> hatchDwells(hatches.size(), settings.m_dPostDelay + dwellInMicroseconds(settings.m_dDwellTime, block.m_PartID));
				if (bHasDwellFactors) {
					auto& dwellModifier = settings.m_DwellModifier;
					std::vector<Lib3MF::sHatch2DFactors> dwellFactors;
					pLayerReader->GetLinearSegmentHatchModificationFactors(nSegmentIndex, dwellModifier.m_eFactor, dwellFactors);
					if (dwellFactors.size() != hatches.size())
						throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a hatch segment with " +
							std::to_string(dwellFactors.size()) + " postwaittime factors for " + std::to_string(hatches.size()) + " hatches");
					for (size_t nHatchIndex = 0; nHatchIndex < hatches.size(); nHatchIndex++) {
						double dFactor1 = checkFactor(dwellFactors[nHatchIndex].m_Point1Factor, block.m_PartID, "postwaittime");
						double dFactor2 = checkFactor(dwellFactors[nHatchIndex].m_Point2Factor, block.m_PartID, "postwaittime");
						if (dwellModifier.m_eType == Lib3MF::eToolpathProfileModificationType::ConstantModification)
							dFactor2 = constantHatchFactor(dFactor1, dFactor2, block.m_PartID, "postwaittime");
						hatchDwells[nHatchIndex] = settings.m_dPostDelay + dwellInMicroseconds(modifierValue(dwellModifier, dFactor2), block.m_PartID);
					}
				}

				// prewaittime waits before every hatch: before the first one as pre-delay, before every other one after the hatch in front of it.
				double dHatchPreWait = dwellInMicroseconds(settings.m_dPreWaitTime, block.m_PartID);
				if ((dHatchPreWait > 0.0) && !hatches.empty()) {
					block.m_PreDelay += dHatchPreWait;
					for (size_t nHatchIndex = 0; nHatchIndex + 1 < hatches.size(); nHatchIndex++)
						hatchDwells[nHatchIndex] += dHatchPreWait;
				}

				// EOS sets pulse and dwell per block, so the segment is split where they change.
				auto applyBlockValues = [&](size_t nHatchIndex) {
					bool bChanged = (hatchDwells[nHatchIndex] != block.m_PostDelay);
					if (!constantValues.empty()) {
						auto& values = constantValues[nHatchIndex];
						for (size_t nIndex = 0; nIndex < activeConstantModifiers.size(); nIndex++)
							bChanged = bChanged || (getConstantModifierTarget(block, activeConstantModifiers[nIndex]->m_eTarget) != values[nIndex]);
					}
					if (!bChanged)
						return;
					splitBlock();
					block.m_PostDelay = hatchDwells[nHatchIndex];
					if (!constantValues.empty()) {
						for (size_t nIndex = 0; nIndex < activeConstantModifiers.size(); nIndex++)
							setConstantModifierTarget(block, activeConstantModifiers[nIndex]->m_eTarget, constantValues[nHatchIndex][nIndex]);
					}
				};

				// Factors and nonlinear interpolation points of every value modifier.
				struct SHatchModifierData {
					std::vector<Lib3MF::sHatch2DFactors> m_Factors;
					std::vector<uint32_t> m_InterpolationCounts;
					std::vector<Lib3MF::sHatchModificationInterpolationData> m_Interpolations;
					size_t m_nInterpolationOffset = 0;
				};
				std::vector<SHatchModifierData> modifierData(activeValueModifiers.size());
				bool bHasNonlinearData = !activeValueModifiers.empty() && pLayerReader->SegmentHasNonlinearHatchModificationInterpolation(nSegmentIndex);
				for (size_t nModifier = 0; nModifier < activeValueModifiers.size(); nModifier++) {
					auto& modifier = *activeValueModifiers[nModifier].m_pModifier;
					auto& sName = activeValueModifiers[nModifier].m_sName;
					auto& data = modifierData[nModifier];
					pLayerReader->GetLinearSegmentHatchModificationFactors(nSegmentIndex, modifier.m_eFactor, data.m_Factors);
					if (data.m_Factors.size() != hatches.size())
						throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a hatch segment with " +
							std::to_string(data.m_Factors.size()) + " " + sName + " factors for " + std::to_string(hatches.size()) + " hatches");
					if ((modifier.m_eType == Lib3MF::eToolpathProfileModificationType::NonlinearModification) && bHasNonlinearData) {
						pLayerReader->GetSegmentAllNonlinearHatchesModificationInterpolation(nSegmentIndex, modifier.m_eFactor, data.m_InterpolationCounts, data.m_Interpolations);
						if (data.m_InterpolationCounts.size() != hatches.size())
							throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a hatch segment with " +
								std::to_string(data.m_InterpolationCounts.size()) + " " + sName + " interpolations for " + std::to_string(hatches.size()) + " hatches");
					}
				}

				std::vector<std::vector<std::pair<double, double>>> valueCurves(activeValueModifiers.size());
				std::vector<double> parameters;
				for (size_t nHatchIndex = 0; nHatchIndex < hatches.size(); nHatchIndex++) {
					auto& hatch = hatches[nHatchIndex];
					applyBlockValues(nHatchIndex);

					// The value of every modifier along the hatch, as a curve over the hatch parameter from 0 to 1.
					parameters.assign({ 0.0, 1.0 });
					for (size_t nModifier = 0; nModifier < activeValueModifiers.size(); nModifier++) {
						auto& modifier = *activeValueModifiers[nModifier].m_pModifier;
						auto& sName = activeValueModifiers[nModifier].m_sName;
						auto& data = modifierData[nModifier];
						auto& curve = valueCurves[nModifier];

						double dFactor1 = checkFactor(data.m_Factors[nHatchIndex].m_Point1Factor, block.m_PartID, sName);
						double dFactor2 = checkFactor(data.m_Factors[nHatchIndex].m_Point2Factor, block.m_PartID, sName);
						if (modifier.m_eType == Lib3MF::eToolpathProfileModificationType::ConstantModification) {
							dFactor2 = constantHatchFactor(dFactor1, dFactor2, block.m_PartID, sName);
							dFactor1 = dFactor2;
						}

						curve.clear();
						curve.push_back(std::make_pair(0.0, modifierValue(modifier, dFactor1)));
						if (!data.m_InterpolationCounts.empty()) {
							uint32_t nCount = data.m_InterpolationCounts[nHatchIndex];
							if (data.m_nInterpolationOffset + nCount > data.m_Interpolations.size())
								throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has inconsistent " + sName + " interpolation data");
							for (uint32_t nIndex = 0; nIndex < nCount; nIndex++) {
								auto& interpolation = data.m_Interpolations[data.m_nInterpolationOffset + nIndex];
								if ((interpolation.m_Parameter <= curve.back().first) || (interpolation.m_Parameter >= 1.0))
									throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has " + sName +
										" interpolation parameters that are not strictly increasing between 0 and 1");
								curve.push_back(std::make_pair(interpolation.m_Parameter, modifierValue(modifier, checkFactor(interpolation.m_Factor, block.m_PartID, sName))));
								parameters.push_back(interpolation.m_Parameter);
							}
							data.m_nInterpolationOffset += nCount;
						}
						curve.push_back(std::make_pair(1.0, modifierValue(modifier, dFactor2)));
					}
					std::sort(parameters.begin(), parameters.end());
					parameters.erase(std::unique(parameters.begin(), parameters.end()), parameters.end());

					auto valuesAt = [&](double dT) {
						SVectorValues values = baseValues;
						for (size_t nModifier = 0; nModifier < activeValueModifiers.size(); nModifier++)
							values.*(activeValueModifiers[nModifier].m_pValue) = curveValueAt(valueCurves[nModifier], dT);
						return values;
					};

					// Every piece between two curve points becomes a sub-hatch; the writer treats touching collinear hatches as one line.
					auto addHatchPoint = [&](double dT) {
						if (dT >= 1.0)
							addPoint(hatch.m_Point2Coordinates[0], hatch.m_Point2Coordinates[1], valuesAt(1.0));
						else
							addPoint(hatch.m_Point1Coordinates[0] + dT * (hatch.m_Point2Coordinates[0] - hatch.m_Point1Coordinates[0]),
								hatch.m_Point1Coordinates[1] + dT * (hatch.m_Point2Coordinates[1] - hatch.m_Point1Coordinates[1]), valuesAt(dT));
					};
					// The dwell follows the hatch, not each of its pieces: only the last piece is in a block with the dwell, so the
					// writer joins the pieces into one line that ends with it.
					size_t nPieceCount = parameters.size() - 1;
					double dHatchDwell = block.m_PostDelay;
					for (size_t nPiece = 0; nPiece < nPieceCount; nPiece++) {
						if ((dHatchDwell > 0.0) && (nPieceCount > 1)) {
							double dPieceDwell = (nPiece + 1 < nPieceCount) ? 0.0 : dHatchDwell;
							if (block.m_PostDelay != dPieceDwell) {
								splitBlock();
								block.m_PostDelay = dPieceDwell;
							}
						}
						addHatchPoint(parameters[nPiece]);
						addHatchPoint(parameters[nPiece + 1]);
					}
				}
				break;
			}
			case Lib3MF::eToolpathSegmentType::Loop:
			case Lib3MF::eToolpathSegmentType::Polyline: {
				std::vector<Lib3MF::sPosition2D> polylinePoints;
				pLayerReader->GetSegmentPointDataInModelUnits(nSegmentIndex, polylinePoints);

				// Pulse settings can only change between blocks, and splitting a polyline would switch the laser off.
				// The first factor belongs to the jump, except for loops, where it also belongs to the closing vector.
				bool bIsLoop = (eSegmentType == Lib3MF::eToolpathSegmentType::Loop);
				size_t nFirstMarkFactor = bIsLoop ? 0 : 1;
				for (auto pConstantModifier : activeConstantModifiers) {
					std::vector<double> pointFactors;
					pLayerReader->GetSegmentPointModificationFactors(nSegmentIndex, pConstantModifier->m_eFactor, pointFactors);
					if (pointFactors.size() != polylinePoints.size())
						throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a polyline segment with " +
							std::to_string(pointFactors.size()) + " " + pConstantModifier->m_sName + " factors for " + std::to_string(polylinePoints.size()) + " points");
					if (pointFactors.size() <= nFirstMarkFactor)
						continue;
					for (size_t nIndex = nFirstMarkFactor; nIndex < pointFactors.size(); nIndex++)
						checkFactor(pointFactors[nIndex], block.m_PartID, pConstantModifier->m_sName);

					double dValue = CPowerRamp::modifiedValue(pConstantModifier->m_dMinValue, pConstantModifier->m_dMaxValue, pointFactors[nFirstMarkFactor]);
					for (size_t nIndex = nFirstMarkFactor + 1; nIndex < pointFactors.size(); nIndex++) {
						if (CPowerRamp::modifiedValue(pConstantModifier->m_dMinValue, pConstantModifier->m_dMaxValue, pointFactors[nIndex]) != dValue)
							throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a polyline segment whose " +
								pConstantModifier->m_sName + " changes along the polyline; EOS can only change it between segments");
					}
					setConstantModifierTarget(block, pConstantModifier->m_eTarget, dValue);
				}
				block.m_Type = Lib3MF4EOS::eEOSTaskBlockType::PolylineBlock;
				block.m_ExposureType = settings.m_bHasExposureType ? settings.m_nExposureType : conversionSettings.m_nDefaultContourExposureType;

				// The values at every point. A vector ends with the values of the point it arrives at.
				std::vector<SVectorValues> pointValues(polylinePoints.size(), baseValues);
				std::vector<std::vector<double>> modifierValues(activeValueModifiers.size());
				for (size_t nModifier = 0; nModifier < activeValueModifiers.size(); nModifier++) {
					auto& modifier = *activeValueModifiers[nModifier].m_pModifier;
					auto& sName = activeValueModifiers[nModifier].m_sName;
					std::vector<double> pointFactors;
					pLayerReader->GetSegmentPointModificationFactors(nSegmentIndex, modifier.m_eFactor, pointFactors);
					if (pointFactors.size() != polylinePoints.size())
						throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a polyline segment with " +
							std::to_string(pointFactors.size()) + " " + sName + " factors for " + std::to_string(polylinePoints.size()) + " points");
					modifierValues[nModifier].resize(pointFactors.size());
					for (size_t nIndex = 0; nIndex < pointFactors.size(); nIndex++) {
						if (nIndex >= nFirstMarkFactor)
							checkFactor(pointFactors[nIndex], block.m_PartID, sName);
						double dValue = modifierValue(modifier, pointFactors[nIndex]);
						modifierValues[nModifier][nIndex] = dValue;
						pointValues[nIndex].*(activeValueModifiers[nModifier].m_pValue) = dValue;
					}
				}

				// The dwell after the vector that arrives at every point.
				std::vector<double> pointDwells(polylinePoints.size(), 0.0);
				for (size_t nIndex = nFirstMarkFactor; nIndex < polylinePoints.size(); nIndex++)
					pointDwells[nIndex] = dwellInMicroseconds(settings.m_dDwellTime, block.m_PartID);
				if (bHasDwellFactors) {
					std::vector<double> dwellFactors;
					pLayerReader->GetSegmentPointModificationFactors(nSegmentIndex, settings.m_DwellModifier.m_eFactor, dwellFactors);
					if (dwellFactors.size() != polylinePoints.size())
						throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "toolpath layer " + std::to_string(nLayerIndex) + " has a polyline segment with " +
							std::to_string(dwellFactors.size()) + " postwaittime factors for " + std::to_string(polylinePoints.size()) + " points");
					for (size_t nIndex = nFirstMarkFactor; nIndex < dwellFactors.size(); nIndex++)
						pointDwells[nIndex] = dwellInMicroseconds(modifierValue(settings.m_DwellModifier, checkFactor(dwellFactors[nIndex], block.m_PartID, "postwaittime")), block.m_PartID);
				}

				if (bIsLoop && !polylinePoints.empty()) {
					polylinePoints.push_back(polylinePoints[0]);
					pointValues.push_back(pointValues[0]);
					for (auto& values : modifierValues)
						values.push_back(values[0]);
					pointDwells.push_back(pointDwells[0]);
				}

				// prewaittime waits before every vector: before the first one as pre-delay, before every other one at the point it starts from.
				double dVectorPreWait = dwellInMicroseconds(settings.m_dPreWaitTime, block.m_PartID);
				if ((dVectorPreWait > 0.0) && (polylinePoints.size() >= 2)) {
					block.m_PreDelay += dVectorPreWait;
					for (size_t nIndex = 1; nIndex + 1 < polylinePoints.size(); nIndex++)
						pointDwells[nIndex] += dVectorPreWait;
				}

				// 3MF ramps linear modifiers along every vector, EOS uses one value per vector.
				// A constant modifier makes each vector a step to the value of the point it arrives at.
				std::vector<SPowerRampSubVector> subVectors;
				size_t nLastPoint = polylinePoints.empty() ? 0 : polylinePoints.size() - 1;
				for (size_t nIndex = 0; nIndex < polylinePoints.size(); nIndex++) {
					double dX = polylinePoints[nIndex].m_Coordinates[0];
					double dY = polylinePoints[nIndex].m_Coordinates[1];
					if (nIndex == 0) {
						addPoint(dX, dY, pointValues[nIndex]);
						continue;
					}

					double dPreviousX = polylinePoints[nIndex - 1].m_Coordinates[0];
					double dPreviousY = polylinePoints[nIndex - 1].m_Coordinates[1];
					SVectorValues endValues = pointValues[nIndex];
					SVectorValues startValues = endValues;
					for (size_t nModifier = 0; nModifier < activeValueModifiers.size(); nModifier++) {
						if (activeValueModifiers[nModifier].m_pModifier->m_eType != Lib3MF::eToolpathProfileModificationType::ConstantModification)
							startValues.*(activeValueModifiers[nModifier].m_pValue) = modifierValues[nModifier][nIndex - 1];
					}

					// Zero-length vectors only carry a value change, which the next vector already has.
					bool bZeroLength = (dX == dPreviousX) && (dY == dPreviousY);
					if (!bZeroLength) {
						if (!CPowerRamp::isRamp(startValues, endValues)) {
							addPoint(dX, dY, endValues);
						}
						else {
							pLog->addSummaryValue("rampedPolylineVectors", 1);
							switch (conversionSettings.m_ePowerRampMode) {
							case Lib3MF4EOS::eEOSPowerRampMode::Reject:
								throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_POWERRAMPREJECTED, "toolpath layer " + std::to_string(nLayerIndex) + " contains a ramp of laser power, scan speed or focus on a polyline");
							case Lib3MF4EOS::eEOSPowerRampMode::Flatten:
								addPoint(dX, dY, CPowerRamp::interpolate(startValues, endValues, 0.5));
								pLog->addCountedMessage(eEOSLogSeverity::Warning, "POWER_RAMP_FLATTENED", nCurrentLayerIndex, block.m_PartID,
									"a ramp of laser power, scan speed or focus was written at its mean values");
								break;
							default:
								subVectors.clear();
								CPowerRamp::appendSubVectors(conversionSettings, dPreviousX * dModelUnitInMM, dPreviousY * dModelUnitInMM, startValues,
									dX * dModelUnitInMM, dY * dModelUnitInMM, endValues, subVectors);
								for (auto& subVector : subVectors)
									addPoint(subVector.m_dEndX / dModelUnitInMM, subVector.m_dEndY / dModelUnitInMM, subVector.m_Values);
								pLog->addSummaryValue("rampSubVectors", subVectors.size());
								break;
							}
						}
					}

					// A dwell inside the polyline ends the block there; the next block starts at the same point.
					if ((nIndex < nLastPoint) && (pointDwells[nIndex] > 0.0)) {
						block.m_PostDelay = pointDwells[nIndex];
						splitBlock();
						block.m_PostDelay = 0.0;
						addPoint(dX, dY, endValues);
					}
				}
				block.m_PostDelay = settings.m_dPostDelay + (pointDwells.empty() ? 0.0 : pointDwells.back());
				break;
			}
			default:
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "unsupported toolpath segment type in layer " + std::to_string(nLayerIndex));
			}

			block.m_EndPointIndex = (uint32_t)points.size();
			if (block.m_EndPointIndex > block.m_StartPointIndex)
				blocks.push_back(block);
		}

		if (blocks.empty()) {
			pLog->addSummaryValue("layersSkipped", 1);
			pLog->addMessage(eEOSLogSeverity::Info, "LAYER_EMPTY_SKIPPED", nCurrentLayerIndex, LIBEOS_CONVERSIONLOG_NOPART,
				"the layer at Z " + jsonNumber(dLayerZInMM) + " mm has no exposure and was not written");
			continue;
		}

		fillStackLayers(stackLayerCursor.passLayersUpTo(nZHeightInMicrometers));

		CEOSRawTaskLayer layer(nZHeightInMicrometers);
		layer.initializeData(points, blocks);
		nWrittenLayerCount++;
		pWriter->writeLayer(nWrittenLayerCount, &layer, nCurrentLayerIndex);
	}
	nCurrentLayerIndex = LIBEOS_CONVERSIONLOG_NOLAYER;
	fillStackLayers(stackLayerCursor.passRemainingLayers());
	addConversionStateToLog(state, pLog);

	return pWriter->ConvertToTask(sName, sComment, sGeneratorName, sGeneratorVersion);
}
