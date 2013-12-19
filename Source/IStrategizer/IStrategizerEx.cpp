#include "IStrategizerEx.h"
#include "MessagePump.h"
#include "OnlineCaseBasedPlannerEx.h"
#include "LearningFromHumanDemonstrationEx.h"
#include "GoalFactory.h"
#include "CaseLearningHelper.h"
#include "IMSystemManager.h"
#include "DataMessage.h"
#include "IStrategizerException.h"

#ifndef SERIALIZATIONESSENTIALS_H
#include "SerializationEssentials.h"
#endif
#ifndef WORLDCLOCK_H
#include "WorldClock.h"
#endif
#ifndef RTSGAME_H
#include "RtsGame.h"
#endif
#ifndef DYNAMICCOMPONENT_H
#include "DynamicComponent.h"
#endif
#ifndef TOOLBOX_H
#include "Toolbox.h"
#endif
#include <cassert>
#include "IMSystemManager.h"

using namespace IStrategizer;

IStrategizerEx::IStrategizerEx(const IStrategizerParam &p_param, PhaseType p_phase, RtsGame* p_rtsGame) 
	: _self(PLAYER_Self),
	_enemy(PLAYER_Enemy),
	_phase(p_phase),
	_param(p_param),
	_caseLearning(nullptr),
	_planner(nullptr),
	_isFirstUpdate(true)
{
    g_Game = p_rtsGame;
    g_Game->Init();

	SerializationEssentials::Init();
	IStrategizer::Init();

	PlanStepParameters params;
	params[PARAM_PlayerId] = _self;
	params[PARAM_StrategyTypeId] = STRTYPE_EarlyTierRush;

	switch(_phase)
	{
	case PHASE_Online:
		_planner = new OnlineCaseBasedPlannerEx();
        _planner->Init(g_GoalFactory.GetGoal(GOALEX_WinGame, params));
		g_OnlineCaseBasedPlanner = _planner;
		break;

	case PHASE_Offline:
		_caseLearning = new LearningFromHumanDemonstrationEx(_self, _enemy);
		g_MessagePump.RegisterForMessage(MSG_GameEnd, this);
		break;
	}

	IMSysManagerParam imSysMgrParam;
	imSysMgrParam.BuildingDataIMCellSize = _param.BuildingDataIMCellSize;
	imSysMgrParam.GroundControlIMCellSize = _param.GroundControlIMCellSize;

	g_IMSysMgr.Init(imSysMgrParam);
	g_MessagePump.RegisterForMessage(MSG_EntityCreate, this);
	g_MessagePump.RegisterForMessage(MSG_EntityDestroy, this);

	DynamicComponent::RealTime(false);
	DynamicComponent::GlobalInvalidation(true);
	DynamicComponent::GlobalInvalidationInterval(2);
}
//---------------------------------------------------------------------------------------------
void IStrategizerEx::NotifyMessegeSent(Message* p_message)
{
	switch(p_message->MessageTypeID())
	{
	case MSG_GameStart:
		_clock.Reset();
	case MSG_GameEnd:
		if (_phase == PHASE_Offline)
		{
			OfflineLearning();
		}
		break;
	}
}
//--------------------------------------------------------------------------------
void IStrategizerEx::Update(unsigned p_gameCycle)
{
	try
	{
		if (_isFirstUpdate)
		{
			_clock.Reset();
			_isFirstUpdate = false;
		}

		_clock.Update(p_gameCycle);
		g_MessagePump.Update(_clock);

		if (p_gameCycle % _param.IMSysUpdateInterval == 0)
			g_IMSysMgr.Update(_clock);

		if (_phase == PHASE_Online)
			_planner->Update(_clock);
	}
	catch (IStrategizer::Exception &e)
	{
		e.To(cout);
	}
	catch (std::exception &e)
	{
		cout << "IStrategizer encountered unhandled std exception: " << e.what() << endl;
	}
}
//--------------------------------------------------------------------------------
void IStrategizerEx::OfflineLearning()
{
	// CaseBaseEx* m_learntCases = _caseLearning->CaseBaseAcquisition();
}
//----------------------------------------------------------------------------------------------
IStrategizerEx::~IStrategizerEx()
{
	g_IMSysMgr.Finalize();
    Toolbox::MemoryClean(_planner);
    Toolbox::MemoryClean(_caseLearning);
}