///> [Serializable]

#ifndef GAMETECHTREE_H
#define GAMETECHTREE_H

#include <vector>
#include <map>
#include "EngineObject.h"

namespace IStrategizer
{
    enum ResearchType;
    class RtsGame;

    ///> class=GameTechTree
    class GameTechTree : public EngineObject
    {
		OBJECT_SERIALIZABLE(GameTechTree);
    public:
        virtual ~GameTechTree() {}
        virtual bool ResearchDone(ResearchType researchId) const = 0;
        virtual bool ResearchAvailable(ResearchType researchId) const = 0;
        virtual void SetOffline(RtsGame* pBelongingGame) = 0;
    };
}

#endif // GAMETECHTREE_H
