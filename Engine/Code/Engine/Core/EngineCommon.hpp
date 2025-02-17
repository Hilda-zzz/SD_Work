#pragma once
#include "Engine/Core/NamedStrings.hpp"
#define UNUSED(x) (void)(x)

class DevConsole;
class EventSystem;
class Window;

extern NamedStrings g_gameConfigBlackboard;
extern DevConsole*  g_theDevConsole;
extern EventSystem* g_theEventSystem;
extern Window* g_theWindow;