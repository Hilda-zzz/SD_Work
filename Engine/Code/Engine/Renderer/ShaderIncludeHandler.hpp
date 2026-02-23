#pragma once
#include <d3dcompiler.h>
#include <string>
#include <unordered_map>
#include <vector>

//====================================================================================
// ShaderIncludeHandler
// 
// Custom ID3DInclude implementation to support #include directives in HLSL shaders
// 
// Usage:
//   ShaderIncludeHandler includeHandler;
//   includeHandler.SetIncludeDirectory("Data/Shaders/"); // Optional: set base directory
//   
//   // Then pass &includeHandler to D3DCompile instead of nullptr
//====================================================================================
class ShaderIncludeHandler : public ID3DInclude
{
public:
    ShaderIncludeHandler();
    ~ShaderIncludeHandler();

    // Set the base directory for shader includes (e.g., "Data/Shaders/")
    void SetIncludeDirectory(const std::string& directory);

    // ID3DInclude interface implementation
    HRESULT __stdcall Open(
        D3D_INCLUDE_TYPE includeType,
        LPCSTR pFileName,
        LPCVOID pParentData,
        LPCVOID* ppData,
        UINT* pBytes) override;

    HRESULT __stdcall Close(LPCVOID pData) override;

private:
    std::string m_includeDirectory;
    std::unordered_map<const void*, std::vector<char>*> m_openedFiles;

    std::string ResolvePath(const std::string& filename, D3D_INCLUDE_TYPE includeType);
    bool ReadFileToBuffer(const std::string& filepath, std::vector<char>& outBuffer);
};
