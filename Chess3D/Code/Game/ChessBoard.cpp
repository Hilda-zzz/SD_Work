#include "ChessBoard.hpp"
#include "ChessPiece.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/ChessObject.hpp"
#include "Game/ChessMatch.hpp"
#include "Engine/Core/ObjLoader.hpp"

extern InputSystem* g_theInput;

ChessBoard::ChessBoard(ChessMatch* match):ChessObject(match)
{
	m_shader = g_theRenderer->CreateShaderFromFile("Data/Shaders/DX11BlinnPhong", VertexType::VERTEX_PCUTBN);
	m_diffuseTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PhongTextures1/Bricks_d.png");
	m_normalTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PhongTextures1/Bricks_n.png");
	m_sgeTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PhongTextures1/Bricks_sge.png");

	ChessPieceDefinition::InitializeChessDefinitionFromFile();

	m_chessPieces.reserve(64);

	PopulateDefaultChess();
	UpdateAndGetBoardStateString();

	AddBoardBlockCollider();
	AddVertsForBoard();

	ObjLoader::LoadObjFromFile_WithTBN("Data/Models/Woman/Woman.obj", m_women);
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
	m_hasValidAimPos = false;
	m_impactedPiece = nullptr;
	float raycastDist = FLT_MAX;
	m_impactedGrid.x = -1; 
	m_impactedGrid.y = -1;
	m_aimHoverQuad.clear();
	m_aimShadowPiece.clear();
	for (int i=0;i<(int)m_chessPieces.size();i++)
	{
		//m_chessPieces[i]->m_isImpacted = false;
		RaycastResult3D raycastResult = RaycastVsZCylinder3D(m_match->m_game->GetPlayerCamPosition(),
			m_match->m_game->GetPlayerCamDirection(),10.f,
			m_chessPieces[i]->m_collider.m_center, m_chessPieces[i]->m_collider.m_radius, m_chessPieces[i]->m_collider.m_halfHeight);
		if (raycastResult.m_didImpact)
		{
			if (raycastResult.m_impactDist < raycastDist)
			{
				raycastDist = raycastResult.m_impactDist;
				m_impactedGrid = m_chessPieces[i]->GetGridPos();
			}
		}
	}
 	for (int j = 0; j < (int)m_boardBlocks.size(); j++)
 	{
 		RaycastResult3D raycastResult = RaycastVsAABB3D(m_match->m_game->GetPlayerCamPosition(),
 			m_match->m_game->GetPlayerCamDirection(), 10.f, m_boardBlocks[j].m_collider);
 		if (raycastResult.m_didImpact)
 		{
 			if (raycastResult.m_impactDist < raycastDist)
 			{
 				raycastDist = raycastResult.m_impactDist;
 				m_impactedGrid = m_boardBlocks[j].m_gridPos;
 			}
 		}
 	}
	if (m_impactedGrid != IntVec2(-1, -1)&&!m_selectedPiece)
	{
		m_impactedPiece=GetChessFromGridPos(m_impactedGrid);
	}
	
	if (m_impactedGrid != IntVec2(-1, -1) && m_selectedPiece)
	{
		std::string fromStr = GetStrFromGridPos(m_selectedPiece->GetGridPos());
		std::string toStr = GetStrFromGridPos(m_impactedGrid);
		bool isCheat = false;
		if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
		{
			isCheat = true;
		}
		ChessMoveResult moveResult = m_match->m_game->CheckMovement(fromStr, toStr, isCheat);
		if (m_match->m_game->IsValidateChessMoveResult(moveResult))
		{
			m_hasValidAimPos = true;
			Vec3 bottomLeft = Vec3((float)m_impactedGrid.x, (float)m_impactedGrid.y, 0.15f + 0.1f);
			Vec3 bottomRight = Vec3((float)m_impactedGrid.x + 1.f, (float)m_impactedGrid.y, 0.15f + 0.1f);
			Vec3 topRight = Vec3((float)m_impactedGrid.x + 1.f, (float)m_impactedGrid.y + 1.f, 0.15f + 0.1f);
			Vec3 topLeft = Vec3((float)m_impactedGrid.x, (float)m_impactedGrid.y + 1.f, 0.15f + 0.1f);

			AddVertsForQuad3D(m_aimHoverQuad,
				bottomLeft,
				bottomRight,
				topRight,
				topLeft
			);

			if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
			{
				EventArgs args;
				args.SetValue("from",fromStr );
				args.SetValue("to",toStr );
				args.SetValue("teleport",std::to_string(isCheat));
				g_theEventSystem->FireEvent("ChessMove",args);
				m_selectedPiece->m_isSelected = false;
				m_selectedPiece = nullptr;
			}
		}
	}

	if (m_impactedPiece&& m_impactedPiece->GetFaction() == m_match->GetCurFaction())
	{
		m_impactedPiece->m_isImpacted = true;
	}

	if (m_impactedPiece)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE)
			&& m_impactedPiece->GetFaction() == m_match->GetCurFaction())
		{
			m_impactedPiece->m_isSelected = true;
			m_selectedPiece = m_impactedPiece;
		}
	}

	if (m_selectedPiece)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
		{
			m_selectedPiece->m_isSelected = false;
			m_selectedPiece = nullptr;
		}
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
	g_theRenderer->BindTexture(m_diffuseTex,m_normalTex,m_sgeTex);
	g_theRenderer->SetPerFrameConstants(0.f, m_debugInt, 0.f);

	g_theRenderer->DrawVertexArray_WithTBN(m_vertexs, m_indexs, m_vertexBuffer, m_indexBuffer);

	for (ChessPiece* chess : m_chessPieces)
	{
		g_theRenderer->BindShader(m_shader);
		chess->Renderer();
	}
	Texture* womenDiffuse = g_theRenderer->CreateOrGetTextureFromFile("Data/Models/Woman/Woman_Diffuse.png");
	Texture* womenNormal = g_theRenderer->CreateOrGetTextureFromFile("Data/Models/Woman/Woman_Normal.png");
	g_theRenderer->BindTexture(womenDiffuse, womenNormal,nullptr);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->DrawVertexArray_WithTBN(m_women);

	if (m_hasValidAimPos)
	{
		Texture* hover = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/HoverSquare.png");
		g_theRenderer->BindTexture(hover);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->DrawVertexArray(m_aimHoverQuad);
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

void ChessBoard::AddBoardBlockCollider()
{
	for (int col = 0; col < 8; col++)
	{
		for (int row = 0; row < 8; row++)
		{
// 			if ((col + row) % 2 == 0)
// 			{
// 				AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
// 					Vec3((float)col, (float)row, -0.15f),
// 					Vec3((float)col + 1.f, (float)row + 1.f, 0.15f),
// 					Rgba8(150, 150, 150),
// 					AABB2::ZERO_TO_ONE);
// 			}
// 			else
// 			{
// 				AddVertsForAABB3D_WithTBN(m_vertexs, m_indexs,
// 					Vec3((float)col, (float)row, -0.15f),
// 					Vec3((float)col + 1.f, (float)row + 1.f, 0.15f),
// 					Rgba8::WHITE,
// 					AABB2::ZERO_TO_ONE);
// 			}
			IntVec2 gridPos = IntVec2(col, row);
			AABB3 collider = AABB3(Vec3((float)col, (float)row, -0.15f),
								Vec3((float)col + 1.f, (float)row + 1.f, 0.15f));
			ChessBoardBlock boardBlock;
			boardBlock.m_collider = collider;
			boardBlock.m_gridPos = gridPos;
			m_boardBlocks.push_back(boardBlock);
		}
	}
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
					Rgba8(150,150,150),
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

std::string ChessBoard::GetStrFromGridPos(IntVec2 const& gridPos)
{
	std::string result;
	if (gridPos.x < 0 || gridPos.x > 7 || gridPos.y < 0 || gridPos.y > 7)
	{
		result = "Invalid";
		return result;
	}
	char file = 'a' + (char)gridPos.x;
	char rank = '1' + (char)gridPos.y;

	result = std::string(1, file) + std::string(1, rank);
	return result;
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

	
