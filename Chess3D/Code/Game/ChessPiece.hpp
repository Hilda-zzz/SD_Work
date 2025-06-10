#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/ChessObject.hpp"
#include "Game/ChessPieceDefinition.hpp"

class Texture;

enum class Faction {
	NONE,
	BLACK,
	WHITE
};

class ChessPiece:public ChessObject
{
public:
	ChessPiece(Faction facion,PieceType type,IntVec2 const& gridPos);
	~ChessPiece() {};

	void Renderer() const;

	IntVec2 GetGridPos();
	void SetGridPos(IntVec2 const& gridPos);
	IntVec2 GetPrevGridPos();
	void SetPrevGridPos(IntVec2 const& gridPos);
	PieceType GetPieceType();
	void SetPieceType(PieceType newType);
	Faction GetFaction();
	bool GetIsFirstMove();
	void SetIsFirstMove(bool isFirstMove);

	Vec3 CalculateCurPosition() const;

private:
	ChessPieceDefinition* m_def = nullptr;
	Faction m_faction = Faction::NONE;
	PieceType m_pieceType = PieceType::NONE;
	IntVec2 m_gridPos = IntVec2(0, 0);
	IntVec2 m_prevCoord = IntVec2(0, 0);
	EulerAngles m_orientaion = EulerAngles(0.f, 0.f, 0.f);
	Texture* m_texWhite = nullptr;
	Texture* m_texBlack = nullptr;
	bool m_isFirstMove = true;

	//new 
	float secondSinceMoved;
	int m_turnLastMoved;
};
