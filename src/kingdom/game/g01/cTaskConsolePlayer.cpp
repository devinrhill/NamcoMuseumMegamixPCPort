#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "kingdom/game/g01/cTaskBoss.hpp"

#include <algorithm>
#include <cmath>

#include "GamePad.hpp"
#include "snd/Mixer.hpp"
#include "KingdomGlobal.hpp"
#include "platform/devtools.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskCamera.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskPacman.hpp"

namespace kingdom::game::g01
{

namespace
{
const int PANEL_ANM[5] = {0x2e, 0x32, 0x36, 0x3a, 0x2a};             // DOL 0x80372018 [audited] (players 1..4, alone): wait; -1 appear, +1 shot, +2 out
const int DIGIT_ANM[10] = {2, 5, 8, 0xb, 0xe, 0x11, 0x14, 0x17, 0x1a, 0x1d};   // DOL 0x80372030 [audited]: digits 0..9
const int DIGIT_LAYER[6] = {8, 7, 6, 5, 4, 3};                      // DOL 0x80371f88 [audited]: anitas 1..6 = 10^0 .. 10^5
const int PANEL_LAYER[5][4] = {{-1, -1, -1, -1}, {0, -1, -1, -1}, {0, 1, -1, -1}, {0, 1, 2, -1}, {0, 1, 2, 3}};   // DOL 0x80372004 [audited] -> 0x80371fb0..
// nw4r::math::IntersectionRay3AABB: slab test, t = distance along the (unit) direction to the entry point
bool RayAABB(const float org[3], const float dir[3], const float mn[3], const float mx[3], float& t)
{
	float t0 = 0.0f, t1 = 1e30f;
	for (int i = 0; i < 3; i++)
	{
		if (std::fabs(dir[i]) < 1e-12f)
		{
			if (org[i] < mn[i] || org[i] > mx[i]) return false;
			continue;
		}
		float a = (mn[i] - org[i]) / dir[i], b = (mx[i] - org[i]) / dir[i];
		if (a > b) std::swap(a, b);
		t0 = std::max(t0, a);
		t1 = std::min(t1, b);
		if (t0 > t1) return false;
	}
	t = t0;
	return true;
}
}

// ---- cTaskBeam -------------------------------------------------------------------------------------------------------------------

// DOL 0x800c7ff8 [audited] (and the cObject part 0x800c7f5c-like init: kind 4 UNVERIFIED, not collidable)
cTaskBeam::cTaskBeam(int nPlayer)
{
	SetName("g01::cTaskBeam");
	nDrawPriority(0x80000004u);   // UNVERIFIED: drawn under the aim (0x80000003 is the aim's)
	m_nPlayer = nPlayer;
	m_b09 = false;
	pAnita(0)->RegistData("ANT_BEAM");
	pAnita(0)->SetAnimNo(1 + 2 * nPlayer, true);
	pAnita(0)->SetState(1u, false);
}

void cTaskBeam::Start(const Vector4& aim)
{
	m_vAim = aim;
	m_nState = 1;
	m_nStep = 0;
	m_b09 = true;
}

// cTaskAim (0x800c7f5c): the player's crosshair, a cTaskCursor whose Draw (0x800c7e7c) draws nothing while paused (DAT_80479118)
class cTaskAim : public kingdom::cTaskCursor
{
public:
	void Draw(int nStatus, cJetTask* pParam) override
	{
		if (cG01GalagaSys::IsPause()) return;
		kingdom::cTaskCursor::Draw(nStatus, pParam);
	}
};

// DOL 0x800c80a4 [audited: state and step are reset natively when the laser ends (the DOL leaves state 1)]
bool cTaskBeam::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_nState == 1)
	{
		if (m_nStep == 0)
		{
			pAnita(0)->SetAnimNo(1 + 2 * m_nPlayer, true);   // DAT_80371360[player]
			pAnita(0)->SetState(1u, true);
			m_b09 = true;
			m_nStep++;
		}
		else if (m_nStep == 1 && pAnita(0)->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END))
		{
			m_b09 = false;
			pAnita(0)->SetState(1u, false);
			m_nState = 0;
		}
	}
	pAnita(0)->vTrans() = Vector4(m_vAim.x, m_vAim.y - 240.0f, 0.0f, 0.0f);   // FLOAT_8047b358
	return cTaskAnita::Execute(pParam);
}

