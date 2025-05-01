#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/ZCylinder.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include"Engine/Core/Vertex_PCUTBN.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Core/Timer.hpp"

class Shader;
class Texture;
class VertexBuffer;
class IndexBuffer;
class Map;
struct ActorHandle;
class Weapon;
class Controller;
class AIController;
class PlayerController;

enum class AnimState
{
	IDLE,       
	PLAYING,   
	FINISHED   
};

class Actor
{
public:
	Actor(int actorDefIndex,Map* curMap,Vec3 const& positon, EulerAngles const& orientaion,bool isProjectile=false);
	~Actor();

	void InitializeFromDef();

	virtual void Update(float deltaSeconds);
	void UpdateAnimation(PlayerController* curPlayer);
	void UpdatePhysics(float deltaSeconds);

	void AddForce(Vec3 const& force);
	void AddImpulse(Vec3 const& impulse);
	void OnCollideOtherActor(ActorHandle* source);
	void OnCollideWorld();
	void OnCollideRay(ActorHandle* source,Vec3 const& rayDirection);

	void MoveInDirection(Vec3 const& direction, float speed);
	void TurnInDirection(Vec3 const& direction,float maxDeg);

	virtual void Render(PlayerController* curPlayer) const;
	Mat44 GetModelMat() const;

	void OnPossessed(Controller* newController);
	void OnUnpossessed();

	void Damage(float damage,ActorHandle const& sourceHandle,PlayerController* curPlayerController);
	void Die(PlayerController* curPlayerController);
	bool Attack(Vec3 const& dir);

	PlayerController* GetListener();


	void EquipWeapon(int weaponIndex);

	//Animation
	bool PlayAnimation(std::string const& animName);
	//bool PlayWeaponAnimation(std::string const& animName);

private:
	

public:
	Map* m_map = nullptr;
	ActorDefinition* m_actorDef;
	bool m_isProjectile = false;

	ActorHandle* m_actorHandle = nullptr;
	Controller* m_controller = nullptr;
	AIController* m_aiController = nullptr;
	
	Timer m_deadTimer;
	bool m_isDead = false;
	bool m_isDestroy = false;

	Faction m_faction = Faction::NEUTRAL;
	Vec3 m_position = Vec3(0.f, 0.f, 0.f);
	EulerAngles m_orientation = EulerAngles(0.f, 0.f, 0.f);
	Vec3 m_weaponDirection = Vec3(0.f, 0.f, 0.f);
	Vec3 m_velocity = Vec3(0.f, 0.f, 0.f);
	Vec3 m_acceleration = Vec3(0.f, 0.f, 0.f);
	float m_curHealth = 1.f;

	ActorHandle* m_owner = nullptr;
	Weapon* m_weapon = nullptr;
	std::vector<Weapon*> m_inventory;
	int m_curWeaponIndex = -1;

	bool m_isVisible = true;

	Rgba8 m_color = Rgba8::WHITE;
	bool m_isStatic = false;
	ZCylinder m_physicCollider = ZCylinder(Vec3(0.f, 0.f, 0.f), 1.f, 1.f);

	std::vector<Vertex_PCU>	m_vertexs;
	std::vector<Vertex_PCU>	m_vertexsWire;
	std::vector<Vertex_PCU>	m_vertexsSprite;
	std::vector<Vertex_PCUTBN> m_vertexsSpriteNorm;
	//std::vector<unsigned int>	m_indexs;
	Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	
	Clock* m_animClock = nullptr;
	Timer m_animTimer;
	//Timer m_weaponAnimTimer;
	int m_curAnimGroupIndex =0;
	int m_nextAnimGroupIndex = 0;
	AnimState m_animState = AnimState::IDLE;



	//VertexBuffer* m_vertexBuffer = nullptr;
	//IndexBuffer* m_indexBuffer = nullptr;

	//------------------------------------------------------------
};