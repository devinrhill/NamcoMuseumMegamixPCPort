#include "kingdom/game/g07/cTaskConsMultiMenuMan.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuArrow.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuBack.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuBg.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuBoard.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuCurtain.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuItem.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuMap.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuMiniArrow.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuPlayer.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuRound.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuRule.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuSingle.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuStart.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuTime.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuWorld.hpp"
#include "KingdomGlobal.hpp"
#include "MotosUIManager.hpp"
#include "MotosUIMotosHome.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <SoundManager.hpp>

#include "GamePad.hpp"

namespace kingdom::game::g07
{

int& cTaskConsMultiMenuMan::nState()
{
	return m_nState;
}

int& cTaskConsMultiMenuMan::nStep()
{
	return m_nStep;
}

int& cTaskConsMultiMenuMan::nSelectObjNow()
{
	return m_nSelectObjNow;
}

int& cTaskConsMultiMenuMan::nSelectObjOld()
{
	return m_nSelectObjOld;
}

uint& cTaskConsMultiMenuMan::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMultiMenuMan::nFreeWorldNo()
{
	return m_nFreeWorldNo;
}

uint& cTaskConsMultiMenuMan::nPlayerMax()
{
	return m_nPlayerMax;
}

cTaskConsMultiMenuBg*& cTaskConsMultiMenuMan::pTaskConsMultiMenuBg()
{
	return m_pTaskConsMultiMenuBg;
}

cTaskConsMultiMenuBack*& cTaskConsMultiMenuMan::pTaskConsMultiMenuBack()
{
	return m_pTaskConsMultiMenuBack;
}

cTaskConsMultiMenuSingle*& cTaskConsMultiMenuMan::pTaskConsMultiMenuSingle()
{
	return m_pTaskConsMultiMenuSingle;
}

cTaskConsMultiMenuRule*& cTaskConsMultiMenuMan::pTaskConsMultiMenuRule()
{
	return m_pTaskConsMultiMenuRule;
}

cTaskConsMultiMenuCurtain*& cTaskConsMultiMenuMan::pTaskConsMultiMenuCurtain()
{
	return m_pTaskConsMultiMenuCurtain;
}

cTaskConsMultiMenuWorld*& cTaskConsMultiMenuMan::pTaskConsMultiMenuWorld()
{
	return m_pTaskConsMultiMenuWorld;
}

cTaskConsMultiMenuBoard*& cTaskConsMultiMenuMan::pTaskConsMultiMenuBoard()
{
	return m_pTaskConsMultiMenuBoard;
}

cTaskConsMultiMenuRound*& cTaskConsMultiMenuMan::pTaskConsMultiMenuRound()
{
	return m_pTaskConsMultiMenuRound;
}

cTaskConsMultiMenuItem*& cTaskConsMultiMenuMan::pTaskConsMultiMenuItem()
{
	return m_pTaskConsMultiMenuItem;
}

cTaskConsMultiMenuTime*& cTaskConsMultiMenuMan::pTaskConsMultiMenuTime()
{
	return m_pTaskConsMultiMenuTime;
}

cTaskConsMultiMenuStart*& cTaskConsMultiMenuMan::pTaskConsMultiMenuStart()
{
	return m_pTaskConsMultiMenuStart;
}

cTaskConsMultiMenuMap*& cTaskConsMultiMenuMan::pTaskConsMultiMenuMap()
{
	return m_pTaskConsMultiMenuMap;
}

cTaskConsMultiMenuPlayer*& cTaskConsMultiMenuMan::pTaskConsMultiMenuPlayer(uint nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < nPlayerMax(), "Arrow Type Over");
	return m_pTaskConsMultiMenuPlayer[nNo];
}

cTaskConsMultiMenuArrow*& cTaskConsMultiMenuMan::pTaskConsMultiMenuArrow(uint nType)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 2, "Arrow Type Over");
	return m_pTaskConsMultiMenuArrow[nType];
}

cTaskConsMultiMenuMiniArrow*& cTaskConsMultiMenuMan::pTaskConsMultiMenuMiniArrow(uint nType)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 6, "Arrow Type Over");
	return m_pTaskConsMultiMenuMiniArrow[nType];
}

