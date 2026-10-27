#include "kingdom/game/g11/cTaskGrobdaEnemy.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g11/cTaskGrobdaCombo.hpp"
#include "kingdom/game/g11/cTaskGrobdaEffect.hpp"
#include "kingdom/game/g11/cTaskGrobdaShot.hpp"
#include "kingdom/game/g11/cTaskGrobdaStage.hpp"

namespace kingdom::game::g11
{

// ---- the direction tables ---------------------------------------------------------------------------------------------------------------

namespace grobda_dir
{

namespace
{
// DOL 0x8038ab60: eight directions, 0x8038abe0: sixteen (vec4 each)
const float DIR8[8][4] = {{0.0f, 0.0f, 1.0f, 0.0f},           {0.70710678f, 0.0f, 0.70710678f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f},
                          {0.70710678f, 0.0f, -0.70710678f, 0.0f}, {0.0f, 0.0f, -1.0f, 0.0f},         {-0.70710678f, 0.0f, -0.70710678f, 0.0f},
                          {-1.0f, 0.0f, 0.0f, 0.0f},          {-0.70710678f, 0.0f, 0.70710678f, 0.0f}};
float s_Dir16[16][4];
bool s_bDir16 = false;

void RotY(float* v, float a)
{
	const float c = std::cos(a), s = std::sin(a);
	const float x = v[0] * c + v[2] * s, z = -v[0] * s + v[2] * c;
	v[0] = x;
	v[2] = z;
}

void FillDir16()
{
	if (s_bDir16) return;
	s_bDir16 = true;
	for (int i = 0; i < 16; i++)
	{
		const float a = (float)i * 0.39269908f;   // 22.5 degrees
		s_Dir16[i][0] = std::sin(a);
		s_Dir16[i][1] = 0.0f;
		s_Dir16[i][2] = std::cos(a);
		s_Dir16[i][3] = 0.0f;
	}
}

float Turn(const float table[][4], float step, int dir, int from, uint t, uint total, float out[4])
{
	const float a = step * ((float)t / (float)total);
	float v[4] = {table[from][0], table[from][1], table[from][2], table[from][3]};
	if (dir == 0)
	{
		if (a != 0.0f) RotY(v, a);
	}
	else if (dir == 1)
	{
		if (-a != 0.0f) RotY(v, -a);
	}
	if (out != nullptr) std::memcpy(out, v, 16);
	return AngleOf(v);
}
}

const float* Dir8(int i) { return DIR8[i & 7]; }
const float* Dir16(int i) { FillDir16(); return s_Dir16[i & 15]; }
float AngleOf(const float* v) { return std::atan2(v[0], v[2]); }   // 0x80300950 * (pi / 128)
bool IsNonZero(const float* v) { return !(std::fabs(v[0]) < 1.17549435e-38f && std::fabs(v[2]) < 1.17549435e-38f); }

float Turn8(int dir, int from, uint t, uint total, float out[4]) { return Turn(DIR8, 0.78539816f, dir, from, t, total, out); }
float Turn16(int dir, int from, uint t, uint total, float out[4]) { FillDir16(); return Turn(s_Dir16, 0.38397244f, dir, from, t, total, out); }   // FLOAT_80479f48 = 22 degrees

int Nearest8(const float* v)
{
	float w[4] = {v[0], 0.0f, v[2], 0.0f};
	int best = 0;
	float bd = w[0] * DIR8[0][0] + w[2] * DIR8[0][2];
	for (int i = 1; i < 8; i++)
	{
		const float d = w[0] * DIR8[i][0] + w[2] * DIR8[i][2];
		if (bd < d) { best = i; bd = d; }
	}
	return best;
}

int Nearest16(const float* v)
{
	FillDir16();
	float w[4] = {v[0], 0.0f, v[2], 0.0f};
	int best = 0;
	float bd = w[0] * s_Dir16[0][0] + w[2] * s_Dir16[0][2];
	for (int i = 1; i < 16; i++)
	{
		const float d = w[0] * s_Dir16[i][0] + w[2] * s_Dir16[i][2];
		if (bd < d) { best = i; bd = d; }
	}
	return best;
}

}

namespace
{
// DOL 0x8002896c: the player nearest to `pos` (XZ); null without players
cTaskGrobdaModel* NearestPlayer(const float* pos)
{
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st == nullptr) return nullptr;
	cTaskGrobdaModel* best = nullptr;
	float bd = 0.0f;
	for (int i = 0; i < st->nPlayers(); i++)
	{
		cTaskGrobdaModel* p = st->Player(i);
		if (p == nullptr) continue;
		const float* pp = p->GetPos();
		const float dx = pp[0] - pos[0], dz = pp[2] - pos[2];
		const float d = dx * dx + dz * dz;
		if (best == nullptr || d < bd) { best = p; bd = d; }
	}
	return best;
}
}

