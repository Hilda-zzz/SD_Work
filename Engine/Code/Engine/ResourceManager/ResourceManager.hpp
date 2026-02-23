#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "MeshResource.hpp"
#include "StaticMeshInfo.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class SpriteSheet;
struct IntVec2;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void Startup(Renderer* curRenderer);
	void Shutdown();

	// 3D
	MeshResource* CreateOrGetObjMeshFromMetaFile(std::string const& metaFilePath);

	// 2D 
	SpriteSheet* CreateOrGetSpriteSheet(std::string const& name, std::string const& texturePath, IntVec2 const& gridLayout);
	SpriteAnimDefinition* CreateOrGetSpriteAnim(std::string const& name, std::string const& sheetName,
		int startFrame, int endFrame, float framesPerSecond, SpriteAnimPlaybackType playbackType);

	SpriteSheet* GetSpriteSheet(std::string const& name) const;
	SpriteAnimDefinition* GetSpriteAnim(std::string const& name) const;

private:
	StaticMeshInfo ParseObjMetaFile(std::string const& metaFilePath);
	MeshResource* CreateObjMeshFromInfo(StaticMeshInfo const& meshInfo);

private:
	// 3D
	static std::vector<MeshResource*> s_meshResources;
	static std::unordered_map<std::string, MeshResource*> s_meshLookup; //mesh meta file path to resource

	// 2D 
	static std::unordered_map<std::string, SpriteSheet*> s_spriteSheetLookup;
	static std::vector<SpriteSheet*> s_spriteSheets;

	static std::unordered_map<std::string, SpriteAnimDefinition*> s_spriteAnimLookup;
	static std::vector<SpriteAnimDefinition*> s_spriteAnims;

	Renderer* m_curRenderer = nullptr;
};