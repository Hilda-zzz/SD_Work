#pragma once
//#include "Engine/Core/NamedStrings.hpp"
#include <vector>
#include <string>
#include <map>
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
	bool operator==(const EventSubscription& other) const {
		return m_callbackFunction == other.m_callbackFunction;
	}
};

typedef std::vector<EventSubscription> SubscriptionList;

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

	void SubscribeEventCallbackFuction(std::string const& eventName, EventCallbackFunction functionPtr);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void FireEvent(std::string const& eventName, EventArgs& args);
	void FireEvent(std::string const& eventName);
protected:
	EventSystemConfig		m_config;
	std::map<std::string, SubscriptionList> m_subscriptionListsByEventName;
};

//-----------------------------------------------------------------------------
// Standalone global-namespace helper functions; these forward to the event system, if it exists
//
//void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
//void UnsubsribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
//void FireEvent(std::string const& eventName, EventArgs& args);
//void FireEvent(std::string const& eventName);
