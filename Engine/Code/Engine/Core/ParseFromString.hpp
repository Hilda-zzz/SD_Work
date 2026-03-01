#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <cstdlib>
#include <string>
#include "ErrorWarningAssert.hpp"
//-----------------------------------------------------------------------------------------------
// ParseFromString<T>
// Internal helper: parse a std::string into type T.
// Add a specialisation for each type your engine needs to support.
// Returns defaultValue if parsing fails or no specialisation exists.
//-----------------------------------------------------------------------------------------------
template<typename T>
T ParseFromString(std::string const& str, T const& defaultValue)
{
	// 没有对应特化 -> 无法解析，返回默认值（比如 Vec2* 的情况）
	(void)str;
	(void)defaultValue;
	ERROR_AND_DIE("ParseFromString: no specialisation for this type");
}

template<> bool        ParseFromString(std::string const& str, bool        const& defaultValue);
template<> int         ParseFromString(std::string const& str, int         const& defaultValue);
template<> float       ParseFromString(std::string const& str, float       const& defaultValue);
template<> std::string ParseFromString(std::string const& str, std::string const& defaultValue);
template<> Rgba8       ParseFromString(std::string const& str, Rgba8       const& defaultValue);
template<> Vec2        ParseFromString(std::string const& str, Vec2        const& defaultValue);
template<> IntVec2     ParseFromString(std::string const& str, IntVec2     const& defaultValue);
