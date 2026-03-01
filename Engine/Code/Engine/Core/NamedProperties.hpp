#pragma once
#include <string>
#include <map>
#include "HashedCaseInsensitiveString.hpp"

#include "Engine/Core/XmlUtils.hpp"
#include "ErrorWarningAssert.hpp"
#include "ParseFromString.hpp"

class TypedPropertyBase
{
public:
	virtual ~TypedPropertyBase() {}
};

template<typename T>
class TypeProperty :public TypedPropertyBase
{
public:
	TypeProperty(T const& value)
		:m_data(value)
	{}
	T m_data;
};

class NamedProperties
{
public:
	NamedProperties() = default;
	~NamedProperties();

	//-------------------------------------------------------------------------
	template<typename T>
	void Set(HashedCaseInsensitiveString const& key, T const& value)
	{
		auto it = m_properties.find(key);
		if (it != m_properties.end())
		{
			delete it->second;
			it->second = new TypeProperty<T>(value);
		}
		else
		{
			m_properties[key] = new TypeProperty<T>(value);
		}
	}

	void Set(HashedCaseInsensitiveString const& key, char const* value)
	{
		Set(key, std::string(value ? value : ""));
	}
	//-------------------------------------------------------------------------
	template<typename T>
	void SetValue(std::string const& keyName, T const& value)
	{
		Set(HashedCaseInsensitiveString(keyName), value);
	}

	void SetValue(std::string const& keyName, char const* value)
	{
		Set(HashedCaseInsensitiveString(keyName), std::string(value ? value : ""));
	}

	//-------------------------------------------------------------------------
	template<typename T>
	T Get(HashedCaseInsensitiveString const& key, T const& defaultValue) const
	{
		auto it = m_properties.find(key);
		if (it == m_properties.end())
			return defaultValue;

		TypeProperty<T>* typed = dynamic_cast<TypeProperty<T>*>(it->second);
		// correct
		if (typed)
			return typed->m_data;

		// try convert string to specific type
		TypeProperty<std::string>* storedAsString = dynamic_cast<TypeProperty<std::string>*>(it->second);
		if (storedAsString)
			return ParseFromString<T>(storedAsString->m_data, defaultValue);

		ERROR_AND_DIE("NamedProperties::Get — type mismatch"); 
		return defaultValue;
	}

	// char const* default → return type is std::string  (HCIS key)
	std::string Get(HashedCaseInsensitiveString const& key, char const* defaultValue) const
	{
		return Get(key, std::string(defaultValue ? defaultValue : ""));
	}

	//-------------------------------------------------------------------------
	template<typename T>
	T GetValue(std::string const& keyName, T const& defaultValue) const
	{
		return Get(HashedCaseInsensitiveString(keyName), defaultValue);
	}

	std::string GetValue(std::string const& keyName, char const* defaultValue) const
	{
		return Get(HashedCaseInsensitiveString(keyName), std::string(defaultValue ? defaultValue : ""));
	}

	//-------------------------------------------------------------------------------------------
	// Utility
	//-------------------------------------------------------------------------------------------
	bool KeyExists(HashedCaseInsensitiveString const& key) const { return m_properties.find(key) != m_properties.end(); }
	bool KeyExists(std::string const& keyName)             const { return KeyExists(HashedCaseInsensitiveString(keyName)); }
	bool KeyExists(char const* keyName)                    const { return KeyExists(HashedCaseInsensitiveString(keyName)); }

	void RemoveValue(HashedCaseInsensitiveString const& key);
	void RemoveValue(std::string const& keyName) { RemoveValue(HashedCaseInsensitiveString(keyName)); }
	void RemoveValue(char const* keyName) { RemoveValue(HashedCaseInsensitiveString(keyName)); }

	void Clear();

	//-------------------------------------------------------------------------------------------
	// Backward compatibility with NamedStrings
	// Stores every XML attribute as a std::string value, keyed by attribute name.
	//-------------------------------------------------------------------------------------------
	void PopulateFromXmlElementAttributes(XmlElement const& element);
private:
	std::map<HashedCaseInsensitiveString, TypedPropertyBase*> m_properties;
};