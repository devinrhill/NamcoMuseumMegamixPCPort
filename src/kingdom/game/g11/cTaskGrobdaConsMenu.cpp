#include "kingdom/game/g11/cTaskGrobdaConsMenu.hpp"

#include <cstdio>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g11
{

namespace
{
constexpr uint ANIM_END = 0x10000u;   // cJetAnita::JET_ANITA_STATE_ANIM_END

const char* Layout(bool multi) { return multi ? "GROBDA_BIN_ANT_MENU_SELECT02" : "GROBDA_BIN_ANT_MENU_SELECT01"; }   // crc 0xf7ced99f / 0x6ec78825

// DOL 0x8047d720 [audited] / 24 / 28 -> m00se_01idou, m00se_02Kettei, m00se_03Cancel
const char* const SE_MOVE = "m00se_01idou";
const char* const SE_DECIDE = "m00se_02Kettei";
const char* const SE_CANCEL = "m00se_03Cancel";
}

// ---- buttons ----------------------------------------------------------------------------------------------------------------------------

cTaskGrobdaMenuButton::cTaskGrobdaMenuButton(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti, int hitPat, const int anm[4], uint priority)
	: cTaskConsMenuBase(pConsMan)
{
	m_nHitPat = hitPat;
	for (int i = 0; i < 4; i++) m_nAnm[i] = anm[i];
	pAnita(0u)->RegistData(Layout(bMulti));
	pAnita(0u)->SetState(1u, false);
	nDrawPriority(priority);
}

// DOL 0x802044e0 [audited] (the shared body: pointer -> layout pixels, IsHit on anita 0 with the pattern)
bool cTaskGrobdaMenuButton::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, m_nHitPat);
}

// DOL 0x80204638 [audited] (Back; the others are identical): the wait anim, no reset
void cTaskGrobdaMenuButton::WaitExec()
{
	pAnita(0u)->SetAnimNo(m_nAnm[1], false);
}

// DOL 0x8020468c [audited]: step 0 shows the layout and starts the appear anim, step 1 ends with it
bool cTaskGrobdaMenuButton::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetState(1u);
		pAnita(0u)->SetAnimNo(m_nAnm[0], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(ANIM_END))
		return true;
	return false;
}

// DOL 0x8020474c [audited]
void cTaskGrobdaMenuButton::PointExec()
{
	pAnita(0u)->SetAnimNo(m_nAnm[2], false);
}

// DOL 0x802047a0 [audited]
bool cTaskGrobdaMenuButton::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(m_nAnm[3], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(ANIM_END))
		return true;
	return false;
}

namespace
{
// {appear, wait, point, decide}: Back 0x2b.. , Start 0x2f.. , Multi 0x34.. (+5 in the multiplayer layout), Rule 0x24.. (the same in both)
struct Anm { int a[4]; };
Anm Col(int appear, bool multi, bool shift)
{
	const int s = (multi && shift) ? 5 : 0;
	return Anm{{appear + s, appear + 1 + s, appear + 2 + s, appear + 3 + s}};
}
}

// DOL 0x802043d8 [audited]
cTaskConsMenuBack::cTaskConsMenuBack(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti)
	: cTaskGrobdaMenuButton(pConsMan, bMulti, 0x1a, Col(0x2b, bMulti, true).a, 0x80000003u)
{
}

// DOL 0x80206e24 [audited] (anims 0x24 appear, 0x25 wait, 0x26 point, 0x27 decide for both layouts)
cTaskConsMenuRule::cTaskConsMenuRule(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti)
	: cTaskGrobdaMenuButton(pConsMan, bMulti, 0x19, Col(0x24, bMulti, false).a, 0x80000004u)
{
}

// DOL 0x80207298 [audited]
cTaskConsMenuStart::cTaskConsMenuStart(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti)
	: cTaskGrobdaMenuButton(pConsMan, bMulti, 0x1b, Col(0x2f, bMulti, true).a, 0x80000006u)
{
}

// DOL 0x80205e60 [audited]
cTaskConsMenuMulti::cTaskConsMenuMulti(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti)
	: cTaskGrobdaMenuButton(pConsMan, bMulti, 0x1c, Col(0x34, bMulti, true).a, 0x80000005u)
{
}

// ---- Bg ---------------------------------------------------------------------------------------------------------------------------------

