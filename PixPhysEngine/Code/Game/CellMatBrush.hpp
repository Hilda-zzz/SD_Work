#pragma once
#include "CellMatDef.hpp"
#include "ThirdParty/imgui/imgui.h"

struct CellMatUIInfo {
	PhyType m_physType;
	std::string m_physTypeName;

	std::string m_name;
	std::string m_description;
	ImVec4 m_color;
};