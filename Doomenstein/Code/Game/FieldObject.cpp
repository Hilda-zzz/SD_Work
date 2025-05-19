#include "FieldObject.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Map.hpp"
#include "PlayerController.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RaycastUtils.hpp"

extern Renderer* g_theRenderer;

FieldObject::FieldObject(Vec3 const& position,Map* curMap):m_position(position),m_curMap(curMap)
{
	m_boundingBox = AABB3(m_position - Vec3(0.5f, 0.5f, 0.f), m_position + Vec3(0.5f, 0.5f, 1.f));

	m_fieldVerts.reserve(6);
	m_plantVerts.reserve(6);

	m_fieldTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Field.png");
	m_plantTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlantGrowth.png");
	m_waterIconTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/WaterIcon.png");
	m_basketIconTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasketIcon.png");
	m_plantSheet = new SpriteSheet(*m_plantTex, IntVec2(4, 1));

	AddVertsForQuad3D_WithTBN(m_fieldVerts, m_position+Vec3(-0.5f, -0.5f, 0.f),
		m_position + Vec3(0.5f, -0.5f, 0.f),
		m_position + Vec3(0.5f, 0.5f, 0.f),
		m_position + Vec3(-0.5f, 0.5f, 0.f),
		Rgba8::WHITE,AABB2::ZERO_TO_ONE);

	m_growthTimer = new Timer(m_seedToSproutTime, m_curMap->m_game->m_gameClock);
	m_growthTimer->Start();
}

FieldObject::~FieldObject()
{
	delete m_growthTimer;
	m_growthTimer = nullptr;

	delete m_plantSheet;
	m_plantSheet = nullptr;
}

void FieldObject::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (!m_growthTimer->IsStopped())
	{
		if (m_growthTimer->GetElapsedFraction() >= 1.f)
		{
			m_growthTimer->Stop();
// 			if (m_curPlantState == PlantState::PLANT_GROWING)
// 			{
// 				m_curPlantState = PlantState::PLANT_HARVESTABLE;
// 			}
// 			else
// 			{
				m_needWater = true;
			//}
		}
	}

	//add verts
	m_plantVerts.clear();

	AddVertsForRoundedQuad3D(m_plantVerts, Vec3(0.f, -0.5f, 0.f), 
		 Vec3(0.f, 0.5f, 0.f),
		 Vec3(0.0f, 0.5f, 3.f), 
		 Vec3(0.f, -0.5f, 3.f),
		Rgba8::WHITE, m_plantSheet->GetSpriteUVs((int)m_curPlantState));

	m_tipIconVerts.clear();

	AddVertsForRoundedQuad3D(m_tipIconVerts, Vec3(0.f, -0.5f, 0.f),
		Vec3(0.f, -0.1f, 0.f),
		Vec3(0.0f, -0.1f,0.4f),
		Vec3(0.f, -0.5f, 0.4f),
		Rgba8::WHITE, AABB2::ZERO_TO_ONE);

}

void FieldObject::Render(PlayerController* curPlayer) const
{
	Mat44 finalModelMat = GetFinalModelMat(curPlayer);

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindShader(m_curMap->m_shader);
	Mat44 lightViewProjection = g_theRenderer->GetDirectLightProjectionMat(m_curMap->m_sunDirection, 
		Vec3(m_curMap->m_dimensions.x * 0.5f - 1.f, m_curMap->m_dimensions.y * 0.5f - 1.f, 0.f), m_curMap->m_dimensions.GetLength() * 0.5f);
	g_theRenderer->SetShadowConstants(lightViewProjection);
	g_theRenderer->SetShadowSampleState();
	g_theRenderer->BindShadowTexture();

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(m_fieldTex);
	g_theRenderer->DrawVertexArray_WithTBN(m_fieldVerts);

	g_theRenderer->SetModelConstants(finalModelMat);
	g_theRenderer->BindTexture(m_plantTex);
	g_theRenderer->DrawVertexArray_WithTBN(m_plantVerts);

	if (m_needWater)
	{
		g_theRenderer->SetModelConstants(finalModelMat);
		g_theRenderer->BindTexture(m_waterIconTex);
		g_theRenderer->DrawVertexArray_WithTBN(m_tipIconVerts);
	}

	if (m_curPlantState == PlantState::PLANT_HARVESTABLE)
	{
		g_theRenderer->SetModelConstants(finalModelMat);
		g_theRenderer->BindTexture(m_basketIconTex);
		g_theRenderer->DrawVertexArray_WithTBN(m_tipIconVerts);
	}
}

void FieldObject::RenderShadowTexture(PlayerController* curPlayer) const
{
	Mat44 finalModelMat = GetFinalModelMat(curPlayer);

	g_theRenderer->BindShader(m_curMap->m_shadowShader);
	g_theRenderer->SetModelConstants(finalModelMat);

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);

	g_theRenderer->BindTexture(m_plantTex);
	g_theRenderer->DrawVertexArray_WithTBN(m_plantVerts);
}

void FieldObject::PourWater()
{
	if (m_needWater)
	{
		m_curPlantState = GetNextState(m_curPlantState);

		switch (m_curPlantState)
		{
		case PlantState::PLANT_SEED:
			m_growthTimer->m_period=m_seedToSproutTime;
			break;

		case PlantState::PLANT_SPROUT:
			m_growthTimer->m_period = m_sproutToGrowingTime;
			m_growthTimer->Start();
			m_needWater = false;
			break;

		case PlantState::PLANT_GROWING:
			m_growthTimer->m_period = m_growingToMatureTime;
			m_growthTimer->Start();
			m_needWater = false;
			break;

		case PlantState::PLANT_HARVESTABLE:
			m_growthTimer->Stop();
			m_needWater = false;
			break;

		default:
			break;
		}
		
	}
}

void FieldObject::Harvest(PlayerController* curPlayer)
{
	if (m_curPlantState == PlantState::PLANT_HARVESTABLE)
	{
		m_isDie = true;
		curPlayer->m_coinCount += m_harvestCoinCount;
	}
}

bool FieldObject::IsReadyToHarvest() const
{
	return false;
}

bool FieldObject::RaycastAgainst(Vec3 const& rayStart, Vec3 const& rayDirection, float& outDistance) const
{
	return RaycastVsAABB3D(rayStart, rayDirection, outDistance, m_boundingBox).m_didImpact;
}

PlantState FieldObject::GetNextState(PlantState currentState)
{
	switch (currentState)
	{
	case PlantState::PLANT_SEED:
		return PlantState::PLANT_SPROUT;

	case PlantState::PLANT_SPROUT:
		return PlantState::PLANT_GROWING;

	case PlantState::PLANT_GROWING:
		return PlantState::PLANT_HARVESTABLE;

	case PlantState::PLANT_HARVESTABLE:
		return PlantState::PLANT_HARVESTABLE;

	default:
		return currentState;
	}
}

Mat44 FieldObject::GetFinalModelMat(PlayerController* curPlayer) const
{
	Mat44 billboardMatrix = Mat44();
	billboardMatrix = GetBillboardMatrix(BillboardType::WORLD_UP_FACING,
		curPlayer->m_playerCam.GetOrientation().GetAsMatrix_IFwd_JLeft_KUp(),
		curPlayer->m_playerCam.GetPosition(),
		m_position);
	return billboardMatrix;
}
