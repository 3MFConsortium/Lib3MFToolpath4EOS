/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Settings for converting toolpaths into EOS tasks.

*/

#ifndef __LIBEOS_EOSTOOLPATHCONVERSIONSETTINGS
#define __LIBEOS_EOSTOOLPATHCONVERSIONSETTINGS

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

#include <map>
#include <string>
#include <utility>

namespace Lib3MF4EOS {
namespace Impl {

// The values of a conversion. Conversions keep their own copy.
struct SConversionSettings {
	bool m_bStrictMode;
	double m_dPowerDelay;
	double m_dMaxPowerStep;
	double m_dMaxSpeedStep;
	double m_dMaxFocusStep;
	double m_dMinSubVectorLength;
	Lib3MF4EOS::eEOSPowerRampMode m_ePowerRampMode;
	Lib3MF4EOS::eEOSHatchWriteMode m_eHatchWriteMode;
	double m_dJoinTolerance;
	double m_dHatchPowerDeadband;
	bool m_bJoinContours;
	bool m_bRefuseUnequalConstantHatch;
	bool m_bDefaultSkywriting;
	uint32_t m_nDefaultHatchExposureType;
	uint32_t m_nDefaultContourExposureType;
	bool m_bLayerOpening;
	double m_dLayerOpeningDelay;
	Lib3MF4EOS::eEOSDwellTimeUnit m_eDwellTimeUnit;
	Lib3MF4EOS::eEOSLayerZConvention m_eLayerZConvention;
	bool m_bAllowDefaultExposureSets;
	// Key: layer thickness in micrometers and whether it is for supports.
	std::map<std::pair<uint32_t, bool>, std::string> m_ExposureSetsForThickness;

	SConversionSettings();

	void validate() const;

	std::string toJSON() const;
};

class CEOSToolpathConversionSettings : public virtual IEOSToolpathConversionSettings, public virtual CBase {
private:
	SConversionSettings m_Settings;

public:
	CEOSToolpathConversionSettings();

	virtual ~CEOSToolpathConversionSettings();

	const SConversionSettings& getSettings() const;

	void ResetToDefaults() override;

	void SetStrictMode(const bool bValue) override;
	bool GetStrictMode() override;

	void SetPowerDelay(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetPowerDelay() override;

	void SetMaxPowerStep(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetMaxPowerStep() override;

	void SetMaxSpeedStep(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetMaxSpeedStep() override;

	void SetMaxFocusStep(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetMaxFocusStep() override;

	void SetMinSubVectorLength(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetMinSubVectorLength() override;

	void SetPowerRampMode(const Lib3MF4EOS::eEOSPowerRampMode eValue) override;
	Lib3MF4EOS::eEOSPowerRampMode GetPowerRampMode() override;

	void SetHatchWriteMode(const Lib3MF4EOS::eEOSHatchWriteMode eValue) override;
	Lib3MF4EOS::eEOSHatchWriteMode GetHatchWriteMode() override;

	void SetJoinTolerance(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetJoinTolerance() override;

	void SetHatchPowerDeadband(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetHatchPowerDeadband() override;

	void SetJoinContours(const bool bValue) override;
	bool GetJoinContours() override;

	void SetRefuseUnequalConstantHatch(const bool bValue) override;
	bool GetRefuseUnequalConstantHatch() override;

	void SetDefaultSkywriting(const bool bValue) override;
	bool GetDefaultSkywriting() override;

	void SetDefaultHatchExposureType(const Lib3MF4EOS_uint32 nValue) override;
	Lib3MF4EOS_uint32 GetDefaultHatchExposureType() override;

	void SetDefaultContourExposureType(const Lib3MF4EOS_uint32 nValue) override;
	Lib3MF4EOS_uint32 GetDefaultContourExposureType() override;

	void SetLayerOpening(const bool bValue) override;
	bool GetLayerOpening() override;

	void SetLayerOpeningDelay(const Lib3MF4EOS_double dValue) override;
	Lib3MF4EOS_double GetLayerOpeningDelay() override;

	void SetDwellTimeUnit(const Lib3MF4EOS::eEOSDwellTimeUnit eValue) override;
	Lib3MF4EOS::eEOSDwellTimeUnit GetDwellTimeUnit() override;

	void SetLayerZConvention(const Lib3MF4EOS::eEOSLayerZConvention eValue) override;
	Lib3MF4EOS::eEOSLayerZConvention GetLayerZConvention() override;

	void SetAllowDefaultExposureSets(const bool bValue) override;
	bool GetAllowDefaultExposureSets() override;

	void SetExposureSetForThickness(const Lib3MF4EOS_uint32 nThickness, const bool bIsSupport, const std::string& sName) override;
	std::string GetExposureSetForThickness(const Lib3MF4EOS_uint32 nThickness, const bool bIsSupport) override;
	void ClearExposureSetsForThickness() override;

	std::string ToJSON() override;
};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSTOOLPATHCONVERSIONSETTINGS
