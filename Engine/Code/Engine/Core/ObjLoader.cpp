#include "ObjLoader.hpp"
#include <fstream>
#include "ErrorWarningAssert.hpp"
#include "StringUtils.hpp"
#include <string>
#include <sstream>
#include <stdlib.h>
#include "../Math/MathUtils.hpp"

void ObjLoader::LoadObjFromFile(const std::string& filePath, std::vector<Vertex_PCU>& verts)
{
	ObjData data=LoadAndPreprocessObjFile(filePath);

	if (data.m_faces.size() != 0)
	{
		for (std::string const& faceLine : data.m_faces)
		{
			AddVertsForEachFaceLine(faceLine, verts, data);
		}
	}
}

void ObjLoader::LoadObjFromFile_WithTBN(const std::string& filePath, std::vector<Vertex_PCUTBN>& verts, float scale, Mat44 const& modelToEngineMat)
{
	ObjData data = LoadAndPreprocessObjFile(filePath);
	if (data.m_faces.size() != 0)
	{
		for (std::string const& faceLine : data.m_faces)
		{
			AddVertsForEachFaceLine_WithTBN(faceLine, verts, data);
		}

		for (Vertex_PCUTBN& vertex : verts)
		{
			vertex.m_position *= scale;
			vertex.m_position = modelToEngineMat.TransformPosition3D(vertex.m_position);
		}
	}
}

// void ObjLoader::LoadObjFromFile_WithIndex(const std::string& filePath, std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes)
// {
// 	ObjData data = LoadAndPreprocessObjFile(filePath);
// }

void ObjLoader::LoadObjFromFile_WithTBN_WithIndex(const std::string& filePath, std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices,
	float scale, Mat44 const& modelToEngineMat)
{
	ObjData data = LoadAndPreprocessObjFile(filePath);
	std::unordered_map<VertexKey, Vertex_PCUTBN, VertexKeyHash> vertexIndexMap;
	std::unordered_map<VertexKey, unsigned int, VertexKeyHash> keyToIndex;

	if (data.m_faces.size() != 0)
	{
		for (std::string const& faceLine : data.m_faces)
		{
			CollectUniqueIndexedVertexEachFaceLine(faceLine, verts, indices, keyToIndex, data, scale, modelToEngineMat);
		}
	}
}

ObjData ObjLoader::LoadAndPreprocessObjFile(const std::string& filePath)
{
	ObjData data;

	std::ifstream curObjfile(filePath);

	if (!curObjfile.is_open()) 
	{
	    ERROR_AND_DIE(Stringf("Failed to open Obj file:  \"%s\".", filePath.c_str()));
	}

	std::string curLine;
	while (std::getline(curObjfile, curLine))
	{
		if (curLine.empty() || curLine[0] == '#')
		{
			continue;
		}

		std::istringstream iss(curLine);
		std::string prefix;           
		iss >> prefix;

		if (prefix == "v")
		{
			float x, y, z;
			iss >> x >> y >> z;
			data.m_positions.push_back(Vec3(x, y, z));
		}
		else if (prefix == "vn")
		{
			float x, y, z;
			iss >> x >> y >> z;
			data.m_normals.push_back(Vec3(x, y, z));
		}
		else if (prefix == "vt")
		{
			float u, v;
			iss >> u >> v;
			data.m_uv.push_back(Vec2(u, v));
		}
		else if (prefix == "f")
		{
			data.m_faces.push_back(curLine);
		}
	}

	curObjfile.close();
	return data;
}

void ObjLoader::CalculateTangentBitangent(const Vec3& pos0, const Vec3& pos1, const Vec3& pos2, const Vec2& uv0, const Vec2& uv1, const Vec2& uv2, Vec3& tangent, Vec3& bitangent)
{
	Vec3 e0 = pos1 - pos0;
	Vec3 e1 = pos2 - pos0;
	float delta_u0 =(uv1.x - uv0.x);
	float delta_u1 = (uv2.x - uv0.x);
	float delta_v0 = (uv1.y - uv0.y);
	float delta_v1 = (uv2.y - uv0.y);

	float r = 1.f / (delta_u0 * delta_v1 - delta_u1 * delta_v0);
	tangent = (r * (delta_v1 * e0 - delta_v0 * e1)).GetNormalized();
	bitangent =(r * (delta_u0 * e1 - delta_u1 * e0)).GetNormalized();
}

