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


Abstract: EOS SDK adapter for version 2.26.

*/

#include "libeos_eossdk_adapter_2_26.hpp"
#include "libeos_eossdk_neutral_checks.hpp"

using namespace Lib3MF4EOS::Impl;

LIBEOS_CHECK_NEUTRAL_TYPES(SDK_2_26)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_ERR_INSUFFICIENT_DISK_SPACE)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_ERR_EOSSIM_CORRUPT)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_EXPOSURE_TYPE_HIDDEN)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_MT_M808)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_MT_M450)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_MT_M451)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_MT_M405)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_26, EOS_BEAMSHAPE_STANDARD)

void CEOSSDKAdapter_2_26::convertMachineInfoExtras(const SDK_2_26::EOS_MACHINE_INFO& info, sEOSMachineInfo& machineInfo)
{
    machineInfo.m_sModelName = AdapterConversion::toWString(info.modelName);
    machineInfo.m_bIsCustomizedMachine = (info.isCustomizedMachine != 0);
}

void CEOSSDKAdapter_2_26::convertMaterialSetInfoExtras(const SDK_2_26::EOS_MATERIALSET_INFO& info, sEOSMaterialSetInfo& materialSetInfo)
{
    materialSetInfo.m_sModelName = AdapterConversion::toWString(info.modelName);
    materialSetInfo.m_bTargetsCustomizedMachine = (info.targetsCustomizedMachine != 0);
}

uint32_t CEOSSDKAdapter_2_26::getVersionMajor() const
{
    return SDK_2_26::VERSION_MAJOR;
}

uint32_t CEOSSDKAdapter_2_26::getVersionMinor() const
{
    return SDK_2_26::VERSION_MINOR;
}

std::string CEOSSDKAdapter_2_26::getOpenJobVersion() const
{
    return "1.5.23";
}

uint32_t CEOSSDKAdapter_2_26::getApiVersion() const
{
    return 2;
}

bool CEOSSDKAdapter_2_26::supportsProductVersion() const
{
    return true;
}

bool CEOSSDKAdapter_2_26::versionHasBeamShape() const
{
    return true;
}

bool CEOSSDKAdapter_2_26::supportsBeamShape() const
{
    return (m_EntryPoints.EosToolpath_WriteSetBeamShape != nullptr);
}

EOS_ERROR CEOSSDKAdapter_2_26::writeSetBeamShape(EOS_FILE_HANDLE hFileHandle, EOS_BEAMSHAPE beamShape, EOS_INT nProfileID)
{
    if (m_EntryPoints.EosToolpath_WriteSetBeamShape == nullptr)
        return EOS_ERR_UNSUPPORTED;

    return (EOS_ERROR)m_EntryPoints.EosToolpath_WriteSetBeamShape(hFileHandle, (SDK_2_26::EOS_BEAMSHAPE)beamShape, nProfileID);
}
