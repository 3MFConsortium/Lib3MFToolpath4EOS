/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: The toolpath profile parameters LibEOS writes to EOS, and the ones it cannot write:
parameters without an EOS equivalent, such as jumpspeed, and parameters it does not know.

*/

#ifndef __LIBEOS_TOOLPATHPROFILEPARAMETERS
#define __LIBEOS_TOOLPATHPROFILEPARAMETERS

#include "lib3mftoolpath4eos_interfaces.hpp"

#include <string>
#include <vector>

namespace Lib3MF4EOS {
namespace Impl {

struct SUnsupportedProfileParameter {
	// The parameter name, with its namespace URI in front if it has one.
	std::string m_sName;
	std::string m_sReason;
};

// Why a profile parameter cannot be written to EOS, or an empty string if it can. Parameters without a namespace
// belong to the 3MF toolpath extension; the eos parameters are taken from every accepted namespace.
std::string unsupportedProfileParameterReason(const std::string& sNameSpace, const std::string& sName, const std::vector<std::string>& acceptedNamespaces);

// The parameters of the profile that cannot be written to EOS, in the order of the profile.
std::vector<SUnsupportedProfileParameter> findUnsupportedProfileParameters(Lib3MF::PToolpathProfile pProfile, const std::vector<std::string>& acceptedNamespaces);

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_TOOLPATHPROFILEPARAMETERS
