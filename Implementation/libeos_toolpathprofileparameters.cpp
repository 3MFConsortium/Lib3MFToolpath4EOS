/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Implementation of the toolpath profile parameter check.

*/

#include "libeos_toolpathprofileparameters.hpp"

#include <algorithm>

namespace Lib3MF4EOS {
namespace Impl {

std::string unsupportedProfileParameterReason(const std::string& sNameSpace, const std::string& sName, const std::vector<std::string>& acceptedNamespaces)
{
	static const std::vector<std::string> toolpathParameters = { "laserpower", "laserspeed", "laserindex", "laserfocus", "prewaittime", "postwaittime" };
	static const std::vector<std::string> eosParameters = { "focus", "exposuretype", "skywriting", "exposuresetindex", "exposurestepindex",
		"pulsedwaveperiodtime", "pulsedwavepowerontime", "exposeddepth", "predelay", "postdelay", "beamshapeprofile" };

	if (sNameSpace.empty()) {
		if (std::find(toolpathParameters.begin(), toolpathParameters.end(), sName) != toolpathParameters.end())
			return std::string();
		if (sName == "jumpspeed")
			return "the EOS toolpath API has no command to set a jump speed";
		return "the 3MF toolpath extension parameter is unknown to LibEOS";
	}

	if (std::find(acceptedNamespaces.begin(), acceptedNamespaces.end(), sNameSpace) != acceptedNamespaces.end()) {
		if (std::find(eosParameters.begin(), eosParameters.end(), sName) != eosParameters.end())
			return std::string();
		return "the EOS parameter is unknown to LibEOS";
	}

	return "the namespace is unknown to LibEOS; register it as legacy toolpath profile namespace if it holds EOS parameters";
}

std::vector<SUnsupportedProfileParameter> findUnsupportedProfileParameters(Lib3MF::PToolpathProfile pProfile, const std::vector<std::string>& acceptedNamespaces)
{
	std::vector<SUnsupportedProfileParameter> parameters;
	uint32_t nParameterCount = pProfile->GetParameterCount();
	for (uint32_t nIndex = 0; nIndex < nParameterCount; nIndex++) {
		std::string sNameSpace = pProfile->GetParameterNameSpace(nIndex);
		std::string sName = pProfile->GetParameterName(nIndex);
		std::string sReason = unsupportedProfileParameterReason(sNameSpace, sName, acceptedNamespaces);
		if (!sReason.empty())
			parameters.push_back(SUnsupportedProfileParameter{ sNameSpace.empty() ? sName : (sNameSpace + ":" + sName), sReason });
	}
	return parameters;
}

} // namespace Impl
} // namespace Lib3MF4EOS
