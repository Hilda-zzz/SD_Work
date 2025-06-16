#include "ChessPiece.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/ChessMatch.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Easing.hpp"
extern Renderer* g_theRenderer;

ChessPiece::ChessPiece(ChessMatch* match,Faction facion, PieceType type, IntVec2 const& gridPos):ChessObject(match)
{
	m_match = match;
	m_texWhite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tree Top_COLOR.png");
	m_texBlack = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/wall3_color.png");
	m_texNormalWhite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tree Top_NRM.png");
	m_texNormalBlack = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/wall3_n.png");

	m_faction = facion;
	m_pieceType = type;
	m_gridPos = gridPos;
	UpdateCurPosFromCurGridPos();
	for (ChessPieceDefinition* def : ChessPieceDefinition::s_chessPieceDefs)
	{
		if (def->m_type == m_pieceType)
		{
			m_def = def;
		}
	}

	if (m_faction == Faction::WHITE)
	{
		m_orientaion = EulerAngles(90.f, 0.f, 0.f);
	}

	if (m_faction == Faction::BLACK)
	{
		m_orientaion = EulerAngles(-90.f, 0.f, 0.f);
	}
}

void ChessPiece::Update()
{
	if (m_isSlide)
	{
		if (m_animationTimer.HasPeriodElapsed())
		{
			m_isSlide = false;
			m_match->m_isMovingChess = false;
			m_animationTimer.Stop();
			SetGridPos(m_aimGridPos);
			UpdateCurPosFromCurGridPos();
		}
		else
		{
			Vec3 startPos = GetPosFromGridPos(m_prevCoord);
			Vec3 endPos = GetPosFromGridPos(m_aimGridPos);
			float t = m_animationTimer.GetElapsedFraction();
			float easingT=SmoothStart3(t);
			Vec3 curPos = Interpolate(startPos, endPos, easingT);
			SetCurPos(curPos);
		}
	}
	else if (m_isHop)
	{
		if (m_animationTimer.HasPeriodElapsed())
		{
			m_isHop = false;
			m_match->m_isMovingChess = false;
			m_animationTimer.Stop();
			SetGridPos(m_aimGridPos);
			UpdateCurPosFromCurGridPos();
		}
		else
		{
			Vec3 startPos = GetPosFromGridPos(m_prevCoord);
			Vec3 endPos = GetPosFromGridPos(m_aimGridPos);
			float t = m_animationTimer.GetElapsedFraction();
			float easingT = SmoothStart3(t);

			Vec3 midPos = (startPos + endPos) * 0.5f;
			float hopHeight = 1.5f;
			float arcHeight = sin(t * 3.14159f) * hopHeight; 

			Vec3 curPos = Interpolate(startPos, endPos, easingT);
			curPos.z += arcHeight; 

			SetCurPos(curPos);
		}
	}
}

void ChessPiece::Renderer() const
{
	Vec3 position = m_curPos;
	Mat44 transMat = Mat44::MakeTranslation3D(position);
	Mat44 rotateMat = m_orientaion.GetAsMatrix_IFwd_JLeft_KUp();
	transMat.Append(rotateMat);

	Rgba8 pieceColor = Rgba8::HILDA;
	if (m_faction == Faction::BLACK)
	{
		pieceColor = Rgba8(100,100,100);
	}
	g_theRenderer->SetModelConstants(transMat, pieceColor);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	

	if (m_faction == Faction::WHITE)
	{
		g_theRenderer->BindTexture(m_texWhite,m_texNormalWhite);
		g_theRenderer->DrawGameIndexedVertexBuffer(m_def->m_vertexBufferByPlyaer[0], m_def->m_indexBufferByPlyaer[0]);
	}
	else
	{
		g_theRenderer->BindTexture(m_texBlack,m_texNormalBlack);
		g_theRenderer->DrawGameIndexedVertexBuffer(m_def->m_vertexBufferByPlyaer[1], m_def->m_indexBufferByPlyaer[1]);
	}
}

IntVec2 ChessPiece::GetAimGridPos()
{
	return m_aimGridPos;
}

void ChessPiece::SetAimGridPos(IntVec2 const& gridPos)
{
	m_aimGridPos = gridPos;
}
	

IntVec2 ChessPiece::GetGridPos()
{
	return m_gridPos;
}

void ChessPiece::SetGridPos(IntVec2 const& gridPos)
{
	m_gridPos = gridPos;
}

IntVec2 ChessPiece::GetPrevGridPos()
{
	return m_prevCoord;
}

void ChessPiece::SetPrevGridPos(IntVec2 const& gridPos)
{
	m_prevCoord = gridPos;
}

Vec3 ChessPiece::GetCurPos()
{
	return m_curPos;
}

void ChessPiece::SetCurPos(Vec3 const& curPos)
{
	m_curPos = curPos;
}

PieceType ChessPiece::GetPieceType()
{
	return m_pieceType;
}

void ChessPiece::SetPieceType(PieceType newType)
{
	m_pieceType = newType;
}

Faction ChessPiece::GetFaction()
{
	return m_faction;
}

bool ChessPiece::GetIsFirstMove()
{
	return m_isFirstMove;
}

void ChessPiece::SetIsFirstMove(bool isFirstMove)
{
	m_isFirstMove = isFirstMove;
}

void ChessPiece::UpdateCurPosFromCurGridPos() 
{
	m_curPos= Vec3((float)m_gridPos.x+0.5f, (float)m_gridPos.y+0.5f, 0.15f);
}

Vec3 ChessPiece::GetPosFromGridPos(IntVec2 const& gridPos)
{
	return Vec3((float)gridPos.x + 0.5f, (float)gridPos.y + 0.5f, 0.15f);
}
