#include "kingdom/game/g03/cTaskConsMenuMan.hpp"
#include "GamePad.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/g03/cTaskConsMenuArrow.hpp"
#include "kingdom/game/g03/cTaskConsMenuBack.hpp"
#include "kingdom/game/g03/cTaskConsMenuBar.hpp"
#include "kingdom/game/g03/cTaskConsMenuBg.hpp"
#include "kingdom/game/g03/cTaskConsMenuCurtain.hpp"
#include "kingdom/game/g03/cTaskConsMenuMap.hpp"
#include "kingdom/game/g03/cTaskConsMenuRule.hpp"
#include "kingdom/game/g03/cTaskConsMenuStage.hpp"
#include "kingdom/game/g03/cTaskConsMenuWorld.hpp"
#include "kingdom/game/g03/cTaskMenuPacman.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "MotosUIManager.hpp"
#include "MotosUIRollHome.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <SoundManager.hpp>
#include <SSaveDataPacnRoll.hpp>

namespace kingdom::game::g03
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

cTaskMenuPacman*& cTaskConsMenuMan::pTaskMenuPacman()
{
	return m_pTaskMenuPacman;
}

cTaskConsMenuStage*& cTaskConsMenuMan::pTaskConsMenuStage(uint nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 6, "Stage Type Over");
	return m_pTaskConsMenuStage[nNo];
}

cTaskConsMenuArrow*& cTaskConsMenuMan::pTaskConsMenuArrow(uint nType)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 2, "Arrow Type Over");
	return m_pTaskConsMenuArrow[nType];
}

cTaskConsMenuBase*& cTaskConsMenuMan::pHitArray(uint nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 13, "error");
	return m_pHitArray[nNo];
}

cTaskConsMenuMan::cTaskConsMenuMan(cAppGameBaseMenu* pApp)
	: cTaskConsMenuManBase(pApp)
{
	UnityUI = MotosUIManager::Get<MotosUIRollHome>(MotosUIManager::EUIIndex::RollHome);
	nState() = 0;
	nStep() = 0;
	StageSetting();
	nSelectObjOld() = (nSelectObjNow() = (int)(4 + nStageNo()));
	cTaskConsMenuBg*& reference = pTaskConsMenuBg();
	cTaskConsMenuManBase* pConsMan = this;
	reference = new cTaskConsMenuBg(pConsMan, nWorldNo());
	CustomKingdomUtil::Assert(pTaskConsMenuBg() != nullptr);
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuBg());
	// DOL 0x8005bfdc: the Back button (+0x1e0) is created right after the Bg (the C# dropped it: Unity left through its own UI)
	pTaskConsMenuBack() = new cTaskConsMenuBack(this);
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuBack());
	pTaskConsMenuRule() = new cTaskConsMenuRule(this);
	CustomKingdomUtil::Assert(pTaskConsMenuRule() != nullptr);
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuRule());
	pTaskConsMenuRule()->UnityUI = UnityUI;
	for (uint num = 0u; num < 2; num++)
	{
		pTaskConsMenuArrow(num) = new cTaskConsMenuArrow(this, num, nFreeWorldNo() != 0);
		CustomKingdomUtil::Assert(pTaskConsMenuArrow(num) != nullptr);
		KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuArrow(num));
	}
	for (uint num2 = 0u; num2 < 6; num2++)
	{
		pTaskConsMenuStage(num2) = new cTaskConsMenuStage(this, nWorldNo(), num2);
		CustomKingdomUtil::Assert(pTaskConsMenuStage(num2) != nullptr);
		pTaskConsMenuStage(num2)->LoadStageData(nWorldNo());
		KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuStage(num2));
	}
	pTaskConsMenuWorld() = new cTaskConsMenuWorld(this);
	CustomKingdomUtil::Assert(pTaskConsMenuWorld() != nullptr);
	pTaskConsMenuWorld()->nSetWorldNo(nWorldNo());
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuWorld());
	pTaskConsMenuBar() = new cTaskConsMenuBar(this);
	CustomKingdomUtil::Assert(pTaskConsMenuBar() != nullptr);
	pTaskConsMenuBar()->LoadRecord(nWorldNo(), nStageNo());
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuBar());
	pTaskConsMenuCurtain() = new cTaskConsMenuCurtain(this);
	CustomKingdomUtil::Assert(pTaskConsMenuCurtain() != nullptr);
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuCurtain());
	pTaskMenuPacman() = new cTaskMenuPacman();
	CustomKingdomUtil::Assert(pTaskMenuPacman() != nullptr);
	pTaskMenuPacman()->RegistData();
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskMenuPacman());
	cTaskConsMenuMap*& reference2 = pTaskConsMenuMap();
	pConsMan = this;
	reference2 = new cTaskConsMenuMap(pConsMan, nWorldNo(), nStageNo());
	CustomKingdomUtil::Assert(pTaskConsMenuMap() != nullptr);
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(pTaskConsMenuMap());
	AddHitArray();
}

