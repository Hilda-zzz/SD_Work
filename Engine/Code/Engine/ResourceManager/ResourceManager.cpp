#include "ResourceManager.hpp"
#include "../Core/XmlUtils.hpp"
#include "../Core/ErrorWarningAssert.hpp"
#include "MeshResource.hpp"

std::unordered_map<std::string, MeshResource*> ResourceManager::s_meshLookup;
std::vector<MeshResource*> ResourceManager::s_meshResources;

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
