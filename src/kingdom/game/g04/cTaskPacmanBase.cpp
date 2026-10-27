#include "kingdom/game/g04/cTaskPacmanBase.hpp"

#include <cmath>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"
#include "kingdom/game/g04/cScoreMessageListener.hpp"
#include "kingdom/game/g04/cTaskBG.hpp"
#include "kingdom/game/g04/cTaskCursor.hpp"
#include "kingdom/game/g04/cTaskEnemy.hpp"
#include "kingdom/game/g04/cTaskEnemyManager.hpp"

namespace kingdom::game::g04
{

namespace
{
constexpr float DEG = 0.017453292f;     // FLOAT_8047cd08
constexpr float HALF_PI_N = -1.5707964f;   // FLOAT_8047cd24
bool IsSingle() { return KingdomGlobal::WaniWaniSys->nPlayers() == 1; }
}

// DOL 0x8019ca4c (the CSV block of cParamPacman; CSV_PACMAN = 0x7bf448e4)
void cParamPacman::Load()
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData("CSV_PACMAN")) return;
	fFailTime = csv.F32(0, 1, 0.0f);
	fReturnTime = csv.F32(1, 1, 0.0f);
	fMinusRange = csv.F32(3, 1, 0.0f);
	fPlusRange = csv.F32(2, 1, 0.0f);
	fComboInput = csv.F32(4, 1, 0.0f);
	fJumpHeight = csv.F32(5, 1, 0.0f);
	fBezierDiv = csv.F32(6, 1, 0.0f);
	for (int p = 0; p < 4; p++)
		vInitPos[p] = Vector4(csv.F32(p + 9, 1, 0.0f), csv.F32(p + 9, 2, 0.0f), csv.F32(p + 9, 3, 0.0f), 1.0f);
	fComboMoveFrame = csv.F32(7, 1, 0.0f);
	for (int i = 0; i < 4; i++)
		vFailOffset[i] = Vector4(csv.F32(i + 13, 1, 0.0f), csv.F32(i + 13, 2, 0.0f), csv.F32(i + 13, 3, 0.0f), 0.0f);
	fWaitShake = csv.F32(8, 1, 0.0f);
	fExPadRadius = csv.F32(0x2d, 1, 0.0f);
	fWinDistance = csv.F32(0x2e, 1, 0.0f);
	fLoseDistance = csv.F32(0x2f, 1, 0.0f);
	for (int i = 0; i < 4; i++)
	{
		vWinOffset[i] = Vector4(csv.F32(i + 0x30, 1, 0.0f), csv.F32(i + 0x30, 2, 0.0f), csv.F32(i + 0x30, 3, 0.0f), csv.F32(i + 0x30, 4, 0.0f));
		vLoseOffset[i] = Vector4(csv.F32(i + 0x34, 1, 0.0f), csv.F32(i + 0x34, 2, 0.0f), csv.F32(i + 0x34, 3, 0.0f), csv.F32(i + 0x34, 4, 0.0f));
	}
}

// DOL 0x801b2298
cTaskPacmanBase::cTaskPacmanBase(uint nPlayer, cTaskBG* pBG, cTaskPacmanManager* pMan) : m_pBG(pBG), m_pMan(pMan), m_nPlayer(nPlayer)
{
	SetName("g04::cTaskPacmanBase");
	Init();
}

cTaskPacmanBase::~cTaskPacmanBase() = default;

int cTaskPacmanBase::Pad() const
{
	// multiplayer: the pad of the matching data's player (+0x54 + player * 0x54); alone: pad 0
	return IsSingle() ? 0 : KingdomGlobal::WaniWaniSys->nWork((int)m_nPlayer);
}

float cTaskPacmanBase::Bob() const
{
	return std::sin(DEG * 5.0f * m_fBob) * m_Param.fWaitShake;   // FLOAT_8047cd0c = 5
}

void cTaskPacmanBase::ResetAcc(int pad)
{
	const GamePad::State& s = GamePad::Get(pad);
	for (auto& v : m_vAcc)
	{
		v[0] = s.acc[0];
		v[1] = s.acc[1];
		v[2] = s.acc[2];
		v[3] = 0.0f;
	}
	m_fSwingMax = 1.0f;   // FLOAT_8047cd00
}

// the step 0x801b2ee4 / 0x801b3458 / 0x801b3b34 share: the pitch (0x80300950 = atan2 in 256ths, * 1.40625 = degrees, 0..360) and the
// acceleration go into the 6-entry history
void cTaskPacmanBase::PushAcc(int pad)
{
	const GamePad::State& s = GamePad::Get(pad);
	float a = std::atan2(s.acc[1], s.acc[2]) * 57.29578f;
	if (a < 0.0f) a += 360.0f;   // FLOAT_8047cd14
	m_vAcc[m_nAcc][0] = s.acc[0];
	m_vAcc[m_nAcc][1] = s.acc[1];
	m_vAcc[m_nAcc][2] = s.acc[2];
	m_fPitch[m_nAcc] = a;
	m_nAcc = (m_nAcc + 1) % 6;
}

