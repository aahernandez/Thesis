#include "Game/Game.hpp"
#include "Game/CharacterBuilder.hpp"
#include "Game/WanderBehavior.hpp"
#include "Game/AttackBehavior.hpp"
#include "Game/FleeBehavior.hpp"
#include "Game/ShowOffBehavior.hpp"
#include "Game/SulkBehavior.hpp"
#include "Game/FreakOutBehavior.hpp"
#include "Game/RecuperateBehavior.hpp"
#include "Game/MateBehavior.hpp"
#include "Game/HideBehavior.hpp"
#include "Game/CryBehavior.hpp"
#include "Game/SmileBehavior.hpp"
#include "Game/TantrumBehavior.hpp"
#include "Game/SighBehavior.hpp"
#include "Game/GloatBehavior.hpp"
#include "Game/ImitateBehavior.hpp"
#include "Game/BlameBehavior.hpp"
#include "Game/DaydreamBehavior.hpp"
#include "Game/FollowBehavior.hpp"
#include "Game/EatBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/AffectiveArchitecture/SocialRole.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/Behavior.hpp"

CharacterBuilder::CharacterBuilder(const ITCXMLNode& xmlNode)
{
	ValidateXmlElement(xmlNode, "Stats,Glyph,GlyphColor,Behavior,LootTable,Faction,ActionAttitudes", "name");
	m_name = ParseXmlAttribute(xmlNode, "name", (std::string)"ERROR_NO_NAME_FOUND");
	StoreXmlChildNodeAttributes(xmlNode, "Glyph", "glyph", m_glyphs);
	StoreXmlChildNodeAttributes(xmlNode, "GlyphColor", "color", m_glyphColors);
	m_lootTableName = ParseXmlAttribute(xmlNode.getChildNode("LootTable"), "table", (std::string)"ERROR_NO_LOOT_TABLE_FOUND");
	m_faction = ParseXmlAttribute(xmlNode.getChildNode("Faction"), "faction", (std::string)"ERROR_NO_FACTION_FOUND");
	m_stats = Stats(xmlNode.getChildNode("Stats"));
	ITCXMLNode behaviorNode = xmlNode.getChildNode("Behavior");
	LoadBehaviors(behaviorNode);
	ITCXMLNode actionAttitudeNode = xmlNode.getChildNode("ActionAttitudes");
	LoadActionAttitudes(actionAttitudeNode);
}

CharacterBuilder::~CharacterBuilder()
{

}

void CharacterBuilder::LoadCharactersFromXML()
{
	IXMLDomParser iDom;
	iDom.setRemoveClears(false);
	ITCXMLNode xMainNode = iDom.openFileHelper("Data/XML/Characters.xml", "Characters");
	ValidateXmlElement(xMainNode, "Character", "");

	for (int nodeCount = 0; nodeCount < xMainNode.nChildNode(); nodeCount++)
	{
		ITCXMLNode childNode = xMainNode.getChildNode(nodeCount);
		RegisterNewType(childNode);
	}
}

