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


Abstract: This is the class declaration of CEOSMaterialSet

*/


#ifndef __LIBEOS_EOSMATERIALSET
#define __LIBEOS_EOSMATERIALSET

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include <vector>

namespace Lib3MF4EOS {
namespace Impl {

struct SEOSMaterialExposureSet {
    std::string m_sName;
    uint32_t m_nLayerThickness;
    bool m_bIsSupport;
};

/*************************************************************************************************************************
 Class declaration of CEOSMaterialSet 
**************************************************************************************************************************/

class CEOSMaterialSet : public virtual IEOSMaterialSet, public virtual CBase {
private:


protected:
    
    std::string m_sName;
    std::string m_sUUID;
    std::string m_sMaterial;
    std::string m_sModelType;
    uint32_t m_nBaseLayerThickness;
    uint32_t m_nParameterSetVersionMajor;
    uint32_t m_nParameterSetVersionMinor;
    uint32_t m_nParameterSetVersionBuild;
    uint32_t m_nParameterSetVersionRevision;
    std::vector<SEOSMaterialExposureSet> m_ExposureSets;

    std::vector<uint8_t> m_EOSParBuffer;

public:

    CEOSMaterialSet(const std::string & sName, const std::string & sUUID, const std::string & sMaterial, const std::string& sModelType, const uint32_t nBaseLayerThickness, const uint32_t nParameterSetVersionMajor, const uint32_t nParameterSetVersionMinor, const uint32_t nParameterSetVersionBuild, const uint32_t nParameterSetVersionRevision);
    virtual ~CEOSMaterialSet();


	std::string GetName() override;

	std::string GetUUID() override;

	std::string GetMaterial() override;
    
    std::string GetModelType() override;

    Lib3MF4EOS_uint32 GetBaseLayerthickness() override;

	std::string GetParameterSetVersion() override;

	Lib3MF4EOS_uint32 GetExposureSetCount() override;

	std::string GetExposureSetName(const Lib3MF4EOS_uint32 nExposureSetIndex) override;

    Lib3MF4EOS_uint32 GetExposureSetLayerThickness(const Lib3MF4EOS_uint32 nExposureSetIndex) override;

    void addExposureSet(const std::string& sName, uint32_t nLayerThickness, bool bIsSupport);

    const std::vector<SEOSMaterialExposureSet>& getExposureSets() const;

    std::vector<uint8_t> & getEOSParBuffer ();

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSMATERIALSET
