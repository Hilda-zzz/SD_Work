#include "GameUISystem.hpp"

GameUISystem::GameUISystem(int inputPriority):IInputConsumer(inputPriority)
{
}

GameUISystem::~GameUISystem()
{
}

bool GameUISystem::ConsumeInput(unsigned char keyCode)
{
	return false;
}

