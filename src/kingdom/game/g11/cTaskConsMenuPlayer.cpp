#include "kingdom/game/g11/cTaskConsMenuPlayer.hpp"

#include "KingdomGlobal.hpp"
#include "gfx/FontWriter.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"

namespace kingdom::game::g11
{
namespace
{
constexpr uint ANIM_END = 0x10000u;   // cJetAnita::JET_ANITA_STATE_ANIM_END
}

// DOL 0x80206344 [audited: the Mii cTaskFaceIcon child (+0x200, RFL) is not ported]: seven anitas on the multiplayer layout, hidden; layer 1 (crown) =
// layer 0 of pattern i + 0xc, the digits (anitas 2..6) = layer n - 2 of pattern i, the name's centre = layer 0 of pattern i + 4 (+ half the 640 x 480
// screen, y flipped); the font writer (priority 0x80000002) is a task of its own that draws above the panel
cTaskConsMenuPlayer::cTaskConsMenuPlayer(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int player, const std::string& name)
	: cTaskConsMenuBase(pConsMan)
{
	SetName("cTaskGrobdaConsMenuPlayer");
	m_Name = name;
	m_nPlayer = player;
	m_nScore = cG11GrobdaSys::Instance()->nScore(player);
	m_bTop = false;
	for (uint i = 0; i < 7; i++)
	{
		pAnita(i)->RegistData("GROBDA_BIN_ANT_MENU_SELECT02");
		pAnita(i)->SetState(1u, false);
	}
	pAnita(1u)->GetLayerPos(pAnita(1u)->vTrans(), 0u, (uint)(player + 0xc));
	for (uint i = 2; i < 7; i++) pAnita(i)->GetLayerPos(pAnita(i)->vTrans(), i - 2, (uint)player);
	pAnita(0u)->GetLayerPos(m_vPos, 0u, (uint)(player + 4));
	m_vPos.x = 0.5f * 640.0f + m_vPos.x;
	m_vPos.y = -(0.5f * 480.0f - m_vPos.y);
	class cTaskName : public cJetTask
	{
	public:
		explicit cTaskName(const cTaskConsMenuPlayer* p) : m_p(p) { nDrawPriority(0x80000002u); SetName("g11::cTaskFontWriter"); }
		void Draw(int, cJetTask*) override { m_p->DrawName(); }

	private:
		const cTaskConsMenuPlayer* m_p;
	};
	KingdomGlobal::KingSys->pAppMain()->ConnectChild(new cTaskName(this));
	m_bShowName = false;
}

// DOL 0x80206954 [audited]: layer 0 = 0x28 + 2 i (appear) / 0x29 + 2 i (the others), layer 1 = 0x3d + kind, layers 2..6 = 3 * (digit n - 2 of the score) + kind
int cTaskConsMenuPlayer::AnimNo(uint layer, int kind) const
{
	if (layer == 0)
	{
		if (kind < 0 || kind > 2) return -1;
		return m_nPlayer * 2 + (kind == 0 ? 0x28 : 0x29);
	}
	if (layer == 1)
	{
		if (kind < 0 || kind > 2) return -1;
		return 0x3d + kind;
	}
	if (layer > 6) return -1;
	int div = 1;
	for (uint i = 2; i < layer; i++) div *= 10;
	const int digit = (m_nScore / div) % 10;   // layer 6: (score % 100000) / 10000
	if (kind < 0 || kind > 2) return -1;
	return digit * 3 + kind;
}

// DOL 0x80206b70 [audited]: every layer on its kind 0 anim (the DOL asks kind 0 here too)
void cTaskConsMenuPlayer::WaitExec()
{
	for (uint i = 0; i < 7; i++) pAnita(i)->SetAnimNo(AnimNo(i, 0), false);
}

// DOL 0x80206be8 [audited]: step 0 shows every layer (the crown only for the best score) on its kind 0 anim; step 1 waits for the anim ends, then the name appears
bool cTaskConsMenuPlayer::AppearExec()
{
	if (nStep() == 0)
	{
		for (uint i = 0; i < 7; i++)
		{
			if (i == 1 && !m_bTop) continue;
			pAnita(i)->SetState(1u);
			pAnita(i)->SetAnimNo(AnimNo(i, 0), false);
		}
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool end = true;
		for (uint i = 0; end && i < 7; i++)
			if (i != 1 || m_bTop) end = pAnita(i)->IsState(ANIM_END);
		if (end)
		{
			m_bShowName = true;   // font writer +0x268; the Mii icon is shown here too (+0x7c, NOT PORTED)
			return true;
		}
	}
	return false;
}

// DOL 0x80206d3c [audited]: the base Execute, then the name's position (every frame: constant here, the name is drawn from m_vPos)
bool cTaskConsMenuPlayer::Execute(cJetTask* pParam)
{
	return cTaskConsMenuBase::Execute(pParam);
}

// UNVERIFIED as g02's cTaskConsMultiMenuPlayer: the name is drawn centred on the pane position, RESIDENT_FONT_MII, size 28
void cTaskConsMenuPlayer::DrawName() const
{
	if (!m_bShowName || m_Name.empty()) return;
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_MII";
	const int height = gfx::FontHeight(st.font);
	st.scaleX = st.scaleY = 28.0f / (float)(height > 0 ? height : 32);
	st.vorigin = gfx::TextStyle::VOrigin::Middle;
	gfx::DrawText((float)(int)m_vPos.x, (float)(int)-m_vPos.y, m_Name, st, gfx::TextAlign::Center);
}
}
