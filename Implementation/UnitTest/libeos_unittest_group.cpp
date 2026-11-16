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

#include "libeos_unittest_group.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

#include <cmath>
#include <sstream>

namespace LibEOSUnitTest {

EUnitTestFailed::EUnitTestFailed(const std::string& sMessage)
	: std::runtime_error(sMessage)
{
}

CUnitTestGroup::CUnitTestGroup()
{
}

CUnitTestGroup::~CUnitTestGroup()
{
}

std::string CUnitTestGroup::withContext(const std::string& sMessage, const std::string& sContext)
{
	return sContext.empty() ? sMessage : (sContext + ": " + sMessage);
}

void CUnitTestGroup::registerTest(const std::string& sName, const std::string& sDescription, std::function<void()> callback)
{
	for (auto& test : m_Tests) {
		if (test.m_sName == sName)
			throw std::runtime_error("duplicate unit test name " + getName() + "." + sName);
	}
	m_Tests.push_back(SUnitTest{ sName, sDescription, callback });
}

void CUnitTestGroup::logInfo(const std::string& sMessage)
{
	if (m_InfoLogger)
		m_InfoLogger(sMessage);
}

void CUnitTestGroup::fail(const std::string& sMessage)
{
	throw EUnitTestFailed(sMessage);
}

void CUnitTestGroup::assertTrue(bool bValue, const std::string& sContext)
{
	if (!bValue)
		fail(withContext("expected true", sContext));
}

void CUnitTestGroup::assertFalse(bool bValue, const std::string& sContext)
{
	if (bValue)
		fail(withContext("expected false", sContext));
}

void CUnitTestGroup::assertEqual(int64_t nActual, int64_t nExpected, const std::string& sContext)
{
	if (nActual != nExpected)
		fail(withContext("expected " + std::to_string(nExpected) + ", got " + std::to_string(nActual), sContext));
}

void CUnitTestGroup::assertNear(double dActual, double dExpected, double dTolerance, const std::string& sContext)
{
	if (!(std::fabs(dActual - dExpected) <= dTolerance)) {
		std::ostringstream stream;
		stream.precision(12);
		stream << "expected " << dExpected << " within " << dTolerance << ", got " << dActual;
		fail(withContext(stream.str(), sContext));
	}
}

void CUnitTestGroup::assertEqualString(const std::string& sActual, const std::string& sExpected, const std::string& sContext)
{
	if (sActual != sExpected)
		fail(withContext("expected \"" + sExpected + "\", got \"" + sActual + "\"", sContext));
}

void CUnitTestGroup::assertContains(const std::string& sText, const std::string& sPart, const std::string& sContext)
{
	if (sText.find(sPart) == std::string::npos)
		fail(withContext("expected \"" + sPart + "\" in \"" + sText + "\"", sContext));
}

std::string CUnitTestGroup::assertThrowsError(std::function<void()> callback, int32_t nExpectedError, const std::string& sContext)
{
	try {
		callback();
	}
	catch (ELib3MF4EOSInterfaceException& e) {
		if (e.getErrorCode() != nExpectedError)
			fail(withContext("expected LibEOS error " + std::to_string(nExpectedError) + ", got error " + std::to_string(e.getErrorCode()) + " (" + e.what() + ")", sContext));
		return e.what();
	}
	catch (EUnitTestFailed&) {
		throw;
	}
	catch (std::exception& e) {
		fail(withContext("expected LibEOS error " + std::to_string(nExpectedError) + ", got another exception (" + e.what() + ")", sContext));
	}
	fail(withContext("expected LibEOS error " + std::to_string(nExpectedError) + ", but nothing was thrown", sContext));
	return "";
}

const std::vector<SUnitTest>& CUnitTestGroup::getTests()
{
	return m_Tests;
}

void CUnitTestGroup::setInfoLogger(std::function<void(const std::string&)> logger)
{
	m_InfoLogger = logger;
}

}
