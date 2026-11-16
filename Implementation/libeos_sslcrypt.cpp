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

#include "libeos_sslcrypt.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "Common/common_utils.hpp"

#include <string>
#include <vector>
#include <memory>
#include <climits>
#include <cstdio>

#include <Windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <wincred.h>


using namespace Lib3MF4EOS::Impl;

namespace {

    void failAuthentication(const std::string& sMessage)
    {
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHAUTHENTICATIONFAILED, sMessage);
    }

    void checkNTStatus(NTSTATUS status, const std::string& sOperation)
    {
        if (!BCRYPT_SUCCESS(status)) {
            char szStatus[16];
            snprintf(szStatus, sizeof(szStatus), "0x%08X", (uint32_t)status);
            failAuthentication(sOperation + " failed (NTSTATUS " + szStatus + ")");
        }
    }

    void secureEraseBuffer(std::vector<uint8_t>& buffer)
    {
        if (!buffer.empty())
            SecureZeroMemory(buffer.data(), buffer.size());
        buffer.clear();
    }

    // Holds key material and zeroes it when it goes out of scope, also when an exception is thrown.
    struct SSecureBuffer
    {
        std::vector<uint8_t> m_Data;

        ~SSecureBuffer()
        {
            secureEraseBuffer(m_Data);
        }
    };

    struct LocalFreeDeleter
    {
        void operator()(void* p) const
        {
            LocalFree(p);
        }
    };

    struct CredentialDeleter
    {
        void operator()(PCREDENTIALW pCredential) const
        {
            if ((pCredential->CredentialBlob != nullptr) && (pCredential->CredentialBlobSize > 0))
                SecureZeroMemory(pCredential->CredentialBlob, pCredential->CredentialBlobSize);
            CredFree(pCredential);
        }
    };

    // Credentials entered through the Credential Manager UI or cmdkey are stored as UTF-16.
    std::string credentialBlobToString(const BYTE* pBlob, DWORD nBlobSize)
    {
        if ((pBlob == nullptr) || (nBlobSize == 0))
            return std::string();

        bool bIsUTF16 = (nBlobSize >= 2) && ((nBlobSize % 2) == 0) && (pBlob[1] == 0);
        if (!bIsUTF16)
            return std::string((const char*)pBlob, nBlobSize);

        const wchar_t* pWide = (const wchar_t*)pBlob;
        int nWideLength = (int)(nBlobSize / 2);
        int nSize = WideCharToMultiByte(CP_UTF8, 0, pWide, nWideLength, nullptr, 0, nullptr, nullptr);
        if (nSize <= 0)
            return std::string();

        std::string sResult(nSize, '\0');
        WideCharToMultiByte(CP_UTF8, 0, pWide, nWideLength, &sResult[0], nSize, nullptr, nullptr);
        return sResult;
    }

    bool isPEMText(const std::string& sText)
    {
        return (sText.find("-----BEGIN ") != std::string::npos) && (sText.find("-----END ") != std::string::npos);
    }

    struct BCryptKeyDeleter
    {
        void operator()(void* p) const
        {
            BCryptDestroyKey((BCRYPT_KEY_HANDLE)p);
        }
    };
    using UniqueBCryptKey = std::unique_ptr<void, BCryptKeyDeleter>;

    class CRSAAlgorithmProvider
    {
    private:
        BCRYPT_ALG_HANDLE m_hAlgorithm;

    public:
        CRSAAlgorithmProvider()
            : m_hAlgorithm(nullptr)
        {
            checkNTStatus(BCryptOpenAlgorithmProvider(&m_hAlgorithm, BCRYPT_RSA_ALGORITHM, nullptr, 0), "BCryptOpenAlgorithmProvider");
        }

        ~CRSAAlgorithmProvider()
        {
            if (m_hAlgorithm != nullptr)
                BCryptCloseAlgorithmProvider(m_hAlgorithm, 0);
        }

        BCRYPT_ALG_HANDLE getHandle() const
        {
            return m_hAlgorithm;
        }
    };

    // The client key issued by EOS is an RSA public key, but a confidential one.
    // Accepts PEM with any header ("RSA PUBLIC KEY" or "PUBLIC KEY") as well as raw Base64.
    void decodePEMToDER(const std::string& sPEM, std::vector<uint8_t>& derBuffer)
    {
        if (sPEM.length() > INT_MAX)
            failAuthentication("excessive client key length");

        DWORD nDERSize = 0;
        if (!CryptStringToBinaryA(sPEM.c_str(), (DWORD)sPEM.length(), CRYPT_STRING_BASE64_ANY, nullptr, &nDERSize, nullptr, nullptr) || (nDERSize == 0))
            failAuthentication("could not decode the PEM data of the client key");

        derBuffer.resize(nDERSize);
        if (!CryptStringToBinaryA(sPEM.c_str(), (DWORD)sPEM.length(), CRYPT_STRING_BASE64_ANY, derBuffer.data(), &nDERSize, nullptr, nullptr))
            failAuthentication("could not decode the PEM data of the client key");

        derBuffer.resize(nDERSize);
    }

    // Tries X.509 SubjectPublicKeyInfo first, then PKCS#1 RSAPublicKey.
    UniqueBCryptKey importRSAPublicKey(const std::vector<uint8_t>& derBuffer)
    {
        CERT_PUBLIC_KEY_INFO* pPublicKeyInfo = nullptr;
        DWORD nPublicKeyInfoSize = 0;
        if (CryptDecodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, derBuffer.data(), (DWORD)derBuffer.size(),
            CRYPT_DECODE_ALLOC_FLAG, nullptr, &pPublicKeyInfo, &nPublicKeyInfoSize)) {

            std::unique_ptr<CERT_PUBLIC_KEY_INFO, LocalFreeDeleter> pPublicKeyInfoHolder(pPublicKeyInfo);

            BCRYPT_KEY_HANDLE hKey = nullptr;
            if (!CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING, pPublicKeyInfo, 0, nullptr, &hKey))
                failAuthentication("could not import the client key");

            return UniqueBCryptKey(hKey);
        }

        BCRYPT_RSAKEY_BLOB* pRSAKeyBlob = nullptr;
        DWORD nRSAKeyBlobSize = 0;
        if (!CryptDecodeObjectEx(X509_ASN_ENCODING, CNG_RSA_PUBLIC_KEY_BLOB, derBuffer.data(), (DWORD)derBuffer.size(),
            CRYPT_DECODE_ALLOC_FLAG, nullptr, &pRSAKeyBlob, &nRSAKeyBlobSize))
            failAuthentication("the client key is neither an X.509 SubjectPublicKeyInfo nor a PKCS#1 RSAPublicKey");

        std::unique_ptr<BCRYPT_RSAKEY_BLOB, LocalFreeDeleter> pRSAKeyBlobHolder(pRSAKeyBlob);

        CRSAAlgorithmProvider algorithmProvider;
        BCRYPT_KEY_HANDLE hKey = nullptr;
        checkNTStatus(BCryptImportKeyPair(algorithmProvider.getHandle(), nullptr, BCRYPT_RSAPUBLIC_BLOB, &hKey,
            (PUCHAR)pRSAKeyBlob, nRSAKeyBlobSize, 0), "BCryptImportKeyPair");

        return UniqueBCryptKey(hKey);
    }

    // RSA-OAEP with SHA-1 for both hash and MGF1 and an empty label.
    void encryptOAEP(BCRYPT_KEY_HANDLE hKey, const uint8_t* pInBuffer, size_t nInBufferSize, std::vector<uint8_t>& outBuffer)
    {
        if (nInBufferSize > ULONG_MAX)
            failAuthentication("excessive challenge length");

        BCRYPT_OAEP_PADDING_INFO paddingInfo = {};
        paddingInfo.pszAlgId = BCRYPT_SHA1_ALGORITHM;
        paddingInfo.pbLabel = nullptr;
        paddingInfo.cbLabel = 0;

        ULONG nEncryptedSize = 0;
        checkNTStatus(BCryptEncrypt(hKey, (PUCHAR)pInBuffer, (ULONG)nInBufferSize, &paddingInfo, nullptr, 0,
            nullptr, 0, &nEncryptedSize, BCRYPT_PAD_OAEP), "BCryptEncrypt");

        if (nEncryptedSize == 0)
            failAuthentication("encryption resulted in zero-length output");
        if (nEncryptedSize > 1024 * 1024)
            failAuthentication("excessive encrypted length");

        outBuffer.resize(nEncryptedSize);
        checkNTStatus(BCryptEncrypt(hKey, (PUCHAR)pInBuffer, (ULONG)nInBufferSize, &paddingInfo, nullptr, 0,
            outBuffer.data(), (ULONG)outBuffer.size(), &nEncryptedSize, BCRYPT_PAD_OAEP), "BCryptEncrypt");

        outBuffer.resize(nEncryptedSize);
    }

}

