#include "Game/AffectiveArchitecture/Action.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"
#include "Engine/Core/FileUtils.hpp"

ActionMap Action::s_actionRegistry;

Action::Action()
{
}

Action::~Action()
{
}

void Action::LoadActionsFromXML()
{
	IXMLDomParser iDom;
	iDom.setRemoveClears(false);
	ITCXMLNode xMainNode = iDom.openFileHelper("Data/XML/Actions.xml", "Actions");
	ValidateXmlElement(xMainNode, "Action", "");

	for (int nodeCount = 0; nodeCount < xMainNode.nChildNode(); nodeCount++)
	{
		ITCXMLNode childNode = xMainNode.getChildNode(nodeCount);
		ValidateXmlElement(childNode, "", "name,effect");
		RegisterNewType(childNode);
	}
}

void Action::RegisterNewType(const ITCXMLNode& xmlNode)
{
	std::string name = ParseXmlAttribute(xmlNode, "name", (std::string)"ERROR_NO_NAME_FOUND");
	s_actionRegistry[name] = new Action();
	s_actionRegistry[name]->m_name = name;
	s_actionRegistry[name]->m_effect = ParseXmlAttribute(xmlNode, "effect", 0.f);
}

void Action::DestroyRegistry()
{
	ActionIterator actionIter;

	for (actionIter = s_actionRegistry.begin(); actionIter != s_actionRegistry.end(); ++actionIter)
	{
		delete actionIter->second;
		actionIter->second = nullptr;
	}
	s_actionRegistry.clear();
}

Action* Action::GetRandomAction()
{
	int randomActionIndex = GetRandomIntInRange(0, (int)s_actionRegistry.size() - 1);
	ActionIterator actionIter = s_actionRegistry.begin();
	std::advance(actionIter, randomActionIndex);

	return (*actionIter).second;
}
