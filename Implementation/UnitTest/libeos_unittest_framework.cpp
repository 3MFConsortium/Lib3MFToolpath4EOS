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

*/

#include "libeos_unittest_framework.hpp"

#include <chrono>

namespace LibEOSUnitTest {

CUnitTestFramework::CUnitTestFramework(std::ostream& output)
	: m_Output(output)
{
}

void CUnitTestFramework::registerGroup(PUnitTestGroup pGroup)
{
	if (pGroup.get() == nullptr)
		throw std::runtime_error("unit test group is null");
	for (auto& pExisting : m_Groups) {
		if (pExisting->getName() == pGroup->getName())
			throw std::runtime_error("duplicate unit test group " + pGroup->getName());
	}
	pGroup->registerTests();
	m_Groups.push_back(pGroup);
}

void CUnitTestFramework::listTests()
{
	for (auto& pGroup : m_Groups) {
		for (auto& test : pGroup->getTests())
			m_Output << pGroup->getName() << "." << test.m_sName << "  " << test.m_sDescription << std::endl;
	}
}

SUnitTestResults CUnitTestFramework::runTests(const std::string& sFilter, bool bVerbose)
{
	SUnitTestResults results;

	for (auto& pGroup : m_Groups) {
		std::string sGroupName = pGroup->getName();
		pGroup->setInfoLogger([this, bVerbose](const std::string& sMessage) {
			if (bVerbose)
				m_Output << "      " << sMessage << std::endl;
		});

		bool bGroupHeaderWritten = false;
		for (auto& test : pGroup->getTests()) {
			std::string sFullName = sGroupName + "." + test.m_sName;
			if (!sFilter.empty() && (sFullName.find(sFilter) == std::string::npos))
				continue;

			if (!bGroupHeaderWritten) {
				m_Output << sGroupName << std::endl;
				bGroupHeaderWritten = true;
			}

			auto startTime = std::chrono::steady_clock::now();
			std::string sFailure;
			try {
				test.m_Callback();
			}
			catch (std::exception& e) {
				sFailure = e.what();
				if (sFailure.empty())
					sFailure = "exception without message";
			}
			catch (...) {
				sFailure = "unknown exception";
			}
			auto nMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();

			if (sFailure.empty()) {
				results.m_nPassed++;
				m_Output << "  ok      " << test.m_sName << " (" << nMilliseconds << " ms)" << std::endl;
			}
			else {
				results.m_nFailed++;
				results.m_FailedTests.push_back(sFullName);
				m_Output << "  FAILED  " << test.m_sName << ": " << sFailure << std::endl;
			}
		}
	}

	return results;
}

}
