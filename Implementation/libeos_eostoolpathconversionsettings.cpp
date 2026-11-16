/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Settings for converting toolpaths into EOS tasks.

*/

#include "libeos_eostoolpathconversionsettings.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_jsonwriter.hpp"

#include <cmath>

using namespace Lib3MF4EOS::Impl;

#define LIBEOS_CONVERSIONSETTINGS_MAXEXPOSURETYPE 16

namespace {

	void checkPositive(const char* pName, double dValue)
	{
		if (!std::isfinite(dValue) || (dValue <= 0.0))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, std::string(pName) + " must be positive: " + std::to_string(dValue));
	}

	void checkNotNegative(const char* pName, double dValue)
	{
		if (!std::isfinite(dValue) || (dValue < 0.0))
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, std::string(pName) + " must not be negative: " + std::to_string(dValue));
	}

	void checkExposureType(const char* pName, uint32_t nValue)
	{
		if (nValue > LIBEOS_CONVERSIONSETTINGS_MAXEXPOSURETYPE)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, std::string(pName) + " is not an EOS exposure type: " + std::to_string(nValue));
	}

	std::string powerRampModeName(Lib3MF4EOS::eEOSPowerRampMode eMode)
	{
		switch (eMode) {
		case Lib3MF4EOS::eEOSPowerRampMode::Polyline: return "Polyline";
		case Lib3MF4EOS::eEOSPowerRampMode::SplitHatches: return "SplitHatches";
		case Lib3MF4EOS::eEOSPowerRampMode::Flatten: return "Flatten";
		case Lib3MF4EOS::eEOSPowerRampMode::Reject: return "Reject";
		default: return "";
		}
	}

	std::string hatchWriteModeName(Lib3MF4EOS::eEOSHatchWriteMode eMode)
	{
		switch (eMode) {
		case Lib3MF4EOS::eEOSHatchWriteMode::JoinTouching: return "JoinTouching";
		case Lib3MF4EOS::eEOSHatchWriteMode::AllPolylines: return "AllPolylines";
		default: return "";
		}
	}

	std::string dwellTimeUnitName(Lib3MF4EOS::eEOSDwellTimeUnit eUnit)
	{
		switch (eUnit) {
		case Lib3MF4EOS::eEOSDwellTimeUnit::Unset: return "Unset";
		case Lib3MF4EOS::eEOSDwellTimeUnit::Microseconds: return "Microseconds";
		case Lib3MF4EOS::eEOSDwellTimeUnit::Milliseconds: return "Milliseconds";
		case Lib3MF4EOS::eEOSDwellTimeUnit::Seconds: return "Seconds";
		default: return "";
		}
	}

	std::string layerZConventionName(Lib3MF4EOS::eEOSLayerZConvention eConvention)
	{
		switch (eConvention) {
		case Lib3MF4EOS::eEOSLayerZConvention::Top: return "Top";
		case Lib3MF4EOS::eEOSLayerZConvention::Base: return "Base";
		default: return "";
		}
	}

	void checkPowerRampMode(Lib3MF4EOS::eEOSPowerRampMode eMode)
	{
		if (powerRampModeName(eMode).empty())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "unknown power ramp mode: " + std::to_string((int32_t)eMode));
	}

	void checkHatchWriteMode(Lib3MF4EOS::eEOSHatchWriteMode eMode)
	{
		if (hatchWriteModeName(eMode).empty())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "unknown hatch write mode: " + std::to_string((int32_t)eMode));
	}

	void checkDwellTimeUnit(Lib3MF4EOS::eEOSDwellTimeUnit eUnit)
	{
		if (dwellTimeUnitName(eUnit).empty())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "unknown dwell time unit: " + std::to_string((int32_t)eUnit));
	}

	void checkLayerZConvention(Lib3MF4EOS::eEOSLayerZConvention eConvention)
	{
		if (layerZConventionName(eConvention).empty())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "unknown layer Z convention: " + std::to_string((int32_t)eConvention));
	}

}

/*************************************************************************************************************************
 SConversionSettings
**************************************************************************************************************************/

