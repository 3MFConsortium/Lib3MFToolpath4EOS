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


#include "common_importstream_memory.hpp"
#include "common_utils.hpp"
#include <string>
#include <exception>

#include <string>


namespace AMCCommon {

	CImportStream_Memory::~CImportStream_Memory()
	{
	}

	bool CImportStream_Memory::seekPosition(uint64_t nPosition, bool bHasToSucceed)
	{
		if (nPosition > m_cbSize) {
			if (bHasToSucceed)
				throw std::runtime_error ("could not seek stream position");
			return false;
		}

		m_nPosition = nPosition;

		return true;
	}

	bool CImportStream_Memory::seekForward (uint64_t cbBytes, bool bHasToSucceed)
	{
		uint64_t nPosition = m_nPosition + cbBytes;

		if (nPosition > m_cbSize) {
			if (bHasToSucceed)
				throw std::runtime_error("could not forward seek stream");
			return false;
		}

		m_nPosition = nPosition;
		return true;
	}

	bool CImportStream_Memory::seekFromEnd( uint64_t cbBytes, bool bHasToSucceed)
	{
		if (cbBytes > m_cbSize) {
			if (bHasToSucceed)
				throw std::runtime_error("could not seek stream from end");
			return false;
		}

		m_nPosition = m_cbSize - cbBytes;
		return true;
	}

	uint64_t CImportStream_Memory::getPosition()
	{
		return m_nPosition;
	}

	uint64_t CImportStream_Memory::readBuffer(uint8_t * pBuffer, uint64_t cbTotalBytesToRead, bool bNeedsToReadAll)
	{
		if (m_nPosition > m_cbSize)
			throw std::runtime_error("invalid stream position");

		uint64_t cbBytesLeft = (m_cbSize - m_nPosition);
		uint64_t cbBytesToRead = cbTotalBytesToRead;

		if (cbBytesToRead > cbBytesLeft)
			cbBytesToRead = cbBytesLeft;

		if (cbBytesToRead > 0) {
			uint64_t nIndex = cbBytesToRead;
			const uint8_t * pSource = getAt(m_nPosition);
			uint8_t* pTarget = pBuffer;

			while (nIndex > 0) {
				*pTarget = *pSource;
				nIndex--;
				pSource++;
				pTarget++;
			}

			m_nPosition += cbBytesToRead;
		}

		if ((cbBytesToRead != cbTotalBytesToRead) && bNeedsToReadAll)
			throw std::runtime_error("could not read full data from stream");

		return cbBytesToRead;

	}

	uint64_t CImportStream_Memory::retrieveSize()
	{
		return m_cbSize;
	}

}
