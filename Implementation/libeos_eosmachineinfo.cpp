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


Abstract: This is a stub class definition of CEOSMachineInfo

*/

#include "libeos_eosmachineinfo.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.
#include "../Common/common_utils.hpp"

using namespace Lib3MF4EOS::Impl;

CEOSMachineInfo::CEOSMachineInfo()
	:  m_VolumeType (Lib3MF4EOS::eEOSVolumeType::Unknown),
	m_ModelType (Lib3MF4EOS::eEOSModelType::Unknown),
	m_dDimensionX (0.0),
	m_dDimensionY (0.0),
	m_dDimensionZ (0.0),
	m_dRadius (0.0),
	m_nSoftwareVersionMajor (0),
	m_nSoftwareVersionMinor (0),
	m_nSoftwareVersionBuild (0),
	m_nSoftwareVersionRevision (0),
	m_nInterfaceVersionMajor (0),
	m_nInterfaceVersionMinor (0),
	m_nInterfaceVersionBuild (0),
	m_nInterfaceVersionRevision (0),
	m_nNumberOfLaserScanners (0),
	m_nSDKModelType (0)

{

}

CEOSMachineInfo::~CEOSMachineInfo()
{

}

Lib3MF4EOS::eEOSVolumeType CEOSMachineInfo::GetVolumeType()
{
	return m_VolumeType;
}

void CEOSMachineInfo::GetDimensions(Lib3MF4EOS_double & dDimensionX, Lib3MF4EOS_double & dDimensionY, Lib3MF4EOS_double & dDimensionZ)
{
	if (m_VolumeType != eEOSVolumeType::CuboidVolume)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUILDVOLUMEISNOTCUBOID);

	dDimensionX = m_dDimensionX;
	dDimensionY = m_dDimensionY;
	dDimensionZ = m_dDimensionZ;
}

void CEOSMachineInfo::GetRadius(Lib3MF4EOS_double & dRadius)
{
	if (m_VolumeType != eEOSVolumeType::CylinderVolume)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUILDVOLUMEISNOTCYLINDER);

	dRadius = m_dRadius;
}

Lib3MF4EOS::eEOSModelType CEOSMachineInfo::GetModelType()
{
	return m_ModelType;
}

std::string CEOSMachineInfo::GetModelTypeIdentifier()
{
	std::string sIdentifier = eosModelTypeToMachineTarget((EOS_MODEL_TYPE)m_ModelType);
	if (sIdentifier.empty ())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_UNKNOWNMODELTYPE, "the machine has the EOS model type " + std::to_string(m_nSDKModelType) + ", which has no known machine target");

	return sIdentifier;
}

std::string CEOSMachineInfo::GetModelTypeProductName()
{
	if (!m_sModelName.empty())
		return m_sModelName;

	switch (m_ModelType) {
	case eEOSModelType::M290: return "EOS M 290 400W";
	case eEOSModelType::M400: return "EOS M 400";
	case eEOSModelType::M404: return "EOS M 400-4";
	case eEOSModelType::P500: return "EOS P 500";
	case eEOSModelType::M304: return "EOS M 300-4";
	case eEOSModelType::P900: return "EOS P 900";
	case eEOSModelType::P396: return "EOS P 396";
	case eEOSModelType::P420: return "Integra P 450";
	case eEOSModelType::P711: return "EOS P 810";
	case eEOSModelType::M100: return "EOS M 100";
	case eEOSModelType::P120: return "FORMIGA P 110 FDR";
	case eEOSModelType::M292: return "EOS M 290XF";
	case eEOSModelType::P520: return "EOS P 500 FDR";
	case eEOSModelType::M293: return "EOS M 290-2";
	case eEOSModelType::P110: return "FORMIGA P 110 Velocis";
	case eEOSModelType::Unknown: return "unknown EOS model type " + std::to_string(m_nSDKModelType);

	default:
		return GetModelTypeIdentifier();
	}

}


void CEOSMachineInfo::GetSoftwareVersion(Lib3MF4EOS_uint32 & nMajor, Lib3MF4EOS_uint32 & nMinor, Lib3MF4EOS_uint32 & nBuild, Lib3MF4EOS_uint32 & nRevision)
{
	nMajor = m_nSoftwareVersionMajor;
	nMinor = m_nSoftwareVersionMinor;
	nBuild = m_nSoftwareVersionBuild;
	nRevision = m_nSoftwareVersionRevision;
}

