/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Implementation of CNamespaceRegistry.

*/

#include "libeos_namespaceregistry.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

#include <algorithm>

using namespace Lib3MF4EOS::Impl;

std::vector<std::string>& CNamespaceRegistry::legacyList(Lib3MF4EOS::eEOSNamespaceKind eKind)
{
	switch (eKind) {
	case Lib3MF4EOS::eEOSNamespaceKind::JobMetadata: return m_LegacyJobMetadata;
	case Lib3MF4EOS::eEOSNamespaceKind::MaterialSetRelationship: return m_LegacyMaterialSetRelationship;
	case Lib3MF4EOS::eEOSNamespaceKind::ToolpathProfile: return m_LegacyToolpathProfile;
	case Lib3MF4EOS::eEOSNamespaceKind::MachinePackage: return m_LegacyMachinePackage;
	default:
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "unknown namespace kind");
	}
}

const std::vector<std::string>& CNamespaceRegistry::legacyList(Lib3MF4EOS::eEOSNamespaceKind eKind) const
{
	switch (eKind) {
	case Lib3MF4EOS::eEOSNamespaceKind::JobMetadata: return m_LegacyJobMetadata;
	case Lib3MF4EOS::eEOSNamespaceKind::MaterialSetRelationship: return m_LegacyMaterialSetRelationship;
	case Lib3MF4EOS::eEOSNamespaceKind::ToolpathProfile: return m_LegacyToolpathProfile;
	case Lib3MF4EOS::eEOSNamespaceKind::MachinePackage: return m_LegacyMachinePackage;
	default:
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "unknown namespace kind");
	}
}

std::string CNamespaceRegistry::currentNamespace(Lib3MF4EOS::eEOSNamespaceKind eKind)
{
	switch (eKind) {
	case Lib3MF4EOS::eEOSNamespaceKind::JobMetadata: return LIBEOS_NAMESPACE_JOBMETADATA;
	case Lib3MF4EOS::eEOSNamespaceKind::MaterialSetRelationship: return LIBEOS_NAMESPACE_MATERIALSETRELATIONSHIP;
	case Lib3MF4EOS::eEOSNamespaceKind::ToolpathProfile: return LIBEOS_NAMESPACE_TOOLPATHPROFILE;
	case Lib3MF4EOS::eEOSNamespaceKind::MachinePackage: return LIBEOS_NAMESPACE_MACHINEPACKAGE;
	default:
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "unknown namespace kind");
	}
}

void CNamespaceRegistry::registerLegacyNamespace(Lib3MF4EOS::eEOSNamespaceKind eKind, const std::string& sNamespace)
{
	if (sNamespace.empty())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "legacy namespace is empty");
	if (sNamespace == currentNamespace(eKind))
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "legacy namespace is the current namespace: " + sNamespace);

	auto& list = legacyList(eKind);
	if (std::find(list.begin(), list.end(), sNamespace) == list.end())
		list.push_back(sNamespace);
}

void CNamespaceRegistry::clearLegacyNamespaces()
{
	m_LegacyJobMetadata.clear();
	m_LegacyMaterialSetRelationship.clear();
	m_LegacyToolpathProfile.clear();
	m_LegacyMachinePackage.clear();
}

bool CNamespaceRegistry::isAccepted(Lib3MF4EOS::eEOSNamespaceKind eKind, const std::string& sNamespace) const
{
	if (sNamespace == currentNamespace(eKind))
		return true;
	const auto& list = legacyList(eKind);
	return std::find(list.begin(), list.end(), sNamespace) != list.end();
}

std::vector<std::string> CNamespaceRegistry::acceptedNamespaces(Lib3MF4EOS::eEOSNamespaceKind eKind) const
{
	std::vector<std::string> result;
	result.push_back(currentNamespace(eKind));
	const auto& list = legacyList(eKind);
	result.insert(result.end(), list.begin(), list.end());
	return result;
}
