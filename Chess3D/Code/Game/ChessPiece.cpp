#include "ChessPiece.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

ChessPiece::ChessPiece(Faction facion, PieceType type, IntVec2 const& gridPos):ChessObject()
{
	m_texWhite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MarbleTexture2.png");
	m_texBlack = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/WoodTexture4.png");

	m_faction = facion;
	m_pieceType = type;
	m_gridPos = gridPos;
	for (ChessPieceDefinition* def : ChessPieceDefinition::s_chessPieceDefs)
	{
		if (def->m_type == m_pieceType)
		{
			m_def = def;
		}
	}

	if (m_faction == Faction::BLACK)
	{
		m_orientaion = EulerAngles(180.f, 0.f, 0.f);
	}
}

void ChessPiece::Renderer() const
{
	Vec3 position = CalculateCurPosition();
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
		g_theRenderer->BindTexture(m_texWhite);
		g_theRenderer->DrawGameIndexedVertexBuffer(m_def->m_vertexBufferByPlyaer[0], m_def->m_indexBufferByPlyaer[0]);
	}
	else
	{
		g_theRenderer->BindTexture(m_texBlack);
		g_theRenderer->DrawGameIndexedVertexBuffer(m_def->m_vertexBufferByPlyaer[1], m_def->m_indexBufferByPlyaer[1]);
	}
}
	

IntVec2 ChessPiece::GetGridPos()
{
	return m_gridPos;
}

void ChessPiece::SetGridPos(IntVec2 const& gridPos)
{
	m_gridPos = gridPos;
}

PieceType ChessPiece::GetPieceType()
{
	return m_pieceType;
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

Vec3 ChessPiece::CalculateCurPosition() const
{
	return Vec3((float)m_gridPos.x+0.5f, (float)m_gridPos.y+0.5f, 0.15f);
}
