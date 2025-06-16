#pragma once
#include "Game/ChessObject.hpp"
#include <vector>
#include <string>
#include "Game/ChessPiece.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

class ChessPiece;
struct IntVec2;
class VertexBuffer;
class IndexBuffer;
class Shader;

class ChessBoard:public ChessObject
{
public:
	//ChessBoard() {};
	ChessBoard(ChessMatch* match);
	~ChessBoard();

	void Update();
	void Renderer() const;

	void UpdateDebugKeyInput();

	std::string const& UpdateAndGetBoardStateString();
	char GetPieceChar(PieceType type, Faction faction);
	int GetIndexFromGridPos(IntVec2 const& gridPos);
	int GetIndexFromCharNum(std::string const& charNum);
	IntVec2 GetGridPosFromIndex(int index);
	ChessPiece* GetChessFromIndex(int index);
	ChessPiece* GetChessFromGridPos(IntVec2 gridPos);
	bool DestroyChess(ChessPiece* curPiece);
	bool IsGridPosValid(IntVec2 gridPos);

	int GetRenderDebugInt();

private:
	void PopulateDefaultChess();
	void AddVertsForBoard();
	
public:
	//ChessMatch* m_match = nullptr;
	ChessPiece* m_lastMovingChess = nullptr;

private:
	std::vector<ChessPiece*> m_chessPieces;
	std::string m_boardStateVisionString;

	Shader* m_shader = nullptr;
	std::vector<Vertex_PCUTBN>	m_vertexs;
	std::vector<unsigned int>	m_indexs;
 	VertexBuffer* m_vertexBuffer = nullptr;
 	IndexBuffer* m_indexBuffer = nullptr;

	//Light
	Vec3 m_sunDirection = Vec3(10.f, 2.f, -3.f);
	float m_sunIntensity = 1.f;
	float m_ambientIntensity = 0.4f;

	// debug int
	int m_debugInt = 0;

	Texture* m_diffuseTex = nullptr;
	Texture* m_normalTex = nullptr;
};

