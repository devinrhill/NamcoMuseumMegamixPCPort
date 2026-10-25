#include "kingdom/cKingdomFade.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom
{

bool cKingdomFade::cFadeAnita::SetTexture(int idx, Texture* tex)
{
	if (cKingdomAnita::SetTexture(idx, tex))
	{
		return true;
	}
	return false;
}

void cKingdomFade::Init()
{
	m_nType = 0u;
	m_fSpeed = 1.0f;
	m_bEnd = true;
	int drawLayer = 0; // TODO(unity): drawLayer = LayerMask.NameToLayer("FrontFrontUI")
	// TODO(unity): m_targetCamera = GameObject.Find("FrontFrontUICamera").GetComponent<Camera>();
	m_Anita = new cKingdomFade::cFadeAnita();
	m_Anita->RegistData("m00a_fade01.anp", drawLayer);
	m_Anita->SetState(2u, false);
}

void cKingdomFade::Term()
{
}

int cKingdomFade::Type2AnimNo(uint type)
{
	switch ((EFadeKind)type)
	{
	case EFadeKind::eFADE_TYPE_IN_WHITE:
		return 1;
	case EFadeKind::eFADE_TYPE_IN_BLACK:
		return 3;
	case EFadeKind::eFADE_TYPE_IN_G01:
		return 5;
	case EFadeKind::eFADE_TYPE_IN_G02:
		return 7;
	case EFadeKind::eFADE_TYPE_IN_G03:
		return 9;
	case EFadeKind::eFADE_TYPE_IN_G04:
		return 13;
	case EFadeKind::eFADE_TYPE_IN_G07:
		return 15;
	case EFadeKind::eFADE_TYPE_OUT_WHITE:
		return 0;
	case EFadeKind::eFADE_TYPE_OUT_BLACK:
		return 2;
	case EFadeKind::eFADE_TYPE_OUT_G01:
		return 4;
	case EFadeKind::eFADE_TYPE_OUT_G02:
		return 6;
	case EFadeKind::eFADE_TYPE_OUT_G03:
		return 8;
	case EFadeKind::eFADE_TYPE_OUT_G04:
		return 12;
	case EFadeKind::eFADE_TYPE_OUT_G07:
		return 14;
	default:
		// TODO(unity): Debug.LogError("invalid type");
		return 0;
	}
}

cKingdomFade::cKingdomFade()
{
	Init();
}

cKingdomFade::~cKingdomFade()
{
	Term();
}

bool cKingdomFade::Fade(uint type, int msec)
{
	CustomKingdomUtil::Assert(!m_Anita->GetData().empty(), "nullpo");
	m_Anita->SetAnimNo(Type2AnimNo(type));
	CustomKingdomUtil::Assert(!m_Anita->IsState(262144u), "error");
	m_Anita->SetState(2u);
	m_fSpeed = 500.0f / (float)msec;
	m_nType = type;
	m_bEnd = false;
	if (IsTypeOfFadeOut())
	{
		(void)(65537);
	}
	// TODO(unity): m_targetCamera.enabled = true;
	return true;
}

bool cKingdomFade::IsTypeOfFadeIn()
{
	if (m_nType != 0)
	{
		return !IsTypeOfFadeOut();
	}
	return false;
}

bool cKingdomFade::IsTypeOfFadeOut()
{
	if (m_nType != 0)
	{
		return (m_nType & 0x10000) != 0;
	}
	return false;
}

bool cKingdomFade::IsEnd()
{
	return m_bEnd;
}

void cKingdomFade::SetTexture(Texture* tex)
{
	if (tex != nullptr)
	{
		m_Anita->SetTexture(1, tex);
	}
}

void cKingdomFade::UpdateFrame()
{
	CustomKingdomUtil::Assert(!m_Anita->GetData().empty(), "nullpo");
	if (m_Anita->IsState(65536u))
	{
		m_bEnd = true;
		// TODO(unity): m_targetCamera.enabled = IsTypeOfFadeOut();
	}
	else
	{
		m_Anita->UpdateFrame(m_fSpeed);
		// TODO(unity): m_targetCamera.enabled = true;
	}
}

void cKingdomFade::Draw()
{
	if (!IsTypeOfFadeIn() || !IsEnd())
	{
		m_Anita->Draw();
	}
}

}
