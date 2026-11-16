/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Minimal JSON writer for the conversion log and the conversion settings.

*/

#ifndef __LIBEOS_JSONWRITER
#define __LIBEOS_JSONWRITER

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace Lib3MF4EOS {
namespace Impl {

std::string jsonString(const std::string& sValue);
// Non-finite values are written as null, which JSON requires.
std::string jsonNumber(double dValue);
std::string jsonInteger(int64_t nValue);
std::string jsonUnsigned(uint64_t nValue);
std::string jsonBool(bool bValue);
std::string jsonNull();

// Members keep their insertion order. Values are JSON text.
class CJSONObject {
private:
	std::vector<std::pair<std::string, std::string>> m_Members;

public:
	void addRaw(const std::string& sKey, const std::string& sJSONValue);
	void addString(const std::string& sKey, const std::string& sValue);
	void addNumber(const std::string& sKey, double dValue);
	void addInteger(const std::string& sKey, int64_t nValue);
	void addUnsigned(const std::string& sKey, uint64_t nValue);
	void addBool(const std::string& sKey, bool bValue);
	void addNull(const std::string& sKey);

	std::string toString() const;
};

class CJSONArray {
private:
	std::vector<std::string> m_Values;

public:
	void addRaw(const std::string& sJSONValue);
	void addString(const std::string& sValue);

	std::string toString() const;
};

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_JSONWRITER
