/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: The log of a toolpath conversion.

*/

#include "libeos_eosconversionlog.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_jsonwriter.hpp"

using namespace Lib3MF4EOS::Impl;

namespace {

	std::string severityName(Lib3MF4EOS::eEOSLogSeverity eSeverity)
	{
		switch (eSeverity) {
		case Lib3MF4EOS::eEOSLogSeverity::Info: return "info";
		case Lib3MF4EOS::eEOSLogSeverity::Warning: return "warning";
		case Lib3MF4EOS::eEOSLogSeverity::Error: return "error";
		default: return "unknown";
		}
	}

	void checkSeverity(Lib3MF4EOS::eEOSLogSeverity eSeverity)
	{
		switch (eSeverity) {
		case Lib3MF4EOS::eEOSLogSeverity::Info:
		case Lib3MF4EOS::eEOSLogSeverity::Warning:
		case Lib3MF4EOS::eEOSLogSeverity::Error:
			return;
		default:
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "unknown log severity: " + std::to_string((int32_t)eSeverity));
		}
	}

}

/*************************************************************************************************************************
 SConversionLogMessage
**************************************************************************************************************************/

std::string SConversionLogMessage::getDisplayText() const
{
	if (m_nCount > 1)
		return m_sText + " (" + std::to_string(m_nCount) + " times)";
	return m_sText;
}

/*************************************************************************************************************************
 CConversionLogData
**************************************************************************************************************************/

CConversionLogData::CConversionLogData()
{
}

uint64_t& CConversionLogData::summaryValue(const std::string& sKey)
{
	auto iIndex = m_SummaryIndices.find(sKey);
	if (iIndex != m_SummaryIndices.end())
		return m_Summary[iIndex->second].second;

	m_SummaryIndices.insert(std::make_pair(sKey, m_Summary.size()));
	m_Summary.push_back(std::make_pair(sKey, (uint64_t)0));
	return m_Summary.back().second;
}

void CConversionLogData::addMessage(Lib3MF4EOS::eEOSLogSeverity eSeverity, const std::string& sCode, int32_t nLayerIndex, uint32_t nPartID, const std::string& sText)
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	m_Messages.push_back(SConversionLogMessage{ eSeverity, sCode, nLayerIndex, nPartID, sText, 1 });
}

void CConversionLogData::addCountedMessage(Lib3MF4EOS::eEOSLogSeverity eSeverity, const std::string& sCode, int32_t nLayerIndex, uint32_t nPartID, const std::string& sText)
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	auto key = std::make_tuple(sCode, nLayerIndex, nPartID, sText);
	auto iIndex = m_CountedMessageIndices.find(key);
	if (iIndex != m_CountedMessageIndices.end()) {
		m_Messages[iIndex->second].m_nCount++;
		return;
	}

	m_CountedMessageIndices.insert(std::make_pair(key, m_Messages.size()));
	m_Messages.push_back(SConversionLogMessage{ eSeverity, sCode, nLayerIndex, nPartID, sText, 1 });
}

void CConversionLogData::addSummaryValue(const std::string& sKey, uint64_t nIncrement)
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	summaryValue(sKey) += nIncrement;
}

void CConversionLogData::setSummaryValue(const std::string& sKey, uint64_t nValue)
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	summaryValue(sKey) = nValue;
}

void CConversionLogData::setSection(const std::string& sKey, const std::string& sJSONValue)
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	for (auto& section : m_Sections) {
		if (section.first == sKey) {
			section.second = sJSONValue;
			return;
		}
	}
	m_Sections.push_back(std::make_pair(sKey, sJSONValue));
}

size_t CConversionLogData::getMessageCount() const
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	return m_Messages.size();
}

SConversionLogMessage CConversionLogData::getMessage(size_t nIndex) const
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	if (nIndex >= m_Messages.size())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "invalid log message index: " + std::to_string(nIndex));
	return m_Messages[nIndex];
}

uint32_t CConversionLogData::getCountBySeverity(Lib3MF4EOS::eEOSLogSeverity eSeverity) const
{
	checkSeverity(eSeverity);

	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	uint32_t nCount = 0;
	for (auto& message : m_Messages) {
		if (message.m_eSeverity == eSeverity)
			nCount++;
	}
	return nCount;
}

bool CConversionLogData::hasErrors() const
{
	return getCountBySeverity(Lib3MF4EOS::eEOSLogSeverity::Error) > 0;
}

uint64_t CConversionLogData::getSummaryValue(const std::string& sKey) const
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	auto iIndex = m_SummaryIndices.find(sKey);
	if (iIndex == m_SummaryIndices.end())
		return 0;
	return m_Summary[iIndex->second].second;
}

