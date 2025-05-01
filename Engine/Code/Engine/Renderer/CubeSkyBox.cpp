 #include "Engine/Renderer/CubeSkyBox.hpp"
 #include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/TextureCube.hpp"
#include "Engine/Core/EngineCommon.hpp"
 
 CubeSkyBox::CubeSkyBox(Renderer* pRenderer, std::string const* filePaths, std::string const* shaderPaths)
     :m_pRenderer(pRenderer)
 {
 	m_pRenderer = pRenderer;
 	m_shader = m_pRenderer->CreateShaderFromFile(shaderPaths->c_str());
    m_texture = m_pRenderer->CreateOrGetCubeTextureFromFiles(filePaths);

    m_scale = 10000.f;
    m_wireframeMode = false;

 	AddVertsForCubeSkyBox(m_verts, Vec3(), m_scale, Rgba8::WHITE);
 }
 
 void CubeSkyBox::Render()
 {
 	m_pRenderer->SetModelConstants();
 
 	m_pRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
 	m_pRenderer->SetBlendMode(BlendMode::ALPHA);
 	m_pRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
 	m_pRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
 	m_pRenderer->BindShader(m_shader);
 	m_pRenderer->BindTextureCube(m_texture);
 	m_pRenderer->DrawVertexArray(m_verts);
 }

 void CubeSkyBox::Update(float deltaTime)
 {
     UNUSED(deltaTime);
 }
