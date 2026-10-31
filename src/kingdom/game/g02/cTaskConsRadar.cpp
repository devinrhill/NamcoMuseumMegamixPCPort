#include "kingdom/game/g02/cTaskConsRadar.hpp"

#include <cmath>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskConsParts.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"

// Rally-X Remix (g02) the radar (kingdom::game::g02::cTaskConsRadar). Ported from main.dol (no C# exists for Rally-X).
namespace kingdom::game::g02
{

namespace
{
const char* const RADAR = "ANITA_RADAR";   // 0x3a24042c

constexpr int RADAR_PAT[5] = {0, 0, 1, 2, 3};   // 0x8036b780 [player count]: the radar frame's layout pattern
constexpr float SCALE = 6.0f;                    // FLOAT_8047a914: radar pixels per world unit
constexpr float NEAR = 5.0f;                     // FLOAT_8047a944: the warning box half size (world units)

constexpr uint CAR_ANITA = 0x17;                 // 0x8036b804: 0x17 .. 0x1d
constexpr uint RED_MARK_ANITA = 0x1e;            // 0x8036b820: 0x1e .. 0x24
constexpr uint FLAG_ANITA = 3;                   // 0x8036b798 / 0x8036b858: 3 .. 0x16
constexpr int CAR_ICON[7] = {0, 0x2f, 0x31, 0x33, 0x35, 0x37, 0x3e};   // 0x8036b83c [colour]
// 0x8036b8a8: the "next flag" blink clip by type - 0xb (20 words; type 31 = flag 20 would read past the table in the DOL)
constexpr int FLAG_NEXT[20] = {0x44, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
                               0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56};
// 0x8036b8f8: the "taken" clip by type - 0xb (21 words)
constexpr int FLAG_TAKEN[21] = {0x04, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16,
                                0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a};

bool IsBoss()
{
	return KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsBossStage();
}

bool IsMulti()
{
	return KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsMulti();   // 0x800c5ca0
}

// cTaskModel::pModel (0x8009a430) + 0x44 of model 0 or 1: the flag is on the map
bool FlagShown(kingdom::cTaskModel* g)
{
	if (g == nullptr) return false;
	kingdom::cKingdomModel* m0 = g->pModel(0);
	kingdom::cKingdomModel* m1 = g->pModel(1);
	return (m0 != nullptr && m0->IsVisible()) || (m1 != nullptr && m1->IsVisible());   // UNVERIFIED: +0x44 = the visible flag
}

void FlagPos(kingdom::cTaskModel* g, float& x, float& z)
{
	x = z = 0.0f;
	kingdom::cKingdomModel* m = g != nullptr ? g->pModel(0) : nullptr;
	if (m == nullptr) return;
	x = m->vTrans()[0];
	z = m->vTrans()[2];
}
}

// DOL 0x80060f54 [audited]
cTaskConsRadar::cTaskConsRadar(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsRadar");
	nDrawPriority(0x80000002u);
	Regist(0, RADAR, 0x3b);
	Show(0, true);
	Regist(1, RADAR, 0x3c);
	Show(1, false);
	Regist(2, RADAR, 0x43);
	Show(2, false);
	for (uint i = 0x16; i >= 3; i--)   // the flags, registered 0x16 down to 3
	{
		Regist(i, RADAR, 0);
		Show(i, false);
	}
	Regist(0x24, RADAR, 0);
	Show(0x24, false);
	for (uint i = 0; i < 7; i++)   // the cars and the red cars' flag marks, interleaved 0x1d / 0x23, 0x1c / 0x22, ... (0x24 first)
	{
		Regist(0x1d - i, RADAR, 0x2f);
		Show(0x1d - i, false);
		if (i < 6)
		{
			Regist(0x23 - i, RADAR, 0);
			Show(0x23 - i, false);
		}
	}
	const int n = m_pConsole != nullptr ? m_pConsole->nPlayer() : 1;
	const int pat = (n >= 0 && n < 5) ? RADAR_PAT[n] : 0;
	SetPosLayer(0, pat, 0);
	SetPosLayer(1, pat, 0);
	m_fOrgX = pAnita(0)->vTrans()[0];
	m_fOrgY = pAnita(0)->vTrans()[1];
	m_nCar = 0;
	m_nPlayerCar = 0;
	m_nFlag = 0;
	m_bDraw = true;
}

// the radar half of DOL 0x80065db0 [audited]: anita 0x17 + n starts with anim = the colour (reset); Execute sets the real icon every frame
void cTaskConsRadar::AddCar(kingdom::cTaskModel* pCar, int nColour, bool bPlayer)
{
	if (m_nCar >= 7) return;   // the DOL does not check (7 entries)
	pAnita(CAR_ANITA + (uint)m_nCar)->SetAnimNo(nColour, true);
	m_Car[m_nCar].pCar = pCar;
	m_Car[m_nCar].nColour = nColour;
	m_Car[m_nCar].bPlayer = bPlayer;
	if (bPlayer) m_nPlayerCar = m_nCar;
	m_nCar++;
}

// the radar half of DOL 0x800662c8 [audited]: type 0xb (a plain flag) starts in state 0 with clip 0; types 12 .. 33 (flags 1..20, lucky, special)
// with clip (type - 12) * 2 + 3 in state 1; other types are not shown. Placed at the gimmick's model position, shown.
void cTaskConsRadar::AddFlag(kingdom::cTaskModel* pGimmick, int nType)
{
	if (m_nFlag >= 20) return;   // the console counts at most 20 (+0x1c0); the radar does not check
	int anim;
	if (nType == 0xb) anim = 0;
	else if ((uint)(nType - 0xc) < 0x16u) anim = (nType - 0xc) * 2 + 3;
	else return;
	sFlag& f = m_Flag[m_nFlag];
	f.nType = nType;
	f.pGimmick = pGimmick;
	const uint a = FLAG_ANITA + (uint)m_nFlag;
	pAnita(a)->SetAnimNo(anim, true);
	float x, z;
	FlagPos(pGimmick, x, z);
	pAnita(a)->vTrans()[0] = SCALE * x + m_fOrgX;
	pAnita(a)->vTrans()[1] = m_fOrgY - SCALE * z;
	Show(a, true);
	if (nType != 0xb) f.nState++;
	m_nFlag++;
}

// the arrow pick inlined at each warning case (DOL 0x8006b6e8 [audited]): with the top view (sys view word 2) the arrow is `base`; otherwise it is
// turned by the player's grid direction (+0x22c). The caller only reaches this with view 2, so the turned picks are dead code in the DOL.
void cTaskConsRadar::Arrow(int base, int dir, int view)
{
	cTaskConsWarning* w = m_pConsole->pWarning();
	if (w == nullptr) return;
	int idx = base;
	if (view != 2)
	{
		// [base][dir 0..3] (dir 0 and dir >= 4 keep base): base 3: dir 2 -> 2, 1 -> 4, 3 -> 1; base 4: 2 -> 1, 1 -> 3, 3 -> 2; base 2: 2 -> 4, 1 -> 1, 3 -> 3; base 1: 2 -> 3, 1 -> 2, 3 -> 4
		static const int MAP[5][4] = {{0, 0, 0, 0}, {1, 2, 3, 4}, {2, 1, 4, 3}, {3, 4, 2, 1}, {4, 3, 1, 2}};
		if (dir >= 0 && dir < 4 && base >= 1 && base <= 4) idx = MAP[base][dir];
	}
	w->Show((uint)idx, true);
}

// DOL 0x8006b6e8
bool cTaskConsRadar::Execute(cJetTask* pParam)
{
	kingdom::cAppKingdom* app = KingdomGlobal::KingSys != nullptr ? KingdomGlobal::KingSys->pAppMain() : nullptr;
	if (app != nullptr && app->IsPause()) return true;   // KingSys +0xa40 -> +0x1b4 == 1
	sConsCar cars[7];
	const uint nCar = (uint)m_nCar < 7u ? (uint)m_nCar : 7u;
	for (uint i = 0; i < nCar; i++) cTaskConsole::CarState(m_Car[i].pCar, cars[i]);

	// the car icons (and the red cars' flag marks)
	for (uint i = 0; i < nCar; i++)
	{
		kingdom::cKingdomAnita* a = pAnita(CAR_ANITA + i);
		a->vTrans()[0] = SCALE * cars[i].fX + m_fOrgX;
		a->vTrans()[1] = m_fOrgY - SCALE * cars[i].fZ;
		// anita rotation = pModel(0) +0x14 (the car model's Y rotation) copied raw in the DOL. UNVERIFIED: the port's anita rotation is in
		// degrees (Unity Euler) and the model's in radians, so it is converted here; the sign is unchecked.
		kingdom::cKingdomModel* m = m_Car[i].pCar != nullptr ? m_Car[i].pCar->pModel(0) : nullptr;
		if (m != nullptr) a->vRot()[2] = m->vRotate()[1] * (180.0f / 3.14159265358979323846f);
		a->SetState(1u, true);
		if (m_Car[i].nColour == 5)
		{
			if (cars[i].nFlags == 0)
			{
				Show(RED_MARK_ANITA + i, false);
			}
			else
			{
				kingdom::cKingdomAnita* r = pAnita(RED_MARK_ANITA + i);
				r->SetAnimNo(0, true);
				r->vTrans()[0] = a->vTrans()[0];
				r->vTrans()[1] = a->vTrans()[1];
				r->SetState(1u, true);
			}
		}
	}

	// enemies near the player (state 4, single player, not the boss stage): the "near" marks 1 / 2 and the warning banner + arrows
	int nNear = 0;
	cTaskConsWarning* warn = m_pConsole->pWarning();
	const sConsCar& me = cars[(uint)m_nPlayerCar < nCar ? m_nPlayerCar : 0];
	const float x0 = me.fX - NEAR, z0 = me.fZ - NEAR, x1 = me.fX + NEAR, z1 = me.fZ + NEAR;
	if (warn != nullptr)
	{
		for (uint k = 1; k < 5; k++) warn->Show(k, false);
	}
	for (uint i = 0; i < nCar; i++)
	{
		const sConsCar& c = cars[i];
		const int colour = m_Car[i].nColour;
		int icon;
		if (!(c.fTimer2c0 > 0.0f))   // 0x80088498
		{
			if (c.n2c8 != 1) icon = (colour >= 0 && colour < 7) ? CAR_ICON[colour] : 0;   // 0x800884c8
			else icon = colour == 6 ? 0x41 : 0x3a;
		}
		else
		{
			if (c.n2c8 != 1) icon = colour == 6 ? 0x40 : 0x39;
			else icon = colour == 6 ? 0x42 : 0x3d;
		}
		SetAnim(CAR_ANITA + i, icon);
		Show(1, false);   // hidden again for every car (a later car outside the box hides the marks of an earlier one)
		Show(2, false);
		if (m_pConsole->nState() == 4 && !IsMulti() && me.fWait == 0.0f && (uint)m_nPlayerCar != i && c.fWait == 0.0f && !IsBoss())
		{
			if (x0 < c.fX && c.fX < x1 && z0 < c.fZ && c.fZ < z1)
			{
				Show(1, true);
				Show(2, true);
				kingdom::cKingdomAnita* p = pAnita(CAR_ANITA + (uint)m_nPlayerCar);
				pAnita(2)->vTrans()[0] = p->vTrans()[0];
				pAnita(2)->vTrans()[1] = p->vTrans()[1];
				const int view = cTaskConsole::SysViewMode(0);   // 0x800c5b88(sys, 0)
				if (view == 2 && warn != nullptr)
				{
					const int dir = me.nDir;   // player +0x22c
					const float dx = me.fX - c.fX, dz = me.fZ - c.fZ;
					const float ax = std::fabs(dx), az = std::fabs(dz);
					if (az <= ax)
					{
						if (ax <= az)   // |dx| == |dz|: both axes
						{
							if (0.0f <= dz)
							{
								if (dz <= 0.0f)
								{
									Arrow(3, dir, view);
									Arrow(4, dir, view);
								}
								else
								{
									Arrow(3, dir, view);
								}
							}
							else
							{
								Arrow(4, dir, view);
							}
							if (0.0f <= dx)
							{
								if (dx <= 0.0f)
								{
									Arrow(2, dir, view);
									Arrow(1, dir, view);
								}
								else
								{
									Arrow(1, dir, view);
								}
							}
							else
							{
								Arrow(2, dir, view);
							}
						}
						else if (0.0f <= dx)
						{
							if (0.0f < dx) Arrow(1, dir, view);
						}
						else
						{
							Arrow(2, dir, view);
						}
					}
					else if (0.0f <= dz)
					{
						if (0.0f < dz) Arrow(3, dir, view);
					}
					else
					{
						Arrow(4, dir, view);
					}
				}
				nNear++;
			}
		}
	}
	if (warn != nullptr)
	{
		if (nNear == 0)
		{
			warn->m_bActive = false;
			warn->HideAll();
		}
		else
		{
			warn->m_bActive = true;
			if (!warn->pAnita(0)->IsState(1u))
			{
				warn->pAnita(0)->SetState(1u, true);
				warn->pAnita(0)->SetAnimNo(0x57, true);
			}
		}
	}

	// the flags: 0 hidden until its model shows, 1 shown (+ the next numbered flag blinks), 2 the taken clip, 3 gone (a respawned gimmick
	// comes back as a plain flag, type 0xb)
	const uint nFlag = (uint)m_nFlag < 20u ? (uint)m_nFlag : 20u;
	for (uint i = 0; i < nFlag; i++)
	{
		sFlag& f = m_Flag[i];
		const uint a = FLAG_ANITA + i;
		if (IsBoss())
		{
			Show(a, false);
			continue;
		}
		switch (f.nState)
		{
		case 0:
			if (FlagShown(f.pGimmick))
			{
				Show(a, true);
				f.nState++;
			}
			else
			{
				Show(a, false);
			}
			break;
		case 1:
			if (FlagShown(f.pGimmick))
			{
				float x, z;
				FlagPos(f.pGimmick, x, z);
				pAnita(a)->vTrans()[0] = SCALE * x + m_fOrgX;
				pAnita(a)->vTrans()[1] = m_fOrgY - SCALE * z;
				Show(a, true);
				// the lowest-numbered flag still on the map (the scan stops at the first entry that is not a numbered flag 12..31)
				uint best = (uint)m_nFlag, bestType = 0x20;
				for (uint k = 0; k < (uint)m_nFlag; k++)
				{
					const uint t = (uint)m_Flag[k].nType;
					if (t < 0xc || 0x1f < t)
					{
						best = (uint)m_nFlag;
						break;
					}
					if (FlagShown(m_Flag[k].pGimmick) && t < bestType)
					{
						bestType = t;
						best = k;
					}
				}
				if (best != (uint)m_nFlag)
				{
					const int idx = m_Flag[best].nType - 0xb;
					if (idx >= 0 && idx < 20) SetAnim(FLAG_ANITA + best, FLAG_NEXT[idx]);   // idx 20 (flag 20): past the DOL table, skipped
				}
			}
			else
			{
				const int t = f.nType;
				if (t == 0x20) pAnita(a)->SetAnimNo(0x2c, true);        // lucky
				else if (t == 0xb) pAnita(a)->SetAnimNo(2, true);
				else if (t == 0x21) pAnita(a)->SetAnimNo(0x2e, true);   // special
				else if (t - 0xb >= 0 && t - 0xb < 21) SetAnim(a, FLAG_TAKEN[t - 0xb]);
				f.nState++;
			}
			break;
		case 2:
			if (IsEnd(a))
			{
				Show(a, false);
				f.nState++;
			}
			break;
		case 3:
			if (!cTaskConsole::GimmickRespawn(f.pGimmick))   // gimmick +0x1dc (0x800b1d6c)
			{
				Show(a, false);
			}
			else
			{
				pAnita(a)->SetAnimNo(1, true);
				f.nType = 0xb;
				f.nState = 0;
			}
			break;
		default:
			break;
		}
	}

	// 4 players: the radar is drawn only while a pad holds B, never in the multiplayer results (states 8 / 9)
	if (cTaskConsole::SysPlayerNum() == 4)
	{
		m_bDraw = false;
		for (int p = 0; p < 4; p++)
		{
			if ((GamePad::Get(p).hold & GamePad::PAD_B) != 0) m_bDraw = true;
		}
		const int s = m_pConsole->nState();
		if (s == 8 || s == 9) m_bDraw = false;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// DOL 0x8006b6d4 [audited]
void cTaskConsRadar::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_bDraw) return;
	cTaskRallyAnitaBase::Draw(nStatus, pParam);
}

}
