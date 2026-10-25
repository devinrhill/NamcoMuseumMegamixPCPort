#include "kingdom/game/g07/cTaskConsItemMultiPlayIcon.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

cTaskConsItemMultiPlayIcon::cTaskConsItemMultiPlayIcon(uint PlayerNum)
{
	m_nPlayerNum = PlayerNum;
	Init();
}

cTaskConsItemMultiPlayIcon::~cTaskConsItemMultiPlayIcon()
{
	Term();
}

void cTaskConsItemMultiPlayIcon::Init()
{
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return;
	}
	CustomKingdomUtil::Assert(m_nPlayerNum < 4, "error!");
	int array[4] = {163, 166, 169, 172};
	int array2[4] = {12, 13, 14, 15};
	int array3[4] = {33, 34, 35, 36};
	int array4[4] = {28, 29, 30, 31};
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(0u)->SetAnimNo(array[m_nPlayerNum]);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, array2[m_nPlayerNum]);
	Invisible(0u);
	pAnita(5u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(5u)->SetAnimNo(175);
	pAnita(0u)->GetLayerPos(pAnita(5u)->vTrans(), m_nPlayerNum, 26);
	Invisible(5u);
	int array5[4] = {178, 181, 184, 187};
	pAnita(1u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(1u)->SetAnimNo(array5[m_nPlayerNum]);
	pAnita(0u)->GetLayerPos(pAnita(1u)->vTrans(), 0u, array4[m_nPlayerNum]);
	Invisible(1u);
	for (uint num = 0u; num < 3; num++)
	{
		uint num2 = 2 + num;
		pAnita(num2)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num2)->SetAnimNo(190);
		if (KingdomGlobal::MotosSys->nWinsRound(m_nPlayerNum) > num)
		{
			SetAnmDiffCurrent(num2, 195);
		}
		int obj[3] = {1, 0, 2};
		int num3 = 3;
		uint lay = (uint)(obj[num] + (long)num3 * (long)m_nPlayerNum);
		pAnita(0u)->GetLayerPos(pAnita(num2)->vTrans(), lay, 38);
		Invisible(num2);
	}
	m_pScoreNum = new cTaskConsItemNumber(6);
	m_pScoreNum->pExecNumber(m_nExecPlayerScore);
	m_pScoreNum->SetFlag(38u, true);
	m_pScoreNum->SetFlag(1u, false);
	Vector4 pos = Vector4();
	pAnita(0u)->GetLayerPos(pos, 0u, array3[m_nPlayerNum]);
	m_pScoreNum->X() = pos[0];
	m_pScoreNum->Y() = pos[1];
	ConnectChild(m_pScoreNum);
}

void cTaskConsItemMultiPlayIcon::Term()
{
}

bool cTaskConsItemMultiPlayIcon::Execute(cJetTask* pParam)
{
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return cTaskConsoleBase::Execute(pParam);
	}
	int num = KingdomGlobal::g_pMotosGame->nStep(0);
	CustomKingdomUtil::Assert(m_nPlayerNum < 4, "error");
	m_nExecPlayerScore->Value = KingdomGlobal::g_pMotosGame->nPoints((int)m_nPlayerNum);
	bool flag = true;
	if (KingdomGlobal::MotosSys->nWinsRound(m_nPlayerNum) != 0)
	{
		for (uint num2 = 0u; num2 < 4; num2++)
		{
			if (num2 != m_nPlayerNum && KingdomGlobal::MotosSys->nWinsRound(m_nPlayerNum) < KingdomGlobal::MotosSys->nWinsRound(num2))
			{
				flag = false;
				break;
			}
		}
	}
	else
	{
		flag = false;
	}
	if (num == 8 || num == 4 || num == 5)
	{
		for (uint num3 = 0u; num3 < 6; num3++)
		{
			Visible(num3);
		}
		int array[4] = {164, 167, 170, 173};
		AnimationLoopFromApear(0u, array[m_nPlayerNum]);
		int array2[4] = {179, 182, 185, 188};
		AnimationLoopFromApear(1u, array2[m_nPlayerNum]);
		AnimationLoopFromApear(5u, 176);
		if (!flag)
		{
			Invisible(5u);
		}
		m_pScoreNum->bExec() = true;
		for (uint num4 = 0u; num4 < 3; num4++)
		{
			uint set = 2 + num4;
			if ((long)KingdomGlobal::g_pMotosGame->pPlayers(m_nPlayerNum)->nSPCount() > (long)num4)
			{
				if (!IsAnmNoSame(set, 195) && !IsAnmNoSame(set, 192))
				{
					SetAnmDiffCurrent(set, 192);
				}
				else if (IsAnmNoSame(set, 192) && IsAnmEnd(set))
				{
					SetAnm(set, 195);
				}
			}
			else if (!IsAnmNoSame(set, 191) && !IsAnmNoSame(set, 196))
			{
				SetAnmDiffCurrent(set, 196);
			}
			else if (IsAnmNoSame(set, 196) && IsAnmEnd(set))
			{
				SetAnm(set, 191);
			}
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
