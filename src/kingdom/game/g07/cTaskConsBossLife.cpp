#include "kingdom/game/g07/cTaskConsBossLife.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

cTaskConsBossLife::cTaskConsBossLife()
{
	Init();
}

cTaskConsBossLife::~cTaskConsBossLife()
{
	Free();
}

void cTaskConsBossLife::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

cTaskAnita* cTaskConsBossLife::Construct()
{
	return new cTaskConsBossLife();
}

void cTaskConsBossLife::Init()
{
	if (KingdomGlobal::MotosSys->IsBossStage())
	{
		m_nTimer = 0;
		int array[2] = {152, 253};
		for (uint num = 0u; num < 2; num++)
		{
			pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
			pAnita(num)->SetAnimNo(array[num]);
			Invisible(num);
		}
	}
}

void cTaskConsBossLife::Term()
{
}

bool cTaskConsBossLife::Execute(cJetTask* pParam)
{
	if (!KingdomGlobal::MotosSys->IsBossStage())
	{
		return cTaskConsoleBase::Execute(pParam);
	}
	switch (nStep())
	{
	case 0:
		Visible(0u);
		StepChange(1);
		if (KingdomGlobal::g_pMotosGame->IsFirstPlay())
		{
			// TODO(unity): new GameObject().AddComponent<CoroutineObject>().ExecCoroutine(WaitOneFramePlaySE());
			break;
		}
		StepChange(3);
		SetAnm(0u, 153);
		pAnita(0u)->SetState(2u, false);
		StepChange(3);
		pAnita(0u)->SetFrame(100.0f);
		break;
	case 1:
		if (IsAnmEnd(0u))
		{
			SetAnm(0u, 153);
			StepChange(2);
		}
		break;
	case 2:
		{
			if (IsAnmEnd(0u) || !KingdomGlobal::g_pMotosGame->IsFirstPlay() || KingdomGlobal::g_pMotosGame->nStep(0) == 1)
			{
				SetAnm(0u, 153);
				pAnita(0u)->SetState(2u, false);
				StepChange(3);
				pAnita(0u)->SetFrame(100.0f);
			}
			cTaskCharBoss* cTaskCharBoss3 = static_cast<cTaskCharBoss*>(KingdomGlobal::g_pMotosGame->pBoss());
			m_nOldHp = cTaskCharBoss3->nHp();
			break;
		}
	case 3:
		{
			cTaskCharBoss* cTaskCharBoss2 = static_cast<cTaskCharBoss*>(KingdomGlobal::g_pMotosGame->pBoss());
			float num = (float)cTaskCharBoss2->nHp() / (float)cTaskCharBoss2->nMaxHp() * 100.0f;
			if (num > 100.0f)
			{
				num = 100.0f;
			}
			else if (num < 0.0f)
			{
				num = 0.0f;
			}
			if (m_nOldHp != cTaskCharBoss2->nHp())
			{
				pAnita(1u)->SetFrame(0.0f);
				Visible(1u);
			}
			m_nOldHp = cTaskCharBoss2->nHp();
			pAnita(0u)->SetFrame(num);
			break;
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

IEnumerator* cTaskConsBossLife::WaitOneFramePlaySE()
{
	// TODO(unity): coroutine; original C# body:
	// 	yield return null;
	// 	yield return new WaitWhile(() => Constants.SYSTEM_IS_PAUSE());
	// 	SoundManager.SE.PlaySE(ESEKind.MOTOS_SND_SE031_BOSS_APPEAR1);
	return nullptr;
}

}