// DOL 0x801b23b8
void cTaskPacmanBase::Init()
{
	m_nChain = 0;
	m_nTarget = 0;
	m_fTimer = 0.0f;
	m_nComboReq = 0;
	m_nHits = 0;
	m_nDamage = 0;
	m_nCombo = 0;
	m_nRedHits = 0;
	m_nAcc = 0;
	ResetAcc(Pad());
	m_fSwingMax = 1.0f;
	m_nComboFlag = 0;
	m_bMouth = false;
	m_fBob = 0.0f;
	m_Param.Load();
	static const char* const MODEL[4] = {"BIN_MODEL_PAC01", "BIN_MODEL_PAC02", "BIN_MODEL_PAC03", "BIN_MODEL_PAC04"};   // 0x8037d438
	kingdom::cKingdomModel* m = CreateModel(0u);
	m->RegistModel(MODEL[m_nPlayer & 3u], 0x36f);
	m->vTrans() = m_Param.vInitPos[m_nPlayer & 3u];
	m_vHome = m->vTrans();
	m_nAimGator = 0;
	m_pCursor = new cTaskCursor(m_nPlayer);   // +0x700 (0x801a7dd8)
	ConnectChild(m_pCursor);
	if (IsSingle()) m_fBob = 0.0f;
	const uint n = KingdomGlobal::WaniWaniSys->nPlayers();
	if (n != 1 && m_nPlayer < n)   // 0x80021a9c: the light ring, looping, dependent, clipped, at Pac-Man
		m_RingFx.Generate(m_nPlayer + 0x12u, false, true, true, 0.0f, pModel(0u)->vTrans());
	for (bool& b : m_bTrail) b = false;   // +0x6a0..+0x6a7
	SetState(1);
}

// DOL 0x801b26f8 (a new game)
void cTaskPacmanBase::Restart()
{
	m_nTarget = 0;
	m_nDamage = 0;
	m_nHits = 0;
	m_nCombo = 0;
	m_nChain = 0;
	if (IsSingle()) m_fBob = 0.0f;
	m_nRedHits = 0;
	m_nComboFlag = 0;
	m_Anime.Attach(pModel(0u), "TEX_ANIME_PAC_01", 0u);
	m_nAcc = 0;
	ResetAcc(Pad());
	m_f244 = 0.0f;
	pModel(0u)->vTrans() = m_Param.vInitPos[m_nPlayer & 3u];
	m_vHome = pModel(0u)->vTrans();
	m_nAimGator = 0;
	SetState(1);
}

// DOL 0x801b50e0
int cTaskPacmanBase::Target(int pad) const
{
	const GamePad::State& s = GamePad::Get(pad);
	const float x = s.stick[0], y = s.stick[1];
	const float r = m_Param.fExPadRadius;
	if (!(r * r < x * x + y * y)) return 2;
	const float ang = (180.0f * std::atan(y / x)) / 3.1415927f;   // FLOAT_8047cd28 / FLOAT_8047cd4c
	int t = 2;
	if (x == 0.0f) t = y <= 0.0f ? 2 : 0;
	else if (x < -0.0f)
	{
		if (0.0f <= y) t = -45.0f < ang ? 1 : 0;
		if (y <= -0.0f) t = 45.0f < ang ? 2 : 1;
	}
	else
	{
		if (0.0f <= y) t = ang < 45.0f ? 3 : 0;
		if (y <= -0.0f) t = ang < -45.0f ? 2 : 3;
	}
	return t;
}

// DOL 0x801b5258: the SDK swing filter (kpaddemo swing.c) over the 5 newest samples; out = the swing direction. 1 = a swing.
bool cTaskPacmanBase::Swing(float minSpeed, float decay)
{
	float sum = 0.0f;
	float last[3]{}, accVec[3]{}, dirVec[2]{};
	float out[3]{};
	for (int i = 0; i < 5; i++)
	{
		int a = m_nAcc - i, b = m_nAcc - 1 - i;
		if (a < 0) a += 6;
		if (b < 0) b += 6;
		sum += m_fPitch[a] - m_fPitch[b];
		float s[3] = {m_vAcc[a][0], m_vAcc[a][1], m_vAcc[a][2]};
		for (int k = 0; k < 3; k++)
		{
			if (s[k] < -3.4f || 3.4f < s[k])   // FLOAT_8047cd5c / cd60 (ACC_MAX)
			{
				accVec[k] *= 0.6f;           // FLOAT_8047cd64 (ACC_DAMP)
				s[k] = accVec[k] + last[k];
			}
			else
				accVec[k] = s[k] - last[k];
			last[k] = s[k];
		}
		dirVec[0] *= decay;   // DIR_VEC_DAMP (0.9)
		dirVec[1] *= decay;
		float f = accVec[2] < 0.0f ? -accVec[2] : accVec[2];
		f = 1.0f - f * f;
		if (0.0f < f)
		{
			dirVec[0] += f * accVec[0];
			dirVec[1] += f * accVec[1];
		}
		m_fSwingMax *= decay;   // DIR_SPEED_DAMP
		const float speed = std::sqrt(dirVec[0] * dirVec[0] + dirVec[1] * dirVec[1]);
		if (minSpeed <= speed && m_fSwingMax < speed)
		{
			m_fSwingMax = speed;
			out[0] = dirVec[0] / speed;
			out[1] = dirVec[1] / speed;
			out[2] = 0.0f;
		}
	}
	// FLOAT_8047cd68 = -1, cd6c = -0.4, DOUBLE_8047cd70 = 0.4
	if (-m_Param.fMinusRange <= sum) return false;
	if ((out[1] < -1.0f || -0.4f <= out[1]) && (1.0f < out[1] || out[1] < 0.4f)) return false;
	return true;
}

