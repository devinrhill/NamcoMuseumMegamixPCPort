#include "kingdom/cTaskCursor.hpp"

#include "GamePad.hpp"
#include "gfx/Gfx.hpp"

namespace kingdom
{

cTaskCursor::cTaskCursor()
{
	SetName("cTaskCursor");
}

cTaskCursor::~cTaskCursor()
{
}

void cTaskCursor::Init(int padNo, const char* layoutName, int idleAnim)
{
	m_nPadNo = padNo;
	m_nIdleAnim = idleAnim;
	pAnita(0u)->RegistData(layoutName);
	pAnita(0u)->SetAnimNo(idleAnim);
	pAnita(0u)->SetState(1u, false);
}

void cTaskCursor::AddButton(unsigned mask, int anim)
{
	m_buttons.push_back({mask, anim});
}

bool cTaskCursor::GetPointer(float& x, float& y) const
{
	const GamePad::State& pad = GamePad::Get(m_nPadNo);
	if (pad.pointerCount > 0)
	{
		x = pad.pointerX;
		y = pad.pointerY;
		return true;
	}
	x = 0.0f;
	y = 0.0f;
	return false;
}

// DOL 0x80095be4 [audited]
bool cTaskCursor::Execute(cJetTask* pParam)
{
	const GamePad::State& pad = GamePad::Get(m_nPadNo);
	const float x = pad.pointerX * (float)gfx::SCREEN_W * 0.5f;
	const float y = pad.pointerY * (float)gfx::SCREEN_H * 0.5f;
	cKingdomAnita* a = pAnita(0u);
	if ((~pad.hold & GamePad::BTN_B) != 0 && pad.pointerCount > 0)
	{
		a->vTrans() = Vector4(x, -y, 0.0f, 0.0f);
		for (const Button& b : m_buttons)
		{
			if (pad.trig & b.mask)
			{
				a->SetAnimNo(b.anim, true, 0.0f);
				m_bPressed = true;
			}
		}
		a->SetState(1u, true);
	}
	else
	{
		a->SetState(1u, false);
	}
	if (m_bPressed && a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END))
	{
		a->SetAnimNo(m_nIdleAnim, true, 0.0f);
		m_bPressed = false;
	}
	AnitaVector->UpdateFrame();
	return true;
}

// DOL 0x80095e0c [audited]: only while enabled (+0x1e1)
void cTaskCursor::Draw(int nStatus, cJetTask* pParam)
{
	if (m_bEnable) AnitaVector->Draw();
}

}