// DOL 0x802048bc [audited]
cTaskConsMenuBg::cTaskConsMenuBg(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti)
	: cTaskConsMenuBase(pConsMan)
{
	pAnita(0u)->RegistData("GROBDA_BIN_ANT_MENU_BG");
	pAnita(0u)->SetAnimNo(bMulti ? 1 : 0, true);
	nDrawPriority(0x10001u);
}

// ---- HighScore --------------------------------------------------------------------------------------------------------------------------

// DOL 0x802049ac [audited]
cTaskConsMenuHighScore::cTaskConsMenuHighScore(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	m_nHigh = cG11GrobdaSys::Instance()->nBest();
	for (uint i = 0; i < 7; i++)
	{
		pAnita(i)->RegistData(Layout(false));
		pAnita(i)->SetState(1u, false);
	}
	for (uint i = 1; i < 7; i++) pAnita(i)->GetLayerPos(pAnita(i)->vTrans(), i - 1, 0);   // FUN_80014408
	nDrawPriority(0x80000000u);
}

// DOL 0x80204aac [audited]: layer 0 = the frame (kind 0 / 1 / 2 -> anim 0x28 / 0x29 / 0x2a), layer n = digit n - 1 of the best score (3 * digit + kind)
int cTaskConsMenuHighScore::AnimNo(uint layer, int kind) const
{
	if (kind < 0 || kind > 2) return -1;
	if (layer == 0) return 0x28 + kind;
	if (layer > 6) return -1;
	int div = 1;
	for (uint i = 1; i < layer; i++) div *= 10;
	const int digit = layer == 6 ? m_nHigh / 100000 : (m_nHigh / div) % 10;
	return digit * 3 + kind;
}

// DOL 0x80204cac [audited]
void cTaskConsMenuHighScore::WaitExec()
{
	for (uint i = 0; i < 7; i++) pAnita(i)->SetAnimNo(AnimNo(i, 1), false);
}

// DOL 0x80204d24 [audited]
bool cTaskConsMenuHighScore::AppearExec()
{
	if (nStep() == 0)
	{
		for (uint i = 0; i < 7; i++)
		{
			pAnita(i)->SetState(1u);
			pAnita(i)->SetAnimNo(AnimNo(i, 0), false);
		}
		nStep()++;
	}
	else if (nStep() == 1)
	{
		for (uint i = 0; i < 7; i++)
			if (!pAnita(i)->IsState(ANIM_END)) return false;
		return true;
	}
	return false;
}

// ---- Tutorial ---------------------------------------------------------------------------------------------------------------------------

namespace
{
using kingdom::game::menu::sTutorialData;
using kingdom::game::menu::sTutorialPage;

// DOL 0x80450360 [audited]: {anim no, text row, layout}; the text GROBDA_BIN_MENU_TEXT_TUTORIAL (crc 0x6e2ca25b)
sTutorialPage s_Page[6] = {
	{0, 1, "GROBDA_BIN_ANT_TUTORIAL"}, {1, 2, "GROBDA_BIN_ANT_TUTORIAL"}, {2, 3, "GROBDA_BIN_ANT_TUTORIAL"},
	{3, 4, "GROBDA_BIN_ANT_TUTORIAL"}, {-1, 5, "GROBDA_BIN_ANT_TUTORIAL"}, {4, 6, "GROBDA_BIN_ANT_TUTORIAL"},
};
// DOL 0x804503a8 [audited]: the multiplayer pages (text rows 8..13)
sTutorialPage s_PageMulti[6] = {
	{0, 8, "GROBDA_BIN_ANT_TUTORIAL"}, {1, 9, "GROBDA_BIN_ANT_TUTORIAL"}, {2, 10, "GROBDA_BIN_ANT_TUTORIAL"},
	{3, 11, "GROBDA_BIN_ANT_TUTORIAL"}, {-1, 12, "GROBDA_BIN_ANT_TUTORIAL"}, {4, 13, "GROBDA_BIN_ANT_TUTORIAL"},
};
sTutorialData* Data(bool multi)   // DOL 0x803812f0 [audited] / 0x803812fc
{
	static sTutorialData s{6u, "GROBDA_BIN_MENU_TEXT_TUTORIAL", {&s_Page[0], &s_Page[1], &s_Page[2], &s_Page[3], &s_Page[4], &s_Page[5]}};
	static sTutorialData m{6u, "GROBDA_BIN_MENU_TEXT_TUTORIAL", {&s_PageMulti[0], &s_PageMulti[1], &s_PageMulti[2], &s_PageMulti[3], &s_PageMulti[4], &s_PageMulti[5]}};
	return multi ? &m : &s;
}
}