// DOL 0x801b4328
void cTaskPacmanBase::SetState(int state)
{
	m_nOldState = m_nState;
	m_nState = state;
	m_fTimer = 0.0f;
	m_nComboReq = 0;
	kingdom::cKingdomModel* m = pModel(0u);
	switch (state)
	{
	case 0:
		m->Invisible();
		break;
	case 2:
		m_bMouth = false;
		m_Anime.Attach(m, "TEX_ANIME_PAC_01", 0u);
		m_nAcc = 0;
		ResetAcc(Pad());
		if (m_pCursor != nullptr) m_pCursor->Show();
		break;
	case 3:
	{
		WiiSnd::Handle h;
		WiiSnd::Play(h, "m04se_10tataki");   // DAT_8047ccf4 (0x80199ee8)
		WiiSnd::Handle hr;                   // 0x801b44e8: FUN_80025b6c(handles, the player's pad (sys +0x54 + player * 0x54 + 0x50), cue, 200 ms fade-in; natively no fade-in)
		WiiSnd::PlayRemote(hr, Pad(), "m04se_10tataki");
		WiiSnd::Detach(hr);
		m_Anime.Attach(m, "TEX_ANIME_PAC_02", 0u);
		m_vFrom = m_vHome;
		m_nAcc = 0;
		ResetAcc(Pad());
		LineFx();
		if (m_pCursor != nullptr) m_pCursor->Lunge();
		WiiSnd::Detach(h);
		break;
	}
	case 4:
		m_vDest = m_vHome;
		m_vFrom = m->vTrans();
		break;
	case 5:
	{
		cTaskEnemy* e = m_pEnemyMan->pEnemy((uint)m_nComboTarget & 3u);
		const float ry = e->pModel(0u)->vRotate()[1];
		m_vDest = m_pBG->WaniNode((uint)m_nComboTarget & 3u);
		m_vDest[0] += 6.0f * std::cos(HALF_PI_N + ry);   // FLOAT_8047cd04
		m_vDest[2] += 6.0f * -std::sin(HALF_PI_N + ry);
		m_nComboReq = 0;
		m_nAcc = 0;
		ResetAcc(Pad());
		if (m_pCursor != nullptr) m_pCursor->Lunge();
		break;
	}
	case 6:
		if (m_pCursor != nullptr) m_pCursor->Hide();
		m->vRotate() = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m->vTrans() = m_vHome;
		break;
	case 7:
		m_Anime.Attach(m, "TEX_ANIME_PAC_01", 0u);
		m->vRotate() = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		if (m_pCursor != nullptr) m_pCursor->Hide();
		break;
	case 0xb:
		m_vFrom = m->vTrans();
		m_vFrom[1] += Bob();
		break;
	default:
		break;
	}
}

