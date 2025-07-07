#pragma once
#include <string>
#include "Engine/Math/Vec3.hpp"

class StaticMeshInfo
{
public:
	StaticMeshInfo() {};
	~StaticMeshInfo() {};

	StaticMeshInfo(std::string const& metaFilePath,
		const std::string& objFilePath,
		const std::string& shaderName,
		const std::string& diffuseTexPath,
		const std::string& normalTexPath,
		const std::string& specularTexPath,
		float unitsPerMeter,
		const Vec3& fwd,
		const Vec3& left,
		const Vec3& up)
		: m_metaFilePath(metaFilePath)
		,m_objFilePath(objFilePath)
		, m_shaderName(shaderName)
		, m_diffuseTexPath(diffuseTexPath)
		, m_normalTexPath(normalTexPath)
		, m_sgeTexPath(specularTexPath)
		, m_unitsPerMeter(unitsPerMeter)
		, m_fwd(fwd)
		, m_left(left)
		, m_up(up)
	{
	}
	std::string m_metaFilePath = "";
	std::string m_objFilePath="";
	std::string m_shaderName = "";
	std::string m_diffuseTexPath = "";
	std::string m_normalTexPath = "";
	std::string m_sgeTexPath = "";
	float m_unitsPerMeter = 1.f;
	bool m_useIndex = false;

	Vec3 m_fwd = Vec3(1.f, 0.f, 0.f);
	Vec3 m_left = Vec3(0.f, 1.f, 0.f);;
	Vec3 m_up = Vec3(0.f, 0.f, 1.f);;
};