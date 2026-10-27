#include "platform/devtools.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsoleBase::cTaskConsoleBase()
{
	SetName("cTaskConsoleBase");
	Init();
}

cTaskConsoleBase::~cTaskConsoleBase()
{
	Term();
}

uint cTaskConsoleBase::nGetState()
{
	return m_nState;
}

uint cTaskConsoleBase::nGetStep()
{
	return m_nStep;
}

void cTaskConsoleBase::Init()
{
	::Constants::jetGeoVecUnit(m_vTrans);
	m_nState = 0u;
	m_nStep = 0u;
	m_nStateNew = 0u;
	m_nStepNew = 0u;
	nDrawPriority(2147483648u);
}

void cTaskConsoleBase::Term()
{
}

bool cTaskConsoleBase::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	return cTaskAnita::Execute(pParam);
}

void cTaskConsoleBase::Draw(int nStatus, cJetTask* pParam)
{
	if (IsManagerVisible() && !devtools::G03HideHud())   // devtools: the Pac-N-Roll editor's "Hide the HUD" (not 1:1)
	{
		cTaskAnita::Draw(nStatus, pParam);
	}
	UpdateState();
}

bool cTaskConsoleBase::Regist(uint anita, string key)
{
	Regist(anita, key, 13);
	return true;
}

bool cTaskConsoleBase::Regist(uint anita, string key, int pat, int lay)
{
	pAnita(anita)->RegistData(ResourceDatabase::GetPath(key));
	pAnita(anita)->SetAnimNo(0);
	::Constants::jetGeoVecZero(pAnita(anita)->vTrans());
	Invisible(anita);
	return SetPosition(anita, pat, lay);
}

bool cTaskConsoleBase::RegistNotSetPos(uint anita, string key)
{
	pAnita(anita)->RegistData(ResourceDatabase::GetPath(key));
	pAnita(anita)->SetAnimNo(0);
	::Constants::jetGeoVecZero(pAnita(anita)->vTrans());
	Invisible(anita);
	return true;
}

bool cTaskConsoleBase::SetPosition(uint anita, int pat, int lay)
{
	CustomKingdomUtil::Assert(lay >= 0, "idx error");
	Vector4 pos = Vector4();
	if (!pAnita(anita)->GetLayerPos(pos, (uint)lay, pat))
	{
		return false;
	}
	pAnita(anita)->vTrans()[0] = pos[0];
	pAnita(anita)->vTrans()[1] = pos[1];
	return true;
}

bool cTaskConsoleBase::SetAnim(uint anita, int ano)
{
	return pAnita(anita)->SetAnimNo(ano);
}

bool cTaskConsoleBase::ChangeAnim(uint anita, int ano)
{
	return pAnita(anita)->SetAnimNo(ano, false);
}

bool cTaskConsoleBase::IsAnimEnd(uint anita)
{
	return pAnita(anita)->IsState(65536u);
}

bool cTaskConsoleBase::IsVisible(uint anita)
{
	return pAnita(anita)->IsState(1u);
}

void cTaskConsoleBase::Visible(uint anita)
{
	pAnita(anita)->SetState(1u);
}

void cTaskConsoleBase::Invisible(uint anita)
{
	pAnita(anita)->SetState(1u, false);
}

void cTaskConsoleBase::SetState(uint num)
{
	if (m_nState != num)
	{
		SetStep(0u);
		m_nStateNew = num;
	}
}

void cTaskConsoleBase::SetStep(uint num)
{
	if (m_nStep != num && m_nState == m_nStateNew)
	{
		m_nStepNew = num;
	}
}

void cTaskConsoleBase::UpdateState()
{
	m_nState = m_nStateNew;
	m_nStep = m_nStepNew;
}

bool cTaskConsoleBase::IsManagerVisible()
{
	return true; // native: the HUD manager (cTaskConsoleMan) is not instantiated, the HUD is always visible
}

}
