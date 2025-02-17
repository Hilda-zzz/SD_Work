#include "DevConsole.hpp"
#include "Time.hpp"
#include "EngineCommon.hpp"
#include "ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <Engine/Input/InputSystem.hpp>
#include "Engine/Core/Timer.hpp"
DevConsole* g_theDevConsole = nullptr;
int DevConsole::m_frameNumber = 0;
extern Renderer* g_theRenderer;

const Rgba8 DevConsole::POINTER_LIGHT = Rgba8(255, 255, 255);
const Rgba8 DevConsole::POINTER_DARK = Rgba8(255, 255, 255,0);
const Rgba8 DevConsole::TIPS = Rgba8(92,215,254);
const Rgba8 DevConsole::HISTORY = Rgba8(209,209,209);
const Rgba8 DevConsole::HELPLIST = Rgba8(79,198,251);
const Rgba8 DevConsole::INPUT = Rgba8(229,158,221);
const Rgba8 DevConsole::UNKNOWN = Rgba8(188,76,116);
const Rgba8 DevConsole::INVALID = Rgba8(209,187,55);
const Rgba8 DevConsole::EVENT_FEEDBACK = Rgba8(142,217,115);
const Rgba8 DevConsole::BKG = Rgba8(0, 0, 0,180);

DevConsole::DevConsole(DevConsoleConfig const& config):m_config(config)
{
	m_insertionPointBlinkTimer = new Timer(0.5f);
}

DevConsole::~DevConsole()
{
	delete m_insertionPointBlinkTimer;
	m_insertionPointBlinkTimer = nullptr;
}

void DevConsole::Startup()
{
	m_font = g_theRenderer->CreateOrGetBitmapFont(m_config.m_fontName.c_str());
	m_lines.reserve(1000);
	m_inputText.reserve(50);

	g_theEventSystem->SubscribeEventCallbackFuction("KeyPressed", DevConsole::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFuction("CharInput", DevConsole::Event_CharInput);
	g_theEventSystem->SubscribeEventCallbackFuction("Test", Command_Test,true);
	g_theEventSystem->SubscribeEventCallbackFuction("help", Command_Help,true);
	g_theEventSystem->SubscribeEventCallbackFuction("clear", Command_Clear,true);
	g_theEventSystem->SubscribeEventCallbackFuction("quit", Command_Quit,true);

	AddLine(DevConsole::TIPS , "Type help for Command List");
}

void DevConsole::Shutdown()
{
}

void DevConsole::BeginFrame()
{
	m_frameNumber++;
	if (m_insertionPointBlinkTimer->DecrementPeriodIfElapsed())
	{
		m_insesrtionPointVisible = !m_insesrtionPointVisible;
	}
}

void DevConsole::EndFrame()
{
}

void DevConsole::Execute(std::string const& consoleCommandText, bool echoCommand)
{
	if (echoCommand)
	{
		g_theDevConsole->AddLine(DevConsole::HISTORY, consoleCommandText);
	}
	//\n
	std::vector<std::string> eachLine;
	eachLine.reserve(5);
	eachLine = SplitStringOnDelimiter(consoleCommandText, '\n');
	int lineCount = (int)eachLine.size();
	for (int i = 0; i < lineCount; i++)
	{
		//" "
		std::vector<std::string> eventNameWithArgs;
		eventNameWithArgs.reserve(2);
		eventNameWithArgs = SplitStringOnDelimiter(eachLine[i], ' ');

		if ((int)eventNameWithArgs.size() == 0)
		{
			continue;
		}
		else if ((int)eventNameWithArgs.size() == 1)
		{
			g_theEventSystem->FireEvent(eventNameWithArgs[0]);
			continue;
		}
		//=
		std::vector<std::string> argsPair;
		argsPair.reserve(2);
		argsPair = SplitStringOnDelimiter(eventNameWithArgs[1], '=');
		if (argsPair.size() == 2)
		{
			EventArgs args;
			args.SetValue(argsPair[0], argsPair[1]);
			g_theEventSystem->FireEvent(eventNameWithArgs[0], args);
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::INVALID, "#Invalid Format# "+consoleCommandText);
		}
	}
}

