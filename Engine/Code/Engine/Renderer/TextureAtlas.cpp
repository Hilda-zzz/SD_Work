#include "TextureAtlas.hpp"
#include "../Core/Image.hpp"


TextureAtlas::TextureAtlas()
{
}

TextureAtlas::~TextureAtlas()
{
}

bool TextureAtlas::CreateAtlas(const std::vector<std::string>& imagePaths)
{
	std::vector<AtlasMember> atlasMembers;

	for (int i = 0; i < (int)imagePaths.size(); i++)
	{
		AtlasMember member;
		member.m_imagePath = imagePaths[i];
		atlasMembers.push_back(member);
	}
	
	Image* atlasImage = nullptr;
	if (atlasMembers.size() > 0)
	{
		atlasImage =new Image(atlasMembers);
		return true;
	}

	// create texture by big image
	

	delete atlasImage;
	atlasImage = nullptr;

	return false;
}

AtlasMember* TextureAtlas::FindMemberByName(const std::string& imageName) const
{
	auto it = m_nameToIndex.find(imageName);
	return (it != m_nameToIndex.end()) ? const_cast<AtlasMember*>(&m_members[it->second]) : nullptr;
}

AABB2 TextureAtlas::GetUVByName(const std::string& imageName) const
{
	AtlasMember* member = FindMemberByName(imageName);
	if (member)
	{
		return member->m_uvInAtlas;
	}
	return AABB2::ZERO_TO_ONE;
}
