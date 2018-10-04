#include "Game/AffectiveArchitecture/SocialRole.hpp"
#include "Game/AffectiveArchitecture/SocialRelation.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"
#include "Engine/Core/FileUtils.hpp"

std::map<std::string, SocialRole*> SocialRole::s_socialRoleRegistry;

SocialRole::SocialRole()
{

}

SocialRole::~SocialRole()
{
	delete m_mainSocialRelation;
}

void SocialRole::LoadSocialRolesFromXML()
{
	IXMLDomParser iDom;
	iDom.setRemoveClears(false);
	ITCXMLNode xMainNode = iDom.openFileHelper("Data/XML/SocialRoles.xml", "SocialRoles");
	ValidateXmlElement(xMainNode, "SocialRole", "");

	for (int nodeCount = 0; nodeCount < xMainNode.nChildNode(); nodeCount++)
	{
		ITCXMLNode childNode = xMainNode.getChildNode(nodeCount);
		ValidateXmlElement(childNode, "DefaultRelations,OppositeRelations", "Name,RoleMain,RoleSub");
		RegisterNewSocialRoles(childNode);
	}
}

void SocialRole::RegisterNewSocialRoles(const ITCXMLNode& xmlNode)
{
	std::string name = ParseXmlAttribute(xmlNode, "Name", (std::string)"ERROR_NO_NAME_FOUND");
	std::string nameRoleMain = ParseXmlAttribute(xmlNode, "RoleMain", (std::string)"ERROR_NO_NAME_FOUND");
	std::string nameRoleSub = ParseXmlAttribute(xmlNode, "RoleSub", (std::string)"ERROR_NO_NAME_FOUND");

	SocialRole *newRole = new SocialRole();
	newRole->m_namePrimary = nameRoleMain;
	newRole->m_nameSecondary = nameRoleSub;
	newRole->m_nameGroup = name;

	ITCXMLNode defaultNode = xmlNode.getChildNode("DefaultRelations");
	ValidateXmlElement(defaultNode, "", "liking,dominance,solidarity,familiarity");
	SocialRelation *newRelation = new SocialRelation();
	newRelation->m_liking = ParseXmlAttribute(defaultNode, "liking", 0.f);
	newRelation->m_dominance = ParseXmlAttribute(defaultNode, "dominance", 0.f);
	newRelation->m_solidarity = ParseXmlAttribute(defaultNode, "solidarity", 0.f);
	newRelation->m_familiarity = ParseXmlAttribute(defaultNode, "familiarity", 0.f);
	newRole->m_mainSocialRelation = newRelation;

	s_socialRoleRegistry[newRole->m_nameGroup] = newRole;

	int numOpposites = xmlNode.nChildNode("OppositeRelations");
	if (numOpposites == 1)
	{
		ITCXMLNode oppositeNode = xmlNode.getChildNode("OppositeRelations");
		ValidateXmlElement(oppositeNode, "", "liking,dominance,solidarity,familiarity");
		SocialRelation *oppositeRelation = new SocialRelation();
		oppositeRelation->m_liking = ParseXmlAttribute(oppositeNode, "liking", 0.f);
		oppositeRelation->m_dominance = ParseXmlAttribute(oppositeNode, "dominance", 0.f);
		oppositeRelation->m_solidarity = ParseXmlAttribute(oppositeNode, "solidarity", 0.f);
		oppositeRelation->m_familiarity = ParseXmlAttribute(oppositeNode, "familiarity", 0.f);
		newRole->m_subSocialRelation = oppositeRelation;
	}
	else
	{
		newRole->m_subSocialRelation = newRole->m_mainSocialRelation;
	}
}

void SocialRole::DestroyRegistry()
{
	std::map<std::string, SocialRole*>::iterator socialRoleIter;

	for (socialRoleIter = s_socialRoleRegistry.begin(); socialRoleIter != s_socialRoleRegistry.end(); ++socialRoleIter)
	{
		delete socialRoleIter->second;
		socialRoleIter->second = nullptr;
	}
	s_socialRoleRegistry.clear();
}

SocialRole* SocialRole::GetRandomSocialRole()
{
	int randomSocialRoleIndex = GetRandomIntInRange(0, (int)s_socialRoleRegistry.size() - 1);
	std::map<std::string, SocialRole*>::iterator socialRoleIter = s_socialRoleRegistry.begin();
	std::advance(socialRoleIter, randomSocialRoleIndex);

	return (*socialRoleIter).second;
}