// DOL 0x801b5cd8: the speed line (slot 7, PTK_EFFECT08_LINE) turned toward the destination
void cTaskPacmanBase::LineFx()
{
	const float dx = m_vDest[0] - m_vFrom[0], dy = m_vDest[1] - m_vFrom[1];
	float a = std::atan(dy / dx);
	if (1.0f * dx + 0.0f * dy < 0.0f) a += 3.1415927f;   // DAT_8037d4b8 = (1, 0)
	a -= 1.5707964f;
	m_LineFx.Generate(7u, true, true, true, 0.0f, pModel(0u)->vTrans(), Vector4(0.0f, 0.0f, a, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}

void cTaskPacmanBase::ClearTrail()
{
	if (KingdomGlobal::WaniWaniSys->nPlayers() == 1) return;
	for (int i = 0; i < 8; i++)
		if (m_bTrail[i])
		{
			m_bTrail[i] = false;
			m_TrailFx[i].Detach(true);   // 0x8008a508(h, 1)
		}
}

// DOL 0x801b5ed0: a quadratic bezier from `from` to `to` whose control point sits at `div` of the way, raised by `height` along
// (d x up) x d (d = the normalised direction, up = (0, 1, 0))
void cTaskPacmanBase::Bezier(float t, float div, float height, float out[3], const float from[3], const float to[3])
{
	float d[3] = {to[0] - from[0], to[1] - from[1], to[2] - from[2]};
	const float l = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
	if (l > 0.0f)
		for (float& v : d) v /= l;
	const float up[3] = {0.0f, 1.0f, 0.0f};   // DAT_8037d4c8
	const float c1[3] = {d[1] * up[2] - d[2] * up[1], d[2] * up[0] - d[0] * up[2], d[0] * up[1] - d[1] * up[0]};
	const float n[3] = {c1[1] * d[2] - c1[2] * d[1], c1[2] * d[0] - c1[0] * d[2], c1[0] * d[1] - c1[1] * d[0]};
	const float u = 1.0f - t;
	for (int k = 0; k < 3; k++)
	{
		const float ctrl = n[k] * height + ((1.0f - div) * from[k] + div * to[k]);
		out[k] = u * (u * from[k] + t * ctrl) + t * (u * ctrl + t * to[k]);
	}
}

// DOL 0x801b2ee4 (states 1 / 2)
void cTaskPacmanBase::Input(cJetTask*)
{
	kingdom::cKingdomModel* m = pModel(0u);
	m_fBob += 1.0f;
	const int pad = Pad();
	const int target = Target(pad);
	PushAcc(pad);
	const bool swing = Swing(m_Param.fPlusRange, 0.9f);   // FLOAT_8047cd1c
	m_nComboTarget = 0;
	m_fTimer += 1.0f;
	Vector4 p = m_vHome;
	p[1] += Bob();
	m->vTrans() = p;
	if (m_nState != 1 && m_nState != 7)
	{
		if (m_pCursor != nullptr) m_pCursor->SetPlace(target);
		ClearTrail();
		if (swing)
		{
			m_fTimer = 0.0f;
			m_vDest = m_pBG->WaniNode((uint)target);
			cTaskEnemy* e = m_pEnemyMan->pEnemy((uint)target);
			const float ff = m_Param.fFailTime;
			if (!e->IsHittableOut(ff) && e->nState() != 0xb && e->nState() != 9 && !e->IsAfterBite(ff))
				m_nAimGator = 0;
			else
			{
				m_nAimGator = 1;
				if (e->fMove() + e->fReturn() <= 10.0f && e->fAfter() + e->fBefore() <= 10.0f) m_nAimGator = 0;   // FLOAT_8047cd20
				if (e->nState() == 0xb && e->fReturn() <= 6.0f) m_nAimGator = 0;
				if (e->IsBeforeBiteEnd(ff)) m_nAimGator = 0;
			}
			m_nTarget = target;
			const float ry = m_pEnemyMan->pEnemy((uint)target)->pModel(0u)->vRotate()[1];
			m_vDest[0] += 6.0f * std::cos(HALF_PI_N + ry);
			m_vDest[2] += 6.0f * -std::sin(HALF_PI_N + ry);
			m_nComboTarget = target;
			m_vDest += m_Param.vFailOffset[target & 3];
			SetState(3);
		}
		if (180.0f <= m_fTimer)   // FLOAT_8047cd28: every 3 s the mouth opens and shuts
		{
			m_Anime.Attach(m, "TEX_ANIME_PAC_03", 0u);   // 0x02a7d981
			m_bMouth = true;
			m_fTimer = 0.0f;
		}
		// fcmpo max, frame; ble skip: the DOL swaps PAC_01 back while frame < numFrame, i.e. in the same tick PAC_03 was attached
		// (cAnimeBase attach 0x80090cf8 -> 0x8001a564 starts at frame 0). Kept 1:1; the on-screen result is UNVERIFIED (no Dolphin)
		if (m_bMouth && m->GetFrameAnmTexPat(0u) < m->GetFrameMaxAnmTexPat(0u))   // 0x8001c8f4 (ResAnmTexPat numFrame) / 0x8001c4d8
		{
			m_bMouth = false;
			m_Anime.Attach(m, "TEX_ANIME_PAC_01", 0u);
		}
		// every 30 frames the Mii listener (+0x1e4, multiplayer) +0x10: NOT PORTED
	}
}

// DOL 0x801b3458 (state 3: the lunge)
void cTaskPacmanBase::Lunge()
{
	const int pad = Pad();
	const int target = Target(pad);
	if (m_pCursor != nullptr) m_pCursor->SetPlace(target);
	PushAcc(pad);
	const bool swing = Swing(m_Param.fComboInput, 0.9f);
	const float ff = m_Param.fFailTime;
	if (swing && m_nComboReq == 0 && target != m_nComboTarget)
	{
		cTaskEnemy* e = m_pEnemyMan->pEnemy((uint)target);
		if (e->IsHittableOut(ff) || e->nState() == 0xb || e->nState() == 9 || e->IsAfterBite(ff))
		{
			m_nComboFlag = 1;
			m_nComboReq = 1;
			m_nComboTarget = target;
		}
	}
	float t = m_fTimer / ff;
	if (1.0f <= t) t = 1.0f;
	kingdom::cKingdomModel* m = pModel(0u);
	Vector4 cur = m->vTrans();
	float out[3] = {cur[0], cur[1], cur[2]};
	if (m_nAimGator == 0)
	{
		const float from[3] = {m_vHome[0], m_vHome[1], m_vHome[2]}, to[3] = {m_vDest[0], m_vDest[1], m_vDest[2]};
		Bezier(t, 0.5f, 0.2f, out, from, to);   // FLOAT_8047cd2c / FLOAT_8047cd30
	}
	else
	{
		// the lunge homes in on the gator's mouth (0x801adcb0), turned by the gator's Y when it is not 0 (FUN_80298d00 = PSMTX44RotTrig 'Y',
		// concatenated onto identity, then M * v: x' = c x + s z, z' = -s x + c z; checked in the disassembly)
		cTaskEnemy* e = m_pEnemyMan->pEnemy((uint)m_nTarget & 3u);
		kingdom::cKingdomModel* gm = e->pRedSelModel();
		const Vector4 gp = gm->vTrans();
		float mouth[3];
		e->MouthPos(mouth);
		const float ry = gm->vRotate()[1];
		float r[3] = {mouth[0], mouth[1], mouth[2]};
		if (ry != 0.0f)
		{
			r[0] = std::cos(ry) * mouth[0] + std::sin(ry) * mouth[2];
			r[2] = -std::sin(ry) * mouth[0] + std::cos(ry) * mouth[2];
		}
		const float g[3] = {gp.x, gp.y, gp.z};
		for (int k = 0; k < 3; k++) out[k] = (1.0f - t) * out[k] + t * (r[k] + g[k]);
	}
	Vector4 rot = m->vRotate();
	rot[0] += 0.5235988f;   // FLOAT_8047cd34: he spins
	m->vRotate() = rot;
	m->vTrans() = Vector4(out[0], out[1], out[2], 1.0f);
	if (t < 1.0f)
		m_fTimer += 1.0f;
	else
	{
		// the miss: into the water (the BG's "mizu" node gives the water line)
		const Vector4 mizu = m_pBG->MizuNode();
		Vector4 p = m->vTrans();
		m_nChain = 0;
		if (m_nTarget == 0) m_MissFx.Generate(10u, true, true, true, 0.0f, p);   // PTK_EFFECT11_SMOKE (the far hole is on land)
		else
		{
			p[1] = mizu.y + 0.2f;
			m_MissFx.Generate(5u, true, true, true, 0.0f, p);   // PTK_EFFECT06_WATER
			WiiSnd::Handle h;
			WiiSnd::Play(h, "m04se_11mizu");   // DAT_8047ccf8
			WiiSnd::Detach(h);
		}
		m_nComboFlag = 0;
		m_nChain = 0;
		rot[0] = 0.0f;
		m->vRotate() = rot;
		m_Anime.Attach(m, "TEX_ANIME_PAC_03", 0u);
		SetState(4);
		// the Mii listener +0x24 (miss): multiplayer, NOT PORTED
	}
	// multiplayer: one trail particle per frame in the first free handle (one-time, dependent, clipped, slot player + 0x17)
	if (KingdomGlobal::WaniWaniSys->nPlayers() == 1) return;
	for (int i = 0; i < 8; i++)
	{
		if (m_bTrail[i]) continue;
		m_vTrail[i] = pModel(0u)->vTrans();
		m_TrailFx[i].Generate(m_nPlayer + 0x17u, true, true, true, 0.0f, m_vTrail[i]);   // 0x80021a9c
		m_bTrail[i] = true;
		break;
	}
}

// DOL 0x801b3fc8 (state 4)
void cTaskPacmanBase::Return()
{
	float t = m_fTimer / m_Param.fReturnTime;
	if (1.0f <= t) t = 1.0f;
	kingdom::cKingdomModel* m = pModel(0u);
	const float u = 1.0f - t;
	m->vTrans() = Vector4(u * m_vFrom[0] + t * m_vHome[0], u * m_vFrom[1] + t * m_vHome[1], u * m_vFrom[2] + t * m_vHome[2], 1.0f);
	if (t < 1.0f) m_fTimer += 1.0f;
	else
	{
		m_fTimer = 0.0f;
		SetState(2);
	}
}

// DOL 0x801b3b34 (state 5: the combo jump to the next hole)
void cTaskPacmanBase::ComboJump()
{
	const int pad = Pad();
	const int target = Target(pad);
	PushAcc(pad);
	const bool swing = Swing(m_Param.fComboInput, 0.9f);
	if (m_nComboReq == 0 && swing && target != m_nComboTarget)
	{
		cTaskEnemy* e = m_pEnemyMan->pEnemy((uint)target);
		const int st = e->nState();
		if (e->IsHittableOut(m_Param.fFailTime) || st == 0xb || st == 9 || st == 10)
		{
			m_nComboReq = 1;
			m_nComboTarget = target;
		}
	}
	float t = m_fTimer / m_Param.fReturnTime;
	if (1.0f <= t) t = 1.0f;
	const float from[3] = {m_vFrom[0], m_vFrom[1], m_vFrom[2]}, to[3] = {m_vDest[0], m_vDest[1], m_vDest[2]};
	float out[3];
	Bezier(t, m_Param.fBezierDiv, m_Param.fJumpHeight, out, from, to);
	Vector4 p = pModel(0u)->vTrans();   // w kept, as in the DOL (the Bezier is inlined there: PSVEC sub / normalize / two crosses)
	p[0] = out[0];
	p[1] = out[1];
	p[2] = out[2];
	pModel(0u)->vTrans() = p;
	if (m_fTimer < 2.0f + m_Param.fReturnTime) m_fTimer += 1.0f;   // FLOAT_8047cd38
	else if (m_nComboReq == 1) SetState(5);
	else
	{
		SetState(4);
		m_nComboFlag = 0;
		m_nChain = 0;
	}
}

// DOL 0x801b40f8 (state 9: the multiplayer result walk)
void cTaskPacmanBase::MultiResult()
{
	float t = m_fTimer * 0.03125f;   // FLOAT_8047cd3c
	m_fTimer += 1.0f;
	if (1.0f <= t) t = 1.0f;
	kingdom::cKingdomModel* m = pModel(0u);
	const Vector4 cur = m->vTrans();
	Vector4 to = m_vHome;
	if (m_nWin == 0)
	{
		to[2] += m_Param.fLoseDistance;
		to += m_Param.vLoseOffset[m_nPlayer & 3u];
	}
	else
	{
		to[2] += m_Param.fWinDistance;
		to += m_Param.vWinOffset[m_nPlayer & 3u];
	}
	m_fBob += 1.0f;
	m_v230 = to;
	m->vTrans() = Vector4(cur.x * (1.0f - t) + to[0] * t, cur.y * (1.0f - t) + t * (Bob() + to[1]), cur.z * (1.0f - t) + to[2] * t, 1.0f);
	if (1.0f <= t) SetState(10);
}

// DOL 0x801b294c
bool cTaskPacmanBase::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app->IsPause() || app->IsHome()) return true;
	kingdom::cKingdomModel* m = pModel(0u);
	switch (m_nState)
	{
	case 1:
	case 2: Input(pParam); break;
	case 3: Lunge(); break;
	case 4: Return(); break;
	case 5: ComboJump(); break;
	case 7:
	{
		m_fTimer += 1.0f;
		m_fBob += 1.0f;
		const float t = m_fTimer / 6.0f;   // FLOAT_8047cd04
		const Vector4 c = m->vTrans();
		m->vTrans() = Vector4((1.0f - t) * c.x + t * m_vHome[0], (1.0f - t) * c.y + t * (Bob() + m_vHome[1]), (1.0f - t) * c.z + t * m_vHome[2], 1.0f);
		if (6.0f <= m_fTimer) SetState(8);
		ClearTrail();
		break;
	}
	case 8:
	{
		m_fBob += 1.0f;
		Vector4 p = m_vHome;
		p[1] += Bob();
		m->vTrans() = p;
		break;
	}
	case 9: MultiResult(); break;
	case 10:
	{
		m_fBob += 1.0f;
		Vector4 p = m_v230;
		p[1] += Bob();
		m->vTrans() = p;
		m_fTimer += 1.0f;
		break;
	}
	case 0xb:
	{
		float t = m_fTimer * 0.125f;   // FLOAT_8047cd10
		if (1.0f <= t)
		{
			m_fBob = 0.0f;
			SetState(1);
			t = 1.0f;
		}
		const float u = 1.0f - t;
		m->vTrans() = Vector4(u * m_vFrom[0] + t * m_vHome[0], u * m_vFrom[1] + t * m_vHome[1], u * m_vFrom[2] + t * m_vHome[2], 1.0f);
		m_fTimer += 1.0f;
		break;
	}
	default: break;
	}
	const uint n = KingdomGlobal::WaniWaniSys->nPlayers();
	if (n != 1 && m_nPlayer < n)   // the ring follows Pac-Man; the trail positions shift one place, the newest = Pac-Man
	{
		m_RingFx.vSetTrans(m->vTrans());   // 0x8008a49c
		for (int i = 7; i > 0; i--) m_vTrail[i] = m_vTrail[i - 1];
		m_vTrail[0] = m->vTrans();
		for (int i = 0; i < 8; i++)
			if (m_bTrail[i]) m_TrailFx[i].vSetTrans(m_vTrail[i]);
	}
	if (360.0f <= m_fBob) m_fBob -= 360.0f;
	// 0x80026620(+0x1dc, model): the 3D sound actor follows Pac-Man (natively no cue plays through it)
	UpdateFrame();   // the app's ScnRoot::UpdateFrame
	return true;
}

