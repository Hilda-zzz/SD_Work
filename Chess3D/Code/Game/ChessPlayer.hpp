#pragma once
#include "ChessPiece.hpp"

class ChessPlayer
{
public:
	ChessPlayer();
	ChessPlayer(Faction faction);
	~ChessPlayer();

private:
	Faction m_faction=Faction::WHITE;
};