void ObjLoader::AddVertsForEachFaceLine(std::string const& faceLine, std::vector<Vertex_PCU>& verts, ObjData const& objData)
{
	std::istringstream iss(faceLine);
	std::string prefix;
	iss >> prefix;
	std::string vertexA_str, vertexB_str, vertexC_str;
	iss >> vertexA_str >> vertexB_str >> vertexC_str;

	if (vertexA_str.empty() || vertexB_str.empty() || vertexC_str.empty())
	{
		ERROR_AND_DIE(Stringf("Obj format wrong!"));
	}
	
	Vertex_PCU vertexA = GetEachPointFromString(vertexA_str,objData);
	Vertex_PCU vertexB = GetEachPointFromString(vertexB_str,objData);
	Vertex_PCU vertexC = GetEachPointFromString(vertexC_str,objData);

	verts.push_back(vertexA);
	verts.push_back(vertexB);
	verts.push_back(vertexC);
}

Vertex_PCU ObjLoader::GetEachPointFromString(std::string const& vertexStr, ObjData const& objData)
{
	Strings splitStrs = SplitStringOnDelimiter(vertexStr, '/');
	int nonEmptyCount = 0;

	int vertexIndex=-1, uvIndex=-1;
	for (std::string const& element : splitStrs)
	{
		// vertex index/ texture coordinate/ normal
		if (!element.empty())
		{
			int curIndex = std::stoi(element)-1;
			switch (nonEmptyCount)
			{
			case 0:  // vertex index
				vertexIndex = curIndex;
				nonEmptyCount++;
				break;
			case 1:  // texture coordinate index
				uvIndex = curIndex;
				nonEmptyCount++;
				break;
			default:
				break;
			}
		}
	}
	if (vertexIndex == -1)
	{
		ERROR_AND_DIE(Stringf("The obj lack a vertex in a face line!"));
	}
	Vec2 uv = Vec2::ONE;
	if (uvIndex != -1)
	{
		uv = objData.m_uv[uvIndex];
	}
	return Vertex_PCU(objData.m_positions[vertexIndex], Rgba8::WHITE, uv);
}


void ObjLoader::AddVertsForEachFaceLine_WithTBN(std::string const& faceLine, std::vector<Vertex_PCUTBN>& verts, ObjData const& objData)
{
	std::istringstream iss(faceLine);
	std::string prefix;
	iss >> prefix;
	std::string vertexA_str, vertexB_str, vertexC_str;
	iss >> vertexA_str >> vertexB_str >> vertexC_str;

	if (vertexA_str.empty() || vertexB_str.empty() || vertexC_str.empty())
	{
		ERROR_AND_DIE(Stringf("Obj format wrong!"));
	}

	bool aHasNormal, bHasNormal, cHasNormal;
	Vertex_PCUTBN vertexA = GetEachPointFromString_WithTBN(vertexA_str, objData, aHasNormal);
	Vertex_PCUTBN vertexB = GetEachPointFromString_WithTBN(vertexB_str, objData, bHasNormal);
	Vertex_PCUTBN vertexC = GetEachPointFromString_WithTBN(vertexC_str, objData, cHasNormal);

	//-------------- Check if not have normals--------
	bool needCalculateNormal = (objData.m_normals.empty() ||
		!aHasNormal ||
		!bHasNormal ||
		!cHasNormal);

	if (needCalculateNormal)
	{
		Vec3 calculatedNormal = CalculateTriangleNormal(vertexA.m_position,
			vertexB.m_position,
			vertexC.m_position);
		vertexA.m_normal = calculatedNormal;
		vertexB.m_normal = calculatedNormal;
		vertexC.m_normal = calculatedNormal;
	}

	//------------------------------------------------
	Vec3 T = Vec3(1.f, 0.f, 0.f);
	Vec3 B = Vec3(0.f, 1.f, 0.f);
	if (!objData.m_uv.empty())
	{
		// Vec3 T, B = Vec3(0.f, 0.f, 0.f);
		CalculateTangentBitangent(vertexA.m_position, vertexB.m_position, vertexC.m_position,
			vertexA.m_uvTexCoords, vertexB.m_uvTexCoords, vertexC.m_uvTexCoords,
			T, B);
	}
	vertexA.m_tangent = T;
	vertexA.m_bitangent = B;
	vertexB.m_tangent = T;
	vertexB.m_bitangent = B;
	vertexC.m_tangent = T;
	vertexC.m_bitangent = B;

	verts.push_back(vertexA);
	verts.push_back(vertexB);
	verts.push_back(vertexC);
}

