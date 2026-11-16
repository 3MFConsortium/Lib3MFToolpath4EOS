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


Abstract: Tests of the namespace registry: writing uses the current schemas.3mf.io namespaces,
          reading accepts the current namespace plus registered legacy ones, and a machine package
          written with a legacy namespace reads only after that namespace is registered.

*/

#include "libeos_unittests.hpp"
#include "libeos_namespaceregistry.hpp"
#include "Common/common_resourcepackage.hpp"
#include "Common/common_exportstream_native.hpp"
#include "Common/common_importstream_native.hpp"
#include "Common/common_utils.hpp"

using namespace Lib3MF4EOS::Impl;
using Lib3MF4EOS::eEOSNamespaceKind;

namespace LibEOSUnitTest {

	// A machine package namespace that a host registers as legacy.
	static const std::string LEGACY_MACHINEPACKAGE = "http://schemas.example.com/legacy/machinepackage/2022/08";
	// The eos.info job namespace, which was dropped completely and is never accepted unless registered.
	static const std::string LEGACY_EOSINFO_JOB = "http://eos.info/3mfutils/2026";

	class CNamespaceTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "Namespaces";
		}

		void registerTests() override
		{
			registerTest("CurrentNamespacesAreSchemas3mfIo", "writing uses the schemas.3mf.io namespaces", [this]() {
				CNamespaceRegistry registry;
				assertEqualString(registry.getNamespace(eEOSNamespaceKind::JobMetadata), "http://schemas.3mf.io/eos/job/2026/11");
				assertEqualString(registry.getNamespace(eEOSNamespaceKind::MaterialSetRelationship), "http://schemas.3mf.io/eos/materialset/2026/11");
				assertEqualString(registry.getNamespace(eEOSNamespaceKind::ToolpathProfile), "http://schemas.3mf.io/eos/toolpath/2026/11");
				assertEqualString(registry.getNamespace(eEOSNamespaceKind::MachinePackage), "http://schemas.3mf.io/eos/machinepackage/2026/11");
			});

			registerTest("RejectsEmptyAndCurrentNamespace", "registering an empty URI or the current namespace fails", [this]() {
				CNamespaceRegistry registry;
				assertTrue(registerThrows(registry, eEOSNamespaceKind::JobMetadata, ""), "empty URI");
				assertTrue(registerThrows(registry, eEOSNamespaceKind::JobMetadata, registry.getNamespace(eEOSNamespaceKind::JobMetadata)), "current namespace");
			});

			registerTest("IgnoresDuplicateRegistration", "registering the same legacy namespace twice keeps a single entry", [this]() {
				CNamespaceRegistry registry;
				registry.registerLegacyNamespace(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB);
				registry.registerLegacyNamespace(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB);
				auto accepted = registry.acceptedNamespaces(eEOSNamespaceKind::JobMetadata);
				assertEqual((int64_t)accepted.size(), 2, "current plus one legacy");
			});

			registerTest("EosInfoJobMetadataRejectedByDefault", "eos.info job metadata is not accepted unless registered", [this]() {
				CNamespaceRegistry registry;
				assertFalse(registry.isAccepted(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB), "rejected by default");
				registry.registerLegacyNamespace(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB);
				assertTrue(registry.isAccepted(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB), "accepted after registration");
			});

			registerTest("CurrentNamespaceWinsOverLegacy", "the accepted list has the current namespace first, so current beats legacy", [this]() {
				CNamespaceRegistry registry;
				registry.registerLegacyNamespace(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB);
				auto accepted = registry.acceptedNamespaces(eEOSNamespaceKind::JobMetadata);
				assertEqual((int64_t)accepted.size(), 2);
				// The profile reader takes the first accepted namespace that has a value, so the current one wins.
				assertEqualString(accepted[0], registry.getNamespace(eEOSNamespaceKind::JobMetadata), "current is first");
				assertEqualString(accepted[1], LEGACY_EOSINFO_JOB, "legacy follows");
			});

			registerTest("ClearLegacyNamespaces", "clearing removes all registered legacy namespaces", [this]() {
				CNamespaceRegistry registry;
				registry.registerLegacyNamespace(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB);
				registry.registerLegacyNamespace(eEOSNamespaceKind::MachinePackage, LEGACY_MACHINEPACKAGE);
				registry.clearLegacyNamespaces();
				assertFalse(registry.isAccepted(eEOSNamespaceKind::JobMetadata, LEGACY_EOSINFO_JOB));
				assertFalse(registry.isAccepted(eEOSNamespaceKind::MachinePackage, LEGACY_MACHINEPACKAGE));
			});

			registerTest("LegacyMachinePackageNeedsRegistration", "a legacy-namespace machine package reads only after registering the namespace", [this]() {
				std::string sFileName = writeMachinePackage(LEGACY_MACHINEPACKAGE);

				// Without the legacy namespace, only the current schemas.3mf.io namespace is accepted, so the read fails.
				CNamespaceRegistry withoutLegacy;
				assertTrue(readPackageThrows(sFileName, withoutLegacy), "fails without registration");

				// After registering the legacy namespace, the same package reads back.
				CNamespaceRegistry withLegacy;
				withLegacy.registerLegacyNamespace(eEOSNamespaceKind::MachinePackage, LEGACY_MACHINEPACKAGE);
				assertFalse(readPackageThrows(sFileName, withLegacy), "reads after registration");
			});
		}

	private:
		static bool registerThrows(CNamespaceRegistry& registry, eEOSNamespaceKind eKind, const std::string& sNamespace)
		{
			try {
				registry.registerLegacyNamespace(eKind, sNamespace);
			}
			catch (std::exception&) {
				return true;
			}
			return false;
		}

		// Writes a minimal machine package whose index xmlns is sNamespace, and returns the file name.
		static std::string writeMachinePackage(const std::string& sNamespace)
		{
			std::string sFileName = AMCCommon::CUtils::findTemporaryFileName(AMCCommon::CUtils::getTempFolder(), "libeos_ut_machinepackage", ".zip", 16);
			std::vector<uint8_t> data{ 1, 2, 3, 4 };
			auto pExportStream = std::make_shared<AMCCommon::CExportStream_Native>(sFileName);
			AMCCommon::CResourcePackageBuilder builder(pExportStream, sNamespace);
			builder.addEntry("config", "config.bin", "bin", "application/binary", data);
			builder.finalize();
			return sFileName;
		}

		static bool readPackageThrows(const std::string& sFileName, const CNamespaceRegistry& registry)
		{
			try {
				AMCCommon::CImportStream_Native importStream(sFileName);
				AMCCommon::CResourcePackage package(&importStream, "package",
					registry.acceptedNamespaces(eEOSNamespaceKind::MachinePackage));
				(void)package.getEntryCount();
			}
			catch (std::exception&) {
				return true;
			}
			return false;
		}
	};

	PUnitTestGroup createNamespaceTests()
	{
		return std::make_shared<CNamespaceTests>();
	}

}
