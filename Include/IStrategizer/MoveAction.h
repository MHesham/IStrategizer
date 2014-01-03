///> [Serializable]
#ifndef MOVEACTION_H
#define MOVEACTION_H
#include "Action.h"
#include "Vector2.h"

 namespace IStrategizer 
{
	class GameEntity;

	///> class=MoveAction
	///> parent=Action
	class MoveAction:public Action
	{
	public:
		OBJECT_SERIALIZABLE(MoveAction);

		MoveAction();
		MoveAction(const PlanStepParameters& p_parameters);
		void        Copy(IClonable* p_dest);
		bool		PreconditionsSatisfied();
		bool		AliveConditionsSatisfied();
		bool		SuccessConditionsSatisfied();

	protected:
		bool		ExecuteAux(const WorldClock& p_clock );
		void		InitializeAddressesAux() ;
		void		HandleMessage(Message* p_pMsg, bool& p_consumed);
		bool		IsEntityCloseToPosition(const TID  p_EntityId, const Vector2 p_position, int p_Epslon);

	private:
		TID			_EntityId;
		Vector2     _position;
		GameEntity*  _pEntity;
	};
}

#endif// ATTACKGROUNDACTION_H 