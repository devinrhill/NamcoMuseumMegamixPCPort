#include "kingdom/game/g02/cTaskConsole.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cParamGame.hpp"
#include "kingdom/game/g02/cTaskConsMud.hpp"
#include "kingdom/game/g02/cTaskConsParts.hpp"
#include "kingdom/game/g02/cTaskConsRadar.hpp"

namespace kingdom::game::g02
{

cTaskConsole::sHooks cTaskConsole::Hooks{};

// DOL 0x80124468 (the inlined constructor: cTaskKingdom, priority 0x10000) + Init 0x8006093c (single-player branch; the multiplayer branch
// (console +0x1bc = 1) is NOT PORTED, see the header)
cTaskConsole::cTaskConsole(kingdom::cAppKingdom* pApp) : m_pApp(pApp)
{
	SetName("g02::cTaskConsole");
	nDrawPriority(0x80000000u);
	m_bMulti = KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsMulti();   // 0x800c5ca0
	m_nFlagTotal = 0;
	m_nPlayer = SysPlayerNum();                                                               // 0x800c5c68 (sys +0x0c)
	// NOT PORTED: the multiplayer set (m_bMulti). The single-player set is built in every case so the stage has a HUD.
	m_pRadar = new cTaskConsRadar(this);
	ConnectChild(m_pRadar);
	m_pInfo = new cTaskConsInfo(this);
	ConnectChild(m_pInfo);
	m_pFuel[0] = new cTaskConsFuel(this);
	ConnectChild(m_pFuel[0]);
	m_pSignal = new cTaskConsSignal(this);
	ConnectChild(m_pSignal);
	m_pTime = new cTaskConsTime(this);
	ConnectChild(m_pTime);
	m_pStage = new cTaskConsStage(this);
	ConnectChild(m_pStage);
	m_pMessage = new cTaskConsMessage(this);
	ConnectChild(m_pMessage);
	m_pMud[0] = new cTaskConsMud(this, 0);
	ConnectChild(m_pMud[0]);
	m_pResult = new cTaskConsResult(this);
	ConnectChild(m_pResult);
	m_pLose = new cTaskConsLose(this);
	ConnectChild(m_pLose);
	m_pBang = new cTaskConsBang(this);
	ConnectChild(m_pBang);
	m_pWarning = new cTaskConsWarning(this);
	ConnectChild(m_pWarning);
}

// DOL 0x801275e4: 0x80065d04 deletes the cKingdomRand (+0x1b8); the parts are child tasks
cTaskConsole::~cTaskConsole()
{
}

bool cTaskConsole::IsPaused() const
{
	return m_pApp != nullptr && m_pApp->IsPause();
}

// the test every part repeats: app +0x1b4 (pause) || app +0x284 (stage end) || get_pad1(0) & 0x400 (HOME)
bool cTaskConsole::IsFrozen() const
{
	if (IsPaused()) return true;
	if (m_App.pStageEnd != nullptr && *m_App.pStageEnd) return true;
	return (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;
}

// DOL 0x80065db0 (single player). Called by the app ctor 0x801236bc for every car: the players first (colour 6, 5 on the boss stage,
// bPlayer = 1), then the enemies (colour 5, 6 on the boss stage, bPlayer = 0).
//   the radar gets every car (anita count + 0x17 starts with anim = the colour, reset; Execute sets the real icon);
//   a player car (kind & 0x10): the fuel gauge (and its layer positions for the player count), the info and the mud follow it;
//   an enemy (kind & 0x20) on the boss stage: the info follows it (the last one registered wins).
// NOT PORTED: the multiplayer branch (console +0x1bc: fuel / mud per player index = car +0x1cc, cTaskConsInfoMulti + 0x10 * index + 0x1d8).
void cTaskConsole::Regist(kingdom::cTaskModel* pCar, int nColour, bool bPlayer)
{
	if (m_pRadar != nullptr) m_pRadar->AddCar(pCar, nColour, bPlayer);
	sConsCar c;
	CarState(pCar, c);
	if ((c.nKind & 0x10) != 0 && !m_bMulti)
	{
		if (m_pFuel[0] != nullptr) m_pFuel[0]->SetCar(pCar, 0);
		if (m_pInfo != nullptr) m_pInfo->m_pCar = pCar;
		if (m_pMud[0] != nullptr) m_pMud[0]->SetCar(pCar);
	}
	if ((c.nKind & 0x20) != 0 && KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsBossStage())
	{
		if (m_pInfo != nullptr) m_pInfo->m_pCar = pCar;
	}
}

// DOL 0x800662c8 (callers: the flag placement of cTaskMap 0x80077e48 / 0x800780f4): the radar shows the flag; +0x1c0 counts the flags (max 20)
void cTaskConsole::RegistFlag(kingdom::cTaskModel* pGimmick, int nType)
{
	if (m_pRadar != nullptr) m_pRadar->AddFlag(pGimmick, nType);
	if (m_nFlagTotal < 0x14) m_nFlagTotal++;
}

// DOL 0x80066554: no signal task -> true; else signal mode > 1 (the compiler's (m ^ 1) >> 1 - ((m ^ 1) & m) sign trick)
bool cTaskConsole::IsSignalGo() const
{
	if (m_pSignal == nullptr) return true;
	return m_pSignal->m_nMode > 1;
}

// DOL 0x80066584
bool cTaskConsole::IsResultEnd() const
{
	return m_pResult != nullptr && m_pResult->m_nMode == 3;
}

// DOL 0x800665fc
bool cTaskConsole::IsLoseShown() const
{
	return m_pLose != nullptr && m_pLose->m_nMode == 2;
}

// DOL 0x80066614
bool cTaskConsole::IsLoseEnd() const
{
	return m_pLose != nullptr && m_pLose->m_nMode == 7;
}

// DOL 0x8006662c (app Execute 0x80125b1c, the crash state)
bool cTaskConsole::ShowBang()
{
	if (m_pBang != nullptr) m_pBang->Show();
	return m_pBang != nullptr;
}

// DOL 0x8006669c
bool cTaskConsole::HideBang()
{
	if (m_pBang != nullptr) m_pBang->Hide();
	return m_pBang != nullptr;
}

// DOL 0x800667c4
void cTaskConsole::HideBang2()
{
	if (m_pBang != nullptr) m_pBang->Hide();
}

float cTaskConsole::fClearFlagWait() const
{
	return m_App.pParam != nullptr ? m_App.pParam->m_fClearFlagWait : 0.0f;
}

float cTaskConsole::fLoseFlagWait() const
{
	return m_App.pParam != nullptr ? m_App.pParam->m_fLoseFlagWait : 0.0f;
}

// a local cRallyXSndHnd + 0x80129d78 (play the cue; in app state 6 (game over) also SetVolume 0.2 = FLOAT_8047c040, NOT PORTED: the
// mixer has no per-voice volume), then DetachSound
void cTaskConsole::PlaySE(const char* cue)
{
	WiiSnd::Handle h;
	WiiSnd::Play(h, cue);
	WiiSnd::Detach(h);
}

bool cTaskConsole::CarState(kingdom::cTaskModel* car, sConsCar& out)
{
	out = sConsCar{};
	if (car == nullptr || Hooks.CarState == nullptr) return false;
	return Hooks.CarState(car, out);
}

bool cTaskConsole::GimmickRespawn(kingdom::cTaskModel* g)
{
	return g != nullptr && Hooks.GimmickRespawn != nullptr && Hooks.GimmickRespawn(g);
}

int cTaskConsole::SysPlayerNum()
{
	return Hooks.SysPlayerNum != nullptr ? Hooks.SysPlayerNum() : 1;
}

int cTaskConsole::SysViewMode(int player)
{
	return Hooks.SysViewMode != nullptr ? Hooks.SysViewMode(player) : 2;
}

int cTaskConsole::StageHighScore(int stage)
{
	return Hooks.StageHighScore != nullptr ? Hooks.StageHighScore(stage) : 0;
}

}
