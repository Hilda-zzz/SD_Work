#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/ZCylinder.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"

class Shader;
class Texture;
class VertexBuffer;
class IndexBuffer;

class Actor
{
public:
	Actor() {};
	Actor(Vec3 const& positon, EulerAngles const& orientaion, 
		Rgba8 const& color = Rgba8::WHITE, bool isStatic = false,ZCylinder const& collider=ZCylinder(Vec3(0.f, 0.f, 0.f), 1.f, 0.03525f));
	~Actor() {};

	void Update();
	void Render() const;
	Mat44 GetModelMat() const;

public:
	Vec3 m_position = Vec3(0.f, 0.f, 0.f);
	EulerAngles m_orientation = EulerAngles(0.f, 0.f, 0.f);
	Rgba8 m_color = Rgba8::WHITE;
	bool m_isStatic = false;
	ZCylinder m_physicCollider = ZCylinder(Vec3(0.f, 0.f, 0.f), 1.f, 1.f);

	Vec3 m_velocity = Vec3(0.f, 0.f, 0.f);

	std::vector<Vertex_PCU>	m_vertexs;
	//std::vector<unsigned int>	m_indexs;
	Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	//VertexBuffer* m_vertexBuffer = nullptr;
	//IndexBuffer* m_indexBuffer = nullptr;

// 	bool m_isPushedByEntities = false;
// 	bool m_doesPushEntities = false;
// 	bool m_isPushedByWalls = false;
// 	bool m_isHitByBullets = false;
};