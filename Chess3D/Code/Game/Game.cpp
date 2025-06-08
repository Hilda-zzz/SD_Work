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

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;
extern Game* g_theGame;

Game::Game()
{
	m_gridTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_gameClock = new Clock();
	m_player = new Player(this);
// 	m_cube = new Prop(this);
// 	m_cube2 = new Prop(this);
// 	m_cube->m_position = Vec3(2.f, 2.f, 0.f);
// 	m_cube2->m_position = Vec3(-2.f, -2.f, 0.f);
// 	m_cube2->m_color = Rgba8(100,100,100,255);
// 	m_groundGrid = new Prop(this);
// 	m_sphere = new Prop(this);
// 	m_sphere->m_position = Vec3(10.f, -5.f, 1.f);
// 	m_sphere->m_texture = m_gridTex;
// 	AddVertsForSphere3D(m_sphere->m_vertexs, m_sphere->m_position, 1.f, Rgba8::WHITE);
// 
// 	AddEntityToList(*m_cube, m_allEntities);
// 	AddEntityToList(*m_cube2, m_allEntities);
// 	AddEntityToList(*m_sphere, m_allEntities);
// 
// 	AddVertsForGroundGrid();
// 	AddVertsForCubes();

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
	std::string skyboxPaths[] = {
 	"Data/Images/SkyBox1/skyhsky_lf.png",
 	"Data/Images/SkyBox1/skyhsky_rt.png",
 	"Data/Images/SkyBox1/skyhsky_dn.png",
 	"Data/Images/SkyBox1/skyhsky_up.png",
 	"Data/Images/SkyBox1/skyhsky_ft.png",
 	"Data/Images/SkyBox1/skyhsky_bk.png",
	};

	std::string skyBoxShaderPath = "Data/Shaders/CubeSkyBox";
	m_cubeSkybox = new CubeSkyBox(g_theRenderer, skyboxPaths,&skyBoxShaderPath);

	//------------Chess Game---------------------------------------------
	StartTheMatch();

}

Game::~Game()
{

// 	delete m_cube;
// 	m_cube = nullptr;
// 
// 	delete m_cube2;
// 	m_cube2 = nullptr;
// 
// 	delete m_player;
// 	m_player = nullptr;
// 
// 	delete m_sphere;
// 	m_sphere = nullptr;
// 
// 	delete m_groundGrid;
// 	m_groundGrid = nullptr;

	if (m_curMatch)
	{
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessMove", Command_ChessMove);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessBegin", Command_ChessBegin);
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

	//chess game
	m_curMatch->Renderer();

	g_theRenderer->EndCamera(m_player->m_playerCam);

	DebugRenderWorld(m_player->m_playerCam);

	g_theRenderer->BeginCamera(m_screenCamera);

	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	font->AddVertsForTextInBox2D(title, "Use the DevVonsole (~) to enter commands.", 
		AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));
	std::string tips;


	std::string cameraMode = (m_camMode == CamMode::FREE) ? "Camera Mode: FREE" : "Camera Mode: AUTO";

	const char* gameStateNames[] = {
		"WAIT TO START GAME",
		"FIRST PLAYER TURN",
		"SECOND PLAYER TURN",
		"FIRST PLAYER WIN",
		"SECOND PLAYER WIN"
	};
	std::string gameState = "Game State: " + std::string(gameStateNames[(int)m_chessGameState]);
	font->AddVertsForTextInBox2D(title, cameraMode,
		AABB2(Vec2(10.f, 745.f), Vec2(500.f, 765.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	font->AddVertsForTextInBox2D(title, gameState,
		AABB2(Vec2(10.f, 720.f), Vec2(700.f, 740.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);

	DebugRenderScreen(m_screenCamera);
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);

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

// void Game::AddVertsForGroundGrid()
// {
// 	// add verts for ground grid
// 	for (int i = 1; i <= 101; i++)
// 	{
// 		if (i % 5 == 1)
// 		{
// 			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.03f, -50.f, -0.03f);
// 			Vec3 yTr = yBl + Vec3(0.06f, 100.f, 0.06f);
// 			//AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(yBl, yTr), Rgba8::GREEN);
// 
// 			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.03f, -0.03f);
// 			Vec3 xTr = xBl + Vec3(100.f, 0.06f, 0.06f);
// 			//AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(xBl, xTr), Rgba8::RED);
// 		}
// 
// 		else
// 		{
// 			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.01f, -50.f, -0.01f);
// 			Vec3 yTr = yBl + Vec3(0.02f, 100.f, 0.02f);
// 			//AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(yBl, yTr), Rgba8(180, 180, 180, 255));
// 
// 			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.01f, -0.01f);
// 			Vec3 xTr = xBl + Vec3(100.f, 0.02f, 0.02f);
// 			//AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(xBl, xTr), Rgba8(180, 180, 180, 255));
// 		}
// 
// 	}
// }

// void Game::AddVertsForCubes()
// {
// 	//add cube vertexs to m_cube
// 	//X
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
// 	//-X
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
// 	//Y
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
// 	//-Y
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
// 	//Z
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
// 	//-Z
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);
// 
// 
// 	//X
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
// 	//-X
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
// 	//Y
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
// 	//-Y
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
// 	//Z
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
// 	//-Z
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);
// 
// 
// 
// }

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
	std::string fromStr = args.GetValue("from", "");
	std::string toStr = args.GetValue("to", "");

	ChessMoveResult moveResult = CheckMovement(fromStr, toStr);

	//----------------------------------------------------------

	if (IsValidateChessMoveResult(moveResult))
	{
		// move the chess
		g_theGame->m_curMatch->MoveTheChessPiece(fromStr, toStr);
		//update cam
		if (g_theGame->m_camMode == CamMode::AUTO|| 
			g_theGame->m_chessGameState==ChessGameState::FIRST_WIN||
			g_theGame->m_chessGameState == ChessGameState::SEC_WIN)
		{
			g_theGame->SetObservationCamPosition(g_theGame->m_chessGameState);
		}
		// print sate
		g_theGame->PrintBoardState();
	}
	else
	{
		return false;
	}
}

bool Game::Command_ChessBegin(EventArgs& args)
{
	UNUSED(args);
	g_theGame->EndTheMatch();
	g_theGame->StartTheMatch();
	return true;
}

ChessMoveResult Game::CheckMovement(std::string fromStr, std::string toStr)
{
	// string input validation
	if (fromStr.empty() || toStr.empty()
		|| fromStr.length() != 2 || toStr.length() != 2)
	{
		AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_BAD_LOCATION);
		return ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
	}

	char fromCol = fromStr[0];
	char toCol = toStr[0];
	if (fromCol < 'a' || fromCol > 'h' || toCol < 'a' || toCol > 'h')
	{
		AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_BAD_LOCATION);
		return ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
	}

	char fromRow = fromStr[1];
	char toRow = toStr[1];
	if (fromRow < '1' || fromRow > '8' || toRow < '1' || toRow > '8')
	{
		AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_BAD_LOCATION);
		return ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
	}

	// zero distance
	if (fromStr == toStr)
	{
		AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE);
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
		AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_NO_PIECE);
		return ChessMoveResult::INVALID_MOVE_NO_PIECE;
	}
	if (curFromPiece->GetFaction() != curFaction)
	{
		AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE);
		return ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
	}

	//if the to have piece belong to current player, it will be invalid
	int toIndex = g_theGame->m_curMatch->m_chessBoard.GetIndexFromCharNum(toStr);
	ChessPiece* curToPiece = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toIndex);
	if (curToPiece && curToPiece->GetFaction() == curFaction)
	{
		AddLineForMoveResult(ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED);
		return ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
	}

	//Chess Piece Rules
	ChessMoveResult moveResult;
	ValidatePieceMovement(fromIndex, toIndex, moveResult);
	AddLineForMoveResult(moveResult);
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
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;

	//Castle

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
	ChessPiece* movingKing = g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(moveChessIndex);
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
		out_result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
		return false;
	}

	return true;
}

