#include "kingdom/cKingdomFade.hpp"

#include "CustomKingdomUtil.hpp"
#include "GamePad.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "platform/platform.hpp"

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

// DOL 0x80016380 [audited]
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

// DOL 0x800164a0 [audited]
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
	case (EFadeKind)8:
		return 17;
	case (EFadeKind)0x10008:
		return 16;
	default:
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

// DOL 0x800165c8 [audited+dolphin]
bool cKingdomFade::Fade(uint type, int msec)
{
	CustomKingdomUtil::Assert(!m_Anita->GetData().empty(), "nullpo");
	m_Anita->SetAnimNo(Type2AnimNo(type));
	CustomKingdomUtil::Assert(!m_Anita->IsState(262144u), "error");
	m_Anita->SetState(2u);
	// 0x8000d7d4: msec * 6000 / 100000 = frames; the clip's own length plays in that many frames (FUN_80141bd4(-1) = current clip), not always 30
	const float frames = (float)msec * 6000.0f / 100000.0f;
	m_fSpeed = (frames != 0.0f) ? m_Anita->GetFrameNum() / frames : 1.0f;
	m_nType = type;
	m_bEnd = false;
	if (IsTypeOfFadeOut())
	{
		// 0x80138e2c: the frame clear colour stays this until the next fade out: white after type 0x10001, else black
		if (type == 0x10001u) platform::SetClearColor(255, 255, 255);
		else platform::SetClearColor(0, 0, 0);
	}
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

// DOL 0x800166f4 [audited]: held 0x400 (HBM open) stops the clip but not the end test; +0x440 (run during HBM) is 0 on every fade
void cKingdomFade::UpdateFrame()
{
	CustomKingdomUtil::Assert(!m_Anita->GetData().empty(), "nullpo");
	if (m_Anita->IsState(65536u))
	{
		m_bEnd = true;
	}
	else if ((GamePad::Get(0).hold & GamePad::PAD_HOME) == 0)
	{
		m_Anita->UpdateFrame(m_fSpeed);
	}
}

// DOL 0x80016774 [audited]
void cKingdomFade::Draw()
{
	if (!IsTypeOfFadeIn() || !IsEnd())
	{
		m_Anita->Draw();
	}
}

}
