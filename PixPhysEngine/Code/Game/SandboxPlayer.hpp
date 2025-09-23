#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "CellMatDef.hpp"
#include "Engine/Math/IntVec2.hpp"
class SandboxMap;

class SandboxPlayer {
public:
	SandboxPlayer(IntVec2 const& mapSize);

	void Update(float deltaTime);
	void Render() const; // render the brush

	void InitCamera(IntVec2 const& mapSize);

	void HandleInput();

	Vec2 GetMouseWorldPosition() const;
	CellMatType GetSelectedMaterial() const { return m_selectedMaterial; }
	int GetBrushSize() const { return m_brushSize; }

	void SetCurMap(SandboxMap* curMap) { m_curMap = curMap; }
	
	bool IsPlacing() const;
	bool IsErasing() const;

	Camera& GetCamera() { return m_camera; }

private:
	void HandleCameraControls();
	void HandleMaterialSelection();
	void HandleBrushControls();
	
public:
	Camera m_camera;
private:
	SandboxMap* m_curMap = nullptr;
	CellMatType m_selectedMaterial = CellMatType::MAT_SAND;
	int m_brushSize = 1;
	IntVec2 m_lastPlacedPos = IntVec2(-1, -1);
	bool m_isSand = true;

	bool m_isRightMouseDragging = false;
	int m_dragStartX = 0;
	int m_dragStartY = 0;
	int m_dragCurrentX = 0;
	int m_dragCurrentY = 0;


};