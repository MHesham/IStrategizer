#include "BuildBaseGoal.h"
#include "And.h"
#include "False.h"

using namespace IStrategizer;

BuildBaseGoal::BuildBaseGoal() : GoalEx(GOALEX_BuildBase)
{
    _params[PARAM_BaseTypeId] = BASETYPE_START;
    vector<Expression*> m_terms;
    m_terms.push_back(new False);
    InitializeConditions();
}
//----------------------------------------------------------------------------------------------
BuildBaseGoal::BuildBaseGoal(const PlanStepParameters& p_parameters): GoalEx(GOALEX_BuildBase, p_parameters)
{
    vector<Expression*> m_terms;
    m_terms.push_back(new False);
    InitializeConditions();
}
//----------------------------------------------------------------------------------------------
void BuildBaseGoal::InitializePostConditions()
{
    //map<EntityClassType, int> m_baseTypeRequiredBuildings;
    //EntityClassType m_baseBuildingClassId;

    //g_Assist.GetRequiredBuildingsForBaseType(PLAYER_Self, (BaseType)_params[PARAM_BaseTypeId], m_baseTypeRequiredBuildings);
    //g_Assist.GetTireBaseBuildingId(PLAYER_Self, (BaseType)_params[PARAM_BaseTypeId], m_baseBuildingClassId);

    //m_baseTypeRequiredBuildings[m_baseBuildingClassId] = 1;

    vector<Expression*> m_termsPost;
    m_termsPost.push_back(new False);
    //m_termsPost.resize(m_baseTypeRequiredBuildings.size());

    //int m_index = 0;
    //for (map<EntityClassType, int>::const_iterator m_itr = m_baseTypeRequiredBuildings.begin(); m_itr != m_baseTypeRequiredBuildings.end(); m_itr++)
    // m_termsPost[m_index++] = new EntityClassExist(PLAYER_Self, (*m_itr).first, (*m_itr).second, true);

    _postCondition = new And(m_termsPost);
}
//----------------------------------------------------------------------------------------------
bool BuildBaseGoal::SuccessConditionsSatisfied(RtsGame* pRtsGame)
{
    return false;
}