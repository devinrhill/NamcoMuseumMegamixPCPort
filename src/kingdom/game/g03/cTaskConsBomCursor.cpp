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

bool cTaskConsBomCursor::IsInScreen()
{
	float num = 0.0f;
	float num2 = 0.0f; // TODO(unity): num2 = Screen.width
	float num3 = 0.0f; // TODO(unity): num3 = Screen.height
	Camera* mainCam = Constants::PACN_APP()->pTaskCamera()->get_MainCam();
	// TODO(unity): m_vTrans = mainCam.WorldToScreenPoint(m_vTargetTrans.GetUnityPos());
	if (m_vTrans[3] == 0.0f)
	{
		m_vTrans[3] = 1.0f;
	}
	(void)(m_vTrans);
	float num4 = 1.0f / m_vTrans[3];
	Vector4 v = Vector4(num4, num4, num4, 1.0f);
	::Constants::jetGeoVecMult(m_vTrans, m_vTrans, v);
	m_vTrans.z *= -1.0f;
	if (0.0f <= m_vTrans[0] && m_vTrans[0] <= num2 && num <= m_vTrans[1] && m_vTrans[1] <= num3 && 0.0f >= m_vTrans[2])
	{
		m_vTrans[1] *= -1.0f;
		Vector4 v2 = Vector4(0.0f - num2 / 2.0f, num3 / 2.0f, 0.0f, 0.0f);
		::Constants::jetGeoVecAdd(m_vTrans, m_vTrans, v2);
		int num5 = 960;
		::Constants::jetGeoVecMult(m_vTrans, m_vTrans, (Vector4)Vector3((float)num5 / num2, (float)num5 / num2, 0.0f));
		m_vTrans.y *= -1.0f;
		return true;
	}
	return false;
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