// DOL 0x8020777c [audited]: FUN_80145890 + the base ctor variant (+0x54)(cursor, BIN_COMMON_ANT_TUTORIAL = crc 0xd70b5cff, data); natively SetCursor + RegistData
cTaskConsMenuTutorial::cTaskConsMenuTutorial(kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	RegistData("BIN_COMMON_ANT_TUTORIAL", Data(cG11GrobdaSys::Instance()->IsMulti()));
}

// ---- MenuChar ---------------------------------------------------------------------------------------------------------------------------

// DOL 0x802079d0 [audited]
cTaskGrobdaMenuChar::cTaskGrobdaMenuChar(int player)
{
	SetName("cTaskGrobdaMenuChar");
	char name[0x200];
	const int n = player + 1;
	std::snprintf(name, sizeof(name), "m11m_menu_c%02d.brres", n);
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(name, 0x36f);
	std::snprintf(name, sizeof(name), "m11m_menu_p%02d.brres", n);
	m->RegistExpand(0, "cannon", name, 0x36f);   // FUN_8001f798: the pilot rides node "cannon"
	m->RegistAnm(0u, "m11m_menu_c01_a01_cha.brres");
	if (cKingdomModel* p = m->pExpandMdl(0)) p->RegistAnm(0u, "m11m_menu_p01_a01_cha.brres");
	// the place: node play<count>p_chr0n of m11s_menu01_s.brres (translate -> position, rotate (degrees) -> rotation, scale)
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find("m11s_menu01_s.brres", size);
	wii::Brres brres;
	wii::Mdl0 mdl;
	if (d == nullptr || !brres.Load(d, size) || brres.ModelNames().empty() || !mdl.Load(brres, brres.ModelNames().front())) return;
	std::snprintf(name, sizeof(name), "play%dp_chr%02d", cG11GrobdaSys::Instance()->nPlayer(), n);   // DOL string 0x803813f7 "play%dp_chr%02d" (player count, player)
	for (const auto& nd : mdl.nodes)
	{
		if (nd.name != name) continue;
		constexpr float DEG = 0.017453292f;   // FLOAT_8047d7ac
		m->vTrans() = Vector4(nd.translate[0], nd.translate[1], nd.translate[2], 1.0f);
		m->vRotate() = Vector4(nd.rotate[0] * DEG, nd.rotate[1] * DEG, nd.rotate[2] * DEG, 0.0f);
		m->vScale() = Vector4(nd.scale[0], nd.scale[1], nd.scale[2], 1.0f);
		return;
	}
}

// DOL 0x80207e18 [audited]: while not paused / HOME not held the model's frame advances (FUN_8009a478)
bool cTaskGrobdaMenuChar::Execute(cJetTask* pParam)
{
	kingdom::cAppKingdom* app = KingdomGlobal::KingSys->pAppMain();
	if (!(app != nullptr && app->IsPause()) && (GamePad::Get(0).hold & 0x400u) == 0) UpdateFrame();
	return cTaskModel::Execute(pParam);
}

// ---- Man --------------------------------------------------------------------------------------------------------------------------------

// DOL 0x80204e94 [audited: the panels' Mii icons not ported]. Every console is a child of the app (KingSys +0xa40), not of the man.
cTaskConsMenuMan::cTaskConsMenuMan(kingdom::game::cAppGameBaseMenu* pApp)
	: cTaskConsMenuManBase(pApp)
{
	SetName("cTaskGrobdaConsMenuMan");
	cG11GrobdaSys* sys = cG11GrobdaSys::Instance();
	const bool multi = sys->IsMulti();
	cTaskConsMenuManBase* pConsMan = this;
	kingdom::cTaskKingdom* app = KingdomGlobal::KingSys->pAppMain();
	// cTaskSceneCamera (+0x1c8): GROBDA_BIN_MENU_CAMERA (crc 0x44a65818) frame 0 on ScnRoot camera 0 (natively sampled once, as g02's menu)
	{
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find("GROBDA_BIN_MENU_CAMERA", size);
		wii::Brres b;
		wii::ScnAnim a;
		wii::CameraResult cam;
		if (d != nullptr && b.Load(d, size) && a.Load(b))
		{
			if (a.SampleCamera("", 0.0f, cam)) kingdom::NativeScene::SetCamera(cam.pos, cam.aimPos, cam.fovy, cam.nearZ, cam.farZ);
			// the file is bound as the ScnRoot's AnmScn (0x80149f74 -> ResAnmScn::Bind, 0x8014a388 -> ScnRoot SetAnmScn), which also applies its
			// light set, ambient and fog: natively only the camera was taken, so the menu models had no light set (too dark)
			wii::LightResult light;
			if (a.SampleLightSet(0.0f, light))
			{
				a.SampleFog(light);
				kingdom::NativeScene::SetLight(0, light);
			}
		}
	}
	for (int i = 0; i < 4 && i < sys->nPlayer(); i++)
	{
		m_pChar[i] = new cTaskGrobdaMenuChar(i);
		app->ConnectChild(m_pChar[i]);
	}
	m_pBg = new cTaskConsMenuBg(pConsMan, multi);
	app->ConnectChild(m_pBg);
	m_pBack = new cTaskConsMenuBack(pConsMan, multi);
	app->ConnectChild(m_pBack);
	m_pRule = new cTaskConsMenuRule(pConsMan, multi);
	app->ConnectChild(m_pRule);
	m_pStart = new cTaskConsMenuStart(pConsMan, multi);
	app->ConnectChild(m_pStart);
	m_pMulti = new cTaskConsMenuMulti(pConsMan, multi);
	app->ConnectChild(m_pMulti);
	if (!multi)
	{
		m_pHigh = new cTaskConsMenuHighScore(pConsMan);
		app->ConnectChild(m_pHigh);
	}
	else
	{
		// one panel per player (priority 0x80000000; its Mii icon 0x80000001 NOT PORTED, its name 0x80000002); the best last score gets the crown
		int best = 0;
		int top = -1;
		for (int i = 0; i < 4 && i < sys->nPlayer(); i++)
		{
			m_pPlayer[i] = new cTaskConsMenuPlayer(pConsMan, i, sys->szName(i));
			m_pPlayer[i]->nDrawPriority(0x80000000u);
			app->ConnectChild(m_pPlayer[i]);
			if (best < sys->nScore(i))
			{
				top = i;
				best = sys->nScore(i);
			}
		}
		if (top >= 0) m_pPlayer[top]->SetTop(true);
	}
	m_pItem[0] = nullptr;
	m_pItem[1] = m_pBack;
	m_pItem[2] = m_pRule;
	m_pItem[3] = m_pMulti;
	m_pItem[4] = m_pStart;
}

// DOL 0x80205c40 [audited]
bool cTaskConsMenuMan::IsAppearEnd()
{
	if (!m_pStart->IsAppearEndState() || !m_pMulti->IsAppearEndState() || !m_pBack->IsAppearEndState() || !m_pRule->IsAppearEndState()) return false;
	if (m_pHigh != nullptr && !m_pHigh->IsAppearEndState()) return false;
	for (auto* p : m_pPlayer)
		if (p != nullptr && !p->IsAppearEndState()) return false;
	return true;
}

void cTaskConsMenuMan::AllWait()
{
	m_pStart->WaitState();
	m_pMulti->WaitState();
	m_pBack->WaitState();
	m_pRule->WaitState();
	if (m_pHigh) m_pHigh->WaitState();
	for (auto* p : m_pPlayer)
		if (p) p->WaitState();
}

// DOL 0x80205b6c [audited]
void cTaskConsMenuMan::AppearStart()
{
	m_nState = 1;
	m_pStart->AppearState();
	m_pMulti->AppearState();
	m_pBack->AppearState();
	m_pRule->AppearState();
	if (m_pHigh) m_pHigh->AppearState();
	for (auto* p : m_pPlayer)
		if (p) p->AppearState();
}

// DOL 0x802056c8 [audited]
void cTaskConsMenuMan::Reset()
{
	AllWait();
	m_nState = 0;
	m_nStep = 0;
}

