#include "kingdom/game/g02/cTaskPlayer.hpp"

#include "GamePad.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g02/cCamera.hpp"
#include "kingdom/game/g02/cParamCar.hpp"
#include "kingdom/game/g02/cTaskMap.hpp"
#include "snd/Mixer.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float PI = 3.1415927f;             // FLOAT_8047a9ec
constexpr float HALF_PI = 1.5707964f;        // FLOAT_8047a9e8 (FLOAT_8047a9f0 = -pi / 2)
constexpr float STICK = 0.1f;                // FLOAT_8047a9d4 (FLOAT_8047a9d8 = -0.1)

bool IsPlaying(const WiiSnd::Handle& h) { return h.voice >= 0 && snd::IsPlaying(h.voice); }   // FUN_80026424

// the gimmick's model (cTaskModel::pModel(g, 0) 0x8009a430): the flag effects read its position
Vector4 GimmickPos(cTaskGimmick* g)
{
	kingdom::cKingdomModel* m = g != nullptr ? g->pModel(0) : nullptr;
	return m != nullptr ? m->vTrans() : Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}
}

void (*cTaskPlayer::CameraRotYGoal)(cCamera* pCam, float fRad) = nullptr;

// ---------------------------------------------------------------------------------------------------------------------------------------------
// construction

// DOL 0x80124e54: the cTaskUnit part (cTaskModel ctor, the unit vtable, its Init, +0x1cc = index, the models FUN_8008120c), +0x9dc the count
// sound handle, the cTaskPlayer vtable, then cTaskPlayer::Init (vtable +0x3c)
cTaskPlayer::cTaskPlayer(cRallyUnitHost* pHost, int nColour, int nIndex) : cTaskUnit(pHost, nColour, nIndex)
{
	SetName("g02::cTaskPlayer");
	InitPlayer();
}

// (the DOL deleting dtor 0x8007bc68 runs after Term; natively non-app tasks never run Term, so the dtor frees what Term would)
cTaskPlayer::~cTaskPlayer()
{
	delete m_pPlayerParam;
	m_pPlayerParam = nullptr;
}

// DOL 0x8007a2e8
void cTaskPlayer::InitPlayer()
{
	m_fSpeedGoal = 0.0f;                         // +0x298 = FLOAT_8047a9c8
	m_pPlayerParam = new cParamCarPlayer();      // 0x38 bytes, CSV_CARPLAYER
	m_pPlayerParam->Load();
	m_fPadWait = 0.0f;                           // +0x9cc
	m_fPadKind = 1.0f;                           // +0x9d0 = FLOAT_8047a9d0
	m_fFuelSave = m_fFuel;                       // +0x9d4 = +0x2ac
	m_nCount = 0;                                // +0x9d8
}

// DOL 0x8007a468 (the CSV_CARPLAYER block only; no chain to cTaskUnit::Term, as in the DOL)
void cTaskPlayer::Term()
{
	if (m_pPlayerParam != nullptr)
	{
		delete m_pPlayerParam;
		m_pPlayerParam = nullptr;
	}
}

// DOL 0x8007ae2c
u32 cTaskPlayer::nUnitId() const { return (u32)m_nIndex | 0x10u; }

// DOL 0x8007b720
int cTaskPlayer::nView() const { return SysView(m_nIndex); }

// DOL 0x8007b754: get_pad1(sys 0x800c5b98(index))->trig & 0x20 (A)
bool cTaskPlayer::IsSmokeReq() { return (GamePad::Get(SysPad(m_nIndex)).trig & GamePad::PAD_A) != 0; }

// ---------------------------------------------------------------------------------------------------------------------------------------------
// Execute