void cTaskConsMenuMan::Free()
{
	if (pTaskMenuPacman() != nullptr)
	{
		pTaskMenuPacman()->TermOnce();
	}
}

void cTaskConsMenuMan::Reset()
{
	AllWaitState();
	nState() = 0;
	nStep() = 0;
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
	else
	{
		uint num = KingdomGlobal::PacnRollSys->nStageMax(nWorldNo());
		if (nStageNo() >= num)
		{
			nStageNo() = num - 1;
		}
	}
	if (KingdomGlobal::PacnRollSys->nWorldStage(nWorldNo(), nStageNo()) == 25)
	{
		nStageNo() = 4u;
	}
}

void cTaskConsMenuMan::StageSetting()
{
	uint nWorldStage = KingdomGlobal::PacnRollSys->nFreeStage();
	nFreeWorldNo() = KingdomGlobal::PacnRollSys->nWorld(nWorldStage);
	nFreeStageNo() = KingdomGlobal::PacnRollSys->nStage(nWorldStage);
	if (KingdomGlobal::SysCore->GetLastJobNo() == 140509184 || KingdomGlobal::SysCore->GetLastJobNo() == 221249536)
	{
		if (KingdomGlobal::PacnRollSys->IsOpenStage())
		{
			nWorldNo() = nFreeWorldNo();
			nStageNo() = nFreeStageNo();
		}
		else
		{
			SSaveDataPacnRoll::sStageRecord _out = SSaveDataPacnRoll::sStageRecord();
			KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, KingdomGlobal::PacnRollSys->nRound());
			if (_out.bFree)
			{
				nWorldNo() = KingdomGlobal::PacnRollSys->nWorld();
				nStageNo() = KingdomGlobal::PacnRollSys->nStage();
			}
			else
			{
				nWorldNo() = KingdomGlobal::PacnRollSys->nWorld(nWorldStage);
				nStageNo() = KingdomGlobal::PacnRollSys->nStage(nWorldStage);
			}
		}
	}
	else if (KingdomGlobal::PacnRollSys->IsAllClear())
	{
		KingdomGlobal::PacnRollSys->nRound() = 0u;
		nWorldNo() = KingdomGlobal::PacnRollSys->nWorld();
		nStageNo() = KingdomGlobal::PacnRollSys->nStage();
	}
	else
	{
		nWorldNo() = nFreeWorldNo();
		nStageNo() = nFreeStageNo();
	}
	if (KingdomGlobal::PacnRollSys->nWorldStage(nWorldNo(), nStageNo()) == 25)
	{
		nStageNo() = 4u;
	}
}

void cTaskConsMenuMan::AddHitArray()
{
	for (uint num = 0u; num < 13; num++)
	{
		pHitArray(num) = nullptr;
	}
	pHitArray(1u) = pTaskConsMenuBack(); // DOL 0x8005c6f4 (the converted C# dropped it): [1] = +0x1e0 Back, [2] Rule, [3] Map, [4..9] Stage, [10..11] Arrow
	pHitArray(2u) = pTaskConsMenuRule();
	pHitArray(3u) = pTaskConsMenuMap();
	pHitArray(4u) = pTaskConsMenuStage(0u);
	pHitArray(5u) = pTaskConsMenuStage(1u);
	pHitArray(6u) = pTaskConsMenuStage(2u);
	pHitArray(7u) = pTaskConsMenuStage(3u);
	pHitArray(8u) = pTaskConsMenuStage(4u);
	pHitArray(9u) = pTaskConsMenuStage(5u);
	pHitArray(10u) = pTaskConsMenuArrow(0u);
	pHitArray(11u) = pTaskConsMenuArrow(1u);
}

// DOL 0x8005c76c (called by SelectProc 0x8005ce1c on a focus change): every object's WaitState (vtable +0x64), the Back button (+0x1e0) first.
// The converted C# had no Back button, so it stayed in its pointed look after the focus moved away ("Return stays in focus")
void cTaskConsMenuMan::AllWaitState()
{
	pTaskConsMenuBack()->WaitState();
	pTaskConsMenuRule()->WaitState();
	pTaskConsMenuWorld()->WaitState();
	pTaskConsMenuBar()->WaitState();
	pTaskConsMenuCurtain()->WaitState();
	pTaskConsMenuMap()->WaitState();
	pTaskMenuPacman()->WaitState();
	for (uint num = 0u; num < 6; num++)
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
	if ((uint)(num - 4) <= 5u)
	{
		nStageNo() = (uint)(nSelectObjNow() - 4);
		pTaskConsMenuMap()->ChangeData(nWorldNo(), nStageNo());
		pTaskConsMenuBar()->LoadRecord(nWorldNo(), nStageNo());
	}
}

