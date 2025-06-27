#pragma once
#include <vector>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/ZCylinder.hpp"

class IndexBuffer;
class VertexBuffer;

enum class PieceType {
	NONE,
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT,
	PAWN,
	NUM
};

class ChessPieceDefinition
{
	friend class Renderere;

public:
	ChessPieceDefinition(XmlElement const* chessDefElement);
	~ChessPieceDefinition();
	
	static void InitializeChessDefinitionFromFile();
	static std::vector<ChessPieceDefinition*> s_chessPieceDefs;

public:
	PieceType m_type = PieceType::NONE;
	char m_glyph = '?';

	// 0 white, 1 black
	IndexBuffer* m_indexBufferByPlyaer[2] = {};
	VertexBuffer* m_vertexBufferByPlyaer[2] = {};
	ZCylinder m_collider;
};