bool Game::ValidatePawnMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result)
{
	IntVec2 moveGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(moveChessIndex);
	IntVec2 toGridPos = g_theGame->m_curMatch->m_chessBoard.GetGridPosFromIndex(toChessIndex);
	IntVec2 moveStep = toGridPos - moveGridPos;
	ChessPiece* movePiece=g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(moveChessIndex);

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
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
			}
			return true;
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
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
			}
			return true;
		}
	}

	//promotion
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
				out_result = ChessMoveResult::VALID_MOVE_PROMOTION;
			}
			else
			{
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
			}
			return true;
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
				out_result = ChessMoveResult::VALID_MOVE_PROMOTION;
			}
			else
			{
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
			}
		}
	}

	// diagonally catch
	if (movePiece->GetFaction() == Faction::WHITE)
	{
		if ((moveStep.x == 1|| moveStep.x == -1) && moveStep.y == 1)
		{
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				return false;
			}
			else
			{
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
				return true;
			}
			
		}
	}
	else if (movePiece->GetFaction() == Faction::BLACK)
	{
		if ((moveStep.x == 1 || moveStep.x == -1) && moveStep.y == -1)
		{
			if (g_theGame->m_curMatch->m_chessBoard.GetChessFromIndex(toChessIndex) == nullptr)
			{
				out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				return false;
			}
			else
			{
				out_result = ChessMoveResult::VALID_CAPTURE_NORMAL;
				return true;
			}

		}
	}


	out_result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
	return false;
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
		g_theDevConsole->AddLine(DevConsole::TIPS, "Castled kingside successfully.");
		break;
	case ChessMoveResult::VALID_CASTLE_QUEENSIDE:
		g_theDevConsole->AddLine(DevConsole::TIPS, "Castled queenside successfully.");
		break;
	case ChessMoveResult::VALID_CAPTURE_NORMAL:
		g_theDevConsole->AddLine(DevConsole::TIPS, "Piece captured successfully.");
		break;
	case ChessMoveResult::VALID_CAPTURE_ENPASSANT:
		g_theDevConsole->AddLine(DevConsole::TIPS, "En passant capture successful.");
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
		g_theDevConsole->AddLine(DevConsole::INVALID, "Invalid move: Cannot capture your own piece.");
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

void Game::StartTheMatch()
{
	if (!m_curMatch)
	{
		m_curMatch = new ChessMatch(this);
		g_theEventSystem->SubscribeEventCallbackFuction("ChessMove", Command_ChessMove, true);
		g_theEventSystem->SubscribeEventCallbackFuction("ChessBegin", Command_ChessBegin, true);
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
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessMove", Command_ChessMove);
		g_theEventSystem->UnsubscribeEventCallbackFunction("ChessBegin", Command_ChessBegin);
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














