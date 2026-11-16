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


Abstract: EOS SDK adapter for version 2.25.

*/

#if __has_include("libeos_eossdk_decl_2_25.hpp")

#include "libeos_eossdk_adapter_2_25.hpp"
#include "libeos_eossdk_neutral_checks.hpp"

using namespace Lib3MF4EOS::Impl;

LIBEOS_CHECK_NEUTRAL_TYPES(SDK_2_25)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_25, EOS_ERR_INSUFFICIENT_DISK_SPACE)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_25, EOS_EXPOSURE_TYPE_HIDDEN)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_25, EOS_MT_M808)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_25, EOS_MT_M450)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_25, EOS_MT_M451)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_25, EOS_MT_M405)
LIBEOS_CHECK_NEUTRAL_ENUM(SDK_2_25, EOS_BEAMSHAPE_STANDARD)

uint32_t CEOSSDKAdapter_2_25::getVersionMajor() const
{
    return SDK_2_25::VERSION_MAJOR;
}

uint32_t CEOSSDKAdapter_2_25::getVersionMinor() const
{
    return SDK_2_25::VERSION_MINOR;
}

std::string CEOSSDKAdapter_2_25::getOpenJobVersion() const
{
    return "1.5.23";
}

uint32_t CEOSSDKAdapter_2_25::getApiVersion() const
{
    return 3;
}

bool CEOSSDKAdapter_2_25::supportsProductVersion() const
{
    return true;
}

bool CEOSSDKAdapter_2_25::versionHasBeamShape() const
{
    return true;
}

bool CEOSSDKAdapter_2_25::supportsBeamShape() const
{
    return (m_EntryPoints.EosToolpath_WriteSetBeamShape != nullptr);
}

EOS_ERROR CEOSSDKAdapter_2_25::writeSetBeamShape(EOS_FILE_HANDLE hFileHandle, EOS_BEAMSHAPE beamShape, EOS_INT nProfileID)
{
    if (m_EntryPoints.EosToolpath_WriteSetBeamShape == nullptr)
        return EOS_ERR_UNSUPPORTED;

    return (EOS_ERROR)m_EntryPoints.EosToolpath_WriteSetBeamShape(hFileHandle, (SDK_2_25::EOS_BEAMSHAPE)beamShape, nProfileID);
}

#endif // __has_include("libeos_eossdk_decl_2_25.hpp")
