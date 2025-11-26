#pragma once
#include "Game/BaseMap.hpp"
#include "Game/Nova2D/Nova2DSystem.hpp"
#include "Game/Nova2D/Nova2DEmitter.hpp"
#include <vector>

class Camera;
class SpriteSheet;
class SpriteAnimDefinition;
class SandboxPlayer;

//==========================================================================
// Nova2D 测试和编辑场景
// 功能：
// 1. 粒子系统测试环境
// 2. ImGui 实时配置编辑器
// 3. 多个预设效果管理
//==========================================================================
class Nova2DTestMap : public BaseMap 
{
public:
	Nova2DTestMap(SandboxPlayer* player);
	virtual ~Nova2DTestMap();

	// BaseMap 接口
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;
	virtual void Render() const override;

	// UI 渲染
	void RenderUI() const;

private:
	// 初始化
	void LoadTextureResources();
	void CreatePresetEmitters();

	// ImGui UI 模块
	void RenderEmitterSelector() const;
	void RenderEmitterPropertiesUI() const;
	void RenderEmissionUI() const;
	void RenderMotionUI() const;
	void RenderAppearanceUI() const;
	void RenderControlPanel() const;

	// 发射器管理
	void CreateNewEmitter();
	void DeleteCurrentEmitter();
	void ApplyConfigToEmitter();  // 将 UI 修改同步到 Emitter

private:
	SandboxPlayer* m_player = nullptr;
	// 发射器列表
	std::vector<Nova2DEmitter*> m_emitters;
	mutable int m_selectedEmitterIndex = 0;  // mutable for ImGui
	Nova2DEmitter* GetCurrentEmitter() const;

	// 纹理资源
	SpriteSheet* m_sparkSheet = nullptr;
	SpriteSheet* m_circleSheet = nullptr;
	SpriteSheet* m_explosionSheet = nullptr;
	SpriteAnimDefinition* m_explosionAnim = nullptr;

	// UI 状态（用于编辑）
	mutable Nova2DEmitterConfig m_editingConfig;	// 当前编辑的配置
	mutable bool m_configDirty = false;				// 配置是否有修改

};