// ---- cTaskGrobdaEnemy --------------------------------------------------------------------------------------------------------------------

// DOL 0x80026e54 (+ Init 0x80026fa0, run from the ctor through vtable +0x88)
cTaskGrobdaEnemy::cTaskGrobdaEnemy(uint type)
{
	SetName("cTaskGrobdaEnemy");
	SetType(type);
	m_pSpark = new kingdom::cKingdomPtkHnd();
	m_pSpark->Generate(0x20, false, true, true, 0.0f, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	if (m_pSpark->IsValid()) { m_pSpark->Inactive(); m_pSpark->Invisible(); }
	m_pFragment = new kingdom::cKingdomPtkHnd();
	m_pFragment->Generate(0x1f, false, true, true, 0.0f, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	if (m_pFragment->IsValid()) { m_pFragment->Inactive(); m_pFragment->Invisible(); }
	SetFlags(0);   // VCALL(+0x98)
	m_nPhase = 0;
	m_bEnter = true;
}

// DOL 0x800271e8 (Term, run by cTaskKingdom::TermOnce when the job releases the task): each handle is detached (FUN_8008a508(.., 1)) and
// destroyed (vtable +0x10 with 1)
void cTaskGrobdaEnemy::ReleaseTerm()
{
	for (kingdom::cKingdomPtkHnd** h : {&m_pSpark, &m_pFragment})
		if (*h != nullptr)
		{
			(*h)->Detach(true);
			delete *h;
			*h = nullptr;
		}
	cTaskGrobdaModel::ReleaseTerm();
}

cTaskGrobdaEnemy::~cTaskGrobdaEnemy()
{
	delete m_pSpark;
	delete m_pFragment;
}

// DOL 0x80027b48
bool cTaskGrobdaEnemy::TakeEnter(bool clear)
{
	if (m_bEnter)
	{
		if (clear) m_bEnter = false;
		return true;
	}
	return false;
}

// DOL 0x80036b68: the object moves to another model slot (translation, rotation, scale), the old one hides, the new one shows
void cTaskGrobdaEnemy::SwitchModel(int idx)
{
	if (idx == m_nModelIdx) return;
	cKingdomModel* a = pModel((uint)m_nModelIdx);
	cKingdomModel* b = pModel((uint)idx);
	if (a != nullptr && b != nullptr)
	{
		b->vTrans() = a->vTrans();
		b->vRotate() = a->vRotate();
		b->vScale() = a->vScale();
		a->Invisible();    // FUN_8001a330
		b->Visible();      // FUN_8001a1f4
	}
	m_nModelIdx = idx;
}

// DOL 0x800272ec
bool cTaskGrobdaEnemy::Execute(cJetTask* pParam)
{
	if (cTaskGrobdaCollision::Running())
	{
		switch (m_nPhase)
		{
		case 0: Phase0(); break;
		case 1: Phase1(); break;
		case 2: Phase2(); break;
		case 3: Phase3(); break;
		case 4: Phase4(); break;
		case 5: Phase5(); break;
		case 6: Phase6(); break;
		case 7: Phase7(); break;
		}
		UpdateFrame();   // FUN_8009a478
		m_bTouchMover = false;
		m_bTouchPickup = false;
	}
	return cTaskGrobdaModel::Execute(pParam);
}

// DOL 0x80027bc0: wait for the stage to start playing
void cTaskGrobdaEnemy::Phase0()
{
	if (m_bEnter) { m_bEnter = false; SetFlags(0); }
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();   // FUN_8009ffac -> 0x80039300: stage state == 2
	if (st != nullptr && st->IsPlaying()) { m_nPhase = 2; m_bEnter = true; }
}

// DOL 0x80027c48: dropping in from y = 50 once the start delay is over; the footprint joins list 8 below y = 1.5
void cTaskGrobdaEnemy::Phase1()
{
	float* pos = GetPos();
	std::memcpy(GetOldPos(), pos, 16);
	if (m_bEnter)
	{
		m_bEnter = false;
		SetFlags(1);
		pos[1] = 50.0f;               // FLOAT_80479f20
		m_vMove[1] = -1.0f;           // FLOAT_80479f24 (the move's y: vtable +0x34(1))
		UpdateShape();
		TimerReset(0);
		TimerStart(0);
	}
	const uint t = TimerMs(0);
	if ((int)t >= m_nDelay)
	{
		m_vMove[1] -= 0.004f;         // FLOAT_80479f28
		pos[1] += m_vMove[1];
		if (pos[1] <= 1.5f)           // FLOAT_80479f2c
		{
			UpdateShape();
			cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
			if (st != nullptr && !st->IsFinishing() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(8, this, nullptr);
		}
		if (pos[1] <= 0.0f)
		{
			pos[1] = 0.0f;
			m_vMove[1] = 0.0f;
			TimerStop(0);
			m_nPhase = 2;
			m_bEnter = true;
		}
	}
}

// DOL 0x80027f24: active; the stage finishing ends it
void cTaskGrobdaEnemy::Phase2()
{
	if (m_bEnter) { m_bEnter = false; SetFlags(2); }
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && st->IsFinishing()) { m_nPhase = 6; m_bEnter = true; }
}

// DOL 0x80027fb8: destroyed: the fragment effect at the wreck
void cTaskGrobdaEnemy::Phase3()
{
	if (m_bEnter)
	{
		m_bEnter = false;
		SetFlags(3);
		if (m_pFragment != nullptr && m_pFragment->IsValid())
		{
			m_pFragment->Active();
			m_pFragment->Visible();
			const float* p = GetPos();
			const float* r = GetRot();
			m_pFragment->vSetTrans(Vector4(p[0], p[1], p[2], 1.0f));
			m_pFragment->vSetRotate(Vector4(r[0], r[1], r[2], 1.0f));
		}
	}
}

// DOL 0x800280b0: the wreck; half of the time the spark effect shows
void cTaskGrobdaEnemy::Phase4()
{
	if (m_bEnter)
	{
		m_bEnter = false;
		SetFlags(4);
		if ((std::rand() & 1) == 0 && m_pSpark != nullptr && m_pSpark->IsValid())   // UNVERIFIED: the sign trick of the DOL's rand() & 1 test
		{
			m_pSpark->Active();
			m_pSpark->Visible();
			const float* p = GetPos();
			const float* r = GetRot();
			m_pSpark->vSetTrans(Vector4(p[0], p[1], p[2], 1.0f));
			m_pSpark->vSetRotate(Vector4(r[0], r[1], r[2], 1.0f));
		}
	}
}

// DOL 0x800281a8: the wreck leaves
void cTaskGrobdaEnemy::Phase5()
{
	if (m_bEnter)
	{
		m_bEnter = false;
		SetFlags(5);
		if (m_pSpark != nullptr && m_pSpark->IsValid()) { m_pSpark->Inactive(); m_pSpark->Invisible(); }
	}
}

// DOL 0x800282a8: the stage ended
void cTaskGrobdaEnemy::Phase6()
{
	if (m_bEnter) { m_bEnter = false; SetFlags(6); }
}

// DOL 0x80028234: gone once no shot is left in the air
void cTaskGrobdaEnemy::Phase7()
{
	if (m_bEnter) m_bEnter = false;
	if (!HasShots()) SetFlags(7);
}

// DOL 0x8002777c: only an active enemy (flag word 2) reads its records: movers / walls (kinds 1, 2) and wrecks (4) set flags, shots and blasts (5, 3) hurt it
void cTaskGrobdaEnemy::Resolve(int a, int b)
{
	(void)b;
	if (a != 0 || nFlags() != 2u) return;
	std::vector<const sGrobdaHit*> hits;
	for (const sGrobdaHit& h : HitList())
	{
		if (h.kind == 1 || h.kind == 2) m_bTouchMover = true;
		else if (h.kind == 4) m_bTouchPickup = true;
		else if (h.kind == 5 || h.kind == 3) hits.push_back(&h);
	}
	if (!hits.empty()) Damage(hits);
}

// DOL 0x80027488: hit points; at 0 the enemy blows up (phase 3) and the nearest hit's owner scores it
void cTaskGrobdaEnemy::Damage(std::vector<const sGrobdaHit*>& hits)
{
	Hp() -= (int)hits.size();
	if (Hp() > 0) return;
	Hp() = 0;
	m_nPhase = 3;
	m_bEnter = true;
	const sGrobdaHit* nearest = nullptr;
	for (const sGrobdaHit* h : hits)
		if (nearest == nullptr || h->dist < nearest->dist) nearest = h;
	if (nearest == nullptr || nearest->pObj == nullptr) return;
	int player = 0;
	uint ctype = 0x4000001u;
	if (cG11GrobdaSys::Instance()->IsMulti())
	{
		const uint t = static_cast<cGrobdaObject*>(nearest->pObj)->nType();
		if (t >= 0x1000001u && t <= 0x1000004u) player = (int)(t - 0x1000001u), ctype = 0x4000001u + (uint)player;
		else if (t >= 0x4000001u && t <= 0x4000004u) player = (int)(t - 0x4000001u), ctype = 0x4000001u + (uint)player;
		else if (t >= 0x4000006u && t <= 0x4000009u) player = (int)(t - 0x4000006u), ctype = 0x4000001u + (uint)player;
		else { player = 0; ctype = 0x4000005u; }
	}
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st == nullptr) return;
	cTaskGrobdaCombo* combo = static_cast<cTaskGrobdaCombo*>(nearest->tag);   // record +0x40: the shooter's combo (a blast carries it)
	if (combo == nullptr) combo = st->AddCombo(ctype);   // 0x80034224 + 0x800397b8
	if (ctype != 0x4000005u && player < 4)
	{
		int pts = BaseScore();
		if (combo != nullptr) pts += std::min(combo->nCount(), 9) * ComboStep();
		st->AddScore(player, pts);   // FUN_8009ffcc
	}
	if (combo != nullptr) combo->Hit(GetPos(), nType() == 0x20000006u ? 1 : 0, true);   // 0x8003458c
}

// ---- cTaskGrobdaEnemyTank -------------------------------------------------------------------------------------------------------------

// DOL 0x8002c704 (Init) + the kind's Init: Purple 0x8002e7f0 (ENEMY_01, no turret), Orange 0x8002dff4 (ENEMY_02), Yellow 0x8002ec98 (ENEMY_03): the turret kinds have a shield effect (slot 0x1e)
cTaskGrobdaEnemyTank::cTaskGrobdaEnemyTank(uint type, int model)
	: cTaskGrobdaEnemy(type), m_nKindType(type)
{
	SetName("cTaskGrobdaEnemyTank");
	SetMode(0);   // 0x80027b84(this, 0)
	static const char* const BODY[3] = {"BIN_MDL_ENEMY_01", "BIN_MDL_ENEMY_02", "BIN_MDL_ENEMY_03"};
	static const char* const CANNON[3] = {"BIN_MDL_ENEMY_01_CANNON", "BIN_MDL_ENEMY_02_CANNON", "BIN_MDL_ENEMY_03_CANNON"};
	static const char* const DEBRIS[3] = {"BIN_MDL_DEBRIS_ENEMY_01", "BIN_MDL_DEBRIS_ENEMY_02", "BIN_MDL_DEBRIS_ENEMY_03"};
	static const char* const CSV[3] = {"EnmTankPurpleParam.csv", "EnmTankOrangeParam.csv", "EnmTankYellowParam.csv"};
	m_bTurret = model != 0;
	if (m_bTurret)
	{
		m_pShield = new kingdom::cKingdomPtkHnd();
		m_pShield->Generate(0x1e, false, true, true, 0.0f, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (m_pShield->IsValid()) { m_pShield->Inactive(); m_pShield->Invisible(); }
	}
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(BODY[model], 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(1);
	m->RegistExpand(0, "cannon", CANNON[model], 0x36f);
	if (cKingdomModel* c = m->pExpandMdl(0)) { c->SetLightSetIdx(1); c->SetFogIdx(1); }
	m->Visible();   // FUN_8001a1f4
	cKingdomModel* d = CreateModel(1);
	d->RegistModel(DEBRIS[model], 0x36f);
	d->SetLightSetIdx(1);
	d->SetFogIdx(1);
	d->Invisible();   // FUN_8001a330
	Anim(0);
	m_nModelIdx = 0;
	CreateShots(type);
	m_fRadius = 0.8f;   // FLOAT_80476a60 / 48 / 78
	{
		const float box[4][4] = {{1.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 1.0f, 0.0f}};   // FUN_8019334c(2, 2)
		SetShape(box, 4, false);
	}
	LoadParams(CSV[model]);
	// the first three words of column 1 (rows 11 .. 13) before SetStart: HP, score, combo step
	Hp() = m_Csv.S32(11, 1, 0xff);
	BaseScore() = m_Csv.S32(12, 1, 0xff);
	ComboStep() = m_Csv.S32(13, 1, 0xff);
}

void cTaskGrobdaEnemyTank::LoadParams(const char* csv)
{
	m_Csv.RegistData(csv);
	for (int l = 0; l < 3; l++)
	{
		sTankParam& p = m_Par[l];
		const int col = l + 1;
		p.speed = m_Csv.F32(0, col, 0.0f);
		p.speedDebris = m_Csv.F32(1, col, 0.0f);
		p.shotSpeed = m_Csv.F32(2, col, 0.0f);
		p.shotCount = (uint)m_Csv.F32(3, col, 0.0f);
		p.moveTime = (uint)m_Csv.F32(4, col, 0.0f);
		p.moveMin = (uint)m_Csv.F32(5, col, 0.0f);
		p.turnTime = (uint)m_Csv.F32(6, col, 0.0f);
		p.wallWait = (uint)m_Csv.F32(7, col, 0.0f);
		p.shotInterval = (uint)m_Csv.F32(8, col, 0.0f);
		p.shotIntervalSeen = (uint)m_Csv.F32(9, col, 0.0f);
		p.turretTime = (uint)m_Csv.F32(10, col, 0.0f);
	}
}

// DOL 0x8002ca34: eight enemy shots, children of the enemy
void cTaskGrobdaEnemyTank::CreateShots(uint type)
{
	for (int i = 0; i < 8; i++)
	{
		m_pShot[i] = new cTaskGrobdaShotEnemy(type);
		m_pShot[i]->m_fRadius = 0.15f;   // FLOAT_80476a30 (0x3e19999a)
		ConnectChild(m_pShot[i]);
	}
}

// DOL 0x8002cad0
bool cTaskGrobdaEnemyTank::HasShots()
{
	for (cTaskGrobdaShotEnemy* s : m_pShot)
		if (s != nullptr && s->nState() != 0) return true;
	return false;
}

void cTaskGrobdaEnemyTank::SetRotY(float a)
{
	cKingdomModel* m = pModel((uint)m_nModelIdx);
	if (m != nullptr) m->vRotate().y = a;
}

// DOL 0x8002c830: the record's three words: the angle in degrees (-> the first of the eight directions), the level (0..2) and the start delay
void cTaskGrobdaEnemyTank::SetStart(int level, int delay, int angleDeg)
{
	int deg = angleDeg;
	if (deg < 0) deg += 360;
	else if (deg >= 360) deg -= 360;
	int idx = 0;
	for (int k = 1; k <= 8; k++)
		if ((uint)deg < (uint)(k * 0x2d)) { idx = k - 1; break; }
	std::memcpy(m_vDir, grobda_dir::Dir8(idx), 16);
	m_nDir = idx;
	if (grobda_dir::IsNonZero(m_vDir)) SetRotY(grobda_dir::AngleOf(m_vDir));
	SetLevel(level, delay);
	Hp() = m_Csv.S32(11, m_nLevel + 1, 0xff);
	BaseScore() = m_Csv.S32(12, m_nLevel + 1, 0xff);
	ComboStep() = m_Csv.S32(13, m_nLevel + 1, 0xff);
}

// DOL 0x8002cb30: the clip sets (CRC names of the stage archive): 0 WAIT, 1 RUN (+ texture scroll), 2 ROT_L, 3 ROT_R, 4 SHOT (blend 1), 5 DIE
void cTaskGrobdaEnemyTank::Anim(int mode)
{
	m_nAnim = mode;
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	auto reg = [&](int blend, int kind, const char* name)
	{
		if (m_sAnim[blend][kind] == name) return;   // FUN_80036c30: already registered
		m_sAnim[blend][kind] = name;
		m->RegistAnm((uint)blend, name);
	};
	switch (mode)
	{
	case 0: reg(0, 0, "BIN_CHA_ENEMY_TANK_WAIT"); break;
	case 1: reg(0, 0, "BIN_CHA_ENEMY_TANK_RUN"); reg(0, 1, "BIN_TSA_ENEMY_TANK_RUN"); break;
	case 2: reg(0, 0, "BIN_CHA_ENEMY_TANK_ROT_L"); reg(0, 1, "BIN_TSA_ENEMY_TANK_ROT_L"); break;
	case 3: reg(0, 0, "BIN_CHA_ENEMY_TANK_ROT_R"); reg(0, 1, "BIN_TSA_ENEMY_TANK_ROT_R"); break;
	case 4: reg(1, 0, "BIN_CHA_ENEMY_TANK_SHOT"); break;
	case 5: reg(0, 0, "BIN_CHA_ENEMY_TANK_DIE"); break;
	}
}

// DOL 0x8002d548: Orange / Yellow track the nearest player with the cannon (sixteen directions); Purple's cannon follows the body
void cTaskGrobdaEnemyTank::Cannon()
{
	if (!m_bTurret)
		std::memcpy(m_vTurret, m_vDir, 16);
	else
	{
		switch (m_nTurretState)
		{
		case 0:
			TimerReset(2);
			TimerStart(2);
			m_nTurretState = 1;
			break;
		case 1:
			if (TimerMs(0) >= 600) m_nTurretState = 2;
			break;
		case 2:
		{
			int target = m_nTurretDir;
			if (cTaskGrobdaModel* pl = NearestPlayer(GetPos()))
			{
				float d[4] = {pl->GetPos()[0] - GetPos()[0], 0.0f, pl->GetPos()[2] - GetPos()[2], 0.0f};
				target = grobda_dir::Nearest16(d);
			}
			const int cur = m_nTurretDir;
			bool near = false;
			for (int k = 0; k < 8; k++)
				if (target == ((cur + k) & 15)) near = true;
			if (cur == target)
				m_nTurretState = 1;
			else
			{
				if (near) { m_nTurretTarget = cur == 15 ? 0 : cur + 1; m_nTurretTurn = 0; }
				else { m_nTurretTarget = cur == 0 ? 15 : cur - 1; m_nTurretTurn = 1; }
				m_nTurretState = 3;
			}
			TimerReset(2);
			TimerStart(2);
			break;
		}
		case 3:
		{
			const uint t = TimerMs(2);
			const uint total = m_Par[m_nLevel].turretTime;
			if (t < total)
				grobda_dir::Turn16(m_nTurretTurn, m_nTurretDir, t, total, m_vTurret);
			else
			{
				m_nTurretDir = m_nTurretTarget;
				std::memcpy(m_vTurret, grobda_dir::Dir16(m_nTurretDir), 16);
				m_nTurretState = 1;
			}
			break;
		}
		}
	}
	// the cannon's yaw relative to the body (the model's expand slot)
	float a1 = 0.0f, a2 = 0.0f;
	if (grobda_dir::IsNonZero(m_vDir)) a1 = grobda_dir::AngleOf(m_vDir);
	if (grobda_dir::IsNonZero(m_vTurret)) a2 = grobda_dir::AngleOf(m_vTurret);
	if (cKingdomModel* m = pModel(0))
		if (cKingdomModel* c = m->pExpandMdl(0)) c->vRotate().y = a2 - a1;
}

// DOL 0x8002d830: the first idle shot leaves one unit ahead of the cannon (y + 0.8) while fewer than the level's shot count are flying
void cTaskGrobdaEnemyTank::Shoot()
{
	uint active = 0;
	for (cTaskGrobdaShotEnemy* s : m_pShot)
		if (s != nullptr && s->nState() != 0) active++;
	if (active >= m_Par[m_nLevel].shotCount) return;
	for (int i = 0; i < 8; i++)
	{
		if (m_pShot[i] == nullptr || m_pShot[i]->nState() != 0 || !grobda_dir::IsNonZero(m_vTurret)) continue;
		const float a = grobda_dir::AngleOf(m_vTurret);
		const float p[3] = {GetPos()[0] + std::sin(a), GetPos()[1] + 0.8f, GetPos()[2] + std::cos(a)};   // FLOAT_80479fb8 = 0.8
		m_pShot[i]->Fire(m_Par[m_nLevel].shotSpeed, p, a);
		return;
	}
}

// DOL 0x8002cdc0: the actions (+0xb74): 0 pick the way (turn toward the nearest player), 1 drive, 3 turn away after a block, 4 drive on; 2 nothing
void cTaskGrobdaEnemyTank::Ai()
{
	float* pos = GetPos();
	std::memcpy(GetOldPos(), pos, 16);
	std::memset(m_vMove, 0, 16);
	m_fReach = 0.0f;   // SetCharSpace(0)
	const sTankParam& P = m_Par[m_nLevel];
	auto beginTurn = [&](int turnDir)
	{
		if (turnDir == 0) { m_nDirTarget = m_nDir == 7 ? 0 : m_nDir + 1; Anim(2); }
		else { m_nDirTarget = m_nDir == 0 ? 7 : m_nDir - 1; Anim(3); }
	};
	if (m_nMode == 2) return;
	if (m_nMode == 0)
	{
		if (TakeModeChanged())
		{
			TimerReset(0);
			TimerStart(0);
			int target = m_nDir;
			if (cTaskGrobdaModel* pl = NearestPlayer(pos))
			{
				float d[4] = {pl->GetPos()[0] - pos[0], 0.0f, pl->GetPos()[2] - pos[2], 0.0f};
				target = grobda_dir::Nearest8(d);
			}
			const int cur = m_nDir;
			bool near = false;
			for (int k = 0; k < 4; k++)
				if (target == ((cur + k) & 7)) near = true;
			if (cur == target) { m_nDirTarget = cur; m_nTurn = 2; }
			else if (near) { m_nTurn = 0; beginTurn(0); }
			else { m_nTurn = 1; beginTurn(1); }
		}
		const uint t = TimerMs(0);
		if (t < P.turnTime && m_nTurn != 2)
		{
			const float a = grobda_dir::Turn8(m_nTurn, m_nDir, t, P.turnTime, m_vDir);
			SetRotY(a);
		}
		else
		{
			m_nDir = m_nDirTarget;
			std::memcpy(m_vDir, grobda_dir::Dir8(m_nDir), 16);
			if (grobda_dir::IsNonZero(m_vDir)) SetRotY(grobda_dir::AngleOf(m_vDir));
			SetMode(1);
		}
		Cannon();
	}
	else if (m_nMode == 1)
	{
		if (TakeModeChanged())
		{
			TimerReset(0);
			TimerStart(0);
			TimerReset(1);
			TimerStart(1);
			Anim(1);
		}
		const float speed = m_bTouchPickup ? P.speedDebris : P.speed;
		m_fReach = speed;
		for (int i = 0; i < 3; i++) m_vMove[i] = m_vDir[i] * speed;
		for (int i = 0; i < 3; i++) pos[i] += m_vMove[i];
		const uint t0 = TimerMs(0);
		Cannon();
		const uint t1 = TimerMs(1);
		if (P.shotInterval <= t1)
		{
			Shoot();
			TimerReset(1);
			TimerStart(1);
		}
		if (t0 < P.moveTime)
		{
			if (P.moveMin <= t0 && m_bTouchMover) SetMode(3);
		}
		else
			SetMode(0);
	}
	else if (m_nMode == 3)
	{
		if (TakeModeChanged())
		{
			TimerReset(0);
			TimerStart(0);
			m_nTurn = (std::rand() & 1) != 0 ? 1 : 0;
			beginTurn(m_nTurn);
		}
		const uint t = TimerMs(0);
		if (t < P.turnTime)
		{
			const float a = grobda_dir::Turn8(m_nTurn, m_nDir, t, P.turnTime, m_vDir);
			SetRotY(a);
		}
		else
		{
			m_nDir = m_nDirTarget;
			std::memcpy(m_vDir, grobda_dir::Dir8(m_nDir), 16);
			if (grobda_dir::IsNonZero(m_vDir)) SetRotY(grobda_dir::AngleOf(m_vDir));
			SetMode(4);
		}
		Cannon();
	}
	else if (m_nMode == 4)
	{
		if (TakeModeChanged())
		{
			TimerReset(0);
			TimerStart(0);
			TimerReset(1);
			TimerStart(1);
			Anim(1);
		}
		Cannon();
		const uint t1 = TimerMs(1);
		if (P.shotInterval <= t1)
		{
			Shoot();
			TimerReset(1);
			TimerStart(1);
		}
		const uint t0 = TimerMs(0);
		if (t0 < P.wallWait)
		{
			const float speed = m_bTouchPickup ? P.speedDebris : P.speed;
			m_fReach = speed;
			for (int i = 0; i < 3; i++) m_vMove[i] = m_vDir[i] * speed;
			for (int i = 0; i < 3; i++) pos[i] += m_vMove[i];
		}
		else if (!m_bTouchMover)
			SetMode(0);
		else
			SetMode(3);
	}
}

// DOL 0x8002da88: the drop-in; on entering, face the nearest player (8 directions, and the turret's 16 where there is one)
void cTaskGrobdaEnemyTank::Phase1()
{
	if (TakeEnter(false))
	{
		int idx = m_nDir;
		if (cTaskGrobdaModel* pl = NearestPlayer(GetPos()))
		{
			float d[4] = {pl->GetPos()[0] - GetPos()[0], 0.0f, pl->GetPos()[2] - GetPos()[2], 0.0f};
			idx = grobda_dir::Nearest8(d);
		}
		std::memcpy(m_vDir, grobda_dir::Dir8(idx), 16);
		m_nDir = idx;
		if (grobda_dir::IsNonZero(m_vDir)) SetRotY(grobda_dir::AngleOf(m_vDir));
		if (m_bTurret)
		{
			m_nTurretDir = idx << 1;
			std::memcpy(m_vTurret, grobda_dir::Dir16(m_nTurretDir), 16);
			float a1 = 0.0f, a2 = 0.0f;
			if (grobda_dir::IsNonZero(m_vDir)) a1 = grobda_dir::AngleOf(m_vDir);
			if (grobda_dir::IsNonZero(m_vTurret)) a2 = grobda_dir::AngleOf(m_vTurret);
			if (cKingdomModel* m = pModel(0))
				if (cKingdomModel* c = m->pExpandMdl(0)) c->vRotate().y = a2 - a1;
		}
	}
	cTaskGrobdaEnemy::Phase1();
}

// DOL 0x8002dbd8 (Orange / Yellow 0x8002e5d4 / 0x8002f278 first follow the shield effect for 120 ms)
void cTaskGrobdaEnemyTank::Phase2()
{
	if (m_bTurret && m_pShield != nullptr && m_pShield->IsValid() && m_pShield->IsVisible())
	{
		if (TimerMs(3) < 0x79)
		{
			const float* p = GetPos();
			const float* r = GetRot();
			m_pShield->vSetTrans(Vector4(p[0], p[1], p[2], 1.0f));
			m_pShield->vSetRotate(Vector4(r[0], r[1], r[2], 1.0f));
		}
		else
			m_pShield->Invisible();
	}
	TakeEnter(false);
	cTaskGrobdaEnemy::Phase2();
	Ai();
	UpdateShape();
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && !st->IsFinishing() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(1, this, nullptr);
}

// DOL 0x8002dc50 (Orange / Yellow 0x8002e68c: the shield effect goes first)
void cTaskGrobdaEnemyTank::Phase3()
{
	if (m_bTurret && TakeEnter(false) && m_pShield != nullptr && m_pShield->IsValid())
	{
		m_pShield->Invisible();
		m_pShield->Inactive();
	}
	if (TakeEnter(false))
	{
		cTaskGrobdaEffect::Spawn(0x18, GetPos());   // the blast effect
		snd::Play("m11se_explosion");               // DAT_80479fa8
		Anim(5);
	}
	cTaskGrobdaEnemy::Phase3();
	if (cKingdomModel* m = pModel(0))
		if (m->GetFrameMaxAnmChr(0) <= m->GetFrameAnmChr(0)) SetPhase(4);
}

// DOL 0x8002dd74: the wreck is the second model; it stands in list 4 (it slows the tanks driving over it)
void cTaskGrobdaEnemyTank::Phase4()
{
	if (TakeEnter(false))
	{
		SwitchModel(1);
		UpdateShape();
	}
	cTaskGrobdaEnemy::Phase4();
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && !st->IsFinishing() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(4, this, nullptr);
}

// DOL 0x8002ddf8: the wreck fades (BIN_CLA_DEBRIS_REMOVE_ENEMY_TANK), then it is gone
void cTaskGrobdaEnemyTank::Phase5()
{
	if (TakeEnter(false))
	{
		if (cKingdomModel* m = pModel(1))
			if (m_sAnim[0][1] != "REMOVE")
			{
				m_sAnim[0][1] = "REMOVE";
				m->RegistAnm(0u, "BIN_CLA_DEBRIS_REMOVE_ENEMY_TANK");
			}
	}
	cTaskGrobdaEnemy::Phase5();
	if (cKingdomModel* m = pModel(1))
		if (m->GetFrameMaxMatClr(0) <= m->GetFrameMatClr(0))
		{
			if (cKingdomModel* a = pModel(0)) a->Invisible();
			m->Invisible();
			SetPhase(7);
		}
}

// DOL 0x8002c7d0; the colours' own Execute (Orange 0x8002e464, Purple 0x8002eb34, Yellow 0x8002f108) are the same wrapper around it: the
// KingSys +0xa40 / +0x1b4 test only gates get_pad1(0) (0x800219bc, an address computation whose result is dropped).
// Orange / Yellow's Term: cTaskGrobdaEnemyTank::ReleaseTerm.
bool cTaskGrobdaEnemyTank::Execute(cJetTask* pParam)
{
	return cTaskGrobdaEnemy::Execute(pParam);
}

// DOL 0x8002e404 / 0x8002f0a8 (Orange / Yellow Term): detach the shield +0xc8c (FUN_8008a508(.., 1)), destroy it, clear it; then the
// enemy's Term (the DOL dtor chain)
void cTaskGrobdaEnemyTank::ReleaseTerm()
{
	if (m_pShield != nullptr)
	{
		m_pShield->Detach(true);
		delete m_pShield;
		m_pShield = nullptr;
	}
	cTaskGrobdaEnemy::ReleaseTerm();
}

// DOL 0x8002e4c4 / 0x8002f168 (Orange / Yellow): a hit that does not kill shows the shield for 120 ms
void cTaskGrobdaEnemyTank::Resolve(int a, int b)
{
	const int before = Hp();
	cTaskGrobdaEnemy::Resolve(a, b);
	if (m_bTurret && a == 0 && Hp() >= 1 && Hp() < before)
	{
		if (m_pShield != nullptr && m_pShield->IsValid() && !m_pShield->IsVisible())
		{
			m_pShield->Active();
			m_pShield->Visible();
			const float* p = GetPos();
			const float* r = GetRot();
			m_pShield->vSetTrans(Vector4(p[0], p[1], p[2], 1.0f));
			m_pShield->vSetRotate(Vector4(r[0], r[1], r[2], 1.0f));
		}
		TimerReset(3);
		TimerStart(3);
	}
}

}
