#pragma once
#include "Engine/Core/NamedStrings.hpp"
#define UNUSED(x) (void)(x)

class DevConsole;
class EventSystem;

extern NamedStrings g_gameConfigBlackboard;
extern DevConsole*  g_theDevConsole;
extern EventSystem* g_theEventSystem;