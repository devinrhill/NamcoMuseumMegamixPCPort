#include "kingdom/MslRand.hpp"
#include "kingdom/game/g02/cTaskEnemy.hpp"

#include "kingdom/game/g02/cTaskConsole.hpp"

#include <cstdlib>

#include "GamePad.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g02/cCamera.hpp"
#include "kingdom/game/g02/cParamCar.hpp"
#include "kingdom/game/g02/cTaskMap.hpp"

namespace kingdom::game::g02
{

namespace
{
int Rand15() { return kingdom::MslRand() & 0x7fff; }   // the SDK rand() (0..32767)

Vector4 ModelPos(kingdom::cTaskModel* t)
{
	kingdom::cKingdomModel* m = t != nullptr ? t->pModel(0) : nullptr;   // cTaskModel::pModel(t, 0) 0x8009a430
	return m != nullptr ? m->vTrans() : Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

// the shift of a target cell along a car's grid direction (0 -z, 1 +z, 2 -x, 3 +x), inlined in 0x8006dac0 / 0x8006f094
void Ahead(int dir, int n, int& x, int& z)
{
	switch (dir)
	{
	case 0: z -= n; break;
	case 1: z += n; break;
	case 2: x -= n; break;
	case 3: x += n; break;
	default: break;
	}
}

int Abs(int v) { return v < 0 ? -v : v; }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// construction

// DOL 0x801255a0 [audited]: the cTaskUnit part (cTaskModel ctor, the unit vtable, its Init, +0x1cc = index, the models FUN_8008120c), the cTaskEnemy
// vtable, the +0x9d0 particle handle, then cTaskEnemy::Init (vtable +0x3c)
cTaskEnemy::cTaskEnemy(cRallyUnitHost* pHost, int nColour, int nIndex) : cTaskUnit(pHost, nColour, nIndex)
{
	SetName("g02::cTaskEnemy");
	InitEnemy();
}

// (the DOL deleting dtor 0x8006f784 runs after Term; natively tasks are never deleted: Term runs through cTaskKingdom::TermOnce, this dtor is kept for completeness)
cTaskEnemy::~cTaskEnemy()
{
	delete m_pEnemyParam;
	m_pEnemyParam = nullptr;
}

// DOL 0x8006d500
void cTaskEnemy::InitEnemy()
{
	m_fNearTime = 0.0f;                          // +0x9c8 = FLOAT_8047a968
	m_fWander = 0.0f;                            // +0x9cc
	m_fSpeedGoal = 0.0f;                         // +0x298
	m_pEnemyParam = new cParamCarEnemy();        // 0x58 bytes, CSV_CARENEMY
	m_pEnemyParam->Load();
}

// DOL 0x8006d778 [audited] (the CSV_CARENEMY block only; no chain to cTaskUnit::Term, as in the DOL)
void cTaskEnemy::Term()
{
	if (m_pEnemyParam != nullptr)
	{
		delete m_pEnemyParam;
		m_pEnemyParam = nullptr;
	}
}

// DOL 0x8006e514 [audited]
u32 cTaskEnemy::nUnitId() const { return (u32)m_nIndex | 0x20u; }

// DOL 0x8006d868 [audited]
void cTaskEnemy::Reset()
{
	m_fNearTime = 0.0f;
	m_fWander = 0.0f;
	cTaskUnit::Reset();                          // 0x8007eb80
}

// DOL 0x8006f304 [audited]
float cTaskEnemy::FuelUse() const { return 0.0f; }

// DOL 0x8006f2fc [audited]
float cTaskEnemy::NitroFuel() const { return 0.0f; }

cTaskUnit* cTaskEnemy::pPlayer() const { return m_pHost != nullptr ? m_pHost->pUnit(0) : nullptr; }

// ---------------------------------------------------------------------------------------------------------------------------------------------
// Execute

// DOL 0x8006d878 [audited] (r4 = the app; natively the host)
bool cTaskEnemy::Execute(cJetTask* pParam)
{
	if (IsFrozen()) return true;                 // HOME || app +0x1b4 || app +0x284 (no cTaskUnit::Execute either)
	if (m_pHost == nullptr || !m_pHost->IsUnitDead())   // FUN_80126928 (always 0)
	{
		// the boss stage's state 2 (the start signal): the movement block runs as well (FUN_80084d1c)
		if (IsBossStageNow() && m_pHost != nullptr && m_pHost->nUnitGameState() == 2) StepMove();
		Step();                                  // 0x80084d6c
	}
	if (IsState(7)) LoseClip();                  // 0x8007de2c (the player cleared the stage)
	if (IsState(5) || IsState(6)) WinClip();     // 0x8007dd14 (the player's miss / game over)
	if (m_pCamera != nullptr) m_pCamera->Update(m_vPos, m_vDir, cCamera::eSPEED_NORMAL);   // 0x80128f24 (enemy 0..3 own an unused camera)
	ChameleonStep();                             // 0x8008848c
	if (m_fNearTime == m_pEnemyParam->fRangeInTime)
	{
		// the "confused" mark over the car while it brakes at the painted player (+0x9d0, slot 0x29)
		const Vector4 pos = ModelPos(this);      // (memcpy of 0x10 bytes from cTaskModel::pModel(this, 0))
		if (!m_FxConfused.IsValid()) Fx(m_FxConfused, 0x29, false, true, true, pos);   // 0x8008a574, FUN_80021a9c
		m_FxConfused.vSetTrans(pos);             // 0x8008a49c
	}
	else FxOff(m_FxConfused, false);             // 0x8008a508(+0x9d0, 0)
	FuelBurstStep();                             // 0x80088450
	ChameleonStep();                             // 0x8008848c (a second time, as in the DOL)
	return cTaskUnit::Execute(pParam);           // 0x8007f714
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// the decision

// the turn choice at the end of 0x8006dac0 and 0x8006e664 (identical): nAxis = |dz| - |dx| (< 0: the x distance is the larger), dx / dz =
// own cell - target cell. ReqLeft 0x80087ca8 / ReqRight 0x80087cc8.
void cTaskEnemy::ChaseTurn(int nAxis, int dx, int dz)
{
	const int d = m_nDir;
	if (nAxis < 0)
	{
		if (dx < 0)   // the target lies toward +x
		{
			if (d == 1) ReqLeft();
			else if (d == 0) ReqRight();
			else if (d == 2)
			{
				if (dz < 0) ReqLeft();
				else if (0 < dz) ReqRight();
			}
		}
		else
		{
			if (d == 0) ReqLeft();
			else if (d == 1) ReqRight();
			else if (d == 3)
			{
				if (dz < 0) ReqRight();
				else if (0 < dz) ReqLeft();
			}
		}
	}
	else if (dz < 0)   // toward +z
	{
		if (d == 2) ReqLeft();
		else if (d == 0)
		{
			if (dx < 0) ReqRight();
			else if (0 < dx) ReqLeft();
		}
		else if (d == 3) ReqRight();
	}
	else
	{
		if (d == 2) ReqRight();
		else if (d == 1)
		{
			if (dx < 0) ReqLeft();
			else if (0 < dx) ReqRight();
		}
		else if (d == 3) ReqLeft();
	}
}

// DOL 0x8006dac0 [audited]
void cTaskEnemy::Decide()
{
	cTaskUnit* pl = pPlayer();
	if (pl == nullptr) return;                   // (the DOL reads car 0 unchecked)
	const cParamCarEnemy& q = *m_pEnemyParam;
	const int px = pl->nCellX(), pz = pl->nCellZ();
	const int mx = CellX(), mz = CellZ();
	// out of the search range?
	const bool far = !(Abs(mx - px) < q.nSearchRange && Abs(mz - pz) < q.nSearchRange);
	int ahead = 0;
	if (m_nAiType == 1) ahead = far ? q.nTargetFar : q.nTargetNear;
	// the painted player close by: count up to RANGE_IN_TIME (+0x2c), else 0
	bool away = !(Abs(mx - px) < q.nSearchChameleon && Abs(mz - pz) < q.nSearchChameleon);   // +0x44
	if (!pl->IsChameleon()) away = true;         // 0x80088498
	if (m_fStartWait != 0.0f) away = true;
	if (away) m_fNearTime = 0.0f;
	else Inc1(q.fRangeInTime, m_fNearTime);      // 0x800c53ec
	int tx = px, tz = pz;
	Ahead(pl->nDir(), ahead, tx, tz);
	const int dx = mx - tx, dz = mz - tz;
	bool brake = false;
	if (pl->IsChameleon() && m_fNearTime == q.fRangeInTime)
	{
		BrakeOn();                               // 0x800879a4
		brake = true;
	}
	if (!brake) BrakeOff();                      // 0x80087a3c
	if (far && pl->IsSpeedZero())                // 0x80088140: the player stands still far away -> wander
	{
		if (!Dec1(0.0f, m_fWander)) return;      // 0x800c547c
		ReqLeft();
		const int n = (int)q.fRandChange;
		m_fWander = q.fRandStability + (float)(n != 0 ? Rand15() % n : 0);   // (the DOL divides by it unchecked)
		return;
	}
	ChaseTurn(Abs(dz) - Abs(dx), dx, dz);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// event hooks

// DOL 0x8006e520 [audited]
void cTaskEnemy::OnStone()
{
	SpinStart(5);                                // 0x80088004
	TurnBackHalf();                              // 0x800873e4
}

// DOL 0x8006e558 [audited] (the player's smoke screen: score, the score effect at the car, spin; a flag drops)
void cTaskEnemy::OnSmoke(cTaskGimmick* g)
{
	(void)g;
	if (m_nSpin == 0)
	{
		const int score = m_pEnemyParam->nSpinScore;   // +0x40
		AddScore(score);                         // 0x800881b8
		FlagFx(score / 100, 0, ModelPos(this));  // 0x800881dc(this, score / 100, 0, cTaskModel::pModel(this, 0))
	}
	SpinStart(3);
	if (m_nFlags != 0 && DropFlag()) nFlags(m_nFlags - 1);   // 0x800881a0
}

// DOL 0x8006e618 [audited]
void cTaskEnemy::OnOil(cTaskGimmick* g)
{
	(void)g;
	SpinStart(1);
}

// DOL 0x8006e620 [audited]
void cTaskEnemy::OnSpike(cTaskGimmick* g)
{
	(void)g;
	if (m_bSpike) return;
	SpinStart(1);                                // (kind 1, the oil spin: enemies do not puncture)
	m_bSpike = true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// the boss stage's red car

// DOL 0x801251e8 [audited]
cTaskEnemyRedMode::cTaskEnemyRedMode(cRallyUnitHost* pHost, int nColour, int nIndex) : cTaskEnemy(pHost, nColour, nIndex)
{
	SetName("g02::cTaskEnemyRedMode");
	m_fNearTime2 = 0.0f;                         // +0xa24 = FLOAT_8047bfe4
	m_fNitroTime = 0.0f;                         // +0xa28
}

// DOL 0x8006f094 [audited]: the next flag (map FUN_80079fcc) moved ahead along the PLAYER's direction by nAhead; out = own cell - that cell. With every
// flag taken the DOL writes the car's own cell (not a difference) instead.
void cTaskEnemyRedMode::FlagTarget(int out[2], int nAhead) const
{
	const int total = m_pHost != nullptr ? m_pHost->MapFlagTotal() : 0;   // FUN_80079ad0(app +0x298)
	if (m_nFlags == total)
	{
		out[0] = CellX();
		out[1] = CellZ();
		return;
	}
	float f[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	if (m_pHost != nullptr) m_pHost->MapNextFlag(m_nFlags, f);   // FUN_80079fcc(map, +0x2d0)
	int fx = CellOf(f[0]) + 4, fz = CellOf(f[2]) + 8;
	if (const cTaskUnit* pl = pPlayer()) Ahead(pl->nDir(), nAhead, fx, fz);
	out[0] = CellX() - fx;
	out[1] = CellZ() - fz;
}

// DOL 0x8006e664 [audited]
void cTaskEnemyRedMode::Decide()
{
	cTaskUnit* pl = pPlayer();
	if (pl == nullptr) return;                   // (the DOL reads car 0 unchecked)
	const cParamCarEnemy& q = *m_pEnemyParam;
	const int px = pl->nCellX(), pz = pl->nCellZ();
	const int mx = CellX(), mz = CellZ();
	const bool far = !(Abs(mx - px) < q.nSearchRange && Abs(mz - pz) < q.nSearchRange);
	int ahead = 0;
	if (m_nAiType == 1) ahead = far ? q.nTargetFar : q.nTargetNear;
	int a[2] = {0, 0};
	FlagTarget(a, ahead);
	const int axis = Abs(a[1]) - Abs(a[0]);
	int d[2] = {0, 0};
	FlagTarget(d, ahead);                        // (computed twice in the DOL)
	if (!Dec1(0.0f, m_fNitroTime)) NitroFree();  // 0x800c547c(+0xa28), 0x80087744: the free nitro lasts NITRO_USE_TIME
	// the player close by for RED_RANGE_IN_TIME (+0x4c) frames: nitro (rand even) or a smoke screen
	const bool away = !(Abs(mx - px) < q.nSearchChameleon && Abs(mz - pz) < q.nSearchChameleon);   // +0x44 (as in 0x8006dac0)
	if (away) m_fNearTime2 = 0.0f;
	else Inc1(q.fRedRangeInTime, m_fNearTime2);  // 0x800c53ec
	if (m_fNearTime2 == q.fRedRangeInTime)
	{
		if (Rand15() % 2 == 0)
		{
			Nitro();                             // 0x80087424
			m_fNitroTime = q.fNitroUseTime;      // +0x50
		}
		else Smoke();                            // 0x80087bd4
		m_fNearTime2 = 0.0f;
	}
	ChaseTurn(axis, d[0], d[1]);
}

// DOL 0x8006ecf8 [audited] (the player touched it: unless every flag is in, the stage is won)
void cTaskEnemyRedMode::OnCrash(cTaskUnit* pOther)
{
	const int total = m_pHost != nullptr ? m_pHost->MapFlagTotal() : 0;   // FUN_80079ad0
	if (m_nFlags == total) return;
	if (m_pHost != nullptr)
	{
		m_pHost->UnitRedCrash();                 // FUN_80126ac8(app)
		m_pHost->UnitRedCrash();                 // (called twice in the DOL)
	}
	if (!Crash()) Bounce(pOther);                // 0x80088264 (always 1), 0x80084984
}

// DOL 0x8006ed84 [audited]
void cTaskEnemyRedMode::OnSmoke(cTaskGimmick* g)
{
	(void)g;
	SmokeHit();                                  // 0x80084c58
}

// DOL 0x8006ed88 [audited]
void cTaskEnemyRedMode::OnOil(cTaskGimmick* g)
{
	(void)g;
	SpinStart(1);
}

// DOL 0x8006ed94 [audited]
void cTaskEnemyRedMode::OnMud(cTaskGimmick* g)
{
	(void)g;
	if (0.0f < m_fSpeed) MudOn();                // 0x800885bc
}

// DOL 0x8006edac [audited]
void cTaskEnemyRedMode::OnItemFuel(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	FuelBurstOn();                               // 0x80088440
}

// DOL 0x8006ee2c [audited]
void cTaskEnemyRedMode::OnItemPaint(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	ChameleonOn();                               // 0x8008847c
}

// DOL 0x8006eeac [audited]
void cTaskEnemyRedMode::OnItemSmoke(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	if (m_nSmokeStock < 5) m_nSmokeStock++;
}

// DOL 0x8006ef38 [audited]
void cTaskEnemyRedMode::OnItemArmor(cTaskGimmick* g)
{
	(void)g;
	Se("m02se_12Item");
	ArmorOn();                                   // 0x800884bc
}

// DOL 0x8006efc0 [audited] (no score: the red car's flags count against the player)
void cTaskEnemyRedMode::OnFlag(cTaskGimmick* g)
{
	cTaskConsole::PlaySE("m02se_10Frag1");       // FUN_80129d78(handle, cue, 0, 0): no actor
	if (m_pHost != nullptr)
	{
		kingdom::cKingdomModel* m = g != nullptr ? g->pModel(0) : nullptr;
		m_pHost->FxRedFlag(m != nullptr ? m->vTrans() : Vector4(0.0f, 0.0f, 0.0f, 1.0f));   // FUN_8006d114(app +0x1bc effect, pModel(g, 0))
	}
	nFlags(m_nFlags + 1);
}

// the enemy part of the app ctor's car loop (FUN_801236bc)
cTaskEnemy* CreateEnemy(cRallyUnitHost* pHost, int nColour, int nIndex, bool bBossStage)
{
	if (bBossStage) return new cTaskEnemyRedMode(pHost, nColour, nIndex);
	return new cTaskEnemy(pHost, nColour, nIndex);
}

}
