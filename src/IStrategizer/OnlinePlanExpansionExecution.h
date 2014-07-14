#ifndef ONLINEPLANEXPANSIONEXECUTION_H
#define ONLINEPLANEXPANSIONEXECUTION_H

#include "EngineObject.h"
#include "WorldClock.h"
#include "GoalEx.h"
#include "OlcbpPlanNodeData.h"
#include "CaseEx.h"

namespace IStrategizer
{
    enum PlayerType;
    class CaseEx;
    class GoalEx;
    class CaseBasedReasonerEx;

    class OnlinePlanExpansionExecution : public EngineObject
    {
    public:
        typedef unsigned GoalTypeID;
        typedef IOlcbpPlan::NodeValue CaseNodeValue;
        typedef IOlcbpPlan::NodeValue ClonedCaseNodeValue;

		// Time is counted as number of game frames
		const unsigned GoalSleepTime = 500;
		const unsigned GoalMaxSleepsCount = 3;
		const unsigned ActionSleepTime = 500;
		const unsigned ActionMaxSleepsCount = 3;
		
		OnlinePlanExpansionExecution(_In_ GoalEx* pInitialGoal, _In_ CaseBasedReasonerEx* pCbReasoner);
        ~OnlinePlanExpansionExecution();

        void Update(_In_ const WorldClock& clock);
        void NotifyMessegeSent(_In_ Message* pMessage);
        void StartPlanning();
        void RootGoal(GoalEx* pGoal) { _ASSERTE(pGoal); m_pRootGoal = pGoal; }
        const IOlcbpPlan* Plan() const { return &*m_pOlcbpPlan; }
        IOlcbpPlan* Plan() { return &*m_pOlcbpPlan; }
        ConstOlcbpPlanContextRef GetContext() const { return m_planContext; }

    private:
        bool IsGoalNode(_In_ IOlcbpPlan::NodeID nodeId) const { return BELONG(GoalType, m_pOlcbpPlan->GetNode(nodeId)->StepTypeId()); }
        bool IsActionNode(_In_ IOlcbpPlan::NodeID nodeId) const { return BELONG(ActionType, m_pOlcbpPlan->GetNode(nodeId)->StepTypeId()); }
        bool IsNodeOpen(_In_ IOlcbpPlan::NodeID nodeId) const { return GetNodeData(nodeId).IsOpen == true; }
        bool IsNodeReady(_In_ IOlcbpPlan::NodeID nodeId) const { return GetNodeData(nodeId).WaitOnParentsCount == 0; }
        bool IsNodeDone(_In_ IOlcbpPlan::NodeID nodeId) const { auto state = m_pOlcbpPlan->GetNode(nodeId)->GetState(); return state == ESTATE_Succeeded || state == ESTATE_Failed; }
        bool IsCaseTried(_In_ IOlcbpPlan::NodeID nodeId, _In_ CaseEx* pCase) const { return GetNodeData(nodeId).TriedCases.count(pCase) > 0; }

        OlcbpPlanNodeData& GetNodeData(_In_ IOlcbpPlan::NodeID nodeId) { _ASSERTE(m_nodeData.count(nodeId) > 0); return m_nodeData[nodeId]; }
        const OlcbpPlanNodeData& GetNodeData(_In_ IOlcbpPlan::NodeID nodeId) const { _ASSERTE(m_nodeData.count(nodeId) > 0); return m_nodeData.at(nodeId); }
        CaseEx* GetLastCaseForGoalNode(_In_ IOlcbpPlan::NodeID nodeId) const { return GetNodeData(nodeId).BelongingCase; }
        void GetSnippetOrphanNodes(_In_ IOlcbpPlan::NodeID snippetRootId, _Out_ IOlcbpPlan::NodeSerializedSet& orphans);
        void SetGoalNodeBelongingCase(_In_ IOlcbpPlan::NodeID nodeId, _In_ CaseEx* pCase) { GetNodeData(nodeId).BelongingCase = pCase; }
        void SetNodeSatisfyingGoal(_In_ IOlcbpPlan::NodeID nodeId, _In_ IOlcbpPlan::NodeID goalNodeId) { GetNodeData(nodeId).SatisfyingGoal = goalNodeId; }