CSSLRSACryptoHelper::CSSLRSACryptoHelper(const std::string& sKey)
    : m_sKey (sKey)
{

}

CSSLRSACryptoHelper::~CSSLRSACryptoHelper()
{
    secureErase(m_sKey);
    secureEraseBuffer(m_ResponseBuffer);
}

bool CSSLRSACryptoHelper::encryptChallenge(const uint8_t* pChallengeBuffer, size_t nChallengeSize)
{
    if (pChallengeBuffer == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "invalid challenge buffer");
    if (nChallengeSize == 0)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "invalid challenge buffer");

    secureEraseBuffer(m_ResponseBuffer);

    SSecureBuffer derBuffer;
    decodePEMToDER(m_sKey, derBuffer.m_Data);
    auto pKey = importRSAPublicKey(derBuffer.m_Data);

    encryptOAEP((BCRYPT_KEY_HANDLE)pKey.get(), pChallengeBuffer, nChallengeSize, m_ResponseBuffer);

    return !m_ResponseBuffer.empty();

}

std::vector<uint8_t>& CSSLRSACryptoHelper::getResponseBuffer()
{
    return m_ResponseBuffer;
}

void Lib3MF4EOS::Impl::secureErase(std::string& sSecret)
{
    if (!sSecret.empty())
        SecureZeroMemory(&sSecret[0], sSecret.size());
    sSecret.clear();
}

