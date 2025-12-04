// Nova2DAssetManager.hpp
#pragma once
#include <string>
#include <map>

class Texture;
class SpriteSheet;
class SpriteDefinition;
class SpriteAnimDefinition;
struct IntVec2;
enum class SpriteAnimPlaybackType;

//------------------------------------------------------------------------------------------------
// Nova2D 粒子资源管理器
// 职责：
//   - 拥有并管理所有 SpriteSheet 和 SpriteAnimDefinition
//   - 提供按名称获取资源的接口
//   - 统一管理资源生命周期
//------------------------------------------------------------------------------------------------
class Nova2DAssetManager
{
public:
	Nova2DAssetManager();
	~Nova2DAssetManager();

	void Startup();
	void Shutdown();

	// ===== 资源加载 =====
	// 加载精灵图集（如果已存在则返回现有的）
	SpriteSheet* LoadSpriteSheet(std::string const& name, std::string const& texturePath, IntVec2 const& gridLayout);

	// 注册单个精灵（给定名称和图集中的索引）
	void RegisterSprite(std::string const& name, std::string const& sheetName, int spriteIndex);

	// 创建动画定义
	SpriteAnimDefinition* CreateAnimation(std::string const& name, std::string const& sheetName,
		int startFrame, int endFrame, float framesPerSecond, SpriteAnimPlaybackType playbackType);

	// ===== 资源获取 =====
	SpriteSheet* GetSpriteSheet(std::string const& name) const;
	SpriteDefinition const* GetSprite(std::string const& name) const;
	SpriteAnimDefinition* GetAnimation(std::string const& name) const;

	// ===== 资源查询 =====
	bool HasSpriteSheet(std::string const& name) const;
	bool HasSprite(std::string const& name) const;
	bool HasAnimation(std::string const& name) const;

private:
	// 资源所有权
	std::map<std::string, SpriteSheet*> m_spriteSheets;              // 拥有 SpriteSheet
	std::map<std::string, SpriteAnimDefinition*> m_animations;       // 拥有 SpriteAnimDefinition
	std::map<std::string, SpriteDefinition const*> m_sprites;        // 引用 SpriteSheet 内部的 SpriteDefinition
};