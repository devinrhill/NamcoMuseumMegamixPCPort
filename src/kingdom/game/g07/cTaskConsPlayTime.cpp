#include "kingdom/game/g07/cTaskConsPlayTime.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskConsPlayTime::cTaskConsPlayTime()
{
	Init();
}

cTaskConsPlayTime::~cTaskConsPlayTime()
{
	Free();
}

void cTaskConsPlayTime::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

Vector4& cTaskConsPlayTime::vOffset()
{
	return m_vOffset;
}

cTaskAnita* cTaskConsPlayTime::Construct()
{
	return new cTaskConsPlayTime();
}

void cTaskConsPlayTime::Init()
{
	::Constants::jetGeoVecUnit(m_vOffset);
	m_nTimer = 0u;
	for (int i = 0; i < (int)std::size(m_aExecNumbers); i++)
	{
		m_aExecNumbers[i] = new Ref<int>();
	}
	for (uint num = 0u; num < 3; num++)
	{
		m_pItemNumbers[num] = new cTaskConsItemNumber(2);
		m_pItemNumbers[num]->pExecNumber(m_aExecNumbers[num]);
		m_pItemNumbers[num]->SetFlag(70u, true);
		ConnectChild(m_pItemNumbers[num]);
	}
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(0u)->SetAnimNo(61);
	pAnita(0u)->SetState(3u, false);
	pAnita(1u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(1u)->SetAnimNo(61);
	pAnita(1u)->SetState(3u, false);
}

void cTaskConsPlayTime::Term()
{
}

bool cTaskConsPlayTime::Execute(cJetTask* pParam)
{
	m_nTimer++;
	m_nTimer %= 42u;
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 8)
	{
		Vector4 pos = Vector4();
		Vector4 pos2 = Vector4();
		Vector4 pos3 = Vector4();
		Vector4 pos4 = Vector4();
		Vector4 pos5 = Vector4();
		pAnita(0u)->GetLayerPos(vTrans(), 7u, 63);
		pAnita(0u)->GetLayerPos(pos, 5u, 63);
		pAnita(0u)->GetLayerPos(pos2, 2u, 63);
		pAnita(0u)->GetLayerPos(pos3, 7u, 63);
		pAnita(0u)->GetLayerPos(pos4, 4u, 63);
		pAnita(0u)->GetLayerPos(pos5, 1u, 63);
		::Constants::jetGeoVecAdd(vTrans(), vTrans(), m_vOffset);
		::Constants::jetGeoVecAdd(pos, pos, m_vOffset);
		::Constants::jetGeoVecAdd(pos2, pos2, m_vOffset);
		::Constants::jetGeoVecAdd(pos3, pos3, m_vOffset);
		::Constants::jetGeoVecAdd(pos4, pos4, m_vOffset);
		::Constants::jetGeoVecAdd(pos5, pos5, m_vOffset);
		uint gamePlayTime = KingdomGlobal::g_pMotosGame->GetGamePlayTime();
		m_aExecNumbers[0]->Value = (int)(gamePlayTime / 1000 / 60);
		m_aExecNumbers[1]->Value = (int)(gamePlayTime / 1000 % 60);
		m_aExecNumbers[2]->Value = (int)(gamePlayTime / 10 % 100);
		pAnita(0u)->vTrans()[0] = pos[0];
		pAnita(0u)->vTrans()[1] = pos[1];
		pAnita(0u)->SetState(3u);
		pAnita(1u)->vTrans()[0] = pos2[0];
		pAnita(1u)->vTrans()[1] = pos2[1];
		pAnita(1u)->SetState(3u);
		for (uint num = 0u; num < 3; num++)
		{
			m_pItemNumbers[num]->bExec() = true;
		}
		int num2 = (int)m_pItemNumbers[0]->pAnita(0u)->GetFrameNum();
		(void)((long)m_nTimer % (long)num2);
		m_pItemNumbers[0]->X() = pos3[0];
		m_pItemNumbers[0]->Y() = pos3[1];
		m_pItemNumbers[1]->X() = pos4[0];
		m_pItemNumbers[1]->Y() = pos4[1];
		m_pItemNumbers[2]->X() = pos5[0];
		m_pItemNumbers[2]->Y() = pos5[1];
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
