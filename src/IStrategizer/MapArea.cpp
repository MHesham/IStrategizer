#include "MapArea.h"
#include "OccupanceDataIM.h"

using namespace IStrategizer;
using namespace std;

MapArea& MapArea::Null()
{
    static MapArea nullptrArea(Vector2::Null(), -1, -1);

    return nullptrArea;
}
//////////////////////////////////////////////////////////////////////////
bool MapArea::IsNull()
{
    return m_pos.IsNull() &&
        m_width == -1 &&
        m_height == -1;
}
//////////////////////////////////////////////////////////////////////////
bool MapArea::Acquire()
{
    OccupanceDataIM *pIM = (OccupanceDataIM*)g_IMSysMgr.GetIM(IM_BuildingData);
    _ASSERTE(pIM);

    return pIM->ReserveArea(m_pos, m_width, m_height);
}
//////////////////////////////////////////////////////////////////////////
bool MapArea::Release()
{
    OccupanceDataIM *pIM = (OccupanceDataIM*)g_IMSysMgr.GetIM(IM_BuildingData);
    _ASSERTE(pIM);

    return pIM->FreeArea(m_pos, m_width, m_height);
}
//////////////////////////////////////////////////////////////////////////
string MapArea::ToString() const
{
    char str[256];
    sprintf_s(str, "MapArea(%dx%d,<%d,%d>)", m_width, m_height, m_pos.X, m_pos.Y);
    return str;
}

