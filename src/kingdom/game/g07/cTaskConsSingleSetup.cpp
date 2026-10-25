#include "kingdom/game/g07/cTaskConsSingleSetup.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/game/g07/cTaskConsPacIconSingleSetting.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "MotosUIManager.hpp"
#include "MotosUIMotosBossText.hpp"
#include "MotosUIMotosStartText.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

const float cTaskConsSingleSetup::aHeight[3] = {-84.0f, -136.0f, -188.0f};
const int cTaskConsSingleSetup::nTikiAnm[4] = {0, 118, 121, 124};
const int cTaskConsSingleSetup::nAnmSelectItem[4] = {0, 119, 122, 125};
const int cTaskConsSingleSetup::nAnmNoSelectItem[4] = {0, 208, 209, 210};
const uint cTaskConsSingleSetup::nAnmBossHintWait[10] = {224u, 227u, 230u, 233u, 236u, 239u, 242u, 245u, 248u, 251u};
const int cTaskConsSingleSetup::nAnmBossHintOut[10] = {225, 228, 231, 234, 237, 240, 243, 246, 249, 252};
const uint cTaskConsSingleSetup::AnimNo[6] = {268u, 120u, 123u, 126u, 108u, 225u};

cTaskConsSingleSetup::cTaskConsSingleSetup()
{
	Init();
}

cTaskAnita* cTaskConsSingleSetup::Construct()
{
	return new cTaskConsSingleSetup();
}

void cTaskConsSingleSetup::Init()
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return;
	}
	m_sBgColor.nRgba = Constants::get_MO_SELECT_DEFAULT_COLOR_1P();
	m_pPacIcon = new cTaskConsPacIconSingleSetting();
	ConnectChild(m_pPacIcon);
	for (int i = 0; i < (int)std::size(m_aExecNumber); i++)
	{
		m_aExecNumber[i] = new Ref<int>(0);
	}
	Vector2 array[3] = {Vector2(-122.0f, -83.0f), Vector2(-122.0f, -135.0f), Vector2(-122.0f, -187.0f)};
	for (uint num = 0u; num < 3; num++)
	{
		m_pItemNumber[num] = new cTaskConsItemNumber(2);
		m_pItemNumber[num]->X() = array[num][0];
		m_pItemNumber[num]->Y() = array[num][1];
		m_pItemNumber[num]->pExecNumber(m_aExecNumber[num]);
		ConnectChild(m_pItemNumber[num]);
	}
	uint array2[6] = {266u, 118u, 121u, 124u, 105u, 223u};
	for (uint num2 = 0u; num2 < 5; num2++)
	{
		int num3 = ::Constants::ARRAY_SIZE(array2);
		CustomKingdomUtil::Assert((long)num2 < (long)num3, "error");
		pAnita(num2)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num2)->SetAnimNo((int)array2[num2]);
		pAnita(num2)->SetState(3u, false);
	}
	UnityUIStart = MotosUIManager::Get<MotosUIMotosStartText>(MotosUIManager::EUIIndex::MotosStartText);
	if (UnityUIStart != nullptr)
	{
		UnityUIStart->Init(MotosUIMotosStartText::EType::Single);
	}
	if (IsBossStageNo() == 0)
	{
		return;
	}
	uint array3[10] = {223u, 226u, 229u, 232u, 235u, 238u, 241u, 244u, 247u, 250u};
	for (uint num4 = 5u; num4 < 7; num4++)
	{
		int num5 = ::Constants::ARRAY_SIZE(array3);
		CustomKingdomUtil::Assert((long)num4 < (long)num5, "error");
		int num6 = (int)((IsBossStageNo() - 1) * 2 + (num4 - 5));
		pAnita(num4)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num4)->SetAnimNo((int)array3[num6]);
		pAnita(num4)->SetState(3u, false);
		if (num4 == 5)
		{
			pAnita(num4)->vTrans()[0] = 161.0f;
			pAnita(num4)->vTrans()[1] = 119.0f;
		}
		else
		{
			pAnita(num4)->vTrans()[0] = -150.0f;
			pAnita(num4)->vTrans()[1] = 119.0f;
		}
	}
	UnityUIBoss = MotosUIManager::Get<MotosUIMotosBossText>(MotosUIManager::EUIIndex::MotosBossText);
	if (UnityUIBoss != nullptr)
	{
		UnityUIBoss->Init((int)KingdomGlobal::MotosSys->nWorld());
	}
}

void cTaskConsSingleSetup::Term()
{
	if (nullptr != UnityUIStart && UnityUIStart->IsEnable())
	{
		if (UnityUIStart != nullptr)
		{
			UnityUIStart->SetEnable(false);
		}
	}
	if (nullptr != UnityUIBoss && UnityUIBoss->IsEnable())
	{
		if (UnityUIBoss != nullptr)
		{
			UnityUIBoss->SetEnable(false);
		}
	}
}

int cTaskConsSingleSetup::IsBossStageNo()
{
	return ((KingdomGlobal::MotosSys->nStageIdx() == 7u) ? 1 : ((KingdomGlobal::MotosSys->nStageIdx() == 14u) ? 2 : ((KingdomGlobal::MotosSys->nStageIdx() == 21u) ? 3 : ((KingdomGlobal::MotosSys->nStageIdx() == 28u) ? 4 : ((KingdomGlobal::MotosSys->nStageIdx() == 35u) ? 5 : (0))))));
}

