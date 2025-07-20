#include "Game/Game.hpp"
#include "App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Prop.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include <Engine/Core/Time.hpp>
#include "Engine/Math/AABB3.hpp"
#include <Engine/Core/DebugRenderSystem.hpp>
#include "Engine/Math/AABB2.hpp"
#include "GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/CubeSkyBox.hpp"

#include "Game/ChessMatch.hpp"
#include "Engine/Core/ObjLoader.hpp"
#include "Engine/Network/NetworkSystem.hpp"

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;
extern Game* g_theGame;
extern NetworkSystem* g_theNetworkSystem;
extern DevConsole* g_theDevConsole;

Game::Game()
{
	m_gridTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_gameClock = new Clock();
	m_player = new Player(this);

	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
	AABB2 viewport = AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
	m_screenCamera.SetViewport(viewport);
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_screenCamera.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f,0.f,0.f));
	m_player->m_playerCam.SetViewport(viewport);

	std::string logString = "\
	Mouse x-axis / Right stick x-axis       Yaw\n\
	Mouse y-axis / Right stick y-axis       Pitch\n\
	Q / E / Left trigger / right trigger    Roll\n\
	A / D / Left stick x-axis               Move left or right, relative to player orientation\n\
	W / S / Left stick y-axis               Move forward or back, relative to player orientation\n\
	Z / C / Left shoulder / right shoulder  Move down or up, relative to the world\n\
	H / Start button                        Reset position and orientation to zero\n\
	Shift / A button                        Increase speed by a factor of 10 while held\n\
	P                                       Pause the game\n\
	O                                       Single step frame\n\
	T                                       Slow motion mode";
	//g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, logString);

	//----------sky box--------------------
	std::string skyBoxShaderPath = "Data/Shaders/CubeSkyBox";
	m_cubeSkybox = new CubeSkyBox(g_theRenderer, m_skyboxPaths,&skyBoxShaderPath);

	//------------Chess Game---------------------------------------------
	StartTheMatch();

	//------------Lighting---------------------
	m_pointLights.reserve(10);
	PointLight pointLight = PointLight(
		Vec3(3.f, 3.f, 1.5f),
		0.1f,3.f,
		Rgba8::CYAN,
		2.0f
	);
	PointLight pointLight2 = PointLight(
		Vec3(2.f, 2.f, 1.5f),
		0.3f, 2.f,
		Rgba8::HILDA,
		3.0f
	);
	m_pointLights.push_back(pointLight);
	//m_pointLights.push_back(pointLight2);

	m_spotLights.reserve(10);
	SpotLight spotLight1 = SpotLight(
		Vec3(3.f, 3.f, 0.6f),              // position
		Vec3(0.5f, 0.5f, -0.5f).GetNormalized(),  // direction
		25.f,                              // innerCutoffAngle
		30.f,                              // outerCutoffAngle
		Rgba8::YELLOW,                     // color
		2.f,                               // intensity
		5.f,                               // innerRadius
		20.f                               // outerRadius
	);
	m_spotLights.push_back(spotLight1);

	//---------------------Network Protocol & Commands-----------------------
	g_theEventSystem->SubscribeEventCallbackFuction("ChessMove", Command_ChessMove, true);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessBeginMatch", Command_ChessBeginMatch, true);

	g_theEventSystem->SubscribeEventCallbackFuction("ChessServerInfo", ChessServerInfo_LocalCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessListen", ChessListen_LocalCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessConnect", ChessConnect_LocalCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessDisconnect", ChessDisconnect_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessPlayerInfo", ChessPlayerInfo_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessBegin", ChessBegin_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessValidate", ChessValidate_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessResign", ChessResign_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("RemoteCmd", RemoteCmd_LocalCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessOfferDraw", ChessOfferDraw_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFuction("ChessAcceptDraw", ChessAcceptDraw_RemoteCmd);
}

Game::~Game()
{
	if (m_curMatch)
	{
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessMove", Command_ChessMove);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessBeginMatch", Command_ChessBeginMatch);

		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessServerInfo", ChessServerInfo_LocalCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessListen", ChessListen_LocalCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessConnect", ChessConnect_LocalCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessDisconnect", ChessDisconnect_RemoteCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessPlayerInfo", ChessPlayerInfo_RemoteCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessBegin", ChessBegin_RemoteCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessResign", ChessResign_RemoteCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessValidate", ChessValidate_RemoteCmd);
		g_theEventSystem->UnsubscribeEventCallbackFunction("RemoteCmd", RemoteCmd_LocalCmd);

		delete m_curMatch;
		m_curMatch = nullptr;
	}

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_cubeSkybox;
	m_cubeSkybox = nullptr;
}


void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();
	UpdateCamera(deltaSeconds);
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		if (g_theDevConsole->GetMode() == HIDDEN)
		{
			g_theDevConsole->SetMode(OPEN_FULL);
			m_isDevConsole = true;
		}
		else
		{
			g_theDevConsole->SetMode(HIDDEN);
			m_isDevConsole = false;
		}
	}
	if (m_isAttractMode)
	{
		UpdateAttractMode(deltaSeconds);
		return;
	}
	UpdateDeveloperCheats(deltaSeconds);
	UpdateGameplayMode(deltaSeconds);
}

void Game::Renderer() const
{
	if (m_isAttractMode)
	{
		RenderAttractMode();
		return;
	}
	g_theRenderer->BeginCamera(m_player->m_playerCam);

	m_cubeSkybox->Render();

	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	// draw models
// 	g_theRenderer->SetModelConstants();
// 	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
// 	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
// 	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
// 	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
// 	Shader* diffuseShader = g_theRenderer->CreateShaderFromFile("Data/Shaders/DX11BlinnPhong");
// 	g_theRenderer->BindShader(diffuseShader);
// 	Texture* tex = g_theRenderer->CreateOrGetTextureFromFile("Data/Models/Woman/Woman_Diffuse.png");
// 	g_theRenderer->BindTexture(tex);
// 	g_theRenderer->DrawVertexArray_WithTBN(m_cube);

	//chess game
	g_theRenderer->SetPointLightsConstants(m_pointLights);
	g_theRenderer->SetSpotLightsConstants(m_spotLights);
	m_curMatch->Renderer();

	g_theRenderer->EndCamera(m_player->m_playerCam);

	DebugRenderWorld(m_player->m_playerCam);

	g_theRenderer->BeginCamera(m_screenCamera);

	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	font->AddVertsForTextInBox2D(title, "Use the DevVonsole (~) to enter commands.", 
		AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));

	std::string cameraMode = (m_camMode == CamMode::FREE) ? "Camera Mode: FREE" : "Camera Mode: AUTO";
	const char* gameStateNames[] = {
		"WAIT TO START GAME",
		"FIRST PLAYER TURN",
		"SECOND PLAYER TURN",
		"FIRST PLAYER WIN",
		"SECOND PLAYER WIN"
	};
	std::string gameState = "Game State: " + std::string(gameStateNames[(int)m_chessGameState]);
	int debugInt=m_curMatch->m_chessBoard.GetRenderDebugInt();
	char const* renderMode = GetDebugRenderModeDesc(debugInt);
	font->AddVertsForTextInBox2D(title, cameraMode,
		AABB2(Vec2(10.f, 745.f), Vec2(500.f, 765.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	font->AddVertsForTextInBox2D(title, gameState,
		AABB2(Vec2(10.f, 720.f), Vec2(700.f, 740.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	font->AddVertsForTextInBox2D(title, renderMode,
		AABB2(Vec2(10.f, 695.f), Vec2(700.f, 715.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);

	DebugRenderScreen(m_screenCamera);
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);

	g_theRenderer->BindTexture(nullptr);
	DebugDrawRing(4.f, 10.f, Rgba8::WHITE, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)|| g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_isAttractMode = false;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);

	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		if (m_camMode == CamMode::FREE)
		{
			m_camMode = CamMode::AUTO;
			if (g_theGame->m_chessGameState == ChessGameState::FIRST_PLAYER_TURN)
			{
				m_player->m_position = g_theGame->m_whiteCamPosition;
				m_player->m_orientation = g_theGame->m_whiteOrientation;
			}
			else if (g_theGame->m_chessGameState == ChessGameState::SEC_PLAYER_TURN)
			{
				m_player->m_position = g_theGame->m_blackCamPosition;
				m_player->m_orientation = g_theGame->m_blackOrientation;
			}

		}
		else
		{
			m_camMode = CamMode::FREE;
		}
	}

	m_player->Update((float)g_systemClock->GetDeltaSeconds());
	char timeBuffer[256];
	snprintf(timeBuffer, sizeof(timeBuffer),
		"Time: %.2f  FPS: %.2f  Scale: %.2f",
		g_systemClock->GetTotalSeconds(), 1.f/g_systemClock->GetDeltaSeconds(), 1.f);
	DebugAddScreenText(std::string(timeBuffer), AABB2(Vec2(1100.f, 720.f), Vec2(1590.f, 790.f)), 20.f, Vec2(1.f, 1.f), 0.f, Rgba8::WHITE, Rgba8::WHITE);

	m_curMatch->Update(deltaTime);
	//-----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isAttractMode=true;
	}
}

void Game::UpdateDeveloperCheats(float deltaTime)
{
	UNUSED(deltaTime);
	AdjustForPauseAndTimeDitortion();
}

void Game::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	
	m_player->m_playerCam.SetPerspectiveView(m_player->m_playerCam.GetViewport().GetDimensions().x/ m_player->m_playerCam.GetViewport().GetDimensions().y, 60.f, 0.1f, 100.f);
}

void Game::AdjustForPauseAndTimeDitortion()
{
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
	}

	if (g_theInput->WasKeyJustPressed('T'))
	{
		m_previousTimeScale = (float)m_gameClock->GetTimeScale();
	}

	if (g_theInput->IsKeyDown('T'))
	{
		m_gameClock->SetTimeScale(0.1f);
	}

	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_gameClock->SetTimeScale(m_previousTimeScale);
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock->StepSingleFrame();
	}
}