size_t CConversionLogData::getSummaryValueCount() const
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	return m_Summary.size();
}

std::pair<std::string, uint64_t> CConversionLogData::getSummaryValueByIndex(size_t nIndex) const
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);
	if (nIndex >= m_Summary.size())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "invalid summary value index: " + std::to_string(nIndex));
	return m_Summary[nIndex];
}

std::string CConversionLogData::toJSON() const
{
	std::lock_guard<std::mutex> lockGuard(m_Mutex);

	uint32_t nInfoCount = 0;
	uint32_t nWarningCount = 0;
	uint32_t nErrorCount = 0;
	CJSONArray messages;
	for (auto& message : m_Messages) {
		switch (message.m_eSeverity) {
		case Lib3MF4EOS::eEOSLogSeverity::Info: nInfoCount++; break;
		case Lib3MF4EOS::eEOSLogSeverity::Warning: nWarningCount++; break;
		case Lib3MF4EOS::eEOSLogSeverity::Error: nErrorCount++; break;
		default: break;
		}

		CJSONObject entry;
		entry.addString("severity", severityName(message.m_eSeverity));
		entry.addString("code", message.m_sCode);
		if (message.m_nLayerIndex != LIBEOS_CONVERSIONLOG_NOLAYER)
			entry.addInteger("layerIndex", message.m_nLayerIndex);
		else
			entry.addNull("layerIndex");
		if (message.m_nPartID != LIBEOS_CONVERSIONLOG_NOPART)
			entry.addUnsigned("partId", message.m_nPartID);
		else
			entry.addNull("partId");
		entry.addString("text", message.m_sText);
		entry.addUnsigned("count", message.m_nCount);
		messages.addRaw(entry.toString());
	}

	CJSONObject counts;
	counts.addUnsigned("info", nInfoCount);
	counts.addUnsigned("warning", nWarningCount);
	counts.addUnsigned("error", nErrorCount);

	CJSONObject summary;
	for (auto& value : m_Summary)
		summary.addUnsigned(value.first, value.second);

	CJSONObject json;
	json.addUnsigned("formatVersion", 1);
	json.addBool("hasErrors", nErrorCount > 0);
	for (auto& section : m_Sections)
		json.addRaw(section.first, section.second);
	json.addRaw("summary", summary.toString());
	json.addRaw("messageCounts", counts.toString());
	json.addRaw("messages", messages.toString());
	return json.toString();
}

/*************************************************************************************************************************
 CEOSConversionLog
**************************************************************************************************************************/

CEOSConversionLog::CEOSConversionLog(PConversionLogData pData)
	: m_pData(pData)
{
	if (pData.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
}

CEOSConversionLog::~CEOSConversionLog()
{
}

Lib3MF4EOS_uint32 CEOSConversionLog::GetLogMessageCount()
{
	return (Lib3MF4EOS_uint32)m_pData->getMessageCount();
}

void CEOSConversionLog::GetLogMessage(const Lib3MF4EOS_uint32 nIndex, Lib3MF4EOS::eEOSLogSeverity& eSeverity, std::string& sCode, Lib3MF4EOS_int32& nLayerIndex, Lib3MF4EOS_uint32& nPartID, std::string& sText)
{
	auto message = m_pData->getMessage(nIndex);
	eSeverity = message.m_eSeverity;
	sCode = message.m_sCode;
	nLayerIndex = message.m_nLayerIndex;
	nPartID = message.m_nPartID;
	sText = message.getDisplayText();
}

Lib3MF4EOS_uint32 CEOSConversionLog::GetCountBySeverity(const Lib3MF4EOS::eEOSLogSeverity eSeverity)
{
	return m_pData->getCountBySeverity(eSeverity);
}

bool CEOSConversionLog::HasErrors()
{
	return m_pData->hasErrors();
}

Lib3MF4EOS_uint64 CEOSConversionLog::GetSummaryValue(const std::string& sKey)
{
	return m_pData->getSummaryValue(sKey);
}

Lib3MF4EOS_uint32 CEOSConversionLog::GetSummaryValueCount()
{
	return (Lib3MF4EOS_uint32)m_pData->getSummaryValueCount();
}

void CEOSConversionLog::GetSummaryValueByIndex(const Lib3MF4EOS_uint32 nIndex, std::string& sKey, Lib3MF4EOS_uint64& nValue)
{
	auto value = m_pData->getSummaryValueByIndex(nIndex);
	sKey = value.first;
	nValue = value.second;
}

std::string CEOSConversionLog::ToJSON()
{
	return m_pData->toJSON();
}
