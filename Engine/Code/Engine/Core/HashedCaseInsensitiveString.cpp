#include "HashedCaseInsensitiveString.hpp"
#include <cstring>

HashedCaseInsensitiveString::HashedCaseInsensitiveString()
	:m_caseIntactText(),m_lowerCaseHash(0)
{
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString(HashedCaseInsensitiveString const& copyFrom)
	: m_caseIntactText(copyFrom.m_caseIntactText)
	, m_lowerCaseHash(copyFrom.m_lowerCaseHash)
{
}


HashedCaseInsensitiveString::HashedCaseInsensitiveString(char const* text)
	: m_caseIntactText(text ? text : "")
	, m_lowerCaseHash(ComputeLowerCaseHash(text))
{
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString(std::string const& text)
	: m_caseIntactText(text)
	, m_lowerCaseHash(ComputeLowerCaseHash(text.c_str()))
{
}

unsigned int HashedCaseInsensitiveString::ComputeLowerCaseHash(char const* text)
{
	unsigned int hash = 0;
	for (char const* scan = text; *scan != '\0'; ++scan)
	{
		hash *= 31; hash += (unsigned int)tolower(*scan);
	}
	return hash;
}

//-----------------------------------------------------------------------------------------------
// operators
//-----------------------------------------------------------------------------------------------
void HashedCaseInsensitiveString::operator=(HashedCaseInsensitiveString const& assignFrom)
{
	m_caseIntactText = assignFrom.m_caseIntactText;
	m_lowerCaseHash = assignFrom.m_lowerCaseHash;
}

void HashedCaseInsensitiveString::operator=(char const* text)
{
	m_caseIntactText = text ? text : "";
	m_lowerCaseHash = ComputeLowerCaseHash(text);
}

void HashedCaseInsensitiveString::operator=(std::string const& text)
{
	m_caseIntactText = text;
	m_lowerCaseHash = ComputeLowerCaseHash(text.c_str());
}

//-----------------------------------------------------------------------------------------------
// Comparison operators
// Strategy: use hash as the fast primary discriminator.
//   - If hashes differ  → strings are definitely different (no false negatives with a good hash).
//   - If hashes match   → perform a full case-insensitive string compare to rule out collisions.
// This means the common "not equal" path is a single integer compare — very fast.
//-----------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator==(HashedCaseInsensitiveString const& compare) const
{
	if (m_lowerCaseHash != compare.m_lowerCaseHash)
		return false;
	return _stricmp(m_caseIntactText.c_str(), compare.m_caseIntactText.c_str()) == 0;
}

bool HashedCaseInsensitiveString::operator!=(HashedCaseInsensitiveString const& compare) const
{
	return !(*this == compare);
}

bool HashedCaseInsensitiveString::operator<(HashedCaseInsensitiveString const& compare) const
{
	// Primary sort key: hash value (fast integer compare for std::map ordering)
	if (m_lowerCaseHash != compare.m_lowerCaseHash)
		return m_lowerCaseHash < compare.m_lowerCaseHash;
	// Collision: fall back to case-insensitive lexicographic order for determinism
	return _stricmp(m_caseIntactText.c_str(), compare.m_caseIntactText.c_str()) < 0;
}

bool HashedCaseInsensitiveString::operator==(char const* text) const
{
	if (m_lowerCaseHash != ComputeLowerCaseHash(text))
		return false;
	return _stricmp(m_caseIntactText.c_str(), text ? text : "") == 0;
}

bool HashedCaseInsensitiveString::operator!=(char const* text) const
{
	return !(*this == text);
}

bool HashedCaseInsensitiveString::operator==(std::string const& text) const
{
	return *this == text.c_str();
}

bool HashedCaseInsensitiveString::operator!=(std::string const& text) const
{
	return !(*this == text);
}
