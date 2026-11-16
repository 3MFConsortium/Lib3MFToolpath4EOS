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


Abstract: This is a stub class definition of CEOSMaterialSet

*/

#include "libeos_eosmaterialset.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.
#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>

#include "../Common/common_utils.hpp"

using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CEOSMaterialSet 
**************************************************************************************************************************/

CEOSMaterialSet::CEOSMaterialSet(const std::string& sName, const std::string& sUUID, const std::string& sMaterial, const std::string& sModelType, const uint32_t nBaseLayerThickness, const uint32_t nParameterSetVersionMajor, const uint32_t nParameterSetVersionMinor, const uint32_t nParameterSetVersionBuild, const uint32_t nParameterSetVersionRevision)
 : m_sName (sName), m_sUUID (sUUID), m_sMaterial(sMaterial), m_nBaseLayerThickness (nBaseLayerThickness), m_nParameterSetVersionMajor (nParameterSetVersionMajor), m_nParameterSetVersionMinor (nParameterSetVersionMinor), m_nParameterSetVersionBuild (nParameterSetVersionBuild), m_nParameterSetVersionRevision (nParameterSetVersionRevision), m_sModelType (sModelType)

{

}

CEOSMaterialSet::~CEOSMaterialSet()
{

}

std::string CEOSMaterialSet::GetName()
{
    return m_sName;
}

std::string CEOSMaterialSet::GetUUID()
{
    return m_sUUID;
}

std::string CEOSMaterialSet::GetMaterial()
{
    return m_sMaterial;
}

std::string CEOSMaterialSet::GetModelType()
{
    return m_sModelType;
}

Lib3MF4EOS_uint32 CEOSMaterialSet::GetBaseLayerthickness()
{
    return m_nBaseLayerThickness;
}

std::string CEOSMaterialSet::GetParameterSetVersion()
{
    return std::to_string(m_nParameterSetVersionMajor) + '.' + std::to_string(m_nParameterSetVersionMinor) + '.' + std::to_string(m_nParameterSetVersionBuild) + '.' + std::to_string(m_nParameterSetVersionRevision);
}

Lib3MF4EOS_uint32 CEOSMaterialSet::GetExposureSetCount()
{
    return (uint32_t)m_ExposureSets.size();
}

std::string CEOSMaterialSet::GetExposureSetName(const Lib3MF4EOS_uint32 nExposureSetIndex)
{
    if (nExposureSetIndex >= m_ExposureSets.size())
        throw std::runtime_error("invalid exposure set name");

    return m_ExposureSets.at(nExposureSetIndex).m_sName;
}


Lib3MF4EOS_uint32 CEOSMaterialSet::GetExposureSetLayerThickness(const Lib3MF4EOS_uint32 nExposureSetIndex)
{
    if (nExposureSetIndex >= m_ExposureSets.size())
        throw std::runtime_error("invalid exposure set name");

    return m_ExposureSets.at(nExposureSetIndex).m_nLayerThickness;

}


void CEOSMaterialSet::addExposureSet(const std::string& sName, uint32_t nLayerThickness, bool bIsSupport)
{
    m_ExposureSets.push_back(SEOSMaterialExposureSet{ sName, nLayerThickness, bIsSupport });
}

const std::vector<SEOSMaterialExposureSet>& CEOSMaterialSet::getExposureSets() const
{
    return m_ExposureSets;
}

std::vector<uint8_t> & CEOSMaterialSet::getEOSParBuffer()
{
    return m_EOSParBuffer;
}

