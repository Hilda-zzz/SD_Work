#pragma once
#include "ChessBoard.hpp"
#include "ChessPlayer.hpp"
#include "ChessPiece.hpp"
#include <string>

class Game;

enum class GameState {
	PLAYING,
	CHECK,
	CHECKMATE,
	//STALEMATE,
	//DRAW_50_MOVE,
	//DRAW_REPETITION,
	//DRAW_AGREEMENT,
	//RESIGNED
};


class ChessMatch
{
public:
	ChessMatch(Game* game);
	~ChessMatch() {};
	void Renderer() const;

	std::string const& GetBoardStateString();
	void MoveTheChessPiece(std::string fromStr, std::string toStr);
	int GetTurnNumber();
	void AddTurnNum();

public:
	ChessBoard m_chessBoard;

private:
	Game* m_game = nullptr;
	GameState m_gameState = GameState::PLAYING;
	ChessPlayer m_playerBlack;
	ChessPlayer m_playerWhite;
	Faction m_currentTurnFaction=Faction::WHITE;
	int m_turnNumber=0;
};

