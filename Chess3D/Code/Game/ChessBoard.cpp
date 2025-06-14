#include "ChessBoard.hpp"
#include "ChessPiece.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"

extern InputSystem* g_theInput;

ChessBoard::ChessBoard(ChessMatch* match):ChessObject(match)
{
	m_shader = g_theRenderer->CreateShaderFromFile("Data/Shaders/Diffuse", VertexType::VERTEX_PCUTBN);
	m_diffuseTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tree Top_OCC.png");
	m_normalTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tree Top_NRM.png");

	ChessPieceDefinition::InitializeChessDefinitionFromFile();

	m_chessPieces.reserve(64);

	PopulateDefaultChess();
	UpdateAndGetBoardStateString();

	AddVertsForBoard();
}

ChessBoard::~ChessBoard()
{
	for (ChessPiece* piece : m_chessPieces)
	{
		delete piece;
	}
	m_chessPieces.clear();

 	delete m_vertexBuffer;
 	m_vertexBuffer = nullptr;
 
 	delete m_indexBuffer;
 	m_indexBuffer = nullptr;

	for (ChessPieceDefinition* def : ChessPieceDefinition::s_chessPieceDefs)
	{
		delete	def;
		def = nullptr;
	}
	ChessPieceDefinition::s_chessPieceDefs.clear();

}

void ChessBoard::Update()
{
	UpdateDebugKeyInput();
	for (ChessPiece* piece : m_chessPieces)
	{
		piece->Update();
	}
}

void ChessBoard::Renderer() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	Vec3 normalSunDirection = m_sunDirection.GetNormalized();
	g_theRenderer->SetLightConstants(normalSunDirection, m_sunIntensity, m_ambientIntensity);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->BindTexture(m_diffuseTex,m_normalTex);
	g_theRenderer->SetPerFrameConstants(0.f, m_debugInt, 0.f);

	g_theRenderer->DrawVertexArray_WithTBN(m_vertexs, m_indexs, m_vertexBuffer, m_indexBuffer);

	for (ChessPiece* chess : m_chessPieces)
	{
		chess->Renderer();
	}
}

void ChessBoard::UpdateDebugKeyInput()
{
	bool isShiftPressed = g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT);
	if (g_theInput->WasKeyJustPressed('0')) m_debugInt = isShiftPressed ? 10 : 0;
	if (g_theInput->WasKeyJustPressed('1')) m_debugInt = isShiftPressed ? 11 : 1;
	if (g_theInput->WasKeyJustPressed('2')) m_debugInt = isShiftPressed ? 12 : 2;
	if (g_theInput->WasKeyJustPressed('3')) m_debugInt = isShiftPressed ? 13 : 3;
	if (g_theInput->WasKeyJustPressed('4')) m_debugInt = isShiftPressed ? 14 : 4;
	if (g_theInput->WasKeyJustPressed('5')) m_debugInt = isShiftPressed ? 15 : 5;
	if (g_theInput->WasKeyJustPressed('6')) m_debugInt = isShiftPressed ? 16 : 6;
	if (g_theInput->WasKeyJustPressed('7')) m_debugInt = isShiftPressed ? 17 : 7;
	if (g_theInput->WasKeyJustPressed('8')) m_debugInt = isShiftPressed ? 18 : 8;
	if (g_theInput->WasKeyJustPressed('9')) m_debugInt = isShiftPressed ? 19 : 9;
}

std::string const& ChessBoard::UpdateAndGetBoardStateString()
{
	m_boardStateVisionString.assign(64, '.');

	//int i = 0;
	for (ChessPiece* piece : m_chessPieces)
	{
		//if (i == 9)
		//{
		//	int x = 1;
		//}
		//
		IntVec2 gridPos = piece->GetGridPos(); 
		int index = gridPos.y * 8 + gridPos.x;
		char pieceChar = GetPieceChar(piece->GetPieceType(), piece->GetFaction());
		m_boardStateVisionString[index] = pieceChar;
		//i++;
	}

	return m_boardStateVisionString;
}

char ChessBoard::GetPieceChar(PieceType type, Faction faction)
{
	char baseChar;

	switch (type)
	{
	case PieceType::KING:   baseChar = 'K'; break;
	case PieceType::QUEEN:  baseChar = 'Q'; break;
	case PieceType::ROOK:   baseChar = 'R'; break;
	case PieceType::BISHOP: baseChar = 'B'; break;
	case PieceType::KNIGHT: baseChar = 'N'; break;
	case PieceType::PAWN:   baseChar = 'P'; break;
	default:                baseChar = '.'; break;
	}

	// white upper case, black lower case
	if (faction == Faction::BLACK)
	{
		baseChar =(char) std::tolower(baseChar);
	}

	return baseChar;
}