Vertex_PCUTBN ObjLoader::GetEachPointFromString_WithTBN(std::string const& vertexStr, ObjData const& objData,bool& hasNormal)
{
	hasNormal = true;

	Strings splitStrs = SplitStringOnDelimiter(vertexStr, '/');
	int nonEmptyCount = 0;

	int vertexIndex = -1, uvIndex = -1,normalIndex=-1;
	for (std::string const& element : splitStrs)
	{
		// vertex index/ texture coordinate/ normal
		if (!element.empty())
		{
			int curIndex = std::stoi(element) - 1;
			switch (nonEmptyCount)
			{
			case 0:  // vertex index
				vertexIndex = curIndex;
				nonEmptyCount++;
				break;
			case 1:  // texture coordinate index
				uvIndex = curIndex;
				nonEmptyCount++;
				break;
			case 2:  // normal index
				normalIndex = curIndex;
				nonEmptyCount++;
				break;
			default:
				break;
			}
		}
	}
	if (vertexIndex == -1)
	{
		ERROR_AND_DIE(Stringf("The obj lack a vertex in a face line!"));
	}
	Vec2 uv = Vec2::ONE;
	if (uvIndex != -1)
	{
		uv = objData.m_uv[uvIndex];
	}
	Vec3 normal= Vec3(0.f,0.f,1.f);
	if (normalIndex != -1)
	{
		normal = objData.m_normals[normalIndex];
	}
	else
	{
		hasNormal = false;
	}
	return Vertex_PCUTBN(objData.m_positions[vertexIndex], Rgba8::WHITE, uv, 
		Vec3(1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), normal);
}

void ObjLoader::CollectUniqueIndexedVertexEachFaceLine(std::string const& faceLine,
	std::vector<Vertex_PCUTBN>& vertices, std::vector<unsigned int>& indices,
	std::unordered_map<VertexKey, unsigned int, VertexKeyHash>& keyToIndex,
	ObjData const& objData, float scale, Mat44 const& transform)
{
	std::basic_istringstream iss(faceLine);
	std::string prefix;
	iss >> prefix;

	std::vector<unsigned int> faceIndices;
	std::string vertexString;

	while (iss >> vertexString)
	{
		VertexKey key=PareseEachVertexKey(vertexString);
		auto it = keyToIndex.find(key);
		unsigned int vertexIndex;
		if (keyToIndex.find(key) == keyToIndex.end()) 
		{
			Vertex_PCUTBN newVertex = CreateVertexFromKey(key, objData, scale, transform);
			vertexIndex = static_cast<unsigned int>(vertices.size());
			vertices.push_back(newVertex);
			keyToIndex[key] = vertexIndex;
		}
		else
		{
			vertexIndex = it->second;
		}
		faceIndices.push_back(vertexIndex);
	}

	if (faceIndices.size() >= 3) 
	{
		for (size_t i = 1; i < faceIndices.size() - 1; i++) 
		{
			unsigned int a = faceIndices[0];
			unsigned int b = faceIndices[i];
			unsigned int c = faceIndices[i + 1];

			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);

			Vec3 tangent, bitangent;
			CalculateTangentBitangent(
				vertices[a].m_position, vertices[b].m_position, vertices[c].m_position,
				vertices[a].m_uvTexCoords, vertices[b].m_uvTexCoords, vertices[c].m_uvTexCoords,
				tangent, bitangent);
			vertices[a].m_tangent = tangent;
			vertices[a].m_bitangent = bitangent;
			vertices[b].m_tangent = tangent;
			vertices[b].m_bitangent = bitangent;
			vertices[c].m_tangent = tangent;
			vertices[c].m_bitangent = bitangent;
		}
	}
}


