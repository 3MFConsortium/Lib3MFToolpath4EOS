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


#ifndef __AMCCOMMON_IMPORTSTREAM_MEMORY
#define __AMCCOMMON_IMPORTSTREAM_MEMORY

#include "common_importstream.hpp"
#include <string>
#include <fstream>

#define NMR_IMPORTSTREAM_READCHUNKSIZE (1024*1024)
#define NMR_IMPORTSTREAM_MAXMEMSTREAMSIZE (1024ULL*1024ULL*1024ULL*1024ULL)


namespace AMCCommon {


	class CImportStream_Memory : public CImportStream {
	protected:
		uint64_t m_cbSize;
		uint64_t m_nPosition;
		
		virtual const uint8_t * getAt(uint64_t nPosition) = 0;
	public:
		virtual ~CImportStream_Memory();

		virtual bool seekPosition(uint64_t nPosition, bool bHasToSucceed);
		virtual bool seekForward(uint64_t cbBytes, bool bHasToSucceed);
		virtual bool seekFromEnd( uint64_t cbBytes, bool bHasToSucceed);
		virtual uint64_t getPosition();
		virtual uint64_t readBuffer(uint8_t * pBuffer, uint64_t cbTotalBytesToRead, bool bNeedsToReadAll) override;
		virtual uint64_t retrieveSize();
		virtual PImportStream copyToMemory(bool bFromCurrentPosition) = 0;
	};

}

#endif // __AMCCOMMON_IMPORTSTREAM_MEMORY
