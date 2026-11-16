/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Shared helpers for the LibEOS C++ examples: argument parsing, library loading,
          waiting for futures with progress output, and error reporting.

*/

#ifndef __LIBEOS_EXAMPLE_COMMON
#define __LIBEOS_EXAMPLE_COMMON

#include "lib3mftoolpath4eos_dynamic.hpp"
#include "lib3mf_dynamic.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <wincred.h>
#endif

namespace LibEOSExamples {

const std::string GENERATOR_NAME = "LibEOS Examples";
const std::string GENERATOR_VERSION = "1.0.0";
// The current LibEOS job metadata namespace and material set relationship type. These match
// EOSContext::GetNamespace / GetMaterialSetRelationshipType; where a context is at hand (see
// ensureMaterialSet) prefer those so the URIs are not duplicated.
const std::string EOS3MF_NAMESPACE = "http://schemas.3mf.io/eos/job/2026/11";
const std::string EOS3MF_MATERIALSET_RELATIONSHIPTYPE = "http://schemas.3mf.io/eos/materialset/2026/11";

class EExampleUsage : public std::runtime_error {
public:
	EExampleUsage(const std::string& sMessage) : std::runtime_error(sMessage) {}
};

// ---------------------------------------------------------------------------
// Command line
// ---------------------------------------------------------------------------

struct SExampleOption {
	std::string m_sName;
	std::string m_sEnvironmentVariable;
	bool m_bIsFlag;
	std::string m_sDescription;
};

class CExampleArguments {
private:
	std::string m_sProgramName;
	std::string m_sPositionalUsage;
	std::string m_sDescription;
	std::vector<SExampleOption> m_Options;
	std::map<std::string, std::string> m_Values;
	std::vector<std::string> m_Positional;

	const SExampleOption* findOption(const std::string& sName) const
	{
		for (auto& option : m_Options)
			if (option.m_sName == sName)
				return &option;
		return nullptr;
	}

public:
	CExampleArguments(const std::string& sDescription, const std::string& sPositionalUsage)
		: m_sDescription(sDescription), m_sPositionalUsage(sPositionalUsage)
	{
		addOption("help", "", true, "Show this help.");
	}

	void addOption(const std::string& sName, const std::string& sEnvironmentVariable, bool bIsFlag, const std::string& sDescription)
	{
		m_Options.push_back({ sName, sEnvironmentVariable, bIsFlag, sDescription });
	}

	// Shared options of all examples that talk to the EOS SDK.
	void addSDKOptions()
	{
		addOption("sdk", "LIBEOS_SDK_DIR", false, "EOSPRINT API directory (auto-detected if omitted).");
		addOption("log", "", false, "EOS SDK log file (a temporary file is used if omitted).");
	}

	void addMachineConfigOption()
	{
		addOption("machine-config", "LIBEOS_MACHINE_CONFIG", false, "Machine configuration: a directory or a machine package file.");
	}

	void addToolpathAPIOptions()
	{
		addOption("client-id", "LIBEOS_TPAPI_CLIENT_ID", false, "Toolpath API client ID.");
		addOption("credential", "LIBEOS_TPAPI_CREDENTIAL", false, "Name of the Windows Credential Manager entry holding the Toolpath API client key (see toolpath_credential).");
	}

	void addMachineOptions()
	{
		addOption("machine", "LIBEOS_MACHINE_ADDRESS", false, "Network address of the machine. Without it, the connection is simulated.");
		addOption("store-task", "", false, "Also copy the created task into this directory.");
	}

	void addConversionOptions()
	{
		addOption("power-ramp-mode", "", false, "How laser power ramps are written: polyline (default), split-hatches, flatten or reject.");
		addOption("conversion-log", "", false, "Write the conversion log as JSON to this file.");
	}

