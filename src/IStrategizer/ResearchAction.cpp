#include "ResearchAction.h"

#include <cassert>
#include "Vector2.h"
#include "OnlineCaseBasedPlannerEx.h"
#include "AbstractAdapter.h"
#include "CellFeature.h"
#include "CaseBasedReasonerEx.h"
#include "DataMessage.h"
#include "EngineAssist.h"
#include "RtsGame.h"
#include "GamePlayer.h"
#include "GameTechTree.h"
#include "GameType.h"
#include "GameEntity.h"
#include "EntityClassExist.h"
#include "ResearchDone.h"
#include "GameResearch.h"

using namespace IStrategizer;
using namespace std;

const unsigned MaxPrepTime = 2000;
// MaxExecTime should be deduced from the research being researched
const unsigned MaxExecTime = 5000;

ResearchAction::ResearchAction() : Action(ACTIONEX_Research, MaxPrepTime, MaxExecTime)
{
	_params[PARAM_ResearchId] = RESEARCH_START;
	CellFeature::Null().To(_params);
}
//----------------------------------------------------------------------------------------------
ResearchAction::ResearchAction(const PlanStepParameters& p_parameters)
: Action(ACTIONEX_Research, p_parameters, MaxPrepTime, MaxExecTime)
{

}
//----------------------------------------------------------------------------------------------
bool ResearchAction::AliveConditionsSatisfied(RtsGame& game)
{
	bool researcherExists = g_Assist.DoesEntityObjectExist(m_researcherId);

	if (!researcherExists)
	{
		ConditionEx* failedCondition = new EntityClassExist(PLAYER_Self, m_researcherType, 1);
		m_history.Add(ESTATE_Failed, failedCondition);
	}

	return researcherExists;
}
//----------------------------------------------------------------------------------------------
bool ResearchAction::SuccessConditionsSatisfied(RtsGame& game)
{
	return game.Self()->TechTree()->ResearchDone((ResearchType)_params[PARAM_ResearchId]);
}
//----------------------------------------------------------------------------------------------
bool ResearchAction::Execute(RtsGame& game, const WorldClock& p_clock)
{
	// FIXME: because we don't have a goal for Research for now, we can use the action as a goal
	// at the same time, by not issuing the research action if it is already done
	if (game.Self()->TechTree()->ResearchDone((ResearchType)_params[PARAM_ResearchId]))
		return true;

	ResearchType researchType = (ResearchType)_params[PARAM_ResearchId];
	GameEntity *pGameResearcher;
	AbstractAdapter *pAdapter = g_OnlineCaseBasedPlanner->Reasoner()->Adapter();

	// Adapt researcher
	m_researcherId = pAdapter->AdaptBuildingForResearch(researchType);

	// Issue research order
	pGameResearcher = game.Self()->GetEntity(m_researcherId);
	_ASSERTE(pGameResearcher);

	return pGameResearcher->Research(researchType);
}
//----------------------------------------------------------------------------------------------
void ResearchAction::InitializePostConditions()
{
	vector<Expression*> m_terms;
	m_terms.push_back(new ResearchDone(PLAYER_Self, (ResearchType)_params[PARAM_ResearchId]));
	_postCondition = new And(m_terms);
}
//----------------------------------------------------------------------------------------------
void ResearchAction::InitializePreConditions()
{
	ResearchType researchType = (ResearchType)_params[PARAM_ResearchId];
	m_researcherType = g_Game->GetResearch(researchType)->SourceEntity();
	vector<Expression*> m_terms;

	m_terms.push_back(new EntityClassExist(PLAYER_Self, m_researcherType, OBJSTATE_Idle, true));
	g_Assist.GetPrerequisites(researchType, PLAYER_Self, m_terms);
	_preCondition = new And(m_terms);
}
//----------------------------------------------------------------------------------------------
bool ResearchAction::Equals(PlanStepEx* p_planStep)
{
	return StepTypeId() == p_planStep->StepTypeId() &&
		_params[PARAM_ResearchId] == p_planStep->Parameter(PARAM_ResearchId);
}