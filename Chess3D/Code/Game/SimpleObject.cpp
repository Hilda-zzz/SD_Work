#include "SimpleObject.hpp"

extern Renderer* g_theRenderer;

SimpleObject::SimpleObject(MeshResource* meshResource)
{
	m_staticMesh = StaticMeshComponent(meshResource);
}

SimpleObject::~SimpleObject()
{
}

void SimpleObject::Update(float deltaSeconds)
{
	m_orientation.m_yawDegrees += deltaSeconds * 20.f;
}

void SimpleObject::Render() const
{
	Mat44 rotateMat=m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	Mat44 translationMat = Mat44::MakeTranslation3D(m_position);
	translationMat.Append(rotateMat);
	m_staticMesh.Render(g_theRenderer, translationMat);
}
