#pragma once
#include <vector>
#include <unordered_set>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "AnimStateEnum.hpp"
#include "StateMachine.hpp"

class Game;

enum class PlayerTools
{
	NONE,       // 没有工具/空手
	AXE,        // 斧头
	HOE,        // 锄头
	PICKAXE,    // 镐子
	SHOVEL,     // 铲子
	SICKLE,     // 镰刀
	WATER,       // 水（浇水工具）
	NUM
};

class Player
{
public:
	Player(Game* game);
	~Player();

	void Update(float deltaSeconds);
	void Render() const;

private:
	void Initialize();
	void InitializeAnims();
	void InitializeStateAnimations(const std::string& stateName,
		Texture* texture,
		const IntVec2& gridSize,
		PlayerBodyStates stateEnum,
		std::map<Direction, SpriteAnimDefinition*>& directionAnimsMap,
		AnimState* state, SpriteAnimPlaybackType playbackType, int framePerSecond);
	void HandleInput();
	void UpdateMovement(float deltaTime);
	void UpdateAnimations(float deltaTime);
	void UpdateToolAimGridPos();
	void UpdateToolUsingResult();
	IntVec2 GetCurrentCursorGridPos();
	IntVec2 GetCurDirectionIntVec2();
	Direction GetDirectionFromIntVec2(IntVec2 const& directionVec);

public:
	Vec2	m_position = Vec2(10.f,-10.f);
	Vec2 m_inputDirection = Vec2::ZERO; 
	Direction m_curDirection = Direction::DOWN;
	float	m_orientation = 0.f;
	Vec2	m_velocity = Vec2(0.f, 0.f);
	float   m_speed = 3.f;
	float	m_physicsRadius = 0.f;
	// Tools
	PlayerTools m_curTool = PlayerTools::NONE;

private:
	Game* m_game = nullptr;
	float	m_cosmeticRadius = 0.f;
	std::vector<Vertex_PCU> m_verts;
	
	// Animation State Machine
	AABB2 m_bodyBox;
	Texture* m_curBodyTex = nullptr;

	Texture* m_idleTex = nullptr;
	Texture* m_walkTex = nullptr;
	Texture* m_runTex = nullptr;
	Texture* m_axeTex = nullptr;
	Texture* m_hoeTex = nullptr;
	Texture* m_pickaxeTex = nullptr;
	Texture* m_shovelTex = nullptr;
	Texture* m_sickleTex = nullptr;
	Texture* m_waterTex = nullptr;

	StateMachine<PlayerBodyStates>	      m_bodyStateMachine;
	std::unordered_map<std::string, bool> m_animConditions;

	std::unordered_map<std::string, SpriteSheet*> m_spriteSheets;
	std::map<Direction, SpriteAnimDefinition*> m_idleDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_walkDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_runDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_axeDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_hoeDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_pickaxeDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_shovelDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_sickleDirectionalAnimDefs;
	std::map<Direction, SpriteAnimDefinition*> m_waterDirectionalAnimDefs;
	//std::unordered_map<std::string, SpriteAnimDefinition*> m_bodySpriteAnimDefs;

	std::vector<Vertex_PCU> m_hoverGridCursorSquareVerts;
	IntVec2 m_curCursorHoverGridPos;
	IntVec2 m_curToolAimGridPos;
	IntVec2 m_curToolToPlayerDirection;

	std::unordered_set<std::string> m_toolStates = {
	"playerAxe",
	"playerHoe",
	"playerPickaxe",
	"playerShovel",
	"playerSickle",
	"playerWater"
	};

	std::string m_previousAnimStateName = "";
	
};