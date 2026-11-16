/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Minimal JSON writer for the conversion log and the conversion settings.

*/

#include "libeos_jsonwriter.hpp"

#include <cmath>
#include <cstdio>

using namespace Lib3MF4EOS::Impl;

std::string Lib3MF4EOS::Impl::jsonString(const std::string& sValue)
{
	std::string sResult;
	sResult.reserve(sValue.size() + 2);
	sResult += '"';
	for (char c : sValue) {
		switch (c) {
		case '"': sResult += "\\\""; break;
		case '\\': sResult += "\\\\"; break;
		case '\b': sResult += "\\b"; break;
		case '\f': sResult += "\\f"; break;
		case '\n': sResult += "\\n"; break;
		case '\r': sResult += "\\r"; break;
		case '\t': sResult += "\\t"; break;
		default:
			if ((unsigned char)c < 0x20) {
				char buffer[8];
				snprintf(buffer, sizeof(buffer), "\\u%04x", (unsigned int)(unsigned char)c);
				sResult += buffer;
			}
			else {
				sResult += c;
			}
		}
	}
	sResult += '"';
	return sResult;
}

std::string Lib3MF4EOS::Impl::jsonNumber(double dValue)
{
	if (!std::isfinite(dValue))
		return "null";

	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%.15g", dValue);
	return buffer;
}

std::string Lib3MF4EOS::Impl::jsonInteger(int64_t nValue)
{
	return std::to_string(nValue);
}

std::string Lib3MF4EOS::Impl::jsonUnsigned(uint64_t nValue)
{
	return std::to_string(nValue);
}

std::string Lib3MF4EOS::Impl::jsonBool(bool bValue)
{
	return bValue ? "true" : "false";
}

std::string Lib3MF4EOS::Impl::jsonNull()
{
	return "null";
}

void CJSONObject::addRaw(const std::string& sKey, const std::string& sJSONValue)
{
	m_Members.push_back(std::make_pair(sKey, sJSONValue));
}

void CJSONObject::addString(const std::string& sKey, const std::string& sValue)
{
	addRaw(sKey, jsonString(sValue));
}

void CJSONObject::addNumber(const std::string& sKey, double dValue)
{
	addRaw(sKey, jsonNumber(dValue));
}

void CJSONObject::addInteger(const std::string& sKey, int64_t nValue)
{
	addRaw(sKey, jsonInteger(nValue));
}

void CJSONObject::addUnsigned(const std::string& sKey, uint64_t nValue)
{
	addRaw(sKey, jsonUnsigned(nValue));
}

void CJSONObject::addBool(const std::string& sKey, bool bValue)
{
	addRaw(sKey, jsonBool(bValue));
}

void CJSONObject::addNull(const std::string& sKey)
{
	addRaw(sKey, jsonNull());
}

std::string CJSONObject::toString() const
{
	std::string sResult = "{";
	bool bFirst = true;
	for (auto& member : m_Members) {
		if (!bFirst)
			sResult += ",";
		bFirst = false;
		sResult += jsonString(member.first) + ":" + member.second;
	}
	sResult += "}";
	return sResult;
}

void CJSONArray::addRaw(const std::string& sJSONValue)
{
	m_Values.push_back(sJSONValue);
}

void CJSONArray::addString(const std::string& sValue)
{
	addRaw(jsonString(sValue));
}

std::string CJSONArray::toString() const
{
	std::string sResult = "[";
	bool bFirst = true;
	for (auto& sValue : m_Values) {
		if (!bFirst)
			sResult += ",";
		bFirst = false;
		sResult += sValue;
	}
	sResult += "]";
	return sResult;
}