void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(nullptr);
	DebugDrawRing(4.f, 20.f, Rgba8::WHITE,Vec2(SCREEN_SIZE_X*0.5f,SCREEN_SIZE_Y*0.5f));
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderUI() const
{
	g_theRenderer->BindTexture(nullptr);
	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));
}

void Game::RenderDebugMode()const
{

}

char const* Game::GetDebugRenderModeDesc(int debugInt) const
{
	switch (debugInt)
	{
	case 0:     return "Lit (including normal maps)";
	case 1:     return "Lit texel only";
	case 2:     return "Vertex Color only (C)";
	case 3:     return "UV TexCoords only (U)";
	case 4:     return "Vertex Tangents: raw, in Model Space (T)";
	case 5:     return "Vertex Bitangents: raw, in Model Space (B)";
	case 6:     return "Vertex Normals: raw, in Model Space (N)";
	case 7:     return "Normal Map texel only";
	case 8:     return "Pixel Normal in TBN space (decoded, raw)";
	case 9:     return "Pixel Normal in World space (decoded, transformed)";
	case 10:    return "Lit, but without normal maps";
	case 11:    return "(incomplete)Light strength (vs. pixel normal in world space)";
	case 12:    return "(incomplete)Light strength (vs. vertex/surface normals only)";
	case 13:    return "???";
	case 14:    return "Vertex Tangents: transformed, into World space (T)";
	case 15:    return "Vertex Bitangents: transformed, into World space (B)";
	case 16:    return "Vertex Normals: transformed, into World space (N)";
	case 17:    return "ModelToWorld I (forward) basis vector, in world space (I)";
	case 18:    return "ModelToWorld J (left) basis vector, in world space (J)";
	case 19:    return "ModelToWorld K (up) basis vector, in world space (K)";
	default:    return "???";
	}
}

void Game::AddEntityToList(Entity& thisEntity, EntityList& list)
{
	for (int i = 0; i < static_cast<int>(list.size()); i++)
	{
		if (list[i] == nullptr)
		{
			list[i] = &thisEntity;
			return;
		}
	}
	list.push_back(&thisEntity);
}

void Game::PrintBoardState()
{
	std::string playerName;
	std::string playerColor;
	std::string	gameStateTip;
	if (m_chessGameState==ChessGameState::FIRST_PLAYER_TURN)
	{
		playerName = "0";
		playerColor = "White";
		gameStateTip = "Game State: First Player's Turn";
		std::string playerTip = "Player" + playerName + " (" + playerColor + ") -- it's your turn";
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, playerTip);
		g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, gameStateTip);
	}
	else if(m_chessGameState == ChessGameState::SEC_PLAYER_TURN)
	{
		playerName = "2";
		playerColor = "Black";
		gameStateTip = "Game State: Second Player's Turn";
		std::string playerTip = "Player" + playerName + " (" + playerColor + ") -- it's your turn";
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, playerTip);
		g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, gameStateTip);
	}
	else if (m_chessGameState == ChessGameState::FIRST_WIN)
	{
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "####################");
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "First Player (White) Win!");
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "####################");
	}
	else if (m_chessGameState == ChessGameState::SEC_WIN)
	{
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "####################");
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "Second Player (Black) Win!");
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "####################");
	}

	std::string boardStateString = m_curMatch->GetBoardStateString();
	std::string formattedBoard;

	formattedBoard = "  BLACK\n";
	formattedBoard += "  ABCDEFGH  \n\
	+--------+\n";
	for (int row = 7; row >= 0; --row)
	{
		int rowNumber = row + 1;  
		formattedBoard += std::to_string(rowNumber) + "|";

		for (int col = 0; col < 8; ++col)
		{
			int index = row * 8 + col;
			formattedBoard += boardStateString[index];
		}

		formattedBoard += "|" + std::to_string(rowNumber);
		formattedBoard += '\n';
	}
	formattedBoard += " +--------+  \n\
	 ABCDEFGH\n\
	 WHITE";
	g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, formattedBoard);
}

