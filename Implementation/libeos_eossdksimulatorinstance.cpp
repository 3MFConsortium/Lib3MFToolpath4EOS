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

CEOSSDKSimulatorInstance::CEOSSDKSimulatorInstance(const std::string& sSDKVersion, eSimulationMode simulationMode)
    : m_sSDKVersion (sSDKVersion), m_SimulationMode (simulationMode), m_nLaserScannerCount (1)
{
    if (simulationMode == eSimulationMode::smRecording)
        m_pToolpathRecorder = std::make_shared<CToolpathRecorder>();
}

CEOSSDKSimulatorInstance::~CEOSSDKSimulatorInstance()
{

}


std::string CEOSSDKSimulatorInstance::GetAPIVersion()
{
    return m_sSDKVersion;
}

eSimulationMode CEOSSDKSimulatorInstance::getSimulationMode()
{
    return m_SimulationMode;
}

PToolpathRecorder CEOSSDKSimulatorInstance::getToolpathRecorder()
{
    return m_pToolpathRecorder;
}

void CEOSSDKSimulatorInstance::setLaserScannerCount(uint32_t nCount)
{
    m_nLaserScannerCount = nCount;
    if (m_pToolpathRecorder.get() != nullptr)
        m_pToolpathRecorder->setLaserScannerCount(nCount);
}

uint32_t CEOSSDKSimulatorInstance::getLaserScannerCount()
{
    return m_nLaserScannerCount;
}
