#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Vertex_PCU.hpp"
#include "Vertex_PCUTBN.hpp"
#include "../Math/Mat44.hpp"

struct ObjData 
{
	std::vector<Vec3> m_positions;
	std::vector<Vec3> m_normals;
	std::vector<Vec2> m_uv;
	std::vector<std::string> m_faces;
};

struct VertexKey
{
	VertexKey(int pos,int uv,int norm)
		:m_posIndex(pos),m_uvIndex(uv),m_normalIndex(norm){}
	bool operator==(VertexKey const& other) const 
	{
		return m_posIndex == other.m_posIndex && 
			m_uvIndex == other.m_uvIndex && 
			m_normalIndex == other.m_normalIndex;
	}

	int m_posIndex;
	int m_uvIndex;
	int m_normalIndex;
};

struct VertexKeyHash
{
	std::size_t operator()(const VertexKey& k) const {
		return std::hash<int>()(k.m_posIndex) ^
			(std::hash<int>()(k.m_uvIndex) << 1) ^
			(std::hash<int>()(k.m_normalIndex) << 2);
	}
};

class ObjLoader
{
public:
	ObjLoader() {};
	~ObjLoader() {};

	static void LoadObjFromFile(const std::string& filePath, std::vector<Vertex_PCU>& verts);
	static void LoadObjFromFile_WithTBN(const std::string& filePath, std::vector<Vertex_PCUTBN>& verts,float scale,Mat44 const& modelToEngineMat);
// 	static void LoadObjFromFile_WithIndex(const std::string& filePath, std::vector<Vertex_PCU>& verts,
// 		std::vector<unsigned int>& indexes);
	static void LoadObjFromFile_WithTBN_WithIndex(const std::string& filePath, std::vector<Vertex_PCUTBN>& verts,
		std::vector<unsigned int>& indexes, float scale, Mat44 const& modelToEngineMat);

private:
	static ObjData LoadAndPreprocessObjFile(const std::string& filePath);

	static void CalculateTangentBitangent(
		const Vec3& pos1, const Vec3& pos2, const Vec3& pos3,
		const Vec2& uv1, const Vec2& uv2, const Vec2& uv3,
		Vec3& tangent, Vec3& bitangent);

	static void AddVertsForEachFaceLine(std::string const& faceLine, std::vector<Vertex_PCU>& verts,ObjData const& objData);
	static Vertex_PCU GetEachPointFromString(std::string const& vertexStr, ObjData const& objData);
	

	static void AddVertsForEachFaceLine_WithTBN(std::string const& faceLine, std::vector<Vertex_PCUTBN>& verts, ObjData const& objData);
	static Vertex_PCUTBN GetEachPointFromString_WithTBN(std::string const& vertexStr, ObjData const& objData);

	static void CollectUniqueIndexedVertexEachFaceLine(std::string const& faceLine,
		std::vector<Vertex_PCUTBN>& vertices, std::vector<unsigned int>& indices,
		std::unordered_map<VertexKey, unsigned int, VertexKeyHash>& keyToIndex,
		ObjData const& objData, float scale, Mat44 const& transform);
	static VertexKey PareseEachVertexKey(std::string const& vertexStr);
	static Vertex_PCUTBN CreateVertexFromKey(VertexKey const& key, ObjData const& objData,
		float scale, Mat44 const& transform);
};