#ifndef ENGINEASSIST_H
#define ENGINEASSIST_H

#include "EngineData.h"
#include "Expression.h"
#include "WorldResources.h"
#include <vector>
#include <map>

#define ENTITY_DEST_ARRIVAL_THRESHOLD_DISTANCE 64

namespace IStrategizer
{
#define ERR_Success 0

    class Vector2;

    class EngineAssist
    {
    protected:
        EngineAssist() {}

    public:
        static EngineAssist& Instance() { static EngineAssist m_instance; return m_instance; }
        CheckReturn int ResearchDone(IN PlayerType p_playerIndex, IN ResearchType p_researchId);
        CheckReturn int GetResourceAmount(IN PlayerType p_playerIndex, IN ResourceType p_resourceId, OUT int& p_availableAmount);
        CheckReturn bool DoesEntityClassExist(IN std::pair<EntityClassType, unsigned> p_entityType, IN PlayerType p_playerType = PLAYER_Self);
        CheckReturn bool DoesEntityClassExist(IN const std::map<EntityClassType, unsigned> &p_entityTypes, IN PlayerType p_playerType = PLAYER_Self);
        CheckReturn bool DoesEntityObjectExist(IN TID p_entityObject, IN PlayerType p_playerType = PLAYER_Self);
        CheckReturn bool DoesEntityObjectExist(IN const std::vector<TID> &p_entityObjects, IN PlayerType p_playerType = PLAYER_Self);
        CheckReturn bool IsEntityCloseToPoint(IN const TID p_entityId, IN const Vector2& p_point, IN const unsigned p_maxDistance);
        CheckReturn void GetPrerequisites(IN int p_entityOrResearchType, IN PlayerType p_playerType, OUT std::vector<Expression*>& p_prerequisites);
        CheckReturn void GetPrerequisiteResources(IN int p_entityOrResearchType, IN PlayerType p_playerType, OUT WorldResources& p_resources);

#define g_Assist EngineAssist::Instance()
    };
}

#endif // ENGINEAPI
