#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* attribute = element.FirstAttribute();
	while (attribute!=nullptr)
	{
		SetValue(attribute->Name(), attribute->Value());
		attribute = attribute->Next();
	}
}

void NamedStrings::DebugPrintContents()
{
	for (auto pair : m_keyValuePairs)
	{
		std::string logString = pair.first +"="+ pair.second+"\n";
		DebuggerPrintf(logString.c_str());
	}
}

void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	//std::map<std::string, std::string>::const_iterator found;
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return found->second;
}

bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	if (strcmp(found->second.c_str(), "True") || strcmp(found->second.c_str(), "TRUE") || strcmp(found->second.c_str(), "1") || strcmp(found->second.c_str(), "true"))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	else
	{
		return atoi(found->second.c_str());
	}
}

float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	else
	{
		return (float)atof(found->second.c_str());
	}
}

std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	else
	{
		return found->second;
	}
}

Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	else
	{
		Rgba8 color;
		color.SetFromText(found->second.c_str());
		return color;
	}
}

Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	else
	{
		Vec2 vec;
		vec.SetFromText(found->second.c_str());
		return vec;
	}
}

IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	else
	{
		IntVec2 vec;
		vec.SetFromText(found->second.c_str());
		return vec;
	}
}
