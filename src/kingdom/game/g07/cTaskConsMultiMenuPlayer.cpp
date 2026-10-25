#include "kingdom/game/g07/cTaskConsMultiMenuPlayer.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

const int cTaskConsMultiMenuPlayer::FONT_BUF_SIZE = 512;
const float cTaskConsMultiMenuPlayer::FONT_SIZE = 18.0f;
const int cTaskConsMultiMenuPlayer::SET_FONT_POS_PAT = 2;
const uint cTaskConsMultiMenuPlayer::SET_FONT_POS_LAY[4] = {3u, 2u, 1u, 0u};
const int cTaskConsMultiMenuPlayer::SET_MII_WND_PAT[4] = {24, 25, 26, 27};
const uint cTaskConsMultiMenuPlayer::SET_MII_WND_LAY[3] = {0u, 1u, 2u};

uint& cTaskConsMultiMenuPlayer::nPlayerNo()
{
	return m_nPlayerNo;
}

sMatchingData::InfoData*& cTaskConsMultiMenuPlayer::pInfo()
{
	return m_pInfo;
}

cTaskConsMultiMenuPlayer::~cTaskConsMultiMenuPlayer()
{
}

cTaskConsMultiMenuPlayer::cTaskConsMultiMenuPlayer(cTaskConsMenuManBase* const& pConsMan, const uint& nPlayerNo, sMatchingData::InfoData* pInfo)
	: cTaskConsMenuBase(pConsMan)
{
	this->pInfo() = pInfo;
	this->nPlayerNo() = nPlayerNo;
}

void cTaskConsMultiMenuPlayer::SetData()
{
	nDrawPriority(2147483657u + nPlayerNo());
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetWndAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsMultiMenuPlayer::nGetWndAnmNo(int nAnmState)
{
	int obj[4][3] = {{99, 100, 101}, {102, 103, 104}, {105, 106, 107}, {108, 109, 110}};
	CustomKingdomUtil::Assert(0 <= nPlayerNo() && nPlayerNo() < 4, "Player Over");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "State Over");
	return obj[nPlayerNo()][nAnmState];
}

bool cTaskConsMultiMenuPlayer::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsMultiMenuPlayer::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetWndAnmNo(1), false);
}

void cTaskConsMultiMenuPlayer::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
}

bool cTaskConsMultiMenuPlayer::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	return cTaskConsMenuBase::Execute(pParam);
}

}
