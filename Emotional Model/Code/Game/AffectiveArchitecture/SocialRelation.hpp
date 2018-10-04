#pragma once

class SocialRole;

class SocialRelation
{
public:
	float m_liking;
	float m_dominance;
	float m_familiarity;
	float m_solidarity;
	int m_numSocialRoles;

	SocialRelation();
	SocialRelation(SocialRole *socialRole);
	SocialRelation(SocialRelation *socialRole);
	SocialRelation(float newLiking, float newDominance, float newFamiliarity, float newSolidarity);
	~SocialRelation();
};