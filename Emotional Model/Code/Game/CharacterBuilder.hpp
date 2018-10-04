#pragma once
#include "Engine/Core/RGBA.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"
#include "Game/DataDrivenBuilder.hpp"
#include <map>
#include <vector>

class Behavior;
class Character;
class Map;

class CharacterBuilder : DataDrivenBuilder<CharacterBuilder>
{
public:
	std::string m_lootTableName;
	std::string m_faction;
	std::vector<RGBA> m_glyphColors;
	std::vector<char> m_glyphs;
	std::vector<Behavior*> m_behaviors;
	std::map<std::string, float> m_actionAttitudes;
	Stats m_stats;

	CharacterBuilder(const ITCXMLNode& xmlNode);
	~CharacterBuilder();

	static void LoadCharactersFromXML();
	static Character* BuildCharacter(std::string name, Map* owningMap);
	static void BuildCharacters(std::vector<Character*>& characters, Map* owningMap);

	void LoadBehaviors(const ITCXMLNode& behaviorNode);
	void LoadActionAttitudes(const ITCXMLNode& actionAttitudesNode);
	static std::string LoadRandomNameFromXML();
};