// DOL 0x801b5778 + 0x801b5644: a sphere (Pac-Man, radius 0.35) against the gator's box (half size 1 / 0.5 / 1, centre z + 0.2, turned by the
// gator's rotation): the box point nearest to him within reach
bool cTaskPacmanBase::HitTest(const float pac[4], kingdom::cKingdomModel* gator)
{
	const Vector4 gp = gator->vTrans();
	const float ry = gator->vRotate()[1];
	const float c[3] = {gp.x, gp.y, gp.z + 0.2f};   // FLOAT_8047cd30
	const float half[3] = {1.0f, 0.5f, 1.0f};        // DAT_8037d478 (2, 1, 2) * 0.5
	const float ax[3][3] = {{std::cos(ry), 0.0f, -std::sin(ry)}, {0.0f, 1.0f, 0.0f}, {std::sin(ry), 0.0f, std::cos(ry)}};
	const float d[3] = {pac[0] - c[0], pac[1] - c[1], pac[2] - c[2]};
	float q[3] = {c[0], c[1], c[2]};
	for (int k = 0; k < 3; k++)
	{
		float s = d[0] * ax[k][0] + d[1] * ax[k][1] + d[2] * ax[k][2];
		if (half[k] < s) s = half[k];
		if (s < -half[k]) s = -half[k];
		for (int j = 0; j < 3; j++) q[j] += s * ax[k][j];
	}
	const float e[3] = {q[0] - pac[0], q[1] - pac[1], q[2] - pac[2]};
	return e[0] * e[0] + e[1] * e[1] + e[2] * e[2] <= pac[3] * pac[3];
}

