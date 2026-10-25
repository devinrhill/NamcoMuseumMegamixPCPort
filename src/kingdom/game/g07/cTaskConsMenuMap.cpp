#include "kingdom/game/g07/cTaskConsMenuMap.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

uint& cTaskConsMenuMap::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMenuMap::nStageNo()
{
	return m_nStageNo;
}

cTaskConsMenuMap::~cTaskConsMenuMap()
{
}

void cTaskConsMenuMap::ChangeData(uint nWorldNo, uint nStageNo)
{
	CustomKingdomUtil::Assert(nWorldNo < 5, "error");
	CustomKingdomUtil::Assert(nStageNo < 7, "error");
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
}

cTaskConsMenuMap::cTaskConsMenuMap(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo)
	: cTaskConsMenuBase(pConsMan)
{
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_STAGE"));
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsMenuMap::nGetMapAnm(uint nState)
{
	int obj[5][7][5] = {{{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}, {10, 11, 12, 13, 14}, {15, 16, 17, 18, 19}, {20, 21, 22, 23, 24}, {25, 26, 27, 28, 29}, {150, 151, 152, 153, 154}}, {{30, 31, 32, 33, 34}, {35, 36, 37, 38, 39}, {40, 41, 42, 43, 44}, {45, 46, 47, 48, 49}, {50, 51, 52, 53, 54}, {55, 56, 57, 58, 59}, {155, 156, 157, 158, 159}}, {{60, 61, 62, 63, 64}, {65, 66, 67, 68, 69}, {70, 71, 72, 73, 74}, {75, 76, 77, 78, 79}, {80, 81, 82, 83, 84}, {85, 86, 87, 88, 89}, {160, 161, 162, 163, 164}}, {{90, 91, 92, 93, 94}, {95, 96, 97, 98, 99}, {100, 101, 102, 103, 104}, {105, 106, 107, 108, 109}, {110, 111, 112, 113, 114}, {115, 116, 117, 118, 119}, {165, 166, 167, 168, 169}}, {{120, 121, 122, 123, 124}, {125, 126, 127, 128, 129}, {130, 131, 132, 133, 134}, {135, 136, 137, 138, 139}, {140, 141, 142, 143, 144}, {145, 146, 147, 148, 149}, {170, 171, 172, 173, 174}}};
	CustomKingdomUtil::Assert(nWorldNo() < 5, "error");
	CustomKingdomUtil::Assert(nStageNo() < 7, "error");
	return obj[nWorldNo()][nStageNo()][nState];
}

// DOL 0x801c8e4c: hit test of anita 0, pattern 0 (the C# returned false: Unity used colliders)
bool cTaskConsMenuMap::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 0);
}

bool cTaskConsMenuMap::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(0u));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsMenuMap::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetMapAnm(1u), false);
}

void cTaskConsMenuMap::PointExec()
{
	pAnita(0u)->SetAnimNo(nGetMapAnm(2u), false);
}

bool cTaskConsMenuMap::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(3u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsMenuMap::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(4u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetState(1u, false);
		return true;
	}
	return false;
}

}
