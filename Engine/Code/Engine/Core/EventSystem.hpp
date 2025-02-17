#pragma once
//#include "Engine/Core/NamedStrings.hpp"
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include <string>

struct EventSystemConfig
{
	int flag = 1;
};

//-------------------------------------------------------------------
class NamedStrings;
typedef NamedStrings EventArgs;
typedef bool(*EventCallbackFunction)(EventArgs& args);

struct EventSubscription
{
	EventCallbackFunction m_callbackFunction;
	//std::vector<EventArgs> m_eventArgs;
	bool m_isCommand;
	bool operator==(const EventSubscription& other) const {
		return m_callbackFunction == other.m_callbackFunction;
	}
};

typedef std::vector<EventSubscription> SubscriptionList;

struct cmpCaseInsensitive
{
	bool operator() (const std::string& a, const std::string& b) const
	{
		std::string lowerCaseA = a;
		std::string lowerCaseB = b;
		std::transform(lowerCaseA.begin(), lowerCaseA.end(), lowerCaseA.begin(),
			[](unsigned char c) { return (unsigned char)std::tolower(c); });
		std::transform(lowerCaseB.begin(), lowerCaseB.end(), lowerCaseB.begin(),
			[](unsigned char c) { return (unsigned char)std::tolower(c); });

		return lowerCaseA < lowerCaseB;
	}
};

//-------------------------------------------------------------------
class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config);
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeEventCallbackFuction(std::string const& eventName, EventCallbackFunction functionPtr, bool isCommand = false);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void FireEvent(std::string const& eventName, EventArgs& args);
	void FireEvent(std::string const& eventName);
	const std::map<std::string, SubscriptionList, cmpCaseInsensitive>& GetAllEventsSubscriptionLists() const;
	const std::vector<std::string>& GetCommandList() const;

protected:
	EventSystemConfig		m_config;
	std::map<std::string, SubscriptionList,cmpCaseInsensitive> m_subscriptionListsByEventName;
	std::vector<std::string> m_commandList;
};

//-----------------------------------------------------------------------------
// Standalone global-namespace helper functions; these forward to the event system, if it exists
//
//void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
//void UnsubsribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
//void FireEvent(std::string const& eventName, EventArgs& args);
//void FireEvent(std::string const& eventName);
