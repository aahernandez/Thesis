#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Game/Stats.hpp"
#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2D.hpp"

#include <map>

class Event;
class Personality;
class Behavior;
class Tile;
class Map;
class Log;

typedef std::vector<Tile*> Path;

class Character : public Entity
{
public:
public:
	std::vector<Behavior*> m_behaviors;
	Behavior *m_currentBehavior;
	Path m_path;
	std::string m_faction;
	Character *m_currentAimTarget;
	Tile *m_owningTile;
	Map *m_owningMap;
	char m_glyph;
	Stats m_stats;
	RGBA m_glyphColor;
	int m_maxHealth;
	int m_currentHealth;
	Log *m_damageLog;
	Vector2 m_logPosition;
	IntVector2 m_target;
	float m_speed;
	std::vector<Character*> m_charactersInSight;
	Disc2D m_bounds;
	float m_hungerFulfillment;

	Character(std::string name);
	~Character();

	void Initialize();
	void Update(float deltaSeconds);
	void UpdateNeeds(float deltaSeconds);

	virtual void Act(float deltaSeconds);

	void GenerateHungerEvent();

	void PutSelfOnNewTile(Tile *newTile);
	void MoveTowardsNewTile(float deltaSeconds, Tile *newTile);
	void AttackTargetTile(Tile *newTile);

	bool IsNorthTileSolid();
	bool IsSouthTileSolid();
	bool IsWestTileSolid();
	bool IsEastTileSolid();

	bool IsOnTile(Tile* tileToCheck);

	virtual void FillLogWithDetails(Log *log);
	void CalcBehaviorUtility();
	void TakeDirectDamage(int damage);
	void TakeDamage(int damage);
	bool AreFactionsSame(std::string factionName);
	float GetTileGCost(std::string tileName);
	void GenerateAndSetRandomPath();
	void GenerateAndSetRandomCardinalPath();
	void GeneratePathToTarget();
	void GenerateAndSetRandomPathWithinRadius(float radius);
	bool IsOtherCharacterInPath();
	void FollowPath(float deltaSeconds);
	void ClearPath();
	bool HasPath();
	Character* GetMostLikedCharacter();
	Character* GetMostLikedCharacterOfSameFaction();
	Character* GetLeastLikedCharacterOfSameFaction();

	Behavior* GetBehaviorWithName(std::string behaviorName);

	void DebugRender() const;

	void RemoveSelfFromOwningTile();
	IntVector2 GetMapCoords();
	void UpdateBehavior(float deltaSeconds);
	void UpdateEntity(float deltaSeconds);
	void Render() const;
	int CalcDamage();
	bool HasLineOfSight(Character *character);

	void SpawnCorpse();
};