Character* CharacterBuilder::BuildCharacter(std::string name, Map* owningMap)
{
	std::string newName = LoadRandomNameFromXML();
	Character *newCharacter = new Character(newName);
	newCharacter->m_owningMap = owningMap;
	newCharacter->m_owningTile = owningMap->FindRandomPassableTileOfType();
	Vector2 randomPosInTile(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne());
	newCharacter->m_bounds = Disc2D(newCharacter->m_owningTile->GetTileCenter() + randomPosInTile, 0.5f);
	newCharacter->m_target = newCharacter->GetMapCoords();
	newCharacter->m_glyph = s_registry[name]->m_glyphs[0];
	newCharacter->m_glyphColor = s_registry[name]->m_glyphColors[0];
	newCharacter->m_stats = s_registry[name]->m_stats;
	newCharacter->m_faction = s_registry[name]->m_faction;
	newCharacter->m_maxHealth = newCharacter->m_stats.m_stats[CONSTITUTION];
	newCharacter->m_currentHealth = newCharacter->m_maxHealth;

	std::map<std::string, float>::iterator actionIter;
	for (actionIter = s_registry[name]->m_actionAttitudes.begin(); actionIter != s_registry[name]->m_actionAttitudes.end(); ++actionIter)
	{
		newCharacter->m_actionPraises[actionIter->first] = actionIter->second;
	}

	std::vector<Behavior*>::iterator behaviorIter;
	for (behaviorIter = s_registry[name]->m_behaviors.begin(); behaviorIter != s_registry[name]->m_behaviors.end(); ++behaviorIter)
	{
		Behavior* currentBehavior = *behaviorIter;
		newCharacter->m_behaviors.push_back(currentBehavior->Clone());
	}
	newCharacter->CalcBehaviorUtility();

	std::vector<Character*>& characters = Game::s_gameInstance->m_gameMap->m_characters;
	std::vector<Character*>::iterator characterIter;
	for (characterIter = characters.begin(); characterIter != characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter != nullptr && currentCharacter != newCharacter)
		{
			if (currentCharacter->m_faction == newCharacter->m_faction && currentCharacter->m_faction == "predator")
			{
				currentCharacter->m_socialRelations[newCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["PackMember"]->m_mainSocialRelation);
				newCharacter->m_socialRelations[currentCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["PackMember"]->m_subSocialRelation);
			}
			else if (currentCharacter->m_faction == newCharacter->m_faction && currentCharacter->m_faction == "prey")
			{
				currentCharacter->m_socialRelations[newCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["HerdMember"]->m_mainSocialRelation);
				newCharacter->m_socialRelations[currentCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["HerdMember"]->m_subSocialRelation);
			}
			else if (currentCharacter->m_faction != newCharacter->m_faction && currentCharacter->m_faction == "predator")
			{
				currentCharacter->m_socialRelations[newCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["Predator-Prey"]->m_mainSocialRelation);
				newCharacter->m_socialRelations[currentCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["Predator-Prey"]->m_subSocialRelation);
			}
			else if (currentCharacter->m_faction != newCharacter->m_faction && currentCharacter->m_faction == "prey")
			{
				currentCharacter->m_socialRelations[newCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["Predator-Prey"]->m_subSocialRelation);
				newCharacter->m_socialRelations[currentCharacter->m_name] = new SocialRelation(SocialRole::s_socialRoleRegistry["Predator-Prey"]->m_mainSocialRelation);
			}
		}
	}

	newCharacter->m_socialRelations[newCharacter->m_name] = new SocialRelation(1.f, 1.f, 1.f, 1.f);

	return newCharacter;
}

