#pragma once
#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

class BaseMap;

class BasePlayer
{
public:
	virtual ~BasePlayer() = default;

	// ========================================
	// Core Process
	// ========================================
	virtual void Update(float deltaTime) = 0;
	virtual void HandleInput() = 0;

	// ========================================
	// Render
	// ========================================
	virtual void RenderImgui() {}

	// ========================================
	// Camera management
	// ========================================
	virtual void InitCamera(IntVec2 const& mapSize);

	/// Help funcs
	Camera& GetCamera() { return m_camera; }
	const Camera& GetCamera() const { return m_camera; }

	// ========================================
	// Map management
	// ========================================
	BaseMap* GetCurMap() { return m_curMap; }
	BaseMap const* GetCurMap() const { return m_curMap; }

	void SetCurMap(BaseMap* curMap) { m_curMap = curMap; }

	// ========================================
	// Mouse
	// ========================================
	virtual Vec2 GetMouseWorldPosition() const;

protected:
	// ========================================
	// Construct
	// ========================================
	//BasePlayer(IntVec2 const& mapSize);
	BasePlayer() = default;

	// ========================================
	// Coords transition
	// ========================================
	Vec2 ScreenUVToWorldPos(Vec2 const& screenUV) const;
	IntVec2 WorldPosToGridCoords(Vec2 const& worldPos) const;

public:
	// ========================================
	// Members
	// ========================================
	Camera m_camera;           
	BaseMap* m_curMap = nullptr; 
};