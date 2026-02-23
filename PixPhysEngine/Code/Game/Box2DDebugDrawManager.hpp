// ========================================
// Box2DDebugDrawManager.hpp
// 全局Box2D调试绘制管理器
// ========================================

#pragma once
#include <ThirdParty/box2d/include/box2d/types.h>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>

class Renderer;

// ========================================
// Box2DDebugDrawManager
// 单例模式，全局共享的Box2D调试绘制
// ========================================
class Box2DDebugDrawManager
{
public:
	// === Singleton Access ===
	static Box2DDebugDrawManager& GetInstance();

	// === Initialization ===
	void Initialize(Renderer* renderer);
	void Shutdown();

	// === Debug Draw Configuration ===
	void SetDrawShapes(bool enable) { m_debugDraw.drawShapes = enable; }
	void SetDrawJoints(bool enable) { m_debugDraw.drawJoints = enable; }
	void SetDrawBounds(bool enable) { m_debugDraw.drawBounds = enable; }
	void SetDrawMass(bool enable) { m_debugDraw.drawMass = enable; }
	void SetDrawContacts(bool enable) { m_debugDraw.drawContacts = enable; }

	// === Rendering ===
	void DrawWorld(b2WorldId worldId);  // 绘制整个Box2D世界

	// === Get b2DebugDraw ===
	b2DebugDraw* GetDebugDraw() { return &m_debugDraw; }
	const b2DebugDraw* GetDebugDraw() const { return &m_debugDraw; }

private:
	// === Singleton ===
	Box2DDebugDrawManager();
	~Box2DDebugDrawManager();
	Box2DDebugDrawManager(const Box2DDebugDrawManager&) = delete;
	Box2DDebugDrawManager& operator=(const Box2DDebugDrawManager&) = delete;

	// === Box2D Callbacks (static) ===
	static void DrawSolidPolygon(
		b2Transform transform,
		const b2Vec2* vertices,
		int vertexCount,
		float radius,
		b2HexColor color,
		void* context
	);

	static void DrawCircle(
		b2Vec2 center,
		float radius,
		b2HexColor color,
		void* context
	);

	static void DrawSolidCircle(
		b2Transform transform,
		float radius,
		b2HexColor color,
		void* context
	);

	static void DrawSegment(
		b2Vec2 p1,
		b2Vec2 p2,
		b2HexColor color,
		void* context
	);

	static void DrawTransform(
		b2Transform transform,
		void* context
	);

	static void DrawPoint(
		b2Vec2 p,
		float size,
		b2HexColor color,
		void* context
	);

	// === Helper Functions ===
	static Rgba8 B2ColorToRgba8(b2HexColor b2Color);
	static Vec2 B2Vec2ToVec2(b2Vec2 b2v);

private:
	b2DebugDraw m_debugDraw;     // Box2D调试绘制结构
	Renderer* m_renderer;        // 渲染器引用
	bool m_isInitialized;        // 是否已初始化
};