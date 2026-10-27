#include "kingdom/game/g07/cTaskConsMenuMan.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/g07/cTaskConsMenuArrow.hpp"
#include "kingdom/game/g07/cTaskConsMenuBack.hpp"
#include "kingdom/game/g07/cTaskConsMenuBar.hpp"
#include "kingdom/game/g07/cTaskConsMenuBg.hpp"
#include "kingdom/game/g07/cTaskConsMenuCurtain.hpp"
#include "kingdom/game/g07/cTaskConsMenuMap.hpp"
#include "kingdom/game/g07/cTaskConsMenuMulti.hpp"
#include "kingdom/game/g07/cTaskConsMenuRule.hpp"
#include "kingdom/game/g07/cTaskConsMenuStage.hpp"
#include "kingdom/game/g07/cTaskConsMenuWorld.hpp"
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

int& cTaskConsMenuMan::nState()
{
	return m_nState;
}

int& cTaskConsMenuMan::nStep()
{
	return m_nStep;
}

uint& cTaskConsMenuMan::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMenuMan::nStageNo()
{
	return m_nStageNo;
}

uint& cTaskConsMenuMan::nFreeWorldNo()
{
	return m_nFreeWorldNo;
}

uint& cTaskConsMenuMan::nFreeStageNo()
{
	return m_nFreeStageNo;
}

int& cTaskConsMenuMan::nSelectObjNow()
{
	return m_nSelectObjNow;
}

int& cTaskConsMenuMan::nSelectObjOld()
{
	return m_nSelectObjOld;
}

cTaskConsMenuBg*& cTaskConsMenuMan::pTaskConsMenuBg()
{
	return m_pTaskConsMenuBg;
}

cTaskConsMenuBack*& cTaskConsMenuMan::pTaskConsMenuBack()
{
	return m_pTaskConsMenuBack;
}

cTaskConsMenuMulti*& cTaskConsMenuMan::pTaskConsMenuMulti()
{
	return m_pTaskConsMenuMulti;
}

cTaskConsMenuRule*& cTaskConsMenuMan::pTaskConsMenuRule()
{
	return m_pTaskConsMenuRule;
}

cTaskConsMenuBar*& cTaskConsMenuMan::pTaskConsMenuBar()
{
	return m_pTaskConsMenuBar;
}

cTaskConsMenuCurtain*& cTaskConsMenuMan::pTaskConsMenuCurtain()
{
	return m_pTaskConsMenuCurtain;
}

cTaskConsMenuWorld*& cTaskConsMenuMan::pTaskConsMenuWorld()
{
	return m_pTaskConsMenuWorld;
}

cTaskConsMenuMap*& cTaskConsMenuMan::pTaskConsMenuMap()
{
	return m_pTaskConsMenuMap;
}

cTaskConsMenuStage*& cTaskConsMenuMan::pTaskConsMenuStage(uint nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 7, "Stage Type Over");
	return m_pTaskConsMenuStage[nNo];
}

cTaskConsMenuArrow*& cTaskConsMenuMan::pTaskConsMenuArrow(uint nType)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 2, "Arrow Type Over");
	return m_pTaskConsMenuArrow[nType];
}

cTaskConsMenuBase*& cTaskConsMenuMan::pHitArray(uint nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 15, "Index Over");
	return m_pHitArray[nNo];
}

cTaskConsMenuMan::cTaskConsMenuMan(cAppGameBaseMenu* pApp)
	: cTaskConsMenuManBase(pApp)
{
	UnityUI = MotosUIManager::Get<MotosUIMotosHome>(MotosUIManager::EUIIndex::MotosHome);
	if (UnityUI != nullptr)
	{
		UnityUI->Init(MotosUIMotosHome::EType::Single);
	}
	// FIXME(cs2cpp): generic name SingletonBase<CheckPadConnectionManager>
	// TODO(unity): SingletonBase<CheckPadConnectionManager>.Instance.SetPlayNum();
	nState() = 0;
	nStep() = 0;
	StageSetting();
	nSelectObjOld() = (nSelectObjNow() = (int)(nStageNo() + 5));
	cTaskConsMenuBg*& reference = pTaskConsMenuBg();
	cTaskConsMenuManBase* pConsMan = this;
	reference = new cTaskConsMenuBg(pConsMan, nWorldNo());
	CustomKingdomUtil::Assert(pTaskConsMenuBg() != nullptr);
	pTaskConsMenuBg()->SetData();
	pApp->ConnectChild(pTaskConsMenuBg());
	// DOL 0x801c72ec: the Back button (+0x1dc, SetData 0x801c5d5c) is created right after the Bg (the C# dropped it: Unity left through its own UI)
	pTaskConsMenuBack() = new cTaskConsMenuBack(pConsMan);
	pTaskConsMenuBack()->SetData();
	pApp->ConnectChild(pTaskConsMenuBack());
	cTaskConsMenuMulti*& reference2 = pTaskConsMenuMulti();
	pConsMan = this;
	reference2 = new cTaskConsMenuMulti(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMenuMulti() != nullptr);
	pTaskConsMenuMulti()->SetData();
	pApp->ConnectChild(pTaskConsMenuMulti());
	cTaskConsMenuRule*& reference3 = pTaskConsMenuRule();
	pConsMan = this;
	reference3 = new cTaskConsMenuRule(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMenuRule() != nullptr);
	pTaskConsMenuRule()->SetData();
	pApp->ConnectChild(pTaskConsMenuRule());
	for (uint num = 0u; num < 2; num++)
	{
		cTaskConsMenuArrow*& reference4 = pTaskConsMenuArrow(num);
		pConsMan = this;
		reference4 = new cTaskConsMenuArrow(pConsMan);
		CustomKingdomUtil::Assert(pTaskConsMenuArrow(num) != nullptr);
		pTaskConsMenuArrow(num)->SetData(num, nFreeWorldNo() != 0);
		pApp->ConnectChild(pTaskConsMenuArrow(num));
	}
	for (uint num2 = 0u; num2 < 7; num2++)
	{
		cTaskConsMenuStage*& reference5 = pTaskConsMenuStage(num2);
		pConsMan = this;
		reference5 = new cTaskConsMenuStage(pConsMan, nWorldNo(), num2);
		CustomKingdomUtil::Assert(pTaskConsMenuStage(num2) != nullptr);
		pTaskConsMenuStage(num2)->LoadStageData(nWorldNo());
		pApp->ConnectChild(pTaskConsMenuStage(num2));
	}
	cTaskConsMenuWorld*& reference6 = pTaskConsMenuWorld();
	pConsMan = this;
	reference6 = new cTaskConsMenuWorld(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMenuWorld() != nullptr);
	pTaskConsMenuWorld()->SetData(nWorldNo());
	pApp->ConnectChild(pTaskConsMenuWorld());
	cTaskConsMenuBar*& reference7 = pTaskConsMenuBar();
	pConsMan = this;
	reference7 = new cTaskConsMenuBar(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMenuBar() != nullptr);
	pTaskConsMenuBar()->LoadRecord(nWorldNo(), nStageNo());
	pApp->ConnectChild(pTaskConsMenuBar());
	cTaskConsMenuCurtain*& reference8 = pTaskConsMenuCurtain();
	pConsMan = this;
	reference8 = new cTaskConsMenuCurtain(pConsMan);
	CustomKingdomUtil::Assert(pTaskConsMenuCurtain() != nullptr);
	pTaskConsMenuCurtain()->SetData();
	pApp->ConnectChild(pTaskConsMenuCurtain());
	cTaskConsMenuMap*& reference9 = pTaskConsMenuMap();
	pConsMan = this;
	reference9 = new cTaskConsMenuMap(pConsMan, nWorldNo(), nStageNo());
	CustomKingdomUtil::Assert(pTaskConsMenuMap() != nullptr);
	pApp->ConnectChild(pTaskConsMenuMap());
	AddHitArray();
}

void cTaskConsMenuMan::Free()
{
	KingdomGlobal::MotosSys->bNewStageApear() = false;
}

