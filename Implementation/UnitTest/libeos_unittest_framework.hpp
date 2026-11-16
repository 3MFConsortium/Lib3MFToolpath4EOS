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


Abstract: Runs unit test groups and reports the results on the console.

*/

#ifndef __LIBEOS_UNITTEST_FRAMEWORK
#define __LIBEOS_UNITTEST_FRAMEWORK

#include "libeos_unittest_group.hpp"

#include <ostream>
#include <string>
#include <vector>

namespace LibEOSUnitTest {

	struct SUnitTestResults {
		uint32_t m_nPassed = 0;
		uint32_t m_nFailed = 0;
		std::vector<std::string> m_FailedTests;
	};

	class CUnitTestFramework {
	private:
		std::vector<PUnitTestGroup> m_Groups;
		std::ostream& m_Output;

	public:
		CUnitTestFramework(std::ostream& output);

		void registerGroup(PUnitTestGroup pGroup);

		// Lists "Group.Test" names with their descriptions.
		void listTests();

		// Runs the tests whose "Group.Test" name contains the filter; an empty filter runs all.
		SUnitTestResults runTests(const std::string& sFilter, bool bVerbose);
	};

}

#endif // __LIBEOS_UNITTEST_FRAMEWORK
