#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Vec3.hpp"

class Clock;
class Prop;
class Player;
class Texture;
class Entity;
class CubeSkyBox;

class ChessMatch;
class ChessPiece;

typedef std::vector<Entity*> EntityList;

enum class CamMode
{
	FREE,
	AUTO
};

enum class ChessGameState
{
	WAIT_TO_START_GAME,
	FIRST_PLAYER_TURN,
	SEC_PLAYER_TURN,
	FIRST_WIN,
	SEC_WIN
};

enum class ChessMoveResult
{
	VALID_MOVE_NORMAL,					//d
	VALID_MOVE_PROMOTION,
	VALID_CASTLE_KINGSIDE,
	VALID_CASTLE_QUEENSIDE,
	VALID_CAPTURE_NORMAL,				//d
	VALID_CAPTURE_ENPASSANT, 

	INVALID_MOVE_BAD_LOCATION,			//d
	INVALID_MOVE_NO_PIECE,				//d
	INVALID_MOVE_NOT_YOUR_PIECE,		//d
	INVALID_MOVE_ZERO_DISTANCE,			//d
	INVALID_MOVE_WRONG_MOVE_SHAPE,		//d
	INVALID_MOVE_DESTINATION_BLOCKED,	//d
	INVALID_MOVE_PATH_BLOCKED,			//d
	INVALID_MOVE_ENDS_IN_CHECK,
	INVALID_MOVE_KING_TOO_CLOSE,

	INVALID_ENPASSANT_STALE, //?
	INVALID_CASTLE_KING_HAS_MOVED,
	INVALID_CASTLE_ROOK_HAS_MOVED,
	INVALID_CASTLE_PATH_BLOCKED,
	INVALID_CASTLE_THROUGH_CHECK,
	INVALID_CASTLE_OUT_OF_CHECK,

	UNKNOWN
};

class Game
{
public:
	Game();
	~Game();

	void Update();
	void Renderer() const;

	void StartTheMatch();
	void EndTheMatch();

	void SetObservationCamPosition(ChessGameState chessGameState);

private:
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);
	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion();
	void RenderAttractMode() const;
	void RenderUI() const;
	void RenderDebugMode() const;

// 	void AddVertsForGroundGrid();
// 	void AddVertsForCubes();
	void AddEntityToList(Entity& thisEntity, EntityList& list);

	void PrintBoardState();

	static bool Command_ChessMove(EventArgs& args);
	static bool Command_ChessBegin(EventArgs& args);
	static ChessMoveResult CheckMovement(std::string fromStr, std::string toStr);
	static bool IsValidateChessMoveResult(ChessMoveResult result);

	static bool ValidatePieceMovement(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result);
	static bool ValidateKingMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result);
	static bool ValidateQueenMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result);
	static bool ValidateRookMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result);
	static bool ValidateBishopMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result);
	static bool ValidateKnightMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result);
	static bool ValidatePawnMove(int moveChessIndex, int toChessIndex, ChessMoveResult& out_result);

	static void AddLineForMoveResult(ChessMoveResult result);

public:
	Clock* m_gameClock = nullptr;
	bool m_isAttractMode = true;
	bool m_isDevConsole = false;
	bool isDebugMode = false;
	RandomNumberGenerator m_rng;

	CamMode m_camMode = CamMode::AUTO;

	ChessGameState m_chessGameState = ChessGameState::WAIT_TO_START_GAME;

private:
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;


	EntityList  m_allEntities;
	Player* m_player = nullptr;
// 	Prop* m_cube = nullptr;
// 	Prop* m_cube2 = nullptr;
// 	Prop* m_sphere = nullptr;
// 	Prop* m_groundGrid = nullptr;

	Texture* m_gridTex = nullptr;

	float m_previousTimeScale = 1.f;

	CubeSkyBox* m_cubeSkybox = nullptr;

	//---------------------Chess Game----------------------------------
	ChessMatch* m_curMatch;
	


	// Camera Position & Orientation Settings
	Vec3 m_whiteCamPosition = Vec3(4.f, -2.f, 4.f);
	Vec3 m_blackCamPosition = Vec3(4.f, 10.f, 4.f);
	Vec3 m_winCamPosition = Vec3(4.f, 4.f, 10.f);
	EulerAngles m_whiteOrientation = EulerAngles(90.f, 40.f, 0.f);
	EulerAngles m_blackOrientation = EulerAngles(-90.f, 40.f, 0.f);
	EulerAngles m_winOrientation = EulerAngles(-90.f, 90.f, 0.f);
};