bool Game::Command_ChessMove(EventArgs& args)
{
	if (g_theGame->m_curMatch->m_isMovingChess)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "The last turn is not finished");
		return false;
	}
	//Basic Check
	if (!g_theGame || !g_theGame->m_curMatch)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No active chess match. Start a match first.");
		return false;
	}

	if (g_theGame->m_chessGameState==ChessGameState::FIRST_WIN||
		g_theGame->m_chessGameState == ChessGameState::SEC_WIN)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "The Player have won the game, please begin a new match.");
		return false;
	}

	// get move info str
	bool isFromRemote = args.GetValue("remote", false);
	std::string fromStr = args.GetValue("from", "");
	std::string toStr = args.GetValue("to", "");
	bool isCheat = args.GetValue("teleport", false);

	ChessMoveResult moveResult = CheckMovement(fromStr, toStr,isCheat);

	//----------------------------------------------------------

	if (IsValidateChessMoveResult(moveResult))
	{
		// if is pawn change to another type
		if (moveResult == ChessMoveResult::VALID_MOVE_PROMOTION)
		{
			std::string promotionType = args.GetValue("promotion", "");
			if(promotionType=="")
			{
				g_theDevConsole->AddLine(DevConsole::INVALID, "Pawn promotion requires promoteTo parameter (knight, bishop, rook, or queen)!");
				return false;
			}

			PieceType newPieceType;
			if (promotionType == "queen")
			{
				newPieceType = PieceType::QUEEN;
			}
			else if (promotionType == "rook")
			{
				newPieceType = PieceType::ROOK;
			}
			else if (promotionType == "bishop")
			{
				newPieceType = PieceType::BISHOP;
			}
			else if (promotionType == "knight")
			{
				newPieceType = PieceType::KNIGHT;
			}
			else
			{
				g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid promotion type! Must be: knight, bishop, rook, or queen");
				return false;
			}

			// Execute the promotion
			g_theGame->m_curMatch->ChangeChessType(fromStr, newPieceType);
			g_theDevConsole->AddLine(DevConsole::TIPS, Stringf("Pawn promoted to %s!", promotionType.c_str()));
		}
		
		// move the chess
		g_theGame->m_curMatch->MoveTheChessPiece(fromStr, toStr,moveResult);
		if (moveResult == ChessMoveResult::VALID_CASTLE_KINGSIDE)
		{
			if (g_theGame->m_curMatch->GetCurFaction()== Faction::WHITE)
			{
				g_theGame->m_curMatch->MoveTheChessPiece("h1", "f1", ChessMoveResult::VALID_CASTLE_KINGSIDE);
			}
			else
			{
				g_theGame->m_curMatch->MoveTheChessPiece("h8", "f8", ChessMoveResult::VALID_CASTLE_KINGSIDE);
			}
		}
		else if (moveResult == ChessMoveResult::VALID_CASTLE_QUEENSIDE)
		{
			if (g_theGame->m_curMatch->GetCurFaction() == Faction::WHITE)
			{
				g_theGame->m_curMatch->MoveTheChessPiece("a1", "d1", ChessMoveResult::VALID_CASTLE_QUEENSIDE);
			}
			else
			{
				g_theGame->m_curMatch->MoveTheChessPiece("a8", "d8", ChessMoveResult::VALID_CASTLE_QUEENSIDE);
			}
		}
		// ---------------------------------update turn ---------------------------------------
		g_theGame->m_curMatch->AddTurnNum();
			//update game state
		if (g_theGame->m_curMatch->m_turnNumber % 2 == 0)
		{
			g_theGame->m_chessGameState = ChessGameState::FIRST_PLAYER_TURN;
			g_theGame->m_curMatch->m_currentTurnFaction = Faction::WHITE;
		}
		else
		{
			g_theGame->m_chessGameState = ChessGameState::SEC_PLAYER_TURN;
			g_theGame->m_curMatch->m_currentTurnFaction = Faction::BLACK;
		}

		//update cam
		if (g_theGame->m_camMode == CamMode::AUTO|| 
			g_theGame->m_chessGameState==ChessGameState::FIRST_WIN||
			g_theGame->m_chessGameState == ChessGameState::SEC_WIN)
		{
			g_theGame->SetObservationCamPosition(g_theGame->m_chessGameState);
		}

		// print sate
		g_theGame->PrintBoardState();

		//-----------------Network Remote Cmd-------------------------
		if (!isFromRemote)
		{
			std::string remoteCmdString = "RemoteCmd cmd=ChessMove from=" + fromStr + " to=" + toStr;
			g_theDevConsole->Execute(remoteCmdString);
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool Game::Command_ChessBeginMatch(EventArgs& args)
{
	UNUSED(args);
	g_theGame->EndTheMatch();
	g_theGame->StartTheMatch();
	return true;
}

ChessMoveResult Game::CheckMovement(std::string fromStr, std::string toStr, bool isCheat)
{
	// string input validation
	if (fromStr.empty() || toStr.empty()
		|| fromStr.length() != 2 || toStr.length() != 2)
	{
		//AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_BAD_LOCATION);
		return ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
	}

	char fromCol = fromStr[0];
	char toCol = toStr[0];
	if (fromCol < 'a' || fromCol > 'h' || toCol < 'a' || toCol > 'h')
	{
		//AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_BAD_LOCATION);
		return ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
	}

	char fromRow = fromStr[1];
	char toRow = toStr[1];
	if (fromRow < '1' || fromRow > '8' || toRow < '1' || toRow > '8')
	{
		//AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_BAD_LOCATION);
		return ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
	}

	// zero distance
	if (fromStr == toStr)
	{
		//AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE);
		return ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
	}

	//get faction
	Faction curFaction;
	int turnNum = g_theGame->m_curMatch->GetTurnNumber();
	if (turnNum % 2 == 0)
	{
		curFaction = Faction::WHITE;
	}
	else
	{
		curFaction = Faction::BLACK;
	}

	//if the from have piece and belong to the right player
	int fromIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromCharNum(fromStr);
	ChessPiece* curFromPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(fromIndex);
	if (!curFromPiece)
	{
		//AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_NO_PIECE);
		return ChessMoveResult::INVALID_MOVE_NO_PIECE;
	}
	if (curFromPiece->GetFaction() != curFaction)
	{
		//AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE);
		return ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
	}

	//if the to have piece belong to current player, it will be invalid
	int toIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromCharNum(toStr);
	ChessPiece* curToPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toIndex);
	if (curToPiece && curToPiece->GetFaction() == curFaction)
	{
		if (!(curFromPiece->GetPieceType() == PieceType::KING && curToPiece->GetPieceType() == PieceType::ROOK))
		{
			//AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED);
			return ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
		}
	}

	//Chess Piece Rules
	ChessMoveResult moveResult;
	if (isCheat)
	{
		moveResult = ChessMoveResult::VALID_MOVE_CHEAT;
		return moveResult;
	}
	
	ValidatePieceMovement(fromIndex, toIndex, moveResult);
	//AddLineForMoveResult(moveResult);
	return moveResult;
}

bool Game::IsValidateChessMoveResult(ChessMoveResult result)
{
	switch (result)
	{
	case ChessMoveResult::VALID_MOVE_NORMAL:
	case ChessMoveResult::VALID_MOVE_PROMOTION:
	case ChessMoveResult::VALID_CASTLE_KINGSIDE:
	case ChessMoveResult::VALID_CASTLE_QUEENSIDE:
	case ChessMoveResult::VALID_CAPTURE_NORMAL:
	case ChessMoveResult::VALID_CAPTURE_ENPASSANT:
	case ChessMoveResult::VALID_MOVE_CHEAT:
		return true;
	case ChessMoveResult::INVALID_MOVE_BAD_LOCATION:
	case ChessMoveResult::INVALID_MOVE_NO_PIECE:
	case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE:
	case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE:
	case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE:
	case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED:
	case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED:
	case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK:
	case ChessMoveResult::INVALID_MOVE_KING_TOO_CLOSE:
	case ChessMoveResult::INVALID_ENPASSANT_STALE:
	case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED:
	case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED:
	case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED:
	case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK:
	case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK:
		return false;
	default:
		return false;
	}
}

Camera Game::GetGameplayCam()
{
	return m_player->m_playerCam;
}

bool Game::ValidatePieceMovement(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
	ChessPiece* movePiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(moveChessIndex);
	if (!movePiece)
	{
		out_result = ChessMoveResult::INVALID_MOVE_NO_PIECE;
		return false;
	}

	PieceType movePieceType = movePiece->GetPieceType();
	switch (movePieceType)
	{
	case PieceType::KING:
		return ValidateKingMove(moveChessIndex, toChessIndex, out_result);
	case PieceType::QUEEN:
		return ValidateQueenMove(moveChessIndex, toChessIndex, out_result);
	case PieceType::ROOK:
		return ValidateRookMove(moveChessIndex, toChessIndex, out_result);
	case PieceType::BISHOP:
		return ValidateBishopMove(moveChessIndex, toChessIndex, out_result);
	case PieceType::KNIGHT:
		return ValidateKnightMove(moveChessIndex, toChessIndex, out_result);
	case PieceType::PAWN:
		return ValidatePawnMove(moveChessIndex, toChessIndex, out_result);
	case PieceType::NONE:
	case PieceType::NUM:
	default:
		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		return false;
	}
}

bool Game::ValidateKingMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
// 	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
// 	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
// 	IntVec2 moveStep = toGridPos - moveGridPos;
// 
// 	ChessPiece* movingKing = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(moveChessIndex);
// 	//ChessPiece* toChess = g_theGame->m_curMatch->m_chessBoard.GetChessFromGridPos(toGridPos);
// 
// 	//Castle
// 	if (abs(moveStep.x) >= 2 && moveStep.y == 0)
//  	{
// 		if (movingKing->GetFaction() == Faction::WHITE)
// 		{
// 			if (moveGridPos == IntVec2(4, 0) && toGridPos == IntVec2(0, 0))
// 			{
// 				if (ValidateCastling(moveChessIndex, toChessIndex, true, out_result))
// 				{
// 					out_result = ChessMoveResult::VALID_CASTLE_QUEENSIDE;
// 					return true;
// 				}
// 				return false;
// 			}
// 			else if (moveGridPos == IntVec2(4, 0) && toGridPos == IntVec2(7, 0))
// 			{
// 				if (ValidateCastling(moveChessIndex, toChessIndex, false, out_result))
// 				{
// 					out_result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
// 					return true;
// 				}
// 				return false;
// 			}
// 		}
// 		else if (movingKing->GetFaction() == Faction::BLACK)
// 		{
// 			if (moveGridPos == IntVec2(4, 7) && toGridPos == IntVec2(0, 7))
// 			{
// 				if (ValidateCastling(moveChessIndex, toChessIndex, true, out_result))
// 				{
// 					out_result = ChessMoveResult::VALID_CASTLE_QUEENSIDE;
// 					return true;
// 				}
// 				return false;
// 			}
// 			else if (moveGridPos == IntVec2(4, 7) && toGridPos == IntVec2(7, 7))
// 			{
// 				if (ValidateCastling(moveChessIndex, toChessIndex, false, out_result))
// 				{
// 					out_result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
// 					return true;
// 				}
// 				return false;
// 			}
// 		}
// 		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
// 		return false;
// 	}
	
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;
	ChessPiece* movingKing = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(moveChessIndex);

	// Castle
	if (abs(moveStep.x) == 2 && moveStep.y == 0) 
	{
		if (movingKing->GetFaction() == Faction::WHITE)
		{
			// e1 -> c1
			if (moveGridPos == IntVec2(4, 0) && toGridPos == IntVec2(2, 0))
			{
				int rookIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(IntVec2(0, 0));
				if (ValidateCastling(moveChessIndex, rookIndex, true, out_result)) // 改为rookIndex
				{
					out_result = ChessMoveResult::VALID_CASTLE_QUEENSIDE;
					return true;
				}
				return false;
			}
			// e1 -> g1
			else if (moveGridPos == IntVec2(4, 0) && toGridPos == IntVec2(6, 0))
			{
				int rookIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(IntVec2(7, 0));
				if (ValidateCastling(moveChessIndex, rookIndex, false, out_result)) // 改为rookIndex
				{
					out_result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
					return true;
				}
				return false;
			}
		}
		else if (movingKing->GetFaction() == Faction::BLACK)
		{
			// e8 -> c8
			if (moveGridPos == IntVec2(4, 7) && toGridPos == IntVec2(2, 7))
			{
				int rookIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(IntVec2(0, 7));
				if (ValidateCastling(moveChessIndex, rookIndex, true, out_result)) // 改为rookIndex
				{
					out_result = ChessMoveResult::VALID_CASTLE_QUEENSIDE;
					return true;
				}
				return false;
			}
			// e8 -> g8
			else if (moveGridPos == IntVec2(4, 7) && toGridPos == IntVec2(6, 7))
			{
				int rookIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(IntVec2(7, 7));
				if (ValidateCastling(moveChessIndex, rookIndex, false, out_result)) // 改为rookIndex
				{
					out_result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
					return true;
				}
				return false;
			}
		}
		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		return false;
	}

	// Regular Move Shape
	if (!(moveStep == IntVec2(1, 0) || moveStep == IntVec2(-1, 0) ||
		moveStep == IntVec2(0, 1) || moveStep == IntVec2(0, -1) ||
		moveStep == IntVec2(1, 1) || moveStep == IntVec2(-1, -1) ||
		moveStep == IntVec2(1, -1) || moveStep == IntVec2(-1, 1)))
	{
		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		return false;
	}

	// Adjacent King from enemy faction
	Faction enemyFaction = (movingKing->GetFaction() == Faction::WHITE) ? Faction::BLACK : Faction::WHITE;
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			if (dx == 0 && dy == 0) continue;

			IntVec2 adjacentPos = toGridPos + IntVec2(dx, dy);
			if (adjacentPos.x >= 0 && adjacentPos.x < 8 && adjacentPos.y >= 0 && adjacentPos.y < 8)
			{
				int adjacentIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(adjacentPos);
				ChessPiece* adjacentPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(adjacentIndex);

				if (adjacentPiece && adjacentPiece->GetPieceType() == PieceType::KING &&
					adjacentPiece->GetFaction() == enemyFaction)
				{
					out_result = ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
					return false;
				}
			}
		}
	}

	// Valid
	if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
	{
		out_result = ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else
	{
		out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
	}

	return true;
}

