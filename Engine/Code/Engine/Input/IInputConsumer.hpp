#pragma once

class IInputConsumer
{
public:

	IInputConsumer(int inputPriority);
	virtual ~IInputConsumer() {};
	virtual bool ConsumeInput(unsigned char keyCode) = 0;
	virtual int GetPriority() const;

	int m_inputPriority;
};