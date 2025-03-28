#pragma once

enum class UpperBodyState
{
	IDLE,
	WALK,
	RUN,

	WEAPON_IDLE,
	WEAPON_FIRE,
	WEAPON_RELOAD,

	ROLL,
	DODGE,
	MELEE_ATTACK,

	HIT_REACTION,
	DEATH
};

enum class LegState
{
	IDLE,
	WALK,
	RUN,
	CROUCH,
	HIDDEN
};