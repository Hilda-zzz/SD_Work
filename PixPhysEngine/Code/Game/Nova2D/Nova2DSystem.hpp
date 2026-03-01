#pragma once
#include <vector>
#include "Nova2DParticle.hpp"

class Renderer;
class Camera;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Shader;
class Nova2DEmitter; 

class Nova2DGPUBuffers;     
class ComputeShader;
struct Nova2DEmitterConfig;

class Nova2DEmitterDefinition;
class Nova2DEmitterInstance;
class Texture2DArray;


struct Nova2DConfig 
{
	int maxParticles = 1000000;  // MVP: 先用小数量
};

enum class Nova2DRenderMode
{
	CPU_VERTEX_BUILD,      // 旧方案：每帧构建顶点
	GPU_INSTANCED,          // 新方案：实例化渲染
	GPU_COMPUTE
};

class Nova2DSystem {
public:
	Nova2DSystem(Renderer* renderer, Nova2DConfig const& config);
	~Nova2DSystem();

	// 生命周期
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Update(float deltaTime, Camera const& camera, Texture* collisionTexture);
	void Render(Camera const& camera) const;

	// 查询
	int GetAliveParticleCount() const;

	// ===== 新增：Definition管理 =====
	uint32_t CreateDefinition(Nova2DEmitterDefinition* definition);
	Nova2DEmitterDefinition* GetDefinition(uint32_t defID);

	// ===== 新增：Instance管理 =====
	uint32_t CreateInstance(uint32_t definitionID, float posX, float posY);
	Nova2DEmitterInstance* GetInstance(uint32_t instanceID);
	void DestroyInstance(uint32_t instanceID);

	// ===== 新增：GPU上传（每帧调用）=====
	void UploadDefinitionsToGPU();
	void UploadInstancesToGPU();

	// =============================== Emitter 管理 ===============================
	void RegisterEmitter(Nova2DEmitter* emitter);
	void UnregisterEmitter(Nova2DEmitter* emitter);

	//  CPU Emit
	void EmitParticle(Vec2 pos, Vec2 vel, float lifetime, Rgba8 m_color, float size);
	void EmitBurst(Vec2 pos, int count, Vec2 baseVelocity, float spread);
	void EmitParticleStruct(Nova2DParticle const& particle);

	// ========  GPU Emit =======
	void EmitParticlesGPU(Nova2DEmitterConfig const& config, int count);
	void EmitParticlesGPU_V2();
	void EmitParticlesGPU_V3();
	// GPU内部方法
	void ResetGPUCounters();
	void UpdateParticlesGPU(float deltaTime, Camera const& camera, Texture* collisionTexture);


private:
	// =============================== CPU模式 ================================================
	void StartupCPU();
	void ShutdownCPU();
	void BeginFrameCPU();
	void EndFrameCPU();
	void UpdateEmittersCPU(float deltaTime);
	void UpdateCPU(float deltaTime);
	void RenderCPU(Camera const& camera) const;

	// CPU内部方法
	void UpdateParticlesCPU(float deltaTime);
	void RenderParticlesCPU(Camera const& camera) const;
	int FindDeadParticleSlot();



	// ================================== GPU Instanced模式 =============================
	void StartupGPUInstanced();
	void ShutdownGPUInstanced();
	void BeginFrameGPUInstanced();
	void EndFrameGPUInstanced();
	void UpdateEmittersGPUInstanced(float deltaTime);
	void UpdateGPUInstanced(float deltaTime);
	void RenderGPUInstanced() const;

	// ====================================== GPU Compute模式 ============================
	void StartupGPUCompute();
	void ShutdownGPUCompute();
	void BeginFrameGPUCompute();
	void EndFrameGPUCompute();
	void UpdateEmittersGPUCompute(float deltaTime);
	void UpdateGPUCompute(float deltaTime, Camera const& camera, Texture* collisionTexture);
	void RenderGPUCompute(Camera const& camera) const;

	// ===== 共用方法 =====
	void UpdateEmitters(float deltaTime);

	// GPU 
	int CalculateEmitCount(Nova2DEmitter* emitter, float deltaTime);

	// Texture 管理
	void InitializeDefaultTextureArray();
	uint32_t RegisterTexture(const std::string& path);
	void RebuildTextureArray();
	void UpdateAllDefinitionsUVScale();

private:
	// 配置
	Nova2DConfig m_config;
	Renderer* m_renderer = nullptr;
	Nova2DRenderMode m_renderMode = Nova2DRenderMode::GPU_COMPUTE;

	// Emitters
	std::vector<Nova2DEmitter*> m_activeEmitters;

	// render resources
	VertexBuffer* m_quadVBO = nullptr;
	VertexBuffer* m_instanceVBO = nullptr;
	Shader* m_particleShader = nullptr;
	Shader* m_bindlessRenderShader = nullptr;

	// =============================== CPU ====================================
	// CPU粒子数组（MVP: 先不用GPU）
	std::vector<Nova2DParticle> m_particles;

	// =============================== GPU ====================================
	Nova2DGPUBuffers* m_gpuBuffers = nullptr;

	ComputeShader* m_resetCountersShader = nullptr;
	ComputeShader* m_emitShader = nullptr;
	ComputeShader* m_emitShader_v2 = nullptr;
	ComputeShader* m_emitShader_v3 = nullptr;
	ComputeShader* m_updateShader = nullptr;
	ComputeShader* m_writeIndirectArgsShader = nullptr;

	// ===== PopulateVBO 相关 =====
	VertexBuffer* m_populatedVBO = nullptr;
	IndexBuffer* m_populatedIBO = nullptr;
	ConstantBuffer* m_populateParamsCBO = nullptr;
	ComputeShader* m_populateVBOShader = nullptr;
	
	int m_pingPongIndex = 0;

	// ===== 新增成员变量 =====
	std::vector<Nova2DEmitterDefinition*> m_emitterDefs;
	std::vector<Nova2DEmitterInstance*> m_emitterInstances;

	uint32_t m_nextEmitterDefID = 0;
	uint32_t m_nextEmitterInstanceID = 0;
	uint32_t m_nextParticlePoolIndex = 0;  // 用于分配粒子池

	// ===== Texture2DArray管理 =====
	Texture2DArray* m_particleTextureArray = nullptr;
	std::vector<std::string> m_registeredTextures;  // 已注册纹理路径
	std::unordered_map<std::string, uint32_t> m_texturePathToIndex;  // 路径→索引
	std::vector<Vec2> m_textureUVScales;
};