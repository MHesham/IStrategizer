#ifndef DEPLOYFSMSTATE
#define DEPLOYFSMSTATE

#include "FSMState.h"
#include "EngineData.h"
#include "RtsGame.h"
#include "GamePlayer.h"
#include "GameType.h"
#include "GameEntity.h"
#include "EngineAssist.h"
#include "MessagePump.h"

namespace IStrategizer
{
    template<class TController, class TControllerTraits = ControllerTraits<TController>>
    class DeployFSMState : public FSMState<TController, ControllerTraits<TController>>
    {
    public:
        DeployFSMState(TController controller, int distance, BattleStateType nextState) : 
            m_distance(distance),
            m_targetPosition(Vector2::Null()),
            m_nextState(nextState),
            FSMState(Deploy, controller) {}

        void Enter(RtsGame& game, const WorldClock& clock)
        {
            m_inRange = false;
            m_targetPosition = Vector2::Null();
        }

        void Update(RtsGame& game, const WorldClock& clock)
        {
            ControllerTraits<TController>::ConstType battle = m_controller;
            TID currentTargetId = TControllerTraits::NextTarget(battle);
            Army* pArmy = TControllerTraits::GetArmy(battle);

            
            GameEntity* pTarget = game.Enemy()->GetEntity(currentTargetId);
            Vector2 targetPosition = pTarget->GetPosition();

            if (m_targetPosition != targetPosition)
            {
                m_targetPosition = targetPosition;

                for (TID attackerId : pArmy->Entities())
                {
                    GameEntity* pAttacker = game.Self()->GetEntity(attackerId);
                    pAttacker->Move(m_targetPosition);
                }
            }
            else
            {
                if (m_targetPosition.Distance(pArmy->Center()) <= m_distance)
                {
                    m_inRange = true;
                    for (TID attackerId : pArmy->Entities())
                    {
                        GameEntity* pAttacker = game.Self()->GetEntity(attackerId);
                        pAttacker->Stop();
                    }
                }
            }
        }

        int CheckTransitions(RtsGame& game, const WorldClock& clock)
        {
            ControllerTraits<TController>::ConstType battle = m_controller;
            Army* pArmy = TControllerTraits::GetArmy(battle);

            if (pArmy->Empty())
            {
                return Finished;
            }

            return m_inRange ? m_nextState : Deploy;
        }

    private:
        int m_distance;
        bool m_inRange;
        Vector2 m_targetPosition;
        BattleStateType m_nextState;
    };
}

#endif // DEPLOYFSMSTATE
