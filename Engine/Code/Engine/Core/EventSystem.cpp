#include "EventSystem.hpp"
#include "ErrorWarningAssert.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/DevConsole.hpp"
extern DevConsole* g_theDevConsole;
EventSystem* g_theEventSystem = nullptr;

EventSystem::EventSystem(EventSystemConfig const& config):m_config(config)
{
	//m_subscriptionListsByEventName.;
}

EventSystem::~EventSystem()
{
}

void EventSystem::Startup()
{
	m_commandList.reserve(20);
}

void EventSystem::Shutdown()
{
}

void EventSystem::BeginFrame()
{
}

void EventSystem::EndFrame()
{
}

void EventSystem::SubscribeEventCallbackFuction(std::string const& eventName, EventCallbackFunction functionPtr,bool isCommand)
{
	//automatically add new event in the map if cannot find the key string
	SubscriptionList& list = m_subscriptionListsByEventName[eventName]; 
	EventSubscription sub;
	sub.m_callbackFunction = functionPtr;
	//sub.m_eventArgs = argsList;
	list.push_back(sub);

	if (isCommand)
	{
		m_commandList.push_back(eventName);
	}
}

void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	auto it = m_subscriptionListsByEventName.find(eventName);
	if (it != m_subscriptionListsByEventName.end()) 
	{
		SubscriptionList& list = it->second;
		EventSubscription sub;
		sub.m_callbackFunction = functionPtr;
		auto removeIt = std::remove(list.begin(), list.end(), sub);
		if (removeIt != list.end())
		{
			list.erase(removeIt, list.end());
		}
		else 
		{
			DebuggerPrintf("Callback function not found for event: ");
			DebuggerPrintf(eventName.c_str());
		}
	}
	else 
	{
		DebuggerPrintf("Event not found: ");
		DebuggerPrintf(eventName.c_str());
	}
}

void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	auto it = m_subscriptionListsByEventName.find(eventName);
	if (it != m_subscriptionListsByEventName.end())
	{
		SubscriptionList& list = it->second;
		for (EventSubscription& callback : list)
		{
			//g_theDevConsole->AddLine(Rgba8::GREEN, "#FireEvent# " + eventName);
			bool isConsume = callback.m_callbackFunction(args);
			if (isConsume)
			{
				break;
			}
		}
	}
	else
	{
		if (g_theDevConsole)
		{
			g_theDevConsole->AddLine(DevConsole::UNKNOWN, "#Unknown Command# " + eventName);
		}
		DebuggerPrintf("No subscribers for event: ");
		DebuggerPrintf(eventName.c_str());
	}
}

void EventSystem::FireEvent(std::string const& eventName)
{
	auto it = m_subscriptionListsByEventName.find(eventName);
	if (it != m_subscriptionListsByEventName.end()) 
	{
		for (EventSubscription& callback : it->second) 
		{
			EventArgs args;
			args.SetValue("", "");
			bool isConsume = callback.m_callbackFunction(args);
			if (isConsume)
			{
				break;
			}
		}
	}
	else 
	{
		g_theDevConsole->AddLine(DevConsole::UNKNOWN, "#Unknown Command# " + eventName);
		DebuggerPrintf("No subscribers for event: ");
		DebuggerPrintf(eventName.c_str());
	}
}

const std::map<std::string, SubscriptionList, cmpCaseInsensitive>& EventSystem::GetAllEventsSubscriptionLists() const
{
	return m_subscriptionListsByEventName;
}

const std::vector<std::string>& EventSystem::GetCommandList() const
{
	return m_commandList;
}


