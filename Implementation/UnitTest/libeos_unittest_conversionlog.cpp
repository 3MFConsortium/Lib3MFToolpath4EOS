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


Abstract: Tests of the conversion log and its JSON form.

*/

#include "libeos_unittests.hpp"
#include "libeos_eosconversionlog.hpp"

using namespace Lib3MF4EOS::Impl;
using Lib3MF4EOS::eEOSLogSeverity;

namespace LibEOSUnitTest {

	class CConversionLogTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "ConversionLog";
		}

		void registerTests() override
		{
			registerTest("Messages", "messages are kept in order and counted by severity", [this]() {
				CConversionLogData log;
				assertFalse(log.hasErrors());
				log.addMessage(eEOSLogSeverity::Info, "FIRST", LIBEOS_CONVERSIONLOG_NOLAYER, LIBEOS_CONVERSIONLOG_NOPART, "first message");
				log.addMessage(eEOSLogSeverity::Warning, "SECOND", 3, 7, "second message");
				log.addMessage(eEOSLogSeverity::Error, "THIRD", 4, 0, "third message");

				assertEqual((int64_t)log.getMessageCount(), 3);
				auto message = log.getMessage(1);
				assertEqualString(message.m_sCode, "SECOND");
				assertEqual(message.m_nLayerIndex, 3);
				assertEqual(message.m_nPartID, 7);
				assertEqual(log.getCountBySeverity(eEOSLogSeverity::Info), 1);
				assertEqual(log.getCountBySeverity(eEOSLogSeverity::Warning), 1);
				assertEqual(log.getCountBySeverity(eEOSLogSeverity::Error), 1);
				assertTrue(log.hasErrors());
				assertThrowsError([&]() { log.getMessage(3); }, LIB3MF4EOS_ERROR_INVALIDPARAM);
			});

			registerTest("CountedMessages", "a repeated counted message only increases its count", [this]() {
				CConversionLogData log;
				for (int nIndex = 0; nIndex < 3; nIndex++)
					log.addCountedMessage(eEOSLogSeverity::Warning, "REPEATED", 1, 2, "the same text");
				log.addCountedMessage(eEOSLogSeverity::Warning, "REPEATED", 2, 2, "the same text");

				assertEqual((int64_t)log.getMessageCount(), 2);
				assertEqual((int64_t)log.getMessage(0).m_nCount, 3);
				assertEqualString(log.getMessage(0).getDisplayText(), "the same text (3 times)");
				assertEqualString(log.getMessage(1).getDisplayText(), "the same text");
			});

			registerTest("Summary", "summary values keep their first insertion order", [this]() {
				CConversionLogData log;
				log.addSummaryValue("b", 2);
				log.addSummaryValue("a", 1);
				log.addSummaryValue("b", 3);
				log.setSummaryValue("a", 10);

				assertEqual((int64_t)log.getSummaryValue("b"), 5);
				assertEqual((int64_t)log.getSummaryValue("a"), 10);
				assertEqual((int64_t)log.getSummaryValue("missing"), 0);
				assertEqual((int64_t)log.getSummaryValueCount(), 2);
				assertEqualString(log.getSummaryValueByIndex(0).first, "b");
				assertEqualString(log.getSummaryValueByIndex(1).first, "a");
			});

			registerTest("JSON", "the JSON has the format version, sections, summary, counts and messages in this order", [this]() {
				CConversionLogData log;
				log.setSection("sdk", "{\"version\":\"2.26\"}");
				log.setSection("fingerprint", "{\"hash\":\"1\"}");
				log.setSection("fingerprint", "{\"hash\":\"2\"}");
				log.addSummaryValue("layersWritten", 4);
				log.addMessage(eEOSLogSeverity::Warning, "CODE", LIBEOS_CONVERSIONLOG_NOLAYER, 5, "text with \"quotes\"");

				std::string sJSON = log.toJSON();
				assertEqualString(sJSON,
					"{\"formatVersion\":1,\"hasErrors\":false,\"sdk\":{\"version\":\"2.26\"},\"fingerprint\":{\"hash\":\"2\"},"
					"\"summary\":{\"layersWritten\":4},\"messageCounts\":{\"info\":0,\"warning\":1,\"error\":0},"
					"\"messages\":[{\"severity\":\"warning\",\"code\":\"CODE\",\"layerIndex\":null,\"partId\":5,\"text\":\"text with \\\"quotes\\\"\",\"count\":1}]}");
			});
		}
	};

	PUnitTestGroup createConversionLogTests()
	{
		return std::make_shared<CConversionLogTests>();
	}

}
