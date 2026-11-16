/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Registry of the XML namespaces LibEOS reads and writes.

Writing always uses the current schemas.3mf.io namespaces. Reading accepts the
current namespace plus any legacy namespaces registered from the outside.

*/

#ifndef __LIBEOS_NAMESPACEREGISTRY
#define __LIBEOS_NAMESPACEREGISTRY

#include "lib3mftoolpath4eos_types.hpp"

#include <string>
#include <vector>

// Current namespaces. Writing always uses these.
#define LIBEOS_NAMESPACE_JOBMETADATA "http://schemas.3mf.io/eos/job/2026/11"
#define LIBEOS_NAMESPACE_MATERIALSETRELATIONSHIP "http://schemas.3mf.io/eos/materialset/2026/11"
#define LIBEOS_NAMESPACE_TOOLPATHPROFILE "http://schemas.3mf.io/eos/toolpath/2026/11"
#define LIBEOS_NAMESPACE_MACHINEPACKAGE "http://schemas.3mf.io/eos/machinepackage/2026/11"

namespace Lib3MF4EOS {
namespace Impl {

// A copyable snapshot of the namespaces LibEOS reads and writes. A context owns one and hands
// copies to the futures and loaders it creates, so a later registration never races with
// background work.
class CNamespaceRegistry {
private:
	std::vector<std::string> m_LegacyJobMetadata;
	std::vector<std::string> m_LegacyMaterialSetRelationship;
	std::vector<std::string> m_LegacyToolpathProfile;
	std::vector<std::string> m_LegacyMachinePackage;

	std::vector<std::string>& legacyList(Lib3MF4EOS::eEOSNamespaceKind eKind);
	const std::vector<std::string>& legacyList(Lib3MF4EOS::eEOSNamespaceKind eKind) const;

public:
	// The current namespace of a kind, which is always used for writing.
	static std::string currentNamespace(Lib3MF4EOS::eEOSNamespaceKind eKind);

	std::string getNamespace(Lib3MF4EOS::eEOSNamespaceKind eKind) const { return currentNamespace(eKind); }

	// Adds a legacy namespace accepted when reading. Fails with INVALIDPARAM on an empty URI or
	// the current namespace; a namespace already registered is ignored.
	void registerLegacyNamespace(Lib3MF4EOS::eEOSNamespaceKind eKind, const std::string& sNamespace);

	void clearLegacyNamespaces();

	// True if sNamespace is the current namespace or a registered legacy namespace of the kind.
	bool isAccepted(Lib3MF4EOS::eEOSNamespaceKind eKind, const std::string& sNamespace) const;

	// The namespaces accepted when reading, the current namespace first, then the legacy ones in
	// registration order.
	std::vector<std::string> acceptedNamespaces(Lib3MF4EOS::eEOSNamespaceKind eKind) const;
};

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_NAMESPACEREGISTRY
