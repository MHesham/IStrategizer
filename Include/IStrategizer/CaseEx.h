///> [Serializable]
#ifndef CASEEX_H
#define CASEEX_H

#include "UserObject.h"
#include "PlanStepEx.h"
#include <vector>

namespace IStrategizer
{
    class GoalEx;
    class RtsGame;

    template<>
    struct AdjListDigraphNodeValueTraits<PlanStepEx*>
    {
        typedef PlanStepEx* Type;
        typedef const PlanStepEx* ConstType;
        static std::string ToString(const PlanStepEx* pStep) { return pStep->ToString(true); }
    };

    ///> alias=OlcbpPlan(AdjListDigraph(PlanStepEx*))
    typedef AdjListDigraph<PlanStepEx*> OlcbpPlan;

    ///> class=CaseEx
    class CaseEx : public Serialization::UserObject
    {
    public:
        CaseEx() 
            : m_pGoal(nullptr),
            m_pGameState(nullptr),
            m_trialCount(1),
            m_successCount(1),
            m_pPlan(nullptr) {};
        
        CaseEx(OlcbpPlan* pPlan, GoalEx* pGoal, RtsGame* pGameState, int trialCount, int successCount)
            : m_pGoal(pGoal),
            m_pGameState(nullptr),
            m_trialCount(trialCount),
            m_successCount(successCount),
            m_pPlan(pPlan) {}

        IOlcbpPlan* Plan() const { return m_pPlan; }
        GoalEx* Goal() const { return m_pGoal; }
        RtsGame* GameState() const { return m_pGameState; }
        int TrialCount() const { return m_trialCount; }
        int SuccessCount() const { return m_successCount; }
        void TrialCount(int val) { m_trialCount = val; }
        void SuccessCount(int val) { m_successCount = val; }

        OBJECT_SERIALIZABLE(CaseEx);
        OBJECT_MEMBERS(5, &m_pGoal, &m_pGameState, &m_trialCount, &m_successCount, &m_pPlan);

    private:
        ///> type=GoalEx*
        GoalEx* m_pGoal;
        ///> type=RtsGame*
        RtsGame* m_pGameState;
        ///> type=int
        int m_trialCount;
        ///> type=int
        int m_successCount;
        ///> type=OlcbpPlan*
        OlcbpPlan *m_pPlan;
    };
}

#endif // CASEEX_H
