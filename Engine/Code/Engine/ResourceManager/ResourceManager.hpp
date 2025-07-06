#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "MeshResource.hpp"
#include "StaticMeshInfo.hpp"

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void Startup(Renderer* curRenderer);
	void Shutdown();

	MeshResource* CreateOrGetObjMeshFromMetaFile(std::string const& metaFilePath);

private:
	StaticMeshInfo ParseObjMetaFile(std::string const& metaFilePath);
	MeshResource* CreateObjMeshFromInfo(StaticMeshInfo const& meshInfo);

private:
	static std::vector<MeshResource*> s_meshResources;
	static std::unordered_map<std::string, MeshResource*> s_meshLookup; //mesh meta file path to resource

	Renderer* m_curRenderer = nullptr;
};