#include "TileMap.hpp"
#include "TileMapManager.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

TileMap::TileMap(XmlElement* rootElement)
{
	int mapWidth = ParseXmlAttribute(rootElement, "width", 0);
	int mapHeight = ParseXmlAttribute(rootElement, "height", 0);
	m_size = IntVec2(mapWidth, mapHeight);

	int tileWidth = ParseXmlAttribute(rootElement, "tilewidth", 16);
	int tileHeight = ParseXmlAttribute(rootElement, "tileheight", 16);
	m_tileSize = IntVec2(tileWidth, tileHeight);

	m_isInfinite = ParseXmlAttribute(rootElement, "infinite", 0) == 1;

	// tileset
	for (XmlElement* tilesetElement = rootElement->FirstChildElement("tileset");
		tilesetElement; tilesetElement = tilesetElement->NextSiblingElement("tileset")) 
	{
		std::string tilesetSource = ParseXmlAttribute(tilesetElement, "source", "");
		Strings sourceArr=SplitStringOnDelimiter(tilesetSource, '.');
		Tileset* tilesetRef=TileMapManager::GetInstance().m_loadedTilesetsByName[sourceArr[0]];
		if (tilesetRef)
		{
			uint32_t firstGid = ParseXmlAttribute(tilesetElement, "firstgid", 1);
			tilesetRef->SetFirstGid(firstGid);
            TileMapManager::GetInstance().m_loadedTilesetsByGID[firstGid] = tilesetRef;
			m_tilesets.push_back(tilesetRef);

            TileMapManager::AddGidToPropertyMaskForEachTileset(tilesetRef);

		}
	}

 	for (XmlElement* layerElement = rootElement->FirstChildElement("layer");
 		layerElement; layerElement = layerElement->NextSiblingElement("layer")) 
 	{
 		TileLayer layer;
        layer.m_name = ParseXmlAttribute(layerElement, "name", "");
        layer.m_id = ParseXmlAttribute(layerElement, "id", 0);
        layer.m_class= ParseXmlAttribute(layerElement, "class", layer.m_class);
        if (layer.m_class == "TileMark")
        {
            m_markLayerIndex=layer.m_id;
        }
 		int width = ParseXmlAttribute(layerElement, "width", 0);
 		int height = ParseXmlAttribute(layerElement, "height", 0);
        m_size = IntVec2(width, height);
 
 		XmlElement* dataElement = layerElement->FirstChildElement("data");
 		if (dataElement) 
 		{
 			for (XmlElement* chunkElement = dataElement->FirstChildElement("chunk");
 				chunkElement; chunkElement = chunkElement->NextSiblingElement("chunk")) 
            {
 				TileChunk chunk;
 				int startX = ParseXmlAttribute(chunkElement, "x", 0);
 				int startY = -ParseXmlAttribute(chunkElement, "y", 0);
                chunk.m_startPosition = IntVec2(startX, startY);
                int chunkWidth= ParseXmlAttribute(chunkElement, "width", 0);
                int chunkHeight= ParseXmlAttribute(chunkElement, "height", 0);
                chunk.m_size = IntVec2(chunkWidth, chunkHeight);
 				const char* csvText = chunkElement->GetText();
 				if (csvText) 
 				{
                    Strings dataStrsWithousLineEnd= SplitStringOnDelimiterIgnoreChangeLine(csvText, ',');
                    for (int i = 0; i < dataStrsWithousLineEnd.size(); i++)
                    {
						int value = std::stoi(dataStrsWithousLineEnd[i]);
                        chunk.m_data.push_back(value);
                    }
 				}
                if (layer.m_name == "TileMarkLayer")
                {
                    int k = 1;
                }
                chunk.InitializeChunkVerts();
 				layer.m_chunks.push_back(chunk);
 			}
 		}
        m_layers.push_back(layer);
 	}
}

TileMap::~TileMap()
{
}

void TileMap::Render() const
{
    for (int i = 0; i < m_layers.size(); i++)
    {
        for (int j = 0; j < m_layers[i].m_chunks.size(); j++)
        {
            Texture* grassTex=g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/FarmTinyAssetPack/Tileset/TilesetGrassSpring.png");
            Texture* tileMarkTex= g_theRenderer->CreateOrGetTextureFromFile("Data/Art/TileMarksSet.png");
            g_theRenderer->BindTexture(grassTex);
            if (i == 3)
            {
                g_theRenderer->BindTexture(tileMarkTex);
            }
            g_theRenderer->SetModelConstants();
            g_theRenderer->DrawVertexArray(m_layers[i].m_chunks[j].m_verts);
        }
    }
}

uint32_t TileMap::GetTileGidFromLayerID(int layerID, IntVec2 const& gridPos)
{
    return m_layers[layerID - 1].GetGidFromGridPos(gridPos);
}
