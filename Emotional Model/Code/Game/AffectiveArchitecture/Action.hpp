#pragma once

#include <string>
#include <map>

class ITCXMLNode;
class Action;

typedef std::map<std::string, Action*> ActionMap;
typedef ActionMap::iterator ActionIterator;

class Action
{
public:
	std::string m_name;
	float m_effect;

	Action();
	~Action();

	static ActionMap s_actionRegistry;
	static void LoadActionsFromXML();
	static void RegisterNewType(const ITCXMLNode& xmlNode);
	static void DestroyRegistry();
	static Action* GetRandomAction();
};