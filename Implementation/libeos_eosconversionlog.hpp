/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: The log of a toolpath conversion.

*/

#ifndef __LIBEOS_EOSCONVERSIONLOG
#define __LIBEOS_EOSCONVERSIONLOG

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace Lib3MF4EOS {
namespace Impl {

#define LIBEOS_CONVERSIONLOG_NOLAYER -1
#define LIBEOS_CONVERSIONLOG_NOPART 0

struct SConversionLogMessage {
	Lib3MF4EOS::eEOSLogSeverity m_eSeverity;
	std::string m_sCode;
	int32_t m_nLayerIndex;
	uint32_t m_nPartID;
	std::string m_sText;
	uint64_t m_nCount;

	// The text with the number of repetitions, if the message was repeated.
	std::string getDisplayText() const;
};

// Shared by the writer, the task creation future and the API objects; all methods are thread safe.
class CConversionLogData {
private:
	mutable std::mutex m_Mutex;

	std::vector<SConversionLogMessage> m_Messages;
	std::map<std::tuple<std::string, int32_t, uint32_t, std::string>, size_t> m_CountedMessageIndices;

	std::vector<std::pair<std::string, uint64_t>> m_Summary;
	std::map<std::string, size_t> m_SummaryIndices;

	// Top level JSON members other than messages and summary, in insertion order.
	std::vector<std::pair<std::string, std::string>> m_Sections;

	uint64_t& summaryValue(const std::string& sKey);

public:
	CConversionLogData();

	void addMessage(Lib3MF4EOS::eEOSLogSeverity eSeverity, const std::string& sCode, int32_t nLayerIndex, uint32_t nPartID, const std::string& sText);

	// A message with the same code, layer, part and text as an earlier one only increases its count.
	void addCountedMessage(Lib3MF4EOS::eEOSLogSeverity eSeverity, const std::string& sCode, int32_t nLayerIndex, uint32_t nPartID, const std::string& sText);

	void addSummaryValue(const std::string& sKey, uint64_t nIncrement);
	void setSummaryValue(const std::string& sKey, uint64_t nValue);

	// Replaces a section with the same key.
	void setSection(const std::string& sKey, const std::string& sJSONValue);

	size_t getMessageCount() const;
	SConversionLogMessage getMessage(size_t nIndex) const;
	uint32_t getCountBySeverity(Lib3MF4EOS::eEOSLogSeverity eSeverity) const;
	bool hasErrors() const;

	uint64_t getSummaryValue(const std::string& sKey) const;
	size_t getSummaryValueCount() const;
	std::pair<std::string, uint64_t> getSummaryValueByIndex(size_t nIndex) const;

	std::string toJSON() const;
};

typedef std::shared_ptr<CConversionLogData> PConversionLogData;

class CEOSConversionLog : public virtual IEOSConversionLog, public virtual CBase {
private:
	PConversionLogData m_pData;

public:
	CEOSConversionLog(PConversionLogData pData);

	virtual ~CEOSConversionLog();

	Lib3MF4EOS_uint32 GetLogMessageCount() override;

	void GetLogMessage(const Lib3MF4EOS_uint32 nIndex, Lib3MF4EOS::eEOSLogSeverity& eSeverity, std::string& sCode, Lib3MF4EOS_int32& nLayerIndex, Lib3MF4EOS_uint32& nPartID, std::string& sText) override;

	Lib3MF4EOS_uint32 GetCountBySeverity(const Lib3MF4EOS::eEOSLogSeverity eSeverity) override;

	bool HasErrors() override;

	Lib3MF4EOS_uint64 GetSummaryValue(const std::string& sKey) override;

	Lib3MF4EOS_uint32 GetSummaryValueCount() override;

	void GetSummaryValueByIndex(const Lib3MF4EOS_uint32 nIndex, std::string& sKey, Lib3MF4EOS_uint64& nValue) override;

	std::string ToJSON() override;
};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSCONVERSIONLOG
