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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
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

#include "common_workingdirectory.hpp"
#include "common_utils.hpp"
#include "common_exportstream_native.hpp"

#include <stdexcept>

#if defined _WIN32
#include <Windows.h>
#endif

namespace AMCCommon {



	CWorkingDirectory::CWorkingDirectory(const std::string& sPrefix)
	{
		std::string sBasePath = AMCCommon::CUtils::getTempFolder ();
		if (sBasePath.empty())
			throw std::runtime_error("empty temporary file base path");

		m_sPath = AMCCommon::CUtils::findTemporaryFileName(sBasePath, sPrefix, "", 64);
		if (m_sPath.empty())
			throw std::runtime_error("could not create working directory");

		AMCCommon::CUtils::createDirectoryOnDisk(m_sPath);
	}

	CWorkingDirectory::~CWorkingDirectory()
	{
		// TODO: verzeichnis aufrùumen
	}

	std::string CWorkingDirectory::getPath()
	{
		return m_sPath;
	}

	std::wstring CWorkingDirectory::getPathW()
	{
		return AMCCommon::CUtils::UTF8toUTF16(m_sPath);
	}


	void CWorkingDirectory::storeData(const std::string& sName, const std::vector<uint8_t> buffer)
	{
		AMCCommon::CExportStream_Native exportStream(m_sPath + "/" + sName);
		if (buffer.size() > 0)
			exportStream.writeBuffer(buffer.data (), buffer.size ());
	}

	std::vector<std::string> CWorkingDirectory::findFilesInDirectory(const std::string& sFilter, bool bReturnFullPaths)
	{
		std::vector<std::string> resultNames;

		std::string sQuery = m_sPath + "/" + sFilter;
		std::wstring sQueryW = AMCCommon::CUtils::UTF8toUTF16(sQuery);
		WIN32_FIND_DATAW ffd;
		HANDLE hFind = FindFirstFileW(sQueryW.c_str(), &ffd);

		if (INVALID_HANDLE_VALUE == hFind) {
			DWORD dwFindError = GetLastError();
			if ((dwFindError == ERROR_FILE_NOT_FOUND) || (dwFindError == ERROR_NO_MORE_FILES))
				return resultNames;
			throw std::runtime_error("could not find files in directory");
		}

		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::string sFileName = AMCCommon::CUtils::UTF16toUTF8(ffd.cFileName);
				if (bReturnFullPaths) {
					resultNames.push_back(m_sPath + "/" + sFileName);
				}
				else {
					resultNames.push_back(sFileName);
				}
			}

		} while (FindNextFileW(hFind, &ffd) != 0);

		DWORD dwError = GetLastError();
		FindClose(hFind);

		if (dwError != ERROR_NO_MORE_FILES)
		{
			throw std::runtime_error("could not find files in directory");
		}

		return resultNames;
	}

}

