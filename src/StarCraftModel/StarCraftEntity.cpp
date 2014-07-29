#ifndef STARCRAFTENTITY_H
#include "StarCraftEntity.h"
#endif
#ifndef METADATA_H
#include "MetaData.h"
#endif
#ifndef DEFINITIONCROSSMAPPING_H
#include "DefinitionCrossMapping.h"
#endif
#include "IStrategizerException.h"
#include "MathHelper.h"
#include "RtsGame.h"
#include "GamePlayer.h"
#include "ObjectFactory.h"
#include "Action.h"

using namespace IStrategizer;
using namespace BWAPI;
using namespace std;

DECL_SERIALIZABLE(StarCraftEntity);

StarCraftEntity::StarCraftEntity(Unit p_unit) :
GameEntity(p_unit->getID()),
m_pUnit(p_unit),
m_isOnline(true)
{
}
//----------------------------------------------------------------------------------------------
int StarCraftEntity::P(EntityObjectProperty attrId) const
{
    // Attributes are accessible from game directly if the entity is online in 2 cases only
    // 1- Unit is visible to player
    // 2- Unit belongs to neutral and game frame is 0
    // 3- Unit is about to hide and this frame is marked as cache frame
    bool isVisible = m_pUnit->isVisible();
    bool isNeutralAtFrame0 = m_pUnit->getPlayer()->isNeutral() && g_Game->GameFrame() == 0;
    bool isCacheFrame = (int)g_Game->GameFrame() == m_cacheFrame;

    if (m_isOnline &&
        (isVisible || isNeutralAtFrame0 || isCacheFrame))
    {
        // Positions are measured in pixels and are the highest resolution
        // Walk Tiles - each walk tile is an 8x8 square of pixels. These are called walk tiles because walkability data is available at this resolution.
        // Build Tiles - each build tile is a 4x4 square of walk tiles, or a 32x32 square of pixels.
        // These are called build tiles because buildability data is available at this resolution, and correspond to the tiles seen in game.
        // For example, a Command Center occupies an area of 4x3 build tiles. Build tiles can be specified in TilePosition objects.
        // We will use walk tiles as a measure of positions units across IStrategizer
        // See: http://code.google.com/p/bwapi/wiki/Misc

        switch (attrId)
        {
        case OP_Health:
            return m_pUnit->getHitPoints();

        case OP_Left:
            if (m_pUnit->getType().isBuilding())
                return UnitPositionFromTilePosition(m_pUnit->getTilePosition().x);
            else
                return m_pUnit->getLeft();

        case OP_Top:
            if (m_pUnit->getType().isBuilding())
                return UnitPositionFromTilePosition(m_pUnit->getTilePosition().y);
            else
                return m_pUnit->getTop();

        case OP_Right:
            if (m_pUnit->getType().isBuilding())
            {
                int w = m_pUnit->getType().tileWidth();
                int x = m_pUnit->getTilePosition().x;
                return UnitPositionFromTilePosition(x + w);
            }
            else
                return m_pUnit->getRight();

        case OP_Bottom:
            if (m_pUnit->getType().isBuilding())
                return UnitPositionFromTilePosition(m_pUnit->getTilePosition().y + m_pUnit->getType().tileHeight());
            else
                return m_pUnit->getBottom();
        case OP_State:
            return FetchState();

        case OP_OwnerId:
            return g_Database.PlayerMapping.GetByFirst(m_pUnit->getPlayer()->getID());

        case OP_TypeId:
            return g_Database.EntityMapping.GetByFirst(m_pUnit->getType().getID());

        case OP_PosCenterX:
            return m_pUnit->getPosition().x;

        case OP_PosCenterY:
            return m_pUnit->getPosition().y;

        case OP_IsGatheringSecondaryResource:
            return m_pUnit->isGatheringGas() || m_pUnit->isCarryingGas();

        case OP_IsGatheringPrimaryResource:
            return m_pUnit->isGatheringMinerals() || m_pUnit->isCarryingMinerals();

        case OP_IsBeingGathered:
            return m_pUnit->isBeingGathered();

        case OP_OrderTargetId:
            return GetTargetId();

        case OP_IsBeingHit:
            return m_pUnit->isUnderAttack();

        case OP_IsMoving:
            return m_pUnit->isMoving();

        case OP_IsAttacking:
            return m_pUnit->isAttacking();

        default:
            DEBUG_THROW(InvalidParameterException(XcptHere));
        }
    }
    else
    {
        _ASSERTE(m_cachedAttr.Contains(attrId));
        return m_cachedAttr.at(attrId);
    }
}
//----------------------------------------------------------------------------------------------
ObjectStateType StarCraftEntity::FetchState() const
{
    bool isIdle = m_pUnit->isIdle();
    bool isCompleted = m_pUnit->isCompleted();
    bool isBeingConstructed = m_pUnit->isBeingConstructed();
    bool isConstructing = m_pUnit->isConstructing();
    bool isMoving = m_pUnit->isMoving();
    bool isGatheringGas = m_pUnit->isGatheringGas();
    bool isGatheringMinerals = m_pUnit->isGatheringMinerals();
    bool isTraining = m_pUnit->isTraining();
    bool isAttacking = m_pUnit->isAttacking();
    bool isUnderAttack = m_pUnit->isUnderAttack();

    if (isIdle && isCompleted && m_pUnit->isInterruptible())
        return OBJSTATE_Idle;
    else if (isBeingConstructed || (isIdle && !isCompleted))
        return OBJSTATE_BeingConstructed;
    else if (isGatheringMinerals)
        return OBJSTATE_GatheringPrimary;
    else if (isGatheringGas)
        return OBJSTATE_GatheringSecondary;
    else if (isConstructing)
        return OBJSTATE_Constructing;
    else if (isMoving)
        return OBJSTATE_Moving;
    else if (isTraining)
        return OBJSTATE_Training;
    else if (isAttacking)
        return OBJSTATE_Attacking;
    else if (isUnderAttack)
        return OBJSTATE_UnderAttack;
    else
        return OBJSTATE_END;
}
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::IsTraining(TID p_traineeId) const
{
    GameEntity* pTrainee = g_Game->GetPlayer((PlayerType)P(OP_OwnerId))->GetEntity(p_traineeId);

    if (nullptr == pTrainee)
        DEBUG_THROW(ItemNotFoundException(XcptHere));

    return MathHelper::RectangleMembership(
        P(OP_Left),
        P(OP_Top),
        P(OP_Right) - P(OP_Left),
        P(OP_Bottom) - P(OP_Top),
        pTrainee->P(OP_PosCenterX),
        pTrainee->P(OP_PosCenterY));
}
//----------------------------------------------------------------------------------------------
string StarCraftEntity::ToString(bool minimal) const
{
    char str[512];
    TID gameTypeId = g_Database.EntityMapping.GetBySecond((EntityClassType)P(OP_TypeId));
    std::string description = g_Database.EntityIdentMapping.GetByFirst(gameTypeId);

    if (minimal)
        sprintf_s(str, "%s[%d]", description.c_str(), m_id);
    else
    {
        std::string asResource = SharedResource::ToString();
        sprintf_s(str, "%s[%d](%s, State=%s)", description.c_str(), m_id, asResource.c_str(), Enums[P(OP_State)]);
    }
    return str;
}
//----------------------------------------------------------------------------------------------
Vector2 StarCraftEntity::GetPosition() const
{
    return Vector2(P(OP_PosCenterX), P(OP_PosCenterY));
}
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::CanGather(TID resourceObjectId) const
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    return m_pUnit->canGather(Broodwar->getUnit(resourceObjectId));
}
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::Build(EntityClassType p_buildingClassId, Vector2 p_position)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    TilePosition pos(TilePositionFromUnitPosition(p_position.X), TilePositionFromUnitPosition(p_position.Y));
    UnitType type;
    TID gameTypeId;
    string typeName;

    gameTypeId = g_Database.EntityMapping.GetBySecond(p_buildingClassId);
    typeName = g_Database.EntityIdentMapping.GetByFirst(gameTypeId);

    type = UnitType::getType(typeName);
    type = BWAPI::UnitType::getType(typeName);

    LogInfo("%s -> Build(%s@<%d,%d>)", ToString().c_str(), type.toString().c_str(), pos.x, pos.y);

    bool bOk = false;

    if (type.isAddon())
        bOk = m_pUnit->buildAddon(type);
    else
    {
        bOk = m_pUnit->build(type, pos);
    }

    if (!bOk)
        DebugDrawMapLastGameError();

    return bOk;
};
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::Research(ResearchType p_researchId)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    bool bOk = false;

    // Is tech
    if ((int)p_researchId >= ((int)(RESEARCH_START + TechIdOffset)))
    {
        TID gameTypeID = g_Database.TechMapping.GetBySecond(p_researchId);

        bOk = m_pUnit->research(TechType(gameTypeID));
    }
    // Is upgrade
    else
    {
        TID gameTypeID = g_Database.UpgradeMapping.GetBySecond(p_researchId);
        bOk = m_pUnit->upgrade(UpgradeType(gameTypeID));
    }

    if (!bOk)
        DebugDrawMapLastGameError();

    return bOk;
}
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::AttackGround(Vector2 p_position)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    Position pos(p_position.X, p_position.Y);
    
    if (m_pUnit->attack(pos))
        return true;
    else
    {
        DebugDrawMapLastGameError();
        return false;
    }
};
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::Stop()
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    LogInfo("%s -> Stop", ToString().c_str());

    if (m_pUnit->stop())
        return true;
    else
    {
        DebugDrawMapLastGameError();
        return false;
    }
};
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::AttackEntity(TID targetId)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    LogInfo("%s -> Attack(%s)", ToString().c_str(), g_Game->GetEntity(targetId)->ToString().c_str());

    // if we have issued a command to this unit already this frame, 
    // ignore this one and raise warning
    if (m_pUnit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
    {
        LogWarning("Entity %s command drop", ToString().c_str());
        return true;
    }

    BWAPI::UnitCommand currentCommand(m_pUnit->getLastCommand());

    if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&
        currentCommand.getTarget()->getID() == targetId &&
        ((!m_pUnit->isStuck() && !m_pUnit->isIdle()) || !m_pUnit->exists()))
    {
        LogWarning("Entity %s command drop", ToString().c_str());
        return true;
    }

    auto target = Broodwar->getUnit(targetId);

    _ASSERTE(m_pUnit->isDetected());

    if (m_pUnit->attack(target))
        return true;
    else
    {
        DebugDrawMapLastGameError();
        return false;
    }
};
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::Train(EntityClassType p_entityClassId)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    Unit building = m_pUnit;
    TID unitTypeId = g_Database.EntityMapping.GetBySecond(p_entityClassId);
    string typeName = g_Database.EntityIdentMapping.GetByFirst(unitTypeId);
    UnitType type = BWAPI::UnitType::getType(typeName);

    _ASSERTE(building->canTrain(type));

    LogInfo("%s -> Train(Trainee=%s)", ToString().c_str(), type.toString().c_str());

    if (building->train(type))
        return true;
    else
    {
        DebugDrawMapLastGameError();
        return false;
    }
};
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::Move(Vector2 targetPos)
{
    _ASSERTE(!targetPos.IsInf());
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    LogInfo("%s -> Move(%s)", ToString().c_str(), targetPos.ToString().c_str());

    // if we have issued a command to this unit already this frame, 
    // ignore this one and raise warning
    if (m_pUnit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
    {
        LogWarning("Entity %s command drop", ToString().c_str());
        return true;
    }

    // get the unit's current command
    BWAPI::UnitCommand currentCommand(m_pUnit->getLastCommand());
    Position pos(targetPos.X, targetPos.Y);

    // if we've already told this unit to attack this target, ignore this command
    if (currentCommand.getType() == BWAPI::UnitCommandTypes::Move &&
        currentCommand.getTargetPosition() == pos &&
        !m_pUnit->isStuck() &&
        !m_pUnit->isIdle())
    {
        LogWarning("Entity %s command drop", ToString().c_str());
        return true;
    }

    //_ASSERTE(Broodwar->isWalkable(WalkPositionFromUnitPosition(pos.x), WalkPositionFromUnitPosition(pos.y)));
    if (m_pUnit->move(pos))
        return true;
    else
    {
        DebugDrawMapLastGameError();
        return false;
    }
}
//----------------------------------------------------------------------------------------------
bool StarCraftEntity::GatherResourceEntity(TID resourceId)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    // if we have issued a command to this unit already this frame, 
    // ignore this one and raise warning
    if (m_pUnit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
    {
        LogWarning("Entity %s command drop", ToString().c_str());
        return true;
    }

    // get the unit's current command
    BWAPI::UnitCommand currentCommand(m_pUnit->getLastCommand());

    // if we've already told this unit to attack this target, ignore this command
    if (currentCommand.getType() == BWAPI::UnitCommandTypes::Right_Click_Unit &&
        currentCommand.getTarget()->getID() == resourceId &&
        !m_pUnit->isStuck() &&
        !m_pUnit->isIdle())
    {
        return true;
    }

    auto resource = Broodwar->getUnit(resourceId);

    LogInfo("%s -> GatherResource(Resource=%s)", ToString().c_str(), resource->getType().toString().c_str());

    if (m_pUnit->gather(resource))
        return true;
    else
    {
        DebugDrawMapLastGameError();
        return false;
    }
}
//----------------------------------------------------------------------------------------------
void StarCraftEntity::SetOffline(RtsGame* pBelongingGame)
{
    CacheAttributes();
    m_isOnline = false;
}
//////////////////////////////////////////////////////////////////////////
void StarCraftEntity::CancelOrders()
{
    LogInfo("%s -> Cancel All Active Orders", ToString().c_str());

    if (m_pUnit->cancelAddon())
        LogInfo("%s canceled add-on construction", ToString().c_str());
    if (m_pUnit->cancelConstruction())
        LogInfo("%s canceled building construction", ToString().c_str());
    if (m_pUnit->cancelMorph())
        LogInfo("%s canceled moreph", ToString().c_str());
    if (m_pUnit->cancelResearch())
        LogInfo("%s canceled research", ToString().c_str());
    if (m_pUnit->cancelTrain())
        LogInfo("%s canceled train", ToString().c_str());
    if (m_pUnit->cancelUpgrade())
        LogInfo("%s canceled upgrade", ToString().c_str());
}
//////////////////////////////////////////////////////////////////////////
bool StarCraftEntity::CanTrain(EntityClassType type)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    TID unitTypeId = g_Database.EntityMapping.GetBySecond(type);
    string typeName = g_Database.EntityIdentMapping.GetByFirst(unitTypeId);
    UnitType gameType = BWAPI::UnitType::getType(typeName);

    return m_pUnit->canTrain(gameType);
}
//////////////////////////////////////////////////////////////////////////
bool StarCraftEntity::Follow(TID entityId)
{
    if (!m_isOnline)
        DEBUG_THROW(InvalidOperationException(XcptHere));

    LogInfo("%s -> Follow(Target=%d)", ToString().c_str(), entityId);

    if (m_pUnit->follow(Broodwar->getUnit(entityId)))
        return true;
    else
        return false;
}
//////////////////////////////////////////////////////////////////////////
bool StarCraftEntity::Exists() const
{
    return m_pUnit->exists();
}
//////////////////////////////////////////////////////////////////////////
TID StarCraftEntity::GetTargetId() const
{
    int id = INVALID_TID;

    BWAPI::Unit target = m_pUnit->getTarget();
    if (target == NULL)
    {
        target = m_pUnit->getOrderTarget();
        if (target != NULL)
        {
            id = target->getID();
        }
    }

    return id;
}
//////////////////////////////////////////////////////////////////////////
Vector2 StarCraftEntity::GetTargetPosition() const
{
    BWAPI::Position pos;

    if (m_pUnit->getTarget() != nullptr)
        pos = m_pUnit->getTarget()->getPosition();
    else
        pos = m_pUnit->getTargetPosition();

    return Vector2(pos.x, pos.y);
}
//////////////////////////////////////////////////////////////////////////
bool StarCraftEntity::CanAttack(_In_ TID targetId) const
{
    return m_pUnit->canAttackUnit(Broodwar->getUnit(targetId));
}
//////////////////////////////////////////////////////////////////////////
void StarCraftEntity::DebugDrawMapLastGameError()
{
    Error lastErr = Broodwar->getLastError();
    Position unitPos = m_pUnit->getPosition();

    Broodwar->registerEvent([unitPos, lastErr](Game*){ Broodwar->drawTextMap(unitPos, "%c%s", Text::Red, lastErr.c_str()); },   // action
        nullptr,    // condition
        Broodwar->getLatencyFrames());  // frames to run

    LogInfo("BWAPI Game Error: %s", lastErr.c_str());
}