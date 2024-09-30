#pragma comment(lib, "OpenGL32.lib")
#include <Engine/Renderer/Renderer.hpp>
#include <windows.h>
#include <gl/gl.h>	

extern HDC g_displayDeviceContext;				// ...becomes void* Window::m_displayContext
extern HGLRC g_openGLRenderingContext ;			// ...becomes void* Renderer::m_apiRenderingContext

void Renderer::CreateRenderingContext()
{
	// Creates an OpenGL rendering context (RC) and binds it to the current window's device context (DC)
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	// These two OpenGL-like functions (wglCreateContext and wglMakeCurrent) will remain here for now.
	int pixelFormatCode = ChoosePixelFormat(g_displayDeviceContext, &pixelFormatDescriptor);
	SetPixelFormat(g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor);
	g_openGLRenderingContext = wglCreateContext(g_displayDeviceContext);
	wglMakeCurrent(g_displayDeviceContext, g_openGLRenderingContext);

	// #SD1ToDo: move all OpenGL functions (including those below) to Renderer.cpp (only!)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::Startup()
{
	CreateRenderingContext();
}

void Renderer::BeginFrame()
{

}

void Renderer::EndFrame()
{
	SwapBuffers(g_displayDeviceContext);
}

void Renderer::Shutdown()
{

}

void Renderer::BeginView()
{
	// Establish a 2D (orthographic) drawing coordinate system: (0,0) bottom-left to (10,10) top-right
	// #SD1ToDo: This will be replaced by a call to g_renderer->BeginView( m_worldView ); or similar
	glLoadIdentity();
	//glOrtho(0.f, 10.f, 0.f, 10.f, 0.f, 1.f); // arguments are: xLeft, xRight, yBottom, yTop, zNear, zFar
	glOrtho(0.f, 200.f, 0.f, 100.f, 0.f, 1.f);

	//Replace by camera??
}

void Renderer::ClearScreen(const Rgba8& clearColor)
{
	// Clear all screen (backbuffer) pixels to medium-blue
	// #SD1ToDo: This will become g_renderer->ClearColor( Rgba8( 0, 0, 127, 255 ) );
	//glClearColor(0.f, 0.f, 0.5f, 1.f); // Note; glClearColor takes colors as floats in [0,1], not bytes in [0,255]
	//glClearColor(0.40f, 0.20f, 0.00f, 1.00f);
	glClearColor(clearColor.r/255.f, clearColor.g/255.f, clearColor.b/255.f, clearColor.a/255.f);
	glClear(GL_COLOR_BUFFER_BIT); // ALWAYS clear the screen at the top of each frame's Render()!
}

void Renderer::BeginCamera(const Camera& camera)
{
	glLoadIdentity();
	glOrtho(0.f, 200.f, 0.f, 100.f, 0.f, 1.f);
	//camera->SetOrthoView();
}

void Renderer::EndCamera(const Camera& camera)
{

}

//void Renderer::DrawVertexArray(float shipx,float shipy)
void Renderer::DrawVertexArray(int numVertexs, const Vertex_PCU* vertexs)
{
	// Draw some triangles (provide 3 vertexes each)
	// #SD1ToDo: Move all OpenGL code into Renderer.cpp (only); call g_renderer->DrawVertexArray() instead
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < numVertexs; i++)
		{
			Rgba8 const& color = vertexs[i].m_color;
			//const Vec2& tex = vertexs[i].m_uvTexCoords;
			Vec3 const& pos = vertexs[i].m_position;

			glColor4ub(color.r, color.g, color.b, color.a);
			glTexCoord2f(0.f, 0.f);
			glVertex2f(pos.x,pos.y);
		}
	}
	glEnd();
}