	void parse(int argc, char** argv)
	{
		m_sProgramName = (argc > 0) ? argv[0] : "example";
		auto nSlash = m_sProgramName.find_last_of("/\\");
		if (nSlash != std::string::npos)
			m_sProgramName = m_sProgramName.substr(nSlash + 1);

		for (int nIndex = 1; nIndex < argc; nIndex++) {
			std::string sArgument = argv[nIndex];
			if (sArgument.rfind("--", 0) == 0) {
				std::string sName = sArgument.substr(2);
				const SExampleOption* pOption = findOption(sName);
				if (pOption == nullptr)
					throw EExampleUsage("unknown option: " + sArgument);

				if (pOption->m_bIsFlag) {
					m_Values[sName] = "1";
				}
				else {
					if (nIndex + 1 >= argc)
						throw EExampleUsage("missing value for option: " + sArgument);
					m_Values[sName] = argv[++nIndex];
				}
			}
			else {
				m_Positional.push_back(sArgument);
			}
		}

		for (auto& option : m_Options) {
			if (option.m_sEnvironmentVariable.empty() || (m_Values.find(option.m_sName) != m_Values.end()))
				continue;
			const char* pszValue = std::getenv(option.m_sEnvironmentVariable.c_str());
			if ((pszValue != nullptr) && (*pszValue != 0))
				m_Values[option.m_sName] = pszValue;
		}
	}

	bool helpRequested() const
	{
		return has("help");
	}

	const std::string& programName() const
	{
		return m_sProgramName;
	}

	void printUsage() const
	{
		std::cout << m_sDescription << std::endl << std::endl;
		std::cout << "Usage: " << m_sProgramName << " [options] " << m_sPositionalUsage << std::endl << std::endl;
		std::cout << "Options:" << std::endl;
		for (auto& option : m_Options) {
			std::string sSyntax = "--" + option.m_sName + (option.m_bIsFlag ? "" : " <value>");
			std::cout << "  " << sSyntax;
			for (size_t nPad = sSyntax.size(); nPad < 28; nPad++)
				std::cout << ' ';
			std::cout << option.m_sDescription;
			if (!option.m_sEnvironmentVariable.empty())
				std::cout << " [env: " << option.m_sEnvironmentVariable << "]";
			std::cout << std::endl;
		}
	}

	bool has(const std::string& sName) const
	{
		return m_Values.find(sName) != m_Values.end();
	}

	std::string get(const std::string& sName, const std::string& sDefault = "") const
	{
		auto iIter = m_Values.find(sName);
		return (iIter != m_Values.end()) ? iIter->second : sDefault;
	}

	std::string require(const std::string& sName) const
	{
		if (!has(sName))
			throw EExampleUsage("missing required option: --" + sName);
		return get(sName);
	}

	const std::vector<std::string>& positional() const
	{
		return m_Positional;
	}

