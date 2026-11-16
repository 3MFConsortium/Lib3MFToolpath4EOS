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


Abstract: Tests of the Toolpath API client key handling: reading the key from the Windows Credential
          Manager (UTF-8 and UTF-16 blobs, missing and invalid credentials), the challenge encryption
          with a generated key pair, and the "client key" wording of the error messages.
          The credential tests write a generic credential with a unique name and delete it again.

*/

#include "libeos_unittests.hpp"
#include "libeos_sslcrypt.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

#include <Windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <wincred.h>

#include <memory>
#include <vector>

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

	static const std::string TEST_PEM =
		"-----BEGIN RSA PUBLIC KEY-----\n"
		"MIIBCgKCAQEAunittestunittestunittestunittestunittestunittestunittest\n"
		"-----END RSA PUBLIC KEY-----\n";

	// Writes a generic credential with a unique name and deletes it when going out of scope.
	class CTemporaryCredential {
	private:
		std::string m_sName;
		bool m_bWritten;

	public:
		CTemporaryCredential(const std::vector<uint8_t>& blob)
			: m_bWritten(false)
		{
			m_sName = "Lib3MF4EOS-UnitTest-" + std::to_string(GetCurrentProcessId()) + "-" + std::to_string(GetTickCount64());
			std::wstring sTargetName(m_sName.begin(), m_sName.end());
			std::vector<uint8_t> blobCopy(blob);

			CREDENTIALW credential = {};
			credential.Type = CRED_TYPE_GENERIC;
			credential.TargetName = &sTargetName[0];
			credential.CredentialBlobSize = (DWORD)blobCopy.size();
			credential.CredentialBlob = blobCopy.empty() ? nullptr : blobCopy.data();
			credential.Persist = CRED_PERSIST_SESSION;
			m_bWritten = (CredWriteW(&credential, 0) != FALSE);
		}

		~CTemporaryCredential()
		{
			if (m_bWritten) {
				std::wstring sTargetName(m_sName.begin(), m_sName.end());
				CredDeleteW(sTargetName.c_str(), CRED_TYPE_GENERIC, 0);
			}
		}

		bool isWritten() const
		{
			return m_bWritten;
		}

		const std::string& getName() const
		{
			return m_sName;
		}
	};

	static std::vector<uint8_t> utf8Blob(const std::string& sText)
	{
		return std::vector<uint8_t>(sText.begin(), sText.end());
	}

	static std::vector<uint8_t> utf16Blob(const std::string& sText)
	{
		std::wstring sWide(sText.begin(), sText.end());
		const uint8_t* pBytes = (const uint8_t*)sWide.data();
		return std::vector<uint8_t>(pBytes, pBytes + sWide.size() * sizeof(wchar_t));
	}

	struct SKeyHandleDeleter {
		void operator()(void* p) const
		{
			BCryptDestroyKey((BCRYPT_KEY_HANDLE)p);
		}
	};

	class CToolpathCredentialTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "ToolpathCredential";
		}

		void registerTests() override
		{
			registerTest("ReadsUTF8Credential", "a credential stored as UTF-8 bytes returns the PEM text", [this]() {
				CTemporaryCredential credential(utf8Blob(TEST_PEM));
				if (!requireCredential(credential))
					return;
				std::string sKey = readToolpathClientKeyFromCredentialStore(credential.getName());
				assertEqualString(sKey, TEST_PEM);
				secureErase(sKey);
			});

			registerTest("ReadsUTF16Credential", "a credential stored as UTF-16, as by the Credential Manager UI, returns UTF-8 PEM text", [this]() {
				CTemporaryCredential credential(utf16Blob(TEST_PEM));
				if (!requireCredential(credential))
					return;
				std::string sKey = readToolpathClientKeyFromCredentialStore(credential.getName());
				assertEqualString(sKey, TEST_PEM);
				secureErase(sKey);
			});

			registerTest("MissingCredential", "a credential name that does not exist fails with TOOLPATHCREDENTIALNOTFOUND", [this]() {
				std::string sMessage = assertThrowsError([]() {
					readToolpathClientKeyFromCredentialStore("Lib3MF4EOS-UnitTest-does-not-exist-" + std::to_string(GetTickCount64()));
				}, LIB3MF4EOS_ERROR_TOOLPATHCREDENTIALNOTFOUND);
				assertContains(sMessage, "no Windows credential named");
			});

			registerTest("CredentialWithoutPEM", "a credential without PEM text fails with TOOLPATHCREDENTIALINVALID", [this]() {
				CTemporaryCredential credential(utf8Blob("not a key"));
				if (!requireCredential(credential))
					return;
				std::string sName = credential.getName();
				std::string sMessage = assertThrowsError([sName]() {
					readToolpathClientKeyFromCredentialStore(sName);
				}, LIB3MF4EOS_ERROR_TOOLPATHCREDENTIALINVALID);
				assertContains(sMessage, "does not hold the PEM text of a client key");
			});

			registerTest("EmptyCredentialName", "an empty credential name fails with INVALIDPARAM", [this]() {
				assertThrowsError([]() {
					readToolpathClientKeyFromCredentialStore("");
				}, LIB3MF4EOS_ERROR_INVALIDPARAM);
			});

			registerTest("SecureEraseClears", "secureErase leaves an empty string", [this]() {
				std::string sSecret = TEST_PEM;
				secureErase(sSecret);
				assertTrue(sSecret.empty());
			});

			registerTest("InvalidKeySaysClientKey", "errors about an unusable key speak of the client key, not a public key", [this]() {
				std::string sMessage = assertThrowsError([]() {
					CSSLRSACryptoHelper helper("-----BEGIN PUBLIC KEY-----\nAAAA\n-----END PUBLIC KEY-----\n");
					uint8_t challenge[16] = { 1 };
					helper.encryptChallenge(challenge, sizeof(challenge));
				}, LIB3MF4EOS_ERROR_TOOLPATHAUTHENTICATIONFAILED);
				assertContains(sMessage, "client key");
				assertTrue(sMessage.find("public key") == std::string::npos, "no \"public key\" wording");
			});

			registerTest("EncryptsChallengeForKeyPair", "the challenge encrypted with a PKCS#1 client key decrypts with the matching private key", [this]() {
				BCRYPT_ALG_HANDLE hAlgorithm = nullptr;
				assertTrue(BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_RSA_ALGORITHM, nullptr, 0)), "open RSA provider");

				BCRYPT_KEY_HANDLE hRawKey = nullptr;
				bool bGenerated = BCRYPT_SUCCESS(BCryptGenerateKeyPair(hAlgorithm, &hRawKey, 2048, 0)) && BCRYPT_SUCCESS(BCryptFinalizeKeyPair(hRawKey, 0));
				std::unique_ptr<void, SKeyHandleDeleter> pKey(hRawKey);
				BCryptCloseAlgorithmProvider(hAlgorithm, 0);
				assertTrue(bGenerated, "generate key pair");

				std::string sPEM = exportPublicKeyAsPEM(hRawKey);

				std::vector<uint8_t> challenge(32);
				for (size_t nIndex = 0; nIndex < challenge.size(); nIndex++)
					challenge[nIndex] = (uint8_t)(nIndex * 7 + 3);

				CSSLRSACryptoHelper helper(sPEM);
				assertTrue(helper.encryptChallenge(challenge.data(), challenge.size()), "encrypt challenge");

				std::vector<uint8_t>& response = helper.getResponseBuffer();
				BCRYPT_OAEP_PADDING_INFO paddingInfo = {};
				paddingInfo.pszAlgId = BCRYPT_SHA1_ALGORITHM;
				ULONG nDecryptedSize = 0;
				std::vector<uint8_t> decrypted(response.size());
				assertTrue(BCRYPT_SUCCESS(BCryptDecrypt(hRawKey, response.data(), (ULONG)response.size(), &paddingInfo, nullptr, 0,
					decrypted.data(), (ULONG)decrypted.size(), &nDecryptedSize, BCRYPT_PAD_OAEP)), "decrypt response");
				decrypted.resize(nDecryptedSize);
				assertTrue(decrypted == challenge, "decrypted response equals the challenge");
			});
		}

	private:
		bool requireCredential(const CTemporaryCredential& credential)
		{
			if (credential.isWritten())
				return true;
			logInfo("skipped: could not write a Windows credential (error " + std::to_string(GetLastError()) + ")");
			return false;
		}

		std::string exportPublicKeyAsPEM(BCRYPT_KEY_HANDLE hKey)
		{
			ULONG nBlobSize = 0;
			assertTrue(BCRYPT_SUCCESS(BCryptExportKey(hKey, nullptr, BCRYPT_RSAPUBLIC_BLOB, nullptr, 0, &nBlobSize, 0)), "public key blob size");
			std::vector<uint8_t> blob(nBlobSize);
			assertTrue(BCRYPT_SUCCESS(BCryptExportKey(hKey, nullptr, BCRYPT_RSAPUBLIC_BLOB, blob.data(), nBlobSize, &nBlobSize, 0)), "export public key");

			DWORD nDERSize = 0;
			assertTrue(CryptEncodeObjectEx(X509_ASN_ENCODING, CNG_RSA_PUBLIC_KEY_BLOB, blob.data(), 0, nullptr, nullptr, &nDERSize) != FALSE, "PKCS#1 size");
			std::vector<uint8_t> der(nDERSize);
			assertTrue(CryptEncodeObjectEx(X509_ASN_ENCODING, CNG_RSA_PUBLIC_KEY_BLOB, blob.data(), 0, nullptr, der.data(), &nDERSize) != FALSE, "encode PKCS#1");

			DWORD nBase64Size = 0;
			assertTrue(CryptBinaryToStringA(der.data(), nDERSize, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCR, nullptr, &nBase64Size) != FALSE, "Base64 size");
			std::string sBase64(nBase64Size, '\0');
			assertTrue(CryptBinaryToStringA(der.data(), nDERSize, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCR, &sBase64[0], &nBase64Size) != FALSE, "encode Base64");
			sBase64.resize(nBase64Size);

			return "-----BEGIN RSA PUBLIC KEY-----\n" + sBase64 + "-----END RSA PUBLIC KEY-----\n";
		}
	};

	PUnitTestGroup createToolpathCredentialTests()
	{
		return std::make_shared<CToolpathCredentialTests>();
	}

}
