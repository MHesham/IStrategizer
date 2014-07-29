#include "EntityFSM.h"
#include "EntityController.h"
#include "GamePlayer.h"
#include "GameEntity.h"
#include "GameType.h"

using namespace IStrategizer;
using namespace std;

//////////////////////////////////////////////////////////////////////////
//
// States
//
void EntityState::Enter()
{
    LogInfo("%s -> Enter", ToString().c_str());

    auto pController = (EntityController*)m_pController;
    m_targetPos1 = pController->TargetPosition();
    m_targetEntity = pController->TargetEntity();
}
//////////////////////////////////////////////////////////////////////////
void EntityState::Exit()
{
    m_targetPos1 = Vector2::Inf();
    m_targetEntity = INVALID_TID;

    LogInfo("%s -> Exit", ToString().c_str());
}
//////////////////////////////////////////////////////////////////////////
void EntityState::Update()
{
    auto pController = (EntityController*)m_pController;
    pController->Entity()->DebugDrawTarget();

    g_Game->DebugDrawMapText(pController->Entity()->GetPosition(), ToString().c_str());
}
//////////////////////////////////////////////////////////////////////////
void ArriveEntityState::Update()
{
    EntityState::Update();

    g_Game->DebugDrawMapCircle(m_targetPos1, EntityController::PositionArriveRadius, GCLR_Yellow);

    if (g_Game->GameFrame() % 4 != 0)
        return;

    auto pController = (EntityController*)m_pController;
    if (!pController->Entity()->P(OP_IsMoving))
    {
        m_targetPos1 = Circle2(m_targetPos1, EntityController::PositionArriveRadius).RandomInside();
        pController->Entity()->Move(m_targetPos1);
    }
}
//////////////////////////////////////////////////////////////////////////
void FleeEntityState::Enter()
{
    EntityState::Enter();

    auto pController = (EntityController*)m_pController;
    pController->OnEntityFleeing();
}
//////////////////////////////////////////////////////////////////////////
void FleeEntityState::Update()
{
    EntityState::Update();

    g_Game->DebugDrawMapCircle(m_targetPos1, EntityController::PositionArriveRadius, GCLR_Yellow);

    if (g_Game->GameFrame() % 4 != 0)
        return;

    auto pController = (EntityController*)m_pController;
    if (!pController->Entity()->P(OP_IsMoving))
    {
        m_targetPos1 = Circle2(g_Game->Self()->StartLocation(), EntityController::PositionArriveRadius).RandomInside();
        pController->Entity()->Move(m_targetPos1);
    }
}
//////////////////////////////////////////////////////////////////////////
void AttackEntityState::Update()
{
    EntityState::Update();

    if (g_Game->GameFrame() % 4 != 0)
        return;

    auto pController = (EntityController*)m_pController;

    // When the unit can attack the target and it is not attacking or
    // it is attacking but a different target, then order it to attack our target
    if (m_targetEntity != INVALID_TID &&
        pController->Entity()->CanAttack(m_targetEntity) &&
        (!pController->Entity()->P(OP_IsAttacking) ||
        pController->Entity()->GetTargetId() != m_targetEntity))
    {
        auto pTarget = g_Game->GetEntity(m_targetEntity);
        if (pTarget->Exists())
        {
            pController->Entity()->AttackEntity(m_targetEntity);
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void AlarmEntityState::Update()
{
    EntityState::Update();

    auto pController = (EntityController*)m_pController;

    g_Game->DebugDrawMapLine(pController->Entity()->GetPosition(), m_targetPos1, GCLR_Orange);
}
//////////////////////////////////////////////////////////////////////////
void RetreatEntityState::Enter()
{
    EntityState::Enter();
}
//////////////////////////////////////////////////////////////////////////
void RetreatEntityState::Update()
{
    EntityState::Exit();
}
//////////////////////////////////////////////////////////////////////////
//
// Machines
//
void ScoutEntityFSM::CheckTransitions()
{
    auto pController = (EntityController*)m_pController;

    // Get target position in case the current state is using it
    auto pCurrState = static_pointer_cast<EntityState>(CurrentState());
    Vector2 targetPos = pCurrState->TargetPosition1();

    switch (pCurrState->TypeId())
    {
    case ArriveEntityState::TypeID:
        if (pController->IsOnCriticalHP() ||
            (m_scoutGoal == SCTGL_Explore && pController->IsTargetInSight(targetPos) && pController->ThreatAtTarget(targetPos)))
        {
            PopState();
            PushState(FleeEntityState::TypeID);
        }
        // Pick next target
        else if ((m_scoutGoal == SCTGL_Visit && pController->ArrivedAtTarget(targetPos)) ||
            (m_scoutGoal == SCTGL_Explore && pController->IsTargetInSight(targetPos) && !pController->ThreatAtTarget(targetPos)))
        {
            if (SetNextTargetPosition())
            {
                PopState();
                PushState(ArriveEntityState::TypeID);
            }
            else
            {
                PopState();
                PushState(FleeEntityState::TypeID);
            }
        }
        break;
    case FleeEntityState::TypeID:
        if (pController->ArrivedAtTarget(targetPos))
        {
            PopState();
            PushState(IdleEntityState::TypeID);
        }
        break;
    }
}
//////////////////////////////////////////////////////////////////////////
bool ScoutEntityFSM::SetNextTargetPosition()
{
    if (MoreTargetsExist())
    {
        ++m_currTargetPosIdx;
        auto pController = (EntityController*)m_pController;
        pController->TargetPosition(pController->MultiTargetPosition()[m_currTargetPosIdx]);

        return true;
    }
    else
        return false;
}
//////////////////////////////////////////////////////////////////////////
bool ScoutEntityFSM::MoreTargetsExist()
{
    auto pController = (EntityController*)m_pController;
    return m_currTargetPosIdx + 1 < (int)pController->MultiTargetPosition().size();
}
//////////////////////////////////////////////////////////////////////////
void ScoutEntityFSM::Reset()
{
    StackFSM::Reset();

    auto pController = (EntityController*)m_pController;
    pController->TargetPosition(pController->MultiTargetPosition()[0]);
}
//////////////////////////////////////////////////////////////////////////
void GuardEntityFSM::CheckTransitions()
{
    auto pController = (EntityController*)m_pController;

    // Get target position in case the current state is using it
    auto pCurrState = static_pointer_cast<EntityState>(CurrentState());
    Vector2 targetPos = pCurrState->TargetPosition1();

    switch (pCurrState->TypeId())
    {
    case FleeEntityState::TypeID:
        if (pController->ArrivedAtTarget(targetPos))
        {
            PopAllAndPushState(IdleEntityState::TypeID);
        }
        break;
    case ArriveEntityState::TypeID:
        if (pController->IsOnCriticalHP())
        {
            PopAllAndPushState(FleeEntityState::TypeID);
        }
        else if (pController->ArrivedAtTarget(targetPos))
        {
            PopState();
        }
        break;
    case AttackEntityState::TypeID:

        if (pController->IsOnCriticalHP())
        {
            PopAllAndPushState(FleeEntityState::TypeID);
        }
        else if (pController->TargetEntity() == INVALID_TID)
        {

        }
        else if (!pController->IsTargetInSight(pCurrState->TargetEntity()) ||
            !pController->EntityExists(pCurrState->TargetEntity()))
        {
            PopState();
        }
        break;
    case AlarmEntityState::TypeID:
        if (!pController->IsTargetInSight(targetPos))
        {
            PushState(ArriveEntityState::TypeID);
        }
        else if (pController->IsAnyEnemyTargetInSight())
        {
            PushState(AttackEntityState::TypeID);
        }
        break;
    }
}
//////////////////////////////////////////////////////////////////////////
void HintNRunEntityFSM::CheckTransitions()
{

}