// DOL 0x800d665c [audited: only the fields, anim tables, panel layers and LoadCarry call compared; the name text / Mii child and the aim cursor are not] (the panel anita, name text, life display ... are not ported). Kind 1 = a console player for the collision pass.
cTaskConsolePlayer::cTaskConsolePlayer(int nPlayer, int nPad, int nPlayers)
{
	SetName("g01::cTaskConsolePlayer");
	m_nPlayer = nPlayer;
	m_nPad = nPad;
	m_nKind = 1;
	m_b08 = true;
	m_b14 = true;
	kingdom::cKingdomCSV csv;   // cTaskConsolePlayer::cPlayerParam: PlayerParam.csv
	csv.RegistData("PlayerParam.csv");
	csv.S32(csv.SearchRow(0, "BEAM_MAX", 0, 0), 1, m_nBeamMax);
	csv.S32(csv.SearchRow(0, "BEAM_BLANK_FRAME", 0, 0), 1, m_nBlank);
	csv.S32(csv.SearchRow(0, "BEAM_WAIT_FRAME", 0, 0), 1, m_nWaitFrame);
	// cTaskAim (0x800c7f5c): a cursor on the player's pad with ANT_BEAM (0x1f3e9d31), priority 0x80000003. Its anim is the looping crosshair
	// 2 * player (ANT_BEAM: even anims loop the target, odd ones are the one-shot lasers of cTaskBeam; UNVERIFIED which call sets it)
	m_pAim = new cTaskAim();
	m_pAim->Init(nPad, "ANT_BEAM", 2 * nPlayer);
	m_pAim->nDrawPriority(0x80000003u);
	m_pAim->SetEnable(false);  // 0x800d665c: cTaskAim +0x231 (= the cursor's +0x1e1) = 0; Execute shows it while the console is enabled
	ConnectChild(m_pAim);
	if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(this);
	if (KingdomGlobal::GalagaSys->nStage() == 10) LoadCarry();   // 0x800d665c: vtable +0x88 for the last stage
	for (int i = 0; i < m_nBeamMax && i < 16; i++)
	{
		m_pBeam[i] = new cTaskBeam(nPlayer);
		ConnectChild(m_pBeam[i]);
	}
	// the panel (0x800d665c): origin = layer PANEL_LAYER[players][player] of pattern = players (DAT_80371ff0) of a scratch ANT_SCORE anita;
	// anita 0 = frame (layer 9), anitas 1..6 = score digits; all hidden until the appear (0x800d572c)
	m_nPanel = nPlayers == 1 ? 4 : nPlayer;
	const int pat = nPlayers < 1 ? 1 : (nPlayers > 4 ? 4 : nPlayers);
	Vector4 origin = Vector4::zero;
	cKingdomAnita* scratch = pAnita(15);   // the DOL uses a stack anita (anitas 0..0xd are the console's own)
	scratch->RegistData("ANT_SCORE");
	scratch->GetLayerPos(origin, (uint)PANEL_LAYER[pat][nPlayer < 4 ? nPlayer : 0], pat);
	scratch->SetState(1u, false);
	auto place = [this, &origin](uint n, uint layer, int anm) {
		cKingdomAnita* a = pAnita(n);
		a->RegistData("ANT_SCORE");
		a->SetAnimNo(anm, true);
		Vector4 pos = Vector4::zero;
		a->GetLayerPos(pos, layer, 0);
		a->vTrans() = Vector4(pos.x + origin.x, pos.y + origin.y, pos.z + origin.z, 0.0f);
		a->SetState(1u, false);
	};
	place(0, 9, PANEL_ANM[m_nPanel]);
	for (uint i = 0; i < 6; i++) place(i + 1, (uint)DIGIT_LAYER[i], DIGIT_ANM[0]);
	for (uint i = 0; i < 6; i++) place(i + 7, (uint)DIGIT_LAYER[i], DIGIT_ANM[0]);   // 0x800d6a1c: the same digits again (the pop's second layer)
}

// DOL 0x800d572c [audited]: 0 panel appear anim (wait - 1); 1 at its end the wait anim, digits appear (anim 0x40); 2 wait for the digits; 3 the
// score's digits (restarted) and done (console state 2). Step 4 = console state 3 (0x800eb694, started for every console by the manager's
// 0x800eb87c; UNVERIFIED: taken as right after the appear): the panel part of 0x800d60c4 every frame
void cTaskConsolePlayer::PanelExec()
{
	const int wait = PANEL_ANM[m_nPanel];
	switch (m_nPanelStep)
	{
	case 0:
		pAnita(0)->SetState(1u, true);
		pAnita(0)->SetAnimNo(wait - 1, true);
		m_nPanelStep++;
		break;
	case 1:
		if (pAnita(0)->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END))
		{
			pAnita(0)->SetAnimNo(wait, true);
			for (uint i = 1; i <= 6; i++)
			{
				pAnita(i)->SetAnimNo(0x40, true);
				pAnita(i)->SetState(1u, true);
			}
			m_nPanelStep++;
		}
		break;
	case 2:
	{
		bool end = true;
		for (uint i = 1; i <= 6 && end; i++) end = pAnita(i)->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END);
		if (end) m_nPanelStep++;
		break;
	}
	case 3:
	{
		int div = 1;
		for (uint i = 0; i < 6; i++, div *= 10)
			pAnita(i + 1)->SetAnimNo(DIGIT_ANM[(m_nScore % (div * 10)) / div], true);
		m_nPanelStep++;
		break;
	}
	default:
		// DOL 0x800d60c4 [audited: panel anim part ported as PanelExec; shot section compared in structure only]: the digits, then the panel: a hit this frame (+0x24d) plays the shot anim (wait + 1, not restarted); once it
		// ends (or the panel is hidden) the wait anim again
		DigitExec(1);
		if (m_bShotHit)
			pAnita(0)->SetAnimNo(wait + 1, false);
		else if (pAnita(0)->GetAnimNo() == wait + 1 &&
		         (!pAnita(0)->IsState(1u) || pAnita(0)->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END)))
			pAnita(0)->SetAnimNo(wait, false);
		break;
	}
}

// DOL 0x800d5aa0 [audited]. Kind 1, when the score moved (+0x244 != +0x248): from the top digit down, a digit that changed (and every one below a
// changed one) pops: anita i + 1 plays the digit's in anim (anim - 1), anita i + 7 is shown with its out anim (anim + 1); the others go back
// to the wait anim (anita i + 1) / are hidden (anita i + 7) once theirs ended. Kind 2: every digit's out anim on both.
void cTaskConsolePlayer::DigitExec(int kind)
{
	auto pow10 = [](int n) { int r = 1; while (n-- > 0) r *= 10; return r; };
	auto digit = [&pow10](int score, int i) { return (score % pow10(i + 1)) / pow10(i); };
	auto idle = [](cKingdomAnita* a) { return !a->IsState(1u) || a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END); };
	if (kind == 1)
	{
		if (m_nScore == m_nScorePrev) return;
		bool changed = false;
		for (int i = 5; i >= 0; i--)
		{
			const int cur = digit(m_nScore, i);
			if (digit(m_nScorePrev, i) != cur) changed = true;
			cKingdomAnita* a = pAnita((uint)i + 1);
			cKingdomAnita* b = pAnita((uint)i + 7);
			if (changed)
			{
				a->SetAnimNo(DIGIT_ANM[cur] - 1, true);
				b->SetAnimNo(DIGIT_ANM[cur] + 1, true);
				b->SetState(1u, true);
			}
			else
			{
				if (idle(a)) a->SetAnimNo(DIGIT_ANM[cur], true);
				if (idle(b)) b->SetState(1u, false);
			}
		}
		m_nScorePrev = m_nScore;
	}
	else if (kind == 2)
	{
		for (int i = 0; i < 6; i++)
		{
			const int cur = digit(m_nScore, i);
			pAnita((uint)i + 1)->SetAnimNo(DIGIT_ANM[cur] + 1, true);
			pAnita((uint)i + 7)->SetAnimNo(DIGIT_ANM[cur] + 1, true);
		}
	}
}

// DOL 0x800d6444 [audited: +0x26c object child +0x7c = 0 not ported]: step 0 the panel's out anim (wait + 2) and the digits' (0x800d5aa0 kind 2); then done when anitas 0..12 are hidden or
// at their end. (+0x26c's object +0x1b4 child +0x7c = 0 first: not ported)
bool cTaskConsolePlayer::ResultExec()
{
	if (m_nResultStep == 0)
	{
		DigitExec(2);
		pAnita(0)->SetAnimNo(PANEL_ANM[m_nPanel] + 2, false);
		m_nResultStep++;
		return false;
	}
	for (uint i = 0; i <= 12; i++)
		if (pAnita(i)->IsState(1u) && !pAnita(i)->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END)) return false;
	return true;
}

