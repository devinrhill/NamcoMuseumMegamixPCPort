#include "kingdom/game/g07/cTaskConsItemSinglePlay.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <iterator>
#include <limits>

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/game/g07/cTaskConsPacIcon.hpp"
#include "kingdom/game/g07/cTaskConsPlayTime.hpp"
#include "kingdom/game/g07/cTaskConsSingleClear.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

const int cTaskConsItemSinglePlay::nTikiAnm[3] = {198, 201, 204};
const int cTaskConsItemSinglePlay::nAnm[3] = {199, 202, 205};
float cTaskConsItemSinglePlay::PLAYER_SYMBOL_POS_X = 234.0f;
float cTaskConsItemSinglePlay::PLAYER_SYMBOL_POS_Y = 114.0f;
float cTaskConsItemSinglePlay::PLAYER_SYMBOL_NUMBER_POS_X = PLAYER_SYMBOL_POS_X + 30.0f;
float cTaskConsItemSinglePlay::PLAYER_SYMBOL_NUMBER_POS_Y = PLAYER_SYMBOL_POS_Y;
float cTaskConsItemSinglePlay::SCORE_NUMBER_POS_X = -206.0f;
float cTaskConsItemSinglePlay::SCORE_NUMBER_POS_Y = 119.0f;
float cTaskConsItemSinglePlay::ITEM_POWER_POS_X = -248.0f;
float cTaskConsItemSinglePlay::ITEM_POWER_POS_Y = -199.0f;
float cTaskConsItemSinglePlay::ITEM_JUMP_POS_X = -36.0f;
float cTaskConsItemSinglePlay::ITEM_JUMP_POS_Y = ITEM_POWER_POS_Y;
float cTaskConsItemSinglePlay::ITEM_CHARGE_POS_X = 173.0f;
float cTaskConsItemSinglePlay::ITEM_CHARGE_POS_Y = ITEM_POWER_POS_Y;
float cTaskConsItemSinglePlay::WORLD_NUMBER_POS_X = 252.0f;
float cTaskConsItemSinglePlay::WORLD_NUMBER_POS_Y = PLAYER_SYMBOL_POS_Y + 35.0f;
float cTaskConsItemSinglePlay::STAGE_NUMBER_POS_X = WORLD_NUMBER_POS_X + 20.0f;
float cTaskConsItemSinglePlay::STAGE_NUMBER_POS_Y = WORLD_NUMBER_POS_Y;

cTaskConsItemSinglePlay::cTaskConsItemSinglePlay()
{
	Init();
}

cTaskConsItemSinglePlay::~cTaskConsItemSinglePlay()
{
	Free();
}

void cTaskConsItemSinglePlay::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

cTaskAnita* cTaskConsItemSinglePlay::Construct()
{
	return new cTaskConsItemSinglePlay();
}

void cTaskConsItemSinglePlay::Setup()
{
	pAnita(0u)->SetState(3u);
}