bool Game::ValidateQueenMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;

	// Regular Move Shape
	if (!(moveStep.x == 0 || moveStep.y == 0 ||
		abs(moveStep.x) == abs(moveStep.y)))
	{
		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		return false;
	}
	// Block
	IntVec2 direction;
	direction.x = (moveStep.x == 0) ? 0 : ((moveStep.x > 0) ? 1 : -1);
	direction.y = (moveStep.y == 0) ? 0 : ((moveStep.y > 0) ? 1 : -1);
	IntVec2 checkPos = moveGridPos + direction;
	while (checkPos != toGridPos)
	{
		int checkIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(checkPos);
		if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(checkIndex) != nullptr)
		{
			out_result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
			return false;
		}
		checkPos += direction;
	}

	// Valid
	if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
	{
		out_result = ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else
	{
		out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
	}

	return true;
}

bool Game::ValidateRookMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;

	// Regular Move Shape
	if (!(moveStep.x == 0 || moveStep.y == 0 ))
	{
		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		return false;
	}
	// Block
	IntVec2 direction;
	direction.x = (moveStep.x == 0) ? 0 : ((moveStep.x > 0) ? 1 : -1);
	direction.y = (moveStep.y == 0) ? 0 : ((moveStep.y > 0) ? 1 : -1);
	IntVec2 checkPos = moveGridPos + direction;
	while (checkPos != toGridPos)
	{
		int checkIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(checkPos);
		if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(checkIndex) != nullptr)
		{
			out_result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
			return false;
		}
		checkPos += direction;
	}

	// Valid
	if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
	{
		out_result = ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else
	{
		out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
	}

	return true;
}

bool Game::ValidateBishopMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;

	// Regular Move Shape
	if (!(abs(moveStep.x) == abs(moveStep.y)))
	{
		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		return false;
	}
	// Block
	IntVec2 direction;
	direction.x = (moveStep.x > 0) ? 1 : -1;
	direction.y = (moveStep.y > 0) ? 1 : -1;
	IntVec2 checkPos = moveGridPos + direction;
	while (checkPos != toGridPos)
	{
		int checkIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(checkPos);
		if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(checkIndex) != nullptr)
		{
			out_result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
			return false;
		}
		checkPos += direction;
	}

	// Valid
	if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
	{
		out_result = ChessMoveResult::VALID_MOVE_NORMAL;
	}
	else
	{
		out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
	}

	return true;
}

bool Game::ValidateKnightMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;
	int deltaX = abs(toGridPos.x - moveGridPos.x);
	int deltaY = abs(toGridPos.y - moveGridPos.y);

	bool isValidLMove = (deltaX == 2 && deltaY == 1) || (deltaX == 1 && deltaY == 2);

	if (!isValidLMove) 
	{
		out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
		return false;
	}

	out_result = ChessMoveResult::VALID_MOVE_NORMAL;
	return true;
}

