#pragma once
#include <string>
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "ActorDefinition.hpp"

class SpawnInfo
{
public:
	SpawnInfo() {}
	SpawnInfo(std::string const& typeName, Faction const& faction, Vec3 const& position, EulerAngles const& orientation);
	~SpawnInfo()=default;
public:
	std::string m_typeName;
	Faction m_faction;
	Vec3 m_position;
	EulerAngles m_orientation;
};