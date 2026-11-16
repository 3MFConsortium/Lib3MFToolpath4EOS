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


Abstract: A group of unit tests. A test is a callback that fails by throwing.

*/

#ifndef __LIBEOS_UNITTEST_GROUP
#define __LIBEOS_UNITTEST_GROUP

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace LibEOSUnitTest {

	class EUnitTestFailed : public std::runtime_error {
	public:
		EUnitTestFailed(const std::string& sMessage);
	};

	struct SUnitTest {
		std::string m_sName;
		std::string m_sDescription;
		std::function<void()> m_Callback;
	};

	class CUnitTestGroup {
	private:
		std::vector<SUnitTest> m_Tests;
		std::function<void(const std::string&)> m_InfoLogger;

		static std::string withContext(const std::string& sMessage, const std::string& sContext);

	protected:
		void registerTest(const std::string& sName, const std::string& sDescription, std::function<void()> callback);

		void logInfo(const std::string& sMessage);

		void fail(const std::string& sMessage);
		void assertTrue(bool bValue, const std::string& sContext = "");
		void assertFalse(bool bValue, const std::string& sContext = "");
		void assertEqual(int64_t nActual, int64_t nExpected, const std::string& sContext = "");
		void assertNear(double dActual, double dExpected, double dTolerance, const std::string& sContext = "");
		void assertEqualString(const std::string& sActual, const std::string& sExpected, const std::string& sContext = "");
		void assertContains(const std::string& sText, const std::string& sPart, const std::string& sContext = "");

		// Fails unless the callback throws an ELib3MF4EOSInterfaceException with the given error code.
		// Returns the exception message.
		std::string assertThrowsError(std::function<void()> callback, int32_t nExpectedError, const std::string& sContext = "");

	public:
		CUnitTestGroup();
		virtual ~CUnitTestGroup();

		virtual std::string getName() = 0;
		virtual void registerTests() = 0;

		const std::vector<SUnitTest>& getTests();
		void setInfoLogger(std::function<void(const std::string&)> logger);
	};

	typedef std::shared_ptr<CUnitTestGroup> PUnitTestGroup;

}

#endif // __LIBEOS_UNITTEST_GROUP
