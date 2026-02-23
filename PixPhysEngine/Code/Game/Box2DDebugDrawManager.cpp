// ========================================
// Box2DDebugDrawManager.cpp
// 全局Box2D调试绘制管理器实现
// ========================================

#include "Box2DDebugDrawManager.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <ThirdParty/box2d/include/box2d/box2d.h>

extern Renderer* g_theRenderer;

// ========================================
// Singleton Instance
// ========================================
Box2DDebugDrawManager& Box2DDebugDrawManager::GetInstance()
{
	static Box2DDebugDrawManager instance;
	return instance;
}

// ========================================
// Constructor / Destructor
// ========================================
Box2DDebugDrawManager::Box2DDebugDrawManager()
	: m_renderer(nullptr)
	, m_isInitialized(false)
{
}

Box2DDebugDrawManager::~Box2DDebugDrawManager()
{
	Shutdown();
}

// ========================================
// Initialize
// ========================================
void Box2DDebugDrawManager::Initialize(Renderer* renderer)
{
	if (m_isInitialized) {
		return;
	}

	m_renderer = renderer ? renderer : g_theRenderer;

	// 初始化b2DebugDraw
	m_debugDraw = b2DefaultDebugDraw();

	// 设置回调函数
	m_debugDraw.DrawSolidPolygonFcn = DrawSolidPolygon;
	m_debugDraw.DrawCircleFcn = DrawCircle;
	m_debugDraw.DrawSolidCircleFcn = DrawSolidCircle;
	m_debugDraw.DrawSegmentFcn = DrawSegment;
	m_debugDraw.DrawTransformFcn = DrawTransform;
	m_debugDraw.DrawPointFcn = DrawPoint;

	// 设置context（指向自己）
	m_debugDraw.context = this;

	// 配置绘制选项
	m_debugDraw.drawShapes = true;
	m_debugDraw.drawJoints = false;
	m_debugDraw.drawBounds = false;
	m_debugDraw.drawMass = false;
	m_debugDraw.drawContacts = false;

	m_isInitialized = true;
}

// ========================================
// Shutdown
// ========================================
void Box2DDebugDrawManager::Shutdown()
{
	m_isInitialized = false;
	m_renderer = nullptr;
}

// ========================================
// Draw World
// ========================================
void Box2DDebugDrawManager::DrawWorld(b2WorldId worldId)
{
	if (!m_isInitialized) {
		return;
	}

	if (B2_IS_NULL(worldId)) {
		return;
	}

	if (!m_renderer) {
		return;
	}

	// 设置渲染状态
	m_renderer->SetModelConstants();
	m_renderer->BindTexture(nullptr);

	// 调用Box2D的调试绘制
	b2World_Draw(worldId, &m_debugDraw);
}

// ========================================
// Draw Solid Polygon
// ========================================
void Box2DDebugDrawManager::DrawSolidPolygon(
	b2Transform transform,
	const b2Vec2* vertices,
	int vertexCount,
	float radius,
	b2HexColor color,
	void* context)
{
	Box2DDebugDrawManager* manager = static_cast<Box2DDebugDrawManager*>(context);
	if (!manager || !manager->m_renderer) {
		return;
	}

	Rgba8 rgba = B2ColorToRgba8(color);
	std::vector<Vertex_PCU> verts;

	// 绘制多边形边框
	for (int i = 0; i < vertexCount; ++i) {
		b2Vec2 p1 = b2TransformPoint(transform, vertices[i]);
		b2Vec2 p2 = b2TransformPoint(transform, vertices[(i + 1) % vertexCount]);

		Vec2 v1 = B2Vec2ToVec2(p1);
		Vec2 v2 = B2Vec2ToVec2(p2);

		AddVertsForLineSegment2D(verts, v1, v2, 0.5f, Rgba8::HILDA);
	}

	if (!verts.empty()) {
		manager->m_renderer->DrawVertexArray(verts);
	}
}

// ========================================
// Draw Circle
// ========================================
void Box2DDebugDrawManager::DrawCircle(
	b2Vec2 center,
	float radius,
	b2HexColor color,
	void* context)
{
	Box2DDebugDrawManager* manager = static_cast<Box2DDebugDrawManager*>(context);
	if (!manager || !manager->m_renderer) {
		return;
	}

	Rgba8 rgba = B2ColorToRgba8(color);
	Vec2 centerVec = B2Vec2ToVec2(center);

	std::vector<Vertex_PCU> verts;
	AddVertsForDisc2D(verts, centerVec, radius, rgba);

	if (!verts.empty()) {
		manager->m_renderer->DrawVertexArray(verts);
	}
}

