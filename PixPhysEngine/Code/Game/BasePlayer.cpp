#include "BasePlayer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "BaseMap.hpp"

extern Window* g_theWindow;

//BasePlayer::BasePlayer(IntVec2 const& mapSize)
//{
//}

void BasePlayer::InitCamera(IntVec2 const& mapSize)
{
	// 获取窗口尺寸
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_camera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));

	// 计算地图中心
	Vec2 mapCenter = Vec2((float)mapSize.x * 0.5f, (float)mapSize.y * 0.5f);

	// 计算视野大小（比地图大 10%）
	float viewWidth = (float)mapSize.x * 1.1f;
	float viewHeight = (float)mapSize.y * 1.1f;

	// 计算以地图为中心的视野边界
	Vec2 viewMins = mapCenter - Vec2(viewWidth * 0.5f, viewHeight * 0.5f);
	Vec2 viewMaxs = mapCenter + Vec2(viewWidth * 0.5f, viewHeight * 0.5f);

	m_camera.SetOrthographicView(viewMins, viewMaxs);
}

// ========================================
// 鼠标/交互查询
// ========================================

Vec2 BasePlayer::GetMouseWorldPosition() const
{
	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	return ScreenUVToWorldPos(mouseUV);
}

// ========================================
// 辅助方法
// ========================================

Vec2 BasePlayer::ScreenUVToWorldPos(Vec2 const& screenUV) const
{
	AABB2 worldBounds = AABB2(m_camera.GetOrthoBottomLeft(), m_camera.GetOrthoTopRight());
	return worldBounds.GetPointAtUV(screenUV);
}

IntVec2 BasePlayer::WorldPosToGridCoords(Vec2 const& worldPos) const
{
	int gridX = static_cast<int>(floor(worldPos.x));
	int gridY = static_cast<int>(floor(worldPos.y));
	return IntVec2(gridX, gridY);
}