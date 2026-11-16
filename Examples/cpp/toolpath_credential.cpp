/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Stores, checks and deletes the EOS Toolpath API client key in the Windows Credential Manager.

  Example 06 reads the client key from the credential named with --credential, so the
  PEM file issued by EOS only has to be imported once and can then be removed from the computer.
  The key is stored as a generic credential of the current Windows user and never printed.

*/

#include "example_common.hpp"

using namespace LibEOSExamples;

static std::string readClientKeyFromFile(const std::string& sFileName)
{
	std::ifstream stream(sFileName, std::ios::in | std::ios::binary);
	if (!stream.is_open())
		throw std::runtime_error("could not open file: " + sFileName);
	std::stringstream content;
	content << stream.rdbuf();
	std::string sClientKey = content.str();
	if ((sClientKey.size() >= 3) && (sClientKey.compare(0, 3, "\xEF\xBB\xBF") == 0))
		sClientKey.erase(0, 3);
	return sClientKey;
}

static std::string readClientKeyFromConsole()
{
	std::cout << "Paste the PEM text of the client key, ending with its -----END ...----- line:" << std::endl;
	std::string sClientKey, sLine;
	while (std::getline(std::cin, sLine)) {
		if (!sLine.empty() && (sLine.back() == '\r'))
			sLine.pop_back();
		if (sLine.empty() && sClientKey.empty())
			continue;
		sClientKey += sLine + "\n";
		if (sLine.rfind("-----END ", 0) == 0)
			break;
	}
	secureErase(sLine);
	return sClientKey;
}

static void storeCredential(const std::string& sCredentialName, std::string& sClientKey)
{
	if (!isPEMText(sClientKey))
		throw EExampleUsage("the client key is not PEM text (-----BEGIN ...----- / -----END ...-----)");
	if (sClientKey.size() > CRED_MAX_CREDENTIAL_BLOB_SIZE)
		throw EExampleUsage("the client key is larger than a Windows credential can hold (" + std::to_string(CRED_MAX_CREDENTIAL_BLOB_SIZE) + " bytes)");

	std::wstring sTargetName = utf8ToWide(sCredentialName);
	std::wstring sComment = L"EOS Toolpath API client key (PEM) for LibEOS";
	std::wstring sUserName = L"LibEOS";

	CREDENTIALW credential = {};
	credential.Type = CRED_TYPE_GENERIC;
	credential.TargetName = &sTargetName[0];
	credential.Comment = &sComment[0];
	credential.UserName = &sUserName[0];
	credential.CredentialBlobSize = (DWORD)sClientKey.size();
	credential.CredentialBlob = (LPBYTE)&sClientKey[0];
	credential.Persist = CRED_PERSIST_LOCAL_MACHINE;

	if (!CredWriteW(&credential, 0))
		throw std::runtime_error("could not write Windows credential \"" + sCredentialName + "\" (error " + std::to_string(GetLastError()) + ")");
}

static std::string pemLabel(const std::string& sClientKey)
{
	auto nBegin = sClientKey.find("-----BEGIN ");
	if (nBegin == std::string::npos)
		return "";
	auto nEnd = sClientKey.find("-----", nBegin + 11);
	if (nEnd == std::string::npos)
		return "";
	return sClientKey.substr(nBegin + 11, nEnd - nBegin - 11);
}

int main(int argc, char** argv)
{
	CExampleArguments arguments(
		"Manages the EOS Toolpath API client key in the Windows Credential Manager of the current user.\n\n"
		"  store <name> [<client-key.pem>]  Stores the PEM text read from the file, or pasted into the console\n"
		"  check <name>                     Checks that the credential exists and holds PEM text\n"
		"  delete <name>                    Deletes the credential\n\n"
		"Pass the name to the examples with --credential <name>. " + TOOLPATH_API_NOTICE,
		"<store|check|delete> <name> [<client-key.pem>]");

	return runExample(arguments, argc, argv, [&]() {
		std::string sCommand = arguments.positional(0, "command");
		std::string sCredentialName = arguments.positional(1, "name");

		if (sCommand == "store") {
			bool bFromFile = arguments.positional().size() > 2;
			std::string sClientKey = bFromFile ? readClientKeyFromFile(arguments.positional(2, "client-key.pem")) : readClientKeyFromConsole();
			try {
				storeCredential(sCredentialName, sClientKey);
			}
			catch (...) {
				secureErase(sClientKey);
				throw;
			}
			secureErase(sClientKey);

			std::cout << "Stored the client key as Windows credential \"" << sCredentialName << "\"." << std::endl;
			if (bFromFile)
				std::cout << "LibEOS no longer needs " << arguments.positional(2, "client-key.pem")
					<< "; remove it from this computer and keep the original in a secure place." << std::endl;
		}
		else if (sCommand == "check") {
			std::string sClientKey = readToolpathClientKey(sCredentialName);
			std::cout << "Windows credential \"" << sCredentialName << "\" holds a PEM " << pemLabel(sClientKey)
				<< " (" << sClientKey.size() << " bytes)." << std::endl;
			secureErase(sClientKey);
		}
		else if (sCommand == "delete") {
			if (!CredDeleteW(utf8ToWide(sCredentialName).c_str(), CRED_TYPE_GENERIC, 0)) {
				DWORD nError = GetLastError();
				if (nError == ERROR_NOT_FOUND)
					throw EExampleUsage("no Windows credential named \"" + sCredentialName + "\"");
				throw std::runtime_error("could not delete Windows credential \"" + sCredentialName + "\" (error " + std::to_string(nError) + ")");
			}
			std::cout << "Deleted Windows credential \"" << sCredentialName << "\"." << std::endl;
		}
		else {
			throw EExampleUsage("unknown command: " + sCommand);
		}
	});
}