	std::string positional(size_t nIndex, const std::string& sName) const
	{
		if (nIndex >= m_Positional.size())
			throw EExampleUsage("missing argument: " + sName);
		return m_Positional[nIndex];
	}
};

// ---------------------------------------------------------------------------
// Files and libraries
// ---------------------------------------------------------------------------

inline std::string getExecutableDirectory()
{
#ifdef _WIN32
	std::vector<wchar_t> buffer(MAX_PATH);
	while (true) {
		DWORD nLength = GetModuleFileNameW(nullptr, buffer.data(), (DWORD)buffer.size());
		if (nLength == 0)
			throw std::runtime_error("could not determine executable path");
		if (nLength < buffer.size()) {
			std::wstring sPath(buffer.data(), nLength);
			auto nSlash = sPath.find_last_of(L"/\\");
			if (nSlash != std::wstring::npos)
				sPath = sPath.substr(0, nSlash);

			int nSize = WideCharToMultiByte(CP_UTF8, 0, sPath.c_str(), (int)sPath.size(), nullptr, 0, nullptr, nullptr);
			std::string sResult(nSize, '\0');
			WideCharToMultiByte(CP_UTF8, 0, sPath.c_str(), (int)sPath.size(), &sResult[0], nSize, nullptr, nullptr);
			return sResult;
		}
		buffer.resize(buffer.size() * 2);
	}
#else
	return ".";
#endif
}

inline bool isDirectory(const std::string& sPath)
{
#ifdef _WIN32
	DWORD nAttributes = GetFileAttributesA(sPath.c_str());
	return (nAttributes != INVALID_FILE_ATTRIBUTES) && ((nAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
	return false;
#endif
}

inline std::string fileExtension(const std::string& sFileName)
{
	auto nDot = sFileName.find_last_of('.');
	auto nSlash = sFileName.find_last_of("/\\");
	if ((nDot == std::string::npos) || ((nSlash != std::string::npos) && (nDot < nSlash)))
		return "";
	std::string sExtension = sFileName.substr(nDot + 1);
	for (auto& c : sExtension)
		c = (char)tolower((unsigned char)c);
	return sExtension;
}

// ---------------------------------------------------------------------------
// Toolpath API client key
// ---------------------------------------------------------------------------

// The Toolpath API client key is a confidential credential issued by EOS GmbH. The examples never read
// it from a file: its PEM text is kept as a generic credential in the Windows Credential Manager,
// which encrypts it for the current Windows user, and is referenced by the credential name.

const std::string TOOLPATH_API_NOTICE =
	"Using the EOS Toolpath API requires a contract with EOS GmbH. EOS toolpaths must not be passed on to third "
	"parties in 3MF or any other human-readable format; see \"EOS Toolpath API\" in README.md.";

inline void secureErase(std::string& sSecret)
{
#ifdef _WIN32
	if (!sSecret.empty())
		SecureZeroMemory(&sSecret[0], sSecret.size());
#endif
	sSecret.clear();
}

inline bool isPEMText(const std::string& sText)
{
	return (sText.find("-----BEGIN ") != std::string::npos) && (sText.find("-----END ") != std::string::npos);
}

#ifdef _WIN32
inline std::wstring utf8ToWide(const std::string& sText)
{
	if (sText.empty())
		return std::wstring();
	int nSize = MultiByteToWideChar(CP_UTF8, 0, sText.c_str(), (int)sText.size(), nullptr, 0);
	std::wstring sResult(nSize, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, sText.c_str(), (int)sText.size(), &sResult[0], nSize);
	return sResult;
}

// Credentials entered through the Credential Manager UI or cmdkey are stored as UTF-16.
inline std::string credentialBlobToString(const BYTE* pBlob, DWORD nBlobSize)
{
	bool bIsUTF16 = (nBlobSize >= 2) && ((nBlobSize % 2) == 0) && (pBlob[1] == 0);
	if (!bIsUTF16)
		return std::string((const char*)pBlob, nBlobSize);

	const wchar_t* pWide = (const wchar_t*)pBlob;
	int nWideLength = (int)(nBlobSize / 2);
	int nSize = WideCharToMultiByte(CP_UTF8, 0, pWide, nWideLength, nullptr, 0, nullptr, nullptr);
	std::string sResult(nSize, '\0');
	WideCharToMultiByte(CP_UTF8, 0, pWide, nWideLength, &sResult[0], nSize, nullptr, nullptr);
	return sResult;
}
#endif

// Returns the PEM text of the client key stored under sCredentialName. The caller erases it after use.
inline std::string readToolpathClientKey(const std::string& sCredentialName)
{
#ifdef _WIN32
	PCREDENTIALW pCredential = nullptr;
	if (!CredReadW(utf8ToWide(sCredentialName).c_str(), CRED_TYPE_GENERIC, 0, &pCredential)) {
		DWORD nError = GetLastError();
		if (nError == ERROR_NOT_FOUND)
			throw EExampleUsage("no Windows credential named \"" + sCredentialName + "\"; store the client key with: toolpath_credential store " + sCredentialName);
		throw std::runtime_error("could not read Windows credential \"" + sCredentialName + "\" (error " + std::to_string(nError) + ")");
	}

	std::string sClientKey = credentialBlobToString(pCredential->CredentialBlob, pCredential->CredentialBlobSize);
	if (pCredential->CredentialBlobSize > 0)
		SecureZeroMemory(pCredential->CredentialBlob, pCredential->CredentialBlobSize);
	CredFree(pCredential);

	if (!isPEMText(sClientKey)) {
		secureErase(sClientKey);
		throw std::runtime_error("Windows credential \"" + sCredentialName + "\" does not contain a PEM client key");
	}
	return sClientKey;
#else
	throw std::runtime_error("the Toolpath API client key is read from the Windows Credential Manager, which is only available on Windows");
#endif
}

struct SLibraries {
	Lib3MF4EOS::PWrapper m_pLibEOS;
	Lib3MF::PWrapper m_pLib3MF;
	// Toolpath layers are read lazily, so the sources of loaded 3MFs must stay open.
	std::vector<Lib3MF::PPersistentReaderSource> m_PersistentSources;
};

// Loads lib3mftoolpath4eos.dll and lib3mf.dll from the executable's directory and injects Lib3MF into LibEOS,
// so that both work on the same Lib3MF instance.
inline SLibraries loadLibraries()
{
	std::string sDirectory = getExecutableDirectory();

	SLibraries libraries;
	libraries.m_pLib3MF = Lib3MF::CWrapper::loadLibrary(sDirectory + "/lib3mf.dll");
	libraries.m_pLibEOS = Lib3MF4EOS::CWrapper::loadLibrary(sDirectory + "/lib3mftoolpath4eos.dll");
	libraries.m_pLibEOS->InjectComponent("Lib3MF", libraries.m_pLib3MF->GetSymbolLookupMethod());

	Lib3MF4EOS_uint32 nMajor, nMinor, nMicro;
	libraries.m_pLibEOS->GetVersion(nMajor, nMinor, nMicro);
	std::cout << "LibEOS " << nMajor << "." << nMinor << "." << nMicro;
	Lib3MF_uint32 n3MFMajor, n3MFMinor, n3MFMicro;
	libraries.m_pLib3MF->GetLibraryVersion(n3MFMajor, n3MFMinor, n3MFMicro);
	std::cout << ", Lib3MF " << n3MFMajor << "." << n3MFMinor << "." << n3MFMicro << std::endl;

	return libraries;
}

inline Lib3MF::PModel read3MF(SLibraries& libraries, const std::string& sFileName)
{
	std::cout << "Reading " << sFileName << std::endl;
	auto pModel = libraries.m_pLib3MF->CreateModel();
	auto pReader = pModel->QueryReader("3mf");
	pReader->SetStrictModeActive(false);
	// lib3mf drops attachments with custom relationship types unless they are registered before reading.
	pReader->AddRelationToRead(EOS3MF_MATERIALSET_RELATIONSHIPTYPE);

	auto pSource = pModel->CreatePersistentSourceFromFile(sFileName);
	pReader->ReadFromPersistentSource(pSource.get());
	libraries.m_PersistentSources.push_back(pSource);
	return pModel;
}

inline void write3MF(Lib3MF::PModel pModel, const std::string& sFileName)
{
	std::cout << "Writing " << sFileName << std::endl;
	auto pWriter = pModel->QueryWriter("3mf");
	pWriter->WriteToFile(sFileName);
}

// ---------------------------------------------------------------------------
// Futures
// ---------------------------------------------------------------------------

// Polls a future until it stops running. The progress callback returns the text shown after the step name.
inline void waitForFuture(Lib3MF4EOS::CFuture* pFuture, const std::string& sStep, const std::function<std::string()>& progress = nullptr)
{
	std::cout << sStep << "..." << std::flush;
	std::string sLastProgress;
	while (pFuture->IsRunning()) {
		if (progress) {
			std::string sProgress = progress();
			if (sProgress != sLastProgress) {
				std::cout << "\r" << sStep << "... " << sProgress << "   " << std::flush;
				sLastProgress = sProgress;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	if (pFuture->HasFailed()) {
		std::cout << std::endl;
		throw std::runtime_error(sStep + " failed: " + pFuture->GetErrorMessage());
	}
	if (pFuture->HasBeenAborted()) {
		std::cout << std::endl;
		throw std::runtime_error(sStep + " was aborted");
	}
	if (!pFuture->HasFinished()) {
		std::cout << std::endl;
		throw std::runtime_error(sStep + " did not finish");
	}

	std::cout << "\r" << sStep << "... done" << std::string(sLastProgress.size() + 3, ' ') << std::endl;
}

inline void waitForTaskCreation(Lib3MF4EOS::PTaskCreationFuture pTask, const std::string& sStep)
{
	waitForFuture(pTask.get(), sStep, [pTask]() {
		Lib3MF4EOS_uint32 nCurrentLayer = 0, nNumberOfLayers = 0, nCurrentZHeight = 0, nTargetZHeight = 0;
		pTask->GetProgress(nCurrentLayer, nNumberOfLayers, nCurrentZHeight, nTargetZHeight);
		return "layer " + std::to_string(nCurrentLayer) + " / " + std::to_string(nNumberOfLayers);
	});
}

inline void waitForToolpathComputation(Lib3MF4EOS::PToolpathComputationFuture pComputation, const std::string& sStep)
{
	waitForFuture(pComputation.get(), sStep, [pComputation]() {
		Lib3MF4EOS_uint32 nCurrentLayer = 0, nNumberOfLayers = 0, nCurrentZHeight = 0, nTargetZHeight = 0;
		pComputation->GetProgress(nCurrentLayer, nNumberOfLayers, nCurrentZHeight, nTargetZHeight);
		return "layer " + std::to_string(nCurrentLayer) + " / " + std::to_string(nNumberOfLayers);
	});
}

inline void waitForUpload(Lib3MF4EOS::PTaskUploadFuture pUpload)
{
	waitForFuture(pUpload.get(), "Uploading task", [pUpload]() {
		return std::to_string((int)(pUpload->GetProgress() * 100.0)) + "%";
	});
}

// ---------------------------------------------------------------------------
// EOS context
// ---------------------------------------------------------------------------

// Loads the SDK if necessary and prints its version and the notes of the version selection.
inline void printSDKVersion(Lib3MF4EOS::PEOSContext pContext)
{
	Lib3MF4EOS_uint32 nMajor = 0, nMinor = 0, nBuild = 0, nRevision = 0;
	pContext->GetSDKProductVersion(nMajor, nMinor, nBuild, nRevision);
	Lib3MF4EOS_uint32 nAdapterMajor = 0, nAdapterMinor = 0;
	pContext->GetSDKAdapterVersion(nAdapterMajor, nAdapterMinor);

	std::string sProductVersion = "unknown";
	if (nMajor != 0)
		sProductVersion = std::to_string(nMajor) + "." + std::to_string(nMinor) + "." + std::to_string(nBuild) + "." + std::to_string(nRevision);

	std::cout << "EOS SDK " << sProductVersion << " (using " << nAdapterMajor << "." << nAdapterMinor << " declarations) from " << pContext->GetSDKFilePath() << std::endl;

	std::stringstream messages(pContext->GetSDKVersionMessages());
	std::string sMessage;
	while (std::getline(messages, sMessage)) {
		if (!sMessage.empty())
			std::cout << "  " << sMessage << std::endl;
	}
}

// Creates the context. Simulated machine connectivity must be enabled before the SDK is loaded,
// so examples that connect to a machine pass whether the connection should be simulated.
inline Lib3MF4EOS::PEOSContext createContext(const SLibraries& libraries, const CExampleArguments& arguments, bool bSimulateMachineConnection = false)
{
	auto pContext = libraries.m_pLibEOS->CreateContext(arguments.get("sdk"), arguments.get("log"));

	if (bSimulateMachineConnection) {
		std::cout << "No --machine given, simulating the machine connection" << std::endl;
		pContext->EnableSDKSimulation(Lib3MF4EOS::eEOSSimulationMode::SimulateConnectivity);
	}

	printSDKVersion(pContext);
	return pContext;
}

// Embeds --material-set if given. Otherwise the model must already carry a material set, because
// LibEOS takes the machine target, parameter set and layer thickness from it.
inline void ensureMaterialSet(Lib3MF4EOS::PEOSContext pContext, Lib3MF::PModel pModel, const CExampleArguments& arguments)
{
	if (arguments.has("material-set")) {
		std::cout << "Embedding material set " << arguments.get("material-set") << std::endl;
		pContext->AttachMaterialSetTo3MF(pModel, arguments.get("material-set"));
		return;
	}

	std::string sRelationshipType = pContext->GetMaterialSetRelationshipType();
	for (uint32_t nIndex = 0; nIndex < pModel->GetAttachmentCount(); nIndex++) {
		if (pModel->GetAttachment(nIndex)->GetRelationShipType() == sRelationshipType)
			return;
	}

	throw EExampleUsage("the 3MF has no embedded EOS material set; pass one with --material-set <file.eospar>");
}

inline void authenticateToolpathAPI(Lib3MF4EOS::PEOSContext pContext, const CExampleArguments& arguments)
{
	std::string sClientID = arguments.require("client-id");
	std::string sCredentialName = arguments.require("credential");

	uint32_t nClientID = 0;
	try {
		nClientID = (uint32_t)std::stoul(sClientID);
	}
	catch (...) {
		throw EExampleUsage("invalid client ID: " + sClientID);
	}

	std::cout << "Authenticating Toolpath API as client " << nClientID << " with credential \"" << sCredentialName << "\"" << std::endl;
	try {
		pContext->AuthenticateToolpathAPIFromCredentialStore(nClientID, sCredentialName);
	}
	catch (Lib3MF4EOS::ELib3MF4EOSException& e) {
		if (e.getErrorCode() == LIB3MF4EOS_ERROR_TOOLPATHCREDENTIALNOTFOUND)
			throw EExampleUsage("no Windows credential named \"" + sCredentialName + "\"; store the client key with: toolpath_credential store " + sCredentialName);
		throw;
	}
}

inline Lib3MF4EOS::PMachineConfigFuture loadMachineConfig(Lib3MF4EOS::PEOSContext pContext, const std::string& sPath)
{
	Lib3MF4EOS::PMachineConfigFuture pConfig;
	if (isDirectory(sPath))
		pConfig = pContext->LoadMachineConfigFromDirectory(sPath);
	else
		pConfig = pContext->LoadMachinePackageFromFile(sPath);

	waitForFuture(pConfig.get(), "Loading machine configuration " + sPath);
	return pConfig;
}

inline Lib3MF4EOS::PMachineSessionFuture connectToMachine(Lib3MF4EOS::PEOSContext pContext, const CExampleArguments& arguments)
{
	std::string sAddress = arguments.get("machine", "simulated-machine");
	auto pSession = pContext->ConnectToMachine(sAddress);
	waitForFuture(pSession.get(), "Connecting to " + sAddress);
	return pSession;
}

// Uses --machine-config if given. Otherwise the configuration is downloaded from the machine,
// which is not possible for a simulated connection.
inline Lib3MF4EOS::PMachineConfigFuture getMachineConfigForUpload(Lib3MF4EOS::PEOSContext pContext, Lib3MF4EOS::PMachineSessionFuture pSession, const CExampleArguments& arguments)
{
	if (arguments.has("machine-config"))
		return loadMachineConfig(pContext, arguments.get("machine-config"));

	if (!arguments.has("machine"))
		throw EExampleUsage("--machine-config is required when the machine connection is simulated");

	auto pConfig = pSession->DownloadMachineConfig();
	waitForFuture(pConfig.get(), "Downloading machine configuration");
	return pConfig;
}

inline void printMachineInfo(Lib3MF4EOS::PEOSMachineInfo pMachineInfo)
{
	std::cout << "Active machine: " << pMachineInfo->GetModelTypeProductName()
		<< " (" << pMachineInfo->GetModelTypeIdentifier() << "), serial " << pMachineInfo->GetSerialNumber()
		<< ", " << pMachineInfo->GetNumberOfLaserScanners() << " laser scanner(s)" << std::endl;
}

// Default conversion settings, changed by the options of addConversionOptions.
inline Lib3MF4EOS::PEOSToolpathConversionSettings createConversionSettings(Lib3MF4EOS::PEOSContext pContext, const CExampleArguments& arguments)
{
	auto pSettings = pContext->CreateToolpathConversionSettings();

	if (arguments.has("power-ramp-mode")) {
		std::string sMode = arguments.get("power-ramp-mode");
		if (sMode == "polyline")
			pSettings->SetPowerRampMode(Lib3MF4EOS::eEOSPowerRampMode::Polyline);
		else if (sMode == "split-hatches")
			pSettings->SetPowerRampMode(Lib3MF4EOS::eEOSPowerRampMode::SplitHatches);
		else if (sMode == "flatten")
			pSettings->SetPowerRampMode(Lib3MF4EOS::eEOSPowerRampMode::Flatten);
		else if (sMode == "reject")
			pSettings->SetPowerRampMode(Lib3MF4EOS::eEOSPowerRampMode::Reject);
		else
			throw EExampleUsage("unknown power ramp mode: " + sMode);
	}

	return pSettings;
}

// Prints the message counts, warnings, errors and non-zero counters, and writes --conversion-log.
inline void printConversionLog(Lib3MF4EOS::PEOSConversionLog pLog, const CExampleArguments& arguments)
{
	const uint32_t nMaxPrintedMessages = 20;

	std::cout << "Conversion log: " << pLog->GetCountBySeverity(Lib3MF4EOS::eEOSLogSeverity::Info) << " info, "
		<< pLog->GetCountBySeverity(Lib3MF4EOS::eEOSLogSeverity::Warning) << " warning(s), "
		<< pLog->GetCountBySeverity(Lib3MF4EOS::eEOSLogSeverity::Error) << " error(s)" << std::endl;

	uint32_t nPrinted = 0;
	uint32_t nMessageCount = pLog->GetLogMessageCount();
	for (uint32_t nIndex = 0; nIndex < nMessageCount; nIndex++) {
		Lib3MF4EOS::eEOSLogSeverity eSeverity;
		std::string sCode, sText;
		Lib3MF4EOS_int32 nLayerIndex = 0;
		Lib3MF4EOS_uint32 nPartID = 0;
		pLog->GetLogMessage(nIndex, eSeverity, sCode, nLayerIndex, nPartID, sText);
		if (eSeverity == Lib3MF4EOS::eEOSLogSeverity::Info)
			continue;

		if (nPrinted == nMaxPrintedMessages) {
			std::cout << "  ... see --conversion-log for all messages" << std::endl;
			break;
		}
		nPrinted++;

		std::cout << "  " << ((eSeverity == Lib3MF4EOS::eEOSLogSeverity::Error) ? "error" : "warning") << " " << sCode;
		if (nLayerIndex >= 0)
			std::cout << " (layer " << nLayerIndex << ")";
		if (nPartID != 0)
			std::cout << " (part " << nPartID << ")";
		std::cout << ": " << sText << std::endl;
	}

	uint32_t nSummaryCount = pLog->GetSummaryValueCount();
	for (uint32_t nIndex = 0; nIndex < nSummaryCount; nIndex++) {
		std::string sKey;
		Lib3MF4EOS_uint64 nValue = 0;
		pLog->GetSummaryValueByIndex(nIndex, sKey, nValue);
		if (nValue != 0)
			std::cout << "  " << sKey << ": " << nValue << std::endl;
	}

	if (arguments.has("conversion-log")) {
		std::string sFileName = arguments.get("conversion-log");
		std::ofstream stream(sFileName, std::ios::out | std::ios::binary);
		if (!stream.is_open())
			throw std::runtime_error("could not write conversion log: " + sFileName);
		stream << pLog->ToJSON();
		std::cout << "Wrote conversion log " << sFileName << std::endl;
	}
}

// Waits for a task created from a toolpath and prints its conversion log, also if the creation failed.
inline void waitForTaskCreationWithLog(Lib3MF4EOS::PTaskCreationFuture pTask, const std::string& sStep, const CExampleArguments& arguments)
{
	try {
		waitForTaskCreation(pTask, sStep);
	}
	catch (...) {
		printConversionLog(pTask->GetConversionLog(), arguments);
		throw;
	}
	printConversionLog(pTask->GetConversionLog(), arguments);
}

inline void storeTaskIfRequested(Lib3MF4EOS::PTaskCreationFuture pTask, const CExampleArguments& arguments)
{
	if (!arguments.has("store-task"))
		return;
	std::string sDirectory = arguments.get("store-task");
	pTask->StoreInDirectory(sDirectory);
	std::cout << "Stored task " << pTask->FindTaskFileName() << " in " << sDirectory << std::endl;
}

inline void uploadTask(Lib3MF4EOS::PMachineSessionFuture pSession, Lib3MF4EOS::PTaskCreationFuture pTask)
{
	auto pUpload = pSession->UploadTask(pTask.get());
	waitForUpload(pUpload);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

inline int runExample(CExampleArguments& arguments, int argc, char** argv, const std::function<void()>& example)
{
	try {
		arguments.parse(argc, argv);
		if (arguments.helpRequested()) {
			arguments.printUsage();
			return 0;
		}

		example();
		std::cout << "Finished successfully." << std::endl;
		return 0;
	}
	catch (EExampleUsage& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Run \"" << arguments.programName() << " --help\" for usage." << std::endl;
		return 2;
	}
	catch (Lib3MF4EOS::ELib3MF4EOSException& e) {
		std::cerr << "LibEOS error: " << e.what() << std::endl;
		return 1;
	}
	catch (Lib3MF::ELib3MFException& e) {
		std::cerr << "Lib3MF error: " << e.what() << std::endl;
		return 1;
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}

} // namespace LibEOSExamples

#endif // __LIBEOS_EXAMPLE_COMMON
