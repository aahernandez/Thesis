#pragma once
#include <string>
#include <map>

class ITCXMLNode;
class SocialRelation;

class SocialRole
{
public:
	std::string m_namePrimary;
	std::string m_nameSecondary;
	std::string m_nameGroup;
	SocialRelation *m_mainSocialRelation;
	SocialRelation *m_subSocialRelation;

	SocialRole();
	~SocialRole();

	static std::map<std::string, SocialRole*> s_socialRoleRegistry;

	static void LoadSocialRolesFromXML();
	static void RegisterNewSocialRoles(const ITCXMLNode& xmlNode);
	static void DestroyRegistry();
	static SocialRole* GetRandomSocialRole();
};