bool cTaskConsSingleSetup::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return cTaskConsoleBase::Execute(pParam);
	}
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 1)
	{
		for (uint num = 0u; num < 3; num++)
		{
			m_pItemNumber[num]->bExec() = true;
		}
		for (uint num2 = 0u; num2 < 3; num2++)
		{
			uint num3 = num2 + 1;
			m_aExecNumber[num2]->Value = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum[num3] - KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nUseItem[num3];
		}
		int num4 = KingdomGlobal::g_pMotosGame->nStep(1);
		CustomKingdomUtil::Assert(0 <= num4 && num4 < ::Constants::ARRAY_SIZE(aHeight), "error");
		if (num4 > 3)
		{
			num4 = 3;
		}
		pAnita(0u)->vTrans()[0] = 0.0f;
		pAnita(0u)->vTrans()[1] = aHeight[num4];
		Visible(0u);
		for (uint num5 = 1u; num5 < 5; num5++)
		{
			if (num5 == 4 && (pAnita(num5)->GetState() & 1) != 0 && (pAnita(num5)->GetState() & 0x10000) != 0)
			{
				pAnita(4u)->SetAnimNo(106);
			}
			bool flag = false;
			if (num5 < 4 && ((IsAnmNoSame(num5, nTikiAnm[num5]) && IsAnmEnd(num5)) || !IsAnmNoSame(num5, nTikiAnm[num5])))
			{
				flag = true;
			}
			if (flag)
			{
				if (KingdomGlobal::g_pMotosGame->nStep(1) == num5 - 1)
				{
					SetAnmDiffCurrent(num5, nAnmSelectItem[num5]);
				}
				else
				{
					SetAnmDiffCurrent(num5, nAnmNoSelectItem[num5]);
				}
			}
			pAnita(num5)->SetState(3u);
			if (nullptr != UnityUIStart && !UnityUIStart->IsEnable())
			{
				if (UnityUIStart != nullptr)
				{
					UnityUIStart->SetEnable(true);
				}
			}
		}
		if (IsBossStageNo() != 0)
		{
			for (uint num6 = 5u; num6 < 7; num6++)
			{
				Visible(num6);
				if (IsAnmEnd(num6))
				{
					int num7 = (int)((IsBossStageNo() - 1) * 2 + (num6 - 5));
					SetAnmDiffCurrent(num6, (int)nAnmBossHintWait[num7]);
				}
			}
			if (nullptr != UnityUIBoss && !UnityUIBoss->IsEnable())
			{
				UnityUIBoss->SetEnable(true);
				UnityUIBoss->ResetBaseAnim();
				UnityUIBoss->PlayBaseAnim(MotosUIMotosBossText::EBaseAnim::In);
			}
		}
	}
	else if (KingdomGlobal::g_pMotosGame->nStep(0) != 2)
	{
		if (m_sBgColor.aCol_3 > 0)
		{
			m_sBgColor.aCol_3 -= 8;
		}
		for (uint num8 = 0u; num8 < 3; num8++)
		{
			m_pItemNumber[num8]->bExec() = false;
		}
		for (uint num9 = 0u; num9 < 5; num9++)
		{
			if ((pAnita(num9)->GetState() & 1) != 0)
			{
				int num10 = ::Constants::ARRAY_SIZE(AnimNo);
				CustomKingdomUtil::Assert((long)num9 < (long)num10, "error");
				if ((long)pAnita(num9)->GetAnimNo() != (long)AnimNo[num9])
				{
					pAnita(num9)->SetAnimNo((int)AnimNo[num9]);
				}
				if ((pAnita(num9)->GetState() & 0x10000) != 0)
				{
					pAnita(num9)->SetState(3u, false);
				}
			}
		}
		if (IsBossStageNo() != 0)
		{
			for (uint num11 = 5u; num11 < 7; num11++)
			{
				int num12 = (int)((IsBossStageNo() - 1) * 2 + (num11 - 5));
				SetAnmDiffCurrent(num11, nAnmBossHintOut[num12]);
			}
			if (UnityUIBoss != nullptr && UnityUIBoss->IsPlayBaseAnim(MotosUIMotosBossText::EBaseAnim::Loop))   // Unity UI is not ported: Get returns nullptr
			{
				if (UnityUIBoss != nullptr)
				{
					UnityUIBoss->PlayBaseAnim(MotosUIMotosBossText::EBaseAnim::Out);
				}
			}
			if (nullptr != UnityUIBoss && UnityUIBoss->IsEnable() && !UnityUIBoss->IsPlayBaseAnim(MotosUIMotosBossText::EBaseAnim::Loop) && !UnityUIBoss->IsPlayBaseAnim(MotosUIMotosBossText::EBaseAnim::Out))
			{
				if (UnityUIBoss != nullptr)
				{
					UnityUIBoss->SetEnable(false);
				}
			}
		}
		if (nullptr != UnityUIStart && UnityUIStart->IsEnable())
		{
			if (UnityUIStart != nullptr)
			{
				UnityUIStart->SetEnable(false);
			}
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

void cTaskConsSingleSetup::Draw(int nState, cJetTask* pParam)
{
	cTaskAnita::Draw(nState, pParam);
}

}
