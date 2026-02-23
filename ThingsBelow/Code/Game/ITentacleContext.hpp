#pragma once
#include "Engine/Math/Vec2.hpp"

class Camera;
class Tentacle;

// ============================================================================
// ITentacleContext - 触手的"世界观"接口
// ============================================================================
// Manager 实现这个接口，触手通过它访问全局数据
// 这样 Tentacle 不需要知道 Manager 的具体实现，保持解耦
// ============================================================================

class ITentacleContext
{
public:
	virtual ~ITentacleContext() = default;
	
	// ========================================================================
	// 数据访问接口
	// ========================================================================
	
	/// 获取鼠标世界坐标
	virtual Vec2 GetMouseWorldPosition() const = 0;
	
	/// 获取相机（用于坐标转换）
	virtual Camera const* GetCamera() const = 0;
	
	// ========================================================================
	// TODO Phase 3: 申请系统
	// ========================================================================
	// virtual bool RequestIdleBehavior(Tentacle* requester, IdleBehaviorType type) = 0;
};
