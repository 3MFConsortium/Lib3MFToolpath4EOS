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


Abstract: This is the class declaration of CEOSMachineInfo

*/


#ifndef __LIBEOS_EOSMACHINEINFO
#define __LIBEOS_EOSMACHINEINFO

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include "libeos_eossdk.hpp"

namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CEOSMachineInfo 
**************************************************************************************************************************/

class CEOSMachineInfo : public virtual IEOSMachineInfo, public virtual CBase {
protected:
	Lib3MF4EOS::eEOSVolumeType m_VolumeType;
	Lib3MF4EOS::eEOSModelType m_ModelType;
	double m_dDimensionX;
	double m_dDimensionY;
	double m_dDimensionZ;
	double m_dRadius;

	uint32_t m_nSoftwareVersionMajor;
	uint32_t m_nSoftwareVersionMinor;
	uint32_t m_nSoftwareVersionBuild;
	uint32_t m_nSoftwareVersionRevision;

	uint32_t m_nInterfaceVersionMajor;
	uint32_t m_nInterfaceVersionMinor;
	uint32_t m_nInterfaceVersionBuild;
	uint32_t m_nInterfaceVersionRevision;

	std::string m_sSerialNumber;
	uint32_t m_nNumberOfLaserScanners;
	std::string m_sPrintDomain;
	std::string m_sModelName;
	// The model type value of the SDK, also if LibEOS does not know it.
	uint32_t m_nSDKModelType;
	// Empty if the SDK gives the number of scanners but no details.
	std::vector<sEOSLaserScannerInfo> m_LaserScanners;

	const sEOSLaserScannerInfo& getLaserScanner(uint32_t nScannerIndex);

public:

	CEOSMachineInfo();

	virtual ~CEOSMachineInfo();

	Lib3MF4EOS::eEOSVolumeType GetVolumeType() override;

	void GetDimensions(Lib3MF4EOS_double & dDimensionX, Lib3MF4EOS_double & dDimensionY, Lib3MF4EOS_double & dDimensionZ) override;

	void GetRadius(Lib3MF4EOS_double & dRadius) override;

	Lib3MF4EOS::eEOSModelType GetModelType() override;

	std::string GetModelTypeIdentifier() override;

	std::string GetModelTypeProductName() override;

	void GetSoftwareVersion(Lib3MF4EOS_uint32 & nMajor, Lib3MF4EOS_uint32 & nMinor, Lib3MF4EOS_uint32 & nBuild, Lib3MF4EOS_uint32 & nRevision) override;

	std::string GetSoftwareVersionString() override;

	void GetInterfaceVersion(Lib3MF4EOS_uint32 & nMajor, Lib3MF4EOS_uint32 & nMinor, Lib3MF4EOS_uint32 & nBuild, Lib3MF4EOS_uint32 & nRevision) override;

	std::string GetInterfaceVersionString() override;

	std::string GetSerialNumber() override;

	Lib3MF4EOS_uint32 GetNumberOfLaserScanners() override;

	std::string GetPrintDomain() override;

	void GetLaserScannerField(const Lib3MF4EOS_uint32 nScannerIndex, Lib3MF4EOS_double & dMinX, Lib3MF4EOS_double & dMinY, Lib3MF4EOS_double & dMaxX, Lib3MF4EOS_double & dMaxY) override;

	void GetLaserScannerLimits(const Lib3MF4EOS_uint32 nScannerIndex, Lib3MF4EOS_double & dNominalPower, Lib3MF4EOS_double & dMaxPower, Lib3MF4EOS_double & dMaxScanSpeed) override;

	void GetLaserScannerCapabilities(const Lib3MF4EOS_uint32 nScannerIndex, bool & bFocusDefocus, bool & bVariableFocus, bool & bBeamShaping) override;

	void loadFromMachineInfo(const sEOSMachineInfo& machineInfo);

	void loadSimulatedMachineInfo(uint32_t nLaserScannerCount);
};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSMACHINEINFO
