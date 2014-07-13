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
        OBJECT_SERIALIZABLE_P(BuildInfrastructureGoal, GoalEx);

    public:
        BuildInfrastructureGoal();
        BuildInfrastructureGoal(const PlanStepParameters& p_parameters);
        std::vector<GoalEx*> GetSucceededInstances(RtsGame &game);
        void AdaptParameters(RtsGame& game);
        void HandleMessage(RtsGame& game, Message* p_msg, bool& p_consumed);
        bool Merge(PlanStepEx* planStep);
        bool Equals(PlanStepEx* p_planStep);

    protected:
        void InitializePostConditions();
        bool SuccessConditionsSatisfied(RtsGame& game);
        int GetAvailableSupplyBuildingsCount(RtsGame &game) const;

    private:
        int m_existingAmount;
        std::map<EntityClassType, int> m_createdBuildings;
        std::vector<GoalEx*> m_succeededInstaces;
    };
}

#endif // BUILDINFRASTRUCTUREGOAL_H