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

Abstract: Tests of the resource package (ZIP) round trip.

*/



#include "libeos_unittests.hpp"
#include "Common/common_resourcepackage.hpp"
#include "Common/common_exportstream_native.hpp"
#include "Common/common_importstream_native.hpp"
#include "Common/common_utils.hpp"

namespace LibEOSUnitTest {

	class CResourcePackageTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "ResourcePackage";
		}

		void registerTests() override
		{
			registerTest("LargeEntryRoundTrip", "an entry larger than one ZIP write chunk (1 MiB) reads back unchanged", [this]() {
				std::vector<uint8_t> data(2621443);
				for (size_t nIndex = 0; nIndex < data.size(); nIndex++)
					data[nIndex] = (uint8_t)((nIndex * 2654435761u) >> 13);

				std::string sNamespace = "http://example.com/libeos/unittest/package";
				std::string sFileName = AMCCommon::CUtils::findTemporaryFileName(AMCCommon::CUtils::getTempFolder(), "libeos_ut_package", ".zip", 16);
				{
					auto pExportStream = std::make_shared<AMCCommon::CExportStream_Native>(sFileName);
					AMCCommon::CResourcePackageBuilder builder(pExportStream, sNamespace);
					builder.addEntry("large", "large.bin", "bin", "application/binary", data);
					builder.finalize();
				}

				AMCCommon::CImportStream_Native importStream(sFileName);
				AMCCommon::CResourcePackage package(&importStream, "package", std::vector<std::string>{ sNamespace });
				std::vector<uint8_t> readBack;
				package.readEntry("large", readBack);
				assertEqual((int64_t)readBack.size(), (int64_t)data.size(), "size");
				assertTrue(readBack == data, "content");
			});
		}
	};

	PUnitTestGroup createResourcePackageTests()
	{
		return std::make_shared<CResourcePackageTests>();
	}

}
