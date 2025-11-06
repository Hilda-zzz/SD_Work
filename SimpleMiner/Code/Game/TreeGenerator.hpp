#pragma once

#include "Engine/Math/IntVec3.hpp"
#include <vector>
#include <string>
#include "Engine/Math/IntVec2.hpp"
#include <utility>
#include "Game/Block.hpp"

enum class TreeSize 
{
	NONE,
	SMALL,
	MEDIUM,
	LARGE
};

enum class TreeType 
{
	OAK,
	BIRCH,
	SPRUCE,
	SPRUCE_SNOWY,
	JUNGLE,
	ACACIA,
	CACTUS
};


struct TreeStamp
{
	TreeType m_type;
	TreeSize m_size;
	int m_radius;  
	std::vector<std::pair<IntVec3, Block>> m_blocks; 

	TreeStamp() : m_type(TreeType::OAK), m_size(TreeSize::SMALL), m_radius(3) {}
};

struct BiomeTreeConfig 
{
	std::vector<TreeType> m_availableTreeTypes;  

	float m_threshold_Small;   
	float m_threshold_Medium;  
	float m_threshold_Large;   

	float m_tempInfluence;
	float m_humidityInfluence;

	BiomeTreeConfig()
		: m_threshold_Small(0.3f)
		, m_threshold_Medium(0.6f)
		, m_threshold_Large(0.8f)
		, m_tempInfluence(0.1f)
		, m_humidityInfluence(0.1f)
	{}
};

struct TreeCandidate 
{
	IntVec2 m_position;     
	TreeSize m_size;         
	TreeType m_type;         
	int m_surfaceZ;          

	TreeCandidate(IntVec2 pos, TreeSize sz, TreeType tp, int z)
		: m_position(pos), m_size(sz), m_type(tp), m_surfaceZ(z) {}
};