void cTaskConsMenuMan::StageSetting()
{
	uint freeStage = KingdomGlobal::MotosSys->GetFreeStage();
	nFreeWorldNo() = KingdomGlobal::MotosSys->Idx2World(freeStage) - 1;
	nFreeStageNo() = KingdomGlobal::MotosSys->Idx2Stage(freeStage) - 1;
	if (KingdomGlobal::SysCore->GetLastJobNo() == 140509184 || KingdomGlobal::SysCore->GetLastJobNo() == 288358400)
	{
		if (KingdomGlobal::MotosSys->IsNewStageApear())
		{
			nWorldNo() = nFreeWorldNo();
			nStageNo() = nFreeStageNo();
		}
		else if (KingdomGlobal::MotosSys->IsStageOpen(KingdomGlobal::MotosSys->nStageIdx()))
		{
			nWorldNo() = KingdomGlobal::MotosSys->nWorld() - 1;
			nStageNo() = KingdomGlobal::MotosSys->nStage() - 1;
		}
		else
		{
			nWorldNo() = nFreeWorldNo();
			nStageNo() = nFreeStageNo();
		}
	}
	else if (KingdomGlobal::MotosSys->IsAllClear())
	{
		KingdomGlobal::MotosSys->nStageIdx() = 1u;
		nWorldNo() = KingdomGlobal::MotosSys->nWorld() - 1;
		nStageNo() = KingdomGlobal::MotosSys->nStage() - 1;
	}
	else
	{
		nWorldNo() = nFreeWorldNo();
		nStageNo() = nFreeStageNo();
	}
}

void cTaskConsMenuMan::FreeStageCheck()
{
	if (nWorldNo() >= nFreeWorldNo())
	{
		nWorldNo() = nFreeWorldNo();
		if (nStageNo() >= nFreeStageNo())
		{
			nStageNo() = nFreeStageNo();
		}
	}
}

void cTaskConsMenuMan::AddHitArray()
{
	for (uint num = 0u; num < 15; num++)
	{
		pHitArray(num) = nullptr;
	}
	pHitArray(1u) = pTaskConsMenuBack(); // DecideCheck maps object 1 to state 2 (back)
	pHitArray(2u) = pTaskConsMenuMulti();
	pHitArray(3u) = pTaskConsMenuRule();
	pHitArray(4u) = pTaskConsMenuMap();
	pHitArray(5u) = pTaskConsMenuStage(0u);
	pHitArray(6u) = pTaskConsMenuStage(1u);
	pHitArray(7u) = pTaskConsMenuStage(2u);
	pHitArray(8u) = pTaskConsMenuStage(3u);
	pHitArray(9u) = pTaskConsMenuStage(4u);
	pHitArray(10u) = pTaskConsMenuStage(5u);
	pHitArray(11u) = pTaskConsMenuStage(6u);
	pHitArray(12u) = pTaskConsMenuArrow(0u);
	pHitArray(13u) = pTaskConsMenuArrow(1u);
}

// DOL 0x801c8a0c: Back (+0x1dc) first, then Multi / Rule / World / Bar / Curtain / Map, the 7 stages and the 2 arrows (the C# dropped Back, so the
// Back button kept its highlight after the selection moved on)
void cTaskConsMenuMan::AllWaitState()
{
	pTaskConsMenuBack()->WaitState();
	pTaskConsMenuMulti()->WaitState();
	pTaskConsMenuRule()->WaitState();
	pTaskConsMenuWorld()->WaitState();
	pTaskConsMenuBar()->WaitState();
	pTaskConsMenuCurtain()->WaitState();
	pTaskConsMenuMap()->WaitState();
	for (uint num = 0u; num < 7; num++)
	{
		pTaskConsMenuStage(num)->WaitState();
	}
	for (uint num2 = 0u; num2 < 2; num2++)
	{
		pTaskConsMenuArrow(num2)->WaitState();
	}
}

void cTaskConsMenuMan::_ObjProc()
{
	int num = nSelectObjNow();
	if ((uint)(num - 5) <= 6u)
	{
		nStageNo() = (uint)(nSelectObjNow() - 5);
		pTaskConsMenuMap()->ChangeData(nWorldNo(), nStageNo());
		pTaskConsMenuBar()->LoadRecord(nWorldNo(), nStageNo());
	}
}

// DOL 0x801c8270 (Select, inlined): every hit object (15) the pad 0 pointer is over becomes the selection, the last one wins; nothing hit -> the
// keys select. UNVERIFIED as in g03: the native pad reports no pointer outside the window, so nothing is hit then.
bool cTaskConsMenuMan::DpdSelect(int padNum)
{
	nSelectObjNow() = nSelectObjOld();
	const GamePad::State& pad = GamePad::Get(0);
	if (pad.pointerCount <= 0) return false;
	bool hit = false;
	for (uint num = 0u; num < 15u; num++)
	{
		if (pHitArray(num) != nullptr && pHitArray(num)->IsDpdHit(pad.pointerX, pad.pointerY))
		{
			nSelectObjNow() = (int)num;
			hit = true;
		}
	}
	return hit;
}

bool cTaskConsMenuMan::IsPointActive()
{
	switch (nSelectObjNow())
	{
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
		return pTaskConsMenuStage((uint)(nSelectObjNow() - 5))->IsActive();
	case 12:
		return pTaskConsMenuArrow(0u)->IsActive();
	case 13:
		return pTaskConsMenuArrow(1u)->IsActive();
	case 14:
		if (pTaskConsMenuArrow(0u)->IsActive())
		{
			return pTaskConsMenuArrow(1u)->IsActive();
		}
		return false;
	default:
		return true;
	}
}

void cTaskConsMenuMan::KeySelect(int padNum)
{
	// DOL 0x801c7abc: {object, up, down} table copied from 0x8037e7c8. The C# rerouted the three rows that lead to the Back button (1),
	// which Unity did not have, to 2 / 14; this is the DOL table.
	int array[14][3] = {{0, -1, -1}, {1, 2, 14}, {2, 5, 1}, {3, 14, 5}, {4, 1, 3}, {5, 3, 9}, {6, 3, 10}, {7, 3, 11}, {8, 3, 2}, {9, 5, 2}, {10, 6, 2}, {11, 7, 2}, {14, 1, 3}, {-1, -1, -1}};
	if (nSelectObjNow() == 13 || nSelectObjNow() == 12)
	{
		nSelectObjNow() = 14;
	}
	if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_UP) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_UP))
	{
		do
		{
			for (uint num5 = 0u; array[num5][0] != -1; num5++)
			{
				if (array[num5][0] == nSelectObjNow())
				{
					nSelectObjNow() = array[num5][1];
					break;
				}
			}
		}
		while (!IsPointActive());
	}
	else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_DOWN) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_DOWN))
	{
		do
		{
			for (uint num6 = 0u; array[num6][0] != -1; num6++)
			{
				if (array[num6][0] == nSelectObjNow())
				{
					nSelectObjNow() = array[num6][2];
					break;
				}
			}
		}
		while (!IsPointActive());
	}
	else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT))
	{
		if (5 <= nSelectObjNow() && nSelectObjNow() <= 8)
		{
			do
			{
				nSelectObjNow()--;
				if (nSelectObjNow() < 5)
				{
					nSelectObjNow() = 8;
				}
			}
			while (!IsPointActive());
		}
		else
		{
			if (9 > nSelectObjNow() || nSelectObjNow() > 11)
			{
				return;
			}
			do
			{
				nSelectObjNow()--;
				if (nSelectObjNow() < 9)
				{
					nSelectObjNow() = 11;
				}
			}
			while (!IsPointActive());
		}
	}
	else
	{
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT) && !InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT))
		{
			return;
		}
		if (5 <= nSelectObjNow() && nSelectObjNow() <= 8)
		{
			do
			{
				nSelectObjNow()++;
				if (nSelectObjNow() > 8)
				{
					nSelectObjNow() = 5;
				}
			}
			while (!IsPointActive());
		}
		else
		{
			if (9 > nSelectObjNow() || nSelectObjNow() > 11)
			{
				return;
			}
			do
			{
				nSelectObjNow()++;
				if (nSelectObjNow() > 11)
				{
					nSelectObjNow() = 9;
				}
			}
			while (!IsPointActive());
		}
	}
}

void cTaskConsMenuMan::SelectProc(int padNum)
{
	if (nSelectObjNow() == 0)
	{
		return;
	}
	if (nSelectObjNow() != nSelectObjOld() && nSelectObjNow() + nSelectObjOld() != 26 && nSelectObjNow() + nSelectObjOld() != 27)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
	}
	if (nSelectObjNow() != nSelectObjOld())
	{
		AllWaitState();
		_ObjProc();
		if (nSelectObjNow() == 14)
		{
			pHitArray(12u)->ResetAnm();
			pHitArray(13u)->ResetAnm();
		}
		else if (nSelectObjNow() == 4)
		{
			pHitArray(4u)->ResetAnm();
			pTaskConsMenuStage(nStageNo())->ResetAnm();
		}
	}
	if (nSelectObjNow() == 14)
	{
		pHitArray(12u)->PointState();
		pHitArray(13u)->PointState();
	}
	else if (nSelectObjNow() == 4)
	{
		pHitArray(4u)->PointState();
		pTaskConsMenuStage(nStageNo())->PointState();
	}
	else
	{
		pHitArray((uint)nSelectObjNow())->PointState();
	}
}

void cTaskConsMenuMan::DecideCheck(int padNum)
{
	int array[15][2] = {{0, -1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 8}, {6, 8}, {7, 8}, {8, 8}, {9, 8}, {10, 8}, {11, 8}, {12, 6}, {13, 7}, {-1, -1}};
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
		if (nSelectObjNow() == 14)
		{
			nState() = 6;
		}
	}
	else if ((InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT)) && nSelectObjNow() == 14)
	{
		nState() = 7;
	}
}

void cTaskConsMenuMan::Select()
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

void cTaskConsMenuMan::Reset()
{
	AllWaitState();
	nState() = 0;
	nStep() = 0;
}

bool cTaskConsMenuMan::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	int pad1No = InGamePadManager::GetPad1No();
	switch (nState())
	{
	case 2:
		if (nStep() == 0)
		{
			pTaskConsMenuBack()->DecideState();
			if (InGamePadManager::GetCancelKeyDown(pad1No))
			{
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE03_CANCEL);
			}
			else
			{
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
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
			pTaskConsMenuMulti()->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 4:
		if (nStep() == 0)
		{
			pTaskConsMenuRule()->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 5:
		if (nStep() == 0)
		{
			pTaskConsMenuMap()->DecideState();
			pTaskConsMenuStage(nStageNo())->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
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
				pTaskConsMenuArrow(0u)->WaitState();
			}
			else
			{
				nType = 0u;
				pTaskConsMenuArrow(1u)->WaitState();
			}
			if (nStep() == 0)
			{
				if (nState() == 7)
				{
					do
					{
						if (nWorldNo() == 4)
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
							nWorldNo() = 4u;
						}
						else
						{
							nWorldNo()--;
						}
					}
					while (nWorldNo() > nFreeWorldNo());
				}
				FreeStageCheck();
				pTaskConsMenuWorld()->DeleteState();
				pTaskConsMenuBar()->DeleteState();
				pTaskConsMenuMap()->DeleteState();
				for (uint num = 0u; num < 7; num++)
				{
					pTaskConsMenuStage(num)->DeleteState();
				}
				pTaskConsMenuArrow(nType)->DecideState();
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE07_WORLDSELECT);
				nStep()++;
			}
			else if (nStep() == 1)
			{
				bool flag = true;
				if (flag)
				{
					flag = pTaskConsMenuWorld()->IsDeleteEndState();
				}
				if (flag)
				{
					flag = pTaskConsMenuBar()->IsDeleteEndState();
				}
				if (flag)
				{
					flag = pTaskConsMenuMap()->IsDeleteEndState();
				}
				if (flag)
				{
					for (uint num2 = 0u; num2 < 7; num2++)
					{
						flag = pTaskConsMenuWorld()->IsDeleteEndState();
						if (!flag)
						{
							break;
						}
					}
				}
				if (flag)
				{
					pTaskConsMenuWorld()->nSetWorldNo(nWorldNo());
					pTaskConsMenuBar()->LoadRecord(nWorldNo(), nStageNo());
					pTaskConsMenuBg()->nSetWorldNo(nWorldNo());
					pTaskConsMenuMap()->ChangeData(nWorldNo(), nStageNo());
					for (uint num3 = 0u; num3 < 7; num3++)
					{
						pTaskConsMenuStage(num3)->LoadStageData(nWorldNo());
					}
					pTaskConsMenuBg()->AppearState();
					pTaskConsMenuWorld()->AppearState();
					pTaskConsMenuBar()->AppearState();
					pTaskConsMenuMap()->AppearState();
					for (uint num4 = 0u; num4 < 7; num4++)
					{
						pTaskConsMenuStage(num4)->AppearState();
					}
					nStep()++;
				}
			}
			else
			{
				if (nStep() != 2)
				{
					break;
				}
				bool flag2 = true;
				if (flag2)
				{
					flag2 = pTaskConsMenuArrow(nType)->IsDecideEndState();
				}
				if (flag2)
				{
					flag2 = pTaskConsMenuWorld()->IsAppearEndState();
				}
				if (flag2)
				{
					flag2 = pTaskConsMenuBar()->IsAppearEndState();
				}
				if (flag2)
				{
					flag2 = pTaskConsMenuMap()->IsAppearEndState();
				}
				if (flag2)
				{
					for (uint num5 = 0u; num5 < 7; num5++)
					{
						flag2 = pTaskConsMenuWorld()->IsAppearEndState();
						if (!flag2)
						{
							break;
						}
					}
				}
				if (flag2)
				{
					nState() = 0;
					nStep() = 0;
				}
			}
			break;
		}
	case 8:
		if (nStep() == 0)
		{
			if (pTaskConsMenuStage(nStageNo())->IsActive())
			{
				pTaskConsMenuStage(nStageNo())->DecideState();
				pTaskConsMenuMap()->DecideState();
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
				nStep()++;
			}
			else
			{
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE08_DECIDE_NG);
				nState() = 0;
			}
		}
		else
		{
			nStep();
			(void)(1);
		}
		break;
	}
	return true;
}

void cTaskConsMenuMan::AppearStart()
{
	pTaskConsMenuBack()->AppearState(); // UNVERIFIED for g07 (g03 DOL 0x8005c76c appears Back first)
	pTaskConsMenuMulti()->AppearState();
	pTaskConsMenuRule()->AppearState();
	pTaskConsMenuWorld()->AppearState();
	pTaskConsMenuBar()->AppearState();
	pTaskConsMenuCurtain()->AppearState();
	pTaskConsMenuMap()->AppearState();
	for (uint num = 0u; num < 7; num++)
	{
		pTaskConsMenuStage(num)->AppearState();
	}
	for (uint num2 = 0u; num2 < 2; num2++)
	{
		pTaskConsMenuArrow(num2)->AppearState();
	}
	if (UnityUI != nullptr) UnityUI->SetEnable(true);
	if (UnityUI != nullptr) UnityUI->PlayBaseAnim(MotosUIMotosHome::EBaseAnim::In);
}

bool cTaskConsMenuMan::IsAppearEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsMenuMulti()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMenuRule()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMenuWorld()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMenuBar()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMenuCurtain()->IsAppearEndState();
	}
	if (flag)
	{
		flag = pTaskConsMenuMap()->IsAppearEndState();
	}
	if (flag)
	{
		for (uint num = 0u; num < 7; num++)
		{
			flag = pTaskConsMenuStage(num)->IsAppearEndState();
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
			flag = pTaskConsMenuArrow(num2)->IsAppearEndState();
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

// DOL 0x801c8c8c: the Back task (+0x1dc) -> IsDecideEndState (vtable slot 0x84, same as IsStageDecide); the C# returned false (Unity left through its own UI)
bool cTaskConsMenuMan::IsBackDecide()
{
	return pTaskConsMenuBack() != nullptr && pTaskConsMenuBack()->IsDecideEndState();
}

bool cTaskConsMenuMan::IsStageDecide()
{
	return pTaskConsMenuStage(nStageNo())->IsDecideEndState();
}

bool cTaskConsMenuMan::IsMultiDecide()
{
	return pTaskConsMenuMulti()->IsDecideEndState();
}

bool cTaskConsMenuMan::IsRuleDecide()
{
	return pTaskConsMenuRule()->IsDecideEndState();
}

void cTaskConsMenuMan::Uplode()
{
	KingdomGlobal::MotosSys->nStageIdx() = nWorldNo() * 7 + nStageNo() + 1;
}

}
