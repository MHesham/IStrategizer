#ifndef GAMEPLAYER_H
#include "GamePlayer.h"
#endif
#include <cassert>
#ifndef GAMEENTITY_H
#include "GameEntity.h"
#endif
#ifndef PLAYERRESOURCES_H
#include "PlayerResources.h"
#endif
#ifndef ENGINEASSIST_H
#include "EngineAssist.h"
#endif
#ifndef TOOLBOX_H
#include "Toolbox.h"
#endif
#include "GameTechTree.h"
#include "IMSystemManager.h"
#include "DataMessage.h"
#include "MessagePump.h"
#include "Logger.h"
#include "AttributesMetaData.h"
#include "GameType.h"
#include "IStrategizerException.h"
#include "SimilarityWeightModel.h"

using namespace IStrategizer;
using namespace std;

GamePlayer::GamePlayer(TID raceId) :
m_pResources(nullptr),
m_pTechTree(nullptr),
m_raceId(raceId),
m_isOnline(true)
{
    m_colonyCenter = MapArea::Null();
}
//////////////////////////////////////////////////////////////////////////
GamePlayer::~GamePlayer()
{
    Finalize();
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::Init()
{
    if (m_isOnline)
    {
        g_MessagePump->RegisterForMessage(MSG_EntityCreate, this);
        g_MessagePump->RegisterForMessage(MSG_EntityDestroy, this);
        g_MessagePump->RegisterForMessage(MSG_EntityRenegade, this);
        g_MessagePump->RegisterForMessage(MSG_EntityShow, this);
        g_MessagePump->RegisterForMessage(MSG_EntityHide, this);
    }
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::Finalize()
{
    for (auto entityEntry : m_entities)
        Toolbox::MemoryClean(entityEntry.second);
    m_entities.clear();

    Toolbox::MemoryClean(m_pResources);
    Toolbox::MemoryClean(m_pTechTree);
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::Entities(EntityList& entityIds)
{
    m_entities.Keys(entityIds);
}
//////////////////////////////////////////////////////////////////////////
GameEntity* GamePlayer::GetEntity(TID id)
{
    _ASSERTE(id != INVALID_TID);

    if (m_entities.count(id) == 0)
        return nullptr;

    return m_entities[id];
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::GetBases(EntityList &bases)
{
    EntityClassType typeId = Race()->GetBaseType();
    Entities(typeId, bases);
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::GetWorkers(_Out_ EntityList& workers)
{
    EntityClassType typeId = Race()->GetWorkerType();
    Entities(typeId, workers);
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::Entities(EntityClassType typeId, EntityList &entityIds, bool checkReadyOnly, bool checkFree)
{
    entityIds.clear();
    for (EntitiesMap::iterator itr = m_entities.begin(); itr != m_entities.end(); ++itr)
    {
        if (itr->second->TypeId() == typeId &&
            (!checkReadyOnly || itr->second->P(OP_State) != OBJSTATE_BeingConstructed) &&
            (!checkFree || !itr->second->IsLocked()))
            entityIds.push_back(itr->first);
    }
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::NotifyMessegeSent(Message* pMsg)
{
    switch (pMsg->TypeId())
    {
    case MSG_EntityRenegade:
        OnEntityRenegade(pMsg);
        break;

    case MSG_EntityCreate:
        OnEntityCreate(pMsg);
        break;

    case MSG_EntityDestroy:
        OnEntityDestroy(pMsg);
        break;

    case MSG_EntityShow:
        OnEntityShow(pMsg);
        break;

    case MSG_EntityHide:
        OnEntityHide(pMsg);
        break;
    }
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::OnEntityCreate(Message* pMsg)
{
    TID entityId;
    EntityCreateMessage *pCreateMsg = nullptr;

    pCreateMsg = (EntityCreateMessage*)pMsg;

    if (pCreateMsg->Data()->OwnerId == m_type)
    {
        entityId = pCreateMsg->Data()->EntityId;

        GameEntity* pEntity = nullptr;

        if (!m_entities.Contains(entityId))
        {
            LogInfo("Entity %d does not exist in Player %s units, will fetch it from game", entityId, Enums[m_type]);
            pEntity = FetchEntity(entityId);
            _ASSERTE(pEntity);
            m_entities[entityId] = pEntity;
            pEntity->CacheAttributes();
        }
        else
        {
            LogInfo("Entity %d already exist in Player %s units, won't fetch it from game", entityId, Enums[m_type]);
            pEntity = GetEntity(entityId);
        }

        g_IMSysMgr.RegisterGameObj(entityId, m_type);

        LogInfo("[%s] %s created at %s",
            Enums[m_type], pEntity->ToString(true).c_str(), pEntity->GetPosition().ToString().c_str());
    }
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::OnEntityDestroy(Message* pMsg)
{
    EntityDestroyMessage *pDestroyMsg = nullptr;
    GameEntity *pEntity = nullptr;
    TID entityId;

    pDestroyMsg = (EntityDestroyMessage*)pMsg;

    if (pDestroyMsg->Data()->OwnerId == m_type)
    {
        entityId = pDestroyMsg->Data()->EntityId;
        _ASSERTE(m_entities.Contains(entityId));
        pEntity = GetEntity(entityId);
        pDestroyMsg->Data()->EntityType = pEntity->TypeId();
        _ASSERTE(pEntity);
        m_entities.erase(entityId);

        LogInfo("[%s] %s destroyed",
            Enums[m_type], pEntity->ToString(true).c_str());

        g_IMSysMgr.UnregisterGameObj(entityId);

        Toolbox::MemoryClean(pEntity);
    }
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::OnEntityRenegade(Message* pMsg)
{
    EntityCreateMessage *pRenMsg = nullptr;
    GameEntity *pEntity = nullptr;
    TID entityId;

    pRenMsg = (EntityCreateMessage*)pMsg;

    entityId = pRenMsg->Data()->EntityId;

    // I am the unit new owner
    if (pRenMsg->Data()->OwnerId == m_type)
    {
        if (!m_entities.Contains(entityId))
        {
            pEntity = FetchEntity(entityId);
            _ASSERTE(pEntity);
            m_entities[entityId] = pEntity;
        }
        else
        {
            pEntity = GetEntity(entityId);
        }

        LogInfo("[%s] %s renegaded TO me",
            Enums[m_type], pEntity->ToString(true).c_str());

        g_IMSysMgr.RegisterGameObj(entityId, m_type);
    }
    // Used to be my unit, but it is not anymore
    else if (pRenMsg->Data()->OwnerId != m_type && m_entities.Contains(entityId))
    {
        pEntity = GetEntity(entityId);
        _ASSERTE(pEntity);

        m_entities.erase(entityId);

        LogInfo("[%s] %s renegaded from me",
            Enums[m_type], pEntity->ToString(true).c_str());

        g_IMSysMgr.UnregisterGameObj(entityId);

        Toolbox::MemoryClean(pEntity);
    }
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::OnEntityShow(Message* pMsg)
{
    GameEntity *pEntity = nullptr;
    TID entityId;
    EntityShowMessage *pShowMsg = nullptr;

    pShowMsg = (EntityCreateMessage*)pMsg;

    if (pShowMsg->Data()->OwnerId == m_type)
    {
        entityId = pShowMsg->Data()->EntityId;

        if (m_entities.Contains(entityId))
        {
            LogInfo("Entity %d already exist in Player %s units", entityId, Enums[m_type]);
            pEntity = GetEntity(entityId);
        }
        else
        {
            LogInfo("Entity %d does not exist in Player %s units, adding it", entityId, Enums[m_type]);
            pEntity = FetchEntity(entityId);
            _ASSERTE(pEntity);
            m_entities[entityId] = pEntity;
        }

        LogInfo("[%s] %s showed at %s",
            Enums[m_type], pEntity->ToString(true).c_str(), pEntity->GetPosition().ToString().c_str());

        g_IMSysMgr.RegisterGameObj(entityId, pShowMsg->Data()->OwnerId);
    }
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::OnEntityHide(Message* pMsg)
{
    TID entityId;
    EntityShowMessage *pShowMsg = nullptr;

    pShowMsg = (EntityCreateMessage*)pMsg;

    if (pShowMsg->Data()->OwnerId == m_type)
    {
        entityId = pShowMsg->Data()->EntityId;

        if (m_entities.Contains(entityId))
        {
            LogInfo("Entity %d exist in Player %s units, caching its attributes before being invisible", entityId, Enums[m_type]);

            auto pEntity = GetEntity(entityId);
            pEntity->CacheAttributes();

            LogInfo("[%s] %s hidden from %s",
                Enums[m_type], pEntity->ToString(true).c_str(), pEntity->GetPosition().ToString().c_str());
        }
    }
}
//////////////////////////////////////////////////////////////////////////
MapArea GamePlayer::GetColonyMapArea()
{
    // We didn't calculate player colony center yet ?
    if (m_colonyCenter.IsNull())
    {
        GameEntity *pPlayerBase = nullptr;
        EntityList playerBases;

        GetBases(playerBases);

        // Player has at least one base, then we use the first one
        // Note that player having many bases it not supported by the engine
        if (!playerBases.empty())
            pPlayerBase = GetEntity(playerBases[0]);
        // No base! This is weird but for the case, we will select the first unit position as the player colony center
        else
        {
            EntityList    playerEntities;

            Entities(playerEntities);
            // This can't happen, If the player has no entities, then he must be losing
            _ASSERTE(!playerEntities.empty());

            pPlayerBase = GetEntity(playerEntities[0]);
        }

        GameType *pGameType = g_Game->GetEntityType(Race()->GetBaseType());
        _ASSERTE(pGameType);

        m_colonyCenter = MapArea(
            Vector2(pPlayerBase->P(OP_Left), pPlayerBase->P(OP_Top)),
            pGameType->P(TP_Width),
            pGameType->P(TP_Height));
    }

    return m_colonyCenter;
}
//////////////////////////////////////////////////////////////////////////
void GamePlayer::SetOffline(RtsGame* pBelongingGame)
{
    g_MessagePump->UnregisterForAllMessages(this);

    m_pResources->SetOffline(pBelongingGame);
    m_pTechTree->SetOffline(pBelongingGame);

    m_isOnline = false;
}
//////////////////////////////////////////////////////////////////////////
int GamePlayer::Attr(PlayerAttribute attribute)
{
    int amount = 0;
    EntityTypeProperty classAttribute;

    switch (attribute)
    {
    case PATTR_AlliedAttackersTotalHP:
        classAttribute = TP_MaxHp;
        break;

    case PATTR_AlliedAttackersTotalDamage:
        classAttribute = TP_GroundAttack;
        break;

    default:
        _ASSERTE(!"Inavlid player attribute!");
        classAttribute = TP_START;
        break;
    }

    for (auto pair : m_entities)
    {
        const GameType* pGameType = pair.second->Type();

        if (!pGameType->P(TP_IsWorker) &&
            !pGameType->P(TP_IsBuilding))
        {
            bool isReady = g_Assist.IsEntityObjectReady(pair.first);
            if (isReady)
            {
                amount += pGameType->P(classAttribute);
            }
        }
    }

    return amount;
}
//////////////////////////////////////////////////////////////////////////
int GamePlayer::CountEntityTypes(_In_ EntityTypeProperty attr, _In_ int val) const
{
    int count = 0;;

    for (auto entityEntry : m_entities)
    {
        if (entityEntry.second->Type()->P(attr) == val)
            ++count;
    }

    return count;
}