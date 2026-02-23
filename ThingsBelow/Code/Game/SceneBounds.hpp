#include "GameCommon.hpp"

class SceneBounds
{
public:
	static void SetBounds(Vec2 const& bottomLeft, Vec2 const& topRight);

	/// 获取场景左下角
	static Vec2 GetBottomLeft() { return s_bottomLeft; }

	/// 获取场景右上角
	static Vec2 GetTopRight() { return s_topRight; }

	/// 检查是否已初始化
	static bool IsInitialized() { return s_initialized; }

private:
	static Vec2 s_bottomLeft;
	static Vec2 s_topRight;
	static bool s_initialized;
};