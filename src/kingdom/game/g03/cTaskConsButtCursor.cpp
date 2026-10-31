#include "kingdom/game/g03/cTaskConsButtCursor.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsButtCursor::cTaskConsButtCursor()
{
	SetName("cTaskConsButtCursor");
	Init();
}

cTaskConsButtCursor::~cTaskConsButtCursor()
{
	Term();
}

void cTaskConsButtCursor::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_nWaitCnt = 0u;
	m_bRed = false;
	::Constants::jetGeoVecUnit(m_vTrans);
	::Constants::jetGeoVecUnit(m_vTargetTrans);
}

void cTaskConsButtCursor::Term()
{
}

void cTaskConsButtCursor::Start(Vector4 trans)
{
	SetState(1u);
	m_bRed = false;
	SetAnim(0u, 164);
	Visible(0u);
	::Constants::jetGeoVecCopy(m_vTargetTrans, trans);
}

bool cTaskConsButtCursor::Red()
{
	if (m_bRed)
	{
		return false;
	}
	SetState(1u);
	m_bRed = true;
	SetAnim(0u, 167);
	Visible(0u);
	return true;
}

void cTaskConsButtCursor::End()
{
	if (nGetState() != 0 && nGetState() != 3)
	{
		if (m_bRed)
		{
			SetAnim(0u, 169);
		}
		else
		{
			SetAnim(0u, 166);
		}
		SetState(3u);
	}
}

bool cTaskConsButtCursor::Execute(cJetTask* pParam)
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
				SetAnim(0u, 168);
			}
			else
			{
				SetAnim(0u, 165);
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
bool cTaskConsButtCursor::IsInScreen()
{
	return WorldToHud(m_vTargetTrans, m_vTrans);
}

void cTaskConsButtCursor::UpdatePosition()
{
	::Constants::jetGeoVecCopy(pAnita(0u)->vTrans(), m_vTrans);
}

void cTaskConsButtCursor::Update(Vector4 trans)
{
	::Constants::jetGeoVecCopy(m_vTargetTrans, trans);
}

}
