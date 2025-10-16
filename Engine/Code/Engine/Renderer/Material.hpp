#pragma once
#include <cstdint>
class Renderer;
class Shader;

enum class MaterialType {
	BLINN_PHONG,
	PBR_METALLIC_ROUGHNESS,
	UNLIT,
};

enum class TextureSlot : uint8_t {

	SLOT_BASE_COLOR = 0,      // Blinn-Phong的Diffuse / PBR的Albedo
	SLOT_NORMAL = 1,          // 法线贴图（通用）
	SLOT_MATERIAL_PROPS = 2,  // Blinn-Phong的SGE / PBR的MRA
	SLOT_AO = 3,              // 环境光遮蔽（PBR专用）
	SLOT_EMISSIVE = 4,        // 自发光（可选）
	SLOT_SHADOW_MAP = 5,      // 阴影贴图
	SLOT_ENV_MAP = 6,         // 环境贴图

	Count               // 总槽位数
};

class Material {
public:
	virtual ~Material() = default;
	virtual MaterialType GetType() const = 0;
	virtual void Bind(Renderer* renderer) = 0;  
	virtual Shader* GetShader() const = 0;
};
