#include "Tileset.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GameCommon.hpp"

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
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(m_imagePath.c_str());
	int imageWidth = ParseXmlAttribute(imageElement, "width", tileWidth);
	int imageHeight = ParseXmlAttribute(imageElement, "height", tileHeight);
	m_imageSize = IntVec2(imageWidth, imageHeight);
}

Tileset::~Tileset()
{
}

IntVec2 Tileset::GetTileTextureCoord(uint32_t gid) const
{
	return IntVec2();
}
