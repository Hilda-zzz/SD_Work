#include "Engine/Core/ParseFromString.hpp"

template<>
bool ParseFromString(std::string const& str, bool const& defaultValue)
{
	if (str == "true" || str == "True" || str == "TRUE" || str == "1")  return true;
	if (str == "false" || str == "False" || str == "FALSE" || str == "0")  return false;
	return defaultValue;
}

template<>
int ParseFromString(std::string const& str, int const& defaultValue)
{
	if (str.empty()) return defaultValue;
	return atoi(str.c_str());
}

template<>
float ParseFromString(std::string const& str, float const& defaultValue)
{
	if (str.empty()) return defaultValue;
	return (float)atof(str.c_str());
}

template<>
std::string ParseFromString(std::string const& str, std::string const& defaultValue)
{
	return str;
}

template<>
Rgba8 ParseFromString(std::string const& str, Rgba8 const& defaultValue)
{
	// 复用你引擎里 Rgba8 已有的字符串构造逻辑
	Rgba8 result = defaultValue;
	result.SetFromText(str.c_str());
	return result;
}

template<>
Vec2 ParseFromString(std::string const& str, Vec2 const& defaultValue)
{
	Vec2 result = defaultValue;
	result.SetFromText(str.c_str());
	return result;
}

template<>
IntVec2 ParseFromString(std::string const& str, IntVec2 const& defaultValue)
{
	IntVec2 result = defaultValue;
	result.SetFromText(str.c_str());
	return result;
}