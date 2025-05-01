#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <vector>

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class TextureCube
{
	friend class Renderer; 

public:

	enum Face
	{
		POSITIVE_X = 0,    
		NEGATIVE_X = 1,    
		POSITIVE_Y = 2,    
		NEGATIVE_Y = 3,    
		POSITIVE_Z = 4,    
		NEGATIVE_Z = 5     
	};

private:
	TextureCube() {}; 
	TextureCube(TextureCube const& copy) = delete; 
	~TextureCube();

public:
	IntVec2 GetDimensions() const { return m_dimensions; }
	//const std::string& GetImageFilePath(Face face) const { return m_filePaths[face]; }
	std::string const& GetAllImageFilePaths() const { return m_firstPath; }
	ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shaderResourceView; }

protected:
	std::string m_firstPath;  
	IntVec2 m_dimensions;                  
	ID3D11Texture2D* m_texture = nullptr; 
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;  
};