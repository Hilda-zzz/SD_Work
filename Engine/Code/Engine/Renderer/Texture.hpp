#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>

// 5. Test imaging loading in ProtogameCmd or similar; load named image from a path starting with "Data/" and
//	print out the image's width and height, bytes-per-pixel (3 for RGB or 4 for RGBA), and the Rgba8 byte colors
//	of the first four texels in the image data (and also the last texel).

// 6. Create a Texture class (using the example below) in Renderer/Texture.hpp and .cpp

// 7. Write Renderer::CreateTextureFromData (using the example below), ignoring (commenting out) the m_loadedTextures.push_back for now

// 8. Write Renderer::BindTexture (using the example below)

// 9. In Protogame2D (or TestGame2D), draw a simple large square AABB2 without texturing in Attract mode.  Make sure it works.

// 10. In Protogame2D's Game::RenderAttract, create a "fake" 4x4 image in temp stack local memory with very specific Rgba8 colors in it;
//	For example:  top row, left to right: Cyan, Magenta, Yellow, Black; then two rows of checkerboard black and brown (200,100,50,255);
//	then the bottom row, left to right: Red, Green, Blue, White.

// 11. Use g_theRenderer->CreateTextureFromData function to create a 4x4 texture (x 4-channel RGBA per texel) from the dummy image data

// 12. Change your RenderAttract square to use textured rendering (see the SomeGameFunction usage example below).

// 13. Write Renderer::CreateTextureFromFile (see example), which uses STB_image to get the texture data, then calls CreateTextureFromData.

// 14. Try to get your test square to load the Test_StbiFlippedAndOpenGL.png test image and draw it textured.

// 15. Create a "registry" in the Renderer which stores a record of all textures previously created.  Whenever any texture is created by
//	CreateTextureFromData (which will be 100% of the time), add it to the registry.

// 16. Write Renderer::CreateOrGetTextureFromFile, which first checks the registry to see if a texture of that name already exists;
//	do this by calling GetTextureForFileName, which will just return the existing Texture* (or nullptr if no such texture exists).

// 17. Make sure everything works.

// 18. Place breakpoints in a Debug build on CreateTextureFromData and CreateTextureFromFile.  Call CreateOrGetTextureFromFile every
//	frame; make sure that they are each called exactly once -- the first time CreateOrGetTextureFromFile is called; after that, they
//	should not get called again (since the texture should be in the Renderer's registry of already-loaded textures, and just get
//	returned by GetTextureForFileName directly).

// 19. Profit.


//-----------------------------------------------------------------------------------------------
// Sample code for loading an image from disk and creating an OpenGL texture from its data.
// 
// Game code calls Renderer::CreateOrGetTextureFromFile(), which in turn will
//	check that name amongst the registry of already-loaded textures (by name).  If that image
//	has already been loaded, the renderer simply returns the Texture* it already has.  If the
//	image has not been loaded before, CreateTextureFromFile() gets called internally, which in
//	turn calls CreateTextureFromData.  The new Texture* is then added to the registry of
//	already-loaded textures, and then returned.
//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------
class Texture
{
	friend class Renderer; // Only the Renderer can create new Texture objects!

private:
	Texture() {}; // can't instantiate directly; must ask Renderer to do it for you
	Texture(Texture const& copy) = delete; // No copying allowed!  This represents GPU memory.
	~Texture();

public:
	IntVec2				GetDimensions() const { return m_dimensions; }
	std::string const& GetImageFilePath() const { return m_name; }

protected:
	std::string			m_name;
	IntVec2				m_dimensions;

	// #ToDo in SD2: Use #if defined( ENGINE_RENDER_D3D11 ) to do something different for DX11; #else do:
	unsigned int		m_openglTextureID = 0xFFFFFFFF;
};


//------------------------------------------------------------------------------------------------
// Recommended alternate override for your Renderer::DrawVertexArray, which is more
//	convenient for when you happen to have a std::vector<Vertex_PCU>, which is often the case.
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------
// Sample usage example code #1 (draws the test texture as a simple quad in TestGame or attract):
//-----------------------------------------------------------------------------------------------