void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	//----------------SplitLine-----------------------------------------
	std::vector<std::string> eachLine;
	eachLine = SplitStringOnDelimiter(text, '\n');
	int lineCount = (int)eachLine.size();

	//push in m_lines
	for (int i = 0; i < lineCount; i++)
	{
		m_lines.emplace_back(color, eachLine[i], m_frameNumber, GetCurrentTimeSeconds());
	}
}

void DevConsole::Render(AABB2 const& bounds, Renderer* rendererOverride) const
{
	if (m_mode == HIDDEN) return;
	if (m_mode == OPEN_FULL) Render_OpenFull(bounds,*rendererOverride,*m_font,m_config.m_fontAspect);
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;
	ToggleOpen(mode);
}

void DevConsole::ToggleOpen(DevConsoleMode mode)
{
	if (mode == OPEN_FULL || mode == OPEN_PARTIAL || mode == COMMAND_PROMPT_ONLY)
	{
		m_open = true;
		m_insertionPointBlinkTimer->Start();
	}
	else
	{
		m_open = false;
		m_insertionPointBlinkTimer->Stop();
	}
}

bool DevConsole::IsOpen()
{
	return m_open;
}

bool DevConsole::Command_Test(EventArgs& args)
{
	std::string logString = "Test command received\ntest the swap the line hahahahahaha\n say something \naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbaa";
	std::string frame = std::to_string(m_frameNumber);
	logString += "#" + frame;
	g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, logString); // where CONSOLE_INFO is an Rgba8 constant!
	args.DebugPrintContents();
	DebuggerPrintf("trigger test dev console event\n");
	return true; 
}

bool DevConsole::Event_KeyPressed(EventArgs& args)
{
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	if (g_theDevConsole->IsOpen()&&keyCode != KEYCODE_TILDE)
	{
		if (keyCode == KEYCODE_ESC)
		{
			if (g_theDevConsole->m_inputText.size() == 0)
			{
				g_theDevConsole->SetMode(HIDDEN);
			}
			else
			{
				g_theDevConsole->m_inputText.clear();
				g_theDevConsole->m_insertionPointPosition = 0;
			}
		}
		else if (keyCode == KEYCODE_ENTER)
		{
			if (g_theDevConsole->m_inputText.size() == 0)
			{
				g_theDevConsole->SetMode(HIDDEN);
			}
			else
			{
				g_theDevConsole->m_commandHistory.push_back(g_theDevConsole->m_inputText);
				g_theDevConsole->Execute(g_theDevConsole->m_inputText,true);
				g_theDevConsole->m_inputText.clear();
				g_theDevConsole->m_insertionPointPosition = 0;
				g_theDevConsole->m_historyIndex = -1;
			}
			
		}
		else if (keyCode == KEYCODE_LEFTARROW)
		{
			if (g_theDevConsole->m_insertionPointPosition > 0)
			{
				g_theDevConsole->m_insertionPointPosition--;
			}
		}
		else if (keyCode == KEYCODE_RIGHTARROW)
		{
			if (g_theDevConsole->m_insertionPointPosition < (int)g_theDevConsole->m_inputText.size())
			{
				g_theDevConsole->m_insertionPointPosition++;
			}
		}
		else if (keyCode == KEYCODE_HOME)
		{
			g_theDevConsole->m_insertionPointPosition = 0;
		}
		else if (keyCode == KEYCODE_END)
		{
			g_theDevConsole->m_insertionPointPosition =(int) g_theDevConsole->m_inputText.size();
		}
		else if (keyCode == KEYCODE_DELETE)
		{
			if (g_theDevConsole->m_insertionPointPosition < (int)g_theDevConsole->m_inputText.size())
			{
				g_theDevConsole->m_inputText.erase(g_theDevConsole->m_inputText.begin()+g_theDevConsole->m_insertionPointPosition);
			}
		}
		else if (keyCode == KEYCODE_BACKSPACE)
		{
			if (g_theDevConsole->m_insertionPointPosition > 0)
			{
				g_theDevConsole->m_inputText.erase(g_theDevConsole->m_inputText.begin() + g_theDevConsole->m_insertionPointPosition -1);
				g_theDevConsole->m_insertionPointPosition--;
			}
		}
		else if (keyCode == KEYCODE_UPARROW)
		{
			if (g_theDevConsole->m_commandHistory.size() > 0)
			{
				if (g_theDevConsole->m_historyIndex > 0 && g_theDevConsole->m_historyIndex < (int)g_theDevConsole->m_commandHistory.size())
				{
					g_theDevConsole->m_historyIndex--;
				}
				else if (g_theDevConsole->m_historyIndex == -1)
				{
					g_theDevConsole->m_oriInputSave = g_theDevConsole->m_inputText;
					g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size() - 1;
				}

				if (g_theDevConsole->m_historyIndex >= 0)
				{
					g_theDevConsole->m_inputText.clear();
					g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
					g_theDevConsole->m_insertionPointPosition = (int)g_theDevConsole->m_inputText.size();
				}
			}
		}
		else if (keyCode == KEYCODE_DOWNARROW)
		{
			if (g_theDevConsole->m_commandHistory.size() > 0)
			{
				if (g_theDevConsole->m_historyIndex >= 0 && g_theDevConsole->m_historyIndex < (int)g_theDevConsole->m_commandHistory.size())
				{
					g_theDevConsole->m_historyIndex++;
				}

				if (g_theDevConsole->m_historyIndex >= 0)
				{
					g_theDevConsole->m_inputText.clear();
					if (g_theDevConsole->m_historyIndex < (int)g_theDevConsole->m_commandHistory.size())
					{
						g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
					}
					else
					{
						g_theDevConsole->m_inputText = g_theDevConsole->m_oriInputSave;
						g_theDevConsole->m_historyIndex = -1;
					}
					g_theDevConsole->m_insertionPointPosition = (int)g_theDevConsole->m_inputText.size();

				}
			}
		}
		return true;
	}
	else
		return false;
}

