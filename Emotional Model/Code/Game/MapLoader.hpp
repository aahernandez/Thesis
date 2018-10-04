#pragma once
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"
#include <string>
#include <map>

class Map;
class TileDefinition;

class MapLoader
{
public:
	std::string m_name;
	std::string m_filePath;
	IntRange m_offsetX;
	IntRange m_offsetY;
	bool m_isMirrored;
	IntRange m_numRightAngleRotations;
	std::map<const char, TileDefinition*> m_keys;

	MapLoader(const ITCXMLNode& xmlNode);
	~MapLoader();

	void PopulateMap(Map *mapToPopulate);
	void LoadKeys(const ITCXMLNode& xmlNode);
	IntVector2 GetRotatedPosition(int numRotation, const IntVector2& position);
};