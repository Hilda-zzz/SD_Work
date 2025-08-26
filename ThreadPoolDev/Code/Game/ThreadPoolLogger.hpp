#pragma once
#include <string>
#include <mutex>

enum class ThreadPoolLogLevel 
{
	NONE = 0,
	ERROR = 1,
	WARN = 2,
	INFO = 3,
	DEBUG = 4
};

class ThreadPoolLogger
{
public:
	ThreadPoolLogger(ThreadPoolLogLevel level = ThreadPoolLogLevel::INFO, bool consoleOutput = true,
		std::string const& logFile = "");
	
	void Log(ThreadPoolLogLevel msgLevel, std::string const& msg);
	void SetLevel(ThreadPoolLogLevel newLevel);
	void SetLogFile(std::string const& filename);
	void SetConsoleOutput(bool enable);

private:
	ThreadPoolLogLevel m_level;
	bool m_consoleOutput = false;
	std::string m_logFile;
	std::mutex m_mutex;
};