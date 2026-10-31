#include "kingdom/game/g03/cTaskConsMenuStage.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include <SSaveDataPacnRoll.hpp>

namespace kingdom::game::g03
{

const int cTaskConsMenuStage::ANM_CROWN_LIST[5] = {51, 52, -1, -1, 53};
const int cTaskConsMenuStage::HIT_PAT = 15;
const int cTaskConsMenuStage::SET_STAGE_PAT[6] = {6, 7, 8, 9, 10, 11};
const uint cTaskConsMenuStage::ANM_BOSS_INDEX = 5u;

bool& cTaskConsMenuStage::bValid()
{
	return m_bValid;
}

bool& cTaskConsMenuStage::bActive()
{
	return m_bActive;
}

uint& cTaskConsMenuStage::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMenuStage::nStageNo()
{
	return m_nStageNo;
}

int& cTaskConsMenuStage::nHitPat()
{
	return m_nHitPat;
}

bool cTaskConsMenuStage::IsActive()
{
	return bActive();
}

bool cTaskConsMenuStage::IsValid()
{
	return bValid();
}

cTaskConsMenuStage::cTaskConsMenuStage(cTaskConsMenuManBase* pConsMan, uint nWorldNo, uint nStageNo)
	: cTaskConsMenuBase(pConsMan), ANM_LIST{{{{54, 55, 56, 57, 58}, {62, 63, 64, 65, 66}, {70, 71, 72, 73, 74}, {78, 79, 80, 81, 82}, {86, 87, 88, 89, 90}, {94, 95, 96, 97, 98}}, {{99, 100, 101, 102, 103}, {107, 108, 109, 110, 111}, {115, 116, 117, 118, 119}, {123, 124, 125, 126, 127}, {131, 132, 133, 134, 135}, {139, 140, 141, 142, 143}}, {{144, 145, 146, 147, 148}, {152, 153, 154, 155, 156}, {160, 161, 162, 163, 164}, {168, 169, 170, 171, 172}, {176, 177, 178, 179, 180}, {184, 185, 186, 187, 188}}, {{189, 190, 191, 192, 193}, {197, 198, 199, 200, 201}, {205, 206, 207, 208, 209}, {213, 214, 215, 216, 217}, {221, 222, 223, 224, 225}, {229, 230, 231, 232, 233}}, {{234, 235, 236, 237, 238}, {242, 243, 244, 245, 246}, {250, 251, 252, 253, 254}, {258, 259, 260, 261, 262}, {266, 267, 268, 269, 270}, {274, 275, 276, 277, 278}}}, {{{-1, -1, -1, -1, -1}, {59, 60, 60, -1, 61}, {67, 68, 68, -1, 69}, {75, 76, 76, -1, 77}, {83, 84, 84, -1, 85}, {91, 92, 92, -1, 93}}, {{-1, -1, -1, -1, -1}, {104, 105, 105, -1, 106}, {112, 113, 113, -1, 114}, {120, 121, 121, -1, 122}, {128, 129, 129, -1, 130}, {136, 137, 137, -1, 138}}, {{-1, -1, -1, -1, -1}, {149, 150, 150, -1, 151}, {157, 158, 158, -1, 159}, {165, 166, 166, -1, 167}, {173, 174, 174, -1, 175}, {181, 182, 182, -1, 183}}, {{-1, -1, -1, -1, -1}, {194, 195, 195, -1, 196}, {202, 203, 203, -1, 204}, {210, 211, 211, -1, 212}, {218, 219, 219, -1, 220}, {226, 227, 227, -1, 228}}, {{-1, -1, -1, -1, -1}, {239, 240, 240, -1, 241}, {247, 248, 248, -1, 249}, {255, 256, 256, -1, 257}, {263, 264, 264, -1, 265}, {271, 272, 272, -1, 273}}}}
{
	CustomKingdomUtil::Assert(nWorldNo < 5, "error");
	CustomKingdomUtil::Assert(0 <= nStageNo && nStageNo < 7, "StageNo Over");
	nDrawPriority(2147483651u);
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
	bActive() = true;
	bValid() = true;
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo(nGetStageAnm(1u));
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, SET_STAGE_PAT[this->nStageNo()]);
	nHitPat() = HIT_PAT;
}

cTaskConsMenuStage::~cTaskConsMenuStage()
{
}

int cTaskConsMenuStage::nGetStageAnm(uint nState)
{
	uint num = KingdomGlobal::PacnRollSys->nStageMax(nWorldNo()) - 1;
	if (nWorldNo() == 4)
	{
		num--;
	}
	uint num2 = ((nStageNo() == num && nWorldNo() != 0) ? ANM_BOSS_INDEX : nStageNo());
	return ANM_LIST[(!bActive()) ? 1u : 0u][nWorldNo()][num2][nState];
}

bool cTaskConsMenuStage::IsDpdHit(float x, float y)
{
	// DOL 0x8005e8d0 [audited: described by the 43rd-session pass]: valid (+0x1d8) and active (+0x1d9), pattern nHitPat (+0x1e4) of anita 0
	return IsValid() && IsActive() && DpdHitPat(x, y, nHitPat());
}

void cTaskConsMenuStage::WaitExec()
{
	nDrawPriority(2147483651u);
	pAnita(0u)->SetAnimNo(nGetStageAnm(1u), false);
}

void cTaskConsMenuStage::PointExec()
{
	nDrawPriority(2147483653u);
	pAnita(0u)->SetAnimNo(nGetStageAnm(2u), false);
}

bool cTaskConsMenuStage::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(0u));
		if (IsValid())
		{
			pAnita(0u)->SetState(1u);
		}
		else
		{
			pAnita(0u)->SetState(1u, false);
		}
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool flag = true;
		if (flag)
		{
			flag = pAnita(0u)->IsState(65536u);
		}
		if (flag)
		{
			return true;
		}
	}
	return false;
}

bool cTaskConsMenuStage::DecideExec()
{
	nDrawPriority(2147483653u);
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(3u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsMenuStage::DeleteExec()
{
	nDrawPriority(2147483651u);
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(4u));
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool flag = true;
		if (flag)
		{
			flag = pAnita(0u)->IsState(65536u);
		}
		if (flag)
		{
			return true;
		}
	}
	return false;
}

void cTaskConsMenuStage::LoadStageData(uint nWorldNo)
{
	this->nWorldNo() = nWorldNo;
	nStageNo();
	uint num = KingdomGlobal::PacnRollSys->nStageMax(nWorldNo);
	if (nWorldNo == 4)
	{
		num--;
	}
	if (nStageNo() < num)
	{
		bValid() = true;
		SSaveDataPacnRoll::sStageRecord _out = SSaveDataPacnRoll::sStageRecord();
		uint round = KingdomGlobal::PacnRollSys->nWorldStage(nWorldNo, nStageNo());
		if (KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, round))
		{
			bActive() = _out.bFree;
		}
		else
		{
			bActive() = false;
		}
	}
	else
	{
		bValid() = false;
		bActive() = false;
	}
}

}
