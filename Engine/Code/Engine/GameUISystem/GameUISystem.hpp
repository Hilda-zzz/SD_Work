#pragma once
#include "../Input/IInputConsumer.hpp"

class GameUISystem: public IInputConsumer
{
public:
	GameUISystem(int inputPriority);
	~GameUISystem();

	// New Input System
	bool ConsumeInput(unsigned char keyCode) override;

private:

};