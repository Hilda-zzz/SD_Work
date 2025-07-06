#pragma once

#include <vector>
#include "Engine/ResourceManager/StaticMeshComponent.hpp"

class SimpleObject
{
public:
	SimpleObject() {};
	SimpleObject(MeshResource* meshResource);
	~SimpleObject();

	void Update(float deltaSeconds);
	void Render() const;
	
public:
	StaticMeshComponent m_staticMesh;
	Vec3 m_position=Vec3(0.f,5.f,0.2f);
	EulerAngles m_orientation;
};