void CharacterBuilder::BuildCharacters(std::vector<Character*>& characters, Map* owningMap)
{
	IXMLDomParser iDom;
	iDom.setRemoveClears(false);
	ITCXMLNode xMainNode = iDom.openFileHelper("Data/XML/SpawnedCharacters.xml", "Characters");
	ValidateXmlElement(xMainNode, "Character", "");

	characters.resize(xMainNode.nChildNode());
	for (int nodeCount = 0; nodeCount < xMainNode.nChildNode(); nodeCount++)
	{
		ITCXMLNode characterNode = xMainNode.getChildNode(nodeCount);
		ValidateXmlElement(characterNode, "SocialRoles", "faction,id");
		std::string characterFaction = ParseXmlAttribute(characterNode, "faction", std::string("NO_FACTION_FOUND"));
		int characterIndex = ParseXmlAttribute(characterNode, "id", -1) - 1;
		Character* newCharacter = BuildCharacter(characterFaction, owningMap);
		characters[characterIndex] = newCharacter;
	}

	std::vector<Character*>::iterator characterIter;
	for (characterIter = characters.begin(); characterIter != characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		std::vector<Character*>::iterator otherCharacterIter;
		for (otherCharacterIter = characters.begin(); otherCharacterIter != characters.end(); ++otherCharacterIter)
		{
			Character* otherCharacter = *otherCharacterIter;

			if (currentCharacter != otherCharacter)
			{
				currentCharacter->m_socialRelations[otherCharacter->m_name] = new SocialRelation();
			}
		}
	}

	for (int nodeCount = 0; nodeCount < xMainNode.nChildNode(); nodeCount++)
	{
		ITCXMLNode socialRolesNode = xMainNode.getChildNode(nodeCount).getChildNode("SocialRoles");
		ValidateXmlElement(socialRolesNode, "Parent-Child,Predator-Prey,Leader-Member,PackMember,HerdMember,Sibling", "");

		for (int roleCount = 0; roleCount < socialRolesNode.nChildNode(); roleCount++)
		{
			ITCXMLNode socialRoleNode = socialRolesNode.getChildNode(roleCount);
			ValidateXmlElement(socialRoleNode, "", "sub,main");

			std::string socialRoleName = socialRoleNode.getName();
			std::string subIndices = ParseXmlAttribute(socialRoleNode, "sub", std::string("NOTHING_FOUND"));
			std::string mainIndices = ParseXmlAttribute(socialRoleNode, "main", std::string("NOTHING_FOUND"));
			std::vector<int> subIDs;
			std::vector<int> mainIDs;
			if (subIndices != "NOTHING_FOUND")
			{
				subIDs = ParseStringIntoIntPiecesByDelimiter(subIndices, ",");
			}
			if (mainIndices != "NOTHING_FOUND")
			{
				mainIDs = ParseStringIntoIntPiecesByDelimiter(mainIndices, ",");
			}
			for (std::vector<int>::iterator currentIDIter = subIDs.begin(); currentIDIter != subIDs.end(); ++currentIDIter)
			{
				int& currentID = *currentIDIter;
				std::string otherCharacterName = characters[currentID - 1]->m_name;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_numSocialRoles++;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_liking += SocialRole::s_socialRoleRegistry[socialRoleName]->m_mainSocialRelation->m_liking;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_dominance += SocialRole::s_socialRoleRegistry[socialRoleName]->m_mainSocialRelation->m_dominance;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_solidarity += SocialRole::s_socialRoleRegistry[socialRoleName]->m_mainSocialRelation->m_solidarity;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_familiarity += SocialRole::s_socialRoleRegistry[socialRoleName]->m_mainSocialRelation->m_familiarity;
			}

			for (std::vector<int>::iterator currentIDIter = mainIDs.begin(); currentIDIter != mainIDs.end(); ++currentIDIter)
			{
				int& currentID = *currentIDIter;
				std::string otherCharacterName = characters[currentID - 1]->m_name;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_numSocialRoles++;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_liking += SocialRole::s_socialRoleRegistry[socialRoleName]->m_subSocialRelation->m_liking;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_dominance += SocialRole::s_socialRoleRegistry[socialRoleName]->m_subSocialRelation->m_dominance;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_solidarity += SocialRole::s_socialRoleRegistry[socialRoleName]->m_subSocialRelation->m_solidarity;
				characters[nodeCount]->m_socialRelations[otherCharacterName]->m_familiarity += SocialRole::s_socialRoleRegistry[socialRoleName]->m_subSocialRelation->m_familiarity;
			}
		}
	}

	for (characterIter = characters.begin(); characterIter != characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		std::map<std::string, SocialRelation*>::iterator socialRelationIter;
		for (socialRelationIter = currentCharacter->m_socialRelations.begin(); socialRelationIter != currentCharacter->m_socialRelations.end(); ++socialRelationIter)
		{
			SocialRelation* currentSocialRelation = socialRelationIter->second;
			if (currentSocialRelation->m_numSocialRoles == 0)
			{
				continue;
			}
			float divisor = 1.f / (float)currentSocialRelation->m_numSocialRoles;
			currentSocialRelation->m_liking *= divisor;
			currentSocialRelation->m_dominance *= divisor;
			currentSocialRelation->m_solidarity *= divisor;
			currentSocialRelation->m_familiarity *= divisor;
		}

		currentCharacter->m_socialRelations[currentCharacter->m_name] = new SocialRelation(1.f, 1.f, 1.f, 1.f);
	}
}

