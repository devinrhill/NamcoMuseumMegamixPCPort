#include "kingdom/game/g11/cTaskGrobdaEnemyBig.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "WiiSnd.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g11/cTaskGrobdaCombo.hpp"
#include "kingdom/game/g11/cTaskGrobdaEffect.hpp"
#include "kingdom/game/g11/cTaskGrobdaPlayer.hpp"
#include "kingdom/game/g11/cTaskGrobdaShot.hpp"
#include "kingdom/game/g11/cTaskGrobdaStage.hpp"

namespace kingdom::game::g11
{

namespace
{
constexpr float DEG = 0.017453292f;
}

// ---- Fortres ---------------------------------------------------------------------------------------------------------------------------

// DOL 0x80028ec4 (type 0x20000006) + Init 0x80028fa0
cTaskGrobdaEnemyFortres::cTaskGrobdaEnemyFortres()
	: cTaskGrobdaEnemy(0x20000006u)
{
	SetName("cTaskGrobdaEnemyFortres");
	cKingdomModel* m = CreateModel(0);
	m->RegistModel("BIN_MDL_ENEMY_06", 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(1);
	m->RegistExpand(0, "cannon", "BIN_MDL_ENEMY_06_CANNON", 0x36f);
	if (cKingdomModel* c = m->pExpandMdl(0)) { c->SetLightSetIdx(1); c->SetFogIdx(1); }
	m->Visible();
	cKingdomModel* d = CreateModel(1);
	d->RegistModel("BIN_MDL_DEBRIS_ENEMY_06", 0x36f);
	d->Invisible();
	Anim(0);
	m_nModelIdx = 0;
	Vector4 off;
	m->GetLocalNodeTrans(off, "cannon");   // FUN_8001dd54(model, +0xc10, "cannon")
	m_vOffset[0] = off.x; m_vOffset[1] = off.y; m_vOffset[2] = off.z; m_vOffset[3] = 0.0f;
	for (int i = 0; i < 16; i++)
	{
		m_pRing[i] = new cTaskGrobdaShotEnemy(0x2000004u);
		m_pRing[i]->m_fRadius = 0.3f;    // FLOAT_804769d4
		ConnectChild(m_pRing[i]);
	}
	for (int i = 0; i < 8; i++)
	{
		m_pAimed[i] = new cTaskGrobdaShotEnemy(0x2000007u);
		m_pAimed[i]->m_fRadius = 0.25f;  // FLOAT_804769d8
		ConnectChild(m_pAimed[i]);
	}
	m_fRadius = 2.0f;   // FLOAT_804769d0
	{
		const float box[4][4] = {{2.0f, 0.0f, 2.0f, 0.0f}, {2.0f, 0.0f, -2.0f, 0.0f}, {-2.0f, 0.0f, -2.0f, 0.0f}, {-2.0f, 0.0f, 2.0f, 0.0f}};   // FUN_8019334c(4, 4)
		SetShape(box, 4, false);
	}
	SetMode(0);
	m_nBurst = 0;
	m_Csv.RegistData("EnmFortresParam.csv");
	Hp() = m_Csv.S32(7, 1, 0xff);
	BaseScore() = m_Csv.S32(8, 1, 0xff);
	ComboStep() = m_Csv.S32(9, 1, 0xff);
	for (int l = 0; l < 3; l++)
	{
		const int col = l + 1;
		m_Par[l].speed = m_Csv.F32(0, col, 0.0f);
		m_Par[l].shotSpeed = m_Csv.F32(1, col, 0.0f);
		m_Par[l].wait = (uint)m_Csv.F32(2, col, 0.0f);
		m_Par[l].move = (uint)m_Csv.F32(3, col, 0.0f);
		m_Par[l].moveMin = (uint)m_Csv.F32(4, col, 0.0f);
		m_Par[l].ring = (uint)m_Csv.F32(5, col, 0.0f);
		m_Par[l].aimed = (uint)m_Csv.F32(6, col, 0.0f);
	}
}

// DOL 0x8002947c: the angle (steps of 22 degrees up to 88) turns the body; the cannon offset turns with it
void cTaskGrobdaEnemyFortres::SetStart(int level, int delay, int angleDeg)
{
	int deg = angleDeg;
	if (deg < 0) deg += 360;
	else if (deg >= 360) deg -= 360;
	int idx = 0;
	for (int k = 1; k <= 4; k++)
		if ((uint)deg < (uint)(k * 0x16)) { idx = k - 1; break; }
	const float yaw = (float)(idx * 0x16) * DEG;
	if (cKingdomModel* m = pModel(0)) m->vRotate().y = yaw;
	if (yaw != 0.0f)
	{
		const float c = std::cos(yaw), s = std::sin(yaw);
		const float x = m_vOffset[0] * c + m_vOffset[2] * s, z = -m_vOffset[0] * s + m_vOffset[2] * c;
		m_vOffset[0] = x;
		m_vOffset[2] = z;
	}
	SetLevel(level, delay);
	Hp() = m_Csv.S32(7, m_nLevel + 1, 0xff);
	BaseScore() = m_Csv.S32(8, m_nLevel + 1, 0xff);
	ComboStep() = m_Csv.S32(9, m_nLevel + 1, 0xff);
}

// DOL 0x80029700
bool cTaskGrobdaEnemyFortres::HasShots()
{
	for (cTaskGrobdaShotEnemy* s : m_pRing) if (s != nullptr && s->nState() != 0) return true;
	for (cTaskGrobdaShotEnemy* s : m_pAimed) if (s != nullptr && s->nState() != 0) return true;
	return false;
}

// DOL 0x80029d38
void cTaskGrobdaEnemyFortres::Anim(int mode)
{
	m_nAnim = mode;
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	static const char* const NAME[7] = {"BIN_CHA_ENEMY_FORTRES_WAIT", "BIN_CHA_ENEMY_FORTRES_RUN", "BIN_CHA_ENEMY_FORTRES_LEVITATION", "BIN_CHA_ENEMY_FORTRES_LAND",
	                                    "BIN_CLA_ENEMY_FORTRES_HIT", "BIN_CLA_ENEMY_FORTRES_HIT_DAMAGE", "BIN_CHA_ENEMY_FORTRES_DIE"};
	if (mode < 0 || mode > 6) return;
	const int kind = (mode == 4 || mode == 5) ? 1 : 0;
	if (m_sAnim[kind] == NAME[mode]) return;
	m_sAnim[kind] = NAME[mode];
	m->RegistAnm(0u, NAME[mode]);
}

// DOL 0x80029f64: four ring shots at 90 degrees from the cannon's yaw, half a unit out
void cTaskGrobdaEnemyFortres::FireRing()
{
	cKingdomModel* m = pModel(0);
	const float cannonY = (m != nullptr && m->pExpandMdl(0) != nullptr) ? m->pExpandMdl(0)->vRotate().y : 0.0f;
	const float bodyY = m != nullptr ? m->vRotate().y : 0.0f;
	int k = 0;
	for (int i = 0; i < 16 && k < 4; i++)
	{
		if (m_pRing[i] == nullptr || m_pRing[i]->nState() != 0) continue;
		const float yaw = 1.5707964f * (float)k + (cannonY + bodyY);
		const float* p = GetPos();
		float pos[3] = {p[0] + m_vOffset[0], p[1] + m_vOffset[1] + 0.8f, p[2] + m_vOffset[2]};
		pos[0] += 0.5f * std::sin(yaw);
		pos[2] += 0.5f * std::cos(yaw);
		m_pRing[i]->Fire(m_Par[m_nLevel].shotSpeed, pos, yaw);
		k++;
	}
}

// DOL 0x80029798
void cTaskGrobdaEnemyFortres::Ai()
{
	float* pos = GetPos();
	std::memcpy(GetOldPos(), pos, 16);
	std::memset(m_vMove, 0, 16);
	m_fReach = 0.0f;
	const sPar& P = m_Par[m_nLevel];
	if (m_nMode == 2)
	{
		if (TakeModeChanged())
		{
			Anim(1);
			std::memcpy(m_vDir, grobda_dir::Dir8(std::rand() & 7), 16);
			TimerReset(0);
			TimerStart(0);
		}
		const uint t = TimerMs(0);
		if (t < P.move)
		{
			if (t < P.moveMin || !m_bTouchMover)
			{
				m_fReach = P.speed;
				for (int i = 0; i < 3; i++) m_vMove[i] = m_vDir[i] * P.speed;
				for (int i = 0; i < 3; i++) pos[i] += m_vMove[i];
			}
			else
				SetMode(3);
		}
		else
			SetMode(3);
	}
	else if (m_nMode == 0)
	{
		if (TakeModeChanged())
		{
			Anim(0);
			m_nBurst = 0;
			TimerReset(0); TimerStart(0);
			TimerReset(1); TimerStart(1);
			TimerReset(3); TimerStart(3);
			TimerReset(2); TimerStart(2);
		}
		const uint t1 = TimerMs(1);
		if (m_nBurst == 1)
		{
			if (TimerMs(2) >= 300 && P.ring <= t1) m_nBurst = 0;
		}
		else if (P.ring <= t1)
		{
			FireRing();
			m_nBurst = 1;
			TimerReset(1); TimerStart(1);
			TimerReset(2); TimerStart(2);
		}
		if (P.aimed <= TimerMs(3))
		{
			for (int i = 0; i < 8; i++)
				if (m_pAimed[i] != nullptr && m_pAimed[i]->nState() == 0)
				{
					const float p[3] = {pos[0], pos[1] + 0.8f, pos[2]};
					const float rot[3] = {0.0f, 0.0f, 0.0f};
					m_pAimed[i]->FireAimed(P.shotSpeed, p, rot);
					break;
				}
			TimerReset(3);
			TimerStart(3);
		}
		if (P.wait <= TimerMs(0))
		{
			m_nBurst = 0;
			SetMode(1);
		}
	}
	else if (m_nMode == 1)
	{
		if (TakeModeChanged()) Anim(2);
		if (cKingdomModel* m = pModel(0))
			if (m->GetFrameMaxAnmChr(0) <= m->GetFrameAnmChr(0)) SetMode(2);
	}
	else if (m_nMode == 3)
	{
		if (TakeModeChanged()) Anim(3);
		if (cKingdomModel* m = pModel(0))
			if (m->GetFrameMaxAnmChr(0) <= m->GetFrameAnmChr(0)) SetMode(0);
	}
	if (m_nBurst != 1)   // the cannon turns on (3 degrees a frame)
		if (cKingdomModel* m = pModel(0))
			if (cKingdomModel* c = m->pExpandMdl(0))
			{
				c->vRotate().y += 0.05235988f;
				if (c->vRotate().y > 6.2831855f) c->vRotate().y -= 6.2831855f;
			}
}

// DOL 0x8002a1e8
void cTaskGrobdaEnemyFortres::Phase2()
{
	TakeEnter(false);
	cTaskGrobdaEnemy::Phase2();
	Ai();
	UpdateShape();
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && !st->IsEnded() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(1, this, nullptr);
}

// DOL 0x8002a260
void cTaskGrobdaEnemyFortres::Phase3()
{
	if (TakeEnter(false))
	{
		cTaskGrobdaEffect::Spawn(0x18, GetPos());
		snd::Play("m11se_explosion");   // DAT_80479f50
		Anim(6);
	}
	cTaskGrobdaEnemy::Phase3();
	if (cKingdomModel* m = pModel(0))
		if (m->GetFrameMaxAnmChr(0) <= m->GetFrameAnmChr(0)) SetPhase(4);
}

// DOL 0x8002a3f4
void cTaskGrobdaEnemyFortres::Phase4()
{
	if (TakeEnter(false))
	{
		SwitchModel(1);
		UpdateShape();
	}
	cTaskGrobdaEnemy::Phase4();
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && !st->IsEnded() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(4, this, nullptr);
}

// DOL 0x8002a478
void cTaskGrobdaEnemyFortres::Phase5()
{
	if (TakeEnter(false))
		if (cKingdomModel* m = pModel(1))
			if (m_sAnim[1] != "REMOVE")
			{
				m_sAnim[1] = "REMOVE";
				m->RegistAnm(0u, "BIN_CLA_DEBRIS_REMOVE_FORTRES");
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

// DOL 0x8002a570: a hit that does not kill flashes the hit-damage clip for 300 ms
void cTaskGrobdaEnemyFortres::Resolve(int a, int b)
{
	const int before = Hp();
	cTaskGrobdaEnemy::Resolve(a, b);
	if (a == 0 && Hp() < before)
	{
		Anim(5);
		TimerReset(4);
		TimerStart(4);
	}
}

// DOL 0x8002936c: the hit flash ends after 300 ms
bool cTaskGrobdaEnemyFortres::Execute(cJetTask* pParam)
{
	if (cTaskGrobdaCollision::Running() && TimerRunning(4) && TimerMs(4) >= 300)
	{
		TimerStop(4);
		if (cKingdomModel* m = pModel(0)) m->ReleaseAnmClr(0);   // FUN_8001af88
		m_sAnim[1].clear();
	}
	return cTaskGrobdaEnemy::Execute(pParam);
}

// ---- Frossor -----------------------------------------------------------------------------------------------------------------------------

// DOL 0x8002bd30 (Brown, type 0x20000005, shots 0x2000006) / 0x8002c1b0 (Green, type 0x20000004, shots 0x2000005) + Init 0x8002be0c / 0x8002c28c + 0x8002a708
cTaskGrobdaEnemyFrossor::cTaskGrobdaEnemyFrossor(uint type, bool brown)
	: cTaskGrobdaEnemy(type)
{
	SetName("cTaskGrobdaEnemyFrossor");
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(brown ? "BIN_MDL_ENEMY_05" : "BIN_MDL_ENEMY_04", 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(1);
	m->Visible();
	cKingdomModel* d = CreateModel(1);
	d->RegistModel(brown ? "BIN_MDL_DEBRIS_ENEMY_05" : "BIN_MDL_DEBRIS_ENEMY_04", 0x36f);
	d->Invisible();
	Anim(0);
	m_nModelIdx = 0;
	for (int i = 0; i < 8; i++)
	{
		m_pShot[i] = new cTaskGrobdaShotEnemy(brown ? 0x2000006u : 0x2000005u);
		m_pShot[i]->m_fRadius = 0.2f;   // FLOAT_804769f0
		ConnectChild(m_pShot[i]);
	}
	m_fRadius = 0.8f;
	{
		const float box[4][4] = {{1.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 1.0f, 0.0f}};
		SetShape(box, 4, false);
	}
	SetMode(0);
	std::memcpy(m_vDir, grobda_dir::Dir16(0), 16);
	std::memcpy(m_vMoveDir, grobda_dir::Dir16(0), 16);
	m_Csv.RegistData(brown ? "EnmFrossorBrownParam.csv" : "EnmFrossorGreenParam.csv");
	Hp() = m_Csv.S32(13, 1, 0xff);
	BaseScore() = m_Csv.S32(14, 1, 0xff);
	ComboStep() = m_Csv.S32(15, 1, 0xff);
	for (int l = 0; l < 3; l++)
	{
		const int c = l + 1;
		sPar& p = m_Par[l];
		p.speed = m_Csv.F32(0, c, 0.0f);
		p.speedDebris = m_Csv.F32(1, c, 0.0f);
		p.shotSpeed = m_Csv.F32(2, c, 0.0f);
		p.shotCount = (uint)m_Csv.F32(3, c, 0.0f);
		p.move = (uint)m_Csv.F32(4, c, 0.0f);
		p.moveMin = (uint)m_Csv.F32(5, c, 0.0f);
		p.turn = (uint)m_Csv.F32(6, c, 0.0f);
		p.wallWait = (uint)m_Csv.F32(7, c, 0.0f);
		p.interval = (uint)m_Csv.F32(8, c, 0.0f);
		p.intervalSeen = (uint)m_Csv.F32(9, c, 0.0f);
		p.dodgeSpeed = m_Csv.F32(10, c, 0.0f);
		p.dodge = (uint)m_Csv.F32(11, c, 0.0f);
		p.dodgeMin = (uint)m_Csv.F32(12, c, 0.0f);
	}
}

// DOL 0x8002a840: the angle in 22 degree steps picks the facing (16 directions)
void cTaskGrobdaEnemyFrossor::SetStart(int level, int delay, int angleDeg)
{
	int deg = angleDeg;
	if (deg < 0) deg += 360;
	else if (deg >= 360) deg -= 360;
	int idx = 0;
	for (int k = 1; k <= 16; k++)
		if ((uint)deg < (uint)(k * 0x16)) { idx = k - 1; break; }
	m_nDirIdx = idx;
	std::memcpy(m_vDir, grobda_dir::Dir16(idx), 16);
	if (grobda_dir::IsNonZero(m_vDir))
		if (cKingdomModel* m = pModel(0)) m->vRotate().y = grobda_dir::AngleOf(m_vDir);
	SetLevel(level, delay);
	Hp() = m_Csv.S32(13, m_nLevel + 1, 0xff);
	BaseScore() = m_Csv.S32(14, m_nLevel + 1, 0xff);
	ComboStep() = m_Csv.S32(15, m_nLevel + 1, 0xff);
}

// DOL 0x8002ab08
bool cTaskGrobdaEnemyFrossor::HasShots()
{
	for (cTaskGrobdaShotEnemy* s : m_pShot) if (s != nullptr && s->nState() != 0) return true;
	return false;
}

// DOL 0x8002ab68
void cTaskGrobdaEnemyFrossor::Anim(int mode)
{
	m_nAnim = mode;
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	static const char* const NAME[4] = {"BIN_CHA_ENEMY_FROSSOR_WAIT", "BIN_CHA_ENEMY_FROSSOR_RUN", "BIN_CHA_ENEMY_FROSSOR_SHOT", "BIN_CHA_ENEMY_FROSSOR_DIE"};
	if (mode < 0 || mode > 3 || m_sAnim == NAME[mode]) return;
	m_sAnim = NAME[mode];
	m->RegistAnm(0u, NAME[mode]);
}

// DOL 0x8002b578: a player that has just fired aims along a line; when it passes within 2 units (distance^2 <= FLOAT_80479f8c = 4) in front of the player, step away from the line
// (the direction nearest to the offset from the line's point; turned a step at a time while a wall is in the way)
bool cTaskGrobdaEnemyFrossor::Danger()
{
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st == nullptr) return false;
	for (int i = 0; i < st->nPlayers(); i++)
	{
		cTaskGrobdaPlayer* p = static_cast<cTaskGrobdaPlayer*>(st->Player(i));
		if (p == nullptr || !p->IsFiring()) continue;
		const float* aim = p->Aim();
		if (!(aim[0] * aim[0] + aim[1] * aim[1] + aim[2] * aim[2] >= 0x1p-125f)) continue;
		const float a[3] = {aim[0], 0.0f, aim[2]};
		const float* pp = p->GetPos();
		const float* me = GetPos();
		const float rel[3] = {me[0] - pp[0], 0.0f, me[2] - pp[2]};
		if (!(rel[0] * a[0] + rel[2] * a[2] > 0.0f)) continue;
		const float p1[3] = {pp[0], pp[1], pp[2]};
		const float p2[3] = {pp[0] + a[0], pp[1], pp[2] + a[2]};
		// FUN_801946a8: the foot of the perpendicular from `me` on the line, and its squared distance
		float u[3] = {p2[0] - p1[0], 0.0f, p2[2] - p1[2]}, w[3] = {me[0] - p1[0], 0.0f, me[2] - p1[2]};
		const float uu = u[0] * u[0] + u[2] * u[2];
		if (!(uu >= 1.17549435e-38f)) continue;
		const float t = (u[0] * w[0] + u[2] * w[2]) / uu;
		if (!(t >= 1.17549435e-38f)) continue;
		const float foot[3] = {p1[0] + u[0] * t, 0.0f, p1[2] + u[2] * t};
		const float d2 = (foot[0] - me[0]) * (foot[0] - me[0]) + (foot[2] - me[2]) * (foot[2] - me[2]);
		if (d2 > 4.0f) continue;
		const float off[3] = {me[0] - foot[0], 0.0f, me[2] - foot[2]};
		int idx = grobda_dir::Nearest16(off);
		for (int k = 0; k < 4; k++)
		{
			const float* d = grobda_dir::Dir16(idx);
			const float v[3] = {d[0] * m_Par[m_nLevel].dodgeSpeed, d[1] * m_Par[m_nLevel].dodgeSpeed, d[2] * m_Par[m_nLevel].dodgeSpeed};
			if (cTaskGrobdaCollision::Instance() == nullptr || !cTaskGrobdaCollision::Instance()->SweepAny(this, v, 2)) { m_nDodgeIdx = idx; return true; }
			idx = idx == 0 ? 15 : idx - 1;
		}
		m_nDodgeIdx = idx == 0 ? 15 : idx - 1;
		return true;
	}
	return false;
}

// DOL 0x8002b824: one homing shot along the facing while fewer than the shot count are in the air
void cTaskGrobdaEnemyFrossor::Shoot()
{
	uint active = 0;
	for (cTaskGrobdaShotEnemy* s : m_pShot) if (s != nullptr && s->nState() != 0) active++;
	if (active >= m_Par[m_nLevel].shotCount) return;
	for (int i = 0; i < 8; i++)
	{
		if (m_pShot[i] == nullptr || m_pShot[i]->nState() != 0 || !grobda_dir::IsNonZero(m_vDir)) continue;
		const float yaw = grobda_dir::AngleOf(m_vDir);
		const float* p = GetPos();
		const float pos[3] = {p[0], p[1] + 0.8f, p[2]};   // FLOAT_80479f90
		const float rot[3] = {0.0f, yaw, 0.0f};
		m_pShot[i]->FireAimed(m_Par[m_nLevel].shotSpeed, pos, rot);
		return;
	}
}

// DOL 0x8002acc8: actions 0 turn in place by random steps, 1 wait, 2 shoot, 3 wander, 4 dodge
void cTaskGrobdaEnemyFrossor::Ai()
{
	float* pos = GetPos();
	std::memcpy(GetOldPos(), pos, 16);
	std::memset(m_vMove, 0, 16);
	m_fReach = 0.0f;
	const sPar& P = m_Par[m_nLevel];
	auto setFacing = [&]() { if (grobda_dir::IsNonZero(m_vDir)) if (cKingdomModel* m = pModel(0)) m->vRotate().y = grobda_dir::AngleOf(m_vDir); };
	if (m_nMode == 2)
	{
		if (TakeModeChanged()) { Anim(2); Shoot(); }
		if (!Danger())
		{
			if (cKingdomModel* m = pModel(0))
				if (m->GetFrameMaxAnmChr(0) <= m->GetFrameAnmChr(0)) SetMode(3);
		}
		else
			SetMode(4);
	}
	else if (m_nMode == 0)
	{
		if (TakeModeChanged())
		{
			Anim(0);
			m_nTurnSteps = std::rand() & 7;
			if (m_nTurnSteps != 0)
			{
				m_nTurnDir = std::rand() & 1;
				m_nDirTarget = m_nTurnDir == 0 ? (m_nDirIdx == 15 ? 0 : m_nDirIdx + 1) : (m_nDirIdx == 0 ? 15 : m_nDirIdx - 1);
			}
			TimerReset(0);
			TimerStart(0);
		}
		if (!Danger())
		{
			if (m_nTurnSteps == 0)
				SetMode(1);
			else
			{
				const uint t = TimerMs(0);
				if (t < P.turn)
					grobda_dir::Turn16(m_nTurnDir, m_nDirIdx, t, P.turn, m_vDir);
				else
				{
					m_nDirIdx = m_nDirTarget;
					std::memcpy(m_vDir, grobda_dir::Dir16(m_nDirTarget), 16);
					if (--m_nTurnSteps != 0)
					{
						m_nDirTarget = m_nTurnDir == 0 ? (m_nDirIdx == 15 ? 0 : m_nDirIdx + 1) : (m_nDirIdx == 0 ? 15 : m_nDirIdx - 1);
						TimerReset(0);
						TimerStart(0);
					}
				}
				setFacing();
			}
		}
		else
			SetMode(4);
	}
	else if (m_nMode == 1)
	{
		if (TakeModeChanged()) { TimerReset(0); TimerStart(0); }
		const uint t = TimerMs(0);
		if (!Danger())
		{
			if (P.interval <= t) SetMode(2);
		}
		else
			SetMode(4);
	}
	else if (m_nMode == 4)
	{
		if (TakeModeChanged())
		{
			Anim(1);
			std::memcpy(m_vMoveDir, grobda_dir::Dir16(m_nDodgeIdx), 16);
			TimerReset(0); TimerStart(0);
			TimerReset(4); TimerStart(4);
		}
		const uint t0 = TimerMs(0), t4 = TimerMs(4);
		if (P.dodgeMin <= t4 || m_bTouchMover)
		{
			int idx = m_nDodgeIdx;
			bool found = false;
			for (int k = 0; k < 4 && !found; k++)
			{
				const float* d = grobda_dir::Dir16(idx);
				const float v[3] = {d[0] * P.dodgeSpeed, d[1] * P.dodgeSpeed, d[2] * P.dodgeSpeed};
				if (cTaskGrobdaCollision::Instance() == nullptr || !cTaskGrobdaCollision::Instance()->SweepAny(this, v, 2)) found = true;
				else idx = idx == 0 ? 15 : idx - 1;
			}
			if (!found) idx = idx == 0 ? 15 : idx - 1;
			m_nDodgeIdx = idx;
			std::memcpy(m_vMoveDir, grobda_dir::Dir16(idx), 16);
			TimerReset(4);
			TimerStart(4);
		}
		if (t0 < P.dodge)
		{
			m_fReach = P.dodgeSpeed;
			for (int i = 0; i < 3; i++) m_vMove[i] = m_vMoveDir[i] * P.dodgeSpeed;
			for (int i = 0; i < 3; i++) pos[i] += m_vMove[i];
		}
		else
			SetMode(0);
	}
	else if (m_nMode == 3)
	{
		if (TakeModeChanged())
		{
			Anim(1);
			std::memcpy(m_vMoveDir, grobda_dir::Dir16(std::rand() & 15), 16);
			TimerReset(0);
			TimerStart(0);
		}
		const uint t = TimerMs(0);
		if (!Danger())
		{
			if (t < P.move)
			{
				if (t < P.moveMin && !m_bTouchMover)
				{
					m_fReach = P.speed;
					for (int i = 0; i < 3; i++) m_vMove[i] = m_vMoveDir[i] * P.speed;
					for (int i = 0; i < 3; i++) pos[i] += m_vMove[i];
				}
				else
					SetMode(0);
			}
			else
				SetMode(0);
		}
		else
			SetMode(4);
	}
}

// DOL 0x8002b9a4
void cTaskGrobdaEnemyFrossor::Phase2()
{
	TakeEnter(false);
	cTaskGrobdaEnemy::Phase2();
	Ai();
	UpdateShape();
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && !st->IsEnded() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(1, this, nullptr);
}

// DOL 0x8002ba1c
void cTaskGrobdaEnemyFrossor::Phase3()
{
	if (TakeEnter(false))
	{
		cTaskGrobdaEffect::Spawn(0x18, GetPos());
		snd::Play("m11se_explosion");   // DAT_80479f80
		Anim(3);
	}
	cTaskGrobdaEnemy::Phase3();
	if (cKingdomModel* m = pModel(0))
		if (m->GetFrameMaxAnmChr(0) <= m->GetFrameAnmChr(0)) SetPhase(4);
}

// DOL 0x8002bb8c
void cTaskGrobdaEnemyFrossor::Phase4()
{
	if (TakeEnter(false))
	{
		SwitchModel(1);
		UpdateShape();
	}
	cTaskGrobdaEnemy::Phase4();
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && !st->IsEnded() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(4, this, nullptr);
}

// DOL 0x8002bc10
void cTaskGrobdaEnemyFrossor::Phase5()
{
	if (TakeEnter(false))
		if (cKingdomModel* m = pModel(1))
			m->RegistAnm(0u, "BIN_CLA_DEBRIS_REMOVE_FROSSOR");
	cTaskGrobdaEnemy::Phase5();
	if (cKingdomModel* m = pModel(1))
		if (m->GetFrameMaxMatClr(0) <= m->GetFrameMatClr(0))
		{
			if (cKingdomModel* a = pModel(0)) a->Invisible();
			m->Invisible();
			SetPhase(7);
		}
}

}
