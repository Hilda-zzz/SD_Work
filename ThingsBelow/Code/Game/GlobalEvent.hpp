#pragma once
#include <cstdint>

// ============================================================================
// GlobalEvent - 全局事件类型
// ============================================================================
// 定义触手系统能响应的所有事件类型
// 每个事件有对应的优先级（数值越大优先级越高）
// ============================================================================

enum class GlobalEvent
{
	NONE = 20,              // 默认状态，普通待机
	MOUSE_TRACKING = 60,    // 鼠标跟随（交互事件）
	DROPPING_FOOD = 80,     // 食物掉落（紧急事件）
	
	// TODO Phase 3: 更多事件
	// HOLDING_FOOD = 60,
	// HOLDING_LIGHT_BALL = 60,
	// MOUSE_HOVERING = 60,
	// IDLE_TAPPING = 40,
	// IDLE_CRAWLING = 40,
	// PERFORMANCE = 100,
};

/// 获取事件优先级（直接返回枚举值）
inline int GetEventPriority(GlobalEvent event)
{
	return static_cast<int>(event);
}
