#pragma once

#include <Engine/Core/Rgba8.hpp>
#include "Camera.hpp"
#include <Engine/Core/Vertex_PCU.hpp>


class Renderer
{
public:
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen(const Rgba8& clearColor);
	void BeginCamera(const Camera& camera);
	void EndCamera(const Camera& camera);
	void DrawVertexArray(int numVertexs, const Vertex_PCU* vertexs);

	void CreateRenderingContext();
	//void BeginView();

private:

};