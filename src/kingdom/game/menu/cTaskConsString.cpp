#include "kingdom/game/menu/cTaskConsString.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

int& cTaskConsString::nStringIdx()
{
	return m_nStringIdx;
}

cTaskConsString::~cTaskConsString()
{
}

void cTaskConsString::nSetStringIdx(int nIdx)
{
	CustomKingdomUtil::Assert(0 <= nIdx && nIdx < 3, "error");
	nStringIdx() = nIdx;
}

int cTaskConsString::nGetStringIdx()
{
	return nStringIdx();
}

cTaskConsString::cTaskConsString(int nId, string Key)
	: cTaskConsMatching(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
	nStringIdx() = -1;
}

int cTaskConsString::nGetStringAnmNo(int nState)
{
	int obj[3][3] = {{7, 8, 9}, {10, 11, 12}, {13, 14, 15}};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	CustomKingdomUtil::Assert(0 <= nStringIdx() && nStringIdx() < 3, "error");
	return obj[nStringIdx()][nState];
}

void cTaskConsString::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetStringAnmNo(1), false);
}

bool cTaskConsString::AppearProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStringAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsString::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStringAnmNo(2));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
