#pragma once
#include <vector>
#include <utility>

class Entity;
class Action;
class Event;

typedef std::pair<Event*,float> EventQuad;
typedef std::vector<EventQuad> EventVector;
typedef std::vector<EventQuad>::iterator EventIterator;

class Event
{
public:
	Entity* m_agent;
	Action* m_action;
	Entity* m_patient;

	Event(Event* copyEvent);
	Event(Entity *agent, Action *action, Entity *patient);
	~Event();

	static void AddEvent(Entity* entityPerceiving, Event* newEvent, float certaintyDegree);
	static void AddEvent(Entity* entityPerceiving, Entity* agent, Action* action, Entity* patient, float certaintyDegree);
};