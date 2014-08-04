#include "StarCraftMap.h"
#include <string>

#include "BWAPI.h"
#include "MapArea.h"
#include "GamePlayer.h"
#include "DefinitionCrossMapping.h"
#include "StarCraftEntity.h"
#include "RtsGame.h"

using namespace IStrategizer;
using namespace BWAPI;
using namespace std;

StarCraftMap::StarCraftMap(RtsGame* pGame, unsigned cellSize, unsigned worldWidth, unsigned worldHeight) :
WorldMap(pGame, cellSize, worldWidth, worldHeight)
{ }
//----------------------------------------------------------------------------------------------
MapArea StarCraftMap::GetSpecialBuildingPosition(EntityClassType p_buildingType) const
{
	if (!m_isOnline)
		DEBUG_THROW(InvalidOperationException(XcptHere));

	// Add-ons are always built in place of its parent building, it has nothing to do with
	// place adaptation
	MapArea candidatePosition = MapArea::Null();

	// Get the unit type object
	UnitType type;
	TID unitTypeId;
	string typeName;
	unitTypeId = g_Database.EntityMapping.GetBySecond(p_buildingType);
	typeName = g_Database.EntityIdentMapping.GetByFirst(unitTypeId);
	type = BWAPI::UnitType::getType(typeName);

	if (type.isRefinery())
	{
		// Get the player base tile position
		MapArea colony = g_Game->Self()->GetColonyMapArea();
		Vector2 colonyTile = colony.Pos();

		int bestDistance = INT_MAX;
		Unitset geysers = Broodwar->getGeysers();
		Unit currentGeyser;

		for (unsigned i = 0; i < geysers.size(); ++i)
		{
			currentGeyser = geysers[i];

			Vector2 currentPosition = Vector2(
				UnitPositionFromTilePosition(currentGeyser->getTilePosition().x),
				UnitPositionFromTilePosition(currentGeyser->getTilePosition().y));

			if (CanBuildHere(currentPosition, p_buildingType))
			{
				int currentDistance = colonyTile.Distance(currentPosition);

				if (currentDistance <= bestDistance)
				{
					bestDistance = currentDistance;
					candidatePosition = MapArea(
						currentPosition,
						type.tileWidth(),
						type.tileHeight());
				}
			}
		}
	}

	return candidatePosition;
}
//----------------------------------------------------------------------------------------------
bool StarCraftMap::CanBuildHere(Vector2 p_worldPosition, EntityClassType p_buildingType) const
{
	if (!m_isOnline)
		DEBUG_THROW(InvalidOperationException(XcptHere));

	UnitType type;
	TID unitTypeId;
	string typeName;

	unitTypeId = g_Database.EntityMapping.GetBySecond(p_buildingType);
	typeName = g_Database.EntityIdentMapping.GetByFirst(unitTypeId);
	type = BWAPI::UnitType::getType(typeName);

	if (type.isAddon())
	{
		type = type.whatBuilds().first;
	}

	auto tilePos = TilePosition(TilePositionFromUnitPosition(p_worldPosition.X), TilePositionFromUnitPosition(p_worldPosition.Y));
	return Broodwar->canBuildHere(tilePos, type) && Broodwar->isExplored(tilePos);
}
//////////////////////////////////////////////////////////////////////////
bool StarCraftMap::CanBuildHere(Vector2 pos) const
{
	if (!m_isOnline)
		DEBUG_THROW(InvalidOperationException(XcptHere));

	return Broodwar->isBuildable(TilePositionFromUnitPosition(pos.X), TilePositionFromUnitPosition(pos.Y), false) &&
		Broodwar->isExplored(TilePositionFromUnitPosition(pos.X), TilePositionFromUnitPosition(pos.Y));
}
//////////////////////////////////////////////////////////////////////////
void StarCraftMap::SpawnLocations(_Out_ std::vector<Vector2>& spawnPoints) const
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    auto locSet = Broodwar->getStartLocations();

    for (auto& loc : locSet)
    {
        spawnPoints.push_back(Vector2(loc.x * TILE_SIZE, loc.y * TILE_SIZE));
    }
}
//////////////////////////////////////////////////////////////////////////
bool StarCraftMap::IsLocationExplored(Vector2 loc) const
{
    return Broodwar->isExplored(TilePositionFromUnitPosition(loc.X), TilePositionFromUnitPosition(loc.Y));
}
//////////////////////////////////////////////////////////////////////////
Vector2 StarCraftMap::GetClosestReachableRegionCenter(_In_ TID entityId) const
{
    auto pUnit = Broodwar->getUnit(entityId);

    if (pUnit == nullptr)
        return Vector2::Inf();

    auto regionCenter = pUnit->getRegion()->getCenter();

    return Vector2(regionCenter.x, regionCenter.y);
}
//////////////////////////////////////////////////////////////////////////
void StarCraftMap::DebugDraw()
{
    for (auto regionR : Broodwar->getAllRegions())
    {
        Broodwar->drawBoxMap(
            regionR->getBoundsLeft(),
            regionR->getBoundsTop(),
            regionR->getBoundsRight(),
            regionR->getBoundsBottom(),
            (regionR->isAccessible() ? BWAPI::Colors::Yellow : BWAPI::Colors::Grey),
            false);

        Broodwar->drawTextMap(regionR->getCenter(), "ID:%d RGRP-ID:%d", regionR->getID(), regionR->getRegionGroupID());

        for (auto nRegionR : regionR->getNeighbors())
        {
            if (nRegionR->isAccessible())
                Broodwar->drawLineMap(regionR->getCenter(), nRegionR->getCenter(), BWAPI::Colors::Green);
        }
    }
}
