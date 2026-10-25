#include "kingdom/cAppSaveLoad.hpp"
#include "gfx/FontWriter.hpp"

#include <cstdlib>
#include <cstdio>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom
{

namespace
{
constexpr uint STATE_COUNT = 21;       // jump table 0x803892a0
constexpr uint ANIM_END = 0x10000;     // cJetAnita state bit: animation finished
constexpr uint ANITA_SHOW = 1;         // cJetAnita state bit: drawn

constexpr uint FADE_IN = 2;            // Fade(2, 500): from black
constexpr uint FADE_OUT = 0x10001;     // Fade(0x10001, 500)
constexpr int FADE_MSEC = 500;

constexpr uint KEY_B = 0x0400;         // held B freezes the whole app (0x8000789c)
// FUN_80009598: any of these in pad word 1 (trig) decides; 0x8 / 0x4 are the upright "up" / "down" bits, i.e. LEFT / RIGHT on the
// sideways Wiimote the game is played with (first option = left, second = right)
constexpr uint TRIG_DECIDE = 0x860;
constexpr uint TRIG_UP = 0x8;   // cursor to the first option (left)
constexpr uint TRIG_DOWN = 0x4; // cursor to the second option (right)

// The dialog icons are animations of RESIDENT_ANITA_FRAME picked by the game kind in cKingdomSys + 0xa44
// (1, 2, 4, 5, 6, 7; anything else uses the default). `alt` is the highlighted variant (+1).
// Two families: "delete" dialog (state 8) and "continue" dialogs (states 11, 14).
struct IconSet
{
	int dflt;
	int byKind[8]; // index = kind; unused kinds (0, 3) = -1
};
int IconAnim(const IconSet& set, uint kind, bool alt)
{
	int anim = (kind < 8 && set.byKind[kind] >= 0) ? set.byKind[kind] : set.dflt;
	return anim + (alt ? 1 : 0);
}

// 0x8000801c / 0x800080bc (state 8)
const IconSet kDeleteL1 = {36, {-1, 31, 41, -1, 46, 51, 56, 129}};
const IconSet kDeleteL2 = {96, {-1, 91, 101, -1, 106, 111, 116, 139}};
// 0x8000868c / 0x8000872c (states 11, 14)
const IconSet kContinueL1 = {6, {-1, 1, 11, -1, 16, 21, 26, 124}};
const IconSet kContinueL2 = {66, {-1, 61, 71, -1, 76, 81, 86, 134}};

// FUN_80009598's sounds (strings at 0x80368258, 0x80368268, 0x80368248)
constexpr const char* SND_DECIDE = "m00se_02Kettei";
constexpr const char* SND_NG = "m00se_08NG";
constexpr const char* SND_MOVE = "m00se_01idou";
}

cAppSaveLoad::cAppSaveLoad(bool force)
	: m_bForce(force)
{
	SetName("cAppSaveLoad");
	Init();
}

// DOL 0x80006fa0
void cAppSaveLoad::Init()
{
	// layers 0-3 all hold the same frame layout, loaded hidden
	for (int i = 0; i < 4; i++)
	{
		const std::vector<byte>& data = KingdomGlobal::KingSys->SearchData("RESIDENT_ANITA_FRAME");
		if (data.empty())
		{
			std::fprintf(stderr, "cAppSaveLoad: RESIDENT_ANITA_FRAME not found (resident.arc not attached?)\n");
			break;
		}
		m_Anita.pAnita(i)->SetData(data);
		m_Anita.pAnita(i)->SetState(ANITA_SHOW, false);
	}
	m_pFade = new cTaskFade();
	m_pFade->SetName("cTaskFade");
	ConnectChild(m_pFade);

	// 0x80007450: FUN_8009dc08(KingSys[0xa50], 1, !KingSys[0xa95])
	KingdomGlobal::KingSys->SetObjA50Flags(true, !KingdomGlobal::KingSys->bUnkA95());
}

// DOL 0x800097ec (FUN_80014408 / FUN_80016f28 / FUN_80016fc8 / FUN_801394b4): the message text (RESIDENT_MSG_NAND_ERROR row index + 1, FUN_801394b4) is centred on the position of layer 3's first layer
// record (FUN_80014408 (pat 0, layer 0): px, py), converted to screen pixels as (320 + px, 240 - py). The game hands it to the font writer at
// +0x294 (queue FUN_80016f28 / FUN_80016fc8, flag word 3). UNVERIFIED: flag 3 is read as "centred" and the font as RESIDENT_FONT_FRAME with scale 1.
void cAppSaveLoad::ShowMessage(int index)
{
	if (!m_msgsLoaded) m_msgsLoaded = m_msgs.Load("RESIDENT_MSG_NAND_ERROR");
	m_msgText = m_msgs.Get(index);
	Vector4 pos;
	m_msgPos = m_Anita.pAnita(3)->GetLayerPos(pos, 0, 0);
	m_msgX = 320.0f + pos.x;
	m_msgY = 240.0f - pos.y;
	static int s_last = -1;
	if (index != s_last)
	{
		s_last = index;
		std::printf("cAppSaveLoad: message %d\n", index);
	}
}

// DOL 0x80009414: where each save status leads. Returns false when the caller should simply try the operation again
// (status 2 for the first three attempts, and statuses the router does not know).
bool cAppSaveLoad::RouteError(uint status)
{
	auto go = [this](uint next) {
		nModeState(0) = next;
		for (uint i = 0; i < 8; i++) nModeProc(0, (int)i) = 0;
	};
	switch (status)
	{
	case 0: return true;                       // nothing to route
	case 1: go(8); return true;                // broken data: offer to delete it
	case 2:                                    // transient: retry up to three times, then give up (state 18)
		m_nRetry++;
		if (m_nRetry - 1 < 3)
		{
			StopWatchReset(0);
			StopWatchStart(0);
			return false;
		}
		go(18);
		return true;
	case 3: go(9); return true;                // not enough blocks
	case 4: go(10); return true;               // not enough files
	case 5: go(15); return true;
	case 6: go(16); return true;
	case 7: go(17); return true;
	case 8: go(18); return true;
	default: return false;                     // 9 and above: not handled
	}
}

// DOL 0x80009598: cursor and decision for the two-option dialogs
void cAppSaveLoad::MenuInput()
{
	const GamePad::State& pad = GamePad::Get(0);
	const uint before = m_nCursor;
	WiiSnd::Handle se;
	if (pad.trig & TRIG_DECIDE)
	{
		if (m_nCursor == 0)
		{
			WiiSnd::Play(se, SND_NG); // nothing highlighted yet
		}
		else
		{
			m_nChoice = (m_nCursor == 1) ? 1 : 2;
			WiiSnd::Play(se, SND_DECIDE);
		}
	}
	else if (pad.trig & (TRIG_UP | TRIG_DOWN))
	{
		m_nCursor = (pad.trig & TRIG_DOWN) ? 2 : 1;
	}
	if (m_nCursor != before)
	{
		WiiSnd::Play(se, SND_MOVE);
	}
}

// DOL 0x8000786c. State S (+0x34), previous O (+0x3c), step P (+0x40) as everywhere; `go` = state change + clear modeproc.
bool cAppSaveLoad::Execute(cJetTask* pParam)
{
	cKingdomSys* sys = KingdomGlobal::KingSys;
	cKingdomSave* save = sys->pSave();
	const GamePad::State& pad = GamePad::Get(0);
	if (pad.hold & KEY_B)
	{
		return true; // 0x8000789c: held B freezes the app (no animation update either)
	}

	uint& S = nModeState(0);
	uint& O = nOldState(0);
	uint& P = nModeStep(0);

	static uint s_lastLogged = ~0u;
	if (S != s_lastLogged)
	{
		s_lastLogged = S;
		std::printf("cAppSaveLoad: state %u (force=%d)\n", S, (int)m_bForce);
	}

	auto layer = [this](int i) { return m_Anita.pAnita(i); };
	auto ended = [&](int i) { return layer(i)->IsState(ANIM_END); };
	auto play = [&](int i, int anim, bool reset = true) { layer(i)->SetAnimNo(anim, reset, 0.0f); };
	auto show = [&](int i, bool on) { layer(i)->SetState(ANITA_SHOW, on); };
	auto go = [&](uint next) {
		S = next;
		for (uint i = 0; i < 8; i++) nModeProc(0, (int)i) = 0;
	};
	auto restartTimer = [&]() { StopWatchReset(0); StopWatchStart(0); };
	auto elapsed = [&]() { return StopWatchCheck(0); };

	// success -> next, failure -> let the router pick the error state, and re-run `op` if it asks for a retry
	auto settle = [&](uint next, auto op) {
		if (m_bOk)
		{
			go(next);
		}
		else if (!RouteError(save->Status()))
		{
			m_bOk = op();
		}
	};

	// the two-option dialog icons (states 8, 11, 14)
	auto dialogIcons = [&](const IconSet& l1, const IconSet& l2) {
		const uint kind = sys->nUnkA44();
		play(1, IconAnim(l1, kind, false)); show(1, true);
		play(2, IconAnim(l2, kind, false)); show(2, true);
	};
	auto cursorIcons = [&](const IconSet& l1, const IconSet& l2) {
		const uint kind = sys->nUnkA44();
		play(1, IconAnim(l1, kind, m_nCursor == 1), false);
		play(2, IconAnim(l2, kind, m_nCursor == 2), false);
	};
	auto hideDialog = [&]() { show(1, false); show(2, false); };
	auto resetMenu = [&]() { m_nChoice = 0; m_nCursor = 0; m_nRetry = 0; };

	if (S < STATE_COUNT)
	{
		O = S;
		switch (P)
		{
		case 0: // enter
			P = 1;
			switch (S)
			{
			case 0:
				if (m_bForce) save->SetDisabled(false);
				break;
			case 1:
				m_pFade->Fade(FADE_IN, FADE_MSEC);
				play(0, 120); show(0, true);
				break;
			case 2: restartTimer(); m_nRetry = 0; m_bOk = save->Check(); break;
			case 3: restartTimer(); m_bOk = save->Load(); break;
			case 4: restartTimer(); break;
			case 5: restartTimer(); m_bOk = save->Save(); break;
			case 6: restartTimer(); break;
			case 7: restartTimer(); m_bOk = save->CheckSpace(); break;
			case 8: dialogIcons(kDeleteL1, kDeleteL2); resetMenu(); break;
			case 9:
			case 10:
				if (!sys->bUnkA95()) sys->SetObjA50Flags(false, false);
				restartTimer();
				break;
			case 11: dialogIcons(kContinueL1, kContinueL2); resetMenu(); break;
			case 12: restartTimer(); break;
			case 13: restartTimer(); break;
			case 14:
				if (!sys->bUnkA95()) sys->SetObjA50Flags(false, false);
				dialogIcons(kContinueL1, kContinueL2);
				resetMenu();
				break;
			case 15: case 16: case 17: case 18:
				sys->SetObjA50Flags(true, false);
				sys->pUnkA90() = save;
				break;
			case 19:
				m_pFade->Fade(FADE_OUT, FADE_MSEC);
				play(0, 122); show(0, true);
				break;
			default: break;
			}
			[[fallthrough]];
		case 1: // run
			switch (S)
			{
			case 0:
				if (save->IsDisabled())
				{
					go(20);
				}
				else if (!m_bForce && !save->IsDirty())
				{
					go(20); // nothing to save
				}
				else
				{
					go(1);
				}
				break;
			case 1:
				if (m_pFade->IsEnd()) go(2);
				break;
			case 2:
				ShowMessage(0);
				if (elapsed() > 1000)
				{
					if (m_bOk)
					{
						go(m_bForce ? 3 : 4);
					}
					else if (save->Status() == 0)
					{
						go(11); // no save data at all: offer to create one
					}
					else if (!RouteError(save->Status()))
					{
						m_bOk = save->Check();
					}
				}
				break;
			case 3:
				ShowMessage(1);
				if (elapsed() > 1000) settle(19, [&] { return save->Load(); });
				break;
			case 4:
				ShowMessage(2);
				if (elapsed() > 250 || save->IsDisabled()) go(5);
				break;
			case 5:
				ShowMessage(2);
				if (elapsed() > 750 || save->IsDisabled()) settle(19, [&] { return save->Save(); });
				break;
			case 6:
				ShowMessage(3);
				if (elapsed() > 250 || save->IsDisabled()) go(7);
				break;
			case 7:
				ShowMessage(3);
				if (elapsed() > 1000) settle(12, [&] { return save->CheckSpace(); });
				break;
			case 8: // "the data is broken, delete it?": 1 = no (-> 14), 2 = delete
				ShowMessage(7);
				if (m_nChoice == 0)
				{
					MenuInput();
					cursorIcons(kDeleteL1, kDeleteL2);
				}
				else if (m_nChoice == 1)
				{
					go(14);
				}
				else if (save->Delete())
				{
					go(13);
				}
				else
				{
					RouteError(save->Status());
				}
				break;
			case 9:  // not enough blocks
			case 10: // not enough files
				ShowMessage(S == 9 ? 8 : 9);
				if (elapsed() > 1000 && (pad.trig & ~KEY_B)) go(14);
				break;
			case 11: // "no save data: create it?": 1 = create (-> 6), 2 = continue without (-> 14)
				ShowMessage(4);
				if (m_nChoice == 0)
				{
					MenuInput();
					cursorIcons(kContinueL1, kContinueL2);
				}
				else if (m_nChoice == 1)
				{
					go(6);
				}
				else
				{
					go(14);
				}
				break;
			case 12:
				ShowMessage(14);
				if (elapsed() > 1000) go(19);
				break;
			case 13:
				ShowMessage(5);
				if (elapsed() > 1000) go(11);
				break;
			case 14: // "continue without saving?": 1 = yes (disable saving, -> 19), 2 = go back
				ShowMessage(6);
				if (m_nChoice == 0)
				{
					MenuInput();
					cursorIcons(kContinueL1, kContinueL2);
				}
				else if (m_nChoice == 1)
				{
					save->SetDisabled(true);
					go(19);
				}
				else
				{
					go(m_nErrState);
				}
				break;
			case 15: ShowMessage(10); break; // fatal NAND errors: the message stays up
			case 16: ShowMessage(11); break;
			case 17: ShowMessage(12); break;
			case 18: ShowMessage(13); break;
			case 19:
				if (m_pFade->IsEnd()) go(20);
				break;
			case 20:
				// 0x80009354: every frame until the job actually changes
				Kingdom_SetJobNo(sys->nNextJob(), false, false);
				break;
			}
			if (O != S) P = 2;
			if (P != 2) break;
			[[fallthrough]];
		case 2: // leave
			P = 0;
			switch (O)
			{
			case 8: case 11: case 14:
				hideDialog();
				if (O == 14 && !sys->bUnkA95()) sys->SetObjA50Flags(true, true);
				break;
			case 9:
			case 10:
				m_nErrState = 11;
				sys->SetObjA50Flags(true, !sys->bUnkA95());
				break;
			default: break;
			}
			break;
		}
	}

	m_Anita.UpdateFrame(); // 0x80009378..: FUN_80014d58
	return true;
}

// DOL 0x8000970c
void cAppSaveLoad::Draw(int nStatus, cJetTask* pParam)
{
	m_Anita.Draw(); // DOL 0x8000970c: 2D projection then the four frame layers
	if (m_msgPos && !m_msgText.empty())
	{
		gfx::TextStyle st;
		st.scaleX = st.scaleY = 0.5f;   // UNVERIFIED size: the dialog pane is far smaller than the font's 40 px
		st.vorigin = gfx::TextStyle::VOrigin::Middle;   // writer mode 3 = flags 0x111 (nw4r VERTICAL_ORIGIN_MIDDLE)
		gfx::DrawText(m_msgX, m_msgY, m_msgText, st, gfx::TextAlign::Center);
	}
}

}
