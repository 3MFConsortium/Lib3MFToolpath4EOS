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


Abstract: This is a stub class definition of CEOSSDKSimulator

*/

#include "libeos_eossdksimulator.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.


using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CEOSSDKSimulator 
**************************************************************************************************************************/

CEOSSDKSimulator::CEOSSDKSimulator(PEOSSDK pEOSSDK)
    : m_pEOSSDK(pEOSSDK)
{
    if (pEOSSDK.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
}

CEOSSDKSimulator::~CEOSSDKSimulator()
{

}


std::string CEOSSDKSimulator::GetAPIVersion()
{
    return m_pEOSSDK->getSimulator ()->GetAPIVersion ();
}

PToolpathRecorder CEOSSDKSimulator::getRecorder()
{
    auto pRecorder = m_pEOSSDK->getToolpathRecorder();
    if (pRecorder.get() == nullptr)
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_SIMULATORNOTRECORDING);
    return pRecorder;
}

bool CEOSSDKSimulator::IsRecording()
{
    return m_pEOSSDK->getToolpathRecorder().get() != nullptr;
}

Lib3MF4EOS_uint64 CEOSSDKSimulator::GetRecordedCommandCount()
{
    return getRecorder()->getCommandCount();
}

std::string CEOSSDKSimulator::GetRecordedCommandName(const Lib3MF4EOS_uint64 nIndex)
{
    return toolpathCommandName(getRecorder()->getCommand(nIndex).m_eCommand);
}

void CEOSSDKSimulator::GetRecordedCommandArguments(const Lib3MF4EOS_uint64 nIndex, Lib3MF4EOS_uint64 nArgumentsBufferSize, Lib3MF4EOS_uint64* pArgumentsNeededCount, Lib3MF4EOS_double* pArgumentsBuffer)
{
    auto command = getRecorder()->getCommand(nIndex);

    if (pArgumentsNeededCount != nullptr)
        *pArgumentsNeededCount = command.m_nArgumentCount;

    if (pArgumentsBuffer != nullptr) {
        if (nArgumentsBufferSize < command.m_nArgumentCount)
            throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUFFERTOOSMALL);
        for (uint32_t nArgument = 0; nArgument < command.m_nArgumentCount; nArgument++)
            pArgumentsBuffer[nArgument] = command.m_Arguments[nArgument];
    }
}

std::string CEOSSDKSimulator::GetRecordedCommandsAsText()
{
    return getRecorder()->toText();
}

Lib3MF4EOS_uint64 CEOSSDKSimulator::GetRecordedLayerCount()
{
    return getRecorder()->getLayerCount();
}

void CEOSSDKSimulator::ClearRecording()
{
    getRecorder()->clear();
}

void CEOSSDKSimulator::SetNumberOfLaserScanners(const Lib3MF4EOS_uint32 nCount)
{
    if ((nCount < 1) || (nCount > LIBEOS_SIMULATOR_MAXLASERSCANNERS))
        throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "the simulated machine can have 1 to " + std::to_string(LIBEOS_SIMULATOR_MAXLASERSCANNERS) +
            " laser scanners, not " + std::to_string(nCount));
    m_pEOSSDK->getSimulator()->setLaserScannerCount(nCount);
}

Lib3MF4EOS_uint32 CEOSSDKSimulator::GetNumberOfLaserScanners()
{
    return m_pEOSSDK->getSimulator()->getLaserScannerCount();
}