// DOL 0x802057ac [audited]
void cTaskConsMenuMan::PointCurrent()
{
	if (m_pItem[m_nSelectOld] != nullptr) m_pItem[m_nSelectOld]->PointState();
}

bool cTaskConsMenuMan::IsBackDecide() { return m_pBack->IsDecideEndState(); }     // DOL 0x80205b54 [audited]
bool cTaskConsMenuMan::IsStartDecide() { return m_pStart->IsDecideEndState(); }   // DOL 0x80205b2c [audited]
bool cTaskConsMenuMan::IsRuleDecide() { return m_pRule->IsDecideEndState(); }     // DOL 0x80205b18 [audited]
bool cTaskConsMenuMan::IsMultiDecide() { return m_pMulti->IsDecideEndState(); }   // DOL 0x80205b40 [audited]

// DOL 0x802057cc [audited]: only while choosing (state 0). A pointer over a button focuses it (the last hit wins), otherwise UP / DOWN move on the tables
// 0x80381108 (focus 0 -> 4 / 4); TWO focuses Back; a second TWO on Back / A B Z decide.
void cTaskConsMenuMan::Select()
{
	if (m_nState != 0) return;
	const GamePad::State& pad = GamePad::Get(0);
	m_nSelectNow = m_nSelectOld;
	bool hit = false;
	for (uint i = 0; i < 5; i++)
	{
		// UNVERIFIED as in g02: the DOL tests the pointer floats even when the remote points away
		if (m_pItem[i] != nullptr && pad.pointerCount > 0 && m_pItem[i]->IsDpdHit(pad.pointerX, pad.pointerY))
		{
			m_nSelectNow = (int)i;
			hit = true;
		}
	}
	if (!hit)
	{
		static const int UP[5] = {4, 3, 1, 4, 2};     // DOL 0x80381108 [audited] (words 1 + 3 f)
		static const int DOWN[5] = {4, 2, 4, 1, 3};   // (words 2 + 3 f)
		if (pad.trig & GamePad::PAD_UP)
			m_nSelectNow = UP[m_nSelectNow];
		else if (pad.trig & GamePad::PAD_DOWN)
			m_nSelectNow = DOWN[m_nSelectNow];
	}
	if (pad.trig & GamePad::PAD_2) m_nSelectNow = 1;
	if (m_nSelectNow != 0 && m_nSelectNow != m_nSelectOld)
	{
		PlaySndSe(SE_MOVE, 0);
		AllWait();
		if (m_pItem[m_nSelectNow] != nullptr) m_pItem[m_nSelectNow]->PointState();
	}
	if (m_nSelectNow != 0)
	{
		if ((pad.trig & GamePad::PAD_2) != 0 && m_nSelectNow == m_nSelectOld)
			m_nState = 2;
		else if ((pad.trig & 0x860u) != 0)   // A / B / Z
		{
			static const int STATE[5] = {-1, 2, 3, 4, 5};   // DOL 0x8038114c [audited] (every second word)
			m_nState = STATE[m_nSelectNow];
		}
	}
	m_nSelectOld = m_nSelectNow;
}

// DOL 0x802053e0 [audited]
bool cTaskConsMenuMan::Execute(cJetTask* pParam)
{
	(void)pParam;
	kingdom::cAppKingdom* app = KingdomGlobal::KingSys->pAppMain();
	if ((app != nullptr && app->IsPause()) || (GamePad::Get(0).hold & 0x400u) != 0 || m_nState == 0) return true;
	if (m_nState == 1)
	{
		if (IsAppearEnd())
		{
			AllWait();
			m_nSelectOld = m_nSelectNow = 4;   // Start is focused first
			m_pItem[4]->PointState();
			m_nState = 0;
		}
	}
	else if (m_nStep == 0)
	{
		const bool two = (GamePad::Get(0).trig & GamePad::PAD_2) != 0;
		switch (m_nState)
		{
		case 2: PlaySndSe(two ? SE_CANCEL : SE_DECIDE, 0); m_pBack->DecideState(); break;
		case 3: PlaySndSe(SE_DECIDE, 0); m_pRule->DecideState(); break;
		case 4: PlaySndSe(SE_DECIDE, 0); m_pMulti->DecideState(); break;
		case 5: PlaySndSe(SE_DECIDE, 0); m_pStart->DecideState(); break;
		}
		m_nStep++;
	}
	return true;
}

}
