#include "ChessMatch.hpp"
#include "Game/Game.hpp"
ChessMatch::ChessMatch(Game* game):m_game(game)
{

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

	ChessPiece* curToPiece;
	if (moveResult == ChessMoveResult::VALID_CAPTURE_ENPASSANT)
	{
		curToPiece = m_chessBoard.m_lastMovingChess;
	}
	else
	{
		curToPiece = m_chessBoard.GetChessFromIndex(toIndex);
	}
	
	curFromPiece->SetGridPos(toPos);
	curFromPiece->SetPrevGridPos(fromPos);
	m_chessBoard.m_lastMovingChess = curFromPiece;

	//delete or reverse toChess
	if (moveResult == ChessMoveResult::VALID_CASTLE_KINGSIDE
		|| moveResult == ChessMoveResult::VALID_CASTLE_QUEENSIDE)
	{
		curToPiece->SetGridPos(fromPos);
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
}

int ChessMatch::GetTurnNumber()
{
	return m_turnNumber;
}

void ChessMatch::AddTurnNum()
{
	m_turnNumber++;
}
