#include "kingdom/game/g04/cTaskCursor.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"

namespace kingdom::game::g04
{

// DOL 0x801a7dd8
cTaskCursor::cTaskCursor(uint nPlayer) : m_nPlayer(nPlayer)
{
	SetName("g04::cTaskCursor");
	cKingdomAnita* a = pAnita(0u);
	a->RegistData("POINTER_01_ANITA");   // 0x5263e420
	a->SetAnimNo((int)(m_nPlayer * 2u), true);   // DAT_8037cdc8[player * 2] = 0, 2, 4, 6
	a->SetState(3u, false);
	m_nPlace = 2;
	m_nOldPlace = 2;
	m_fSlide = 0.0f;
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_GAME_LAYOUT"))   // 0xde890dc6
	{
		for (int p = 0; p < 4; p++)
			for (int i = 0; i < 4; i++)
			{
				m_nX[p][i] = csv.S32(p * 4 + i, 1, 0);
				m_nY[p][i] = csv.S32(p * 4 + i, 2, 0);
			}
		for (int i = 0; i < 4; i++)
		{
			m_nSingleX[i] = csv.S32(i + 0x10, 1, 0);
			m_nSingleY[i] = csv.S32(i + 0x10, 2, 0);
		}
	}
	nDrawPriority(0x80000007u);
}

// DOL 0x801a820c
bool cTaskCursor::Execute(cJetTask* pParam)
{
	if ((uint)(m_nState - 1) < 2u)
	{
		const bool single = KingdomGlobal::WaniWaniSys->nPlayers() == 1;
		const int p = (int)(m_nPlayer & 3u);
		const float nx = (float)(single ? m_nSingleX[m_nPlace & 3] : m_nX[p][m_nPlace & 3]);
		const float ny = (float)(single ? m_nSingleY[m_nPlace & 3] : m_nY[p][m_nPlace & 3]);
		const float ox = (float)(single ? m_nSingleX[m_nOldPlace & 3] : m_nX[p][m_nOldPlace & 3]);
		const float oy = (float)(single ? m_nSingleY[m_nOldPlace & 3] : m_nY[p][m_nOldPlace & 3]);
		m_fSlide += 0.2f;   // FLOAT_8047cc3c
		if (1.0f <= m_fSlide) m_fSlide = 1.0f;
		const float t = m_fSlide;
		// the render mode's fb width / efb height (640 x 480) halved (FLOAT_8047cc44 = 0.5)
		const float x = (1.0f - t) * -(640.0f * 0.5f - ox) + t * -(640.0f * 0.5f - nx);
		const float y = (1.0f - t) * (480.0f * 0.5f - oy) + t * (480.0f * 0.5f - ny);
		cKingdomAnita* a = pAnita(0u);
		Vector4 v = a->vTrans();
		v.x = x;
		v.y = y;
		a->vTrans() = v;
	}
	return cTaskAnita::Execute(pParam);   // 0x80014d58: the anitas advance
}

// DOL 0x801a843c
void cTaskCursor::Show()
{
	cKingdomAnita* a = pAnita(0u);
	a->SetAnimNo((int)(m_nPlayer * 2u), true);
	m_nState = 1;
	a->SetState(3u, true);
}

// DOL 0x801a84cc
void cTaskCursor::SetPlace(int place)
{
	if (m_nPlace == place) return;
	m_fSlide = 0.0f;
	m_nOldPlace = m_nPlace;
	m_nPlace = place;
}

// DOL 0x801a84ec
void cTaskCursor::Hide()
{
	m_nState = 0;
	pAnita(0u)->SetState(3u, false);
}

// DOL 0x801a8550
void cTaskCursor::Lunge()
{
	if (m_nState != 2) pAnita(0u)->SetAnimNo((int)(m_nPlayer * 2u + 1u), true);   // DAT_8037cdcc
	m_nState = 2;
}

}