std::string Lib3MF4EOS::Impl::readToolpathClientKeyFromCredentialStore(const std::string& sCredentialName)
{
    if (sCredentialName.empty())
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "empty credential name");

    std::wstring sTargetName = AMCCommon::CUtils::UTF8toUTF16(sCredentialName);

    PCREDENTIALW pRawCredential = nullptr;
    if (!CredReadW(sTargetName.c_str(), CRED_TYPE_GENERIC, 0, &pRawCredential)) {
        DWORD nError = GetLastError();
        if (nError == ERROR_NOT_FOUND)
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHCREDENTIALNOTFOUND, "no Windows credential named \"" + sCredentialName + "\"");
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHCREDENTIALINVALID,
            "could not read Windows credential \"" + sCredentialName + "\" (error " + std::to_string(nError) + ")");
    }
    std::unique_ptr<CREDENTIALW, CredentialDeleter> pCredential(pRawCredential);

    std::string sClientKey = credentialBlobToString(pCredential->CredentialBlob, pCredential->CredentialBlobSize);
    if (!isPEMText(sClientKey)) {
        secureErase(sClientKey);
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHCREDENTIALINVALID,
            "Windows credential \"" + sCredentialName + "\" does not hold the PEM text of a client key");
    }

    return sClientKey;
}