// DOL 0x801b48d0 (cHitCallback<cTaskPacmanBase, cTaskEnemy>::Call 0x8009460c, every Pac-Man x gator pair each frame)
bool cTaskPacmanBase::HitEnemy(cTaskEnemy* e)
{
	if (m_nState != 3 && m_nState != 5) return false;
	kingdom::cKingdomModel* m = pModel(0u);
	const Vector4 pp = m->vTrans();
	const float pac[4] = {pp.x, pp.y, pp.z, 0.35f};   // FLOAT_8047cd40
	if (!HitTest(pac, e->pRedSelModel())) return false;
	WiiSnd::Handle h;
	WiiSnd::Play(h, "m04se_08ide");   // DAT_8047ccf0
	e->HitEffect(m_nPlayer);
	float mouth[3];
	e->MouthPos(mouth);
	const Vector4 gp = e->pRedSelModel()->vTrans();
	m->vTrans() = Vector4(gp.x + mouth[0], gp.y + 0.7f + mouth[1], gp.z + mouth[2], 1.0f);   // FLOAT_8047cd48
	m_Anime.Attach(m, "TEX_ANIME_PAC_03", 0u);
	Vector4 rot = m->vRotate();
	rot[0] = 0.0f;   // FLOAT_8047ccfc
	m->vRotate() = rot;
	m_vFrom = m->vTrans();
	const int place = (int)e->nPlace();
	if (m_pMan->nPlayer() < 2)
	{
		if (m_nComboFlag == 1)
		{
			m_vFrom = m->vTrans();
			SetState(m_nComboReq == 1 ? 5 : 4);
			if (e->nState() != 0xc) m_nHits++;
			if (m_nLastPlace == place)
			{
				m_nComboFlag = 0;
				m_nChain = 1;
			}
			else
			{
				if (m_nChain != 0) m_nCombo++;
				m_nChain++;
			}
			if (e->IsRed())
			{
				e->nRedHP()--;
				m_nRedHits++;
			}
		}
		else
		{
			if (m_nLastPlace == place)
			{
				m_nComboFlag = 0;
				m_nChain = 0;
			}
			else if (1u < (uint)++m_nChain)
				m_nCombo++;
			if (e->nState() != 0xc) m_nHits++;
			if (e->IsRed())
			{
				if (e->nRedHP() != 0) m_nRedHits++;
				e->nRedHP()--;
			}
			SetState(4);
		}
		if (!e->IsBackingOff())
		{
			if (!e->IsRed() || e->nRedHP() == 0) e->SetState(0xc);
			else e->HitBlink(0x1e, m_nPlayer);
		}
		if (m_pScore != nullptr) m_pScore->SetBonus(this, m_nRedHits);   // listener +0x0c (0x801b5018): the red gator bonus (was missing)
		if (m_pScore != nullptr) m_pScore->Combo(this, m_nCombo, (uint)m_nChain);
	}
	else
	{
		// multiplayer: red = +2 while it has HP, the own colour = +2, another colour = +1 (the Mii listener +0x14: NOT PORTED)
		const bool own = e->nColor() == (int)m_nPlayer;
		if (m_nComboFlag == 1)
		{
			m_vFrom = m->vTrans();
			SetState(m_nComboReq == 1 ? 5 : 4);
			if (e->IsRed())
			{
				if (e->nRedHP() != 0 && e->nState() != 0xc) m_nHits += 2;
				e->nRedHP()--;
			}
			else if (own)
			{
				if (e->nState() != 0xc) m_nHits += 2;
			}
			else if (e->nState() != 0xc)
				m_nHits += 1;
		}
		else
		{
			if (m_nLastPlace == place)
			{
				m_nComboFlag = 0;
				m_nChain = 0;
			}
			else if (1u < (uint)++m_nChain)
				m_nCombo++;
			if (e->IsRed())
			{
				if (e->nRedHP() != 0 && e->nState() != 0xc) m_nHits += 2;
				e->nRedHP()--;
			}
			else
			{
				if (e->nState() != 0xc) m_nHits += own ? 2 : 1;
				e->SetState(0xc);
			}
			SetState(4);
		}
		if (!e->IsRed() || e->nRedHP() == 0) e->SetState(0xc);
		else e->HitBlink(0x1e, m_nPlayer);
		if (m_pScore != nullptr) m_pScore->Combo(this, m_nCombo, (uint)m_nChain);
	}
	m_nLastPlace = place;
	if (m_pScore != nullptr) m_pScore->SetHits(this, m_nHits);
	m_nComboReq = 0;
	WiiSnd::Detach(h);
	return true;
}

