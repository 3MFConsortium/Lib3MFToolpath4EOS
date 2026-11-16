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


Abstract: Console entry point of the unit tests.
Usage: libeos_unittests [--filter TEXT] [--list] [--verbose]

*/

#include "libeos_unittest_framework.hpp"
#include "libeos_unittests.hpp"

#include <iostream>
#include <string>

using namespace LibEOSUnitTest;

int main(int argc, char* argv[])
{
	try {
		std::string sFilter;
		bool bList = false;
		bool bVerbose = false;

		for (int nIndex = 1; nIndex < argc; nIndex++) {
			std::string sArgument = argv[nIndex];
			if (sArgument == "--filter") {
				if (nIndex + 1 >= argc)
					throw std::runtime_error("--filter needs a value");
				sFilter = argv[++nIndex];
			}
			else if (sArgument == "--list") {
				bList = true;
			}
			else if (sArgument == "--verbose") {
				bVerbose = true;
			}
			else {
				throw std::runtime_error("unknown argument " + sArgument + "; usage: libeos_unittests [--filter TEXT] [--list] [--verbose]");
			}
		}

		CUnitTestFramework framework(std::cout);
		framework.registerGroup(createToolpathRecorderTests());
		framework.registerGroup(createRawTaskWriterTests());
		framework.registerGroup(createPowerRampTests());
		framework.registerGroup(createSDKVersionTests());
		framework.registerGroup(createConversionSettingsTests());
		framework.registerGroup(createConversionLogTests());
		framework.registerGroup(createToolpathLayersTests());
		framework.registerGroup(createEOSTo3MFTests());
		framework.registerGroup(createEOSTo3MFExportTests());
		framework.registerGroup(createProfileParameterTests());
		framework.registerGroup(createResourcePackageTests());
		framework.registerGroup(createWorkingDirectoryTests());
		framework.registerGroup(createNamespaceTests());
		framework.registerGroup(createToolpathCredentialTests());
		framework.registerGroup(createFutureAbortTests());

		if (bList) {
			framework.listTests();
			return 0;
		}

		std::cout << "LibEOS unit tests" << std::endl;
		auto results = framework.runTests(sFilter, bVerbose);

		std::cout << std::endl << results.m_nPassed << " passed, " << results.m_nFailed << " failed" << std::endl;
		for (auto& sName : results.m_FailedTests)
			std::cout << "  failed: " << sName << std::endl;

		if ((results.m_nPassed + results.m_nFailed) == 0) {
			std::cout << "no test matches the filter" << std::endl;
			return 2;
		}
		return (results.m_nFailed == 0) ? 0 : 1;
	}
	catch (std::exception& e) {
		std::cout << "Fatal error: " << e.what() << std::endl;
		return 3;
	}
}
