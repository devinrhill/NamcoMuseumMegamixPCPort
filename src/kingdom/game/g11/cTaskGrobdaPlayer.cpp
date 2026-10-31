#include "kingdom/game/g11/cTaskGrobdaPlayer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g11/cTaskGrobdaCombo.hpp"
#include "kingdom/game/g11/cTaskGrobdaConsole.hpp"
#include "kingdom/game/g11/cTaskGrobdaEffect.hpp"
#include "kingdom/game/g11/cTaskGrobdaShot.hpp"
#include "kingdom/game/g11/cTaskGrobdaStage.hpp"

namespace kingdom::game::g11
{

namespace
{
constexpr float DEG = 0.017453292f;   // FLOAT_8047ca10

void Normalize(float v[3])
{
	const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (l > 0.0f) { v[0] /= l; v[1] /= l; v[2] /= l; }
}

// rotation of v around +Y by a (radians), the FUN_80298d00 'Y' matrix
void RotY(float v[3], float a)
{
	const float c = std::cos(a), s = std::sin(a);
	const float x = v[0] * c + v[2] * s, z = -v[0] * s + v[2] * c;
	v[0] = x;
	v[2] = z;
}

// signed angle around +Y that turns `from` onto `to` (FUN_80026c04 quaternion -> FUN_80026d88 euler y; both on the XZ plane here)
float AngleY(const float from[3], const float to[3])
{
	return std::atan2(from[2] * to[0] - from[0] * to[2], from[0] * to[0] + from[2] * to[2]);
}
}

// DOL 0x80194d0c [audited] + Init 0x80194e74
cTaskGrobdaPlayer::cTaskGrobdaPlayer(int player)
	: m_nPlayer(player)
{
	SetName("cTaskGrobdaPlayer");
	nDrawPriority(0x10002u);
	SetType(0x10000001u + (uint)player);   // GXInitTexObjUserData(this, type)
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("PlayerParam.csv"))
	{
		m_fSpeedMin = csv.F32(0, 1, 0.05f);
		m_fSpeedMax = csv.F32(1, 1, 0.25f);
		m_fSpeedDebris = csv.F32(2, 1, 0.03f);
		m_fAccele = csv.F32(3, 1, 0.005f);
		m_fGravity = csv.F32(4, 1, 0.05f);
		m_fShotSpeed = csv.F32(5, 1, 0.6f);
		m_nShotInterval = csv.S32(6, 1, 5);
		m_fCannonTurn = csv.F32(7, 1, 8.0f) * DEG;
		m_fBodyTurn = csv.F32(8, 1, 5.0f) * DEG;
		m_fEnMax = csv.F32(9, 1, 2000.0f);
		m_nEnRecvStop = csv.S32(10, 1, 5);
		m_nEnRecvMove = csv.S32(11, 1, 2);
		m_nEnShot = csv.S32(12, 1, 4);
		m_nEnShield = csv.S32(13, 1, 5);
		m_nEnDefense = csv.S32(14, 1, 4);
		m_nEnGrenade = csv.S32(15, 1, 2000);
		m_fShieldMiddle = csv.F32(16, 1, 0.5f);
		m_fShieldSmall = csv.F32(17, 1, 0.25f);
		m_fGreSwing = csv.F32(18, 1, 3.0f);
		m_fGreSpeed = csv.F32(19, 1, 0.5f);
		m_fGreAngle = csv.F32(20, 1, 75.0f) * DEG;
		m_fGreGravity = csv.F32(21, 1, 0.01f);
		m_nRecvWait = csv.S32(22, 1, 120);
		m_fRecvHeight = csv.F32(23, 1, 10.0f);
		m_nRecvInvincible = (int)csv.F32(24, 1, 120.0f);
		m_nStealLaser = csv.S32(25, 1, 1000);
		m_nStealBomb = csv.S32(26, 1, 3000);
	}
	m_fEnergy = m_fEnMax;       // piVar4[0x2f6] = piVar4[0x30e]
	m_fSpeed = m_fSpeedMin;     // piVar4[0x304] = piVar4[0x305]
	cTaskGrobdaShotGrenade::s_fGravity = m_fGreGravity;   // FLOAT_804772dc
	static const char* const BODY[4] = {"BIN_MDL_PLAYER_01", "BIN_MDL_PLAYER_02", "BIN_MDL_PLAYER_03", "BIN_MDL_PLAYER_04"};
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(BODY[player & 3], 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(1);
	m->IsDrawShadowDepth(true);   // 0x80195354: vtable +0x20 (1)
	// FUN_8001f798: the cannon (model slot 1 in the DOL) rides node "cannon" of the body
	m->RegistExpand(0, "cannon", "BIN_MDL_PLAYER_CANNON", 0x36f);
	if (cKingdomModel* c = m->pExpandMdl(0))
	{
		c->SetLightSetIdx(1);
		c->SetFogIdx(1);
		c->IsDrawShadowDepth(true);   // 0x8019536c
	}
	m_fRadius = 0.8f;   // +0x24 (FLOAT_80478370)
	{
		const float box[4][4] = {{1.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 1.0f, 0.0f}};   // FUN_8019334c(2, 2)
		SetShape(box, 4, false);
	}
	for (int i = 0; i < 3; i++)   // the shield effects L / M / S (slots 4 i + player), hidden
	{
		m_pShield[i] = new kingdom::cKingdomPtkHnd();
		m_pShield[i]->Generate((uint)(4 * i + player), false, true, true, 0.0f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		if (m_pShield[i]->IsValid()) { m_pShield[i]->Inactive(); m_pShield[i]->Invisible(); }
	}
	if (cG11GrobdaSys::Instance()->IsMulti())   // +0xb4c: the player marker of a multiplayer match (slot 0x21 + player), follows the tank
	{
		m_pMarker = new kingdom::cKingdomPtkHnd();
		m_pMarker->Generate((uint)(cTaskGrobdaEffect::eP1P + player), false, true, false, 0.0f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	for (int i = 0; i < 16; i++)   // 16 x cTaskGrobdaShotPlayer (0xb40) as children of the model task
	{
		m_pShot[i] = new cTaskGrobdaShotPlayer(player);
		ConnectChild(m_pShot[i]);
	}
	m_pGrenade = new cTaskGrobdaShotGrenade(0x3000001u + (uint)player);
	ConnectChild(m_pGrenade);
	SetAnim(0);
}

cTaskGrobdaPlayer::~cTaskGrobdaPlayer()
{
	for (kingdom::cKingdomPtkHnd* h : m_pShield) delete h;
	delete m_pMarker;
}
// DOL 0x8003822c [audited] (the placement) + 0x80199818 (the heading): the start of the map's record; the cannon and the body face (0, 0, 1) turned by `angle` around Y
void cTaskGrobdaPlayer::SetStart(float x, float z, float angle)
{
	float* pos = GetPos();
	pos[0] = x;
	pos[1] = 0.0f;
	pos[2] = z;
	pos[3] = 0.0f;
	std::memcpy(GetOldPos(), pos, 16);
	m_vSpawn[0] = x;
	m_vSpawn[1] = 0.0f;
	m_vSpawn[2] = z;
	m_vSpawn[3] = 0.0f;
	const float dir[4] = {0.0f, 0.0f, 1.0f, 0.0f};   // 0x8037c070
	std::memcpy(m_vAim, dir, 16);
	std::memcpy(m_vBody, dir, 16);
	if (angle != 0.0f)
	{
		RotY(m_vAim, angle);
		RotY(m_vBody, angle);
	}
	Normalize(m_vAim);
	Normalize(m_vBody);
	if (cKingdomModel* m = pModel(0)) m->vRotate() = Vector4(0.0f, angle, 0.0f, 0.0f);
}

// the clip pair of the body (c) and the pilot (p): "m11m_%c01_a%02d_cha.brres" (PTR 0x80449210 ..)
void cTaskGrobdaPlayer::SetAnim(int anim)
{
	static const int NO[7] = {1, 2, 3, 4, 6, 7, 8};   // anim id -> clip number
	if (anim < 0 || anim > 6) return;
	m_nAnim = anim;
	char name[0x200];
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	std::snprintf(name, sizeof(name), "m11m_c01_a%02d_cha.brres", NO[anim]);
	m->RegistAnm(0u, name);
	if (cKingdomModel* c = m->pExpandMdl(0))
	{
		std::snprintf(name, sizeof(name), "m11m_p01_a%02d_cha.brres", NO[anim]);
		c->RegistAnm(0u, name);
	}
	m_fFrom = 0.0f;
	m_fTo = m->GetFrameMaxAnmChr(0);
}

// the clip ran out: its frame is past the end noted when it started, or before the frame noted at the start of this Execute (it wrapped)
bool cTaskGrobdaPlayer::AnimDone()
{
	cKingdomModel* m = pModel(0);
	if (m == nullptr || m->AnmChrOf(0u) == nullptr) return true;
	const float cur = m->GetFrameAnmChr(0);
	return m_fTo < cur || cur < m_fFrom;
}

// DOL 0x801970d0 [audited]: the first free shot leaves the cannon tip (pos + rotY(aim) * (0, 0, 1), y + 0.8 = FLOAT_8047ca28) along the aim; needs EN_SHOT of energy
void cTaskGrobdaPlayer::Fire()
{
	if (m_fEnergy < (float)m_nEnShot || m_nShotWait > 0) return;
	for (cTaskGrobdaShotPlayer* s : m_pShot)
	{
		if (s == nullptr || s->IsActive()) continue;
		const float aim = std::atan2(m_vAim[0], m_vAim[2]);
		const float* p = GetPos();
		const float pos[3] = {p[0] + std::sin(aim), p[1] + 0.8f, p[2] + std::cos(aim)};
		s->Fire(m_fShotSpeed, pos, aim);
		m_nShotWait = m_nShotInterval;   // +0xbd4 = SHOT_INTERVAL
		if (!HasFlag(4)) SetAnim(2);     // standing: the a03 clip (restarted by every shot)
		m_bFiring = true;
		return;
	}
}

// DOL 0x80196124 [audited] (+ the input 0x80196054)
void cTaskGrobdaPlayer::Steer()
{
	float* pos = GetPos();
	std::memcpy(GetOldPos(), pos, 16);
	m_vMove[0] = m_vMove[2] = m_vMove[3] = 0.0f;
	float in[3] = {0.0f, 0.0f, 0.0f};
	const int padNo = cG11GrobdaSys::Instance()->nPad(m_nPlayer);   // the matching block's player +0x4c
	if (WiiPad::NunchukConnected(padNo))
	{
		const WiiPad::Stick s = WiiPad::NunchukStick(padNo);
		in[0] = s.x;
		in[2] = -s.y;
	}
	else
	{
		const uint hold = GamePad::Get(padNo).hold;
		if (hold & GamePad::PAD_RIGHT) in[2] = -1.0f;
		else if (hold & GamePad::PAD_LEFT) in[2] = 1.0f;
		if (hold & GamePad::PAD_DOWN) in[0] = 1.0f;
		else if (hold & GamePad::PAD_UP) in[0] = -1.0f;
	}
	float speed = 0.0f;
	if (in[0] * in[0] + in[2] * in[2] < 0.0625f)   // FLOAT_8047ca18: no input
	{
		m_fSpeed = m_fSpeedMin;
		ClearFlag(4);
		if (m_nAnim != 2 && m_nAnim != 0) SetAnim(0);
	}
	else
	{
		Normalize(in);
		// (the stage camera's Y rotation, +0x34, is 0 here)
		const float dot = m_vAim[0] * in[0] + m_vAim[1] * in[1] + m_vAim[2] * in[2];
		if (dot > 0.98f)   // FLOAT_8047ca1c
		{
			m_fSpeed += m_fAccele;
			if (m_fSpeed > m_fSpeedMax) m_fSpeed = m_fSpeedMax;
			speed = m_fSpeed;
		}
		else if (dot > 0.0f)
		{
			speed = m_fSpeed = m_fSpeedMin;
		}
		if (speed > 0.0f && m_bOnDebris)
		{
			m_fSpeed = m_fSpeedMin;
			speed = m_fSpeedDebris;
		}
		if (dot <= -0.99f) RotY(in, 1.0f * DEG);   // FLOAT_8047ca20: a straight turn-around picks a side
		float a = AngleY(m_vAim, in);
		const float ct = m_fCannonTurn;
		if (a > ct) a = ct;
		if (a < -ct) a = -ct;
		if (a != 0.0f) RotY(m_vAim, a);
		Normalize(m_vAim);
		float b = AngleY(m_vBody, m_vAim);
		const float bt = m_fBodyTurn;
		if (b > bt) b = bt;
		if (b < -bt) b = -bt;
		if (b != 0.0f) RotY(m_vBody, b);
		Normalize(m_vBody);
		m_vMove[0] = m_vBody[0] * speed;
		m_vMove[2] = m_vBody[2] * speed;
		AddFlag(4);
		// the clip follows the body's turn: straight a02, one way a07, the other a08
		if (b == 0.0f)
			{ if (m_nAnim != 1) SetAnim(1); }
		else if (b > 0.0f)
			{ if (m_nAnim != 5) SetAnim(5); }
		else if (m_nAnim != 6)
			SetAnim(6);
	}
	m_fReach = speed;   // SetCharSpace(speed)
	m_vMove[1] -= m_fGravity;
	for (int i = 0; i < 3; i++) pos[i] += m_vMove[i];
	if (pos[1] < 0.0f)
	{
		pos[1] = 0.0f;
		m_vMove[1] = 0.0f;
	}
	if (cKingdomModel* m = pModel(0))
	{
		const float body = std::atan2(m_vBody[0], m_vBody[2]);   // 0x80300950 * FLOAT_8047ca24
		const float aim = std::atan2(m_vAim[0], m_vAim[2]);
		m->vRotate() = Vector4(0.0f, body, 0.0f, 0.0f);
		if (cKingdomModel* c = m->pExpandMdl(0)) c->vRotate() = Vector4(0.0f, aim - body, 0.0f, 0.0f);
	}
}

// DOL 0x80197508 [audited]: the swing throws the grenade when the energy is full enough and the last one is gone
void cTaskGrobdaPlayer::Grenade()
{
	if (m_fEnergy < (float)m_nEnGrenade) return;
	if (m_pGrenade == nullptr || m_pGrenade->nState() != 0) return;
	const float yaw = std::atan2(m_vAim[0], m_vAim[2]);
	const float* p = GetPos();
	const float pos[3] = {p[0], p[1] + 0.5f, p[2]};   // DOUBLE_8047ca38
	const float rot[3] = {-m_fGreAngle, yaw, 0.0f};
	m_pGrenade->Throw(m_fGreSpeed, pos, rot);
	m_fSpeed = m_fSpeedMin;
	m_fEnergy -= (float)m_nEnGrenade;
	m_vMove[0] = 0.0f;
	m_vMove[2] = 0.0f;
	if (m_nAnim != 2) SetAnim(2);
}

// DOL 0x801987c8 [audited]
void cTaskGrobdaPlayer::Control()
{
	const int padNo = cG11GrobdaSys::Instance()->nPad(m_nPlayer);   // the matching block's player +0x4c
	const GamePad::State& pad = GamePad::Get(padNo);
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && st->IsFinishing())   // FUN_8009ffb4
	{
		m_nState = 4;
		m_bEnter = true;
		return;
	}
	if (HasFlag(8) && --m_nTimer < 0)
	{
		ClearFlag(8);
		ClearFlag(0x10);
		for (uint i = 0; i < 2; i++)
			if (cKingdomModel* m = pModel(i)) m->Visible();
	}
	if (m_nShotWait > 0) m_nShotWait--;
	const bool nunchuk = WiiPad::NunchukConnected(padNo);
	bool fire = nunchuk ? (pad.hold & (GamePad::PAD_Z | GamePad::PAD_B)) != 0 : (pad.hold & GamePad::PAD_1) != 0;
	if (fire) Fire();
	const float acc = std::sqrt(pad.acc[0] * pad.acc[0] + pad.acc[1] * pad.acc[1] + pad.acc[2] * pad.acc[2]);
	// DEVIATION: the emulated Shake (acc 2.5 g) stays under GRE_SWING (3 g): its accSpeed of 2 counts as a swing too
	if (acc > m_fGreSwing || pad.accSpeed >= 2.0f) Grenade();
	const bool shield = nunchuk ? (pad.hold & GamePad::PAD_A) != 0 : (pad.hold & GamePad::PAD_2) != 0;
	if (shield)
	{
		AddFlag(2);
		if (!HasFlag(4) && m_nAnim != 3) SetAnim(3);
	}
	else
	{
		ClearFlag(2);
		if (!HasFlag(4) && m_nAnim != 0) SetAnim(0);
	}
	Steer();
	UpdateShape();
	if (st != nullptr && !st->IsEnded() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(0, this, nullptr);
	if (AnimDone() && m_nAnim == 2) SetAnim(0);   // the grenade throw ended
}

// DOL 0x80197894 [audited]: the energy: it recovers each frame (EN_RECV_STOP, EN_RECV_MOVE while moving), the shield and the laser cost, the bar shows it, the alarm sounds at the small level
void cTaskGrobdaPlayer::Energy()
{
	m_fEnergy += (float)(HasFlag(4) ? m_nEnRecvMove : m_nEnRecvStop);
	auto level = [&](float e)
	{
		const float fr = m_fEnMax > 0.0f ? e / m_fEnMax : 0.0f;
		if (m_fShieldSmall < fr) return fr <= m_fShieldMiddle ? 1 : 0;
		return 2;
	};
	if (HasFlag(2) && level(m_fEnergy) != 2) m_fEnergy -= (float)m_nEnShield;
	if (m_nShotWait > 0) m_fEnergy -= (float)m_nEnShot;
	if (m_fEnergy < 0.0f) m_fEnergy = 0.0f;
	else if (m_fEnergy > m_fEnMax) m_fEnergy = m_fEnMax;
	if (level(m_fEnergy) == 2)
	{
		if (m_AlarmSnd.voice < 0) WiiSnd::Play(m_AlarmSnd, "m11se_alarm");   // DAT_8047ca00
	}
	else if (m_AlarmSnd.voice >= 0)
		WiiSnd::Stop(m_AlarmSnd);
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && st->Console() != nullptr) st->Console()->SetGauge(m_nPlayer, m_fEnMax > 0.0f ? m_fEnergy / m_fEnMax : 0.0f);   // 0x80130ce4
}

// DOL 0x80197ae4 [audited]: while the shield is held the effect of its level (L above half the energy, M above a quarter, S below) follows the tank; at S it breaks
void cTaskGrobdaPlayer::ShieldEffects()
{
	if (!HasFlag(2))
	{
		for (kingdom::cKingdomPtkHnd* h : m_pShield)
			if (h != nullptr && h->IsValid()) { h->Inactive(); h->Invisible(); }
		if (m_ShieldSnd.voice >= 0) WiiSnd::Stop(m_ShieldSnd);
		return;
	}
	const float* p = GetPos();
	const Vector4 at(p[0], p[1] + 0.5f, p[2], 1.0f);   // DOUBLE_8047ca38
	const float fr = m_fEnMax > 0.0f ? m_fEnergy / m_fEnMax : 0.0f;
	const int lv = fr > m_fShieldSmall ? (fr > m_fShieldMiddle ? 0 : 1) : 2;
	for (int i = 0; i < 3; i++)
	{
		kingdom::cKingdomPtkHnd* h = m_pShield[i];
		if (h == nullptr) continue;
		if (i == lv)
		{
			if (h->IsValid() && !h->IsActive()) { h->Active(); h->Visible(); }
		}
		else if (h->IsValid())
		{
			h->Inactive();
			h->Invisible();
		}
		h->vSetTrans(at);
	}
	if (lv == 2)
	{
		ClearFlag(2);
		if (m_ShieldSnd.voice >= 0) WiiSnd::Stop(m_ShieldSnd);
	}
	else if (m_ShieldSnd.voice < 0)
		WiiSnd::Play(m_ShieldSnd, "m11se_shield");   // DAT_8047c9f8
}

// DOL 0x80198454 [audited]: wrecks under the tank (kind 4) slow it; shots, blasts and crushes (5, 6, 3, 8) hurt
void cTaskGrobdaPlayer::Resolve(int a, int b)
{
	(void)b;
	if (a != 0) return;
	std::vector<const sGrobdaHit*> hits;
	for (const sGrobdaHit& h : HitList())
	{
		if (h.kind == 4) m_bOnDebris = true;
		else if (h.kind == 5 || h.kind == 6 || h.kind == 3 || h.kind == 8) hits.push_back(&h);
	}
	if (!hits.empty()) Damage(hits);
}

// DOL 0x80197d30 [audited]: the shield absorbs (L: everything, M: shots but not blasts, S / none: nothing); a crush (kind 8) always kills; the nearest killer's owner may steal
// score in multiplayer; the tank's own blast joins the combo
void cTaskGrobdaPlayer::Damage(std::vector<const sGrobdaHit*>& hits)
{
	if (HasFlag(8)) return;   // recovering: invulnerable (the DOL's first test, flag 8)
	if (Shielding())
		for (size_t i = 0; i < hits.size(); i++) m_fEnergy -= (float)m_nEnDefense;   // EN_DEFENSE per absorbed hit
	int lv = 2;
	if (Shielding())
	{
		const float fr = m_fEnMax > 0.0f ? m_fEnergy / m_fEnMax : 0.0f;
		lv = fr > m_fShieldSmall ? (fr <= m_fShieldMiddle ? 1 : 0) : 2;
	}
	const bool large = Shielding() && lv == 0;
	const bool medium = Shielding() && lv == 1;
	std::vector<const sGrobdaHit*> fatal;
	for (const sGrobdaHit* h : hits)
		if (h->kind == 8 || (!large && (!medium || static_cast<cGrobdaObject*>(h->pObj)->IsKind(0x4000000u)))) fatal.push_back(h);
	if (fatal.empty()) return;
	const sGrobdaHit* nearest = nullptr;
	for (const sGrobdaHit* h : fatal)
		if (nearest == nullptr || h->dist < nearest->dist) nearest = h;
	int killer = 0;
	uint ctype = 0x4000001u;
	const uint t = static_cast<cGrobdaObject*>(nearest->pObj)->nType();
	const bool multi = cG11GrobdaSys::Instance()->IsMulti();
	if (t >= 0x1000001u && t <= 0x1000004u) killer = (int)(t - 0x1000001u), ctype = 0x4000001u + (uint)killer;
	else if (t >= 0x4000001u && t <= 0x4000004u) killer = (int)(t - 0x4000001u), ctype = 0x4000001u + (uint)killer;
	else if (t >= 0x4000006u && t <= 0x4000009u) killer = (int)(t - 0x4000006u), ctype = 0x4000001u + (uint)killer;
	else { killer = 0; ctype = 0x4000005u; }
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (multi && ctype != 0x4000005u && killer != m_nPlayer)
	{
		int pts = 0;
		if (t >= 0x4000001u && t <= 0x4000004u) pts = m_nStealBomb;
		else if ((t >= 0x1000001u && t <= 0x1000004u) || (t >= 0x4000006u && t <= 0x4000009u)) pts = m_nStealLaser;
		if (st != nullptr) { st->AddScore(killer, pts); st->AddScore(m_nPlayer, -pts); }
	}
	else if (multi && st != nullptr)
		st->AddScore(m_nPlayer, -500);   // 0xfffffe0c
	if (st != nullptr)
	{
		cTaskGrobdaCombo* combo = static_cast<cTaskGrobdaCombo*>(nearest->tag);
		if (combo == nullptr) combo = st->AddCombo(ctype);
		combo->Hit(GetPos(), 0, true);
	}
	m_nState = 2;
	m_bEnter = true;
}

// DOL 0x80199094 [audited]: the respawn: back at the start, RECV_HEIGHT up, recovering (blinking, invulnerable) with a full energy bar; it falls and lands in state 1
void cTaskGrobdaPlayer::Respawn()
{
	if (m_bEnter)
	{
		m_bEnter = false;
		for (uint i = 0; i < 2; i++)
			if (cKingdomModel* m = pModel(i)) m->Visible();
		std::memcpy(GetPos(), m_vSpawn, 16);
		std::memset(m_vMove, 0, 16);
		GetPos()[1] = m_fRecvHeight;
		if (m_nAnim != 0) SetAnim(0);
		SetFlags(0);
		AddFlag(8);
		AddFlag(0x10);
		m_fEnergy = m_fEnMax;
		m_nTimer = m_nRecvInvincible;
		m_fSpeed = m_fSpeedMin;
	}
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr && st->IsFinishing())
	{
		m_nState = 4;
		m_bEnter = true;
		return;
	}
	Steer();
	UpdateShape();
	if (GetPos()[1] < 1.0f)
	{
		m_nState = 1;
		m_bEnter = true;
		if (st != nullptr && !st->IsFinishing() && cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(7, this, nullptr);   // the landing crushes
	}
}

// DOL 0x80199478 [audited]: the stage is over: on entering the tank shows itself and idles; then it settles on the ground
void cTaskGrobdaPlayer::Settle()
{
	if (!m_bEnter)
	{
		m_vMove[0] = m_vMove[2] = 0.0f;
		m_vMove[1] -= m_fGravity;
		float* pos = GetPos();
		for (int i = 0; i < 3; i++) pos[i] += m_vMove[i];
		if (pos[1] < 0.0f) { pos[1] = 0.0f; m_vMove[1] = 0.0f; }
		return;
	}
	m_bEnter = false;
	for (uint i = 0; i < 2; i++)
		if (cKingdomModel* m = pModel(i)) m->Visible();
	if (m_nAnim != 0) SetAnim(0);
	SetFlags(0);
	m_nShotWait = 0;
}

// DOL 0x80195a0c [audited]
bool cTaskGrobdaPlayer::Execute(cJetTask* pParam)
{
	if (!cTaskGrobdaCollision::Running())
	{
		WiiSnd::Stop(m_AlarmSnd);    // FUN_80026318(+0xbac, 5): the loops stop while paused
		WiiSnd::Stop(m_ShieldSnd);   // FUN_80026318(+0xba0, 5)
		return cTaskGrobdaModel::Execute(pParam);
	}
	m_bFiring = false;   // +0xbbe = 0
	if (cKingdomModel* m = pModel(0)) m_fFrom = m->GetFrameAnmChr(0);
	UpdateFrame();   // FUN_8009a478
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	switch (m_nState)
	{
	case 0:
		UpdateShape();
		if (st != nullptr && st->IsPlaying()) { m_nState = 1; m_bEnter = true; }
		break;
	case 1:
		Control();
		break;
	case 2:
		if (m_bEnter)
		{
			m_bEnter = false;
			snd::Play("m11se_explosion");   // DAT_8047c9fc
			if (m_nAnim != 4) SetAnim(4);
			m_nTimer = m_nRecvWait;
			m_nShotWait = 0;
			SetFlags(1);
			AddFlag(0x10);
			cTaskGrobdaEffect::Spawn(0x1f, GetPos());   // the fragments
		}
		else if (AnimDone() && --m_nTimer < 1)
		{
			m_nState = 3;
			m_bEnter = true;
		}
		break;
	case 3:
		Respawn();
		break;
	case 4:
		Settle();
		break;
	}
	if (!HasFlag(0x10))
		m_nBlink = 0;
	else if (--m_nBlink < 0)
	{
		m_nBlink = 3;   // the models blink every fourth frame
		for (uint i = 0; i < 2; i++)
			if (cKingdomModel* m = pModel(i))
			{
				if (m->IsVisible()) m->Invisible();
				else m->Visible();
			}
	}
	Energy();
	ShieldEffects();
	if (m_pMarker != nullptr)
	{
		if (m_pMarker->IsValid() && !m_pMarker->IsActive()) { m_pMarker->Active(); m_pMarker->Visible(); }
		const float* p = GetPos();
		m_pMarker->vSetTrans(Vector4(p[0], p[1], p[2], 1.0f));
	}
	m_bOnDebris = false;
	return cTaskGrobdaModel::Execute(pParam);
}

}