bool Game::ValidatePawnMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;
	ChessPiece* movePiece=g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(moveChessIndex);

	out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;  //initial

	// normal
	if (movePiece->GetFaction() == Faction::WHITE)
	{
		if (moveStep.x == 0 && moveStep.y == 1)
		{
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				out_result = ChessMoveResult::VALID_MOVE_NORMAL;
			}
			else
			{
				out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE; 
				return false;
			}
		}
	}
	else if (movePiece->GetFaction() == Faction::BLACK)
	{
		if (moveStep.x == 0 && moveStep.y == -1)
		{
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				out_result = ChessMoveResult::VALID_MOVE_NORMAL;
			}
			else
			{
				out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				return false;
			}
		}
	}

	// 2 dist
	if (movePiece->GetFaction() == Faction::WHITE)
	{
		if (moveStep.x == 0 && moveStep.y == 2&&movePiece->GetIsFirstMove())
		{
			// Block
			IntVec2 checkGrid = moveGridPos + IntVec2(0, 1);
			int checkIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(checkGrid);
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(checkIndex) != nullptr)
			{
				out_result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
				return false;
			}

			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				out_result = ChessMoveResult::VALID_MOVE_NORMAL;
			}
			else
			{
				out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				return false;
			}
			//movePiece->SetIsFirstMove(false);
		}
	}
	else if (movePiece->GetFaction() == Faction::BLACK)
	{
		if (moveStep.x == 0 && moveStep.y == -2 && movePiece->GetIsFirstMove())
		{
			// Block
			IntVec2 checkGrid = moveGridPos + IntVec2(0, -1);
			int checkIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(checkGrid);
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(checkIndex) != nullptr)
			{
				out_result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
				return false;
			}

			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				out_result = ChessMoveResult::VALID_MOVE_NORMAL;
			}
			else
			{
				out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				return false;
			}
		}
	}

	// diagonally catch
	if (movePiece->GetFaction() == Faction::WHITE)
	{
		if ((moveStep.x == 1 || moveStep.x == -1) && moveStep.y == 1)
		{
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				// ENPASSANT
				IntVec2 leftGridPos = moveGridPos + IntVec2(-1, 0);
				IntVec2 rightGridPos = moveGridPos + IntVec2(1, 0);
				ChessPiece* leftPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromGridPos(leftGridPos);
				ChessPiece* rightPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromGridPos(rightGridPos);

				if (g_theGame->m_curMatch->m_chessBoard.m_lastMovingChess == leftPiece
					&&leftPiece->GetGridPos()-leftPiece->GetPrevGridPos()==IntVec2(0,-2))
				{
					out_result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
				}
				else if (g_theGame->m_curMatch->m_chessBoard.m_lastMovingChess == rightPiece
					&& rightPiece->GetGridPos() - rightPiece->GetPrevGridPos() == IntVec2(0, -2))
				{
					out_result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
				}
				else
				{
					out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
					return false;
				}
			}
			else
			{
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
			}
			
		}
	}
	else if (movePiece->GetFaction() == Faction::BLACK)
	{
		if ((moveStep.x == 1 || moveStep.x == -1) && moveStep.y == -1)
		{
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				// ENPASSANT
				IntVec2 leftGridPos = moveGridPos + IntVec2(-1, 0);
				IntVec2 rightGridPos = moveGridPos + IntVec2(1, 0);
				ChessPiece* leftPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromGridPos(leftGridPos);
				ChessPiece* rightPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromGridPos(rightGridPos);

				if (g_theGame->m_curMatch->m_chessBoard.m_lastMovingChess == leftPiece
					&& leftPiece->GetGridPos() - leftPiece->GetPrevGridPos() == IntVec2(0, 2))
				{
					out_result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
				}
				else if (g_theGame->m_curMatch->m_chessBoard.m_lastMovingChess == rightPiece
					&& rightPiece->GetGridPos() - rightPiece->GetPrevGridPos() == IntVec2(0, 2))
				{
					out_result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
				}
				else
				{
					out_result = ChessMoveResult::INVALID_ENPASSANT_STALE;
					return false;
				}
			}
			else
			{
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
			}
		}
	}

	// promotion
	if ((toGridPos.y == 7&& movePiece->GetFaction() == Faction::WHITE)||
		(toGridPos.y == 0&& movePiece->GetFaction() == Faction::BLACK))
	{
		out_result = ChessMoveResult::VALID_MOVE_PROMOTION;
	}

	if (IsValidateChessMoveResult(out_result))
	{
		return true;
	}
	
	return false;
}

bool Game::ValidateCastling(int kingIndex, int targetIndex, bool isQueenside, ChessMoveResult& out_result)
{
// 	ChessPiece* kingChess = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(kingIndex);
// 	if (!kingChess->GetIsFirstMove())
// 	{
// 		out_result = ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED;
// 		return false;
// 	}
// 
// 	ChessPiece* targetChess= g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(targetIndex);
// 	if (targetChess->GetPieceType() != PieceType::ROOK||!targetChess->GetIsFirstMove())
// 	{
// 		out_result = ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
// 		return false;
// 	}
// 
// 	int startX = std::min(kingChess->GetGridPos().x, targetChess->GetGridPos().x) + 1;
// 	int endX = std::max(kingChess->GetGridPos().x, targetChess->GetGridPos().x) - 1;
// 
// 	for (int x = startX; x <= endX; x++)
// 	{
// 		IntVec2 checkPos(x, kingChess->GetGridPos().y);
// 		int checkIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(checkPos);
// 		if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(checkIndex) != nullptr)
// 		{
// 			out_result = ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED;
// 			return false;
// 		}
// 	}
// 
// 	isQueenside ? out_result = ChessMoveResult::VALID_CASTLE_QUEENSIDE : out_result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
// 	//out_result = ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED;
// 	return true;
	ChessPiece* kingChess = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(kingIndex);
    ChessPiece* rookChess = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(targetIndex);
    
    if (!kingChess->GetIsFirstMove())
    {
        out_result = ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED;
        return false;
    }

    if (!rookChess || rookChess->GetPieceType() != PieceType::ROOK || !rookChess->GetIsFirstMove())
    {
        out_result = ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
        return false;
    }
    
    IntVec2 kingPos = kingChess->GetGridPos();
    IntVec2 rookPos = rookChess->GetGridPos();
    
    int startX = std::min(kingPos.x, rookPos.x) + 1;
    int endX = std::max(kingPos.x, rookPos.x) - 1;
    
    for (int x = startX; x <= endX; x++)
    {
        IntVec2 checkPos(x, kingPos.y);
        int checkIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromGridPos(checkPos);
        if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(checkIndex) != nullptr)
        {
            out_result = ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED;
            return false;
        }
    }
    
	if (isQueenside)
	{
		out_result = ChessMoveResult::VALID_CASTLE_QUEENSIDE;
	}
	else
	{
		out_result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
	}
    return true;
}


void Game::AddLineForMoveResult(ChessMoveResult result)
{
	switch (result)
	{
	case ChessMoveResult::VALID_MOVE_NORMAL:
		break;
	case ChessMoveResult::VALID_MOVE_PROMOTION:
		g_theDevConsole->AddLine(DevConsole::TIPS, "Pawn promoted successfully.");
		break;
	case ChessMoveResult::VALID_CASTLE_KINGSIDE:
		g_theDevConsole->AddLine(DevConsole::TIPS, "Castled king-side successfully.");
		break;
	case ChessMoveResult::VALID_CASTLE_QUEENSIDE:
		g_theDevConsole->AddLine(DevConsole::TIPS, "Castled queen-side successfully.");
		break;
	case ChessMoveResult::VALID_CAPTURE_NORMAL:
		g_theDevConsole->AddLine(DevConsole::TIPS, "Piece captured successfully.");
		break;
	case ChessMoveResult::VALID_CAPTURE_ENPASSANT:
		g_theDevConsole->AddLine(DevConsole::TIPS, "En passant capture successful.");
		break;
	case ChessMoveResult::VALID_MOVE_CHEAT:
		g_theDevConsole->AddLine(DevConsole::TIPS, "Teleporting cheat.");
		break;
	case ChessMoveResult::INVALID_MOVE_BAD_LOCATION:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: Position is outside the board (must be a1-h8).");
		break;
	case ChessMoveResult::INVALID_MOVE_NO_PIECE:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: No piece at the specified 'from' position.");
		break;
	case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: The piece at 'from' position doesn't belong to you.");
		break;
	case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: Cannot move a piece to its current position.");
		break;
	case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: This piece cannot move in that pattern.");
		break;
	case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: DESTINATION BLOCKED.");
		break;
	case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: Path is blocked by another piece.");
		break;
	case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: This move would put your king in check.");
		break;
	case ChessMoveResult::INVALID_MOVE_KING_TOO_CLOSE:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: The King of enemy faction is too closed.");
		break;
	case ChessMoveResult::INVALID_ENPASSANT_STALE:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid en passant: Can only capture en passant immediately after opponent's pawn moves 2 squares.");
		break;
	case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid castling: King has already moved.");
		break;
	case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid castling: Rook has already moved.");
		break;
	case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid castling: Path between king and rook is blocked.");
		break;
	case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid castling: Cannot castle through check.");
		break;
	case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid castling: Cannot castle while in check.");
		break;
	case ChessMoveResult::UNKNOWN:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Unknown move result.");
		break;
	default:
		g_theDevConsole->AddLine(DevConsole::INVALID, "Unknown move result.");
		break;
	}
}