// ========================================
// Draw Solid Circle
// ========================================
void Box2DDebugDrawManager::DrawSolidCircle(
	b2Transform transform,
	float radius,
	b2HexColor color,
	void* context)
{
	Box2DDebugDrawManager* manager = static_cast<Box2DDebugDrawManager*>(context);
	if (!manager || !manager->m_renderer) {
		return;
	}

	Rgba8 rgba = B2ColorToRgba8(color);
	Vec2 center = B2Vec2ToVec2(transform.p);

	std::vector<Vertex_PCU> verts;
	AddVertsForDisc2D(verts, center, radius, rgba);

	if (!verts.empty()) {
		manager->m_renderer->DrawVertexArray(verts);
	}
}

// ========================================
// Draw Segment
// ========================================
void Box2DDebugDrawManager::DrawSegment(
	b2Vec2 p1,
	b2Vec2 p2,
	b2HexColor color,
	void* context)
{
	Box2DDebugDrawManager* manager = static_cast<Box2DDebugDrawManager*>(context);
	if (!manager || !manager->m_renderer) {
		return;
	}

	Rgba8 rgba = B2ColorToRgba8(color);
	Vec2 v1 = B2Vec2ToVec2(p1);
	Vec2 v2 = B2Vec2ToVec2(p2);

	std::vector<Vertex_PCU> verts;
	AddVertsForLineSegment2D(verts, v1, v2, 0.1f, rgba);

	if (!verts.empty()) {
		manager->m_renderer->DrawVertexArray(verts);
	}
}

// ========================================
// Draw Transform
// ========================================
void Box2DDebugDrawManager::DrawTransform(
	b2Transform transform,
	void* context)
{
	Box2DDebugDrawManager* manager = static_cast<Box2DDebugDrawManager*>(context);
	if (!manager || !manager->m_renderer) {
		return;
	}

	const float axisScale = 1.0f;
	Vec2 origin = B2Vec2ToVec2(transform.p);

	// X轴（红色）
	b2Vec2 xAxisEnd = b2TransformPoint(transform, b2Vec2{ axisScale, 0.0f });
	Vec2 xEnd = B2Vec2ToVec2(xAxisEnd);

	std::vector<Vertex_PCU> xVerts;
	AddVertsForLineSegment2D(xVerts, origin, xEnd, 0.1f, Rgba8::RED);
	manager->m_renderer->DrawVertexArray(xVerts);

	// Y轴（绿色）
	b2Vec2 yAxisEnd = b2TransformPoint(transform, b2Vec2{ 0.0f, axisScale });
	Vec2 yEnd = B2Vec2ToVec2(yAxisEnd);

	std::vector<Vertex_PCU> yVerts;
	AddVertsForLineSegment2D(yVerts, origin, yEnd, 0.1f, Rgba8::GREEN);
	manager->m_renderer->DrawVertexArray(yVerts);
}

// ========================================
// Draw Point
// ========================================
void Box2DDebugDrawManager::DrawPoint(
	b2Vec2 p,
	float size,
	b2HexColor color,
	void* context)
{
	Box2DDebugDrawManager* manager = static_cast<Box2DDebugDrawManager*>(context);
	if (!manager || !manager->m_renderer) {
		return;
	}

	Rgba8 rgba = B2ColorToRgba8(color);
	Vec2 point = B2Vec2ToVec2(p);

	std::vector<Vertex_PCU> verts;
	AddVertsForDisc2D(verts, point, size, rgba);

	if (!verts.empty()) {
		manager->m_renderer->DrawVertexArray(verts);
	}
}

// ========================================
// Helper: Convert b2HexColor to Rgba8
// ========================================
Rgba8 Box2DDebugDrawManager::B2ColorToRgba8(b2HexColor b2Color)
{
	// b2HexColor格式：0xRRGGBBAA
	unsigned char r = (b2Color >> 24) & 0xFF;
	unsigned char g = (b2Color >> 16) & 0xFF;
	unsigned char b = (b2Color >> 8) & 0xFF;
	unsigned char a = b2Color & 0xFF;

	return Rgba8(r, g, b, a);
}

// ========================================
// Helper: Convert b2Vec2 to Vec2
// ========================================
Vec2 Box2DDebugDrawManager::B2Vec2ToVec2(b2Vec2 b2v)
{
	// 注意：需要根据你的坐标系统转换
	// 如果使用 METERS_PER_CELL 缩放，在这里处理
	constexpr float METERS_PER_CELL = 1.0f / 16.0f;  // 假设16 cells = 1 meter

	return Vec2(
		b2v.x / METERS_PER_CELL,
		b2v.y / METERS_PER_CELL
	);
}