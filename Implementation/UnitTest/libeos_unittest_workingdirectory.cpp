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

Abstract: Tests of the process-wide current directory override around EOS SDK calls.

*/

#include "libeos_unittests.hpp"
#include "libeos_workingdirectory.hpp"
#include "Common/common_utils.hpp"

#include <Windows.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

	class CWorkingDirectoryTests : public CUnitTestGroup {
	private:

		static std::string normalized(std::wstring sPath)
		{
			wchar_t fullPath[MAX_PATH * 4] = {};
			if (GetFullPathNameW(sPath.c_str(), MAX_PATH * 4, fullPath, nullptr) > 0)
				sPath = fullPath;
			while ((sPath.size() > 1) && ((sPath.back() == L'\\') || (sPath.back() == L'/')))
				sPath.pop_back();
			for (auto& c : sPath)
				c = towlower(c);
			return AMCCommon::CUtils::UTF16toUTF8(sPath);
		}

		static std::string currentDirectory()
		{
			wchar_t buffer[MAX_PATH * 4] = {};
			GetCurrentDirectoryW(MAX_PATH * 4, buffer);
			return normalized(buffer);
		}

		static std::string dllDirectory()
		{
			wchar_t buffer[MAX_PATH * 4] = {};
			if (GetDllDirectoryW(MAX_PATH * 4, buffer) == 0)
				return "";
			return normalized(buffer);
		}

		static std::string firstDirectory()
		{
			return AMCCommon::CUtils::getTempFolder();
		}

		static std::string secondDirectory()
		{
			wchar_t buffer[MAX_PATH] = {};
			GetSystemWindowsDirectoryW(buffer, MAX_PATH);
			return AMCCommon::CUtils::UTF16toUTF8(buffer);
		}

		static std::string normalizedUTF8(const std::string& sPath)
		{
			return normalized(AMCCommon::CUtils::UTF8toUTF16(sPath));
		}

	public:
		std::string getName() override
		{
			return "WorkingDirectory";
		}

		void registerTests() override
		{
			registerTest("RestoresAfterScope", "the current and DLL directory are set during the scope and restored afterwards", [this]() {
				std::string sOriginal = currentDirectory();
				std::string sOriginalDLLDirectory = dllDirectory();
				{
					CCurrentDirectoryOverride directoryOverride(firstDirectory());
					assertEqualString(currentDirectory(), normalizedUTF8(firstDirectory()), "current directory during the scope");
					assertEqualString(dllDirectory(), normalizedUTF8(firstDirectory()), "DLL directory during the scope");
				}
				assertEqualString(currentDirectory(), sOriginal, "current directory after the scope");
				assertEqualString(dllDirectory(), sOriginalDLLDirectory, "DLL directory after the scope");
			});

			registerTest("Nested", "a nested override of the same directory keeps it until the outer one ends", [this]() {
				std::string sOriginal = currentDirectory();
				{
					CCurrentDirectoryOverride outerOverride(firstDirectory());
					{
						CCurrentDirectoryOverride innerOverride(firstDirectory() + "\\");
					}
					assertEqualString(currentDirectory(), normalizedUTF8(firstDirectory()), "after the inner scope");
				}
				assertEqualString(currentDirectory(), sOriginal, "after the outer scope");
			});

			registerTest("OutOfOrderRelease", "overrides released in a different order than created restore the original directory", [this]() {
				std::string sOriginal = currentDirectory();
				auto pFirst = std::make_unique<CCurrentDirectoryOverride>(firstDirectory());
				auto pSecond = std::make_unique<CCurrentDirectoryOverride>(firstDirectory());
				pFirst.reset();
				assertEqualString(currentDirectory(), normalizedUTF8(firstDirectory()), "while the second override is active");
				pSecond.reset();
				assertEqualString(currentDirectory(), sOriginal, "after both overrides");
			});

			registerTest("ConcurrentThreads", "overrides on several threads at the same time leave the original directory behind", [this]() {
				std::string sOriginal = currentDirectory();
				std::string sDirectory = firstDirectory();
				std::atomic<uint32_t> nWrongDirectories(0);
				std::vector<std::thread> threads;
				for (uint32_t nThread = 0; nThread < 8; nThread++) {
					threads.emplace_back([&]() {
						for (uint32_t nIteration = 0; nIteration < 200; nIteration++) {
							CCurrentDirectoryOverride directoryOverride(sDirectory);
							if (currentDirectory() != normalizedUTF8(sDirectory))
								nWrongDirectories++;
						}
					});
				}
				for (auto& thread : threads)
					thread.join();

				assertEqual(nWrongDirectories.load(), 0, "scopes that saw a different directory");
				assertEqualString(currentDirectory(), sOriginal, "after all threads");
			});

			registerTest("ConflictingDirectory", "a second directory is refused while another one is active", [this]() {
				std::string sOriginal = currentDirectory();
				{
					CCurrentDirectoryOverride directoryOverride(firstDirectory());
					bool bThrown = false;
					try {
						CCurrentDirectoryOverride conflictingOverride(secondDirectory());
					}
					catch (std::runtime_error& e) {
						bThrown = true;
						assertContains(e.what(), "conflicts", "message");
					}
					assertTrue(bThrown, "conflicting override refused");
					assertEqualString(currentDirectory(), normalizedUTF8(firstDirectory()), "active override unchanged");
				}
				{
					CCurrentDirectoryOverride directoryOverride(secondDirectory());
					assertEqualString(currentDirectory(), normalizedUTF8(secondDirectory()), "second directory once the first is released");
				}
				assertEqualString(currentDirectory(), sOriginal, "after both");
			});
		}
	};

	PUnitTestGroup createWorkingDirectoryTests()
	{
		return std::make_shared<CWorkingDirectoryTests>();
	}

}
