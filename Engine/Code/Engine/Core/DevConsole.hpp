#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include "Engine/Core/EventSystem.hpp"
class BitmapFont;
//class EventArgs;

struct DevConsoleConfig
{
	DevConsoleConfig(BitmapFont* font, float fontAspect,float overFullCount) :m_font(font), m_fontAspect(fontAspect),m_overFullCount(overFullCount)
	{

	}
	BitmapFont* m_font;
	float m_fontAspect;
	float m_overFullCount;
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

	void Execute(std::string const& consoleCommandText);
	void AddLine(Rgba8 const& color, std::string const& text);
	void Render(AABB2 const& bounds, Renderer* rendererOverride = nullptr) const;

	DevConsoleMode GetMode() const;
	void SetMode(DevConsoleMode mode);
	void ToggleMode(DevConsoleMode mode);
	
	//------------------------------------------------------------------------------------------------
	static bool Command_Test(EventArgs& args);
	//------------------------------------------------------------------------------------------------

	static const Rgba8 ERROR;
	static const Rgba8 WARNING;
	static const Rgba8 INFO_MAJOR;
	static const Rgba8 INFO_MINOR;
protected:
	void Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect = 1.f) const;
protected:
	DevConsoleConfig		m_config;
	DevConsoleMode			m_mode = DevConsoleMode::HIDDEN;
	std::vector<DevConsoleLine> m_lines; //#ToDo: support a max limited # of line (e.g. fixed circular buffer)
	static int m_frameNumber;
	
};