// DOL 0x801b5e24 (a bite): the combo breaks; when the score can pay, the damage count rises and the console shows it
void cTaskPacmanBase::Damage()
{
	m_nComboFlag = 0;
	m_nChain = 0;
	// the Mii listener +0x18: multiplayer, NOT PORTED
	if (m_pScore != nullptr && m_pScore->CanPay(this, m_nDamage + 1))
	{
		m_nDamage++;
		m_pScore->SetDamage(this, m_nDamage);   // vtable +0x18, r5 = the new count (disassembly 0x801b5e90..)
		m_nLastPlace = -1;
	}
}

// DOL 0x801b6120
void cTaskPacmanBase::SetWin(int win)
{
	m_nWin = win;
	m_Anime.Attach(pModel(0u), win == 0 ? "TEX_ANIME_PAC_01" : "TEX_ANIME_PAC_04", 0u);
}

// ---- cTaskPacmanManager ----

// DOL 0x801b6310 (the builder 0x80093624 sets +0x1c8 = BG and the player count with 0x801b6550)
cTaskPacmanManager::cTaskPacmanManager(cTaskBG* pBG) : m_pBG(pBG)
{
	SetName("g04::cTaskPacmanManager");
	for (uint i = 0; i < 4; i++)
	{
		m_pPac[i] = new cTaskPacmanBase(i, pBG, this);
		ConnectChild(m_pPac[i]);
	}
}

