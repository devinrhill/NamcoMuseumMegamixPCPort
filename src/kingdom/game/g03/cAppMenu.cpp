#include "kingdom/game/g03/cAppMenu.hpp"

#include <cmath>
#include <cstdio>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/KingdomJob.hpp"
#include "snd/Mixer.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"
#include "platform/boot.hpp"

// Positions the DOL uses are Wii space; the converted game code works in the X-mirrored (Unity) convention, so x is negated wherever a DOL
// coordinate enters (or leaves) the game objects.
namespace kingdom::game::g03
{
namespace
{
// DOL 0x8001014c: where Pac-Man appears depending on the job the hub is entered from (Jbc_GetProcNoOld): x, y, z (Wii), facing angle
struct Arrival { uint prevJob; float x, y, z, angle; };
const Arrival kArrivals[] = {
	{0x0df00000u, 0.0f, 4.35f, -38.05f, 0.269f},     // Pac-N-Roll left (job 223)
	{0x0bf00000u, 28.17f, 0.46f, -16.22f, -1.25f},   // Galaga
	{0x0cf00000u, -30.0f, 0.46f, 17.38f, 2.475f},    // Rally-X
	{0x0ef00000u, 34.61f, 1.9f, 19.98f, -2.192f},    // Wani Wani Panic
	{0x11f00000u, -27.34f, 0.55f, -15.76f, 0.715f},  // Pac-Motos
	{0x08c00000u, 0.0f, 0.55f, 35.0f, -3.141f},      // classic menu
	{0x15f00000u, -25.0f, 0.4f, 0.0f, 1.4f},         // Grobda
};
// the nodes of BIN_SET_MDL (DOL 0x80012d80 string table 0x80368c98): door and "_center" node per game
const char* const kNodes[7][2] = {{"waniwani", "waniwani_center"}, {"rallyx", "rallyx_center"}, {"pacnroll", "pacnroll_center"}, {"galaga", "galaga_center"},
                                  {"motos", "motos_center"}, {"arcade", "arcade_center"}, {"grobda", "grobda_center"}};
// DOL 0x80012204 switch: focus index -> row of CSV_MENU_PARAM holding the camera offset of that door (POINT_*)
const int kPointRow[7] = {15, 14, 13, 16, 17, 18, 19};
// DOL 0x80010eb4 mode 2: selection -> job
const uint kSelJob[9] = {0, 0x0b000000u, 0x0c000000u, 0x0d000000u, 0x0e000000u, 0x11000000u, 0x08a00000u, 0x08c00000u, 0x15000000u};

void Copy3(float* d, const float* s) { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; }
}

cAppMenu::cAppMenu()
{
	SetName("cAppMenu");
	KingdomGlobal::KingSys->AttachArcData("g10_menu/g10menu.lzs");   // DOL 0x8001014c
	m_csv.RegistData("CSV_MENU_PARAM");
	m_pFade = new cTaskFade();
	m_pFade->nDrawPriority(0x8fffffffu);
	ConnectChild(m_pFade);
	LoadNodes();

	cTaskCharPacman* pac = pTaskCharPacman();
	float fadeIn = 1;   // Fade kind: 1 white (first arrival), 2 black (coming back from a game)
	const uint prev = KingdomGlobal::SysCore->GetLastJobNo();
	bool known = false;
	for (const Arrival& a : kArrivals)
	{
		if (a.prevJob != prev) continue;
		pac->vTrans() = Vector4(a.x, a.y, a.z, 1.0f);   // Wii space like the camera (a mirrored x put Pac-Motos arrivals at the Galaga door)
		m_fSavedAngle = a.angle;
		fadeIn = 2;
		known = true;
		break;
	}
	if (!known) m_bLanding = true;   // from the title: the stage's start position, Pac-Man lands with a bounce
	m_pFade->Fade((uint)fadeIn, 500);
	m_vSaved[0] = pac->vTrans().x; m_vSaved[1] = pac->vTrans().y; m_vSaved[2] = pac->vTrans().z;
	pac->RotateYPacMan(m_fSavedAngle);
	pac->GetParamCommon()->fHangHeight = 1.5f;   // DOL 0x8001014c end: +0x4e8 = 1.5
	Copy3(m_vTarget, m_vSaved);
	Copy3(m_vMenuCam, m_vSaved);
	m_nBgmVoice = snd::Play("m00bgm_Menu");
}

// DOL 0x80012d80: the door and centre nodes of the placement model (nodes are in Wii space)
void cAppMenu::LoadNodes()
{
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find("BIN_SET_MDL", size);
	wii::Brres brres;
	wii::Mdl0 set;
	if (d == nullptr || !brres.Load(d, size) || brres.ModelNames().empty() || !set.Load(brres, brres.ModelNames().front()))
	{
		std::fprintf(stderr, "cAppMenu: BIN_SET_MDL not found\n");
		return;
	}
	auto find = [&](const char* name, float out[3]) {
		for (const auto& n : set.nodes)
			if (n.name == name) { out[0] = n.translate[0]; out[1] = n.translate[1]; out[2] = n.translate[2]; return; }
		std::fprintf(stderr, "cAppMenu: node %s not in BIN_SET_MDL\n", name);
	};
	for (int i = 0; i < 7; i++) { find(kNodes[i][0], m_vDoor[i]); find(kNodes[i][1], m_vPoint[i]); }
}

void cAppMenu::ToMode(uint mode)
{
	nModeState(0u) = mode;
	m_nStep = 0;
}

// DOL 0x80012a1c (current job != 0x08c00000): the kind of the face under Pac-Man (+0x544) -> door number
int cAppMenu::Select()
{
	const Constants::sFace* face = pTaskCharPacman()->pHitFace(1);
	if (face == nullptr) return 0;
	switch (face->nKind)
	{
	case Constants::eFACE_KIND_MENU_G00: return 1;   // mgal
	case Constants::eFACE_KIND_MENU_G01: return 2;   // mral
	case Constants::eFACE_KIND_MENU_G02: return 3;   // mpac
	case Constants::eFACE_KIND_MENU_G03: return 4;   // mwan
	case Constants::eFACE_KIND_MENU_G04: return 5;   // mmot
	case Constants::eFACE_KIND_MENU_G05: return 7;   // mold
	case Constants::eFACE_KIND_MENU_G07: return 8;   // mgro
	default: return 0;
	}
}

// the loop of Execute: the nearest "_center" node on the ground plane within CSV row 11 (AREA_INSIDE) of Pac-Man, else 8 (follow Pac-Man)
int cAppMenu::NearestPoint() const
{
	const Vector4& p = const_cast<cAppMenu*>(this)->pTaskCharPacman()->vTrans();
	const float inside = const_cast<cAppMenu*>(this)->Csv(11, 1);
	float best = 100.0f;   // FLOAT_80479d28
	int idx = 8;
	for (int i = 0; i < 7; i++)
	{
		const float dx = m_vPoint[i][0] - p.x, dz = m_vPoint[i][2] - p.z;
		const float d = std::sqrt(dx * dx + dz * dz);
		if (d < inside && d < best) { idx = i; best = d; }
	}
	return idx;
}

// DOL 0x80012204 (hub branch)
void cAppMenu::Focus(int idx)
{
	cTaskCamera* cam = pTaskCamera();
	const Vector4 pac = pTaskCharPacman()->vTrans();
	const float p = m_fFocusProgress;
	if (idx == 8)
	{
		if (m_nFocusIdx == 8)
		{
			const float dv[3] = {pac.x - m_vFocus[0], pac.y - m_vFocus[1], pac.z - m_vFocus[2]};
			const float d = std::sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
			if (d <= 0.01f) { m_vFocus[0] = pac.x; m_vFocus[1] = pac.y; m_vFocus[2] = pac.z; }
			else
			{
				const float s = d <= 2.0f ? 0.15f * (d * 0.5f) : 0.15f + (d - 2.0f);
				for (int i = 0; i < 3; i++) m_vFocus[i] += dv[i] / d * s;
			}
		}
		else
		{
			m_fFocusProgress = 0.0f;
			m_nFocusIdx = 8;
			m_vFocus[0] = pac.x; m_vFocus[1] = pac.y; m_vFocus[2] = pac.z;
		}
		const float q = m_fFocusProgress;
		if (q >= 1.0f) Copy3(m_vTarget, m_vFocus);
		else for (int i = 0; i < 3; i++) m_vTarget[i] += q * (m_vFocus[i] - m_vTarget[i]);
		// camera offset from the distance to the stage origin: nearer = higher / closer (CSV rows 1..6)
		const float dist = std::sqrt(pac.x * pac.x + pac.y * pac.y + pac.z * pac.z);
		const float dMax = Csv(2, 1), dMin = Csv(1, 1);
		const float t = dist <= dMax ? (dMin <= dist ? (dMax - dist) / (dMax - dMin) : 1.0f) : 0.0f;
		const float y = t * Csv(5, 1) + Csv(3, 1);
		const float z = Csv(4, 1) + -(t * Csv(6, 1) - Csv(6, 1));
		m_vMenuCam[0] += q * (0.0f - m_vMenuCam[0]);
		m_vMenuCam[1] += q * (y - m_vMenuCam[1]);
		m_vMenuCam[2] += q * (z - m_vMenuCam[2]);
	}
	else
	{
		if (m_nFocusIdx != idx)
		{
			const int row = kPointRow[idx];
			m_vFocusOffset[0] = Csv(row, 1); m_vFocusOffset[1] = Csv(row, 2); m_vFocusOffset[2] = Csv(row, 3);
			m_fFocusProgress = 0.0f;
			m_nFocusIdx = idx;
		}
		const float q = m_fFocusProgress;
		for (int i = 0; i < 3; i++) m_vTarget[i] += q * (m_vDoor[idx][i] - m_vTarget[i]);
		for (int i = 0; i < 3; i++) m_vMenuCam[i] += q * (m_vFocusOffset[i] - m_vMenuCam[i]);
	}
	cam->vMenuCamera() = Vector4(m_vMenuCam[0], m_vMenuCam[1], m_vMenuCam[2], 1.0f);
}

// DOL 0x80010eb4 (hub branch)
bool cAppMenu::Execute(cJetTask* pParam)
{
	const bool home = (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;   // the Home Button Menu freezes the logic
	if (home) return true;
	pTaskCollision()->ClearEntry();   // (the DOL's cAppG03PacnRollGame::Execute does this; the hub Execute does not call it, UNVERIFIED who does)
	cTaskCharPacman* pac = pTaskCharPacman();
	uint& mode = nModeState(0u);
	switch (mode)
	{
	case MODE_START:   // wait for the fade-in, Pac-Man waits at the saved position
		if (m_nStep == 0)
		{
			m_nStep = 1;
			pac->vTrans() = Vector4(m_vSaved[0], m_vSaved[1], m_vSaved[2], 1.0f);
			pac->RotateYPacMan(m_fSavedAngle);
			m_fFocusProgress = 1.0f;
			Focus(8);
		}
		if (m_pFade->IsEnd()) ToMode(MODE_RUN);
		m_fFocusProgress = 1.0f;
		Focus(NearestPoint());
		break;
	case MODE_RUN:
		if (m_nStep == 0)
		{
			m_nStep = 1;
			m_fFocusProgress = 1.0f;
			Focus(NearestPoint());
		}
		if (!m_bLanding) IsStart() = true;
		else if (pac->pHitFace(1) != nullptr)
		{
			// first landing: a small bounce (DOL: vMove.y += 0.3 + param[0]; param first field UNVERIFIED: sParam.fPadForce)
			pac->vMove().y += 0.3f + pac->GetParam()->fPadForce;
			snd::Play("m03se_061_Tyakuti1");
			m_bLanding = false;
		}
		if (m_nSel != 0) m_nSelOld = m_nSel;
		m_nSel = Select();
		Focus(NearestPoint());
		if (m_nSel != 0 && m_nSel < 9) ToMode(MODE_SELECTED);
		if (pac->vTrans().y < -5.0f)   // fell off the stage: white out, then back to the saved position
		{
			m_pFade->Fade(0x10001u, 500);
			ToMode(MODE_FELL);
		}
		if (!m_bGoalSound && pTaskItem() != nullptr && pTaskItem()->GetAteCookie() == pTaskItem()->GetCookieMax())
		{
			m_bGoalSound = true;
			snd::Play("m03se_107_Goal_1");
		}
		break;
	case MODE_SELECTED:
		if (m_nStep == 0)
		{
			m_nStep = 1;
			m_pFade->Fade(0x10002u, 500);
			if (m_nBgmVoice >= 0) snd::FadeOut(m_nBgmVoice, 500);
		}
		if (m_nSel != 0 && m_pFade->IsEnd())
		{
			const uint job = m_nSel < 9 ? kSelJob[m_nSel] : 0;
			if (boot::JobAvailable(job))
			{
				kingdom::Constants::Kingdom_SetJobNo(job, false, false);
				ToMode(MODE_WAIT_JOB);
			}
			else
			{
				std::fprintf(stderr, "cAppMenu: job %#x (door %d) is not ported yet: staying in the hub\n", job, m_nSel);
				m_pFade->Fade(2u, 500);   // black in
				m_nSel = m_nSelOld = 0;
				if (m_nBgmVoice >= 0) m_nBgmVoice = snd::Play("m00bgm_Menu");
				ToMode(MODE_START);
			}
		}
		break;
	case MODE_FELL:
		if (m_pFade->IsEnd())
		{
			m_pFade->Fade(1u, 500);
			ToMode(MODE_START);
		}
		break;
	default:
		break;
	}
	// the camera follows the smoothed look-at point; the focus progress ramps up (CSV row 7 / 8 per frame)
	pTaskCamera()->vTarget() = Vector4(m_vTarget[0], m_vTarget[1], m_vTarget[2], 1.0f);
	if (m_fFocusProgress < 1.0f) m_fFocusProgress += Csv(m_nFocusIdx == 8 ? 8 : 7, 1);
	return true;
}

}