bool Game::ChessServerInfo_LocalCmd(EventArgs& args)
{
	std::string newIP = args.GetValue("ip", "");
	std::string newPort = args.GetValue("port", "");
	if (g_theNetworkSystem && g_theNetworkSystem->IsConnected())
	{
		if (!newIP.empty() || !newPort.empty())
		{
			g_theDevConsole->AddLine(DevConsole::INVALID, "Cannot change IP/port while connected!");
			return false;
		}
	}

	if (!newIP.empty())
	{
		g_theNetworkSystem->m_networkConfig.m_serverIP = newIP;
	}

	if (!newPort.empty())
	{
		int portNum = atoi(newPort.c_str());
		if (portNum > 0 && portNum <= 65535)
		{
			g_theNetworkSystem->m_networkConfig.m_serverPort = (uint16_t)portNum;
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid port number!");
			return false;
		}
	}

	g_theDevConsole->AddLine(DevConsole::TIPS, "=== Chess Network Info ===");
	g_theDevConsole->AddLine(DevConsole::PLAYER_TIP,
		Stringf("Player Name: %s", g_theNetworkSystem->m_networkConfig.m_playerName.c_str()));
	g_theDevConsole->AddLine(DevConsole::PLAYER_TIP,
		Stringf("Server IP: %s", g_theNetworkSystem->m_networkConfig.m_serverIP.c_str()));
	g_theDevConsole->AddLine(DevConsole::PLAYER_TIP,
		Stringf("Port: %d", g_theNetworkSystem->m_networkConfig.m_serverPort));

	// 显示连接状态
	if (g_theNetworkSystem)
	{
		std::string connectionStatus = "DISCONNECTED";
		NetState netState = g_theNetworkSystem->GetNetState();
		if (netState ==NetState::NET_STATE_SERVER_LISTENING)
		{
			connectionStatus = "SERVER (Listening)";
		}
		else if (netState == NetState::NET_STATE_CLIENT_CONNECTED)
		{
			connectionStatus = "CLIENT (Connected)";
		}
		else if (netState == NetState::NET_STATE_CLIENT_CONNECTING)
		{
			connectionStatus = "CLIENT (Connecting...)";
		}
		else if (netState == NetState::NET_STATE_INACTIVE)
		{
			connectionStatus = "INACTIVE";
		}
		else if (netState == NetState::NET_STATE_IDLE)
		{
			connectionStatus = "IDLE";
		}
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP,
			Stringf("Connection Status: %s", connectionStatus.c_str()));
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "Connection Status: Network System Not Started");
	}

	// 显示游戏状态
// 	if (g_theChessGame)
// 	{
// 		std::string gameState = g_theChessGame->GetCurrentGameStateString(); // 你需要实现这个方法
// 		g_theDevConsole->AddLine(DevConsole::INFO_MINOR,
// 			Stringf("Game State: %s", gameState.c_str()));
// 	}
// 	else
// 	{
// 		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Game State: No Active Game");
// 	}

	return true;
}

bool Game::ChessListen_LocalCmd(EventArgs& args)
{
	std::string portStr = args.GetValue("port", "");
	if (!portStr.empty())
	{
		g_theNetworkSystem->m_networkConfig.m_serverPort= (uint16_t)atoi(portStr.c_str());
	}
	g_theNetworkSystem->StartServer(g_theNetworkSystem->m_networkConfig.m_serverPort);
	return false;
}

bool Game::ChessConnect_LocalCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}

	NetworkSystemConfig networkConfig = g_theNetworkSystem->m_networkConfig;
	//NetState curState = g_theNetworkSystem->GetNetState();
	if (g_theNetworkSystem->GetNetState() ==NetState::NET_STATE_SERVER_LISTENING 
		|| g_theNetworkSystem->GetNetState() == NetState::NET_STATE_CLIENT_CONNECTED
		|| g_theNetworkSystem->GetNetState() == NetState::NET_STATE_CLIENT_CONNECTING)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "Already connected or connecting!");
		return false;
	}

	std::string targetIP = args.GetValue("ip", networkConfig.m_serverIP);
	std::string portStr = args.GetValue("port", std::to_string(networkConfig.m_serverPort));

	int targetPort = atoi(portStr.c_str());
	if (targetPort <= 0 || targetPort > 65535)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID,
			Stringf("Invalid port number: %s", portStr.c_str()));
		return false;
	}

	if (targetIP.empty())
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid IP address!");
		return false;
	}

	g_theNetworkSystem->m_networkConfig.m_serverIP = targetIP;
	g_theNetworkSystem->m_networkConfig.m_serverPort = (uint16_t)targetPort;

	if (g_theNetworkSystem->GetNetState() ==NetState::NET_STATE_INACTIVE)
	{
		g_theNetworkSystem->Startup();
		if (g_theNetworkSystem->GetNetState() == NetState::NET_STATE_INACTIVE)
		{
			g_theDevConsole->AddLine(DevConsole::INVALID, "Failed to startup NetworkSystem!");
			return false;
		}
	}

	g_theNetworkSystem->StartClient(targetIP, targetPort);
	if (g_theNetworkSystem->GetNetState()!=NetState::NET_STATE_CLIENT_CONNECTED&&
		g_theNetworkSystem->GetNetState() != NetState::NET_STATE_CLIENT_CONNECTING)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID,
			Stringf("Failed to start connection to %s:%d", targetIP.c_str(), targetPort));
		return false;
	}

	g_theDevConsole->AddLine(DevConsole::TIPS,
		Stringf("Attempting to connect to %s:%d...", targetIP.c_str(), targetPort));

	return true;
}

bool Game::ChessDisconnect_RemoteCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}
	NetState curState = g_theNetworkSystem->GetNetState();
	if (curState != NetState::NET_STATE_SERVER_LISTENING &&
		curState != NetState::NET_STATE_CLIENT_CONNECTED &&
		curState != NetState::NET_STATE_CLIENT_CONNECTING)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "Not connected to any remote host!");
		return false;
	}

	std::string reason = args.GetValue("reason", "");
	std::string remoteFlag = args.GetValue("remote", "false");
	bool isRemoteCommand = (remoteFlag == "true");
	if (isRemoteCommand)
	{
		// 远程断开命令 - 只断开，不回发
		if (!reason.empty())
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("Remote host disconnected: %s", reason.c_str()));
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::TIPS, "Remote host disconnected.");
		}

		// 根据当前状态调用对应的停止函数
		if (curState == NetState::NET_STATE_SERVER_LISTENING)
		{
			g_theNetworkSystem->StopServer();
		}
		else if (curState == NetState::NET_STATE_CLIENT_CONNECTED ||
			curState == NetState::NET_STATE_CLIENT_CONNECTING)
		{
			g_theNetworkSystem->StopClient();
		}
	}
	else
	{
		std::string remoteCmdString = "RemoteCmd cmd=ChessDisconnect";
		if (!reason.empty())
		{
			remoteCmdString += " reason=" + reason;
		}
		g_theDevConsole->Execute(remoteCmdString);

		if (!reason.empty())
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("Disconnecting: %s", reason.c_str()));
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::TIPS, "Disconnecting...");
		}

		// Stop Self
		if (curState == NetState::NET_STATE_SERVER_LISTENING)
		{
			g_theNetworkSystem->StopServer();
		}
		else if (curState == NetState::NET_STATE_CLIENT_CONNECTED ||
			curState == NetState::NET_STATE_CLIENT_CONNECTING)
		{
			g_theNetworkSystem->StopClient();
		}
	}

	// Verify if success disconnect
	NetState newState = g_theNetworkSystem->GetNetState();
	if (newState != NetState::NET_STATE_IDLE && newState != NetState::NET_STATE_INACTIVE)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "Failed to disconnect properly!");
		return false;
	}

	return true;
}

