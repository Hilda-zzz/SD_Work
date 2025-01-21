#include "Engine/Core/XmlUtils.hpp"

int ParseXmlAttribute(XmlElement const* element, char const* attributeName, int defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		const char* value = attribute->Value();
		if (value != nullptr)
		{
			return atoi(value);
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

char ParseXmlAttribute(XmlElement const* element, char const* attributeName, char defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		const char* value = attribute->Value();
		if (value!= nullptr)
		{
			return value[0];
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

bool ParseXmlAttribute(XmlElement const* element, char const* attributeName, bool defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		const char* value = attribute->Value();
		if (value!= nullptr)
		{
			if (strcmp(value, "true") == 0 || strcmp(value, "True") == 0 || strcmp(value, "TRUE") == 0 || strcmp(value, "1") == 0) {
				return true;
			}
			if (strcmp(value, "false") == 0 || strcmp(value, "False") == 0 || strcmp(value, "FALSE") == 0 || strcmp(value, "0") == 0) {
				return false;
			}
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

float ParseXmlAttribute(XmlElement const* element, char const* attributeName, float defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		if (attribute->Value() != nullptr)
		{
			return (float)atof(attribute->Value());
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

Rgba8 ParseXmlAttribute(XmlElement const* element, char const* attributeName, Rgba8 const& defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		if (attribute->Value() != nullptr)
		{
			Rgba8 color;
			color.SetFromText(attribute->Value());
			return color;
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

Vec2 ParseXmlAttribute(XmlElement const* element, char const* attributeName, Vec2 const& defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		if (attribute->Value() != nullptr)
		{
			Vec2 vec;
			vec.SetFromText(attribute->Value());
			return vec;
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

IntVec2 ParseXmlAttribute(XmlElement const* element, char const* attributeName, IntVec2 const& defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		if (attribute->Value() != nullptr)
		{
			IntVec2 vec;
			vec.SetFromText(attribute->Value());
			return vec;
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

std::string ParseXmlAttribute(XmlElement const* element, char const* attributeName, std::string const& defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		if (attribute->Value() != nullptr)
		{
			return std::string(attribute->Value());
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}

Strings ParseXmlAttribute(XmlElement const* element, char const* attributeName, Strings const& defaultValues)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		const char* value = attribute->Value();
		if (value != nullptr)
		{
			return SplitStringOnDelimiter(value,',');
		}
		else
		{
			return defaultValues;
		}
	}
	return defaultValues;
}

std::string ParseXmlAttribute(XmlElement const* element, char const* attributeName, char const* defaultValue)
{
	XmlAttribute const* attribute = element->FindAttribute(attributeName);
	if (attribute != nullptr)
	{
		const char* value = attribute->Value();
		if (value != nullptr)
		{
			return std::string(value);
		}
		else
		{
			return defaultValue;
		}
	}
	return defaultValue;
}
