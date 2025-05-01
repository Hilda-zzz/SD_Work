#include "SpawnInfo.hpp"

SpawnInfo::SpawnInfo(std::string const& typeName, Faction const& faction, Vec3 const& position, EulerAngles const& orientation)
	:m_typeName(typeName),m_faction(faction),m_position(position),m_orientation(orientation)
{
}