cTaskConsMenuBase*& cTaskConsMultiMenuMan::pHitArray(uint nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 18, "Arrow Type Over");
	return m_pHitArray[nNo];
}

cTaskConsMultiMenuMan::cTaskConsMultiMenuMan(cAppGameBaseMenu* pApp)
	: cTaskConsMenuManBase(pApp)
{
	UnityUI = MotosUIManager::Get<MotosUIMotosHome>(MotosUIManager::EUIIndex::MotosHome);
	if (UnityUI != nullptr)
	{
		UnityUI->Init(MotosUIMotosHome::EType::Multi);
	}
	if (KingdomGlobal::SysCore->GetLastJobNo() != 288358400)
	{
		nWorldNo() = 0u;
		KingdomGlobal::MotosSys->VsItemIdx() = 2u;
		KingdomGlobal::MotosSys->VsTimeIdx() = 2u;
		KingdomGlobal::MotosSys->VsRoundIdx() = 2u;
	}
	else
	{
		nWorldNo() = KingdomGlobal::MotosSys->nStageIdxTmp() - 36;
	}
	nState() = 0;
	nStep() = 0;
	nSelectObjOld() = (nSelectObjNow() = 5);
	nPlayerMax() = KingdomGlobal::MotosSys->pGetMatchingData()->nNum;
	// FIXME(cs2cpp): generic name SingletonBase<CheckPadConnectionManager>
	// TODO(unity): SingletonBase<CheckPadConnectionManager>.Instance.SetPlayNum((int)nPlayerMax());
	nFreeWorldNo() = 10u;
	cTaskConsMultiMenuBg*& reference = pTaskConsMultiMenuBg();
	cTaskConsMenuManBase* pConsMan;
	reference = new cTaskConsMultiMenuBg(pConsMan, nWorldNo());
	CustomKingdomUtil::Assert(pTaskConsMultiMenuBg() != nullptr);
	pTaskConsMultiMenuBg()->SetData();
	pApp->ConnectChild(pTaskConsMultiMenuBg());
	cTaskConsMultiMenuSingle*& reference2 = pTaskConsMultiMenuSingle();
	pConsMan = this;
	reference2 = new cTaskConsMultiMenuSingle(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuSingle() != nullptr);
	pTaskConsMultiMenuSingle()->SetData();
	pApp->ConnectChild(pTaskConsMultiMenuSingle());
	cTaskConsMultiMenuRule*& reference3 = pTaskConsMultiMenuRule();
	pConsMan = this;
	reference3 = new cTaskConsMultiMenuRule(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuRule() != nullptr);
	pTaskConsMultiMenuRule()->SetData();
	pApp->ConnectChild(pTaskConsMultiMenuRule());
	for (uint num = 0u; num < 2; num++)
	{
		cTaskConsMultiMenuArrow*& reference4 = pTaskConsMultiMenuArrow(num);
		pConsMan = this;
		reference4 = new cTaskConsMultiMenuArrow(pConsMan);
		CustomKingdomUtil::Assert(pTaskConsMultiMenuArrow(num) != nullptr);
		pTaskConsMultiMenuArrow(num)->SetData(num, nFreeWorldNo() != 0);
		pApp->ConnectChild(pTaskConsMultiMenuArrow(num));
	}
	for (uint num2 = 0u; num2 < 6; num2++)
	{
		cTaskConsMultiMenuMiniArrow*& reference5 = pTaskConsMultiMenuMiniArrow(num2);
		pConsMan = this;
		reference5 = new cTaskConsMultiMenuMiniArrow(pConsMan);
		CustomKingdomUtil::Assert(pTaskConsMultiMenuMiniArrow(num2) != nullptr);
		pTaskConsMultiMenuMiniArrow(num2)->SetData(num2);
		pApp->ConnectChild(pTaskConsMultiMenuMiniArrow(num2));
	}
	for (uint nPlayerNo = 0u; nPlayerNo < nPlayerMax(); nPlayerNo++)
	{
		cTaskConsMultiMenuPlayer*& reference6 = pTaskConsMultiMenuPlayer(nPlayerNo);
		pConsMan = this;
		reference6 = new cTaskConsMultiMenuPlayer(pConsMan, nPlayerNo, KingdomGlobal::MotosSys->pGetMatchingData()->Info[nPlayerNo]);
		CustomKingdomUtil::Assert(pTaskConsMultiMenuPlayer(nPlayerNo) != nullptr);
		pTaskConsMultiMenuPlayer(nPlayerNo)->SetData();
		pApp->ConnectChild(pTaskConsMultiMenuPlayer(nPlayerNo));
	}
	cTaskConsMultiMenuWorld*& reference7 = pTaskConsMultiMenuWorld();
	pConsMan = this;
	reference7 = new cTaskConsMultiMenuWorld(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuWorld() != nullptr);
	pTaskConsMultiMenuWorld()->SetData(nWorldNo());
	pApp->ConnectChild(pTaskConsMultiMenuWorld());
	cTaskConsMultiMenuStart*& reference8 = pTaskConsMultiMenuStart();
	pConsMan = this;
	reference8 = new cTaskConsMultiMenuStart(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuStart() != nullptr);
	pTaskConsMultiMenuStart()->SetData();
	pApp->ConnectChild(pTaskConsMultiMenuStart());
	cTaskConsMultiMenuRound*& reference9 = pTaskConsMultiMenuRound();
	pConsMan = this;
	reference9 = new cTaskConsMultiMenuRound(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuRound() != nullptr);
	pTaskConsMultiMenuRound()->SetData(KingdomGlobal::MotosSys->GetVsRound());
	pApp->ConnectChild(pTaskConsMultiMenuRound());
	cTaskConsMultiMenuTime*& reference10 = pTaskConsMultiMenuTime();
	pConsMan = this;
	reference10 = new cTaskConsMultiMenuTime(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuTime() != nullptr);
	pTaskConsMultiMenuTime()->SetData(KingdomGlobal::MotosSys->GetVsTime());
	pApp->ConnectChild(pTaskConsMultiMenuTime());
	cTaskConsMultiMenuItem*& reference11 = pTaskConsMultiMenuItem();
	pConsMan = this;
	reference11 = new cTaskConsMultiMenuItem(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuItem() != nullptr);
	pTaskConsMultiMenuItem()->SetData(KingdomGlobal::MotosSys->GetVsItem());
	pApp->ConnectChild(pTaskConsMultiMenuItem());
	cTaskConsMultiMenuBoard*& reference12 = pTaskConsMultiMenuBoard();
	pConsMan = this;
	reference12 = new cTaskConsMultiMenuBoard(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuBoard() != nullptr);
	pTaskConsMultiMenuBoard()->SetData();
	pApp->ConnectChild(pTaskConsMultiMenuBoard());
	cTaskConsMultiMenuCurtain*& reference13 = pTaskConsMultiMenuCurtain();
	pConsMan = this;
	reference13 = new cTaskConsMultiMenuCurtain(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMultiMenuCurtain() != nullptr);
	pTaskConsMultiMenuCurtain()->SetData();
	pApp->ConnectChild(pTaskConsMultiMenuCurtain());
	cTaskConsMultiMenuMap*& reference14 = pTaskConsMultiMenuMap();
	pConsMan = this;
	reference14 = new cTaskConsMultiMenuMap(pConsMan, nWorldNo());
	CustomKingdomUtil::Assert(pTaskConsMultiMenuMap() != nullptr);
	pApp->ConnectChild(pTaskConsMultiMenuMap());
	AddHitArray();
}

