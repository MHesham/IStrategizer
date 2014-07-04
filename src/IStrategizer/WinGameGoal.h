///> [Serializable]
#ifndef WINGAMEGOAL_H
#define WINGAMEGOAL_H

#include "GoalEx.h"
#include <vector>

namespace IStrategizer
{
    ///> class=WinGameGoal
    ///> parent=GoalEx
    class WinGameGoal : public GoalEx
    {
        OBJECT_SERIALIZABLE(WinGameGoal);

    public:
        WinGameGoal();
        WinGameGoal(const PlanStepParameters& p_parameters);
        std::vector<GoalEx*> GetSucceededInstances(RtsGame &game);
        bool Equals(PlanStepEx* p_planStep);
        bool Merge(PlanStepEx* planStep) { return false; }
        
    protected:
        void InitializePostConditions();
        bool SuccessConditionsSatisfied(RtsGame& game);
    };
}

#endif // WINGAMEGOAL_H
