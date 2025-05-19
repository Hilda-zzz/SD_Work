#include "PointLightAsset.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "PlayerController.hpp"
#include "Engine/Renderer/Renderer.hpp"



extern Renderer* g_theRenderer;

PointLightAsset::PointLightAsset(Vec3 const& position, Map* currentMap):m_position(position),m_curMap(currentMap)
{
	m_pointLight = PointLight(
		m_position,
		5.0f,                       
		Rgba8(235, 169, 49),         
		1.5f,                      
		Vec3(1.0f, 0.35f, 0.15f)    
	);
	m_curMap->m_pointLights.push_back(m_pointLight);

	AddVertsForQuad3D(m_billboardVerts, Vec3(0.f, -0.2f, 0.f),
		Vec3(0.f, 0.2f, 0.f),
		Vec3(0.0f, 0.2f, 0.2f),
		Vec3(0.f, -0.2f, 0.2f),
		Rgba8::WHITE, AABB2::ZERO_TO_ONE);

	m_lanternTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/HangLantern.png");
}

void PointLightAsset::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void PointLightAsset::Render(PlayerController* curPlayer) const
{
	Mat44 finalModelMat = GetBillboardModelMat(curPlayer);

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindShader(nullptr);

	g_theRenderer->SetModelConstants(finalModelMat);
	g_theRenderer->BindTexture(m_lanternTexture);
	g_theRenderer->DrawVertexArray(m_billboardVerts);
}

Mat44 PointLightAsset::GetBillboardModelMat(PlayerController* curPlayer) const
{
	Mat44 billboardMatrix = Mat44();
	billboardMatrix = GetBillboardMatrix(BillboardType::WORLD_UP_FACING,
		curPlayer->m_playerCam.GetOrientation().GetAsMatrix_IFwd_JLeft_KUp(),
		curPlayer->m_playerCam.GetPosition(),
		m_position);
	return billboardMatrix;
}
