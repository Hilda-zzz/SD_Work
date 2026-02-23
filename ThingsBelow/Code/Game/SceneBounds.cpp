#include "SceneBounds.hpp"
// ============================================================================
// Scene Bounds Implementation
// ============================================================================

Vec2 SceneBounds::s_bottomLeft = Vec2::ZERO;
Vec2 SceneBounds::s_topRight = Vec2::ZERO;
bool SceneBounds::s_initialized = false;

void SceneBounds::SetBounds(Vec2 const& bottomLeft, Vec2 const& topRight)
{
	s_bottomLeft = bottomLeft;
	s_topRight = topRight;
	s_initialized = true;
}