bool DevConsole::Event_CharInput(EventArgs& args)
{
	char charInput = (char)args.GetValue("CharInput", '`');
	if (g_theDevConsole->IsOpen()&&charInput >= 32 && charInput <= 126 && charInput != '`' && charInput != '~')
	{
		g_theDevConsole->m_insesrtionPointVisible = true;
		g_theDevConsole->m_insertionPointBlinkTimer->Start();
		g_theDevConsole->m_inputText.insert(g_theDevConsole->m_inputText.begin() + g_theDevConsole->m_insertionPointPosition, charInput);
		g_theDevConsole->m_insertionPointPosition++;
	}

	return true;
}

bool DevConsole::Command_Clear(EventArgs& args)
{
	UNUSED(args);
	g_theDevConsole->m_lines.clear();
	return true;
}

bool DevConsole::Command_Help(EventArgs& args)
{
	UNUSED(args);
	const std::vector<std::string>& commandList= g_theEventSystem->GetCommandList();
	for (int i=0;i< (int)commandList.size();i++)
	{
		g_theDevConsole->AddLine(DevConsole::HELPLIST, commandList[i]);
	}
	return true;
}

bool DevConsole::Command_Quit(EventArgs& args)
{
	UNUSED(args);
	g_theEventSystem->FireEvent("CloseWindow");
	return true;
}

