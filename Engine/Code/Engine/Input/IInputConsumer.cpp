#include "IInputConsumer.hpp"

IInputConsumer::IInputConsumer(int inputPriority) : m_inputPriority(inputPriority)
{

}

int IInputConsumer::GetPriority() const
{
	return m_inputPriority;
}