SConversionSettings::SConversionSettings()
	: m_bStrictMode(true),
	m_dPowerDelay(190.0),
	m_dMaxPowerStep(1.0),
	m_dMaxSpeedStep(10.0),
	m_dMaxFocusStep(0.05),
	m_dMinSubVectorLength(0.05),
	m_ePowerRampMode(Lib3MF4EOS::eEOSPowerRampMode::Polyline),
	m_eHatchWriteMode(Lib3MF4EOS::eEOSHatchWriteMode::JoinTouching),
	m_dJoinTolerance(0.001),
	m_dHatchPowerDeadband(0.0),
	m_bJoinContours(true),
	m_bRefuseUnequalConstantHatch(true),
	m_bDefaultSkywriting(true),
	m_nDefaultHatchExposureType(2),
	m_nDefaultContourExposureType(10),
	m_bLayerOpening(true),
	m_dLayerOpeningDelay(10.0),
	m_eDwellTimeUnit(Lib3MF4EOS::eEOSDwellTimeUnit::Unset),
	m_eLayerZConvention(Lib3MF4EOS::eEOSLayerZConvention::Top),
	m_bAllowDefaultExposureSets(false)
{
}

void SConversionSettings::validate() const
{
	checkNotNegative("power delay", m_dPowerDelay);
	checkPositive("maximum power step", m_dMaxPowerStep);
	checkPositive("maximum speed step", m_dMaxSpeedStep);
	checkPositive("maximum focus step", m_dMaxFocusStep);
	checkPositive("minimum sub-vector length", m_dMinSubVectorLength);
	checkPowerRampMode(m_ePowerRampMode);
	checkHatchWriteMode(m_eHatchWriteMode);
	checkNotNegative("join tolerance", m_dJoinTolerance);
	checkNotNegative("hatch power deadband", m_dHatchPowerDeadband);
	checkExposureType("default hatch exposure type", m_nDefaultHatchExposureType);
	checkExposureType("default contour exposure type", m_nDefaultContourExposureType);
	checkNotNegative("layer opening delay", m_dLayerOpeningDelay);
	checkDwellTimeUnit(m_eDwellTimeUnit);
	checkLayerZConvention(m_eLayerZConvention);

	for (auto& assignment : m_ExposureSetsForThickness) {
		if ((assignment.first.first == 0) || assignment.second.empty())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "invalid exposure set assignment for thickness " + std::to_string(assignment.first.first));
	}
}

std::string SConversionSettings::toJSON() const
{
	CJSONArray exposureSets;
	for (auto& assignment : m_ExposureSetsForThickness) {
		CJSONObject entry;
		entry.addUnsigned("thickness", assignment.first.first);
		entry.addBool("isSupport", assignment.first.second);
		entry.addString("exposureSet", assignment.second);
		exposureSets.addRaw(entry.toString());
	}

	CJSONObject json;
	json.addBool("strictMode", m_bStrictMode);
	json.addNumber("powerDelay", m_dPowerDelay);
	json.addNumber("maxPowerStep", m_dMaxPowerStep);
	json.addNumber("maxSpeedStep", m_dMaxSpeedStep);
	json.addNumber("maxFocusStep", m_dMaxFocusStep);
	json.addNumber("minSubVectorLength", m_dMinSubVectorLength);
	json.addString("powerRampMode", powerRampModeName(m_ePowerRampMode));
	json.addString("hatchWriteMode", hatchWriteModeName(m_eHatchWriteMode));
	json.addNumber("joinTolerance", m_dJoinTolerance);
	json.addNumber("hatchPowerDeadband", m_dHatchPowerDeadband);
	json.addBool("joinContours", m_bJoinContours);
	json.addBool("refuseUnequalConstantHatch", m_bRefuseUnequalConstantHatch);
	json.addBool("defaultSkywriting", m_bDefaultSkywriting);
	json.addUnsigned("defaultHatchExposureType", m_nDefaultHatchExposureType);
	json.addUnsigned("defaultContourExposureType", m_nDefaultContourExposureType);
	json.addBool("layerOpening", m_bLayerOpening);
	json.addNumber("layerOpeningDelay", m_dLayerOpeningDelay);
	json.addString("dwellTimeUnit", dwellTimeUnitName(m_eDwellTimeUnit));
	json.addString("layerZConvention", layerZConventionName(m_eLayerZConvention));
	json.addBool("allowDefaultExposureSets", m_bAllowDefaultExposureSets);
	json.addRaw("exposureSetsForThickness", exposureSets.toString());
	return json.toString();
}

/*************************************************************************************************************************
 CEOSToolpathConversionSettings
**************************************************************************************************************************/