void DevConsole::Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect) const
{
	//bkg
	renderer.BindTexture(nullptr);
	Vertex_PCU bkgVertices[6];
	bkgVertices[0] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), DevConsole::BKG, Vec2(0.f, 0.f));
	bkgVertices[1] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.f), DevConsole::BKG, Vec2(0.f, 0.f));
	bkgVertices[2] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), DevConsole::BKG, Vec2(0.f, 0.f));
	bkgVertices[3] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), DevConsole::BKG, Vec2(0.f, 0.f));
	bkgVertices[4] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.f), DevConsole::BKG, Vec2(0.f, 0.f));
	bkgVertices[5] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), DevConsole::BKG, Vec2(0.f, 0.f));
	renderer.DrawVertexArray(6, bkgVertices);

	//cur input line
	std::vector<Vertex_PCU> boxTextVerts2;
	std::vector<Vertex_PCU> shadowTextVerts;
	boxTextVerts2.reserve(1000);
	float curBotLeftX = 20.f;
	float curBotLeftY = 0.f;
	float cellHeight = 800.f / m_config.m_overFullCount;

	AABB2 inputTextBox = AABB2(Vec2(curBotLeftX, curBotLeftY), Vec2(1550.f, curBotLeftY + cellHeight));
	float inputTextEachWidth = font.AddVertsForTextInBox2D(boxTextVerts2, m_inputText,
		inputTextBox, cellHeight, DevConsole::INPUT, fontAspect, Vec2(0.f, 0.f), OVERRUN);


	std::vector<Vertex_PCU> boxTextVerts;
	//std::vector<Vertex_PCU> shadowTextVerts;
	//history line
	curBotLeftY+=cellHeight;
	int count = 0;
	for (int i = (int)m_lines.size() - 1; i >= 0; i--)
	{
		count++;
		AABB2 testTextBox = AABB2(Vec2(curBotLeftX, curBotLeftY), Vec2(1550.f, curBotLeftY + cellHeight));
		font.AddVertsForTextInBox2D(boxTextVerts, m_lines[i].m_text,
			testTextBox, cellHeight, m_lines[i].m_color, fontAspect, Vec2(0.f, 0.f), OVERRUN);
		curBotLeftY += cellHeight;

		if (count > m_config.m_overFullCount + 1)
		{
			break;
		}
	}

	//render all text
	shadowTextVerts.reserve(boxTextVerts.size());
	renderer.BindTexture(&font.GetTexture());
	for (int i = 0; i < (int)boxTextVerts.size(); ++i)
	{
		shadowTextVerts.push_back(Vertex_PCU(boxTextVerts[i].m_position + Vec3(2.f, 0.f, 0.f), Rgba8::TRANSP_BLACK, boxTextVerts[i].m_uvTexCoords));
	}
	renderer.DrawVertexArray(shadowTextVerts);
	renderer.DrawVertexArray(boxTextVerts);
	renderer.DrawVertexArray(boxTextVerts2);

	//pointer
	Rgba8 curPointerColor;
	if (m_insesrtionPointVisible)
	{
		curPointerColor = DevConsole::POINTER_LIGHT;
	}
	else
	{
		curPointerColor = DevConsole::POINTER_DARK;
	}
	renderer.BindTexture(nullptr);
	float pointStartX = curBotLeftX +m_insertionPointPosition * inputTextEachWidth;
	float pointEndX = curBotLeftX + m_insertionPointPosition * inputTextEachWidth + 2.f;
	Vertex_PCU pointVertices[6];
	pointVertices[0] = Vertex_PCU(Vec3(pointStartX, 2.f, 0.f), curPointerColor, Vec2(0.f, 0.f));
	pointVertices[1] = Vertex_PCU(Vec3(pointStartX, cellHeight, 0.f), curPointerColor, Vec2(0.f, 0.f));
	pointVertices[2] = Vertex_PCU(Vec3(pointEndX, cellHeight, 0.f), curPointerColor, Vec2(0.f, 0.f));
	pointVertices[3] = Vertex_PCU(Vec3(pointStartX, 2.f, 0.f), curPointerColor, Vec2(0.f, 0.f));
	pointVertices[4] = Vertex_PCU(Vec3(pointEndX, 2.f, 0.f), curPointerColor, Vec2(0.f, 0.f));
	pointVertices[5] = Vertex_PCU(Vec3(pointEndX, cellHeight, 0.f), curPointerColor, Vec2(0.f, 0.f));
	renderer.DrawVertexArray(6, pointVertices);
}