void CharacterBuilder::LoadBehaviors(const ITCXMLNode& behaviorNode)
{
	ValidateXmlElement(behaviorNode, "Wander,Attack,Flee,Follow,Hide,Mate,Pursue,FreakOut,Recuperate,ShowOff,Sulk,Cry,Smile,Tantrum,Sigh,Gloat,Imitate,Blame,Daydream,Eat", "");
	for (int behaviorCount = 0; behaviorCount < behaviorNode.nChildNode(); behaviorCount++)
	{
		bool didAddBehavior = false;
		ITCXMLNode childNode = behaviorNode.getChildNode(behaviorCount);

		if (std::strcmp(childNode.getName(), "Wander") == 0)
		{
			m_behaviors.push_back(Behavior::s_wanderBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Attack") == 0)
		{
			m_behaviors.push_back(Behavior::s_attackBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Flee") == 0)
		{
			m_behaviors.push_back(Behavior::s_fleeBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Follow") == 0)
		{
			m_behaviors.push_back(Behavior::s_followBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Hide") == 0)
		{
			m_behaviors.push_back(Behavior::s_hideBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Mate") == 0)
		{
			m_behaviors.push_back(Behavior::s_mateBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "FreakOut") == 0)
		{
			m_behaviors.push_back(Behavior::s_recklessBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Recuperate") == 0)
		{
			m_behaviors.push_back(Behavior::s_recuperateBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "ShowOff") == 0)
		{
			m_behaviors.push_back(Behavior::s_showOffBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Sulk") == 0)
		{
			m_behaviors.push_back(Behavior::s_sulkBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Cry") == 0)
		{
			m_behaviors.push_back(Behavior::s_cryBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Smile") == 0)
		{
			m_behaviors.push_back(Behavior::s_smileBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Tantrum") == 0)
		{
			m_behaviors.push_back(Behavior::s_tantrumBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Sigh") == 0)
		{
			m_behaviors.push_back(Behavior::s_sighBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Gloat") == 0)
		{
			m_behaviors.push_back(Behavior::s_gloatBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Imitate") == 0)
		{
			m_behaviors.push_back(Behavior::s_imitateBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Blame") == 0)
		{
			m_behaviors.push_back(Behavior::s_blameBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Daydream") == 0)
		{
			m_behaviors.push_back(Behavior::s_daydreamBehavior->Clone());
			didAddBehavior = true;
		}
		else if (std::strcmp(childNode.getName(), "Eat") == 0)
		{
			m_behaviors.push_back(Behavior::s_eatBehavior->Clone());
			didAddBehavior = true;
		}
		if (didAddBehavior)
		{
			m_behaviors.back()->m_mapRange = ParseXmlAttribute(childNode, "mapRange", IntRange(0, 0));
			m_behaviors.back()->m_speedModifier = ParseXmlAttribute(childNode, "speedChangePercentage", 1.f);
			m_behaviors.back()->m_skittishness = ParseXmlAttribute(childNode, "skittishness", 0.f);
			m_behaviors.back()->m_sightRange = ParseXmlAttribute(childNode, "sightRange", 0);
			m_behaviors.back()->m_behaviorRange = ParseXmlAttribute(childNode, "behaviorRange", 0.f);
		}
	}
}

void CharacterBuilder::LoadActionAttitudes(const ITCXMLNode& actionAttitudesNode)
{
	ValidateXmlElement(actionAttitudesNode, "ActionAttitude", "");
	for (int actionCount = 0; actionCount < actionAttitudesNode.nChildNode(); actionCount++)
	{
		ITCXMLNode actionAttitudeNode = actionAttitudesNode.getChildNode("ActionAttitude", actionCount);
		ValidateXmlElement(actionAttitudeNode, "", "actionName,attitude");
		std::string attitudeName = ParseXmlAttribute(actionAttitudeNode, "actionName", std::string("ACTION_NAME_NOT_FOUND"));
		float attitude = ParseXmlAttribute(actionAttitudeNode, "attitude", 0.f);
		m_actionAttitudes[attitudeName] = attitude;
	}
}

std::string CharacterBuilder::LoadRandomNameFromXML()
{
	IXMLDomParser iDom;
	iDom.setRemoveClears(false);
	ITCXMLNode xMainNode = iDom.openFileHelper("Data/XML/Names.xml", "Names");
	ValidateXmlElement(xMainNode, "Name", "");

	int numChildren = xMainNode.nChildNode();
	int childIndex = GetRandomIntInRange(0, numChildren);

	ITCXMLNode childNode = xMainNode.getChildNode("Name", childIndex);
	std::string newName = ParseXmlAttribute(childNode, "name", std::string("NO_NAME_FOUND"));
	return newName;
}
