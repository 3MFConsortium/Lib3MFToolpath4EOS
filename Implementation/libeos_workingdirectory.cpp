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

#include "libeos_workingdirectory.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "Common/common_utils.hpp"

#include <Windows.h>

#include <mutex>

using namespace Lib3MF4EOS::Impl;

namespace {

	struct sDirectoryOverrideState {
		std::mutex m_Mutex;
		uint32_t m_nActiveOverrides = 0;
		std::wstring m_sOverrideDirectory;
		std::wstring m_sPreviousCurrentDirectory;
		std::wstring m_sPreviousDLLDirectory;
	};

	sDirectoryOverrideState& directoryOverrideState()
	{
		static sDirectoryOverrideState state;
		return state;
	}

	std::wstring normalizedDirectory(const std::wstring& sPath)
	{
		std::wstring sResult = sPath;
		DWORD nLength = GetFullPathNameW(sPath.c_str(), 0, nullptr, nullptr);
		if (nLength > 0) {
			sResult.resize((size_t)nLength);
			nLength = GetFullPathNameW(sPath.c_str(), nLength, &sResult[0], nullptr);
			sResult.resize(nLength);
		}
		while ((sResult.size() > 1) && ((sResult.back() == L'\\') || (sResult.back() == L'/')))
			sResult.pop_back();
		return sResult;
	}

	bool isSameDirectory(const std::wstring& sPath1, const std::wstring& sPath2)
	{
		std::wstring sNormalized1 = normalizedDirectory(sPath1);
		std::wstring sNormalized2 = normalizedDirectory(sPath2);
		return CompareStringOrdinal(sNormalized1.c_str(), (int)sNormalized1.size(), sNormalized2.c_str(), (int)sNormalized2.size(), TRUE) == CSTR_EQUAL;
	}

	std::wstring getCurrentDirectory()
	{
		DWORD nLength = GetCurrentDirectoryW(0, nullptr);
		if (nLength == 0)
			throw std::runtime_error("could not retrieve current directory: " + std::to_string(GetLastError()));
		std::wstring sDirectory((size_t)nLength, L'\0');
		nLength = GetCurrentDirectoryW(nLength, &sDirectory[0]);
		if (nLength == 0)
			throw std::runtime_error("could not retrieve current directory: " + std::to_string(GetLastError()));
		sDirectory.resize(nLength);
		return sDirectory;
	}

	std::wstring getDLLDirectory()
	{
		DWORD nLength = GetDllDirectoryW(0, nullptr);
		if (nLength == 0)
			return std::wstring();
		std::wstring sDirectory((size_t)nLength, L'\0');
		nLength = GetDllDirectoryW(nLength, &sDirectory[0]);
		sDirectory.resize(nLength);
		return sDirectory;
	}

	// GetDllDirectoryW cannot tell an unset DLL directory from an empty one, so both are restored as empty,
	// which removes the current directory from the DLL search order.
	void restoreDirectories(const sDirectoryOverrideState& state)
	{
		SetCurrentDirectoryW(state.m_sPreviousCurrentDirectory.c_str());
		SetDllDirectoryW(state.m_sPreviousDLLDirectory.c_str());
	}

}

CCurrentDirectoryOverride::CCurrentDirectoryOverride (const std::string& newWorkingDirectory)
{
	if (newWorkingDirectory.empty())
		throw std::runtime_error("invalid DLL working directory");

	std::wstring sNewDirectory = AMCCommon::CUtils::UTF8toUTF16(newWorkingDirectory);

	auto& state = directoryOverrideState();
	std::lock_guard<std::mutex> lock(state.m_Mutex);

	if (state.m_nActiveOverrides > 0) {
		if (!isSameDirectory(state.m_sOverrideDirectory, sNewDirectory))
			throw std::runtime_error("the EOS SDK directory " + newWorkingDirectory + " conflicts with the SDK directory that is already in use: "
				+ AMCCommon::CUtils::UTF16toUTF8(state.m_sOverrideDirectory));
		state.m_nActiveOverrides++;
		return;
	}

	state.m_sPreviousCurrentDirectory = getCurrentDirectory();
	state.m_sPreviousDLLDirectory = getDLLDirectory();

	if (!SetCurrentDirectoryW(sNewDirectory.c_str()))
		throw std::runtime_error("could not change the current directory to " + newWorkingDirectory + ": " + std::to_string(GetLastError()));

	if (!SetDllDirectoryW(sNewDirectory.c_str())) {
		DWORD nError = GetLastError();
		restoreDirectories(state);
		throw std::runtime_error("could not set the DLL directory to " + newWorkingDirectory + ": " + std::to_string(nError));
	}

	state.m_sOverrideDirectory = sNewDirectory;
	state.m_nActiveOverrides = 1;
}

CCurrentDirectoryOverride::~CCurrentDirectoryOverride ()
{
	auto& state = directoryOverrideState();
	std::lock_guard<std::mutex> lock(state.m_Mutex);

	if (state.m_nActiveOverrides == 0)
		return;

	state.m_nActiveOverrides--;
	if (state.m_nActiveOverrides == 0) {
		restoreDirectories(state);
		state.m_sOverrideDirectory.clear();
	}
}