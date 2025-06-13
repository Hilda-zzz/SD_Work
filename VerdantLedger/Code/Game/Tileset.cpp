#include "Tileset.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GameCommon.hpp"
#include "TileMapManager.hpp"

extern Renderer*g_theRenderer;

Tileset::Tileset(XmlElement* rootElement)
{
	m_name = ParseXmlAttribute(rootElement, "name", m_name);
	int tileWidth = 0;
	tileWidth=ParseXmlAttribute(rootElement, "tilewidth", tileWidth);
	int tileHeight = 0;
	tileHeight=ParseXmlAttribute(rootElement, "tileheight", tileHeight);
	m_tileSize = IntVec2(tileWidth, tileHeight);
	m_tileCount=ParseXmlAttribute(rootElement, "tilecount", m_tileCount);
	m_columns= ParseXmlAttribute(rootElement, "columns", m_columns);

	XmlElement const* imageElement = rootElement->FirstChildElement("image");
	m_imagePath=ParseXmlAttribute(imageElement, "source", m_imagePath);
	m_imagePath = TileMapManager::ConvertTiledPathToGamePath(m_imagePath);
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(m_imagePath.c_str());
	int imageWidth = ParseXmlAttribute(imageElement, "width", tileWidth);
	int imageHeight = ParseXmlAttribute(imageElement, "height", tileHeight);
	m_imageSize = IntVec2(imageWidth, imageHeight);
	m_spriteSheet = new SpriteSheet(*m_texture, IntVec2(m_columns, m_tileCount / m_columns));

	for (XmlElement* tilesetElement = rootElement->FirstChildElement("tile");
		tilesetElement; tilesetElement = tilesetElement->NextSiblingElement("tile"))
	{
		uint32_t localID= ParseXmlAttribute(tilesetElement, "id", -1);
		if (localID != -1)
		{
			XmlElement* propertyElements= tilesetElement->FirstChildElement("properties");
			for (XmlElement* propElement = propertyElements->FirstChildElement("property");
				propElement; propElement = propertyElements->NextSiblingElement("property"))
			{
				std::string propertyName = ParseXmlAttribute(propElement, "name", "");
				bool isTure= ParseXmlAttribute(propElement, "value", false);
				if (propertyName != "")
				{
					TileProperty newProperty;
					newProperty.m_localID = localID;
					newProperty.m_propertyName = propertyName;
					newProperty.m_value = isTure;
					m_properties.push_back(newProperty);
				}
			}
		}
 	}
}

Tileset::~Tileset()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}

AABB2 Tileset::GetTileUVByInnerIndex(int index) const
{
	return m_spriteSheet->GetSpriteUVs(index);
}

IntVec2 Tileset::GetTileTextureCoord(uint32_t gid) const
{
	return IntVec2();
}