std::string CEOSMachineInfo::GetSoftwareVersionString()
{
	return std::to_string(m_nSoftwareVersionMajor) + "." + std::to_string(m_nSoftwareVersionMinor) + "." + std::to_string(m_nSoftwareVersionBuild) + "." + std::to_string(m_nSoftwareVersionRevision);
}

void CEOSMachineInfo::GetInterfaceVersion(Lib3MF4EOS_uint32 & nMajor, Lib3MF4EOS_uint32 & nMinor, Lib3MF4EOS_uint32 & nBuild, Lib3MF4EOS_uint32 & nRevision)
{
	nMajor = m_nInterfaceVersionMajor;
	nMinor = m_nInterfaceVersionMinor;
	nBuild = m_nInterfaceVersionBuild;
	nRevision = m_nInterfaceVersionRevision;
}

std::string CEOSMachineInfo::GetInterfaceVersionString()
{
	return std::to_string(m_nInterfaceVersionMajor) + "." + std::to_string(m_nInterfaceVersionMinor) + "." + std::to_string(m_nInterfaceVersionBuild) + "." + std::to_string(m_nInterfaceVersionRevision);
}

std::string CEOSMachineInfo::GetSerialNumber()
{
	return m_sSerialNumber;
}

Lib3MF4EOS_uint32 CEOSMachineInfo::GetNumberOfLaserScanners()
{
	return m_nNumberOfLaserScanners;
}

std::string CEOSMachineInfo::GetPrintDomain()
{
	return m_sPrintDomain;
}

const sEOSLaserScannerInfo& CEOSMachineInfo::getLaserScanner(uint32_t nScannerIndex)
{
	if (nScannerIndex >= m_nNumberOfLaserScanners)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "laser scanner index " + std::to_string(nScannerIndex) + " is out of range; the machine has " +
			std::to_string(m_nNumberOfLaserScanners) + " laser scanners");
	if (nScannerIndex >= m_LaserScanners.size())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTGETMACHINEINFORMATION, "the EOS SDK gave no details of the laser scanners");

	return m_LaserScanners[nScannerIndex];
}

void CEOSMachineInfo::GetLaserScannerField(const Lib3MF4EOS_uint32 nScannerIndex, Lib3MF4EOS_double & dMinX, Lib3MF4EOS_double & dMinY, Lib3MF4EOS_double & dMaxX, Lib3MF4EOS_double & dMaxY)
{
	auto& scanner = getLaserScanner(nScannerIndex);
	dMinX = scanner.m_dMinX;
	dMinY = scanner.m_dMinY;
	dMaxX = scanner.m_dMaxX;
	dMaxY = scanner.m_dMaxY;
}

void CEOSMachineInfo::GetLaserScannerLimits(const Lib3MF4EOS_uint32 nScannerIndex, Lib3MF4EOS_double & dNominalPower, Lib3MF4EOS_double & dMaxPower, Lib3MF4EOS_double & dMaxScanSpeed)
{
	auto& scanner = getLaserScanner(nScannerIndex);
	dNominalPower = scanner.m_dLaserPowerNominal;
	dMaxPower = scanner.m_dLaserPowerMax;
	dMaxScanSpeed = scanner.m_dMaxScanSpeed;
}

void CEOSMachineInfo::GetLaserScannerCapabilities(const Lib3MF4EOS_uint32 nScannerIndex, bool & bFocusDefocus, bool & bVariableFocus, bool & bBeamShaping)
{
	auto& scanner = getLaserScanner(nScannerIndex);
	bFocusDefocus = (scanner.m_nCapabilities & EOS_LSC_FOCUS_DEFOCUS) != 0;
	bVariableFocus = (scanner.m_nCapabilities & EOS_LSC_VARIABLE_FOCUS) != 0;
	bBeamShaping = (scanner.m_nCapabilities & EOS_LSC_BEAMSHAPING) != 0;
}


