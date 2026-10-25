#include "kingdom/game/g11/cTaskGrobdaShot.hpp"

#include <cmath>
#include <cstring>

#include "WiiSnd.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g11/cTaskGrobdaEffect.hpp"
#include "kingdom/game/g11/cTaskGrobdaCombo.hpp"
#include "kingdom/game/g11/cTaskGrobdaStage.hpp"

namespace kingdom::game::g11
{

// DOL 0x800aaa0c
cTaskGrobdaShot::cTaskGrobdaShot()
{
	m_fRadius = 0.2f;   // +0x24 (FLOAT_804772f0 / 804772c0)
	m_pHnd = new kingdom::cKingdomPtkHnd();
}

cTaskGrobdaShot::~cTaskGrobdaShot()
{
	delete m_pHnd;
}

// FUN_8009ff60(app, slot, pos, 0, handle, 0): the effect is generated once into the shot's handle and kept hidden until a shot flies
void cTaskGrobdaShot::GenerateHandle(uint slot)
{
	m_pHnd->Generate(slot, false, true, true, 0.0f, Vector4(0.0f, -100.0f, 0.0f, 1.0f));
}

// DOL 0x800ab424
void cTaskGrobdaShot::Stop()
{
	m_nState = 0;
	if (m_pHnd != nullptr && m_pHnd->IsValid())
	{
		m_pHnd->Inactive();    // FUN_8008a59c
		m_pHnd->Invisible();   // FUN_8008a5bc
	}
}

// DOL 0x800ab090: velocity = rotY(angle) * (0, 0, speed) = the move vector, the handle shown at the shot
void cTaskGrobdaShot::BaseFire(float speed, const float pos[3], float rotY)
{
	m_nMode = 0;
	m_nState = 1;
	for (int i = 0; i < 3; i++) m_vPos[i] = pos[i];
	m_vRot[1] = rotY;
	m_vMove[0] = std::sin(rotY) * speed;
	m_vMove[1] = 0.0f;
	m_vMove[2] = std::cos(rotY) * speed;
	m_vMove[3] = 0.0f;
	if (m_pHnd != nullptr && m_pHnd->IsValid())
	{
		m_pHnd->Active();    // FUN_8008a590
		m_pHnd->Visible();   // FUN_8008a5b0
		m_pHnd->vSetTrans(Vector4(pos[0], pos[1], pos[2], 1.0f));
		m_pHnd->vSetRotate(Vector4(0.0f, rotY, 0.0f, 1.0f));
	}
}

// DOL 0x800aac50: a flying shot remembers its position, and while x and z stay within -50 .. 50 (FLOAT_8047ae90 / 94) moves by its move vector (0x800aadfc,
// the homing mode 1 NOT PORTED) and registers in its list; else it ends
bool cTaskGrobdaShot::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (!cTaskGrobdaCollision::Running() || m_nState != 1) return true;
	std::memcpy(m_vOld, m_vPos, 16);   // old = pos
	if (m_vPos[0] >= -50.0f && m_vPos[0] <= 50.0f && m_vPos[2] >= -50.0f && m_vPos[2] <= 50.0f)
	{
		if (m_nMode == 1 && m_pTarget != nullptr)   // 0x800aadfc: homing, 1.5 degrees a frame towards the player
		{
			const float* tp = m_pTarget->GetPos();
			const float dx = tp[0] - m_vPos[0], dz = tp[2] - m_vPos[2];
			if ((std::fabs(dx) >= 1.17549435e-38f || std::fabs(dz) >= 1.17549435e-38f) && (std::fabs(m_vMove[0]) >= 1.17549435e-38f || std::fabs(m_vMove[2]) >= 1.17549435e-38f))
			{
				auto deg = [](float a, float b) { float d = std::atan2(a, b) * 57.2957795f; if (d < 0.0f) d += 360.0f; return d; };   // 0x80300950(z, x) * 1.40625
				float ang = deg(m_vMove[2], m_vMove[0]);
				const float delta = deg(dz, dx) - ang;
				if ((0.0f < delta && delta <= 180.0f) || (delta < 0.0f && delta <= -180.0f)) ang += 1.5f;
				else if ((0.0f < delta && 180.0f < delta) || (delta < 0.0f && -180.0f < delta)) ang -= 1.5f;
				if (ang >= 360.0f) ang -= 360.0f;
				if (ang < 0.0f) ang += 360.0f;
				const float r = ang * 0.017453292f;
				m_vMove[0] = m_fHomeSpeed * std::cos(r);
				m_vMove[1] = 0.0f;
				m_vMove[2] = m_fHomeSpeed * std::sin(r);
				m_vMove[3] = 0.0f;
			}
		}
		for (int i = 0; i < 3; i++) m_vPos[i] += m_vMove[i];   // 0x800aadfc
		if (m_pHnd != nullptr && m_pHnd->IsValid()) m_pHnd->vSetTrans(Vector4(m_vPos[0], m_vPos[1], m_vPos[2], 1.0f));
		if (Layer() >= 0 && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(Layer(), this, nullptr);
		return true;
	}
	Stop();
	return true;
}

// DOL 0x800ab4b4: the nearest record (smallest squared distance) of a kind < 3 (a mover, a wall) shows the hit effect (slot 0x17) at its point
void cTaskGrobdaShot::HitEffect()
{
	const sGrobdaHit* best = nullptr;
	for (const sGrobdaHit& h : HitList())
		if (h.kind < 3 && (best == nullptr || h.dist < best->dist)) best = &h;
	if (best != nullptr)
		cTaskGrobdaEffect::Spawn((uint)cTaskGrobdaEffect::eHIT, best->point);
}

// DOL 0x800ac738 / 0x800ab930
void cTaskGrobdaShot::Resolve(int a, int b)
{
	const size_t hits = HitList().size();
	if (a == 0) HitEffect();
	(void)b;
	if (hits != 0) Stop();
}

// DOL 0x800ac4a8: the effect is generated once (BIN_EFF_SHOT_PLAYER_0N) and kept hidden until a shot flies
cTaskGrobdaShotPlayer::cTaskGrobdaShotPlayer(int player)
	: m_nPlayer(player)
{
	SetName("cTaskGrobdaShotPlayer");
	SetType((uint)player);              // GXInitTexObjUserData(this, param_2): 0xf000000 + the player id
	GenerateHandle((uint)(cTaskGrobdaEffect::eSHOT_PLAYER + (player & 3)));
	Stop();
}

// DOL 0x800ac640 (the sound m11se_laser) + 0x800ab090
void cTaskGrobdaShotPlayer::Fire(float speed, const float pos[3], float rotY)
{
	snd::Play("m11se_laser");   // DAT_8047aed8
	BaseFire(speed, pos, rotY);
}

// DOL 0x800ab5b8
cTaskGrobdaShotEnemy::cTaskGrobdaShotEnemy(uint type)
{
	SetName("cTaskGrobdaShotEnemy");
	SetType(type);
	m_fRadius = 0.2f;   // FLOAT_804772c0
	m_nState = 0;
}

// DOL 0x800ab7cc + 0x800ab254 (+ the nearest player, 0x800aba50): the shot leaves along rot[1] and then homes in on the nearest player
bool cTaskGrobdaShotEnemy::FireAimed(float speed, const float pos[3], const float rot[3])
{
	uint slot = 0x10;
	if (nType() >= 0x2000001u && nType() <= 0x2000007u) slot = 0x10u + (nType() - 0x2000001u);
	GenerateHandle(slot);
	if (m_pHnd == nullptr || !m_pHnd->IsValid()) return false;
	snd::Play("m11se_beam");   // DAT_8047aeb8
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	cTaskGrobdaModel* best = nullptr;
	float bd = 0.0f;
	if (st != nullptr)
		for (int i = 0; i < st->nPlayers(); i++)
		{
			cTaskGrobdaModel* p = st->Player(i);
			if (p == nullptr) continue;
			const float dx = p->GetPos()[0] - pos[0], dz = p->GetPos()[2] - pos[2];
			const float d = dx * dx + dz * dz;
			if (best == nullptr || d < bd) { best = p; bd = d; }
		}
	if (best == nullptr) return false;
	m_pTarget = best;
	m_fHomeSpeed = speed;
	BaseFire(speed, pos, rot[1]);
	m_nMode = 1;
	return true;
}

// DOL 0x800ab6e0: the effect of the enemy kind (slot 0x10 + type - 0x2000001; 0x800ab9ac) is generated into the shot's handle, then the beam sound and the fire
bool cTaskGrobdaShotEnemy::Fire(float speed, const float pos[3], float rotY)
{
	uint slot = 0x10;
	if (nType() >= 0x2000001u && nType() <= 0x2000007u) slot = 0x10u + (nType() - 0x2000001u);
	GenerateHandle(slot);
	if (m_pHnd == nullptr || !m_pHnd->IsValid()) return false;
	snd::Play("m11se_beam");   // DAT_8047aeb8
	BaseFire(speed, pos, rotY);
	return true;
}

float cTaskGrobdaShotGrenade::s_fGravity = 0.01f;

// DOL 0x800abb8c + 0x800abc90: radius (FLOAT_804772d0 .. d8), the trail effect (slot 0x1a + player), hidden
cTaskGrobdaShotGrenade::cTaskGrobdaShotGrenade(uint type)
{
	SetName("cTaskGrobdaShotGrenade");
	SetType(type);
	m_fRadius = 0.3f;   // UNVERIFIED: FLOAT_804772d8 (the sphere is never swept: no list)
	GenerateHandle((uint)(cTaskGrobdaEffect::eGRENADE_LOCUS + (int)(type - 0x3000001u)));
	Stop();
	m_nState = 0;
}

// DOL 0x800ac050: sound, the shot's fire, then the velocity = (0, 0, 1) turned about X by rot[0] and about Y by rot[1], times the speed
void cTaskGrobdaShotGrenade::Throw(float speed, const float pos[3], const float rot[3])
{
	snd::Play("m11se_grenade");   // DAT_8047aec0
	BaseFire(speed, pos, rot[1]);
	float v[3] = {0.0f, 0.0f, 1.0f};
	if (rot[0] != 0.0f)   // MTXRotRad 'X'
	{
		const float c = std::cos(rot[0]), s = std::sin(rot[0]);
		const float y = v[1] * c - v[2] * s, z = v[1] * s + v[2] * c;
		v[1] = y;
		v[2] = z;
	}
	if (rot[1] != 0.0f)   // 'Y'
	{
		const float c = std::cos(rot[1]), s = std::sin(rot[1]);
		const float x = v[0] * c + v[2] * s, z = -v[0] * s + v[2] * c;
		v[0] = x;
		v[2] = z;
	}
	for (int i = 0; i < 3; i++) m_vMove[i] = v[i] * speed;
	m_vRot[0] = rot[0];
	m_bFlight = true;
}

// DOL 0x800ac320: a blast of the owner's combo (kind 2 = the grenade size, not counted), the sound, the ball ends
void cTaskGrobdaShotGrenade::Explode()
{
	uint ctype = 0x4000005u;
	const uint t = nType();
	if (t >= 0x3000001u && t <= 0x3000004u) ctype = 0x4000001u + (t - 0x3000001u);
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr)
	{
		cTaskGrobdaCombo* c = st->AddCombo(ctype);
		c->Hit(GetPos(), 2, false);
	}
	snd::Play("m11se_explosion");   // DAT_8047aec4
	m_bFlight = false;
	Stop();
}

// DOL 0x800abddc: gravity, the landing, the turn of the ball to its flight direction, then the shot's move
bool cTaskGrobdaShotGrenade::Execute(cJetTask* pParam)
{
	if (cTaskGrobdaCollision::Running() && m_nState == 1)
	{
		m_vMove[1] -= s_fGravity;
		if (m_vPos[1] + m_vMove[1] < 0.0f) Explode();
		const float l = std::sqrt(m_vMove[0] * m_vMove[0] + m_vMove[1] * m_vMove[1] + m_vMove[2] * m_vMove[2]);
		if (l > 0.0f)
		{
			const float dx = m_vMove[0] / l, dy = m_vMove[1] / l, dz = m_vMove[2] / l;
			m_vRot[0] = std::atan2(-dy, std::fabs(dx) + std::fabs(dz));
		}
	}
	return cTaskGrobdaShot::Execute(pParam);
}

}
