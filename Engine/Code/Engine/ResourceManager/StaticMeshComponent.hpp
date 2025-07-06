#pragma once
#include "../Math/Mat44.hpp"
#include "../Math/EulerAngles.hpp"

class Renderer;
class MeshResource;

class StaticMeshComponent
{
public:
	StaticMeshComponent() {};
	StaticMeshComponent(MeshResource* meshResource);
	~StaticMeshComponent();

	void Render(Renderer* curRenderer,Mat44 const& modelToWorldMat) const;

	void SetRelativePosition(Vec3 const& relativePos, EulerAngles const& relativeOrientation);

private:

public:

private:
	MeshResource* m_meshResource=nullptr;
	Vec3 m_relativePos=Vec3(0.f,0.f,0.f);
	EulerAngles m_relativeOrientation=EulerAngles(0.f,0.f,0.f);
};