// the ray of a shot: cTaskBeam 0x800c8310 unprojects the aim point (anita +0x400, layout pixels) through the camera's projection and view
// matrices; here the same ray from the camera position, built from the pointer (-1..1, y down) and the camera's field of view (aspect 4:3)
void cTaskConsolePlayer::Fire()
{
	const GamePad::State& pad = GamePad::Get(m_nPad);
	float eye[3], aim[3], fovy = 60.0f;
	kingdom::NativeScene::GetCamera(eye, aim, fovy);   // the current camera (0x800c8310 uses ScnRoot::GetCurrentCamera's matrices)
	const float th = std::tan(fovy * 0.5f * 3.14159265f / 180.0f);
	const float x = pad.pointerX * th * (4.0f / 3.0f), y = -pad.pointerY * th;
	// the view basis of the aim posture (as kingdom::NativeScene builds the view matrix): f = aim - eye, right = f x (0, 1, 0), up = right x f
	float f[3] = {aim[0] - eye[0], aim[1] - eye[1], aim[2] - eye[2]};
	float len = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
	if (len <= 0.0f) return;
	for (float& v : f) v /= len;
	float r[3] = {-f[2], 0.0f, f[0]};   // f x (0, 1, 0)
	len = std::sqrt(r[0] * r[0] + r[2] * r[2]);
	if (len <= 0.0f) return;
	for (float& v : r) v /= len;
	const float u[3] = {r[1] * f[2] - r[2] * f[1], r[2] * f[0] - r[0] * f[2], r[0] * f[1] - r[1] * f[0]};
	len = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		m_vRayOrg[i] = eye[i];
		m_vRayDir[i] = f[i] + r[i] * x + u[i] * y;
		len += m_vRayDir[i] * m_vRayDir[i];
	}
	len = std::sqrt(len);
	for (float& v : m_vRayDir) v /= len;
	if (devtools::G01AutoAim() && cTaskEnemyMan::s_pInstance != nullptr) // devtools: --g01-autoaim (not 1:1)
	{
		float t[3];
		if (cTaskEnemyMan::s_pInstance->NearestTarget(eye, f, t))
		{
			float l2 = 0.0f;
			for (int i = 0; i < 3; i++) { m_vRayDir[i] = t[i] - eye[i]; l2 += m_vRayDir[i] * m_vRayDir[i]; }
			l2 = std::sqrt(l2);
			for (float& v : m_vRayDir) v /= l2;
		}
	}
	m_bShot = true;
	m_nShot++;   // +0x23c
	for (cTaskBeam* b : m_pBeam)   // a free beam (+9 == 0) shows the laser at the aim point (the cursor anita +0x400)
		if (b != nullptr && b->IsFree())
		{
			b->Start(Vector4(pad.pointerX * 320.0f, -pad.pointerY * 240.0f, 0.0f, 0.0f));
			break;
		}
	snd::Play("m01se_02laser");   // FUN_801c15ac(player, DAT_8047b4a0): the shot sound (the DOL also routes it to the player's remote)
}

// DOL 0x800d60c4 (the shot part; the panel animations are not ported)
bool cTaskConsolePlayer::Execute(cJetTask* pParam)
{
	m_pAim->SetEnable(m_bEnable);   // DOL 0x800d6080 [audited]: the aim is shown exactly while the console is enabled (+0x24c; its "off while paused" store is overwritten)
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: the console base Execute 0x800eb400 does nothing while paused
	// DOL 0x800d6400 [audited: swing source (pad +0xa8/+0x84) is the native accSpeed, UNVERIFIED]: a swing (pad +0x84 > 0.5, FLOAT_8047b4ac) asks Pac-Man (vtable +0xf8) to jump. UNVERIFIED: the native pad's accSpeed
	// stands for +0x84, and the test is made only while the console is enabled
	if (m_bEnable && GamePad::Get(m_nPad).accSpeed > 0.5f)
		if (cTaskPacman* pac = cTaskPacman::s_pInstance) pac->RequestJump();
	if (m_bPanel) PanelExec();
	if (m_bResult && ResultExec()) m_bResult = false;   // console base 0x800eb400 state 4 -> 5
	cTaskAnita::Execute(pParam);
	m_bShot = false;
	m_bShotHit = false;
	const GamePad::State& pad = GamePad::Get(m_nPad);
	if ((pad.hold & (GamePad::PAD_A | GamePad::PAD_B)) != 0)
	{
		const bool pointing = (pad.hold & GamePad::PAD_HOME) == 0 && pad.pointerCount > 0;
		if (pointing && m_bEnable)
		{
			if (0.0f < m_fWait)
			{
				m_fWait -= 1.0f;
			}
			else
			{
				Fire();
				if (++m_nBurst < m_nBeamMax)
				{
					m_fWait = (float)m_nBlank;
				}
				else
				{
					m_nBurst = 0;
					m_fWait = (float)m_nWaitFrame;
				}
			}
			return true;
		}
	}
	m_nBurst = 0;
	m_fWait = 0.0f;
	return true;
}

