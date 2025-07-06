#pragma once
#include <string>
#include <vector>
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

class ObjLoader
{
public:
	ObjLoader() {};
	~ObjLoader() {};

	static void LoadObjFromFile(const std::string& filePath, std::vector<Vertex_PCU>& verts);
	static void LoadObjFromFile_WithTBN(const std::string& filePath, std::vector<Vertex_PCUTBN>& verts,float scale,Mat44 const& modelToEngineMat);
	static void LoadObjFromFile_WithIndex(const std::string& filePath, std::vector<Vertex_PCU>& verts,
		std::vector<unsigned int>& indexes);
	static void LoadObjFromFile_WithTBN_WithIndex(const std::string& filePath, std::vector<Vertex_PCUTBN>& verts,
		std::vector<unsigned int>& indexes);

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

	static void AddVertsForEachFaceLine_WithTBN_WithIndex(std::string const& faceLine, std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexs,ObjData const& objData);
	static Vertex_PCUTBN GetEachPointFromString_WithTBN_WithIndex(std::string const& vertexStr, ObjData const& objData);
};