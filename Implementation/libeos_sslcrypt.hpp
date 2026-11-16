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


Abstract: This is the class declaration of CEOSContext

*/


#ifndef __LIBEOS_SSLCRYPT
#define __LIBEOS_SSLCRYPT

#include <vector>
#include <string>

namespace Lib3MF4EOS {
    namespace Impl {

        class CSSLRSACryptoHelper {

        private:

            std::vector<uint8_t> m_ResponseBuffer;
            std::string m_sKey;
                

        public:

            CSSLRSACryptoHelper(const std::string & sKey);

            virtual ~CSSLRSACryptoHelper();

            bool encryptChallenge(const uint8_t * pChallengeBuffer, size_t nChallengeSize);

            std::vector<uint8_t> & getResponseBuffer ();

        };

        // Overwrites the characters with zeros before clearing, so that key material does not linger in freed memory.
        void secureErase(std::string& sSecret);

        // Returns the PEM text of the Toolpath API client key stored as generic credential sCredentialName in the
        // Windows Credential Manager of the current user. The caller erases it with secureErase after use.
        std::string readToolpathClientKeyFromCredentialStore(const std::string& sCredentialName);

    }

}

#endif //__LIBEOS_SSLCRYPT