void CEOSMachineInfo::loadFromMachineInfo(const sEOSMachineInfo& machineInfo)
{
	switch (machineInfo.m_VolumeType) {
		case EOS_VOLUME_TYPE::EOS_VOL_CUBOID: 
			m_VolumeType = Lib3MF4EOS::eEOSVolumeType::CuboidVolume;
			m_dDimensionX = machineInfo.m_dDimX;
			m_dDimensionY = machineInfo.m_dDimY;
			m_dDimensionZ = machineInfo.m_dDimZ;
			break;

		case EOS_VOLUME_TYPE::EOS_VOL_CYLINDER: m_VolumeType = Lib3MF4EOS::eEOSVolumeType::CylinderVolume;
			m_dRadius = machineInfo.m_dDimR;
			break;

		default:
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_UNKNOWNVOLUMETYPE);
	}

	// The neutral model type values are identical to the API enum values.
	m_nSDKModelType = (uint32_t)machineInfo.m_ModelType;
	if (eosModelTypeToIdentifier(machineInfo.m_ModelType).empty())
		m_ModelType = Lib3MF4EOS::eEOSModelType::Unknown;
	else
		m_ModelType = (Lib3MF4EOS::eEOSModelType)machineInfo.m_ModelType;

	m_nSoftwareVersionMajor = machineInfo.m_MachineSoftwareVersion.m_nMajor;
	m_nSoftwareVersionMinor = machineInfo.m_MachineSoftwareVersion.m_nMinor;
	m_nSoftwareVersionBuild = machineInfo.m_MachineSoftwareVersion.m_nBuild;
	m_nSoftwareVersionRevision = machineInfo.m_MachineSoftwareVersion.m_nRevision;

	m_nInterfaceVersionMajor = machineInfo.m_MachineInterfaceVersion.m_nMajor;
	m_nInterfaceVersionMinor = machineInfo.m_MachineInterfaceVersion.m_nMinor;
	m_nInterfaceVersionBuild = machineInfo.m_MachineInterfaceVersion.m_nBuild;
	m_nInterfaceVersionRevision = machineInfo.m_MachineInterfaceVersion.m_nRevision;

	m_nNumberOfLaserScanners = machineInfo.m_nNumberOfLaserScanners;
	m_LaserScanners = machineInfo.m_LaserScanners;

	m_sSerialNumber = AMCCommon::CUtils::UTF16toUTF8(machineInfo.m_sSerialNumber);
	m_sPrintDomain = AMCCommon::CUtils::UTF16toUTF8(machineInfo.m_sPrintDomain);
	m_sModelName = AMCCommon::CUtils::UTF16toUTF8(machineInfo.m_sModelName);

}


void CEOSMachineInfo::loadSimulatedMachineInfo(uint32_t nLaserScannerCount)
{
	m_ModelType = eEOSModelType::M290;
	m_nSDKModelType = (uint32_t)eEOSModelType::M290;
	m_VolumeType = eEOSVolumeType::CuboidVolume;

	m_dDimensionX = 321;
	m_dDimensionY = 345;
	m_dDimensionZ = 432;

	m_nSoftwareVersionMajor = 1;
	m_nSoftwareVersionMinor = 2;
	m_nSoftwareVersionBuild = 3;
	m_nSoftwareVersionRevision = 4;

	m_nInterfaceVersionMajor = 5;
	m_nInterfaceVersionMinor = 6;
	m_nInterfaceVersionBuild = 7;
	m_nInterfaceVersionRevision = 8;

	m_nNumberOfLaserScanners = nLaserScannerCount;

	// Invented values that fit the simulated build volume.
	sEOSLaserScannerInfo scanner;
	scanner.m_dLaserPowerNominal = 380.0;
	scanner.m_dLaserPowerMax = 415.0;
	scanner.m_dMaxScanSpeed = 6800.0;
	scanner.m_dMinX = -160.5;
	scanner.m_dMinY = -172.5;
	scanner.m_dMaxX = 160.5;
	scanner.m_dMaxY = 172.5;
	scanner.m_nCapabilities = EOS_LSC_FOCUS_DEFOCUS;
	m_LaserScanners.assign(nLaserScannerCount, scanner);

	m_sSerialNumber = "ABCDEF";
	m_sPrintDomain = "PD000001";

}

