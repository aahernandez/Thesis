#include "Game/AffectiveArchitecture/Event.hpp"
#include "Game/Character.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtilities.hpp"

Event::Event(Event* copyEvent)
{
	m_agent = copyEvent->m_agent;
	m_action = copyEvent->m_action;
	m_patient = copyEvent->m_patient;
}

Event::Event(Entity* agent, Action* action, Entity* patient)
{
	m_agent = agent;
	m_action = action;
	m_patient = patient;
}

Event::~Event()
{
	m_agent = nullptr;
	m_action = nullptr;
	m_patient = nullptr;
}

void Event::AddEvent(Entity* entityPerceiving, Event* newEvent, float certaintyDegree)
{
	Event *copyEvent = new Event(newEvent);
	std::pair<Event*,float> eventPair = std::make_pair(copyEvent, certaintyDegree);
	entityPerceiving->m_perceivedEvents.push_back(eventPair);
}

void Event::AddEvent(Entity *entityPerceiving, Entity *agent, Action *action, Entity *patient, float certaintyDegree)
{
	Event *newEvent = new Event(agent, action, patient);
	entityPerceiving->m_perceivedEvents.push_back(std::make_pair(newEvent, certaintyDegree));
}