#include "DevConsole.hpp"
#include "Time.hpp"
#include "EngineCommon.hpp"
#include "ErrorWarningAssert.hpp"

DevConsole* g_theDevConsole = nullptr;
int DevConsole::m_frameNumber = 0;
DevConsole::DevConsole(DevConsoleConfig const& config):m_config(config)
{
}

DevConsole::~DevConsole()
{
}

void DevConsole::Startup()
{
	m_lines.reserve(1000);
	g_theEventSystem->SubscribeEventCallbackFuction("Test", Command_Test);
}

void DevConsole::Shutdown()
{
}

void DevConsole::BeginFrame()
{
	m_frameNumber++;
}

void DevConsole::EndFrame()
{
}

void DevConsole::Execute(std::string const& consoleCommandText)
{
	//\n
	std::vector<std::string> eachLine;
	eachLine.reserve(5);
	eachLine = SplitStringOnDelimiter(consoleCommandText, '\n');
	int lineCount = (int)eachLine.size();
	for (int i = 0; i < lineCount; i++)
	{
		AddLine(Rgba8::GREEN, "#FireEvent# "+eachLine[i]);
		//" "
		std::vector<std::string> eventNameWithArgs;
		eventNameWithArgs.reserve(2);
		eventNameWithArgs = SplitStringOnDelimiter(eachLine[i], ' ');
		//=
		std::vector<std::string> argsPair;
		argsPair.reserve(2);
		argsPair = SplitStringOnDelimiter(eventNameWithArgs[1], '=');
		EventArgs args;
		args.SetValue(argsPair[0], argsPair[1]);
		g_theEventSystem->FireEvent(eventNameWithArgs[0], args);
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
		m_lines.push_back(DevConsoleLine(color, eachLine[i], m_frameNumber, GetCurrentTimeSeconds()));
	}
}

void DevConsole::Render(AABB2 const& bounds, Renderer* rendererOverride) const
{
	if (m_mode == HIDDEN) return;
	if (m_mode == OPEN_FULL) Render_OpenFull(bounds,*rendererOverride,*m_config.m_font,m_config.m_fontAspect);
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;
}

void DevConsole::ToggleMode(DevConsoleMode mode)
{
	m_mode = mode;
}

bool DevConsole::Command_Test(EventArgs& args)
{
	std::string logString = "Test command received\ntest the swap the line hahahahahaha\n say something \naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbaa";
	std::string frame = std::to_string(m_frameNumber);
	logString += "#" + frame;
	g_theDevConsole->AddLine(Rgba8::RED, logString); // where CONSOLE_INFO is an Rgba8 constant!
	args.DebugPrintContents();
	DebuggerPrintf("trigger test dev console event\n");
	return false; 
}

void DevConsole::Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect) const
{
	renderer.BindTexture(nullptr);
	Vertex_PCU vertices[6];
	vertices[0] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.f), Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f));
	vertices[3] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.f), Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f));
	renderer.DrawVertexArray(6, vertices);

	std::vector<Vertex_PCU> boxTextVerts;
	boxTextVerts.reserve(1000);
	float cellHeight = 800.f / m_config.m_overFullCount;
	float curBotLeftY = 0.f;
	int count = 0;
	for (int i = (int)m_lines.size() - 1; i >= 0; i--)
	{
		count++;
		AABB2 testTextBox = AABB2(Vec2(20.f, curBotLeftY), Vec2(1550.f, curBotLeftY + cellHeight));
		font.AddVertsForTextInBox2D(boxTextVerts, m_lines[i].m_text,
			testTextBox, cellHeight, m_lines[i].m_color, fontAspect, Vec2(0.f, 0.f), SHRINK_TO_FIT);
		curBotLeftY += cellHeight;

		if (count > m_config.m_overFullCount + 3)
		{
			break;
		}
	}
	renderer.BindTexture(&font.GetTexture());
	renderer.DrawVertexArray(boxTextVerts);
}
