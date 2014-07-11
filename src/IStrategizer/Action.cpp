#include "Action.h"
#include "And.h"
#include "Logger.h"
#include "CompositeExpression.h"
#include "RtsGame.h"
#include "MathHelper.h"
#include "CellFeature.h"
#include "EngineDefs.h"

using namespace std;
using namespace IStrategizer;

Action::Action(ActionType p_actionType, unsigned p_maxPrepTime, unsigned p_maxExecTrialTime, unsigned p_maxExecTime)
: PlanStepEx(p_actionType, ESTATE_END), _preCondition(nullptr)
{
    _stateTimeout[INDEX(ESTATE_NotPrepared, ExecutionStateType)] = p_maxPrepTime;
    _stateTimeout[INDEX(ESTATE_Executing, ExecutionStateType)] = p_maxExecTrialTime;
}
//////////////////////////////////////////////////////////////////////////
Action::Action(ActionType p_actionType, const PlanStepParameters& p_parameters, unsigned p_maxPrepTime,  unsigned p_maxExecTrialTime, unsigned p_maxExecTime)
: PlanStepEx(p_actionType, ESTATE_END, p_parameters), _preCondition(nullptr)
{
    _stateTimeout[INDEX(ESTATE_NotPrepared, ExecutionStateType)] = p_maxPrepTime;
    _stateTimeout[INDEX(ESTATE_Executing, ExecutionStateType)] = p_maxExecTrialTime;
}
//////////////////////////////////////////////////////////////////////////
Action::~Action()
{
	SAFE_DELETE(_preCondition);
}
//////////////////////////////////////////////////////////////////////////
void Action::State(ExecutionStateType p_state, RtsGame& game, const WorldClock& p_clock)
{
    PlanStepEx::State(p_state, game, p_clock);

    switch (p_state)
    {
    case ESTATE_Succeeded:
        OnSucccess(game, p_clock);
        break;
    case ESTATE_Failed:
        OnFailure(game, p_clock);
        break;
    }
}
bool Action::PreconditionsSatisfied(RtsGame& game)
{
    if (_preCondition == nullptr) { InitializeConditions(); }
    bool satisfied = _preCondition->Evaluate(game);

    return satisfied;
}
//////////////////////////////////////////////////////////////////////////
void Action::InitializeConditions()
{
    PlanStepEx::InitializeConditions();
    InitializePreConditions();
}
//////////////////////////////////////////////////////////////////////////
bool Action::Execute(RtsGame& game, const WorldClock& p_clock)
{
    _ASSERTE(PlanStepEx::State() == ESTATE_NotPrepared);

    LogInfo("Trying to execute action %s", ToString().c_str());
    return ExecuteAux(game, p_clock);
}
//////////////////////////////////////////////////////////////////////////
void Action::Reset(RtsGame& game, const WorldClock& p_clock)
{
    if (PlanStepEx::State() != ESTATE_NotPrepared)
        State(ESTATE_NotPrepared, game, p_clock);
}
//////////////////////////////////////////////////////////////////////////
void Action::UpdateAux(RtsGame& game, const WorldClock& p_clock)
{
    ExecutionStateType state = PlanStepEx::State();
    
    switch (state)
    {
    case ESTATE_NotPrepared:
        if (PreconditionsSatisfied(game))
        {
            LogInfo("Preconditions satisfied, trying to execute action %s", ToString().c_str());

            if (Execute(game, p_clock))
            {
                State(ESTATE_Executing, game, p_clock);
            }
            else
            {
                LogInfo("Execution failed for action '%s', do not fail and keep retrying", ToString().c_str());
                OnFailure(game, p_clock);
                // It's intentional to comment this block as some actions require retrying.
                //LogInfo("Execution failed for action %s", ToString().c_str());
                //State(ESTATE_Failed, game, p_clock);
            }
        }
        break;

    case ESTATE_Executing:
        if(AliveConditionsSatisfied(game))
        { 
            if (SuccessConditionsSatisfied(game))
            {
                State(ESTATE_Succeeded, game, p_clock);
                m_history.Add(ESTATE_Succeeded);
            }
        }
        else
        {
            LogInfo("%s alive conditions not satisfied", ToString().c_str());
            State(ESTATE_Failed, game, p_clock);
        }
        break;
    }
}
//////////////////////////////////////////////////////////////////////////
void Action::Copy(IClonable* p_dest)
{
    PlanStepEx::Copy(p_dest);

    Action* m_dest = static_cast<Action*>(p_dest);
    m_dest->_preCondition = _preCondition ? static_cast<CompositeExpression*>(_preCondition->Clone()) : nullptr;
}
//////////////////////////////////////////////////////////////////////////
unsigned Action::Hash(bool quantified) const
{
    auto& params = Parameters();
    // + 1 to include the StepTypeId since it is used as well in the hashing
    size_t numWords = (params.size() + 1);
    vector<int> str(numWords);

    PlanStepParameters dummyParams;
    CellFeature::Null().To(dummyParams);

    str.push_back(StepTypeId());
    for (auto& param : Parameters())
    {
        if (!quantified && param.first == PARAM_Amount)
            continue;

        // FIXME: for now we ignore cell features for simplicity
        if (dummyParams.count(param.first) > 0)
            continue;

        str.push_back(param.second);
    }

    unsigned h = MathHelper::SuperFastHash((const char*)&*str.cbegin(), str.size() * sizeof(int));
    return h;
}