#include "Engine/Renderer/ShaderIncludeHandler.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <fstream>
#include <sstream>
#include <Engine/Core/EngineCommon.hpp>

//====================================================================================
ShaderIncludeHandler::ShaderIncludeHandler()
    : m_includeDirectory("")
{
}

//====================================================================================
ShaderIncludeHandler::~ShaderIncludeHandler()
{
    // Clean up any files that weren't properly closed
    for (auto& pair : m_openedFiles)
    {
        delete pair.second;
    }
    m_openedFiles.clear();
}

//====================================================================================
void ShaderIncludeHandler::SetIncludeDirectory(const std::string& directory)
{
    m_includeDirectory = directory;
    
    // Ensure directory ends with a slash
    if (!m_includeDirectory.empty() && 
        m_includeDirectory.back() != '/' && 
        m_includeDirectory.back() != '\\')
    {
        m_includeDirectory += '/';
    }
}

//====================================================================================
std::string ShaderIncludeHandler::ResolvePath(const std::string& filename, D3D_INCLUDE_TYPE includeType)
{
    // D3D_INCLUDE_LOCAL: search relative to the including file (we'll use base directory)
    // D3D_INCLUDE_SYSTEM: search in system directories (we'll also use base directory)
    
    std::string fullPath;
    
    // Try with base directory first
    if (!m_includeDirectory.empty())
    {
        fullPath = m_includeDirectory + filename;
    }
    else
    {
        fullPath = filename;
    }
    
    return fullPath;
}

//====================================================================================
bool ShaderIncludeHandler::ReadFileToBuffer(const std::string& filepath, std::vector<char>& outBuffer)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open())
    {
        DebuggerPrintf("ShaderIncludeHandler: Failed to open include file: %s\n", filepath.c_str());
        return false;
    }
    
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    outBuffer.resize(static_cast<size_t>(fileSize));
    
    if (!file.read(outBuffer.data(), fileSize))
    {
        DebuggerPrintf("ShaderIncludeHandler: Failed to read include file: %s\n", filepath.c_str());
        return false;
    }
    
    file.close();
    return true;
}

//====================================================================================
HRESULT ShaderIncludeHandler::Open(
    D3D_INCLUDE_TYPE includeType,
    LPCSTR pFileName,
    LPCVOID pParentData,
    LPCVOID* ppData,
    UINT* pBytes)
{
    UNUSED(pParentData); // Could be used to track parent files for better path resolution
    
    if (pFileName == nullptr || ppData == nullptr || pBytes == nullptr)
    {
        return E_INVALIDARG;
    }
    
    // Resolve the full path to the include file
    std::string fullPath = ResolvePath(pFileName, includeType);
    
    // Allocate a new buffer for this file
    std::vector<char>* fileBuffer = new std::vector<char>();
    
    // Read the file
    if (!ReadFileToBuffer(fullPath, *fileBuffer))
    {
        delete fileBuffer;
        return E_FAIL;
    }
    
    // Return the data and size
    *ppData = fileBuffer->data();
    *pBytes = static_cast<UINT>(fileBuffer->size());
    
    // Track the opened file so we can delete it later
    m_openedFiles[*ppData] = fileBuffer;
    
    return S_OK;
}

//====================================================================================
HRESULT ShaderIncludeHandler::Close(LPCVOID pData)
{
    if (pData == nullptr)
    {
        return E_INVALIDARG;
    }
    
    // Find the buffer associated with this data pointer
    auto it = m_openedFiles.find(pData);
    if (it == m_openedFiles.end())
    {
        return E_FAIL;
    }
    
    // Delete the buffer and remove from tracking
    delete it->second;
    m_openedFiles.erase(it);
    
    return S_OK;
}
