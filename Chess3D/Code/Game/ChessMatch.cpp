#include "ChessMatch.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;


ChessMatch::ChessMatch(Game* game):m_game(game),m_chessBoard(this)
{
	//m_chessBoard = ChessBoard(this);
}

void ChessMatch::Update()
{
	m_chessBoard.Update();
}

void ChessMatch::Renderer() const
{
	m_chessBoard.Renderer();
}

std::string const& ChessMatch::GetBoardStateString()
{
	return m_chessBoard.UpdateAndGetBoardStateString();
}

void ChessMatch::MoveTheChessPiece(std::string fromStr, std::string toStr,ChessMoveResult moveResult)
{	
	//move the chess
	char fromCol = fromStr[0];
	char toCol = toStr[0];
	char fromRow = fromStr[1];
	char toRow = toStr[1];
	IntVec2 toPos(toCol - 'a', toRow - '1');
	IntVec2 fromPos(fromCol - 'a', fromRow - '1');

	int fromIndex = m_chessBoard.GetIndexFromCharNum(fromStr);
	ChessPiece* curFromPiece = m_chessBoard.GetChessFromIndex(fromIndex);
	int toIndex = m_chessBoard.GetIndexFromCharNum(toStr);

	curFromPiece->SetIsFirstMove(false);

	ChessPiece* curToPiece;
	if (moveResult == ChessMoveResult::VALID_CAPTURE_ENPASSANT)
	{
		curToPiece = m_chessBoard.m_lastMovingChess;
	}
	else
	{
		curToPiece = m_chessBoard.GetChessFromIndex(toIndex);
	}
	
	m_isMovingChess = true;

	if (curFromPiece->GetPieceType() == PieceType::KNIGHT) {
		curFromPiece->m_isHop = true;
	}
	else {
		curFromPiece->m_isSlide = true;
	}
	
	curFromPiece->m_animationTimer = Timer(2.f, m_game->m_gameClock);
	curFromPiece->SetAimGridPos(toPos);
	curFromPiece->SetPrevGridPos(fromPos);
	int taxiDist= GetTaxicabDistance2D(toPos, fromPos);
	curFromPiece->m_animationTimer.m_period = 0.2f+taxiDist*0.1f;
	curFromPiece->m_animationTimer.Start();
	curFromPiece->SetGridPos(toPos);
	m_chessBoard.m_lastMovingChess = curFromPiece;

	//delete or reverse toChess
	if (moveResult == ChessMoveResult::VALID_CASTLE_KINGSIDE
		|| moveResult == ChessMoveResult::VALID_CASTLE_QUEENSIDE)
	{
		// ??????????????
		ChessPiece* rook = nullptr;
		IntVec2 rookFromPos, rookToPos;

		if (moveResult == ChessMoveResult::VALID_CASTLE_KINGSIDE)
		{
			// ????
			if (curFromPiece->GetFaction() == Faction::WHITE)
			{
				rookFromPos = IntVec2(7, 0);  // h1
				rookToPos = IntVec2(5, 0);    // f1
			}
			else
			{
				rookFromPos = IntVec2(7, 7);  // h8
				rookToPos = IntVec2(5, 7);    // f8
			}
		}
		else // VALID_CASTLE_QUEENSIDE
		{
			// ????
			if (curFromPiece->GetFaction() == Faction::WHITE)
			{
				rookFromPos = IntVec2(0, 0);  // a1
				rookToPos = IntVec2(3, 0);    // d1
			}
			else
			{
				rookFromPos = IntVec2(0, 7);  // a8
				rookToPos = IntVec2(3, 7);    // d8
			}
		}

		// ?????
		rook = m_chessBoard.GetChessFromGridPos(rookFromPos);
		if (rook)
		{
			rook->SetIsFirstMove(false);
			rook->m_isSlide = true;
			rook->m_animationTimer = Timer(2.f, m_game->m_gameClock);
			rook->SetAimGridPos(rookToPos);
			rook->SetPrevGridPos(rookFromPos);
			int rookTaxiDist = GetTaxicabDistance2D(rookToPos, rookFromPos);
			rook->m_animationTimer.m_period = 0.2f + rookTaxiDist * 0.1f;
			rook->m_animationTimer.Start();
			rook->SetGridPos(rookToPos);
		}
	}
	else
	{
		//Destroy
		if (curToPiece && curToPiece->GetFaction() != m_currentTurnFaction)
		{
			if (curToPiece->GetPieceType() == PieceType::KING)
			{
				if (m_game->m_chessGameState == ChessGameState::FIRST_PLAYER_TURN)
				{
					m_game->m_chessGameState = ChessGameState::FIRST_WIN;
				}
				else if (m_game->m_chessGameState == ChessGameState::SEC_PLAYER_TURN)
				{
					m_game->m_chessGameState = ChessGameState::SEC_WIN;
				}
				m_chessBoard.DestroyChess(curToPiece);
				return;
			}
			m_chessBoard.DestroyChess(curToPiece);
		}
	}


    AddTurnNum();

	//update game state
	if (m_turnNumber % 2 == 0)
	{
		m_game->m_chessGameState = ChessGameState::FIRST_PLAYER_TURN;
		m_currentTurnFaction = Faction::WHITE;
	}
	else
	{
		m_game->m_chessGameState = ChessGameState::SEC_PLAYER_TURN;
		m_currentTurnFaction = Faction::BLACK;
	}
}

void ChessMatch::ChangeChessType(std::string posStr, PieceType pieceType)
{
	int pieceIndex = m_chessBoard.GetIndexFromCharNum(posStr);
	ChessPiece* curPiece = m_chessBoard.GetChessFromIndex(pieceIndex);

	curPiece->SetPieceType(pieceType);
	for (ChessPieceDefinition* def : ChessPieceDefinition::s_chessPieceDefs)
	{
		if (def->m_type == pieceType)
		{
			curPiece->m_def = def;
		}
	}
}

int ChessMatch::GetTurnNumber()
{
	return m_turnNumber;
}

Faction ChessMatch::GetCurFaction()
{
	if (m_turnNumber % 2 == 0)
	{
		return  Faction::WHITE;
	}
	else
	{
		return Faction::BLACK;
	}
}

void ChessMatch::AddTurnNum()
{
	m_turnNumber++;
}