// DOL 0x801b63fc
bool cTaskPacmanManager::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app->IsPause() || app->IsHome()) return true;
	return cTaskKingdom::Execute(pParam);
}

// DOL 0x801b63b0
void cTaskPacmanManager::Restart()
{
	for (cTaskPacmanBase* p : m_pPac) p->Restart();
}

// DOL 0x801b6484: the player's remote (sys + player * 0x54 + 0x54 = nWork) rumbles 500 ms (0x80021a14), then the penalty
void cTaskPacmanManager::Damage(uint player)
{
	GamePad::Rumble(KingdomGlobal::WaniWaniSys->nWork((int)(player & 3u)), 500);
	m_pPac[player & 3u]->Damage();
}

// DOL 0x801b64e0
void cTaskPacmanManager::SetAllState(int state)
{
	for (uint i = 0; i < m_nPlayer; i++) m_pPac[i]->SetState(state);
}

// DOL 0x801b6550
void cTaskPacmanManager::SetPlayers(uint n)
{
	m_nPlayer = n;
	for (uint i = 0; i < n && i < 4; i++)
	{
		m_pPac[i]->SetState(1);
		m_pPac[i]->pModel(0u)->Visible();
	}
	for (uint i = n; i < 4; i++) m_pPac[i]->SetState(0);
}

// DOL 0x801b65f8
void cTaskPacmanManager::ForgetPlace(uint place)
{
	for (uint i = 0; i < m_nPlayer; i++) m_pPac[i]->ForgetPlace(place);
}

// DOL 0x801b6668
void cTaskPacmanManager::Result(uint winner)
{
	if (m_nPlayer == 1)
		m_pPac[0]->SetState(7);
	else
		for (uint i = 0; i < m_nPlayer; i++)
		{
			m_pPac[i]->SetState(9);
			m_pPac[i]->SetWin(winner == i ? 1 : 0);
		}
}

}