// DOL 0x8005d270 (Select, DpdSelect inlined at 0x8005d298..0x8005d2fc): pad 0's pointer is tested against every object of the hit array in index order;
// the LAST object that is hit becomes the selection. UNVERIFIED: the DOL has no "pointer valid" test here (pad +0xa8 is only checked by the cursor);
// the native pad reports no pointer when the mouse is outside the window, so nothing is hit then and the keys select instead.
bool cTaskConsMenuMan::DpdSelect(int padNum)
{
	nSelectObjNow() = nSelectObjOld();
	const GamePad::State& pad = GamePad::Get(0);
	if (pad.pointerCount <= 0) return false;
	bool hit = false;
	for (uint num = 0u; num < 13; num++)
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
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		return pTaskConsMenuStage((uint)(nSelectObjNow() - 4))->IsActive();
	case 10:
		return pTaskConsMenuArrow(0u)->IsActive();
	case 11:
		return pTaskConsMenuArrow(1u)->IsActive();
	case 12:
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
	// DOL 0x8005c8f4: {object, up, down} table copied from 0x8036aba0. The C# rerouted the rows that lead to the Back button (1), which Unity
	// did not have; this is the DOL table.
	int array[12][3] = {{0, -1, -1}, {1, 4, 12}, {2, 12, 4}, {3, 1, 2}, {4, 2, 7}, {5, 2, 8}, {6, 2, 9}, {7, 4, 1}, {8, 5, 1}, {9, 6, 1}, {12, 1, 2}, {-1, -1, -1}};
	if (nSelectObjNow() == 11 || nSelectObjNow() == 10)
	{
		nSelectObjNow() = 12;
	}
	if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_UP) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_UP))
	{
		do
		{
			for (uint num = 0u; array[num][0] != -1; num++)
			{
				if (array[num][0] == nSelectObjNow())
				{
					nSelectObjNow() = array[num][1];
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
			for (uint num2 = 0u; array[num2][0] != -1; num2++)
			{
				if (array[num2][0] == nSelectObjNow())
				{
					nSelectObjNow() = array[num2][2];
					break;
				}
			}
		}
		while (!IsPointActive());
	}
	else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT))
	{
		if (4 <= nSelectObjNow() && nSelectObjNow() <= 6)
		{
			do
			{
				nSelectObjNow()--;
				if (nSelectObjNow() < 4)
				{
					nSelectObjNow() = 6;
				}
			}
			while (!IsPointActive());
		}
		else
		{
			if (7 > nSelectObjNow() || nSelectObjNow() > 9)
			{
				return;
			}
			do
			{
				nSelectObjNow()--;
				if (nSelectObjNow() < 7)
				{
					nSelectObjNow() = 9;
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
		if (4 <= nSelectObjNow() && nSelectObjNow() <= 6)
		{
			do
			{
				nSelectObjNow()++;
				if (nSelectObjNow() > 6)
				{
					nSelectObjNow() = 4;
				}
			}
			while (!IsPointActive());
		}
		else
		{
			if (7 > nSelectObjNow() || nSelectObjNow() > 9)
			{
				return;
			}
			do
			{
				nSelectObjNow()++;
				if (nSelectObjNow() > 9)
				{
					nSelectObjNow() = 7;
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
	if (nSelectObjNow() != nSelectObjOld() && nSelectObjNow() + nSelectObjOld() != 22 && nSelectObjNow() + nSelectObjOld() != 23)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE01_SELECT);
	}
	if (nSelectObjNow() != nSelectObjOld())
	{
		AllWaitState();
		_ObjProc();
		if (nSelectObjNow() == 12)
		{
			pHitArray(10u)->ResetAnm();
			pHitArray(11u)->ResetAnm();
		}
		else if (nSelectObjNow() == 3)
		{
			pHitArray(3u)->ResetAnm();
			pTaskConsMenuStage(nStageNo())->ResetAnm();
		}
	}
	if (nSelectObjNow() == 12)
	{
		pHitArray(10u)->PointState();
		pHitArray(11u)->PointState();
	}
	else if (nSelectObjNow() == 3)
	{
		pHitArray(3u)->PointState();
		pTaskConsMenuStage(nStageNo())->PointState();
	}
	else
	{
		pHitArray((uint)nSelectObjNow())->PointState();
	}
}

void cTaskConsMenuMan::DecideCheck(int padNum)
{
	int array[13][2] = {{0, -1}, {1, 2}, {2, 3}, {3, 4}, {4, 7}, {5, 7}, {6, 7}, {7, 7}, {8, 7}, {9, 7}, {10, 5}, {11, 6}, {-1, -1}};
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
			if (array[num][0] == nSelectObjNow() && array[num][1] != -1)
			{
				nState() = array[num][1];
			}
		}
	}
	else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT))
	{
		if (nSelectObjNow() == 12)
		{
			nState() = 5;
		}
	}
	else if ((InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT)) && nSelectObjNow() == 12)
	{
		nState() = 6;
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

bool cTaskConsMenuMan::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	switch (nState())
	{
	case 2:
		// DOL 0x8005d348 state 2 (dropped by the C#): Back decided; cancel sound when the trigger word has bit 0x80, else the decide sound
		if (nStep() == 0)
		{
			pTaskConsMenuBack()->DecideState();
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
		break;
	case 3:
		if (nStep() == 0)
		{
			pTaskConsMenuRule()->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			nStep()++;
		}
		break;
	case 4:
		if (nStep() == 0)
		{
			pTaskConsMenuMap()->DecideState();
			pTaskConsMenuStage(nStageNo())->DecideState();
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			nStep()++;
		}
		else if (nStep() != 1)
		{
		}
		break;
	case 5:
	case 6:
		{
			uint nType;
			if (nState() == 6)
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
				if (nState() == 6)
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
				for (uint num = 0u; num < 6; num++)
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
					for (uint num2 = 0u; num2 < 6; num2++)
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
					pTaskConsMenuBg()->nSetWorldNo(nWorldNo());
					pTaskConsMenuBar()->LoadRecord(nWorldNo(), nStageNo());
					pTaskConsMenuMap()->ChangeData(nWorldNo(), nStageNo());
					for (uint num3 = 0u; num3 < 6; num3++)
					{
						pTaskConsMenuStage(num3)->LoadStageData(nWorldNo());
					}
					pTaskConsMenuBg()->AppearState();
					pTaskConsMenuWorld()->AppearState();
					pTaskConsMenuBar()->AppearState();
					pTaskConsMenuMap()->AppearState();
					for (uint num4 = 0u; num4 < 6; num4++)
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
					for (uint num5 = 0u; num5 < 6; num5++)
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
	case 7:
		if (nStep() == 0)
		{
			if (pTaskConsMenuStage(nStageNo())->IsActive())
			{
				pTaskConsMenuStage(nStageNo())->DecideState();
				pTaskConsMenuMap()->DecideState();
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
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
	pTaskConsMenuBack()->AppearState(); // Back (+0x1e0) first, in the order of the DOL's wait-all 0x8005c76c (UNVERIFIED for the appear call)
	pTaskConsMenuRule()->AppearState();
	pTaskConsMenuWorld()->AppearState();
	pTaskConsMenuBar()->AppearState();
	pTaskConsMenuCurtain()->AppearState();
	pTaskConsMenuMap()->AppearState();
	pTaskMenuPacman()->AppearState();
	for (uint num = 0u; num < 6; num++)
	{
		pTaskConsMenuStage(num)->AppearState();
	}
	for (uint num2 = 0u; num2 < 2; num2++)
	{
		pTaskConsMenuArrow(num2)->AppearState();
	}
	if (UnityUI != nullptr)
	{
		UnityUI->SetEnable(true);
	}
	if (UnityUI != nullptr)
	{
		UnityUI->PlayBaseAnim(MotosUIRollHome::EBaseAnim::In);
	}
}

bool cTaskConsMenuMan::IsAppearEnd()
{
	bool flag = true;
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
		flag = pTaskMenuPacman()->IsAppearEndState();
	}
	if (flag)
	{
		for (uint num = 0u; num < 6; num++)
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
	if (nullptr != UnityUI && flag)
	{
		flag = !UnityUI->IsPlayBaseAnim(MotosUIRollHome::EBaseAnim::In);
	}
	return flag;
}

// DOL 0x8005dbd8: the Back task (+0x1e0) -> IsDecideEndState (vtable slot 0x84, same as IsStageDecide); the C# returned false (Unity left through its own UI)
bool cTaskConsMenuMan::IsBackDecide()
{
	return pTaskConsMenuBack() != nullptr && pTaskConsMenuBack()->IsDecideEndState();
}

bool cTaskConsMenuMan::IsStageDecide()
{
	return pTaskConsMenuStage(nStageNo())->IsDecideEndState();
}

bool cTaskConsMenuMan::IsRuleDecide()
{
	return pTaskConsMenuRule()->IsDecideEndState();
}

void cTaskConsMenuMan::Uplode()
{
	KingdomGlobal::PacnRollSys->nSetRound(nWorldNo(), nStageNo());
}

}