void cTaskConsMultiMenuMan::AddHitArray()
{
	for (uint num = 0u; num < 18; num++)
	{
		pHitArray(num) = nullptr;
	}
	pHitArray(2u) = pTaskConsMultiMenuSingle();
	pHitArray(3u) = pTaskConsMultiMenuRule();
	pHitArray(4u) = pTaskConsMultiMenuMap();
	pHitArray(6u) = pTaskConsMultiMenuArrow(0u);
	pHitArray(7u) = pTaskConsMultiMenuArrow(1u);
	pHitArray(9u) = pTaskConsMultiMenuMiniArrow(0u);
	pHitArray(10u) = pTaskConsMultiMenuMiniArrow(1u);
	pHitArray(12u) = pTaskConsMultiMenuMiniArrow(2u);
	pHitArray(13u) = pTaskConsMultiMenuMiniArrow(3u);
	pHitArray(15u) = pTaskConsMultiMenuMiniArrow(4u);
	pHitArray(16u) = pTaskConsMultiMenuMiniArrow(5u);
	pHitArray(5u) = pTaskConsMultiMenuStart();
}

cTaskConsMultiMenuMan::~cTaskConsMultiMenuMan()
{
}

void cTaskConsMultiMenuMan::AllWaitState()
{
	pTaskConsMultiMenuSingle()->WaitState();
	pTaskConsMultiMenuStart()->WaitState();
	pTaskConsMultiMenuRule()->WaitState();
	pTaskConsMultiMenuWorld()->WaitState();
	pTaskConsMultiMenuCurtain()->WaitState();
	pTaskConsMultiMenuBoard()->WaitState();
	pTaskConsMultiMenuMap()->WaitState();
	for (uint num = 0u; num < nPlayerMax(); num++)
	{
		pTaskConsMultiMenuPlayer(num)->WaitState();
	}
	for (uint num2 = 0u; num2 < 2; num2++)
	{
		pTaskConsMultiMenuArrow(num2)->WaitState();
	}
	for (uint num3 = 0u; num3 < 6; num3++)
	{
		pTaskConsMultiMenuMiniArrow(num3)->WaitState();
	}
}

void cTaskConsMultiMenuMan::_ObjProc()
{
}

// DOL 0x801cc8c4 (Select, inlined): every hit object (18) the pad 0 pointer is over becomes the selection, the last one wins; nothing hit -> the
// keys select. UNVERIFIED as in g03: the native pad reports no pointer outside the window, so nothing is hit then.
bool cTaskConsMultiMenuMan::DpdSelect(int padNum)
{
	nSelectObjNow() = nSelectObjOld();
	const GamePad::State& pad = GamePad::Get(0);
	if (pad.pointerCount <= 0) return false;
	bool hit = false;
	for (uint num = 0u; num < 18u; num++)
	{
		if (pHitArray(num) != nullptr && pHitArray(num)->IsDpdHit(pad.pointerX, pad.pointerY))
		{
			nSelectObjNow() = (int)num;
			hit = true;
		}
	}
	return hit;
}

bool cTaskConsMultiMenuMan::IsPointActive()
{
	return true;
}

void cTaskConsMultiMenuMan::KeySelect(int padNum)
{
	int num = 1;
	int num2 = 1;
	num = 2;
	num2 = 8;
	int array[11][3] = {{0, -1, -1}, {1, 2, 8}, {2, 5, num2}, {3, 8, 11}, {5, 17, 2}, {4, num, 3}, {8, num, 3}, {11, 3, 14}, {14, 11, 17}, {17, 14, 5}, {-1, -1, -1}};
	if (nSelectObjNow() == 7 || nSelectObjNow() == 6)
	{
		nSelectObjNow() = 8;
	}
	if (nSelectObjNow() == 10 || nSelectObjNow() == 9)
	{
		nSelectObjNow() = 11;
	}
	if (nSelectObjNow() == 13 || nSelectObjNow() == 12)
	{
		nSelectObjNow() = 14;
	}
	if (nSelectObjNow() == 16 || nSelectObjNow() == 15)
	{
		nSelectObjNow() = 17;
	}
	if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_UP) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_UP))
	{
		do
		{
			for (uint num3 = 0u; array[num3][0] != -1; num3++)
			{
				if (array[num3][0] == nSelectObjNow())
				{
					nSelectObjNow() = array[num3][1];
					break;
				}
			}
		}
		while (!IsPointActive());
	}
	else
	{
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_DOWN) && !InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_DOWN))
		{
			return;
		}
		do
		{
			for (uint num4 = 0u; array[num4][0] != -1; num4++)
			{
				if (array[num4][0] == nSelectObjNow())
				{
					nSelectObjNow() = array[num4][2];
					break;
				}
			}
		}
		while (!IsPointActive());
	}
}

void cTaskConsMultiMenuMan::SelectProc(int padNum)
{
	if (nSelectObjNow() == 0)
	{
		return;
	}
	if (nSelectObjNow() != nSelectObjOld())
	{
		bool flag = false;
		int num = nSelectObjNow() * nSelectObjOld();
		if (!flag)
		{
			flag = num == 48;
		}
		if (!flag)
		{
			flag = num == 56;
		}
		if (!flag)
		{
			flag = num == 99;
		}
		if (!flag)
		{
			flag = num == 110;
		}
		if (!flag)
		{
			flag = num == 168;
		}
		if (!flag)
		{
			flag = num == 182;
		}
		if (!flag)
		{
			flag = num == 255;
		}
		if (!flag)
		{
			flag = num == 272;
		}
		if (!flag)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE01_SELECT);
		}
		AllWaitState();
		_ObjProc();
		if (nSelectObjNow() == 8)
		{
			pHitArray(6u)->ResetAnm();
			pHitArray(7u)->ResetAnm();
		}
		else if (nSelectObjNow() == 11)
		{
			pHitArray(9u)->ResetAnm();
			pHitArray(10u)->ResetAnm();
		}
		else if (nSelectObjNow() == 14)
		{
			pHitArray(12u)->ResetAnm();
			pHitArray(13u)->ResetAnm();
		}
		else if (nSelectObjNow() == 17)
		{
			pHitArray(15u)->ResetAnm();
			pHitArray(16u)->ResetAnm();
		}
		else if (nSelectObjNow() == 4)
		{
			pHitArray(4u)->ResetAnm();
			pTaskConsMultiMenuStart()->ResetAnm();
		}
	}
	if (nSelectObjNow() == 8)
	{
		pHitArray(6u)->PointState();
		pHitArray(7u)->PointState();
	}
	else if (nSelectObjNow() == 11)
	{
		pHitArray(9u)->PointState();
		pHitArray(10u)->PointState();
	}
	else if (nSelectObjNow() == 14)
	{
		pHitArray(12u)->PointState();
		pHitArray(13u)->PointState();
	}
	else if (nSelectObjNow() == 17)
	{
		pHitArray(15u)->PointState();
		pHitArray(16u)->PointState();
	}
	else if (nSelectObjNow() == 4)
	{
		pHitArray(4u)->PointState();
		pTaskConsMultiMenuStart()->PointState();
	}
	else
	{
		pHitArray((uint)nSelectObjNow())->PointState();
	}
}

void cTaskConsMultiMenuMan::DecideCheck(int padNum)
{
	int array[15][2] = {{0, -1}, {1, 2}, {2, 3}, {3, 4}, {5, 14}, {4, 5}, {6, 6}, {7, 7}, {9, 8}, {10, 9}, {12, 10}, {13, 11}, {15, 12}, {16, 13}, {-1, -1}};
	if (nSelectObjNow() == 0)
	{
		return;
	}
	if (InGamePadManager::GetCancelKeyDown(padNum) && nSelectObjNow() == nSelectObjOld() && nSelectObjOld() == 1)
	{
		nState() = 2;
	}
	else if (InGamePadManager::GetEnterKeyDown(padNum))
	{
		for (uint num = 0u; array[num][0] != -1; num++)
		{
			if (array[num][0] == nSelectObjNow())
			{
				nState() = array[num][1];
			}
		}
	}
	else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT))
	{
		if (nSelectObjNow() == 8)
		{
			nState() = 6;
		}
		else if (nSelectObjNow() == 11)
		{
			nState() = 8;
		}
		else if (nSelectObjNow() == 14)
		{
			nState() = 10;
		}
		else if (nSelectObjNow() == 17)
		{
			nState() = 12;
		}
	}
	else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT))
	{
		if (nSelectObjNow() == 8)
		{
			nState() = 7;
		}
		else if (nSelectObjNow() == 11)
		{
			nState() = 9;
		}
		else if (nSelectObjNow() == 14)
		{
			nState() = 11;
		}
		else if (nSelectObjNow() == 17)
		{
			nState() = 13;
		}
	}
}

void cTaskConsMultiMenuMan::Select()
{
	if (nState() == 0)
	{
		int pad1No = InGamePadManager::GetPad1No();
		if (!DpdSelect(pad1No))
		{
			KeySelect(pad1No);
		}
		SelectProc(pad1No);
		DecideCheck(pad1No);
		nSelectObjOld() = nSelectObjNow();
	}
}

void cTaskConsMultiMenuMan::Reset()
{
	AllWaitState();
	nState() = 0;
	nStep() = 0;
}

bool cTaskConsMultiMenuMan::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	switch (nState())
	{
	case 2:
		if (nStep() == 0)
		{
			pTaskConsMultiMenuBack()->DecideState();
			if (InGamePadManager::GetCancelKeyDown(InGamePadManager::GetPad1No()))
			{
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE03_CANCEL);
			}
			else
			{
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			}
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 3:
		if (nStep() == 0)
		{
			pTaskConsMultiMenuSingle()->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 14:
		if (nStep() == 0)
		{
			pTaskConsMultiMenuStart()->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 4:
		if (nStep() == 0)
		{
			pTaskConsMultiMenuRule()->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 8:
	case 9:
		{
			uint nType3;
			if (nState() == 9)
			{
				nType3 = 1u;
				pTaskConsMultiMenuMiniArrow(0u)->WaitState();
			}
			else
			{
				nType3 = 0u;
				pTaskConsMultiMenuMiniArrow(1u)->WaitState();
			}
			if (nStep() == 0)
			{
				pTaskConsMultiMenuRound()->DeleteState();
				pTaskConsMultiMenuMiniArrow(nType3)->DecideState();
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE07_WORLDSELECT);
				nStep()++;
			}
			else if (nStep() == 1)
			{
				if (!pTaskConsMultiMenuRound()->IsDeleteEndState())
				{
					break;
				}
				if (nState() == 9)
				{
					KingdomGlobal::MotosSys->VsRoundIdx()++;
					if (KingdomGlobal::MotosSys->VsRoundIdx() >= 4)
					{
						KingdomGlobal::MotosSys->VsRoundIdx() = 0u;
					}
				}
				else if (KingdomGlobal::MotosSys->VsRoundIdx() == 0)
				{
					KingdomGlobal::MotosSys->VsRoundIdx() = 3u;
				}
				else
				{
					KingdomGlobal::MotosSys->VsRoundIdx()--;
				}
				pTaskConsMultiMenuRound()->nSetRound(KingdomGlobal::MotosSys->GetVsRound());
				pTaskConsMultiMenuRound()->AppearState();
				nStep()++;
			}
			else if (nStep() == 2)
			{
				bool flag4 = true;
				if (flag4)
				{
					flag4 = pTaskConsMultiMenuMiniArrow(nType3)->IsDecideEndState();
				}
				if (flag4)
				{
					flag4 = pTaskConsMultiMenuRound()->IsAppearEndState();
				}
				if (flag4)
				{
					nState() = 0;
					nStep() = 0;
				}
			}
			break;
		}
	case 10:
	case 11:
		{
			uint nType4;
			if (nState() == 11)
			{
				nType4 = 3u;
				pTaskConsMultiMenuMiniArrow(2u)->WaitState();
			}
			else
			{
				nType4 = 2u;
				pTaskConsMultiMenuMiniArrow(3u)->WaitState();
			}
			if (nStep() == 0)
			{
				pTaskConsMultiMenuTime()->DeleteState();
				pTaskConsMultiMenuMiniArrow(nType4)->DecideState();
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE07_WORLDSELECT);
				nStep()++;
			}
			else if (nStep() == 1)
			{
				if (!pTaskConsMultiMenuTime()->IsDeleteEndState())
				{
					break;
				}
				if (nState() == 11)
				{
					KingdomGlobal::MotosSys->VsTimeIdx()++;
					if (KingdomGlobal::MotosSys->VsTimeIdx() >= 5)
					{
						KingdomGlobal::MotosSys->VsTimeIdx() = 0u;
					}
				}
				else if (KingdomGlobal::MotosSys->VsTimeIdx() == 0)
				{
					KingdomGlobal::MotosSys->VsTimeIdx() = 4u;
				}
				else
				{
					KingdomGlobal::MotosSys->VsTimeIdx()--;
				}
				pTaskConsMultiMenuTime()->nSetTime(KingdomGlobal::MotosSys->GetVsTime());
				pTaskConsMultiMenuTime()->AppearState();
				nStep()++;
			}
			else if (nStep() == 2)
			{
				bool flag5 = true;
				if (flag5)
				{
					flag5 = pTaskConsMultiMenuMiniArrow(nType4)->IsDecideEndState();
				}
				if (flag5)
				{
					flag5 = pTaskConsMultiMenuTime()->IsAppearEndState();
				}
				if (flag5)
				{
					nState() = 0;
					nStep() = 0;
				}
			}
			break;
		}
	case 12:
	case 13:
		{
			uint nType2;
			if (nState() == 13)
			{
				nType2 = 5u;
				pTaskConsMultiMenuMiniArrow(4u)->WaitState();
			}
			else
			{
				nType2 = 4u;
				pTaskConsMultiMenuMiniArrow(5u)->WaitState();
			}
			if (nStep() == 0)
			{
				pTaskConsMultiMenuItem()->DeleteState();
				pTaskConsMultiMenuMiniArrow(nType2)->DecideState();
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE07_WORLDSELECT);
				nStep()++;
			}
			else if (nStep() == 1)
			{
				if (!pTaskConsMultiMenuItem()->IsDeleteEndState())
				{
					break;
				}
				if (nState() == 13)
				{
					KingdomGlobal::MotosSys->VsItemIdx()++;
					if (KingdomGlobal::MotosSys->VsItemIdx() >= 6)
					{
						KingdomGlobal::MotosSys->VsItemIdx() = 0u;
					}
				}
				else if (KingdomGlobal::MotosSys->VsItemIdx() == 0)
				{
					KingdomGlobal::MotosSys->VsItemIdx() = 5u;
				}
				else
				{
					KingdomGlobal::MotosSys->VsItemIdx()--;
				}
				pTaskConsMultiMenuItem()->nSetItem(KingdomGlobal::MotosSys->GetVsItem());
				pTaskConsMultiMenuItem()->AppearState();
				nStep()++;
			}
			else if (nStep() == 2)
			{
				bool flag3 = true;
				if (flag3)
				{
					flag3 = pTaskConsMultiMenuMiniArrow(nType2)->IsDecideEndState();
				}
				if (flag3)
				{
					flag3 = pTaskConsMultiMenuItem()->IsAppearEndState();
				}
				if (flag3)
				{
					nState() = 0;
					nStep() = 0;
				}
			}
			break;
		}
	case 5:
		if (nStep() == 0)
		{
			pTaskConsMultiMenuMap()->DecideState();
			pTaskConsMultiMenuStart()->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 6:
	case 7:
		{
			uint nType;
			if (nState() == 7)
			{
				nType = 1u;
				pTaskConsMultiMenuArrow(0u)->WaitState();
			}
			else
			{
				nType = 0u;
				pTaskConsMultiMenuArrow(1u)->WaitState();
			}
			if (nStep() == 0)
			{
				if (nState() == 7)
				{
					do
					{
						if (nWorldNo() == 9)
						{
							nWorldNo() = 0u;
						}
						else
						{
							nWorldNo()++;
						}
					}
					while (nWorldNo() > nFreeWorldNo());
				}
				else
				{
					do
					{
						if (nWorldNo() == 0)
						{
							nWorldNo() = 9u;
						}
						else
						{
							nWorldNo()--;
						}
					}
					while (nWorldNo() > nFreeWorldNo());
				}
				pTaskConsMultiMenuWorld()->DeleteState();
				pTaskConsMultiMenuMap()->DeleteState();
				pTaskConsMultiMenuArrow(nType)->DecideState();
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE07_WORLDSELECT);
				nStep()++;
			}
			else if (nStep() == 1)
			{
				bool flag = true;
				if (flag)
				{
					flag = pTaskConsMultiMenuWorld()->IsDeleteEndState();
				}
				if (flag)
				{
					flag = pTaskConsMultiMenuMap()->IsDeleteEndState();
				}
				if (flag)
				{
					pTaskConsMultiMenuWorld()->nSetWorldNo(nWorldNo());
					pTaskConsMultiMenuBg()->nSetWorldNo(nWorldNo());
					pTaskConsMultiMenuMap()->ChangeData(nWorldNo());
					pTaskConsMultiMenuBg()->AppearState();
					pTaskConsMultiMenuWorld()->AppearState();
					pTaskConsMultiMenuMap()->AppearState();
					nStep()++;
				}
			}
			else if (nStep() == 2)
			{
				bool flag2 = true;
				if (flag2)
				{
					flag2 = pTaskConsMultiMenuArrow(nType)->IsDecideEndState();
				}
				if (flag2)
				{
					flag2 = pTaskConsMultiMenuWorld()->IsAppearEndState();
				}
				if (flag2)
				{
					flag2 = pTaskConsMultiMenuMap()->IsAppearEndState();
				}
				if (flag2)
				{
					nState() = 0;
					nStep() = 0;
				}
			}
			break;
		}
	}
	return true;
}

void cTaskConsMultiMenuMan::AppearStart()
{
	pTaskConsMultiMenuSingle()->AppearState();
	pTaskConsMultiMenuStart()->AppearState();
	pTaskConsMultiMenuRule()->AppearState();
	pTaskConsMultiMenuWorld()->AppearState();
	pTaskConsMultiMenuCurtain()->AppearState();
	pTaskConsMultiMenuBoard()->AppearState();
	pTaskConsMultiMenuRound()->AppearState();
	pTaskConsMultiMenuItem()->AppearState();
	pTaskConsMultiMenuTime()->AppearState();
	pTaskConsMultiMenuMap()->AppearState();
	for (uint num = 0u; num < nPlayerMax(); num++)
	{
		pTaskConsMultiMenuPlayer(num)->AppearState();
	}
	for (uint num2 = 0u; num2 < 2; num2++)
	{
		pTaskConsMultiMenuArrow(num2)->AppearState();
	}
	for (uint num3 = 0u; num3 < 6; num3++)
	{
		pTaskConsMultiMenuMiniArrow(num3)->AppearState();
	}
	if (UnityUI != nullptr) UnityUI->SetEnable(true);
	if (UnityUI != nullptr) UnityUI->PlayBaseAnim(MotosUIMotosHome::EBaseAnim::In);
}

bool cTaskConsMultiMenuMan::IsAppearEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsMultiMenuSingle()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuStart()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuRule()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuWorld()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuCurtain()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuBoard()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuRound()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuItem()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuTime()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMultiMenuMap()->IsAppearEndState();
	}
	if (flag)
	{
		for (uint num = 0u; num < nPlayerMax(); num++)
		{
			flag = pTaskConsMultiMenuPlayer(num)->IsAppearEndState();
			if (!flag)
			{
				break;
			}
		}
	}
	if (flag)
	{
		for (uint num2 = 0u; num2 < 2; num2++)
		{
			flag = pTaskConsMultiMenuArrow(num2)->IsAppearEndState();
			if (!flag)
			{
				break;
			}
		}
	}
	if (flag)
	{
		for (uint num3 = 0u; num3 < 6; num3++)
		{
			flag = pTaskConsMultiMenuMiniArrow(num3)->IsAppearEndState();
			if (!flag)
			{
				break;
			}
		}
	}
	if (flag)
	{
		flag = (UnityUI == nullptr || !UnityUI->IsPlayBaseAnim(MotosUIMotosHome::EBaseAnim::In));
	}
	return flag;
}

bool cTaskConsMultiMenuMan::IsBackDecide()
{
	return false;
}

bool cTaskConsMultiMenuMan::IsStartDecide()
{
	return pTaskConsMultiMenuStart()->IsDecideEndState();
}

bool cTaskConsMultiMenuMan::IsSingleDecide()
{
	return pTaskConsMultiMenuSingle()->IsDecideEndState();
}

bool cTaskConsMultiMenuMan::IsRuleDecide()
{
	return pTaskConsMultiMenuRule()->IsDecideEndState();
}

void cTaskConsMultiMenuMan::Upload()
{
	KingdomGlobal::MotosSys->nStageIdx() = 36 + nWorldNo();
}

}
