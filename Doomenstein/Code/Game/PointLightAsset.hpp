#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/PointLight.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"

class Map;
class Texture;
class PlayerController;

class PointLightAsset
{
public:
	PointLightAsset(Vec3 const& position, Map* currentMap = nullptr);
	~PointLightAsset() {};

	void Update(float deltaSeconds);
	void Render(PlayerController* curPlayer) const;

	Vec3 GetPosition() const { return m_position; }
	PointLight* GetPointLight() { return &m_pointLight; }

private:

	Mat44 GetBillboardModelMat(PlayerController* curPlayer) const;

	Map* m_curMap = nullptr;

	PointLight m_pointLight;

	std::vector<Vertex_PCU> m_billboardVerts;
	Texture* m_lanternTexture = nullptr;

	Vec3 m_position;
};