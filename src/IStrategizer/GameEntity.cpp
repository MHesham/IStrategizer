#include "GameEntity.h"

using namespace IStrategizer;

unsigned GameEntity::Distance(_In_ const GameEntity *pOther) const
{
	int dX = Attr(EOATTR_PosCenterX) - pOther->Attr(EOATTR_PosCenterX);
	int dY = Attr(EOATTR_PosCenterY) - pOther->Attr(EOATTR_PosCenterY);
	return (unsigned)sqrt((float)(unsigned)((dX * dX) + (dY * dY)));
}