void ChessBoard::PopulateDefaultChess()
{
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::ROOK, IntVec2(0, 0)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::KNIGHT, IntVec2(1, 0)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::BISHOP, IntVec2(2, 0)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::QUEEN, IntVec2(3, 0)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::KING, IntVec2(4, 0)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::BISHOP, IntVec2(5, 0)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::KNIGHT, IntVec2(6, 0)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::ROOK, IntVec2(7, 0)));

	for (int x = 0; x < 8; ++x) {
		m_chessPieces.push_back(new ChessPiece(m_match, Faction::WHITE, PieceType::PAWN, IntVec2(x, 1)));
	}

	for (int x = 0; x < 8; ++x){
		m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::PAWN, IntVec2(x, 6)));
	}

	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::ROOK, IntVec2(0, 7)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::KNIGHT, IntVec2(1, 7)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::BISHOP, IntVec2(2, 7)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::QUEEN, IntVec2(3, 7)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::KING, IntVec2(4, 7)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::BISHOP, IntVec2(5, 7)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::KNIGHT, IntVec2(6, 7)));
	m_chessPieces.push_back(new ChessPiece(m_match, Faction::BLACK, PieceType::ROOK, IntVec2(7, 7)));
}

void ChessBoard::AddVertsForBoard()
{
 	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(24, sizeof(Vertex_PCUTBN));
 	m_indexBuffer = g_theRenderer->CreateIndexBuffer(24);
 
	for (int col = 0; col < 8; col++)
	{
		for (int row = 0; row < 8; row++)
		{
			if ((col + row) % 2 == 0)
			{
				AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
					Vec3((float)col, (float)row, -0.15f),
					Vec3((float)col + 1.f, (float)row + 1.f, 0.15f),
					Rgba8::BLACK,
					AABB2::ZERO_TO_ONE);
			}
			else
			{
				AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
					Vec3((float)col, (float)row, -0.15f),
					Vec3((float)col + 1.f, (float)row + 1.f, 0.15f),
					Rgba8::WHITE,
					AABB2::ZERO_TO_ONE);
			}
		}
	}
 
 	for (int i = 0; i < 10; i++)
 	{
 		AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
 			Vec3(-1.f+i,-1.f, -0.15f),
 			Vec3((float)i, 0.f, 0.15f),
 			Rgba8(100, 100, 100),
 			AABB2::ZERO_TO_ONE);
 
 		AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
 			Vec3(-1.f + i, 8.f, -0.15f),
 			Vec3((float)i, 9.f, 0.15f),
 			Rgba8(100,100,100),
 			AABB2::ZERO_TO_ONE);
 	}
 
 
 	for (int i = 0; i < 8; i++)
 	{
 		AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
 			Vec3(-1.f, (float)i, -0.15f),
 			Vec3(0.f, (float)i+1, 0.15f),
 			Rgba8(100, 100, 100),
 			AABB2::ZERO_TO_ONE);
 
 		AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
 			Vec3(8.f, (float)i, -0.15f),
 			Vec3(9.f, (float)i + 1, 0.15f),
 			Rgba8(100, 100, 100),
 			AABB2::ZERO_TO_ONE);
 	}

 	//AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs, Vec3(0.f, 0.f, -0.15f), Vec3(16.f, 16.f, 0.15f), Rgba8::HILDA, AABB2::ZERO_TO_ONE);
}

int ChessBoard::GetIndexFromGridPos(IntVec2 const& gridPos)
{
	if (!IsGridPosValid(gridPos))
	{
		return -1;
	}
	return gridPos.y * 8 + gridPos.x;
}

int ChessBoard::GetIndexFromCharNum(std::string const& charNum)
{
	char colChar = charNum[0];  
	char rowChar = charNum[1]; 

	int x = colChar - 'a';  
	int y = rowChar - '1'; 

	int index = y * 8 + x;

	return index;
}

IntVec2 ChessBoard::GetGridPosFromIndex(int index)
{
	int x = index % 8;
	int y = index / 8;
	return IntVec2(x, y);
}

ChessPiece* ChessBoard::GetChessFromIndex(int index)
{
	for (ChessPiece* piece : m_chessPieces)
	{
		IntVec2 gridPos = piece->GetGridPos();
		if (GetIndexFromGridPos(gridPos) == index)
		{
			return piece;
		}
	}
	return nullptr;
}

ChessPiece* ChessBoard::GetChessFromGridPos(IntVec2 gridPos)
{
	if (IsGridPosValid(gridPos))
	{
		int index = GetIndexFromGridPos(gridPos);
		return GetChessFromIndex(index);
	}
	return nullptr;
}

bool ChessBoard::DestroyChess(ChessPiece* curPiece)
{
	auto it = std::find(m_chessPieces.begin(), m_chessPieces.end(), curPiece);
	if (it != m_chessPieces.end())
	{
		delete* it;
		m_chessPieces.erase(it);
		return true;
	}
	return false;
}

bool ChessBoard::IsGridPosValid(IntVec2 gridPos)
{
	if (gridPos.x >= 0 && gridPos.x <= 7 && gridPos.y >= 0 && gridPos.y <= 7)
	{
		return true;
	}
	return false;
}

int ChessBoard::GetRenderDebugInt()
{
	return m_debugInt;
}

	
