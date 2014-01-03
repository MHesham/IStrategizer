#include "ResearchAction.h"
#include "BuildAction.h"
#include "BuildActionEx.h"
#include "TrainAction.h"
#include "AttackGroundAction.h"
#include "AttackEntityAction.h"
#include "MoveEntityAction.h"
#include "ActionFactory.h"
#include "CellFeature.h"

using namespace IStrategizer;

Action* ActionFactory::GetAction(ActionType p_actionType, const PlanStepParameters& p_paramaters, bool p_initConditions){
    Action* m_action = nullptr;

	switch(p_actionType)
    {
    case ACTIONEX_Research:
        m_action = new ResearchAction(p_paramaters);
		
		break;

	case ACTIONEX_BuildEx:
		m_action = new BuildActionEx(p_paramaters);
		break;

	case ACTIONEX_Train:
        return new TrainAction(p_paramaters);
		break;

	case ACTIONEX_AttackGround:
        m_action = new AttackGroundAction(p_paramaters);
		break;

	case ACTIONEX_AttackEntity:
        return new AttackEntityAction(p_paramaters);
		break;

    case ACTIONEX_MoveEntity:
        return new MoveEntityAction(p_paramaters);
        break;

	default:
		throw exception("Action not supported");
    }

	if(p_initConditions)
		m_action->InitializeConditions();

	return m_action;
}
//-------------------------------------------------------------------------------------------------------
Action* ActionFactory::GetAction(ActionType p_actionType, bool p_initConditions)
{
	Action* m_action = nullptr;

	switch(p_actionType)
	{
	case ACTIONEX_Research:
		m_action = new ResearchAction();

		break;

	case ACTIONEX_BuildEx:
		m_action = new BuildActionEx();
		break;

	case ACTIONEX_Train:
		m_action = new TrainAction();
		break;

	case ACTIONEX_AttackGround:
		m_action = new AttackGroundAction();
		break;

	case ACTIONEX_AttackEntity:
		m_action = new AttackEntityAction();
		break;

    case ACTIONEX_MoveEntity:
        m_action = new MoveEntityAction();
        break;

	default:
		throw exception("Action not supported");
	}

	if(p_initConditions)
		m_action->InitializeConditions();

	return m_action;
}