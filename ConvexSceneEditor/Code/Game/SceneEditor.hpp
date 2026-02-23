#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/ConvexScene.hpp"
#include "Engine/Math/RaycastUtils.hpp"

class InputSystem;
class RandomNumberGenerator;
class Camera;


//-----------------------------------------------------------------------------------------------
// EditorMode - Different interaction modes
//-----------------------------------------------------------------------------------------------
enum class EditorMode
{
	NORMAL = 0,				// Normal mode - can select and manipulate objects
	RAYCAST_START_DRAG,		// Dragging raycast start point
	RAYCAST_END_DRAG,		// Dragging raycast end point
	OBJECT_DRAG,			// Dragging an object
};

class SceneEditor
{
public:
	SceneEditor(ConvexScene* scene, Camera* camera);
	~SceneEditor();

	void Update(float deltaSeconds);

	void Render() const;
	void RenderVisibleRay() const;
	void RenderDebugInfo() const;			// Render single-object raycast debug visualization
	void RenderUI();
	//-----------------------------------------------------------------------------------------------
	// Configuration Accessors
	//-----------------------------------------------------------------------------------------------
	int GetNumObjects() const;
	int GetNumInvisibleRays() const { return m_numInvisibleRays; }
	float GetLastRaycastTime() const { return m_lastRaycastTimeMs; }
	bool ShouldShowBoundingDiscs() const { return m_showBoundingDiscs; }
	bool ShouldShowSpatialStructure() const { return m_showSpatialStructure; }
	int GetDrawMode() const { return m_drawMode; }
	OptimizationMode GetOptimizationMode() const;

	// Get the visible raycast result for rendering
	RaycastResult2D const& GetVisibleRaycastResult() const { return m_visibleRayResult; }
	bool ShouldShowSingleObjectDebug() const;

private:
	void UpdateInput(float deltaSeconds);
	void UpdateMouseWorldPosition();

	// Mouse ray snapping (S = start, E = end, LMB = start, RMB = end)
	void HandleRaycastDragging();

	// Object transformation
	void HandleObjectSelection();
	void HandleObjectRotation(float deltaSeconds);
	void HandleObjectScaling(float deltaSeconds);
	void HandleObjectDragging();

	// Scene management
	void HandleSceneControls();
	void HandleDebugToggles();
	void HandleOptimizationCycling();

	void DoubleCovexObjects();
	void HalveConvexObjects();
	void RegenerateConvexObjects();
	void DoubleRaycasts();
	void HalveRaycasts();

	// Performance testing
	void HandleBatchRaycastTest();

	//-----------------------------------------------------------------------------------------------
	// Helper Methods
	//-----------------------------------------------------------------------------------------------

	void UpdateVisibleRay();

	void PerformBatchRaycastTest();

	// Check if only one object exists (for single-object debug visualization)
	bool IsOnlyOneObjectInScene() const;

	void RenderSingleObjectDebug() const;

private:
	//-----------------------------------------------------------------------------------------------
	// Scene reference
	//-----------------------------------------------------------------------------------------------
	ConvexScene* m_scene = nullptr;			// Not owned, just a reference
	Camera* m_camera = nullptr;				// Not owned, reference to screen camera
	RandomNumberGenerator* m_rng = nullptr;	// Owned by editor

	//-----------------------------------------------------------------------------------------------
	// Visible raycast (always drawn, can be manipulated)
	//-----------------------------------------------------------------------------------------------
	Vec2 m_visibleRayStart = Vec2(200.f, 400.f);
	Vec2 m_visibleRayEnd = Vec2(400.f, 600.f);
	RaycastResult2D m_visibleRayResult;

	//-----------------------------------------------------------------------------------------------
	// Object selection and manipulation
	//-----------------------------------------------------------------------------------------------
	ConvexObject2* m_currentObject = nullptr;	// Currently highlighted object (under mouse)
	ConvexObject2* m_lockedObject = nullptr;	// Locked object during drag/transform operations
	bool m_isDragging = false;
	Vec2 m_dragOffset = Vec2::ZERO;				// Offset from mouse to shape center when drag started

	//-----------------------------------------------------------------------------------------------
	// Editor mode
	//-----------------------------------------------------------------------------------------------
	EditorMode m_mode = EditorMode::NORMAL;

	//-----------------------------------------------------------------------------------------------
	// Mouse tracking
	//-----------------------------------------------------------------------------------------------
	AABB2 m_worldUV;
	Vec2 m_mouseScreenPos = Vec2::ZERO;
	Vec2 m_mouseWorldPos = Vec2::ZERO;
	Vec2 m_lastMouseWorldPos = Vec2::ZERO;		// Previous frame's mouse position

	//-----------------------------------------------------------------------------------------------
	// Performance testing
	//-----------------------------------------------------------------------------------------------
	int m_numInvisibleRays = 1024;				// Number of rays to cast when 'T' is pressed
	float m_lastRaycastTimeMs = 0.f;			// Time taken by last batch raycast test

	bool m_useBoundingDiscCheck = false;

	//-----------------------------------------------------------------------------------------------
	// Spatial Configuration
	//-----------------------------------------------------------------------------------------------
	int m_bvhMaxObjectsPerLeaf = 3;
	int m_bvhMaxTreeDepth = 16;
	int m_quadTreeMaxObjectsPerLeaf = 8;
	int m_quadTreeMaxTreeDepth = 8;
	//-----------------------------------------------------------------------------------------------
	// Display settings
	//-----------------------------------------------------------------------------------------------
	int m_drawMode = 0;							// 0 = composite, 1 = individual shapes
	bool m_showBoundingDiscs = false;			// F1 toggle
	bool m_showSpatialStructure = false;		// F4 toggle

	//-----------------------------------------------------------------------------------------------
	// Transformation speeds
	//-----------------------------------------------------------------------------------------------
	float m_rotationSpeed = 180.f;				// Degrees per second when holding rotation key
	float m_scaleSpeed = 1.0f;					// Scale multiplier per second when holding scale key

	//-----------------------------------------------------------------------------------------------
	// Constants
	//-----------------------------------------------------------------------------------------------
	static constexpr float RAY_ARROW_LENGTH = 20.f;
	static constexpr float RAY_ARROW_ANGLE = 30.f;
	static constexpr float RAY_THICKNESS = 3.f;
	static constexpr float IMPACT_NORMAL_LENGTH = 30.f;
};