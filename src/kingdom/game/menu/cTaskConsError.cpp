#include "kingdom/game/menu/cTaskConsError.hpp"

#include "cJetTask.hpp"
#include "gfx/FontWriter.hpp"
#include "gfx/Gfx.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/menu/cParamMessage.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"

namespace kingdom::game::menu
{

cParamMessage*& cTaskConsError::pParamMessage()
{
	return m_pParamMessage;
}

uint& cTaskConsError::nErrorCode()
{
	return m_nErrorCode;
}

Vector4& cTaskConsError::vTrans()
{
	return m_vTrans;
}

cTaskConsError::~cTaskConsError()
{
}

void cTaskConsError::nSetErrorCode(uint nIdx)
{
	nErrorCode() = nIdx;
}

uint cTaskConsError::nGetErrorCode()
{
	return nErrorCode();
}

// DOL 0x801d4060 [audited: text drawn natively in Draw (font writer child not ported); origin = layer pos + (320, -240)]: also a cTaskFontWriter child (0x26c bytes, priority 0x80000021, Init(0x400, 0x200) 0x80016d2c, font KingSys +0xa64 =
// RESIDENT_FONT_FRAME (CRC 0x52901112, 0x8000aa90), colour -1 0x80016dd0, size 25.0 0x80016dd8, hidden), and the text origin +0x1e0 = anita 0
// layer 0 pattern 0 (0x80014408) + (0.5 * fb width, -0.5 * efb height) (render mode 0x80138e58 +4 / +6: 640 x 480)
cTaskConsError::cTaskConsError(int nId, string Key, cParamMessage* pMessage)
	: cTaskConsWndBase(nId)
{
	nErrorCode() = 0u;
	pParamMessage() = pMessage;
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
	m_bTextVisible = false;
	pAnita(0u)->GetLayerPos(vTrans(), (uint)SET_LAY, (int)SET_PAT);
	vTrans().x += 0.5f * (float)gfx::SCREEN_W;
	vTrans().y += -0.5f * (float)gfx::SCREEN_H;
}

bool cTaskConsError::AppearProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(0u));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		m_bTextVisible = true;   // DOL 0x801d4370 [audited]: writer +0x268 = 1
		return true;
	}
	return false;
}

bool cTaskConsError::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(2u));
		m_bTextVisible = false;   // DOL 0x801d4440 [audited]: writer +0x268 = 0
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetState(1u, false);
		return true;
	}
	return false;
}

int cTaskConsError::nGetWndAnmNo(uint nState)
{
	int obj[3] = {0, 1, 2};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	return obj[nState];
}

// DOL 0x801d4544 [audited: text queued natively in Draw]: while the writer is visible, queue the message of the error code (pParamMessage +0x20 + code * 0x1000) with 0x80016e88
// (mode 2 = flags 0x311: baseline, centred) at ((int)x, (int)-y); the writer task draws the queue. Natively the text is drawn in Draw.
bool cTaskConsError::Execute(cJetTask* pParam)
{
	return cTaskConsMatching::Execute(pParam);
}

void cTaskConsError::Draw(int nStatus, cJetTask* pParam)
{
	cTaskConsWndBase::Draw(nStatus, pParam);
	if (!m_bTextVisible || pParamMessage() == nullptr || nErrorCode() >= 15u) return;
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_FRAME";
	const int height = gfx::FontHeight(st.font);
	st.scaleX = st.scaleY = (float)FONT_SIZE / (float)(height > 0 ? height : 32);   // nw4r SetFontSize(25)
	st.vorigin = gfx::TextStyle::VOrigin::Baseline;
	gfx::DrawText((float)(int)vTrans().x, (float)(int)-vTrans().y, pParamMessage()->pStr(nErrorCode()), st, gfx::TextAlign::Center);
}

}
