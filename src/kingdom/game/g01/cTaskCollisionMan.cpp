#include "kingdom/game/g01/cTaskCollisionMan.hpp"

#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include <algorithm>

#include "kingdom/NativeScene.hpp"
#include "kingdom/game/g01/cObject.hpp"

namespace kingdom::game::g01
{

// DOL 0x801b971c
cTaskCollisionMan::cTaskCollisionMan()
{
	SetName("g01::cTaskCollisionMan");
}

// DOL 0x800d41c8 (the DOL reserves 0x100 entries first, 0x800d395c)
void cTaskCollisionMan::Regist(cObject* p)
{
	if (std::find(m_Regist.begin(), m_Regist.end(), p) == m_Regist.end()) m_Regist.push_back(p);
}

void cTaskCollisionMan::Remove(cObject* p)
{
	m_Regist.erase(std::remove(m_Regist.begin(), m_Regist.end(), p), m_Regist.end());
}

// DOL 0x800d3b70: insertion sort by squared distance from the camera; objects behind the camera (dot(view, pos - eye) < 0) are skipped
void cTaskCollisionMan::HitCheckAll()
{
	m_Sorted.clear();
	float eye[3] = {}, aim[3] = {}, fovy = 0.0f;
	kingdom::NativeScene::GetCamera(eye, aim, fovy);
	const float view[3] = {aim[0] - eye[0], aim[1] - eye[1], aim[2] - eye[2]};
	for (cObject* p : m_Regist)
	{
		if (!p->bActive() || !p->bCollide()) continue;
		float dist = 0.0f;
		if (p->nKind() != 1)
		{
			const float* pos = p->vPos();
			const float d[3] = {pos[0] - eye[0], pos[1] - eye[1], pos[2] - eye[2]};
			if (view[0] * d[0] + view[1] * d[1] + view[2] * d[2] < 0.0f) continue;
			dist = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
		}
		auto it = std::find_if(m_Sorted.begin(), m_Sorted.end(), [dist](const auto& e) { return dist < e.second; });
		m_Sorted.insert(it, {p, dist});
	}
}

// DOL 0x800d40b8 (nothing while paused)
bool cTaskCollisionMan::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	(void)pParam;
	if (!m_bActive) return true;
	HitCheckAll();
	for (auto& a : m_Sorted)
		for (auto& b : m_Sorted)
		{
			if (a.first == b.first) continue;
			if (a.first->HitCheck(b.first))
			{
				a.first->HitCallback(b.first);
				b.first->HitCallback(a.first);
			}
		}
	return true;
}

}
