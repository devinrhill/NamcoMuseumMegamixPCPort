#include "kingdom/game/g07/cTaskConsItemMultiPlay.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemMultiPlayIcon.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

const float cTaskConsItemMultiPlay::PLAYER_WINNER_POS[4][2] = {};
const float cTaskConsItemMultiPlay::PLAYER_ICON_POS[4][2] = {{-281.0f, 183.0f}, {-281.0f, 103.0f}, {221.0f, 183.0f}, {221.0f, 103.0f}};

cTaskConsItemMultiPlay::cTaskConsItemMultiPlay()
{
	Init();
}

cTaskConsItemMultiPlay::~cTaskConsItemMultiPlay()
{
	Term();
}

cTaskAnita* cTaskConsItemMultiPlay::Construct()
{
	return new cTaskConsItemMultiPlay();
}

void cTaskConsItemMultiPlay::Init()
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		m_nTimer = (m_nDrawTimer = 0u);
		for (uint num = 0u; num < KingdomGlobal::MotosSys->pGetMatchingData()->nNum; num++)
		{
			m_pIcon[num] = new cTaskConsItemMultiPlayIcon(num);
			CustomKingdomUtil::Assert(m_pIcon[num] != nullptr, "nullpo");
			ConnectChild(m_pIcon[num]);
			m_pIcon[num]->X() = PLAYER_ICON_POS[num][0];
			m_pIcon[num]->Y() = PLAYER_ICON_POS[num][1];
		}
		pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(0u)->SetAnimNo(280);
		Invisible(0u);
		pAnita(1u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		Vector4 pos = Vector4();
		pAnita(0u)->GetLayerPos(pos, 2u, 9);
		pAnita(1u)->SetAnimNo(60);
		pAnita(1u)->vTrans()[0] = pos[0];
		pAnita(1u)->vTrans()[1] = pos[1];
		Invisible(1u);
		Vector4 pos2 = Vector4();
		pAnita(0u)->GetLayerPos(pos2, 0u, 9);
		m_pTimerNum[0] = new cTaskConsItemNumber(2);
		m_pTimerNum[0]->X() = pos2[0];
		m_pTimerNum[0]->Y() = pos2[1];
		m_pTimerNum[0]->pExecNumber(m_TimerSecconds);
		m_pTimerNum[0]->SetFlag(6u, true);
		m_pTimerNum[0]->SetFlag(1u, false);
		ConnectChild(m_pTimerNum[0]);
		Vector4 pos3 = Vector4();
		pAnita(0u)->GetLayerPos(pos3, 3u, 9);
		m_pTimerNum[1] = new cTaskConsItemNumber(1);
		m_pTimerNum[1]->X() = pos3[0];
		m_pTimerNum[1]->Y() = pos3[1];
		m_pTimerNum[1]->pExecNumber(m_TimerMin);
		m_pTimerNum[1]->SetFlag(6u, true);
		m_pTimerNum[1]->SetFlag(1u, false);
		ConnectChild(m_pTimerNum[1]);
		pAnita(2u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(2u)->SetAnimNo(149);
		Invisible(2u);
		pAnita(3u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(3u)->SetAnimNo(146);
		Invisible(3u);
	}
}

void cTaskConsItemMultiPlay::Term()
{
}

// DOL 0x800fdd34 [audited: per AUDIT_G03_G07 row]
bool cTaskConsItemMultiPlay::Execute(cJetTask* pParam)
{
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return cTaskConsoleBase::Execute(pParam);
	}
	m_TimerSecconds->Value = KingdomGlobal::MotosSys->nVsTimeCount() % 60;
	m_TimerMin->Value = KingdomGlobal::MotosSys->nVsTimeCount() / 60;
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 8)
	{
		for (uint num = 0u; num < 2; num++)
		{
			m_pTimerNum[num]->bExec() = true;
		}
		Visible(1u);
		Visible(0u);
		AnimationLoopFromApear(0u, 281);
		if (IsAnmEnd(1u))
		{
			SetAnmDiffCurrent(1u, 61);
		}
	}
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 4)
	{
		Visible(2u);
		if (IsAnmNoSame(2u, 149))
		{
			if (IsAnmEnd(2u))
			{
				SetAnm(2u, 150);
				m_nTimer = 0u;
			}
		}
		else if (IsAnmNoSame(2u, 150))
		{
			if (m_nTimer++ > 120)
			{
				SetAnm(2u, 151);
			}
		}
		else
		{
			IsAnmNoSame(2u, 151);
		}
		if (KingdomGlobal::g_pMotosGame->nStep(2) >= 2)
		{
		}
	}
	else if (KingdomGlobal::g_pMotosGame->nStep(0) == 5 && KingdomGlobal::g_pMotosGame->nStep(1) != 0 && KingdomGlobal::g_pMotosGame->nMultiBattleResult() == 2)
	{
		Visible(3u);
		if (IsAnmNoSame(3u, 146))
		{
			if (IsAnmEnd(3u))
			{
				SetAnm(3u, 147);
				m_nDrawTimer = 0u;
			}
		}
		else if (IsAnmNoSame(3u, 147))
		{
			if (m_nDrawTimer++ > 80)
			{
				SetAnm(3u, 148);
			}
		}
		else
		{
			IsAnmNoSame(3u, 148);
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
