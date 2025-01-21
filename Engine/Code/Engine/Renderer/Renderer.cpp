#pragma comment(lib, "OpenGL32.lib")
#include "Engine/Renderer/Renderer.hpp"
#include <windows.h>
#include <gl/gl.h>	
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//extern HDC g_displayDeviceContext;				// ...becomes void* Window::m_displayContext

//extern HGLRC g_openGLRenderingContext ;			// ...becomes void* Renderer::m_apiRenderingContext

Renderer::Renderer(RendererConfig rendererConfig):m_config(rendererConfig)
{

}

Renderer::~Renderer()
{
	for (Texture* texture :m_loadedTextures)
	{
		delete texture;
	}
}

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
	HDC displayContext = static_cast<HDC>(m_config.m_window->GetDisplayContext());
	int pixelFormatCode = ChoosePixelFormat(displayContext, &pixelFormatDescriptor);
	SetPixelFormat(displayContext, pixelFormatCode, &pixelFormatDescriptor);
	HGLRC g_openGLRenderingContext = wglCreateContext(displayContext);
	wglMakeCurrent(displayContext, g_openGLRenderingContext);

	// #SD1ToDo: move all OpenGL functions (including those below) to Renderer.cpp (only!)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Texture* Renderer::GetTextureForFileName(char const* imageFilePath)
{
	for (int i = 0; i < static_cast<int>(m_loadedTextures.size()); i++)
	{
		if (m_loadedTextures[i]->GetImageFilePath() == imageFilePath)
		{
			return m_loadedTextures[i];
		}
	}
	return nullptr;
}

BitmapFont* Renderer::GetBitmapFontFromFileName(char const* imageFilePath)
{
	for (int i = 0; i < static_cast<int>(m_loadedFonts.size()); i++)
	{
		if (m_loadedFonts[i]->GetTexture().m_name == imageFilePath)
		{
			return m_loadedFonts[i];
		}
	}
	return nullptr;
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	if (blendMode == BlendMode::ALPHA) // enum class BlendMode, defined near top of Renderer.hpp
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (blendMode == BlendMode::ADDITIVE)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		ERROR_AND_DIE(Stringf("Unknown / unsupported blend mode #%i", blendMode));
	}
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
	if (m_config.m_window)
	{
		HDC displayContext = static_cast<HDC>(m_config.m_window->GetDisplayContext());
		SwapBuffers(displayContext);
	}
}	

void Renderer::Shutdown()
{

}

//void Renderer::BeginView()
//{
//	// Establish a 2D (orthographic) drawing coordinate system: (0,0) bottom-left to (10,10) top-right
//	// #SD1ToDo: This will be replaced by a call to g_renderer->BeginView( m_worldView ); or similar
//	glLoadIdentity();
//	//glOrtho(0.f, 10.f, 0.f, 10.f, 0.f, 1.f); // arguments are: xLeft, xRight, yBottom, yTop, zNear, zFar
//	glOrtho(0.f, 200.f, 0.f, 100.f, 0.f, 1.f);
//
//	//Replace by camera??
//}

void Renderer::ClearScreen(const Rgba8& clearColor)
{
	glClearColor(clearColor.r/255.f, clearColor.g/255.f, clearColor.b/255.f, clearColor.a/255.f);
	glClear(GL_COLOR_BUFFER_BIT); // ALWAYS clear the screen at the top of each frame's Render()!
}

void Renderer::BeginCamera(const Camera& camera)
{
	glLoadIdentity();
	glOrtho(camera.GetOrthoBottomLeft().x, camera.GetOrthoTopRight().x,
		camera.GetOrthoBottomLeft().y,camera.GetOrthoTopRight().y, 0.f, 1.f);
}

void Renderer::EndCamera(const Camera& camera)
{
	UNUSED(camera);
}


void Renderer::DrawVertexArray(int numVertexs, const Vertex_PCU* vertexs)
{
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < numVertexs; i++)
		{
			Rgba8 const& color = vertexs[i].m_color;
			Vec3 const& pos = vertexs[i].m_position;
			Vec2 const& uvCoord = vertexs[i].m_uvTexCoords;
			glColor4ub(color.r, color.g, color.b, color.a);
			glTexCoord2f(uvCoord.x, uvCoord.y);
			glVertex2f(pos.x,pos.y);
		}
	}
	glEnd();
}

void Renderer::DrawVertexArray(const std::vector<Vertex_PCU>& vertexs)
{
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < (int)vertexs.size(); i++)
		{
			Rgba8 const& color = vertexs[i].m_color;
			Vec3 const& pos = vertexs[i].m_position;
			Vec2 const& uv = vertexs[i].m_uvTexCoords;

			glColor4ub(color.r, color.g, color.b, color.a);
			glTexCoord2f(uv.x, uv.y);
			glVertex2f(pos.x, pos.y);
		}
	}
	glEnd();
}

Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;

	// Enable OpenGL texturing
	glEnable(GL_TEXTURE_2D);

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures(1, (GLuint*)&newTexture->m_openglTextureID);

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture(GL_TEXTURE_2D, newTexture->m_openglTextureID);

	// Set texture clamp vs. wrap (repeat) default settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP or GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	// Pick the appropriate OpenGL format (RGB or RGBA) for this texel data
	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if (bytesPerTexel == 3)
	{
		bufferFormat = GL_RGB;
	}
	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; technically allows us to translate into a different texture format as we upload to OpenGL

	// Upload the image texel data (raw pixels bytes) to OpenGL under the currently-bound OpenGL texture ID
	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		dimensions.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
		dimensions.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1, recommend 0)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
		texelData);		// Address of the actual pixel data bytes/buffer in system memory

	m_loadedTextures.push_back(newTexture);
	return newTexture;
}


//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture(Texture* texture)
{
	if (texture)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture->m_openglTextureID);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath); // You need to write this
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}


//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	IntVec2 dimensions = IntVec2::ZERO;		// This will be filled in for us to indicate image width & height
	int bytesPerTexel = 0;					// ...and how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load(imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, 0);

	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePath));

	Texture* newTexture = CreateTextureFromData(imageFilePath, dimensions, bytesPerTexel, texelData);

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free(texelData);

	return newTexture;
}

BitmapFont* Renderer::CreateOrGetBitmapFont(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	BitmapFont* existingFont = GetBitmapFontFromFileName(imageFilePath);
	if (existingFont)
	{
		return existingFont;
	}

	// Never seen this texture before!  Let's load it.
	BitmapFont* newFont = CreateBitmapFont(imageFilePath);
	m_loadedFonts.push_back(newFont);
	return newFont;
}

BitmapFont* Renderer::CreateBitmapFont(char const* imageFilePath)
{
	std::string pathWithExtension = imageFilePath;
	pathWithExtension += ".png";
	Texture* texture=CreateOrGetTextureFromFile(pathWithExtension.c_str());
	BitmapFont* newFont= new BitmapFont(imageFilePath, *texture, IntVec2(16, 16));
	return newFont;
}
