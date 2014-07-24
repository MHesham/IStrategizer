#ifndef STARCRAFTSTRATEGYSELECTOR_H
#define STARCRAFTSTRATEGYSELECTOR_H

#include "StrategySelector.h"
#include "StackFSM.h"

namespace IStrategizer
{
#define STRATEGY_TVP_Marines_Rush "TvP-Marines-Rush"
#define STRATEGY_TVP_Vultures_Rush "TvP-Vultures-Rush"

    class StarCraftStrategySelector : public StrategySelector
    {
    public:
        StackFSMPtr SelectArmyMacro(_In_ const Army& pArmy) const
        {
            DEBUG_THROW(NotImplementedException(XcptHere));
        }

        StackFSMPtr SelectEntityMicro(_In_ TID entityId) const
        {
            DEBUG_THROW(NotImplementedException(XcptHere));
        }

        void SelectGameOpening(_Out_ PlanStepParameters& trainArmyParams) const;
        TID SelectScout() const;
    };
}

#endif // STARCRAFTSTRATEGYSELECTOR_H