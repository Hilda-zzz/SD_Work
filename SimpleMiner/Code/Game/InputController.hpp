#pragma once
class InputController 
{
public:
	virtual void HandleKeyboardInput(float deltaTime) = 0;
	virtual void HandleMouseInput(float deltaTime) = 0;
	virtual void HandleControllerInput(float deltaTime) = 0;
	virtual void Update(float deltaTime) = 0;
};