bool Game::ChessPlayerInfo_RemoteCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}
	std::string playerName = args.GetValue("name", "");
	if (playerName.empty())
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "Player name cannot be empty!");
		return false;
	}

	bool isRemoteCommand = args.GetValue("remote", false);
	if (isRemoteCommand)
	{
		// 远程命令：设置对手名称
		g_theNetworkSystem->m_networkConfig.m_opponentName = playerName;
		g_theDevConsole->AddLine(DevConsole::TIPS,
			Stringf("Opponent player name set to: %s", playerName.c_str()));
	}
	else
	{
		// 本地命令：设置自己的名称并发送给远程主机
		g_theNetworkSystem->m_networkConfig.m_playerName = playerName;
		g_theDevConsole->AddLine(DevConsole::TIPS,
			Stringf("Local player name set to: %s", playerName.c_str()));

		// 检查是否有连接可以发送命令
		NetState curState = g_theNetworkSystem->GetNetState();
		if (curState == NetState::NET_STATE_CLIENT_CONNECTED ||
			(curState == NetState::NET_STATE_SERVER_LISTENING && g_theNetworkSystem->HasConnectedClients()))
		{
			std::string remoteCmdString = "RemoteCmd cmd=ChessPlayerInfo name=" + playerName;
			g_theDevConsole->Execute(remoteCmdString);
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				"Player name updated locally (no remote connection).");
		}
	}

	return true;
}

bool Game::ChessBegin_RemoteCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}
	NetState curState = g_theNetworkSystem->GetNetState();
	if (curState != NetState::NET_STATE_CLIENT_CONNECTED &&
		(curState != NetState::NET_STATE_SERVER_LISTENING || !g_theNetworkSystem->HasConnectedClients()))
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No remote connection available!");
		return false;
	}

	std::string firstPlayerName = args.GetValue("firstPlayer", "");
	std::string remoteFlag = args.GetValue("remote", "false");
	bool isRemoteCommand = (remoteFlag == "true");
	std::string myName = g_theNetworkSystem->m_networkConfig.m_playerName;
	std::string opponentName = g_theNetworkSystem->m_networkConfig.m_opponentName;

	if (isRemoteCommand)
	{
		// 远程命令：对方发起新游戏
		if (firstPlayerName.empty())
		{
			firstPlayerName = opponentName;
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("Remote host started game without specifying first player, defaulting to %s.", opponentName.c_str()));
		}

		// 验证firstPlayer是否为有效的玩家名称
		if (firstPlayerName != myName && firstPlayerName != opponentName)
		{
			g_theDevConsole->AddLine(DevConsole::INVALID,
				Stringf("Invalid firstPlayer '%s' from remote host. Must be '%s' or '%s'. Rejecting game start.",
					firstPlayerName.c_str(), myName.c_str(), opponentName.c_str()));
			return false;
		}

		// 确定谁是先手
		bool selfIsFirst = (firstPlayerName == myName);

		// 开始新游戏
		g_theEventSystem->FireEvent("ChessBeginMatch");
		g_theGame->m_curMatch->m_selfIsFirst = selfIsFirst;
		g_theGame->m_chessGameState = ChessGameState::FIRST_PLAYER_TURN;
		// 显示游戏开始信息
		if (selfIsFirst)
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("New game started by %s. You play first (White).", opponentName.c_str()));
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("New game started by %s. %s plays first (White).", opponentName.c_str(), firstPlayerName.c_str()));
		}
	}
	else
	{
		// 本地命令，本地发起新游戏
		// 如果没有指定firstPlayer，默认为自己
		if (firstPlayerName.empty())
		{
			firstPlayerName = myName;
		}

		if (firstPlayerName != myName && firstPlayerName != opponentName)
		{
			g_theDevConsole->AddLine(DevConsole::INVALID,
				Stringf("Invalid firstPlayer: %s. Must be '%s' or '%s'.",
					firstPlayerName.c_str(), myName.c_str(), opponentName.c_str()));
			return false;
		}

		bool selfIsFirst = (firstPlayerName == myName);

		g_theEventSystem->FireEvent("ChessBeginMatch");
		g_theGame->m_curMatch->m_selfIsFirst = selfIsFirst;
		g_theGame->m_chessGameState = ChessGameState::FIRST_PLAYER_TURN;
		// construct the cmd
		std::string remoteCmdString = "RemoteCmd cmd=ChessBegin firstPlayer=" + firstPlayerName;
		g_theDevConsole->Execute(remoteCmdString);

		// 显示游戏开始信息
		if (selfIsFirst)
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				"New game started. You play first (White).");
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("New game started. %s plays first (White).", firstPlayerName.c_str()));
		}
	}
	return true;
}

bool Game::ChessValidate_RemoteCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}

	NetState curState = g_theNetworkSystem->GetNetState();
	if (curState != NetState::NET_STATE_CLIENT_CONNECTED &&
		(curState != NetState::NET_STATE_SERVER_LISTENING || !g_theNetworkSystem->HasConnectedClients()))
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No remote connection available!");
		return false;
	}

	if (!g_theGame->m_curMatch)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No active chess match!");
		return false;
	}

	std::string remoteFlag = args.GetValue("remote", "false");
	bool isRemoteCommand = (remoteFlag == "true");

	if (isRemoteCommand)
	{
		//std::string remoteState = args.GetValue("state", "");
		std::string remotePlayer1 = args.GetValue("player1", "");
		std::string remotePlayer2 = args.GetValue("player2", "");
		std::string remoteMoveStr = args.GetValue("move", "");
		std::string remoteBoard = args.GetValue("board", "");

		// 获取本地游戏状态
		std::string localPlayer1 = g_theNetworkSystem->m_networkConfig.m_playerName;
		std::string localPlayer2 = g_theNetworkSystem->m_networkConfig.m_opponentName;
		int localMove = g_theGame->m_curMatch->m_turnNumber;
		std::string localBoardString = g_theGame->m_curMatch->GetBoardStateString();
		g_theGame->PrintBoardState();

		// 验证各项是否一致
		std::vector<std::string> discontinuities;

		bool playerNamesMatch = false;
		if ((remotePlayer1 == localPlayer1 && remotePlayer2 == localPlayer2) ||
			(remotePlayer1 == localPlayer2 && remotePlayer2 == localPlayer1))
		{
			playerNamesMatch = true;
		}

		if (!playerNamesMatch)
		{
			discontinuities.push_back(Stringf("Player Names: remote='%s,%s', local='%s,%s'",
				remotePlayer1.c_str(), remotePlayer2.c_str(),
				localPlayer1.c_str(), localPlayer2.c_str()));
		}

		int remoteMove = atoi(remoteMoveStr.c_str());
		if (remoteMove != localMove)
		{
			discontinuities.push_back(Stringf("Move Number: remote=%d, local=%d",
				remoteMove, localMove));
		}

		if (remoteBoard != localBoardString)
		{
			discontinuities.push_back(Stringf("Board State: remote='%s', local='%s'",
				remoteBoard.c_str(), localBoardString.c_str()));
		}

		// 如果发现任何不一致
		if (!discontinuities.empty())
		{
			g_theDevConsole->AddLine(DevConsole::INVALID, "=== VALIDATION FAILED ===");
			g_theDevConsole->AddLine(DevConsole::INVALID, "Game state discontinuities found:");

			for (const std::string& discontinuity : discontinuities)
			{
				g_theDevConsole->AddLine(DevConsole::INVALID, "  " + discontinuity);
			}

			// 发送断开连接消息
			std::string disconnectCmd = "RemoteCmd cmd=ChessDisconnect reason=\"VALIDATION FAILED\"";
			g_theDevConsole->Execute(disconnectCmd);

			// 断开连接
			if (curState == NetState::NET_STATE_SERVER_LISTENING)
			{
				g_theNetworkSystem->StopServer();
			}
			else if (curState == NetState::NET_STATE_CLIENT_CONNECTED)
			{
				g_theNetworkSystem->StopClient();
			}

			return false;
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::TIPS, "Game state validation passed successfully.");
		}
	}
	else
	{
		// 本地验证命令：发送当前游戏状态给对方验证

		//std::string currentState = g_theGame->m_curMatch->GetCurrentGameStateString();
		std::string player1 = g_theNetworkSystem->m_networkConfig.m_playerName;
		std::string player2 = g_theNetworkSystem->m_networkConfig.m_opponentName;
		int currentMove = g_theGame->m_curMatch->m_turnNumber;
		std::string boardState = g_theGame->m_curMatch->GetBoardStateString();

		// 构建完整的验证命令
		std::string validateCmd = "RemoteCmd cmd=ChessValidate";
		//validateCmd += " state=" + currentState;
		validateCmd += " player1=" + player1;
		validateCmd += " player2=" + player2;
		validateCmd += " move=" + std::to_string(currentMove);
		validateCmd += " board=" + boardState;

		// 发送验证命令
		g_theDevConsole->Execute(validateCmd);
// 		if (sendSuccess)
// 		{
// 			g_theDevConsole->AddLine(DevConsole::TIPS, "Validation request sent to opponent.");
// 		}
// 		else
// 		{
// 			g_theDevConsole->AddLine(DevConsole::INVALID, "Failed to send validation request!");
// 			return false;
// 		}
	}

	return true;
}