CEOSToolpathConversionSettings::CEOSToolpathConversionSettings()
{
}

CEOSToolpathConversionSettings::~CEOSToolpathConversionSettings()
{
}

const SConversionSettings& CEOSToolpathConversionSettings::getSettings() const
{
	return m_Settings;
}

void CEOSToolpathConversionSettings::ResetToDefaults()
{
	m_Settings = SConversionSettings();
}

void CEOSToolpathConversionSettings::SetStrictMode(const bool bValue)
{
	m_Settings.m_bStrictMode = bValue;
}

bool CEOSToolpathConversionSettings::GetStrictMode()
{
	return m_Settings.m_bStrictMode;
}

void CEOSToolpathConversionSettings::SetPowerDelay(const Lib3MF4EOS_double dValue)
{
	checkNotNegative("power delay", dValue);
	m_Settings.m_dPowerDelay = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetPowerDelay()
{
	return m_Settings.m_dPowerDelay;
}

void CEOSToolpathConversionSettings::SetMaxPowerStep(const Lib3MF4EOS_double dValue)
{
	checkPositive("maximum power step", dValue);
	m_Settings.m_dMaxPowerStep = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetMaxPowerStep()
{
	return m_Settings.m_dMaxPowerStep;
}

void CEOSToolpathConversionSettings::SetMaxSpeedStep(const Lib3MF4EOS_double dValue)
{
	checkPositive("maximum speed step", dValue);
	m_Settings.m_dMaxSpeedStep = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetMaxSpeedStep()
{
	return m_Settings.m_dMaxSpeedStep;
}

void CEOSToolpathConversionSettings::SetMaxFocusStep(const Lib3MF4EOS_double dValue)
{
	checkPositive("maximum focus step", dValue);
	m_Settings.m_dMaxFocusStep = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetMaxFocusStep()
{
	return m_Settings.m_dMaxFocusStep;
}

void CEOSToolpathConversionSettings::SetMinSubVectorLength(const Lib3MF4EOS_double dValue)
{
	checkPositive("minimum sub-vector length", dValue);
	m_Settings.m_dMinSubVectorLength = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetMinSubVectorLength()
{
	return m_Settings.m_dMinSubVectorLength;
}

void CEOSToolpathConversionSettings::SetPowerRampMode(const Lib3MF4EOS::eEOSPowerRampMode eValue)
{
	checkPowerRampMode(eValue);
	m_Settings.m_ePowerRampMode = eValue;
}

Lib3MF4EOS::eEOSPowerRampMode CEOSToolpathConversionSettings::GetPowerRampMode()
{
	return m_Settings.m_ePowerRampMode;
}

void CEOSToolpathConversionSettings::SetHatchWriteMode(const Lib3MF4EOS::eEOSHatchWriteMode eValue)
{
	checkHatchWriteMode(eValue);
	m_Settings.m_eHatchWriteMode = eValue;
}

Lib3MF4EOS::eEOSHatchWriteMode CEOSToolpathConversionSettings::GetHatchWriteMode()
{
	return m_Settings.m_eHatchWriteMode;
}

void CEOSToolpathConversionSettings::SetJoinTolerance(const Lib3MF4EOS_double dValue)
{
	checkNotNegative("join tolerance", dValue);
	m_Settings.m_dJoinTolerance = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetJoinTolerance()
{
	return m_Settings.m_dJoinTolerance;
}

void CEOSToolpathConversionSettings::SetHatchPowerDeadband(const Lib3MF4EOS_double dValue)
{
	checkNotNegative("hatch power deadband", dValue);
	m_Settings.m_dHatchPowerDeadband = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetHatchPowerDeadband()
{
	return m_Settings.m_dHatchPowerDeadband;
}

void CEOSToolpathConversionSettings::SetJoinContours(const bool bValue)
{
	m_Settings.m_bJoinContours = bValue;
}

bool CEOSToolpathConversionSettings::GetJoinContours()
{
	return m_Settings.m_bJoinContours;
}

void CEOSToolpathConversionSettings::SetRefuseUnequalConstantHatch(const bool bValue)
{
	m_Settings.m_bRefuseUnequalConstantHatch = bValue;
}

bool CEOSToolpathConversionSettings::GetRefuseUnequalConstantHatch()
{
	return m_Settings.m_bRefuseUnequalConstantHatch;
}

void CEOSToolpathConversionSettings::SetDefaultSkywriting(const bool bValue)
{
	m_Settings.m_bDefaultSkywriting = bValue;
}

bool CEOSToolpathConversionSettings::GetDefaultSkywriting()
{
	return m_Settings.m_bDefaultSkywriting;
}

void CEOSToolpathConversionSettings::SetDefaultHatchExposureType(const Lib3MF4EOS_uint32 nValue)
{
	checkExposureType("default hatch exposure type", nValue);
	m_Settings.m_nDefaultHatchExposureType = nValue;
}

Lib3MF4EOS_uint32 CEOSToolpathConversionSettings::GetDefaultHatchExposureType()
{
	return m_Settings.m_nDefaultHatchExposureType;
}

void CEOSToolpathConversionSettings::SetDefaultContourExposureType(const Lib3MF4EOS_uint32 nValue)
{
	checkExposureType("default contour exposure type", nValue);
	m_Settings.m_nDefaultContourExposureType = nValue;
}

Lib3MF4EOS_uint32 CEOSToolpathConversionSettings::GetDefaultContourExposureType()
{
	return m_Settings.m_nDefaultContourExposureType;
}

void CEOSToolpathConversionSettings::SetLayerOpening(const bool bValue)
{
	m_Settings.m_bLayerOpening = bValue;
}

bool CEOSToolpathConversionSettings::GetLayerOpening()
{
	return m_Settings.m_bLayerOpening;
}

void CEOSToolpathConversionSettings::SetLayerOpeningDelay(const Lib3MF4EOS_double dValue)
{
	checkNotNegative("layer opening delay", dValue);
	m_Settings.m_dLayerOpeningDelay = dValue;
}

Lib3MF4EOS_double CEOSToolpathConversionSettings::GetLayerOpeningDelay()
{
	return m_Settings.m_dLayerOpeningDelay;
}

void CEOSToolpathConversionSettings::SetDwellTimeUnit(const Lib3MF4EOS::eEOSDwellTimeUnit eValue)
{
	checkDwellTimeUnit(eValue);
	m_Settings.m_eDwellTimeUnit = eValue;
}

Lib3MF4EOS::eEOSDwellTimeUnit CEOSToolpathConversionSettings::GetDwellTimeUnit()
{
	return m_Settings.m_eDwellTimeUnit;
}

void CEOSToolpathConversionSettings::SetLayerZConvention(const Lib3MF4EOS::eEOSLayerZConvention eValue)
{
	checkLayerZConvention(eValue);
	m_Settings.m_eLayerZConvention = eValue;
}

Lib3MF4EOS::eEOSLayerZConvention CEOSToolpathConversionSettings::GetLayerZConvention()
{
	return m_Settings.m_eLayerZConvention;
}

void CEOSToolpathConversionSettings::SetAllowDefaultExposureSets(const bool bValue)
{
	m_Settings.m_bAllowDefaultExposureSets = bValue;
}

bool CEOSToolpathConversionSettings::GetAllowDefaultExposureSets()
{
	return m_Settings.m_bAllowDefaultExposureSets;
}

void CEOSToolpathConversionSettings::SetExposureSetForThickness(const Lib3MF4EOS_uint32 nThickness, const bool bIsSupport, const std::string& sName)
{
	if (nThickness == 0)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDCONVERSIONSETTINGS, "layer thickness of an exposure set assignment must be positive");

	auto key = std::make_pair((uint32_t)nThickness, bIsSupport);
	if (sName.empty())
		m_Settings.m_ExposureSetsForThickness.erase(key);
	else
		m_Settings.m_ExposureSetsForThickness[key] = sName;
}

std::string CEOSToolpathConversionSettings::GetExposureSetForThickness(const Lib3MF4EOS_uint32 nThickness, const bool bIsSupport)
{
	auto iAssignment = m_Settings.m_ExposureSetsForThickness.find(std::make_pair((uint32_t)nThickness, bIsSupport));
	if (iAssignment == m_Settings.m_ExposureSetsForThickness.end())
		return "";
	return iAssignment->second;
}

void CEOSToolpathConversionSettings::ClearExposureSetsForThickness()
{
	m_Settings.m_ExposureSetsForThickness.clear();
}

std::string CEOSToolpathConversionSettings::ToJSON()
{
	return m_Settings.toJSON();
}
