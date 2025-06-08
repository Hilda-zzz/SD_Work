#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include "Engine/Core/EventSystem.hpp"
class BitmapFont;
class Timer;
//class EventArgs;

struct DevConsoleConfig
{
	DevConsoleConfig(std::string const& fontName, float fontAspect,float overFullCount) 
		:m_fontName(fontName), m_fontAspect(fontAspect),m_overFullCount(overFullCount)
	{

	}
	std::string m_fontName;
	float m_fontAspect;
	float m_overFullCount;

	Renderer* m_renderer = nullptr;
	Camera* m_camera = nullptr;
	int m_maxCommandHistory = 128;
	bool m_startOpen = false;
};

enum DevConsoleMode
{
	//#TODO now just use hidden as the default mode, open full when opening it
	OPEN_FULL,
	OPEN_PARTIAL,
	COMMAND_PROMPT_ONLY,
	HIDDEN,
	NUM,
};

struct DevConsoleLine
{
	DevConsoleLine(Rgba8 color,std::string const& text,int frameNum,double time):m_color(color),m_text(text),m_frameNumberPrinted(frameNum),m_timePrinted(time)
	{}
	Rgba8 m_color;
	std::string m_text;
	int m_frameNumberPrinted;
	double m_timePrinted;
};

class DevConsole
{
public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute(std::string const& consoleCommandText,bool echoCommand=true);
	void AddLine(Rgba8 const& color, std::string const& text);
	void Render(AABB2 const& bounds, Renderer* rendererOverride = nullptr) const;

	DevConsoleMode GetMode() const;
	void SetMode(DevConsoleMode mode);
	void ToggleOpen(DevConsoleMode mode);
	bool IsOpen();
	
	//------------------------------------------------------------------------------------------------
	static bool Command_Test(EventArgs& args);

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_CharInput(EventArgs& args);
	static bool Command_Clear(EventArgs& args);
	static bool Command_Help(EventArgs& args);
	static bool Command_Quit(EventArgs& args);
	//------------------------------------------------------------------------------------------------

	static const	Rgba8 POINTER_LIGHT;
	static const	Rgba8 POINTER_DARK;
	static const	Rgba8 TIPS;
	static const	Rgba8 HISTORY;
	static const	Rgba8 HELPLIST;
	static const	Rgba8 INPUT;
	static const	Rgba8 UNKNOWN;
	static const	Rgba8 INVALID;
	static const	Rgba8 EVENT_FEEDBACK;
	static const	Rgba8 BKG;
	static const	Rgba8 PLAYER_TIP;

protected:
	void Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect = 1.f) const;
protected:
	BitmapFont*				m_font;
	DevConsoleConfig		m_config;
	DevConsoleMode			m_mode = DevConsoleMode::HIDDEN;
	bool					m_open = false;
	std::vector<DevConsoleLine> m_lines; //#ToDo: support a max limited # of line (e.g. fixed circular buffer)
	static int				m_frameNumber;

	std::string				m_inputText;
	std::string				m_oriInputSave;
	int						m_insertionPointPosition = 0;
	bool					m_insesrtionPointVisible = true;
	Timer*					m_insertionPointBlinkTimer;
	std::vector<std::string> m_commandHistory;
	int						m_historyIndex = -1;
	
};