///> [Serializable]
#ifndef STARCRAFTENTITY_H
#define STARCRAFTENTITY_H

#ifndef GAMEENTITY_H
#include "GameEntity.h"
#endif

#include "BWAPI.h"

#define TilePositionFromUnitPosition(UnitPos) ((UnitPos) / 32)
#define UnitPositionFromTilePosition(TilePos) ((TilePos) * 32)
#define WalkPositionFromUnitPosition(UnitPos) ((UnitPos) / 8)
#define UnitPositionFromWalkPosition(TilePos) ((TilePos) * 8)

namespace IStrategizer
{
    enum ObjectStateType;
    enum EntityClassType;
    enum PlayerType;
    class RtsGame;

    ///> class=StarCraftEntity
    ///> parent=GameEntity
    class StarCraftEntity: public GameEntity
    {
		OBJECT_SERIALIZABLE_P(StarCraftEntity, GameEntity, &m_isOnline, &m_cachedAttr);
    public:
        StarCraftEntity() : GameEntity(INVALID_TID), m_pUnit(nullptr) {}
        StarCraftEntity(BWAPI::Unit p_unit);

        // Game Properties
        Vector2 Position() const;
        int P(EntityObjectProperty p_attrId) const;
        bool IsTraining(TID p_traineeId) const;
        bool IsNull() { return m_isOnline && m_pUnit == nullptr; }
        bool CanGather(TID resourceObjectId) const;
		std::string ToString(bool minimal = false) const;
        TID TargetId() const;
        Vector2 TargetPosition() const;
        bool Exists() const;
        bool CanAttack(_In_ TID targetId) const;
        bool CanRepair(_In_ TID entityId) const;
        bool CanReach(_In_ Vector2 dest) const;
        bool CanBuild(_In_ EntityClassType typeId) const;
        int LastCommandFrame() const;

        // Game Commands
        bool Research(IStrategizer::ResearchType p_researchId);
        bool Build(EntityClassType p_buildingClassId, Vector2 p_position);
        bool AttackGround(Vector2 p_position);
        bool AttackEntity(TID p_targetEntityObjectId);
        bool Train(EntityClassType p_entityClassId);
        bool Move(Vector2 p_position);
		bool GatherResourceEntity(TID p_resourceEntityObjectId);
		bool Stop();
		void CancelOrders();
		bool CanTrain(EntityClassType type);
        bool Follow(TID entityId);
        bool Repair(_In_ TID targetId);
        

        void SetOffline(RtsGame* pBelongingGame);

    protected:
        void DebugDrawMapLastGameError();

        ObjectStateType FetchState() const;
        ///> type=int
        bool m_isOnline;
        BWAPI::Unit m_pUnit;
        EntityClassType m_typeId;
        PlayerType m_ownerId;
    };
}

#endif // STARCRAFTENTITY_H