// DOL 0x800d7198 [audited: the ray / AABB test is the port's own slab test (nw4r IntersectionRay3AABB not compared)]: the last boss (kind 0xd) via 0x800cf250 (its parts); kinds 8..0xb and 0xe..0xf: the shot's ray against the
// target's world AABB (+0x68)
bool cTaskConsolePlayer::HitCheck(cObject* other)
{
	if (!m_bShot || m_bShotHit) return false;
	const int kind = other->nKind();
	if (kind == 0xd)
	{
		// the last boss: 0x800cf250 (its shells, then its hit model), the hit at org + dir * t
		cTaskBoss* boss = dynamic_cast<cTaskBoss*>(other);
		float t = 0.0f;
		if (boss == nullptr || !boss->RayHit(m_vRayOrg, m_vRayDir, t)) return false;
		for (int i = 0; i < 3; i++) m_vHitPos[i] = m_vRayOrg[i] + m_vRayDir[i] * t;
		return true;
	}
	if (!((8 <= kind && kind < 0xc) || (0xd < kind && kind < 0x10))) return false;
	const cObjectAABB* box = dynamic_cast<const cObjectAABB*>(other);
	if (box == nullptr || !box->bAABB()) return false;
	float t = 0.0f;
	if (!RayAABB(m_vRayOrg, m_vRayDir, box->vWorldMin(), box->vWorldMax(), t)) return false;
	for (int i = 0; i < 3; i++) m_vHitPos[i] = m_vRayOrg[i] + m_vRayDir[i] * t;   // 0x800c8764: +0x260 = org + dir * t
	return true;
}

// DOL 0x800d7344 [audited]
void cTaskConsolePlayer::HitCallback(cObject* other)
{
	const int kind = other->nKind();
	if ((unsigned)(kind - 0xd) < 3 || (unsigned)(kind - 10) < 2 || kind == 8 || kind == 5) m_nHit++;
	if (kind != 9) m_bShotHit = true;
}


// cTaskConsolePlayer +0x18 (task part) = 0x800d65bc: disabled (+0x24c = 0), returns 0 - SetEnable(false).
// DOL 0x800eb548 (cTaskGameConsoleBase): only while active (+8) and shown (+9), the anitas over a 640 x 480 ortho
void cTaskConsolePlayer::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskAnita::Draw(nStatus, pParam);   // FUN_80014df8
}


// DOL 0x800d65cc [audited]: the boss stage continues from the run so far: alone, the stage work (cG01GalagaSys +0x1c, filled by stage 9's SaveRecord):
// score +0x10 (-> shown and real score +0x244 / +0x248), hit rate +0x08 (-> +0x240, blended into the boss result), kills +0x00; with several
// players the multi work (+0x20) by player: +0x28 / +0x18 / +0x08 + 4 * player
void cTaskConsolePlayer::LoadCarry()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	if (!sys->IsMulti())   // 0x800afd40 (named IsSingle in the map; it is "is multiplayer")
	{
		const sStageWork* w = sys->pStageWork();
		m_nScore = m_nScorePrev = w->n10;   // +0x244 and +0x248
		m_nRateCarry = w->n08;
		m_nKill = w->n00;
	}
	else if (m_nPlayer >= 0 && m_nPlayer < 4)
	{
		const sMultiWork* w = sys->pMultiWork();
		m_nScore = m_nScorePrev = w->n[10 + m_nPlayer];
		m_nRateCarry = w->n[6 + m_nPlayer];
		m_nKill = w->n[2 + m_nPlayer];
	}
}

}
