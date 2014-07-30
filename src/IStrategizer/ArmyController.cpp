#include "ArmyController.h"
#include "MessagePump.h"
#include "DataMessage.h"
#include "RtsGame.h"
#include "GamePlayer.h"
#include "GameType.h"
#include "GameEntity.h"
#include "EngineAssist.h"
#include "MathHelper.h"
#include "EntityFSM.h"
#include "ArmyFSM.h"

using namespace IStrategizer;
using namespace std;

ArmyController::ArmyController(StrategySelectorPtr pConsultant) :
m_pConsultant(pConsultant),
m_targetEntityId(INVALID_TID),
m_singleTargetPos(Vector2::Inf()),
m_isFormationInOrder(false)
{
    g_MessagePump->RegisterForMessage(MSG_EntityDestroy, this);

    m_pLogic = StackFSMPtr(new IdleArmyFSM(this));
    m_pLogic->Reset();
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::ControlArmy()
{
    ReleaseArmy();

    // For now, army controls only current free healthy attackers
    for (auto& entityR : g_Game->Self()->Entities())
    {
        ControlEntity(entityR.first);
    }

    m_pLogic->Reset();
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::ReleaseArmy()
{
    while (!m_entities.empty())
    {
        ReleaseEntity(m_entities.begin()->first);
    }
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::NotifyMessegeSent(_In_ Message* pMsg)
{
    if (pMsg->TypeId() == MSG_EntityDestroy)
    {
        EntityDestroyMessage* pDestroyMsg = static_cast<EntityDestroyMessage*>(pMsg);
        _ASSERTE(pDestroyMsg && pDestroyMsg->Data());

        if (pDestroyMsg->Data()->OwnerId == PLAYER_Self)
        {
            OnEntityDestroyed(pDestroyMsg->Data()->EntityId);
        }
    }
}
//////////////////////////////////////////////////////////////////////////
ArmyGroup ArmyController::Classify(const GameType* pType)
{
    if (pType->P(TP_IsWorker))
        return ARMGRP_Worker;
    if (!pType->P(TP_IsAttacker))
        return ARMGRP_Supporter;
    else if (pType->P(TP_IsMelee))
        return ARMGRP_MeleelAttacker;
    else
        return ARMGRP_RangedAttacker;
}
//////////////////////////////////////////////////////////////////////////
bool ArmyController::HasType(EntityClassType type)
{
    for (auto& entityR : m_entities)
    {
        if (entityR.second->Entity()->TypeId() == type)
            return true;
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::ReleaseEntity(_In_ TID entityId)
{
    if (m_entities.count(entityId) > 0)
    {
        m_entities[entityId]->ReleaseEntity();
        m_entities.erase(entityId);
        auto pEntity = g_Game->Self()->GetEntity(entityId);

        if (pEntity)
            LogInfo("Release %s from army", pEntity->ToString().c_str());
        else
            LogInfo("Released non-existing entity %d from army, to control %d entities", entityId, m_entities.size());
    }
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::Update()
{
    if (!IsControllingArmy())
        return;

    CalcCetner();
    CalcTargetEntity();
    //CalcIsInOrder();

    m_pLogic->Update();

    for (auto& entityR : m_entities)
    {
        auto pEntityCtrlr = entityR.second;
        pEntityCtrlr->Update();
    }

    for (auto fleeingId : m_currFramefleeingEntities)
    {
        m_entities.erase(fleeingId);
    }

    m_currFramefleeingEntities.clear();
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::DefendArea(_In_ Vector2 pos)
{
    // Set the logic input position to defend
    TargetPosition(pos);

    m_pLogic = StackFSMPtr(new GuardArmyFSM(this));
    m_pLogic->Reset();
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::CalcCetner()
{
    if (m_entities.empty())
    {
        m_center = Vector2::Inf();
        return;
    }

    m_center = Vector2::Zero();

    for (auto& entityR : m_entities)
    {
        m_center += entityR.second->Entity()->GetPosition();
    }

    m_center /= (int)m_entities.size();
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::CalcTargetEntity()
{
    EntityList enemies;

    Vector2 selfPos = Center();
    Vector2 otherPos = Vector2::Inf();
    Circle2 sightArea = SightArea();

    m_closestEnemyInSight.clear();

    for (auto& entityR : g_Game->Enemy()->Entities())
    {
        otherPos = entityR.second->GetPosition();
        int dist = selfPos.Distance(otherPos);

        if (entityR.second->Exists() &&
            entityR.second->P(OP_IsVisible) &&
            dist < SightAreaRadius)
        {
            m_closestEnemyInSight.insert(make_pair(dist, entityR.first));
        }
    }

    if (m_closestEnemyInSight.empty())
    {
        m_targetEntityId = INVALID_TID;
    }
    else if (m_targetEntityId != m_closestEnemyInSight.begin()->second)
    {
        m_targetEntityId = m_closestEnemyInSight.begin()->second;
        LogInfo("New Enemy target choosen %s", g_Game->Enemy()->GetEntity(m_targetEntityId)->ToString(true).c_str());
    }
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::CalcIsFormationInOrder()
{
    if (m_entities.empty())
    {
        m_isFormationInOrder = false;
        return;
    }

    m_isFormationInOrder = true;
    auto orderArea = FocusArea();

    for (auto& entityR : m_entities)
    {
        // Fast return as soon as we see astray units out of focus area
        if (!orderArea.IsInside(entityR.second->Entity()->GetPosition()))
        {
            m_isFormationInOrder = false;
            return;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
bool ArmyController::IsInOrder(const EntityControllersMap& entities, _In_ Vector2 pos)
{
    if (entities.empty())
    {
        return false;
    }

    Circle2 orderArea(pos, FocusAreaRadius);

    for (auto& entityR : entities)
    {
        // Fast return as soon as we see astray units out of focus area
        if (!orderArea.IsInside(entityR.second->Entity()->GetPosition()))
        {
            return false;
        }
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////
bool ArmyController::CanControl(_In_ const GameEntity* pEntity)
{
    _ASSERTE(pEntity != nullptr);

    auto pEntityType = pEntity->Type();

    return pEntity->Exists() &&
        !pEntityType->P(TP_IsWorker) &&
        !pEntityType->P(TP_IsBuilding) &&
        pEntity->P(OP_State) != OBJSTATE_BeingConstructed &&
        !pEntity->IsLocked() &&
        !EntityController::IsOnCriticalHP(pEntity);
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::ControlEntity(_In_ TID entityId)
{
    auto pEntity = g_Game->Self()->GetEntity(entityId);

    if (CanControl(pEntity))
    {
        auto pController = EntityControllerPtr(new EntityController(this));

        // We can't reach here if we already control the entity
        // All controlled entities are locked by us
        _ASSERTE(m_entities.count(entityId) == 0);

        m_entities[entityId] = pController;
        pController->ControlEntity(entityId);
        pController->HardResetLogic();

        LogInfo("Added %s to army, to control %d entities", pEntity->ToString().c_str(), m_entities.size());
    }
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::OnEntityDestroyed(_In_ TID entityId)
{
    ReleaseEntity(entityId);
}
//////////////////////////////////////////////////////////////////////////
void ArmyController::OnEntityFleeing(_In_ TID entityId)
{
    m_currFramefleeingEntities.insert(entityId);
}
//////////////////////////////////////////////////////////////////////////
