#include "TileMap.hpp"
#include "TileMapManager.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GroundObstacle.hpp"

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
            m_markLayerId=layer.m_id;
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
                    Strings dataStrsWithoutLineEnd= SplitStringOnDelimiterIgnoreChangeLine(csvText, ',');
                    for (int i = 0; i < (int)dataStrsWithoutLineEnd.size(); i++)
                    {
                        // can be optimized
						int value = std::stoi(dataStrsWithoutLineEnd[i]);
                        chunk.m_terrianData.push_back(value);
                    }
 				}
//                 if (layer.m_name == "TileMarkLayer")
//                 {
//                     //int k = 1;
//                 }
                chunk.InitializeChunkVerts();
 				layer.m_chunks.push_back(chunk);
 			}
 		}
        m_layers.push_back(layer);
 	}
    m_markLayer = FindLayerById(m_markLayerId);
}

TileMap::~TileMap()
{
	for (GroundObstacle* obstacle : m_obstacles)
	{
        delete obstacle;
        obstacle = nullptr;
	}
    m_obstacles.clear();
}

void TileMap::Render() const
{
    for (int i = 0; i < (int)m_layers.size(); i++)
    {
        for (int j = 0; j < (int)m_layers[i].m_chunks.size(); j++)
        {
            Texture* grassTex=g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/FarmTinyAssetPack/Tileset/TilesetGrassSpring.png");
            Texture* tileMarkTex= g_theRenderer->CreateOrGetTextureFromFile("Data/Art/TileMarksSet.png");
            g_theRenderer->BindTexture(grassTex);
            if (i == 3)
            {
                continue;
                //g_theRenderer->BindTexture(tileMarkTex);
            }
            g_theRenderer->SetModelConstants();
            g_theRenderer->DrawVertexArray(m_layers[i].m_chunks[j].m_terrianVerts);
        }
    }
    TileLayer* markLayer = m_markLayer;
    for (int i = 0; i < markLayer->m_chunks.size(); i++)
    {
        markLayer->m_chunks[i].RenderDynamicContent();
    }
//     for (GroundObstacle* obstacle : m_obstacles)
//     {
//         obstacle->Render();
//     }
}

uint32_t TileMap::GetTileGidFromLayerID(int layerID, IntVec2 const& gridPos)
{
    return m_layers[layerID - 1].GetGidFromGridPos(gridPos);
}

uint64_t TileMap::GetTileKey(IntVec2 const& gridPos)
{
    // deal with the negative grid position
    // below version can not deal with negative grid pos
	//return (static_cast<uint64_t>(gridPos.x) << 32) |  // left shift x | y
		//static_cast<uint32_t>(gridPos.y);
	uint32_t x_bits = *reinterpret_cast<const uint32_t*>(&gridPos.x);
	uint32_t y_bits = *reinterpret_cast<const uint32_t*>(&gridPos.y);
	return (static_cast<uint64_t>(x_bits) << 32) | y_bits;
}

TileLayer* TileMap::FindLayerById(int layerId)
{
	for (TileLayer& layer : m_layers) 
    {
		if (layer.m_id == layerId) 
        {
			return &layer;
		}
	}
	return nullptr;
}
