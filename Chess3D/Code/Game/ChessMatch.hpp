#pragma once
#include "ChessBoard.hpp"
#include "ChessPlayer.hpp"
#include "ChessPiece.hpp"
#include <string>
#include "Game/Game.hpp"

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
	void Update(float deltaTime);
	void Renderer() const;

	std::string const& GetBoardStateString();
	void MoveTheChessPiece(std::string fromStr, std::string toStr, ChessMoveResult moveResult);
	void ChangeChessType(std::string posStr, PieceType pieceType);
	int GetTurnNumber();
	Faction GetCurFaction();
	void AddTurnNum();


public:
	ChessBoard m_chessBoard;
	bool m_isMovingChess = false;
	Game* m_game = nullptr;
	bool m_selfIsFirst = true;
	int m_turnNumber = 0;
	Faction m_currentTurnFaction = Faction::WHITE;
	bool m_isOpponentOfferedDraw = false;
	bool m_isSelfOfferedDraw = false;
private:
	GameState m_gameState = GameState::PLAYING;
	ChessPlayer m_playerBlack;
	ChessPlayer m_playerWhite;
	
	
};

