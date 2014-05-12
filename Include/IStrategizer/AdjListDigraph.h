///> [Serializable]
#ifndef ADJLISTDIGRAPH_H
#define ADJLISTDIGRAPH_H

#include <mutex>
#include "IDigraph.h"
#include "SMap.h"
#include "SPair.h"
#include "SSet.h"
#include "UserObject.h"

using namespace std;

namespace IStrategizer
{
    template<class T>
    struct AdjListDigraphNodeValueTraits
    {
        typedef T Type;
        typedef const T ConstType;
        static std::string ToString(ConstType& v) { return std::to_string(v); }
    };

    ///> class=AdjListDigraph(TNodeValue)
    ///> parent=IDigraph(TNodeValue)
    template<class TNodeValue, class TNodeValueTraits = AdjListDigraphNodeValueTraits<TNodeValue>>
    class AdjListDigraph :  public Serialization::UserObject, public IDigraph<TNodeValue>
    {
    public:
        ///> alias=NodeEntry(pair(NodeValue,NodeSet))
        typedef Serialization::SPair<NodeValue, NodeSet> NodeEntry;

        AdjListDigraph()
            : m_lastNodeId(0)
        {
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::AddNode
        // Description:	Add a new node to the Digraph without connecting it
        // Parameter: 	NodeValue val: A data value that is associated with the added node
        // Returns:   	NodeID:  A unique ID used to reference the added node
        // in further Digraph methods
        //************************************
        NodeID AddNode(const _In_ NodeValue& val)
        {
            m_adjList.insert(make_pair(++m_lastNodeId, MakePair(val, NodeSet())));

            return m_lastNodeId;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::AddNode
        // Description:	Add a new node to the Digraph without connecting it and identify it using
        // the passed in id instead of auto generating a new id
        // Parameter: 	NodeValue val: A data value that is associated with the added node
        // Parameter:   NodeID:  A unique ID used to reference the added node
        // in further Digraph methods
        //************************************
        NodeID AddNode(const _In_ NodeValue& val, NodeID id)
            throw(ItemAlreadyExistsException)
        {
            if (m_adjList.count(id) > 0)
                DEBUG_THROW(ItemAlreadyExistsException(XcptHere));

            m_adjList.insert(make_pair(id, MakePair(val, NodeSet())));
            m_lastNodeId = id;

            return id;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::RemoveNode
        // Description:	Disconnect a node from the digraph and removes it
        // Parameter: 	NodeID id: Unique ID to identify the node
        // Returns:   	void
        //************************************
        void RemoveNode(_In_ NodeID id)
            throw(ItemNotFoundException)
        {
            if (m_adjList.count(id) == 0)
                DEBUG_THROW(ItemNotFoundException(XcptHere));

            // Disconnect the node from graph nodes
            for (Serialization::SMap<NodeID, NodeEntry>::iterator nodeEntryItr = m_adjList.begin();
                nodeEntryItr != m_adjList.end(); ++nodeEntryItr)
            {
                NodeSet& adjNodes = nodeEntryItr->second.second;
                if (adjNodes.count(id) > 0)
                {
                    adjNodes.erase(id);
                }
            }

            m_adjList.erase(id);
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::AddEdge
        // Description:	Create a directed edge that goes from sourceNode to a destNode
        // Parameter: 	NodeID sourceNodeId: Unique ID to identify sourceNode
        // Parameter: 	NodeID destNodeId: Unique ID to identify destNode
        // Returns:   	void
        //************************************
        void AddEdge(_In_ NodeID sourceNodeId, _In_ NodeID destNodeId) 
            throw(ItemAlreadyExistsException)
        {
            if (m_adjList.count(sourceNodeId) == 0 ||
                m_adjList.count(destNodeId) == 0)
                DEBUG_THROW(ItemNotFoundException(XcptHere));

            m_adjList[sourceNodeId].second.insert(destNodeId);
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::RemoveEdge
        // Description:	Removes the edge that goes from sourceNode to destNode
        // Parameter: 	NodeID sourceNodeId: Unique ID to identify sourceNode
        // Parameter: 	NodeID destNodeId: Unique ID to identify destNode
        // Returns:   	void
        //************************************
        void RemoveEdge(_In_ NodeID sourceNodeId, _In_ NodeID destNodeId) 
            throw(ItemNotFoundException)
        {
            if (m_adjList.count(sourceNodeId) == 0 ||
                m_adjList.count(destNodeId) == 0)
                DEBUG_THROW(ItemNotFoundException(XcptHere));

            m_adjList[sourceNodeId].second.erase(destNodeId);
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::GetNode
        // Description:	Get the value associated with the node
        // Parameter: 	NodeID id: Unique ID to identify the node
        // Returns:   	IStrategizer::NodeValue
        //************************************
        NodeValue& GetNode(_In_ NodeID id)
            throw(ItemNotFoundException)
        {
            if (m_adjList.count(id) == 0)
                DEBUG_THROW(ItemNotFoundException(XcptHere));

            return m_adjList[id].first;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::GetNodes
        // Description:	Returns a set of all node ids inside the digraph
        // Returns:   	NodeSet
        //************************************
        NodeSet GetNodes() const
        {
            NodeSet nodes;

            for (auto nodeEntry : m_adjList)
            {
                nodes.insert(nodeEntry.first);
            }

            return nodes;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::Size
        // Description:	Returns the number of nodes inside the digraph
        // Returns:   	size_t
        //************************************
        size_t Size() const { return m_adjList.size(); }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::Clear
        // Description:	Delete all graph nodes and edges
        // Returns:   	void
        //************************************
        virtual void Clear() { m_adjList.clear(); }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::IsAdjacent
        // Description:	Check whether there is an edge that goes from sourceNode to destNode
        // Parameter: 	NodeID sourceNodeId: Unique ID to identify sourceNode
        // Parameter: 	NodeID destNodeId: Unique ID to identify destNode
        // Returns:   	bool
        //************************************
        bool IsAdjacent(_In_ NodeID sourceNodeId, _In_ NodeID destNodeId) const 
        {
            if (m_adjList.count(sourceNodeId) == 0 ||
                m_adjList.count(destNodeId) == 0)
                return false;

            return m_adjList.at(sourceNodeId).second.count(destNodeId) > 0;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::GetAdjacentNodes
        // Description:	Get all nodes that has an edge going from sourceNode to it
        // Parameter: 	NodeID sourceNodeId: Unique ID to identify sourceNode
        // Returns:   	NodeSet: A set of all node ids adjacent to sourceNode
        //************************************
        const NodeSet& GetAdjacentNodes(_In_ NodeID sourceNodeId) const 
            throw(ItemNotFoundException)
        {
            if (m_adjList.count(sourceNodeId) == 0)
                DEBUG_THROW(ItemNotFoundException(XcptHere));

            return m_adjList.at(sourceNodeId).second;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::SubGraphSubstitution
        // Description:	Replaces a sub-part of the IDigraph with the given TNodeValue provided.
        // Parameter: 	NodeList p_subGraphIndexes: The indexes describing the sub-part to replace.
        // Parameter:   TNodeValue p_substitute: The TNodeValue to replace the sub-part with.
        //************************************      
        void SubGraphSubstitution(NodeList subGraphIndexes, TNodeValue substitute)
        {
            NodeList    m_parents;
            NodeList    m_children;

            for each(NodeID m_subGraphIndex in subGraphIndexes)
            {
                NodeSet m_tempParents = GetParents(m_subGraphIndex, m_parents);
                m_parents.insert(m_parents.end(), m_tempParents.begin(), m_tempParents.end());

                NodeSet m_tempChildren = GetChildren(m_subGraphIndex, m_children);
                m_children.insert(m_children.end(), m_tempChildren.begin(), m_tempChildren.end());
            }

            NodeID m_nodeID = AddNode(substitute, substitute->Id());

            for each(NodeID m_parent in m_parents) AddEdge(m_parent, m_nodeID);
            for each(NodeID m_child in m_children) AddEdge(m_nodeID, m_child);
            for each(NodeID m_subGraphIndex in subGraphIndexes) RemoveNode(m_subGraphIndex);
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::GetOrphanNodes
        // Description:	Get all nodes that do not have an edge going from any node to them
        // Returns:   	NodeSet: A set of all orphan node ids
        //************************************
        NodeSet GetOrphanNodes() const
        {
            // Algorithm:
            // Assume all nodes are orphans
            // ..O is set of all graph nodes
            // ..For each node N in the graph G
            // ....For each adj node A in N adj nodes
            // ......Remove A from O
            NodeSet orphans = GetNodes();

            for (auto nodeEntry : m_adjList)
            {
                NodeSet& adjacents = nodeEntry.second.second;

                for (auto adjNodeId : adjacents)
                {
                    orphans.erase(adjNodeId);
                }
            }

            return orphans;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::GetLeafNodes
        // Description:	Get all nodes that do not have an edge going from them to any node
        // Returns:   	NodeSet
        //************************************
        NodeSet GetLeafNodes() const
        {
            NodeSet leaves;

            for (auto nodeEntry : m_adjList)
            {
                NodeSet& adjacents = nodeEntry.second.second;

                if (adjacents.empty())
                    leaves.insert(nodeEntry.first);
            }

            return leaves;
        }

        bool IsSubGraphOf(AdjListDigraph<TNodeValue, TNodeValueTraits>& p_parentGraph, NodeList& p_matchedIndexes)
        {
            NodeSet m_parentNodes;
            UnorderedNodeSet m_matchedNodes;

            for each (NodeID nodeID in p_parentGraph.GetNodes())
            {
                m_parentNodes.insert(nodeID);
            }

            NodeSet roots = GetOrphanNodes();

            bool result = MatchNodesAndChildren(roots, m_parentNodes, p_parentGraph, m_matchedNodes);
            p_matchedIndexes.insert(p_matchedIndexes.begin(), m_matchedNodes.begin(), m_matchedNodes.end());
            return result;
        }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::Lock
        // Description:	Locks the graph for exclusive read/write access by the caller thread
        // Returns:   	void
        //************************************
        void Lock() { m_lock.lock(); }

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::Unlock
        // Description:	Unlocks the graph acquisition by caller thread
        // Returns:   	void
        //************************************
        void Unlock() { m_lock.unlock(); }        OBJECT_SERIALIZABLE(AdjListDigraph);

        //************************************
        // IStrategizer::IDigraph<TNodeValue>::ToString
        // Description:	Visualize the digraph in a string representation that describes
        // its nodes and the connections between them
        // Returns:   	std::string
        //************************************
        std::string ToString() const
        {
            std::string str;

            NodeSet roots = GetOrphanNodes();
            str = "R = {";
            for (auto rootID : roots)
            {
                str += to_string(rootID);
                str += ',';
            }
            str += "}\n";

            for (auto nodeEntry : m_adjList)
            {
                if (m_adjList.count(nodeEntry.first) == 0)
                    DEBUG_THROW(ItemNotFoundException(XcptHere));

                AdjListDigraphNodeValueTraits<TNodeValue>::ConstType nodeVal = m_adjList.at(nodeEntry.first).first;
                str += TNodeValueTraits::ToString(nodeVal);
                str += '[';
                str += to_string(nodeEntry.first);
                str += ']';
                str += " -> ";

                NodeSet& adjacents = nodeEntry.second.second;

                for (auto adjNodeID : adjacents)
                {
                    if (m_adjList.count(adjNodeID) == 0)
                        DEBUG_THROW(ItemNotFoundException(XcptHere));

                    AdjListDigraphNodeValueTraits<TNodeValue>::ConstType adjNodeVal = m_adjList.at(adjNodeID).first;

                    str += TNodeValueTraits::ToString(adjNodeVal);
                    str += '[';
                    str += to_string(nodeEntry.first);
                    str += ']';
                    str += ',';
                }

                str += '\n';
            }

            return str;
        }

        OBJECT_MEMBERS(2 ,&m_lastNodeId, &m_adjList);

    private:
        ///> type=NodeID
        NodeID m_lastNodeId;
        ///> type=map(pair(NodeID,NodeEntry))
        Serialization::SMap<NodeID, NodeEntry> m_adjList;

        std::mutex m_lock;

        bool MatchNodesAndChildren(
            NodeSet& p_candidateNodes,
            NodeSet& p_parentNodes,
            AdjListDigraph<TNodeValue, TNodeValueTraits>& p_parentGraph,
            UnorderedNodeSet& p_matchedIndexes)
        {
            for each(NodeID m_candidateNodeId in p_candidateNodes)
            {
                bool m_foundMatch = false;

                for each(NodeID m_parentNodeId in p_parentNodes)
                {
                    if (p_matchedIndexes.find(m_parentNodeId) != p_matchedIndexes.end())
                    {
                        continue;
                    }

                    IComparable* m_candidateNode = dynamic_cast<IComparable*>(GetNode(m_candidateNodeId));
                    IComparable* m_parentNode = dynamic_cast<IComparable*>(p_parentGraph.GetNode(m_parentNodeId));

                    if (m_candidateNode->Compare(m_parentNode) == 0)
                    {
                        UnorderedNodeSet m_currentMatchedSubNodes;

                        NodeSet candidateNodeChildren = GetChildren(m_candidateNodeId);
                        NodeSet parentNodeChildren = p_parentGraph.GetChildren(m_parentNodeId);

                        if (MatchNodesAndChildren(candidateNodeChildren, parentNodeChildren, p_parentGraph, m_currentMatchedSubNodes))
                        {
                            m_currentMatchedSubNodes.insert(m_parentNodeId);

                            for each (NodeID m_matchedSubNode in m_currentMatchedSubNodes)
                            {
                                if (p_matchedIndexes.find(m_matchedSubNode) == p_matchedIndexes.end())
                                {
                                    p_matchedIndexes.insert(m_matchedSubNode);
                                }
                            }

                            m_foundMatch = true;
                            break;
                        }
                    }
                }

                if(m_foundMatch == false)
                {
                    return false;
                }
            }

            return true;
        }

        NodeSet GetChildren(NodeID p_nodeIndex, NodeList p_execluded = NodeList()) const
        {
            NodeSet m_children;

            for each(auto child in GetAdjacentNodes(p_nodeIndex))
            {
                if(find(p_execluded.begin(), p_execluded.end(), child) == p_execluded.end())
                {
                    m_children.insert(child);
                }
            }

            return m_children;
        }

        NodeSet GetParents(NodeID p_nodeIndex, NodeList p_execluded = NodeList()) const
        {
            if (m_adjList.count(p_nodeIndex) == 0)
                throw ItemNotFoundException(XcptHere);

            NodeSet m_parents;

            for each(auto parent in m_adjList)
            {
                if(find(p_execluded.begin(), p_execluded.end(), parent.first) == p_execluded.end()
                    && find(parent.second.second.begin(), parent.second.second.end(), p_nodeIndex) != parent.second.second.end())
                {
                    m_parents.insert(parent.first);
                }
            }

            return m_parents;
        }
    };
}

#endif