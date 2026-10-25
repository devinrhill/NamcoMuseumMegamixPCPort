#include "kingdom/game/g07/cTaskConsMenuStage.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

const int cTaskConsMenuStage::ANM_MOTOS_LIST[5] = {51, 52, -1, -1, 53};
const int cTaskConsMenuStage::HIT_PAT = 15;
const int cTaskConsMenuStage::SET_STAGE_PAT[7] = {5, 6, 7, 8, 9, 10, 11};
const int cTaskConsMenuStage::SET_MOTOS_PAT = 3;
const uint cTaskConsMenuStage::SET_MOTOS_LAY[2] = {1u, 0u};

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

cTaskConsMenuStage::cTaskConsMenuStage(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo)
	: cTaskConsMenuBase(pConsMan), ANM_LIST{{{{54, 55, 56, 57, 58}, {62, 63, 64, 65, 66}, {70, 71, 72, 73, 74}, {78, 79, 80, 81, 82}, {86, 87, 88, 89, 90}, {94, 95, 96, 97, 98}, {102, 103, 104, 105, 106}}, {{107, 108, 109, 110, 111}, {115, 116, 117, 118, 119}, {123, 124, 125, 126, 127}, {131, 132, 133, 134, 135}, {139, 140, 141, 142, 143}, {147, 148, 149, 150, 151}, {155, 156, 157, 158, 159}}, {{160, 161, 162, 163, 164}, {168, 169, 170, 171, 172}, {176, 177, 178, 179, 180}, {184, 185, 186, 187, 188}, {192, 193, 194, 195, 196}, {200, 201, 202, 203, 204}, {208, 209, 210, 211, 212}}, {{213, 214, 215, 216, 217}, {221, 222, 223, 224, 225}, {229, 230, 231, 232, 233}, {237, 238, 239, 240, 241}, {245, 246, 247, 248, 249}, {253, 254, 255, 256, 257}, {261, 262, 263, 264, 265}}, {{266, 267, 268, 269, 270}, {274, 275, 276, 277, 278}, {282, 283, 284, 285, 286}, {290, 291, 292, 293, 294}, {298, 299, 300, 301, 302}, {306, 307, 308, 309, 310}, {314, 315, 316, 317, 318}}}, {{{-1, -1, -1, -1, -1}, {59, 60, 60, -1, 61}, {67, 68, 68, -1, 69}, {75, 76, 76, -1, 77}, {83, 84, 84, -1, 85}, {91, 92, 92, -1, 93}, {99, 100, 100, -1, 101}}, {{-1, -1, -1, -1, -1}, {112, 113, 113, -1, 114}, {120, 121, 121, -1, 122}, {128, 129, 129, -1, 130}, {136, 137, 137, -1, 138}, {144, 145, 145, -1, 146}, {152, 153, 153, -1, 154}}, {{-1, -1, -1, -1, -1}, {165, 166, 166, -1, 167}, {173, 174, 174, -1, 175}, {181, 182, 182, -1, 183}, {189, 190, 190, -1, 191}, {197, 198, 198, -1, 199}, {205, 206, 206, -1, 207}}, {{-1, -1, -1, -1, -1}, {218, 219, 219, -1, 220}, {226, 227, 227, -1, 228}, {234, 235, 235, -1, 236}, {242, 243, 243, -1, 244}, {250, 251, 251, -1, 252}, {258, 259, 259, -1, 260}}, {{-1, -1, -1, -1, -1}, {271, 272, 272, -1, 273}, {279, 280, 280, -1, 281}, {287, 288, 288, -1, 289}, {295, 296, 296, -1, 297}, {303, 304, 304, -1, 305}, {311, 312, 312, -1, 313}}}}
{
	CustomKingdomUtil::Assert(nWorldNo < 5, "error");
	CustomKingdomUtil::Assert(nStageNo < 7, "error");
	nDrawPriority(2147483653u);
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
	bActive() = true;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetStageAnm(1u));
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, SET_STAGE_PAT[this->nStageNo()]);
	uint num = 1u;
	uint num2 = 0u;
	while (num <= 2)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
		pAnita(num)->SetAnimNo(nGetStageAnm(1u));
		pAnita(num)->SetState(1u, false);
		pAnita(num)->GetLayerPos(pAnita(num)->vTrans(), SET_MOTOS_LAY[num2], SET_MOTOS_PAT);
		::Constants::jetGeoVecAdd(pAnita(num)->vTrans(), pAnita(num)->vTrans(), pAnita(0u)->vTrans());
		num++;
		num2++;
	}
	nHitPat() = HIT_PAT;
}

cTaskConsMenuStage::~cTaskConsMenuStage()
{
}

int cTaskConsMenuStage::nGetStageAnm(uint nState)
{
	CustomKingdomUtil::Assert(nWorldNo() < 5, "error");
	CustomKingdomUtil::Assert(nStageNo() < 7, "error");
	return ANM_LIST[(!bActive()) ? 1u : 0u][nWorldNo()][nStageNo()][nState];
}

void cTaskConsMenuStage::SetAnmMotos(uint nType)
{
	for (uint num = 1u; num <= 2; num++)
	{
		pAnita(num)->SetAnimNo(ANM_MOTOS_LIST[nType]);
	}
}

// DOL 0x801ca06c: hit test of anita 0, only while active (+0x1d8: the stage is open), pattern +0x1e4 (the C# returned false: Unity used colliders)
bool cTaskConsMenuStage::IsDpdHit(float x, float y)
{
	return bActive() && DpdHitPat(x, y, nHitPat());
}

void cTaskConsMenuStage::WaitExec()
{
	nDrawPriority(2147483653u);
	pAnita(0u)->SetAnimNo(nGetStageAnm(1u), false);
	SetAnmMotos(1u);
}

void cTaskConsMenuStage::PointExec()
{
	nDrawPriority(2147483655u);
	pAnita(0u)->SetAnimNo(nGetStageAnm(2u), false);
}

bool cTaskConsMenuStage::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(0u));
		pAnita(0u)->SetState(1u);
		SetAnmMotos(0u);
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
			for (uint num = 1u; num <= 2; num++)
			{
				flag = pAnita(num)->IsState(65536u);
				if (!flag)
				{
					break;
				}
			}
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
	nDrawPriority(2147483655u);
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
	nDrawPriority(2147483653u);
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(4u));
		SetAnmMotos(4u);
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
			for (uint num = 1u; num <= 2; num++)
			{
				flag = pAnita(num)->IsState(65536u);
				if (!flag)
				{
					break;
				}
			}
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
	bActive() = KingdomGlobal::MotosSys->IsStageOpen(nWorldNo + 1, nStageNo() + 1);
}

}
