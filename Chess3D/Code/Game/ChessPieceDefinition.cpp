#include "ChessPieceDefinition.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/OBB3.hpp"
extern Renderer* g_theRenderer;

std::vector<ChessPieceDefinition*> ChessPieceDefinition::s_chessPieceDefs;

ChessPieceDefinition::ChessPieceDefinition(XmlElement const* chessDefElement)
{
	std::string type = "?";
	type=ParseXmlAttribute(chessDefElement, "type", type);
	if (type!="?")
	{
		m_vertexBufferByPlyaer[0] = g_theRenderer->CreateVertexBuffer(2000, sizeof(Vertex_PCUTBN));
		m_vertexBufferByPlyaer[1] = g_theRenderer->CreateVertexBuffer(2000, sizeof(Vertex_PCUTBN));
		m_indexBufferByPlyaer[0] = g_theRenderer->CreateIndexBuffer(2000);
		m_indexBufferByPlyaer[1] = g_theRenderer->CreateIndexBuffer(2000);

		if (type == "KING")
		{
			m_type = PieceType::KING;
		}
		else if (type == "QUEEN")
		{
			m_type = PieceType::QUEEN;
		}
		else if (type == "ROOK")
		{
			m_type = PieceType::ROOK;
		}
		else if (type == "BISHOP")
		{
			m_type = PieceType::BISHOP;
		}
		else if (type == "KNIGHT")
		{
			m_type = PieceType::KNIGHT;
		}
		else if (type == "PAWN")
		{
			m_type = PieceType::PAWN;
		}
		else
		{
			m_type = PieceType::NONE;
		}
	}

	std::string glyph = "?";
	glyph = ParseXmlAttribute(chessDefElement, "glyph", glyph);
	m_glyph = glyph[0];
	
	XmlElement const* geometryElement = chessDefElement->FirstChildElement("Geometry");
	if (geometryElement)
	{
		std::vector<Vertex_PCUTBN> meshVerts;
		std::vector<unsigned int>  meshIndexs;
		for (XmlElement const*modelComponentElement = geometryElement->FirstChildElement("Component");
			modelComponentElement != nullptr;
			modelComponentElement = modelComponentElement->NextSiblingElement("Component"))
		{
			std::string componentType = "?";
			componentType = ParseXmlAttribute(modelComponentElement, "type", type);
			if (componentType != "?")
			{
				if (componentType == "AABB3")
				{
					Vec3 bl= ParseXmlAttribute(modelComponentElement, "bl", bl);
					Vec3 tr = ParseXmlAttribute(modelComponentElement, "tr", tr);
					AddVertsForAABB3D_WithTBN(meshVerts, meshIndexs, bl, tr, Rgba8::WHITE, AABB2::ZERO_TO_ONE);
				}
				else if (componentType == "OBB3")
				{
					Vec3 center= ParseXmlAttribute(modelComponentElement, "center", center);
					EulerAngles euler = ParseXmlAttribute(modelComponentElement, "euler", euler);
					Vec3 halfDimensions = ParseXmlAttribute(modelComponentElement, "halfDimensions", halfDimensions);
					Vec3 i, j, k;
					euler.GetAsVectors_IFwd_JLeft_KUp(i, j, k);
					OBB3 obb=OBB3(i,j,k,halfDimensions,center);
					AddVertsForOBB3D_WithTBN(meshVerts, meshIndexs, obb);
				}
				else if (componentType == "ZCylinder")
				{
					Vec3 start = ParseXmlAttribute(modelComponentElement, "start", start);
					Vec3 end = ParseXmlAttribute(modelComponentElement, "end", end);
					float radius = 0.1f;
					radius = ParseXmlAttribute(modelComponentElement, "radius", radius);
					AddVertsForCylinder3D_WithTBN(meshVerts, meshIndexs, start, end, radius);
				}
				else if (componentType == "ZSphere")
				{
					Vec3 position = ParseXmlAttribute(modelComponentElement, "position", position);
					
					float radius = 0.1f;
					radius=ParseXmlAttribute(modelComponentElement, "radius", radius);
					AddVertsForSphere3D_WithTBN(meshVerts, meshIndexs, position, radius);
				}
				else if (componentType == "ZCone")
				{

				}
			}
		}
		g_theRenderer->CopyGameVertexBufferToGPU(meshVerts.data(), (int)meshVerts.size(), m_vertexBufferByPlyaer[0]);
		g_theRenderer->CopyGameVertexBufferToGPU(meshVerts.data(), (int)meshVerts.size(), m_vertexBufferByPlyaer[1]);
		g_theRenderer->CopyGameIndexBufferToGPU(meshIndexs.data(), (int)meshIndexs.size(), m_indexBufferByPlyaer[0]);
		g_theRenderer->CopyGameIndexBufferToGPU(meshIndexs.data(), (int)meshIndexs.size(), m_indexBufferByPlyaer[1]);
	}
}

ChessPieceDefinition::~ChessPieceDefinition()
{
	for (VertexBuffer* buffer : m_vertexBufferByPlyaer)
	{
		delete buffer;
		buffer = nullptr;
	}

	for (IndexBuffer* buffer : m_indexBufferByPlyaer)
	{
		delete buffer;
		buffer = nullptr;
	}
}

void ChessPieceDefinition::InitializeChessDefinitionFromFile()
{
	XmlDocument chessDefsXml;
	char const* filePath = "Data/Definitions/ChessPieceDefinitions.xml";
	XmlResult result = chessDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required ChessPieceDefinitions \"%s\"", filePath));

	XmlElement* rootElement = chessDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Faile to find map root element");

	XmlElement* chessDefElement = rootElement->FirstChildElement();
	while (chessDefElement)
	{
		std::string elementName = chessDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "PieceDefinition", Stringf("Root child element in %s was <%s>, must be <PieceDefinition>!", filePath, elementName.c_str()));
		ChessPieceDefinition* newChessDef = new ChessPieceDefinition(chessDefElement);
		s_chessPieceDefs.push_back(newChessDef);
		chessDefElement = chessDefElement->NextSiblingElement();
	}
}