// DOL 0x8007a558 (r4 = the app; natively the host)
bool cTaskPlayer::Execute(cJetTask* pParam)
{
	cRallyUnitHost* host = m_pHost;
	if (host == nullptr || !host->IsUnitDead())   // FUN_80126928 (always 0)
	{
		if (host == nullptr || !host->IsUnitStageEnd())   // app +0x284
		{
			if (IsFrozen()) return true;             // app +0x1b4 || +0x284 || HOME (no cTaskUnit::Execute either)
			FuelBurstStep();                         // 0x80088450
			ChameleonStep();                         // 0x8008848c
			MudStep();                               // 0x80088658
			Step();                                  // 0x80084d6c
			if (IsState(7)) WinClip();               // 0x8007dd14
			if (IsState(10)) LoseClip();             // 0x8007de2c
			if (IsState(8) || IsState(9))
			{
				if (SysWinner(m_nIndex)) WinClip();  // sys 0x800c58b0(index)
				else LoseClip();
			}
		}
		else if (host->nUnitStageEndId() == (int)nUnitId())   // app +0x288 == vtable +0x44: the car whose lucky flag ended the stage
		{
			if ((GamePad::Get(0).hold & GamePad::PAD_HOME) != 0) return true;
			if (host->IsUnitPause())
			{
				// FUN_80026394(0.0, +0x9dc, 500) while the count sound plays: the volume fade is NOT PORTED (no volume control in WiiSnd)
				return true;
			}
			// FUN_80026394(1.0, +0x9dc, 500) while it plays: NOT PORTED (volume)
			const int step = m_nCount;
			if (step == 0)
			{
				WiiSnd::Play(m_SndCount, "m02se_21Count");   // FUN_80129d78(+0x9dc, cue, 0, 0): no actor
				m_fFuelSave = m_fFuel;
				m_nCount++;
			}
			else if (step == 1)
			{
				m_fFuel = m_fFuel - 1.0f;            // FLOAT_8047a9d0
				AddScore(0x32);                      // 0x800881b8
				if (m_fFuel < 0.0f)
				{
					m_fFuel = 0.0f;
					m_nCount++;
				}
			}
			else
			{
				m_nCount = step + 1;
				if (10 < step)
				{
					// not every flag yet (the lucky flag ended the stage): the fuel comes back
					if ((u32)m_nFlags < (u32)host->MapFlagTotal()) m_fFuel = m_fFuelSave;   // FUN_80079ad0(app +0x298)
					host->UnitStageEnd(false, 0);    // FUN_80126a54(app, 0, 0)
					WiiSnd::Stop(m_SndCount);        // FUN_80026318(+0x9dc, 0)
					m_nCount = 0;
				}
			}
		}
	}
	if (m_pCamera != nullptr)                    // 0x80088138
	{
		int mode = cCamera::eSPEED_NORMAL;
		if (IsSpeedZero()) mode = cCamera::eSPEED_BRAKE;   // 0x80088140 (+0x29c == 0)
		if (0.0f < m_fNitro) mode = cCamera::eSPEED_NITRO;
		if (m_fStartWait != 0.0f) mode = cCamera::eSPEED_NORMAL;
		if (!IsFrozen()) m_pCamera->Update(m_vPos, m_vDir, mode);   // 0x80128f24
	}
	return cTaskUnit::Execute(pParam);           // 0x8007f714
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// the decision

// DOL 0x8007a8e0
void cTaskPlayer::Decide()
{
	// (the DOL first calls get_pad1(sys 0x800c5b98(index)) and drops the result)
	const GamePad::State& pad = GamePad::Get(SysPad(m_nIndex));
	bool nitro = false;
	if (m_pHost != nullptr && m_pHost->nUnitGameState() == 4)   // app +0x2e4
	{
		const uint mask = nView() == 2 ? GamePad::PAD_Z : GamePad::PAD_1;
		nitro = m_pCamera != nullptr && (pad.hold & mask) != 0;
	}
	if (nitro)
	{
		if (m_fNitro <= 0.0f) Nitro();           // 0x80087424
		else NitroFree();                        // 0x80087744 (holding on: the nitro time restarts without fuel)
	}
	else
	{
		const uint mask = nView() == 2 ? GamePad::PAD_C : GamePad::PAD_2;
		if ((pad.trig & mask) != 0)
		{
			BrakeOn();                           // 0x800879a4
			Se("m02se_05Break");                 // FUN_80129d78(handle, cue, 0, actor +0x378)
			// + the remote speaker FUN_80025b6c(handle, sys 0x800c5b98(index), cue, 0): NOT PORTED
		}
		else if ((pad.hold & mask) != 0) BrakeOn();
		else BrakeOff();                         // 0x80087a3c
	}
	if (IsSmokeReq() && IsSmokeFree()) Smoke();  // vtable +0xa0, 0x80088158, 0x80087bd4
	// FUN_80087994: an empty function
	Dec1(0.0f, m_fPadWait);                      // 0x800c547c(0, +0x9cc)
	const int view = SysView(m_nIndex);          // sys 0x800c5b88
	if (view == 2) SteerStick();
	else if (view == 1 || view == 3) SteerRemote();   // 0x8007ae38
}

// the view 2 part of 0x8007a8e0: the Nunchuk stick (pad +0x80 x, +0x84 y; up = +y) toward an absolute direction, by the start direction
// (+0x240). Want(n) = 0x80087d04 / 0x80087dc4 / 0x80087e84 / 0x80087f44 for n = 0 / 1 / 2 / 3.
void cTaskPlayer::SteerStick()
{
	const GamePad::State& pad = GamePad::Get(SysPad(m_nIndex));
	const float x = pad.stick[0], y = pad.stick[1];
	// [start direction][x right, x left, y up, y down]
	static const int WANT[4][4] = {
		{3, 2, 0, 1},   // start 0 (facing -z)
		{2, 3, 1, 0},   // start 1 (+z)
		{0, 1, 2, 3},   // start 2 (-x)
		{1, 0, 3, 2},   // start 3 (+x)
	};
	if ((unsigned)m_nStartDir > 3u) return;      // (the DOL switch has no other case)
	const int* w = WANT[m_nStartDir];
	if (STICK < x) Want(w[0]);
	else if (x < -STICK) Want(w[1]);
	else if (STICK < y) Want(w[2]);
	else if (y < -STICK) Want(w[3]);
	else m_nSteer = 1;
}

// DOL 0x8007ae38 (acc: pad +0x30 x / +0x34 y / +0x38 z)
void cTaskPlayer::SteerRemote()
{
	const GamePad::State& pad = GamePad::Get(SysPad(m_nIndex));
	const cParamCarPlayer& q = *m_pPlayerParam;
	if (pad.acc[1] < q.fPadTurn)                 // +0x34 < PAD_TURN (+0x28): the U-turn jerk
	{
		m_nSteer = 4;
		if (!IsSpeedZero() && (m_fPadWait == 0.0f || m_fPadKind != 4.0f)) ReqBack();   // 0x80088140, FLOAT_8047a9dc, 0x80087ce8
	}
	if (q.fPadLeft < pad.acc[2])                 // +0x38 > PAD_LEFT (+0x20)
	{
		m_nSteer = 2;
		if (m_fPadWait == 0.0f || m_fPadKind != 2.0f) ReqLeft();   // FLOAT_8047a9e4, 0x80087ca8
	}
	else if (pad.acc[2] < q.fPadRight)           // below PAD_RIGHT (+0x24)
	{
		m_nSteer = 3;
		if (m_fPadWait == 0.0f || m_fPadKind != 3.0f) ReqRight();  // FLOAT_8047a9e0, 0x80087cc8
	}
	else m_nSteer = 1;
	// the D-pad looks around: 0x80128f1c(camera, angle) (the camera pointer is not checked in the DOL)
	float look = 0.0f;
	if ((pad.hold & GamePad::PAD_UP) != 0) look = HALF_PI;
	else if ((pad.hold & GamePad::PAD_LEFT) != 0) look = PI;
	else if ((pad.hold & GamePad::PAD_DOWN) != 0) look = -HALF_PI;
	if (m_pCamera != nullptr && CameraRotYGoal != nullptr) CameraRotYGoal(m_pCamera, look);
}

// DOL 0x8007afcc
void cTaskPlayer::OnTurn(int nKind)
{
	m_fPadKind = (float)nKind;
	m_fPadWait = nKind == 4 ? m_pPlayerParam->fPadWaitTurn : m_pPlayerParam->fPadWait;   // +0x30 / +0x2c
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// event hooks

// DOL 0x8007b01c (Crash 0x80088264 always returns 1: the Bounce 0x80084984 never runs)
void cTaskPlayer::OnCrash(cTaskUnit* pOther)
{
	if (!Crash()) Bounce(pOther);
}

// DOL 0x8007b068
void cTaskPlayer::OnStone()
{
	if (ArmorHit()) Crash();
}

// DOL 0x8007b0a4
void cTaskPlayer::OnSmoke(cTaskGimmick* g)
{
	(void)g;
	SmokeHit();                                  // 0x80084c58
}

// DOL 0x8007b0a8
void cTaskPlayer::OnOil(cTaskGimmick* g)
{
	(void)g;
	SpinStart(1);                                // 0x80088004
}

// DOL 0x8007b0b0 (once per spike cell: +0x2d8 is cleared when the car leaves the cell)
void cTaskPlayer::OnSpike(cTaskGimmick* g)
{
	(void)g;
	if (m_bSpike) return;
	if (SpinStart(2)) Se("m02se_09Punk");
	if (m_nFlags != 0)
	{
		if (DropFlag()) nFlags(m_nFlags - 1);    // 0x800881a0
		m_bSpecial = 0;                          // +0x2e8
		m_nFlagChain = 0;                        // +0x2ec
	}
	m_bSpike = true;
}

// DOL 0x8007b194
void cTaskPlayer::OnMud(cTaskGimmick* g)
{
	(void)g;
	if (0.0f < m_fSpeed) MudOn();                // 0x800885bc
}

// DOL 0x8007b1ac
void cTaskPlayer::OnItemFuel(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	FuelBurstOn();                               // 0x80088440
}

// DOL 0x8007b22c
void cTaskPlayer::OnItemPaint(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	ChameleonOn();                               // 0x8008847c
}

// DOL 0x8007b2ac
void cTaskPlayer::OnItemSmoke(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	if (m_nSmokeStock < 5) m_nSmokeStock++;
}

// DOL 0x8007b338
void cTaskPlayer::OnItemArmor(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	ArmorOn();                                   // 0x800884bc
}

// the shared tail of the flag hooks 0x8007b3b8 / 0x8007b4d0 / 0x8007b610: count, chain (max 20), score 100 (200 after the special flag)
// times the chain (single player), then the score effect at the gimmick (0x800881dc with cTaskModel::pModel(g, 0))
void cTaskPlayer::ScoreFlag(cTaskGimmick* g, const char* cue)
{
	Se(cue);
	nFlags(m_nFlags + 1);
	m_nFlagChain++;
	if (0x14 < m_nFlagChain) m_nFlagChain = 0x14;
	const int base = m_bSpecial != 0 ? 200 : 100;
	if (!IsMultiStage()) AddScore(base * m_nFlagChain);   // sys 0x800c5ca0
}

// DOL 0x8007b3b8
void cTaskPlayer::OnFlagSpecial(cTaskGimmick* g)
{
	m_bSpecial = 1;                              // (set before the score: the special flag itself already scores double)
	ScoreFlag(g, "m02se_11Frag2");
	FlagFx(m_nFlagChain, m_bSpecial, GimmickPos(g));
}

// DOL 0x8007b4d0
void cTaskPlayer::OnFlagLucky(cTaskGimmick* g)
{
	ScoreFlag(g, "m02se_11Frag2");
	if (m_pHost != nullptr) m_pHost->UnitStageEnd(true, (int)nUnitId());   // FUN_80126a54(app, 1, vtable +0x44)
	FlagFx(m_nFlagChain, m_bSpecial, GimmickPos(g));
}

// DOL 0x8007b610
void cTaskPlayer::OnFlag(cTaskGimmick* g)
{
	ScoreFlag(g, "m02se_10Frag1");
	FlagFx(m_nFlagChain, m_bSpecial, GimmickPos(g));
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// the boss stage's player

// DOL 0x80124ac4
cTaskPlayerRedMode::cTaskPlayerRedMode(cRallyUnitHost* pHost, int nColour, int nIndex) : cTaskPlayer(pHost, nColour, nIndex)
{
	SetName("g02::cTaskPlayerRedMode");
}

// DOL 0x8007b79c
void cTaskPlayerRedMode::OnSmoke(cTaskGimmick* g)
{
	(void)g;
	SmokeHit();
}

// DOL 0x8007b7a0
void cTaskPlayerRedMode::OnOil(cTaskGimmick* g)
{
	(void)g;
	SpinStart(1);
}

// DOL 0x8007b7ac
void cTaskPlayerRedMode::OnMud(cTaskGimmick* g)
{
	(void)g;
	if (0.0f < m_fSpeed) MudOn();
}

// the player part of the app ctor's car loop (FUN_801236bc)
cTaskPlayer* CreatePlayer(cRallyUnitHost* pHost, int nColour, int nIndex, bool bBossStage)
{
	if (bBossStage) return new cTaskPlayerRedMode(pHost, nColour, nIndex);
	return new cTaskPlayer(pHost, nColour, nIndex);
}

}