void cTaskConsItemSinglePlay::Init()
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return;
	}
	int num = 0;
	if (!KingdomGlobal::MotosSys->IsBossStage())
	{
		num = 45;
	}
	m_nExecNumber = new Ref<int>(0);
	m_nExecScoreNumber = new Ref<int>(0);
	m_nWorldNumber = new Ref<int>(0);
	m_nStageNumber = new Ref<int>(0);
	for (int i = 0; i < (int)std::size(m_nItemUseNumber); i++)
	{
		m_nItemUseNumber[i] = new Ref<int>(0);
	}
	int array[6] = {198, 201, 204, 220, 157, 163};
	for (uint num2 = 0u; num2 < 6; num2++)
	{
		pAnita(num2)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num2)->SetAnimNo(array[num2]);
		Invisible(num2);
	}
	pAnita(3u)->vTrans()[1] = num;
	pAnita(4u)->vTrans()[1] = num;
	pAnita(5u)->vTrans()[0] = PLAYER_SYMBOL_POS_X;
	pAnita(5u)->vTrans()[1] = PLAYER_SYMBOL_POS_Y + (float)num;
	Vector4 pos = Vector4();
	pAnita(5u)->GetLayerPos(pos, 0u, 7);
	float array2[3][2] = {{ITEM_POWER_POS_X, pos[1]}, {ITEM_JUMP_POS_X, pos[1]}, {ITEM_CHARGE_POS_X, pos[1]}};
	for (uint num3 = 0u; num3 < 3; num3++)
	{
		m_pPacIcon[num3] = new cTaskConsPacIcon();
		for (uint num4 = 0u; num4 < 5; num4++)
		{
			Vector4 pos2 = Vector4();
			uint lay = 4 - num4 + (10 - num3 * 5);
			pAnita(5u)->GetLayerPos(pos2, lay, 7);
			m_pPacIcon[num3]->fConsoleX(num4) = pos2[0];
		}
		m_pPacIcon[num3]->fConsoleX(num3);
		m_pPacIcon[num3]->X() = 0.0f;
		m_pPacIcon[num3]->Y() = array2[num3][1];
		m_pPacIcon[num3]->SetpExecNumber(m_nItemUseNumber[num3]);
		ConnectChild(m_pPacIcon[num3]);
	}
	m_pPlayTime = new cTaskConsPlayTime();
	m_pPlayTime->vOffset()[1] = num;
	ConnectChild(m_pPlayTime);
	m_pSingleClear = new cTaskConsSingleClear();
	ConnectChild(m_pSingleClear);
	m_pPlayerNum = new cTaskConsItemNumber(1);
	m_pPlayerNum->X() = PLAYER_SYMBOL_NUMBER_POS_X;
	m_pPlayerNum->Y() = PLAYER_SYMBOL_NUMBER_POS_Y + (float)num;
	m_pPlayerNum->pExecNumber(m_nExecNumber);
	ConnectChild(m_pPlayerNum);
	m_pPlayerScoreNum = new cTaskConsItemNumber(6);
	m_pPlayerScoreNum->X() = SCORE_NUMBER_POS_X;
	m_pPlayerScoreNum->Y() = SCORE_NUMBER_POS_Y + (float)num;
	m_pPlayerScoreNum->pExecNumber(m_nExecScoreNumber);
	m_pPlayerScoreNum->SetFlag(38u, true);
	m_pPlayerScoreNum->SetFlag(1u, false);
	ConnectChild(m_pPlayerScoreNum);
	m_pWorldNum = new cTaskConsItemNumber(1);
	m_pWorldNum->X() = WORLD_NUMBER_POS_X;
	m_pWorldNum->Y() = WORLD_NUMBER_POS_Y + (float)num;
	m_pWorldNum->pExecNumber(m_nWorldNumber);
	m_pWorldNum->SetFlag(4u, true);
	ConnectChild(m_pWorldNum);
	m_pStageNum = new cTaskConsItemNumber(1);
	m_pStageNum->X() = STAGE_NUMBER_POS_X;
	m_pStageNum->Y() = STAGE_NUMBER_POS_Y + (float)num;
	m_pStageNum->pExecNumber(m_nStageNumber);
	m_pStageNum->SetFlag(16u, true);
	ConnectChild(m_pStageNum);
}

void cTaskConsItemSinglePlay::Term()
{
}

bool cTaskConsItemSinglePlay::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return cTaskConsoleBase::Execute(pParam);
	}
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 8)
	{
		if (IsStateChange())
		{
			for (uint num = 0u; num < 3; num++)
			{
				uint num2 = num + 1;
				m_nItemUseNumber[num]->Value = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nUseItem[num2];
			}
		}
		else
		{
			m_nExecNumber->Value = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife - 1;
			m_nExecScoreNumber->Value = KingdomGlobal::g_pMotosGame->nPoints(0);
			m_nWorldNumber->Value = (int)KingdomGlobal::MotosSys->nWorld();
			m_nStageNumber->Value = (int)KingdomGlobal::MotosSys->nStage();
		}
		m_pPlayerNum->bExec() = true;
		m_pPlayerScoreNum->bExec() = true;
		m_pWorldNum->bExec() = true;
		m_pStageNum->bExec() = true;
		for (uint num3 = 0u; num3 < 3; num3++)
		{
			m_pPacIcon[num3]->Visible(std::numeric_limits<uint>::max());
		}
		for (uint num4 = 0u; num4 < 6; num4++)
		{
			if (!IsStateChange())
			{
				Visible(num4);
			}
			if (num4 < 3)
			{
				if ((IsAnmNoSame(num4, nTikiAnm[num4]) && IsAnmEnd(num4)) || !IsAnmNoSame(num4, nTikiAnm[num4]))
				{
					SetAnmDiffCurrent(num4, nAnm[num4]);
				}
			}
			else if (5 == num4 && IsAnmEnd(num4) && !IsAnmNoSame(num4, 164))
			{
				pAnita(num4)->SetAnimNo(164);
			}
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
