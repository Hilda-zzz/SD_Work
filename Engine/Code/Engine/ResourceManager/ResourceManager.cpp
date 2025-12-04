#include "ResourceManager.hpp"
#include "../Core/XmlUtils.hpp"
#include "../Core/ErrorWarningAssert.hpp"
#include "MeshResource.hpp"
#include "Engine/Renderer/Renderer.hpp"


std::unordered_map<std::string, MeshResource*> ResourceManager::s_meshLookup;
std::vector<MeshResource*> ResourceManager::s_meshResources;

std::unordered_map<std::string, SpriteSheet*> ResourceManager::s_spriteSheetLookup;
std::vector<SpriteSheet*> ResourceManager::s_spriteSheets;

std::unordered_map<std::string, SpriteAnimDefinition*> ResourceManager::s_spriteAnimLookup;
std::vector<SpriteAnimDefinition*> ResourceManager::s_spriteAnims;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::Startup(Renderer* curRenderer)
{
	m_curRenderer = curRenderer;
}

void ResourceManager::Shutdown()
{
	for (MeshResource* mesh : s_meshResources)
	{
		delete mesh;
		mesh = nullptr;
	}

	for (SpriteAnimDefinition* anim : s_spriteAnims)
	{
		delete anim;
	}
	s_spriteAnims.clear();
	s_spriteAnimLookup.clear();

	for (SpriteSheet* sheet : s_spriteSheets)
	{
		delete sheet;
	}
	s_spriteSheets.clear();
	s_spriteSheetLookup.clear();
}

MeshResource* ResourceManager::CreateOrGetObjMeshFromMetaFile(std::string const& metaFilePath)
{
	// look up first
	auto meshResource = s_meshLookup.find(metaFilePath);
	if (meshResource != s_meshLookup.end())
	{
		return meshResource->second;
	}
	// if not exist, create it
	StaticMeshInfo meshInfo = ParseObjMetaFile(metaFilePath);
	return CreateObjMeshFromInfo(meshInfo);
}

SpriteSheet* ResourceManager::CreateOrGetSpriteSheet(std::string const& name, std::string const& texturePath, IntVec2 const& gridLayout)
{
	auto it = s_spriteSheetLookup.find(name);
	if (it != s_spriteSheetLookup.end())
	{
		return it->second;
	}

	Texture* texture = m_curRenderer->CreateOrGetTextureFromFile(texturePath.c_str());
	GUARANTEE_OR_DIE(texture, Stringf("Failed to load texture: %s", texturePath.c_str()));

	SpriteSheet* sheet = new SpriteSheet(*texture, gridLayout);
	s_spriteSheets.push_back(sheet);
	s_spriteSheetLookup[name] = sheet;

	return sheet;
}

SpriteAnimDefinition* ResourceManager::CreateOrGetSpriteAnim(std::string const& name, std::string const& sheetName, int startFrame, int endFrame, float framesPerSecond, SpriteAnimPlaybackType playbackType)
{
	// 检查是否已存在
	auto it = s_spriteAnimLookup.find(name);
	if (it != s_spriteAnimLookup.end())
	{
		return it->second;
	}

	// 获取 SpriteSheet
	SpriteSheet* sheet = GetSpriteSheet(sheetName);
	GUARANTEE_OR_DIE(sheet, Stringf("SpriteSheet not found: %s", sheetName.c_str()));

	// 创建动画
	SpriteAnimDefinition* anim = new SpriteAnimDefinition(*sheet, startFrame, endFrame, framesPerSecond, playbackType);
	s_spriteAnims.push_back(anim);
	s_spriteAnimLookup[name] = anim;

	return anim;
}

SpriteSheet* ResourceManager::GetSpriteSheet(std::string const& name) const
{
	auto it = s_spriteSheetLookup.find(name);
	if (it != s_spriteSheetLookup.end())
	{
		return it->second;
	}
	return nullptr;
}

