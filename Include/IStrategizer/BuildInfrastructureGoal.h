///> [Serializable]
#ifndef BUILDINFRASTRUCTUREGOAL_H
#define BUILDINFRASTRUCTUREGOAL_H

#include "GoalEx.h"

namespace IStrategizer
{
    ///> class=BuildInfrastructureGoal
    ///> parent=GoalEx
    class BuildInfrastructureGoal : public GoalEx
    {
        OBJECT_SERIALIZABLE(BuildInfrastructureGoal);

    public:
        BuildInfrastructureGoal();
        BuildInfrastructureGoal(const PlanStepParameters& p_parameters);
        std::vector<GoalEx*> GetSucceededInstances(RtsGame &game);
        void AdaptParameters(RtsGame& game);

    protected:
        void InitializePostConditions();
        bool SuccessConditionsSatisfied(RtsGame& game);

    private:
        int m_existingAmount;
        bool m_firstLearningInstance;
    };
}

#endif // BUILDINFRASTRUCTUREGOAL_H