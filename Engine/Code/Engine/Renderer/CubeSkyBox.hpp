 #pragma once
 #include "Engine/Renderer/Renderer.hpp"
 #include "Engine/Math/Vec3.hpp"
 
class TextureCube;

 class CubeSkyBox
 {
 public:
 	CubeSkyBox(Renderer* pRenderer, std::string const* filePaths, std::string const* shaderPaths);
    ~CubeSkyBox() {};
 
 	void Render();
 
    void SetScale(float scale) { m_scale = scale; };
    void SetWireframeMode(bool enable) { m_wireframeMode = enable; };
 
 	void Update(float deltaTime);
 

 private:
 	Renderer* m_pRenderer=nullptr;
 	Shader* m_shader = nullptr;
	float m_scale = 10000.f;
	bool m_wireframeMode = false;

    TextureCube* m_texture=nullptr;
    std::vector<Vertex_PCU> m_verts;
};