SpriteAnimDefinition* ResourceManager::GetSpriteAnim(std::string const& name) const
{
	auto it = s_spriteAnimLookup.find(name);
	if (it != s_spriteAnimLookup.end())
	{
		return it->second;
	}
	return nullptr;
}

StaticMeshInfo ResourceManager::ParseObjMetaFile(std::string const& metaFilePath)
{
	StaticMeshInfo meshInfo;
	meshInfo.m_metaFilePath = metaFilePath;

	XmlDocument metaXml;
	XmlResult result = metaXml.LoadFile(metaFilePath.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required meta file \"%s\"", metaFilePath.c_str()));

	XmlElement* rootElement = metaXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Failed to find meta file root element");
	GUARANTEE_OR_DIE(std::string(rootElement->Name()) == "MeshMeta", Stringf("Root element in %s was <%s>, must be <MeshMeta>!", metaFilePath.c_str(), rootElement->Name()));

	// Parse MeshInfo section
	XmlElement const* meshInfoElement = rootElement->FirstChildElement("MeshInfo");
	if (meshInfoElement)
	{
		std::string objFilePath = "";
		objFilePath = ParseXmlAttribute(meshInfoElement, "ObjFilePath", objFilePath);
		meshInfo.m_objFilePath = objFilePath;

		std::string shaderName = "";
		shaderName = ParseXmlAttribute(meshInfoElement, "ShaderName", shaderName);
		meshInfo.m_shaderName = shaderName;

		float unitsPerMeter = 1.0f;
		unitsPerMeter = ParseXmlAttribute(meshInfoElement, "UnitsPerMeter", unitsPerMeter);
		meshInfo.m_unitsPerMeter = unitsPerMeter;

		meshInfo.m_useIndex = ParseXmlAttribute(meshInfoElement, "UseIndex", false);
	}

	// Parse Textures section
	XmlElement const* texturesElement = rootElement->FirstChildElement("Textures");
	if (texturesElement)
	{
		std::string diffuseTexture = "";
		diffuseTexture = ParseXmlAttribute(texturesElement, "DiffuseTexture", diffuseTexture);
		meshInfo.m_diffuseTexPath = diffuseTexture;

		std::string normalTexture = "";
		normalTexture = ParseXmlAttribute(texturesElement, "NormalTexture", normalTexture);
		meshInfo.m_normalTexPath = normalTexture;

		std::string specularTexture = "";
		specularTexture = ParseXmlAttribute(texturesElement, "SpecularTexture", specularTexture);
		meshInfo.m_sgeTexPath = specularTexture;
	}

	// Parse CoordinateSystem section
	XmlElement const* coordSystemElement = rootElement->FirstChildElement("CoordinateSystem");
	if (coordSystemElement)
	{
		XmlElement const* forwardElement = coordSystemElement->FirstChildElement("Forward");
		if (forwardElement)
		{
			Vec3 fwd = Vec3(1.0f, 0.0f, 0.0f);
			fwd = ParseXmlAttribute(forwardElement, "xyz", fwd);
			meshInfo.m_fwd = fwd;
		}

		XmlElement const* leftElement = coordSystemElement->FirstChildElement("Left");
		if (leftElement)
		{
			Vec3 left = Vec3(0.0f, 1.0f, 0.0f);
			left = ParseXmlAttribute(leftElement, "xyz", left);
			meshInfo.m_left = left;
		}

		XmlElement const* upElement = coordSystemElement->FirstChildElement("Up");
		if (upElement)
		{
			Vec3 up = Vec3(0.0f, 0.0f, 1.0f);
			up = ParseXmlAttribute(upElement, "xyz", up);
			meshInfo.m_up = up;
		}
	}

	return meshInfo;
}

MeshResource* ResourceManager::CreateObjMeshFromInfo(StaticMeshInfo const& meshInfo)
{
	MeshResource* mesh = new MeshResource(meshInfo,m_curRenderer);
	// add to vector and map
	s_meshResources.push_back(mesh);
	s_meshLookup[meshInfo.m_metaFilePath] = mesh;

	return mesh;
}
