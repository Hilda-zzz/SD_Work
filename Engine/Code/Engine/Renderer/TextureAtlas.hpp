#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"

struct AtlasMember 
{
	std::string m_imagePath="";
	AABB2 m_uvInAtlas=AABB2::ZERO_TO_ONE;         
	IntVec2 pixelOffset;    
	IntVec2 pixelSize;      
	IntVec2 originalSize;   
};

class TextureAtlas 
{
public:
	TextureAtlas();
	~TextureAtlas();

	bool CreateAtlas(const std::vector<std::string>& imagePaths);

	AtlasMember* FindMemberByName(const std::string& imageName) const;
	AABB2 GetUVByName(const std::string& imageName) const;

	Texture* GetTexture() const { return m_combinedTexture; }
	IntVec2 GetAtlasSize() const { return m_atlasSize; }

public:

private:
	std::string m_atlasName;
	std::vector<AtlasMember> m_members;
	std::unordered_map<std::string, size_t> m_nameToIndex;
	Texture* m_combinedTexture = nullptr;
	IntVec2 m_atlasSize;
};
