#ifndef WARGUSPLAYERRESOURCES_H
#include "StarCraftPlayerResources.h"
#endif

#include "BWAPI.h"
#include "DefinitionCrossMapping.h"
#include "IStrategizerException.h"

using namespace StarCraftModel;
using namespace BWAPI;

int StarCraftPlayerResources::Supply() 
{
    // The amount of supply is doubled because of an issue with the Zerg supply
    // that's why we divide the supplyTotal over 2.
    // For more info check documentation for supplyTotal API.
    int bwapiTotalSupply = m_player->supplyTotal();
    int bwapiUsedSupply = m_player->supplyUsed();
    int totalSupply = bwapiTotalSupply / 2;
    int usedSupply = (bwapiUsedSupply / 2);
    int amount = totalSupply - usedSupply;

    return amount;
}
//////////////////////////////////////////////////////////////////////////
int StarCraftPlayerResources::Secondary()
{
    int amount = m_player->gas();
    return amount;
}
//////////////////////////////////////////////////////////////////////////
int StarCraftPlayerResources::Primary()
{
    int amount = m_player->minerals();
    return amount;
}
//////////////////////////////////////////////////////////////////////////
EntityClassType StarCraftPlayerResources::GetEntityClassType(ResourceType p_type) const
{
    switch(p_type)
    {
    case RESOURCE_Primary:
        return g_Database.EntityMapping.GetByFirst(UnitTypes::Resource_Mineral_Field.getID());
    case RESOURCE_Secondary:
        return g_Database.EntityMapping.GetByFirst(UnitTypes::Terran_Refinery.getID());
    default:
        DEBUG_THROW(InvalidParameterException(XcptHere));
    }
}
