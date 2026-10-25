#include "kingdom/game/g07/cTaskGimmickSwitch.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskPanel.hpp"
#include "kingdom/game/g07/cTaskPanelMan.hpp"
#include "kingdom/game/g07/sMOTOS_POS2D.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

cTaskGimmickSwitch::cTaskGimmickSwitch()
{
	Init();
}

cTaskGimmickBase* cTaskGimmickSwitch::Construct()
{
	return new cTaskGimmickSwitch();
}

void cTaskGimmickSwitch::Init()
{
	for (int i = 0; i < (int)std::size(m_apPanels); i++)
	{
		m_apPanels[i] = nullptr;
	}
	m_nMode = 0u;
}

void cTaskGimmickSwitch::Term()
{
	cTaskGimmickBase::Term();
}

bool cTaskGimmickSwitch::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_GIMMICK_SWITCH");
	string array[3] = {"TSA_GIMMICK_SWITCH", "CLA_GIMMICK_SWITCH", "CHA_GIMMICK_SWITCH"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	return true;
}

bool cTaskGimmickSwitch::Execute(cJetTask* pParam)
{
	switch (m_nMode)
	{
	case 0u:
		if (CustomKingdomUtil::IF(m_nAnimeKick & 1) && CustomKingdomUtil::IF(m_nFlag & 8))
		{
			m_nComa = (m_nComa + 1) % 25;
		}
		break;
	case 1u:
		if (++nTimer() > 20)
		{
			PanelExplode();
			m_nComa = 25;
			m_nMode = 2u;
		}
		break;
	case 2u:
		UpdateFrame();
		if (++m_nComa >= 40)
		{
			m_nComa = 39;
			m_nMode = 3u;
		}
		break;
	case 3u:
		UpdateFrame();
		break;
	}
	return true;
}

void cTaskGimmickSwitch::BootSwitch()
{
	if (m_nMode == 0)
	{
		nTimer() = 0u;
		m_nMode = 1u;
		Constants::SpawnEffect(35, m_vMTrans);
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE011_GIMMICK_SW);
	}
}

void cTaskGimmickSwitch::PanelExplode()
{
	uint num = 0u;
	int num2 = (int)m_nMapPos[0];
	int num3 = (int)m_nMapPos[1];
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	CustomKingdomUtil::Assert(cTaskPanelMan2 != nullptr, "nullpo");
	sMOTOS_POS2D array[4] = {[&]() { sMOTOS_POS2D _o6 = sMOTOS_POS2D(); _o6.pos[0] = 1; _o6.pos[1] = 0; return _o6; }(), [&]() { sMOTOS_POS2D _o7 = sMOTOS_POS2D(); _o7.pos[0] = -1; _o7.pos[1] = 0; return _o7; }(), [&]() { sMOTOS_POS2D _o8 = sMOTOS_POS2D(); _o8.pos[0] = 0; _o8.pos[1] = 1; return _o8; }(), [&]() { sMOTOS_POS2D _o9 = sMOTOS_POS2D(); _o9.pos[0] = 0; _o9.pos[1] = -1; return _o9; }()};
	for (uint num4 = 0u; num4 < 4; num4++)
	{
		int x = num2 + array[num4].pos[0];
		int z = num3 + array[num4].pos[1];
		Constants::PANEL_INFO* pANEL_INFO = cTaskPanelMan2->SearchPanelNo(x, z);
		if (pANEL_INFO != nullptr && pANEL_INFO->m_nKind == 4)
		{
			num |= 1;
		}
	}
	if (num != 0)
	{
		for (uint num5 = 0u; num5 < 4; num5++)
		{
			int x2 = num2 + array[num5].pos[0];
			int z2 = num3 + array[num5].pos[1];
			cTaskPanel* pTaskPanel = cTaskPanelMan2->SearchPanelNo(x2, z2)->m_pTaskPanel;
			CustomKingdomUtil::Assert(pTaskPanel != nullptr, "error");
			pTaskPanel->ExplodePanel();
		}
		return;
	}
	for (int i = 0; i < 256; i++)
	{
		Constants::PANEL_INFO* pANEL_INFO2 = cTaskPanelMan2->pPanel((uint)i);
		if (pANEL_INFO2 != nullptr && pANEL_INFO2->m_nKind == 4)
		{
			int num6 = (int)pANEL_INFO2->m_nMapPos[0];
			int num7 = (int)pANEL_INFO2->m_nMapPos[1];
			if (ToSwitchLength(pANEL_INFO2, cTaskPanelMan2->SearchPanelNo(num6 + 1, num7)) && ToSwitchLength(pANEL_INFO2, cTaskPanelMan2->SearchPanelNo(num6 - 1, num7)) && ToSwitchLength(pANEL_INFO2, cTaskPanelMan2->SearchPanelNo(num6, num7 + 1)) && ToSwitchLength(pANEL_INFO2, cTaskPanelMan2->SearchPanelNo(num6, num7 - 1)))
			{
				int num8 = (int)KingdomGlobal::g_pMotosGame->pMap()->nWidth();
				uint num9 = (uint)::Constants::ARRAY_SIZE(m_apPanels);
				CustomKingdomUtil::Assert((uint)(num6 + num7 * num8) < num9, "error");
				m_apPanels[num6 + num7 * num8] = pANEL_INFO2;
			}
		}
	}
	for (int j = 0; j < 256; j++)
	{
		if (m_apPanels[j] != nullptr)
		{
			m_apPanels[j]->m_pTaskPanel->ExplodePanel();
		}
	}
}

bool cTaskGimmickSwitch::ToSwitchLength(Constants::PANEL_INFO* p1, Constants::PANEL_INFO* p2)
{
	if (p2 == nullptr || p2->m_nKind != 4)
	{
		return true;
	}
	Vector2 v = Vector2();
	uint num = KingdomGlobal::g_pMotosGame->pMap()->nWidth();
	uint num2 = KingdomGlobal::g_pMotosGame->pMap()->nDepth();
	float num3 = m_nMapPos[0];
	float num4 = m_nMapPos[1];
	float value = (float)p1->m_nMapPos[0] - num3;
	float value2 = (float)p1->m_nMapPos[1] - num4;
	v[0] = value;
	v[1] = value2;
	float num5 = Constants::tspVector2Length(v);
	value = (float)p2->m_nMapPos[0] - num3;
	value2 = (float)p2->m_nMapPos[1] - num4;
	v[0] = value;
	v[1] = value2;
	float num6 = Constants::tspVector2Length(v);
	uint num7;
	uint num8;
	bool result;
	if (num5 < num6)
	{
		num7 = p2->m_nMapPos[0];
		num8 = p2->m_nMapPos[1];
		result = true;
	}
	else
	{
		num7 = p1->m_nMapPos[0];
		num8 = p1->m_nMapPos[1];
		result = false;
	}
	if (num7 < 0 || num7 >= num || num8 < 0 || num8 >= num2)
	{
		return false;
	}
	CustomKingdomUtil::Assert((long)(num7 + num8 * num) < (long)::Constants::ARRAY_SIZE(m_apPanels), "error");
	m_apPanels[num7 + num8 * num] = nullptr;
	return result;
}

}
