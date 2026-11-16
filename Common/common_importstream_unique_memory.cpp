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


#include "common_importstream_shared_memory.hpp"
#include "common_importstream_unique_memory.hpp"
#include "common_utils.hpp"
#include <string>
#include <exception>

#include <string>


namespace AMCCommon {

	CImportStream_Unique_Memory::CImportStream_Unique_Memory()
	{
		m_cbSize = 0;
		m_nPosition = 0;
	}


	CImportStream_Unique_Memory::CImportStream_Unique_Memory(CImportStream * pStream, uint64_t cbBytesToCopy, bool bNeedsToCopyAllBytes)
	{
		if (pStream == nullptr)
			throw std::runtime_error("invalid stream parameter");

		if (cbBytesToCopy > NMR_IMPORTSTREAM_MAXMEMSTREAMSIZE)
			throw std::runtime_error("invalid bytes to copy");


		// Retrieve Capacity and allocate buffer.
		uint64_t cbCapacity = cbBytesToCopy;
		try {
			m_Buffer.resize((size_t)cbCapacity);
		}
		catch (std::bad_alloc&) {
			throw std::runtime_error("could not allocate memory");
		}

		m_cbSize = 0;
		m_nPosition = 0;

		while (m_cbSize < cbCapacity) {

			// Only read in chunks
			uint64_t cbBytesToRead = cbCapacity - m_cbSize;
			if (cbBytesToRead > NMR_IMPORTSTREAM_READCHUNKSIZE)
				cbBytesToRead = NMR_IMPORTSTREAM_READCHUNKSIZE;

			// Read bytes into memory
			uint64_t cbBytesRead;

			cbBytesRead = pStream->readBuffer(&m_Buffer[(size_t) m_cbSize], cbBytesToRead, false);

			// increase size
			m_cbSize += cbBytesRead;

			if (cbBytesRead != cbBytesToRead)
				break;
		}

		if ((m_cbSize != cbCapacity) && (bNeedsToCopyAllBytes))
			throw std::runtime_error("could not read full data");


	}

	CImportStream_Unique_Memory::CImportStream_Unique_Memory(const uint8_t * pBuffer, uint64_t cbBytes)
	{
		if (pBuffer == nullptr)
			std::runtime_error("invalid buffer parameter");

		if (cbBytes > NMR_IMPORTSTREAM_MAXMEMSTREAMSIZE)
			std::runtime_error("invalid buffer size");

		// Retrieve Capacity and allocate buffer.
		m_Buffer.resize((size_t) cbBytes);

		m_cbSize = cbBytes;
		m_nPosition = 0;

		const uint8_t * pSource = pBuffer;
		uint8_t* pTarget = &m_Buffer[0];

		uint64_t nIndex = cbBytes;
		while (nIndex > 0) {
			*pTarget = *pSource;

			pSource++;
			pTarget++;
			nIndex--;
		}
	}	

	PImportStream CImportStream_Unique_Memory::copyToMemory(bool bFromCurrentPosition)
	{
		if (!bFromCurrentPosition)
			seekPosition(0, true);

		if (m_nPosition > m_cbSize)
			throw std::runtime_error("invalid stream position");

		return std::make_shared<CImportStream_Unique_Memory>(this, m_cbSize - m_nPosition, true);
	}

	const uint8_t * CImportStream_Unique_Memory::getAt(uint64_t nPosition) { 

		if (nPosition >= (uint64_t)m_Buffer.size())
			throw std::runtime_error("invalid stream position");

		return &m_Buffer[(size_t) nPosition]; 
	}

}