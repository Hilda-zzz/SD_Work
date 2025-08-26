#include "ThreadPoolLogger.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <iosfwd>
#include <fstream>

ThreadPoolLogger::ThreadPoolLogger(ThreadPoolLogLevel level, bool consoleOutput,std::string  const& logFile)
	: m_level(level)
	, m_consoleOutput(consoleOutput)
	, m_logFile(logFile)
{
}

void ThreadPoolLogger::Log(ThreadPoolLogLevel msgLevel, std::string const& msg)
{
	if (m_level == ThreadPoolLogLevel::NONE || msgLevel > m_level) return;

	std::lock_guard<std::mutex> lock(m_mutex);

	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream timestamp;
	timestamp << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");

	std::string levelStr;
	switch (msgLevel) 
	{
	case ThreadPoolLogLevel::ERROR: 
		levelStr = "ERROR"; break;
	case ThreadPoolLogLevel::INFO:  
		levelStr = "INFO"; break;
	case ThreadPoolLogLevel::DEBUG: 
		levelStr = "DEBUG"; break;
	default: levelStr = "UNKNOWN";
	}

	std::stringstream logMsg;
	logMsg << "[" << timestamp.str() << "] [" << levelStr << "] " << msg;

	if (m_consoleOutput) {
		if (msgLevel == ThreadPoolLogLevel::ERROR) 
		{
			std::cerr << logMsg.str() << std::endl;
		}
		else {
			std::cout << logMsg.str() << std::endl;
		}
	}

	if (!m_logFile.empty()) 
	{
		try 
		{
			std::ofstream file(m_logFile, std::ios::app);
			if (file.is_open()) 
			{
				file << logMsg.str() << std::endl;
			}
		}
		catch (...) 
		{
			if (m_consoleOutput) 
			{
				std::cerr << "Cannot write in thread pool logs: " << m_logFile << std::endl;
			}
		}
	}
}

void ThreadPoolLogger::SetLevel(ThreadPoolLogLevel newLevel)
{
	std::lock_guard<std::mutex> lock(m_mutex); // not sure
	m_level = newLevel;
}

void ThreadPoolLogger::SetLogFile(std::string const& filename)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_logFile = filename;
}

void ThreadPoolLogger::SetConsoleOutput(bool enable)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_consoleOutput = enable;
}
