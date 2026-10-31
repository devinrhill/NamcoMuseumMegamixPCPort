#include "kingdom/game/g07/cTaskConsPlayerNumber.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskConsPlayerNumber::cTaskConsPlayerNumber()
{
	Init();
}

cTaskConsPlayerNumber::~cTaskConsPlayerNumber()
{
	Term();
}

bool& cTaskConsPlayerNumber::bExec()
{
	return m_bExec;
}

uint& cTaskConsPlayerNumber::nPlayerNo()
{
	return m_nPlayerNo;
}

cTaskAnita* cTaskConsPlayerNumber::Construct()
{
	return new cTaskConsPlayerNumber();
}

void cTaskConsPlayerNumber::Init()
{
	m_nPlayerNo = 0u;
	m_bExec = false;
	int array[1] = {178};
	for (uint num = 0u; num < 1; num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num)->SetAnimNo(array[num]);
		pAnita(num)->SetState(3u, false);
	}
}

void cTaskConsPlayerNumber::Term()
{
}

// DOL 0x801030f0 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskConsPlayerNumber::Execute(cJetTask* pParam)
{
	if (m_bExec)
	{
		int array[4] = {179, 182, 185, 188};
		Visible(0u);
		SetAnmDiffCurrent(0u, array[m_nPlayerNo]);
		::Constants::jetGeoVecCopy(pAnita(0u)->vTrans(), vTrans());
	}
	else
	{
		int array2[4] = {180, 183, 186, 189};
		SetAnmDiffCurrent(0u, array2[m_nPlayerNo]);
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
