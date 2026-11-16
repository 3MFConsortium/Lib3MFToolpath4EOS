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


Abstract: This is a stub class definition of CEOSRawTaskLayer

*/

#include "libeos_eosrawtasklayer.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.


using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CEOSRawTaskLayer 
**************************************************************************************************************************/

CEOSRawTaskLayer::CEOSRawTaskLayer(int64_t zHeightInMicrometers)
	: m_zHeightInMicrometers(zHeightInMicrometers)
{

}

CEOSRawTaskLayer::~CEOSRawTaskLayer()
{

}


Lib3MF4EOS_int64 CEOSRawTaskLayer::GetZHeight()
{
	return m_zHeightInMicrometers;
}

void CEOSRawTaskLayer::GetPointData(Lib3MF4EOS_uint64 nPointsBufferSize, Lib3MF4EOS_uint64* pPointsNeededCount, Lib3MF4EOS::sEOSTaskPoint * pPointsBuffer)
{
	if (pPointsNeededCount != nullptr)
		*pPointsNeededCount = m_Points.size();

	if (pPointsBuffer != nullptr) {
		if (nPointsBufferSize < m_Points.size())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUFFERTOOSMALL);

		Lib3MF4EOS::sEOSTaskPoint* pCurrentTarget = pPointsBuffer;
		for (auto& point : m_Points) {
			*pCurrentTarget = point;
			pCurrentTarget++;
		}
	}
}

void CEOSRawTaskLayer::GetBlockData(Lib3MF4EOS_uint64 nBlocksBufferSize, Lib3MF4EOS_uint64* pBlocksNeededCount, Lib3MF4EOS::sEOSTaskBlock * pBlocksBuffer)
{
	if (pBlocksNeededCount != nullptr)
		*pBlocksNeededCount = m_Blocks.size();

	if (pBlocksBuffer != nullptr) {
		if (nBlocksBufferSize < m_Blocks.size())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUFFERTOOSMALL);

		Lib3MF4EOS::sEOSTaskBlock* pCurrentTarget = pBlocksBuffer;
		for (auto& block : m_Blocks) {
			*pCurrentTarget = block;
			pCurrentTarget++;
		}
	}
}

void CEOSRawTaskLayer::ClearData()
{
	m_Points.clear();
	m_Blocks.clear();
}

void CEOSRawTaskLayer::initializeData(const std::deque<Lib3MF4EOS::sEOSTaskPoint>& newPoints, const std::deque<Lib3MF4EOS::sEOSTaskBlock>& newBlocks)
{
	m_Points = newPoints;	
	m_Blocks = newBlocks;
}


void CEOSRawTaskLayer::AddData(const Lib3MF4EOS_uint64 nPointsBufferSize, const Lib3MF4EOS::sEOSTaskPoint* pPointsBuffer, const Lib3MF4EOS_uint64 nBlocksBufferSize, const Lib3MF4EOS::sEOSTaskBlock* pBlocksBuffer)
{

	if (nPointsBufferSize > 0) {

		if (pPointsBuffer == nullptr)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

		uint64_t nCurrentPointCount = m_Points.size();
		
		for (Lib3MF4EOS_uint64 nIndex = 0; nIndex < nPointsBufferSize; nIndex++) {
			m_Points.push_back (pPointsBuffer[nIndex]);
		}

		if (nBlocksBufferSize > 0) {
			if (pBlocksBuffer == nullptr)
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

			const Lib3MF4EOS::sEOSTaskBlock* pCurrentBlock = pBlocksBuffer;
			for (uint64_t nBlockIndex = 0; nBlockIndex < nBlocksBufferSize; nBlockIndex++) {
				Lib3MF4EOS::sEOSTaskBlock newBlock = *pCurrentBlock;
				if (newBlock.m_StartPointIndex >= nPointsBufferSize)
					throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPOINTINDEXINBLOCK);
			if (newBlock.m_EndPointIndex > nPointsBufferSize)
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPOINTINDEXINBLOCK);
				if (newBlock.m_StartPointIndex > newBlock.m_EndPointIndex)
					throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_STARTINDEXLARGERTHANENDINDEX);

				newBlock.m_StartPointIndex += (uint32_t)nCurrentPointCount;
				newBlock.m_EndPointIndex += (uint32_t)nCurrentPointCount;
				pCurrentBlock++;

				m_Blocks.push_back (newBlock);
			}


		}


	}
	else {
		if (nBlocksBufferSize > 0)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_NOPOINTSTOADDTOLAYER);
	}
	
}

 