        void ExpandGoal(IOlcbpPlan::NodeID goalNode, CaseEx* pCase);
        void UpdateBelongingSubplanChildrenWithParentReadiness(_In_ IOlcbpPlan::NodeID nodeId);
        bool DestroyGoalSnippetIfExist(_In_ IOlcbpPlan::NodeID planGoalNodeId);
        bool HasExecutingAction(IOlcbpPlan::NodeID nodeId);
        void AddReadyChildrenToUpdateQueue(_In_ IOlcbpPlan::NodeID nodeId, _Inout_ IOlcbpPlan::NodeQueue &updateQ);
        void UpdateActionNode(_In_ IOlcbpPlan::NodeID currentNode, _In_ const WorldClock& clock);
        void UpdateGoalNode(_In_ IOlcbpPlan::NodeID currentNode, _In_ const WorldClock& clock);
        void LinkNodes(_In_ IOlcbpPlan::NodeID srcNodeId, _In_ IOlcbpPlan::NodeID dstNodeId) { m_pOlcbpPlan->AddEdge(srcNodeId, dstNodeId); }
        void UnlinkNodes(_In_ IOlcbpPlan::NodeID srcNodeId, _In_ IOlcbpPlan::NodeID dstNodeId) { m_pOlcbpPlan->RemoveEdge(srcNodeId, dstNodeId); }
        void ComputeFreshSnippetWaitOnParentsCount(_In_ IOlcbpPlan::NodeID subGraphRootId);
        void UpdateHistory(CaseEx* pCase);
        void MarkCaseAsTried(_In_ IOlcbpPlan::NodeID nodeId, _In_ CaseEx* pCase);
        void OpenNode(_In_ IOlcbpPlan::NodeID nodeId) { LogInfo("Opening node %s", m_pOlcbpPlan->GetNode(nodeId)->ToString().c_str()); GetNodeData(nodeId).IsOpen = true; }
        void CloseNode(_In_ IOlcbpPlan::NodeID nodeId) { LogInfo("Closing node %s", m_pOlcbpPlan->GetNode(nodeId)->ToString().c_str()); GetNodeData(nodeId).IsOpen = false; }
        void GetReachableReadyNodes(_Out_ IOlcbpPlan::NodeQueue& actionQ, _Out_ IOlcbpPlan::NodeQueue& goalQ);
        int AdaptSnippet(_In_ IOlcbpPlan::NodeID snippetRootGoalId);
        void GetAncestorSatisfyingGoals(_In_ IOlcbpPlan::NodeID nodeId, _Out_ IOlcbpPlan::NodeSet& ancestors) const;

        void OnGoalNodeSucceeded(_In_ IOlcbpPlan::NodeID nodeId);
        void OnGoalNodeFailed(_In_ IOlcbpPlan::NodeID nodeId);
        void OnActionNodeSucceeded(_In_ IOlcbpPlan::NodeID nodeId);
        void OnActionNodeFailed(_In_ IOlcbpPlan::NodeID nodeId);
        void OnNodeDone(_In_ IOlcbpPlan::NodeID nodeId);
        void RemoveExecutingNode(IOlcbpPlan::NodeID nodeId);
        void AddExecutingNode(IOlcbpPlan::NodeID currentNode);
        void ClearPlan();

        CaseBasedReasonerEx *m_pCbReasoner;
        OlcbpPlanNodeDataMap m_nodeData;
        IOlcbpPlan::NodeID m_planRootNodeId;
        std::shared_ptr<IOlcbpPlan> m_pOlcbpPlan;
        bool m_planStructureChangedThisFrame;
        std::map<CaseNodeValue, ClonedCaseNodeValue> m_clonedNodesMapping;
        GoalType m_rootGoalType;
        GoalEx* m_pRootGoal;
        IOlcbpPlan::NodeSet m_activeGoalSet;
        OlcbpPlanContext m_planContext;
        std::map<IOlcbpPlan::NodeID, std::set<IOlcbpPlan::NodeID>> m_executingActions;
		std::set<IOlcbpPlan::NodeID> m_succeededSnippets;
    };
}

#endif // ONLINEPLANEXPANSIONEXECUTION_H