VertexKey ObjLoader::PareseEachVertexKey(std::string const& vertexStr)
{
	Strings splitStrs = SplitStringOnDelimiter(vertexStr, '/');
	int nonEmptyCount = 0;

	int vertexIndex = -1, uvIndex = -1, normalIndex = -1;
	for (std::string const& element : splitStrs)
	{
		// vertex index/ texture coordinate/ normal
		if (!element.empty())
		{
			int curIndex = std::stoi(element) - 1;
			switch (nonEmptyCount)
			{
			case 0:  // vertex index
				vertexIndex = curIndex;
				nonEmptyCount++;
				break;
			case 1:  // texture coordinate index
				uvIndex = curIndex;
				nonEmptyCount++;
				break;
			case 2:  // normal index
				normalIndex = curIndex;
				nonEmptyCount++;
				break;
			default:
				break;
			}
		}
	}
	if (vertexIndex == -1)
	{
		ERROR_AND_DIE(Stringf("The obj lack a vertex in a face line!"));
	}

	return VertexKey(vertexIndex, uvIndex, normalIndex);
}

Vertex_PCUTBN ObjLoader::CreateVertexFromKey(VertexKey const& key, ObjData const& objData, float scale, Mat44 const& transform)
{
	Vertex_PCUTBN vertex;

	if (key.m_posIndex >= 0 && key.m_posIndex < (int)objData.m_positions.size())
	{
		vertex.m_position = objData.m_positions[key.m_posIndex] * scale;
		vertex.m_position = transform.TransformPosition3D(vertex.m_position);
	}

	if (key.m_uvIndex >= 0 && key.m_uvIndex < (int)objData.m_uv.size())
	{
		vertex.m_uvTexCoords = objData.m_uv[key.m_uvIndex];
	}

	if (key.m_normalIndex >= 0 && key.m_normalIndex < (int)objData.m_normals.size())
	{
		vertex.m_normal = objData.m_normals[key.m_normalIndex];
		vertex.m_normal = transform.TransformPosition3D(vertex.m_normal).GetNormalized();
	}

	vertex.m_color = Rgba8::WHITE;

	vertex.m_tangent = Vec3(1, 0, 0);
	vertex.m_bitangent = Vec3(0, 1, 0);

	return vertex;
}

Vec3 ObjLoader::CalculateTriangleNormal(Vec3 const& posA, Vec3 const& posB, Vec3 const& posC)
{
	Vec3 edge1 = posB - posA;  // AB
	Vec3 edge2 = posC - posA;  // AC

	Vec3 normal = CrossProduct3D(edge1, edge2);
	normal.Normalized();

	return normal;
}

// bool ObjLoader::HasValidNormalInFaceLine(std::string const& vertexStr)
// {
// 	Strings splitStrs = SplitStringOnDelimiter(vertexStr, '/');
// 	return (splitStrs.size() >= 3 && !splitStrs[2].empty());
// }

// void ObjLoader::BuildVertexAndIndexArrays(ObjData const& objData, 
// 	std::unordered_map<VertexKey, Vertex_PCUTBN, VertexKeyHash> const& vertexMap, 
// 	std::vector<Vertex_PCUTBN>& vertices, std::vector<unsigned int>& indices)
// {
// 	std::unordered_map<VertexKey, unsigned int, VertexKeyHash> keyToIndex;
// 
// 	// vertices
// 	unsigned int currentIndex = 0;
// 	for (const auto& pair : vertexMap) 
// 	{
// 		vertices.push_back(pair.second);
// 		keyToIndex[pair.first] = currentIndex;
// 		currentIndex++;
// 	}
// 	// indices
// 	for (const std::string& faceLine : objData.m_faces) 
// 	{
// 		AddIndicesForFace(faceLine, keyToIndex, indices);
// 	}
// }

// void ObjLoader::AddVertsForEachFaceLine_WithTBN_WithIndex(std::string const& faceLine,
// 	std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexs, 
// 	std::unordered_map<VertexKey, Vertex_PCUTBN, VertexKeyHash>& vertexMap, ObjData const& objData)
// {
// 
// }

