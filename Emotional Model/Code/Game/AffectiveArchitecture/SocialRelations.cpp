#include "Game/AffectiveArchitecture/SocialRelation.hpp"
#include "Game/AffectiveArchitecture/SocialRole.hpp"

SocialRelation::SocialRelation()
	: m_liking(0)
	, m_dominance(0)
	, m_solidarity(0)
	, m_familiarity(0)
	, m_numSocialRoles(0)
{
}

SocialRelation::SocialRelation(SocialRole *socialRole)
	: m_numSocialRoles(0)
{
	m_liking = socialRole->m_mainSocialRelation->m_liking;
	m_dominance = socialRole->m_mainSocialRelation->m_dominance;
	m_solidarity = socialRole->m_mainSocialRelation->m_solidarity;
	m_familiarity = socialRole->m_mainSocialRelation->m_familiarity;
}

SocialRelation::SocialRelation(float newLiking, float newDominance, float newFamiliarity, float newSolidarity)
	: m_liking(newLiking)
	, m_dominance(newDominance)
	, m_solidarity(newSolidarity)
	, m_familiarity(newFamiliarity)
	, m_numSocialRoles(0)
{
}

SocialRelation::SocialRelation(SocialRelation *socialRelation)
	: m_liking(socialRelation->m_liking)
	, m_dominance(socialRelation->m_dominance)
	, m_solidarity(socialRelation->m_solidarity)
	, m_familiarity(socialRelation->m_familiarity)
	, m_numSocialRoles(1)
{
}

SocialRelation::~SocialRelation()
{

}