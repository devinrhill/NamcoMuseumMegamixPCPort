#include "kingdom/game/g03/cTaskConsBomCursor.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsBomCursor::cTaskConsBomCursor()
{
	SetName("cTaskConsBomCursor");
	Init();
}

cTaskConsBomCursor::~cTaskConsBomCursor()
{
	Term();
}

void cTaskConsBomCursor::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_nWaitCnt = 0u;
	m_bRed = false;
	::Constants::jetGeoVecUnit(m_vTrans);
	::Constants::jetGeoVecUnit(m_vTargetTrans);
}

void cTaskConsBomCursor::Term()
{
}

void cTaskConsBomCursor::Start(Vector4 trans)
{
	SetState(1u);
	m_bRed = false;
	SetAnim(0u, 158);
	Visible(0u);
	::Constants::jetGeoVecCopy(m_vTargetTrans, trans);
}

bool cTaskConsBomCursor::Red()
{
	if (m_bRed)
	{
		return false;
	}
	SetState(1u);
	m_bRed = true;
	SetAnim(0u, 161);
	Visible(0u);
	return true;
}

void cTaskConsBomCursor::End()
{
	if (nGetState() != 0 && nGetState() != 3)
	{
		if (m_bRed)
		{
			SetAnim(0u, 163);
		}
		else
		{
			SetAnim(0u, 160);
		}
		SetState(3u);
	}
}

bool cTaskConsBomCursor::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	switch (nGetState())
	{
	case 1u:
		if (IsAnimEnd(0u))
		{
			if (m_bRed)
			{
				SetAnim(0u, 162);
			}
			else
			{
				SetAnim(0u, 159);
			}
			SetState(2u);
			m_nWaitCnt = 0u;
		}
		break;
	case 3u:
		if (IsAnimEnd(0u))
		{
			Invisible(0u);
			SetState(0u);
		}
		break;
	}
	if (IsInScreen())
	{
		if (nGetState() != 0)
		{
			Visible(0u);
		}
	}
	else
	{
		Invisible(0u);
	}
	UpdatePosition();
	return cTaskConsoleBase::Execute(pParam);
}

// DOL 0x8018a200 [audited: per AUDIT_G03_G07 row] (shared by both cursors): the target through the camera and viewport; the cursor is hidden when it is off screen
bool cTaskConsBomCursor::IsInScreen()
{
	return WorldToHud(m_vTargetTrans, m_vTrans);
}

void cTaskConsBomCursor::UpdatePosition()
{
	::Constants::jetGeoVecCopy(pAnita(0u)->vTrans(), m_vTrans);
}

void cTaskConsBomCursor::Update(Vector4 trans)
{
	::Constants::jetGeoVecCopy(m_vTargetTrans, trans);
}

}
