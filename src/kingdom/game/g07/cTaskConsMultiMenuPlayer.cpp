#include "kingdom/game/g07/cTaskConsMultiMenuPlayer.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "gfx/FontWriter.hpp"
#include "gfx/Gfx.hpp"

namespace kingdom::game::g07
{

const int cTaskConsMultiMenuPlayer::FONT_BUF_SIZE = 512;
const float cTaskConsMultiMenuPlayer::FONT_SIZE = 18.0f;
const int cTaskConsMultiMenuPlayer::SET_FONT_POS_PAT = 2;
const uint cTaskConsMultiMenuPlayer::SET_FONT_POS_LAY[4] = {3u, 2u, 1u, 0u};
const int cTaskConsMultiMenuPlayer::SET_MII_WND_PAT[4] = {24, 25, 26, 27};
const uint cTaskConsMultiMenuPlayer::SET_MII_WND_LAY[3] = {0u, 1u, 2u};

uint& cTaskConsMultiMenuPlayer::nPlayerNo()
{
	return m_nPlayerNo;
}

sMatchingData::InfoData*& cTaskConsMultiMenuPlayer::pInfo()
{
	return m_pInfo;
}

cTaskConsMultiMenuPlayer::~cTaskConsMultiMenuPlayer()
{
}

cTaskConsMultiMenuPlayer::cTaskConsMultiMenuPlayer(cTaskConsMenuManBase* const& pConsMan, const uint& nPlayerNo, sMatchingData::InfoData* pInfo)
	: cTaskConsMenuBase(pConsMan)
{
	this->pInfo() = pInfo;
	this->nPlayerNo() = nPlayerNo;
}

// DOL 0x801cdb44 [audited: described by the 43rd-session pass] (Mii face icon child not ported: RFL is out of scope; the name font writer child is drawn in Draw)
void cTaskConsMultiMenuPlayer::SetData()
{
	nDrawPriority(2147483657u + nPlayerNo());
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetWndAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	Vector4 vFont = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	pAnita(0u)->GetLayerPos(vFont, SET_FONT_POS_LAY[nPlayerNo()], SET_FONT_POS_PAT);
	m_fFontX = 0.5f * (float)gfx::SCREEN_W + vFont[0]; // DOL: 0.5 * render mode fbWidth (+4) + x
	m_fFontY = vFont[1] - 0.5f * (float)gfx::SCREEN_H; // 0.5 * efbHeight (+6)
	m_bFontVisible = false;
}

// DOL 0x801ce1a0 [audited: anim tables and priorities compared word for word with the DOL data, step logic vs decomp]
int cTaskConsMultiMenuPlayer::nGetWndAnmNo(int nAnmState)
{
	int obj[4][3] = {{99, 100, 101}, {102, 103, 104}, {105, 106, 107}, {108, 109, 110}};
	CustomKingdomUtil::Assert(0 <= nPlayerNo() && nPlayerNo() < 4, "Player Over");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "State Over");
	return obj[nPlayerNo()][nAnmState];
}

// DOL 0x801ce248 [audited: anim tables and priorities compared word for word with the DOL data, step logic vs decomp]
bool cTaskConsMultiMenuPlayer::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		m_bFontVisible = true; // DOL 0x801ce248: font writer +0x268 = 1
		return true;
	}
	return false;
}

// DOL 0x801ce330 [audited: anim tables and priorities compared word for word with the DOL data, step logic vs decomp]
void cTaskConsMultiMenuPlayer::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetWndAnmNo(1), false);
}

// DOL 0x801ce394 [audited: described by the 43rd-session pass] (the name is printed by the font writer child that Execute 0x801ce470 queues at ((int)x, (int)-y); UNVERIFIED: font RESIDENT_FONT_MII, baseline)
void cTaskConsMultiMenuPlayer::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
	if (!m_bFontVisible || pInfo() == nullptr || pInfo()->Name.empty())
	{
		return;
	}
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_MII";
	const int height = gfx::FontHeight(st.font);
	st.scaleX = st.scaleY = FONT_SIZE / (float)(height > 0 ? height : 32);
	st.vorigin = gfx::TextStyle::VOrigin::Baseline;
	gfx::DrawText((float)(int)m_fFontX, (float)(int)-m_fFontY, pInfo()->Name, st);
}

// DOL 0x801ce470 [audited: anim tables and priorities compared word for word with the DOL data, step logic vs decomp]
bool cTaskConsMultiMenuPlayer::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	return cTaskConsMenuBase::Execute(pParam);
}

}
