///> [Serializable]
#ifndef ENGINEDATA_H
#define ENGINEDATA_H

#include <vector>
#include <map>
#include <set>
#include "MetaData.h"

#ifndef SVECTOR_H
#include "SVector.h"
#endif

#ifndef USEROBJECT_H
#include "UserObject.h"
#endif

#include "AdjListDigraph.h"

namespace IStrategizer
{
    enum BattleStateType
    {
        // State for targeting an enemy unit and deploying the units there.
        Deploy,

        // State for performing attack action on the targeted unit.
        Attack,

        // State indicating that the battle is finished. Either all army entities or 
        // targeted enemy entities are destroyed
        Finished
    };

    class PlanStepEx;
    class ConditionEx;
    class GoalEx;

    typedef std::string TAction;
    typedef std::string TEntity;
    typedef std::string TUpgrade;

    typedef int TPlayer;
    typedef int TID;

    typedef std::set<TID> EntitySet;
    typedef std::vector<TID> EntityList;

    ///> alias=PlanStepParameters(map(pair(int,int)))
    typedef Serialization::SMap<ParameterType, int> PlanStepParameters;

    ///> alias=IOlcbpPlan(IDigraph(PlanStepEx*))
    typedef IDigraph<PlanStepEx*> IOlcbpPlan;

    ///> alias=ShallowFeaturesEx(vector(float))
    typedef Serialization::SVector<float> ShallowFeaturesEx;

    ///> alias=DeepFeaturesEx(vector(float))
    typedef Serialization::SVector<float> DeepFeaturesEx;

#define INVALID_TID -1
#define DONT_CARE -1
#define ExactSimilarity 100

    /********************************************************************* Header Annotations ************************************************************************/
#ifndef IN
#define IN __in
#endif

#ifndef OUT
#define OUT __out
#endif

#ifndef CheckReturn
#define CheckReturn __checkReturn
#endif
}

#endif // ENGINEDATA_H
