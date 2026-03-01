#include "NamedProperties.hpp"
#include <ThirdParty/TinyXML2/tinyxml2.h>


NamedProperties::~NamedProperties()
{
	Clear();
}

void NamedProperties::RemoveValue(HashedCaseInsensitiveString const& key)
{
	auto it = m_properties.find(key);
	if (it != m_properties.end())
	{
		delete it->second;
		m_properties.erase(it);
	}
}

void NamedProperties::Clear()
{
	for (auto& pair : m_properties)
		delete pair.second;

	m_properties.clear();
}

void NamedProperties::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* attribute = element.FirstAttribute();
	while (attribute != nullptr)
	{
		SetValue(attribute->Name(), attribute->Value());
		attribute = attribute->Next();
	}
}
