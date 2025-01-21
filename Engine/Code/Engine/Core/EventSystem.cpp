#include "EventSystem.hpp"
#include "ErrorWarningAssert.hpp"
#include "Engine/Core/NamedStrings.hpp"

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

void EventSystem::SubscribeEventCallbackFuction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	//automatically add new event in the map if cannot find the key string
	SubscriptionList& list = m_subscriptionListsByEventName[eventName]; 
	EventSubscription sub;
	sub.m_callbackFunction = functionPtr;
	list.push_back(sub);
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
			bool isConsume = callback.m_callbackFunction(args);
			if (isConsume)
			{
				break;
			}
		}
	}
	else
	{
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
		DebuggerPrintf("No subscribers for event: ");
		DebuggerPrintf(eventName.c_str());
	}
}
