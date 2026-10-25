#include "DObjParam.hpp"

DObjParam::sObj* DObjParam::sObjParam::GetParam(int x, int y)
{
	auto it = std::find_if(m_sObj.begin(), m_sObj.end(), [&](sObj* v) { return v->m_pos[0] == x && v->m_pos[1] == y; });
	return (it != m_sObj.end()) ? *it : nullptr;
}