bool Game::ChessResign_RemoteCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}

	if (!g_theGame->m_curMatch)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No active chess match!");
		return false;
	}

	// 检查游戏是否正在进行中
	if (g_theGame->m_chessGameState==ChessGameState::WAIT_TO_START_GAME)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "Game is already over!");
		return false;
	}

	std::string remoteFlag = args.GetValue("remote", "false");
	bool isRemoteCommand = (remoteFlag == "true");

	if (isRemoteCommand)
	{
		// 远程认输：对手认输了，我获胜
		g_theDevConsole->AddLine(DevConsole::TIPS,
			Stringf("%s has resigned. You win!",
				g_theNetworkSystem->m_networkConfig.m_opponentName.c_str()));
	}
	else
	{
		// 本地认输：我认输，对手获胜
		// 检查是否有网络连接来通知对手
		NetState curState = g_theNetworkSystem->GetNetState();
		if (curState == NetState::NET_STATE_CLIENT_CONNECTED ||
			(curState == NetState::NET_STATE_SERVER_LISTENING && g_theNetworkSystem->HasConnectedClients()))
		{
			// 通知对手我认输了
			std::string resignCmd = "RemoteCmd cmd=ChessResign";
			g_theDevConsole->Execute(resignCmd);
		}

		g_theDevConsole->AddLine(DevConsole::TIPS, "You have resigned. Game over.");
	}

	g_theGame->m_chessGameState = ChessGameState::WAIT_TO_START_GAME;
	return true;
}

bool Game::ChessOfferDraw_RemoteCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}

	if (!g_theGame->m_curMatch)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No active chess match!");
		return false;
	}

	std::string remoteFlag = args.GetValue("remote", "false");
	bool isRemoteCommand = (remoteFlag == "true");

	if (isRemoteCommand)
	{
		g_theDevConsole->AddLine(DevConsole::TIPS,
			Stringf("%s is offering a draw. Use 'ChessAcceptDraw' or 'ChessRejectDraw' to respond.",
				g_theNetworkSystem->m_networkConfig.m_opponentName.c_str()));

		// 设置对手提议和棋的状态
		g_theGame->m_curMatch->m_isOpponentOfferedDraw = true;
	}
	else
	{
		// 本地提议：我向对手提议和棋
		// 发送和棋提议给对手
		std::string offerDrawCmd = "RemoteCmd cmd=ChessOfferDraw";
		g_theDevConsole->Execute(offerDrawCmd);

		g_theDevConsole->AddLine(DevConsole::TIPS, "Draw offer sent to opponent. Waiting for response...");

		// 记录自己在本回合提议了和棋
		g_theGame->m_curMatch->m_isSelfOfferedDraw=true;
	}

	return true;
}

bool Game::ChessAcceptDraw_RemoteCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}

	if (!g_theGame->m_curMatch)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No active chess match!");
		return false;
	}

	std::string remoteFlag = args.GetValue("remote", "false");
	bool isRemoteCommand = (remoteFlag == "true");

	if (isRemoteCommand)
	{
		if (g_theGame->m_curMatch->m_isSelfOfferedDraw)
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("Opponent accept your draw offer."));
		}
	}
	else
	{
		if (g_theGame->m_curMatch->m_isOpponentOfferedDraw)
		{
			g_theDevConsole->AddLine(DevConsole::TIPS,
				Stringf("You accept opponent's draw."));
			std::string resignCmd = "RemoteCmd cmd=ChessAcceptDraw";
			g_theDevConsole->Execute(resignCmd);
		}
	}
	g_theGame->m_curMatch->m_isSelfOfferedDraw = false;
	g_theGame->m_curMatch->m_isOpponentOfferedDraw = false;
	g_theGame->m_chessGameState = ChessGameState::WAIT_TO_START_GAME;

	return true;
}

bool Game::RemoteCmd_LocalCmd(EventArgs& args)
{
	if (!g_theNetworkSystem)
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "NetworkSystem not available!");
		return false;
	}

	// 检查网络连接
	NetState curState = g_theNetworkSystem->GetNetState();
	if (curState != NetState::NET_STATE_CLIENT_CONNECTED &&
		(curState != NetState::NET_STATE_SERVER_LISTENING || !g_theNetworkSystem->HasConnectedClients()))
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "No remote connection available!");
		return false;
	}

	// 获取命令名称
	std::string commandName = args.GetValue("cmd", "");
	if (commandName.empty())
	{
		g_theDevConsole->AddLine(DevConsole::INVALID, "RemoteCmd requires cmd parameter!");
		return false;
	}

	// 构建远程命令字符串（不包含 "RemoteCmd cmd="）
	std::string remoteCommand = commandName;

	// 添加其他所有参数（除了cmd）
	for (const auto& pair : args.m_keyValuePairs)
	{
		if (pair.first != "cmd")
		{
			remoteCommand += " " + pair.first + "=" + pair.second;
		}
	}

	// 发送到网络
	bool sendSuccess = g_theNetworkSystem->SendCommandToRemote(remoteCommand);

	if (sendSuccess)
	{
		g_theDevConsole->AddLine(DevConsole::TIPS,
			Stringf("Sent remote command: %s", remoteCommand.c_str()));
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::INVALID,
			Stringf("Failed to send remote command: %s", remoteCommand.c_str()));
	}

	return sendSuccess;
}

void Game::StartTheMatch()
{
	if (!m_curMatch)
	{
		m_curMatch = new ChessMatch(this);
		m_chessGameState = ChessGameState::FIRST_PLAYER_TURN;
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "The Match is Begin!");
		PrintBoardState();

		if (m_camMode == CamMode::AUTO)
		{
			m_player->m_position = m_whiteCamPosition;
			m_player->m_orientation = m_whiteOrientation;
		}
	}
}

void Game::EndTheMatch()
{
	if (m_curMatch)
	{
		delete m_curMatch;
		m_curMatch = nullptr;
		m_chessGameState = ChessGameState::WAIT_TO_START_GAME;
		g_theDevConsole->AddLine(DevConsole::PLAYER_TIP, "The Match is Ended, Try ChessBegin Command!");
	}
}

void Game::SetObservationCamPosition(ChessGameState chessGameState)
{
	switch (chessGameState)
	{
	case ChessGameState::WAIT_TO_START_GAME:
		break;

	case ChessGameState::FIRST_PLAYER_TURN:
		g_theGame->m_player->m_position = g_theGame->m_whiteCamPosition;
		g_theGame->m_player->m_orientation = g_theGame->m_whiteOrientation;
		break;
	case ChessGameState::SEC_PLAYER_TURN:
		g_theGame->m_player->m_position = g_theGame->m_blackCamPosition;
		g_theGame->m_player->m_orientation = g_theGame->m_blackOrientation;
		break;

	case ChessGameState::FIRST_WIN:
	case ChessGameState::SEC_WIN:
		m_player->m_position = g_theGame->m_winCamPosition;
		m_player->m_orientation = g_theGame->m_winOrientation;
		break;
	default:
		break;
	}
}

Vec3 Game::GetPlayerCamPosition()
{
	return m_player->m_position;
}

Vec3 Game::GetPlayerCamDirection()
{
	return m_player->m_orientation.GetForward_IFwd();
}














