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

	CImportStream_Shared_Memory::CImportStream_Shared_Memory(_In_ const uint8_t * pBuffer, _In_ uint64_t cbBytes)
	{
		if (pBuffer == nullptr)
			throw std::runtime_error("invalid buffer pointer");

		if (cbBytes > NMR_IMPORTSTREAM_MAXMEMSTREAMSIZE)
			throw std::runtime_error("invalid buffer size");

		m_cbSize = cbBytes;
		m_nPosition = 0;

		m_Buffer = pBuffer;
	}	

	PImportStream CImportStream_Shared_Memory::copyToMemory(bool bFromCurrentPosition)
	{
		if (!bFromCurrentPosition)
			seekPosition(0, true);

		if (m_nPosition > m_cbSize)
			throw std::runtime_error("invalid stream position");

		return std::make_shared<CImportStream_Unique_Memory>(this, m_cbSize - m_nPosition, true);
	}

	const uint8_t * CImportStream_Shared_Memory::getAt(uint64_t nPosition) { 
		if (m_nPosition >= m_cbSize)
			throw std::runtime_error("invalid stream position");

		return &m_Buffer[nPosition]; 
	}

}