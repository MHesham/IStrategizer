#ifndef STRATEGYSELECTOR_H
#define STRATEGYSELECTOR_H

#include "StackFSM.h"

namespace IStrategizer
{
    class RtsGame;
    class EntityController;
    class ArmyController;

    struct Strategy
    {
        int Id;
        std::string Name;
    };

    class StrategySelector : public EngineObject
    {
    public:
        StrategySelector() :
            m_currStrategy({ -1, "" })
        {}

        virtual void SelectGameOpening() = 0;
        virtual void SelectNextStrategy() = 0;
        virtual bool IsGoodTimeToPush() = 0;
        virtual bool IsGoodTimeToScout() = 0;
        virtual StackFSMPtr SelectMicroLogic(_In_ ArmyController* armyCtrlr, _In_ EntityController* pController) const = 0;

        const Strategy& CurrStrategy() const { return m_currStrategy; }
        const PlanStepParameters& CurrStrategyGoalParams() const { return m_currStrategyGoalParams; }

    protected:
        Strategy m_currStrategy;
        PlanStepParameters m_currStrategyGoalParams;
    };

    typedef std::shared_ptr<StrategySelector> StrategySelectorPtr;
}

#endif // STRATEGYSELECTOR_H