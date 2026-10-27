#include "SoundManager.hpp"
#include "kingdom/game/g03/cAppMenu.hpp"
#include "platform/devtools.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "gfx/FontWriter.hpp"
#include "gfx/Gfx.hpp"
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
	{job::PACNROLL_TERM, 0.0f, 4.35f, -38.05f, 0.269f},  // Pac-N-Roll left (job 223)
	{job::GALAGA_TERM, 28.17f, 0.46f, -16.22f, -1.25f},  // Galaga
	{job::RALLYX_TERM, -30.0f, 0.46f, 17.38f, 2.475f},   // Rally-X
	{job::WANIWANI_TERM, 34.61f, 1.9f, 19.98f, -2.192f}, // Wani Wani Panic
	{job::MOTOS_TERM, -27.34f, 0.55f, -15.76f, 0.715f},  // Pac-Motos
	{job::MENU_CLASSIC, 0.0f, 0.55f, 35.0f, -3.141f},    // classic menu
	{job::GROBDA_TERM, -25.0f, 0.4f, 0.0f, 1.4f},        // Grobda
};
// the nodes of BIN_SET_MDL (DOL 0x80012d80 string table 0x80368c98): door and "_center" node per game
const char* const kNodes[7][2] = {{"waniwani", "waniwani_center"}, {"rallyx", "rallyx_center"}, {"pacnroll", "pacnroll_center"}, {"galaga", "galaga_center"},
                                  {"motos", "motos_center"}, {"arcade", "arcade_center"}, {"grobda", "grobda_center"}};
// DOL 0x80012204 switch: focus index -> row of CSV_MENU_PARAM holding the camera offset of that door (POINT_*)
const int kPointRow[7] = {15, 14, 13, 16, 17, 18, 19};
// DOL 0x80010eb4 mode 2: selection -> job
const uint kSelJob[9] = {0, job::GALAGA_INIT, job::RALLYX_INIT, job::PACNROLL_INIT, job::WANIWANI_INIT, job::MOTOS_INIT, job::MENU, job::MENU_CLASSIC, job::GROBDA_INIT};

// DOL 0x80012d80 table 0x80368bc0: the cabinet nodes of the classic hall (index = the arcade game index of jobs 141..143: 0..8 vol 1, 9..17 vol 2)
const char* const kCabinets[18] = {"dig_dug", "xevious", "super_pac_man", "pac_pal", "mappy", "gaplus", "galaxian", "cutie_q", "pacmania",
                                   "pacman", "rally", "new_rally", "king", "garaga", "bosconian", "dig_two", "grobda", "motos"};
// DOL 0x803689d0: MAIN_MENU_ANITA_OLDGAME anims of each game's name plate: appear, decided, disappear
const int kGameAnim[18][3] = {{37, 38, 39}, {43, 44, 45}, {52, 53, 54}, {55, 56, 57}, {40, 41, 42}, {49, 50, 51}, {34, 35, 36}, {61, 62, 63},
                              {58, 59, 60}, {64, 65, 66}, {67, 68, 69}, {70, 71, 72}, {73, 74, 75}, {76, 77, 78}, {79, 80, 81}, {82, 83, 84},
                              {85, 86, 87}, {88, 89, 90}};
// DOL 0x80368aa8: digit d = anims 3d (appear), 3d + 1 (decided), 3d + 2 (disappear)
int DigitAnim(uint d, int kind) { return (int)d * 3 + kind; }
// DOL 0x80368b38: the hi scores shown while a volume has no save data (or the game's score is 0)
const uint kDefaultHiScore[18] = {10000, 40000, 40000, 30000, 20000, 50000, 0, 0, 100000, 0, 15000, 20000, 0, 20000, 20000, 20000, 10000, 10000};
const char* const kOldGameAnita = "MAIN_MENU_ANITA_OLDGAME";   // CRC 0xde39e57a

void Copy3(float* d, const float* s) { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; }
std::uint32_t Be32(const std::uint8_t* p) { return (std::uint32_t)p[0] << 24 | (std::uint32_t)p[1] << 16 | (std::uint32_t)p[2] << 8 | p[3]; }
}

cAppMenu::cAppMenu()
{
	SetName("cAppMenu");
	nDrawPriority(0x80000000u);   // DOL 0x8001014c: the app draws in the 2D pass (the classic panel)
	m_bClassic = KingdomGlobal::SysCore->GetJobNo() == job::MENU_CLASSIC;
	KingdomGlobal::KingSys->AttachArcData("g10_menu/g10menu.lzs");   // DOL 0x8001014c
	m_csv.RegistData("CSV_MENU_PARAM");
	m_csvOld.RegistData("menu_old_param.csv");
	m_pFade = new cTaskFade();
	m_pFade->nDrawPriority(0x8fffffffu);
	ConnectChild(m_pFade);
	LoadNodes();

	cTaskCharPacman* pac = pTaskCharPacman();
	if (m_bClassic) ClassicInit();
	else
	{
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
	}
	m_vSaved[0] = pac->vTrans().x; m_vSaved[1] = pac->vTrans().y; m_vSaved[2] = pac->vTrans().z;
	pac->RotateYPacMan(m_fSavedAngle);
	pac->GetParamCommon()->fHangHeight = 1.5f;   // DOL 0x8001014c end: +0x4e8 = 1.5
	Copy3(m_vTarget, m_vSaved);
	Copy3(m_vMenuCam, m_vSaved);
	// the base Init (DOL 0x800b3c2c) started the round's BGM (round 27 -> m03bgm_World1) on the same BGM player; the Wii's BGM player plays one
	// sound at a time, so this start replaces it (UNVERIFIED: player max-playable-count 1 assumed, see NOTES "Hub BGM overlap")
	SoundManager::get_BGM()->Stop();
	m_nBgmVoice = snd::Play("m00bgm_Menu");
	if (m_bClassic) LoadHiScores();
}

// the classic half of DOL 0x8001014c. Arrival: from the hub (138) the CSV's DEFAULT_POS; back from a game (134 save / 143 RSO end) in front of
// the cabinet of KingSys +0xa70, RETURN_LENGTH away along its facing (0x80298d00 = RotY); other jobs keep the stage start (no fade is started)
void cAppMenu::ClassicInit()
{
	cTaskCharPacman* pac = pTaskCharPacman();
	const uint prev = KingdomGlobal::SysCore->GetLastJobNo();
	m_fSavedAngle = 0.0f;
	if (prev == job::MENU)
	{
		pac->vTrans() = Vector4(CsvOld(7, 1), CsvOld(7, 2), CsvOld(7, 3), 1.0f);
		m_pFade->Fade(2u, 500);
	}
	else if (prev == job::SAVE || prev == job::RSO_TERM)
	{
		const int g = KingdomGlobal::KingSys->nRsoGame();
		if (g >= 0 && g < 18)
		{
			const float a = m_vDoorRot[g][1], d = CsvOld(2, 1);
			m_fSavedAngle = a;
			pac->vTrans() = Vector4(m_vDoor[g][0] + std::sin(a) * d, m_vDoor[g][1], m_vDoor[g][2] + std::cos(a) * d, 1.0f);
		}
		m_pFade->Fade(2u, 500);
	}

	// the panel: anita 0 = the game's name plate, 1 = "HI-SCORE", 2..10 = the digits (placed on layers 8..0 of pattern 1); all hidden
	m_anita.pAnita(0)->RegistData(kOldGameAnita);
	m_anita.pAnita(0)->SetAnimNo(0, true, 0.0f);
	m_anita.pAnita(0)->SetState(1u, false);
	m_anita.pAnita(1)->RegistData(kOldGameAnita);
	m_anita.pAnita(1)->SetAnimNo(0, true, 0.0f);
	m_anita.pAnita(1)->SetState(3u, false);
	for (uint i = 0; i < 9; i++)
	{
		cKingdomAnita* a = m_anita.pAnita(i + 2);
		a->RegistData(kOldGameAnita);
		a->SetAnimNo(1, true, 0.0f);
		a->SetState(3u, false);
		Vector4 pos;
		if (a->GetLayerPos(pos, 8 - i, 1))
		{
			a->vTrans()[0] = pos[0];
			a->vTrans()[1] = pos[1];
		}
	}
	// the description: CSV_MSG_OLD_INFO, origin = layer 0 of the first digit's pattern 0 in screen pixels (+ half the frame buffer)
	m_msg.Load("CSV_MSG_OLD_INFO");
	Vector4 pos;
	m_anita.pAnita(2)->GetLayerPos(pos, 0, 0);
	m_vText[0] = (float)gfx::SCREEN_W * 0.5f + pos[0];
	m_vText[1] = (float)gfx::SCREEN_H * 0.5f - pos[1];
}

// the tail of DOL 0x8001014c: block 6 holds both volumes' slices (0x800 bytes each: "NMRM" / "NMMX", then 9 hi scores); a game without data
// shows the DOL's default (capped at 999999999)
void cAppMenu::LoadHiScores()
{
	std::vector<std::uint8_t> buf(0x2000, 0);
	KingdomGlobal::KingSys->LoadGameData(6, buf.data(), 0x2000);
	const bool vol1 = std::memcmp(buf.data(), "NMRM", 4) == 0;
	const bool vol2 = std::memcmp(buf.data() + 0x800, "NMMX", 4) == 0;
	for (uint i = 0; i < 18; i++)
	{
		const bool ok = i < 9 ? vol1 : vol2;
		const uint v = Be32(buf.data() + (i < 9 ? 0 : 0x800) + 4 + (i % 9) * 4);
		m_nHiScore[i] = (v == 0 || !ok) ? kDefaultHiScore[i] : (v > 999999999u ? 999999999u : v);
	}
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
	auto rot = [&](const char* name, float out[3]) {   // degrees * 0.0174533 (FLOAT_80479dd0)
		for (const auto& n : set.nodes)
			if (n.name == name) { for (int k = 0; k < 3; k++) out[k] = n.rotate[k] * 0.017453292f; return; }
	};
	if (m_bClassic)
		for (int i = 0; i < 18; i++) { find(kCabinets[i], m_vDoor[i]); rot(kCabinets[i], m_vDoorRot[i]); }
	else
		for (int i = 0; i < 7; i++) { find(kNodes[i][0], m_vDoor[i]); rot(kNodes[i][0], m_vDoorRot[i]); find(kNodes[i][1], m_vPoint[i]); }
}

void cAppMenu::ToMode(uint mode)
{
	nModeState(0u) = mode;
	m_nStep = 0;
}

// DOL 0x80012a1c. Hub: the kind of the face under Pac-Man (+0x544) -> door number. Classic hall: 6 (back to the hub) inside the exit area
// (EXIT_X_AREA < x, z > EXIT_Z_LIMIT), else 9 + the nearest cabinet on the ground plane within VALID_LENGTH (and 3.0), else 0
int cAppMenu::Select()
{
	if (devtools::G03NoMenuWarp()) return 0; // devtools: --g03-no-menu-warp (not 1:1)
	if (m_bClassic)
	{
		const Vector4& p = pTaskCharPacman()->vTrans();
		if (CsvOld(8, 1) < p.x && p.x < CsvOld(8, 2) && CsvOld(9, 1) < p.z) return 6;
		const float valid = CsvOld(1, 1);
		float best = 3.0f;
		int sel = 0;
		for (int i = 0; i < 18; i++)
		{
			const float dx = m_vDoor[i][0] - p.x, dz = m_vDoor[i][2] - p.z;
			const float d = std::sqrt(dx * dx + dz * dz);
			if (d < best && d < valid) { sel = i + 9; best = d; }
		}
		return sel;
	}
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
	if (m_bClassic) return 8;
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

// DOL 0x80012204. Classic hall: a fixed camera offset (CAM_POS) and the look-at = Pac-Man clamped to the CAM_TGT_*_LIMIT box
void cAppMenu::Focus(int idx)
{
	cTaskCamera* cam = pTaskCamera();
	if (m_bClassic)
	{
		cam->vMenuCamera() = Vector4(CsvOld(3, 1), CsvOld(3, 2), CsvOld(3, 3), 0.0f);
		const Vector4& pac = pTaskCharPacman()->vTrans();
		m_vTarget[0] = pac.x; m_vTarget[1] = pac.y; m_vTarget[2] = pac.z;
		for (int i = 0; i < 3; i++)
		{
			const float lo = CsvOld(4 + i, 1), hi = CsvOld(4 + i, 2);
			if (hi < m_vTarget[i]) m_vTarget[i] = hi;
			if (m_vTarget[i] < lo) m_vTarget[i] = lo;
		}
		return;
	}
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
	m_bPanel = false;
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
		if (m_bClassic)
		{
			ClassicPanel();
			m_bPanel = true;
			Focus(0);
		}
		else Focus(NearestPoint());
		if (m_nSel != 0)
		{
			if (m_nSel < 9) ToMode(MODE_SELECTED);
			else if ((GamePad::Get(0).trig & GamePad::PAD_A) != 0) ClassicDecide();   // a cabinet: A starts its game
		}
		if (pac->vTrans().y < -5.0f)   // fell off the stage: white out (classic hall: black), then back to the saved position
		{
			m_pFade->Fade(m_bClassic ? 0x10002u : 0x10001u, 500);
			ToMode(MODE_FELL);
		}
		if (!m_bClassic && !m_bGoalSound && pTaskItem() != nullptr && pTaskItem()->GetAteCookie() == pTaskItem()->GetCookieMax())
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
		if (m_bClassic) m_bPanel = true;
		if (m_nSel != 0 && m_pFade->IsEnd())
		{
			// 9..26 = a cabinet: job 141 with the arcade game index in KingSys +0xa70 (and 2 in +0xa74)
			const uint job = m_nSel < 9 ? kSelJob[m_nSel] : m_nSel <= 26 ? job::RSO_INIT : 0;
			if (job == job::RSO_INIT)
			{
				KingdomGlobal::KingSys->nRsoGame() = (int)m_nSel - 9;
				KingdomGlobal::KingSys->nRsoA74() = 2;
			}
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
			m_pFade->Fade(m_bClassic ? 2u : 1u, 500);
			ToMode(MODE_START);
		}
		break;
	default:
		break;
	}
	// the camera follows the smoothed look-at point; the focus progress ramps up (CSV row 7 / 8 per frame)
	pTaskCamera()->vTarget() = Vector4(m_vTarget[0], m_vTarget[1], m_vTarget[2], 1.0f);
	if (m_fFocusProgress < 1.0f) m_fFocusProgress += Csv(m_nFocusIdx == 8 ? 8 : 7, 1);
	m_anita.UpdateFrame();   // 0x80014d58
	return true;
}

// the A press on a cabinet in Execute mode 1: the decide SE, the name plate / HI-SCORE / digits switch to their "decided" anims
void cAppMenu::ClassicDecide()
{
	m_bDecided = true;
	snd::Play("m00se_02Kettei");   // 0x80025b6c (TV + remote speaker variant)
	const uint g = m_nSel - 9;
	m_nPanelGame = g;
	if (g < 18)
	{
		m_anita.pAnita(0)->SetAnimNo(kGameAnim[g][1], true, 0.0f);
		m_anita.pAnita(0)->SetState(3u);
		m_anita.pAnita(1)->SetAnimNo(0x20, true, 0.0f);
		m_anita.pAnita(1)->SetState(3u);
		uint score = m_nHiScore[g];
		m_nPanelScore = score;
		for (uint i = 0; i < 9; i++)
		{
			m_anita.pAnita(i + 2)->SetAnimNo(DigitAnim(score % 10, 1), true, 0.0f);
			m_anita.pAnita(i + 2)->SetState(3u);
			score /= 10;
		}
	}
	ToMode(MODE_SELECTED);
}

// DOL 0x80011c9c: after 21 frames on the same cabinet the panel appears (name plate, "HI-SCORE" 0x1e then its loop 0x1f, the digits);
// leaving it plays the disappear anims from the mirrored frame of the current one (so a half-shown panel closes from where it is)
void cAppMenu::ClassicPanel()
{
	const uint old = (uint)m_nSelOld;
	if (old <= 7) return;
	const uint g = old - 9;
	auto setIfNew = [](cKingdomAnita* a, int anim) {
		if (anim == a->GetAnimNo()) return;
		a->SetAnimNo(anim, true, 0.0f);
		a->SetState(3u);
	};
	// set `anim` starting at `rest` frames into it (clamped to the anim), as 0x80141c44 refreshes it
	auto setMirrored = [](cKingdomAnita* a, int anim, float& rest) {
		a->SetAnimNo(anim, true, 0.0f);
		a->SetState(3u);
		const float n = a->GetFrameNum(-1);
		if (rest < n) { if (rest < 0.0f) rest = 0.0f; }
		else rest = n;
		a->SetFrame(rest);
		a->AnimUpdate(0.0f);
	};
	if (old == (uint)m_nSel)
	{
		if (m_nPanelWait < 0x15) { m_nPanelWait++; return; }
		m_nPanelGame = g;
		if (g < 18) m_nPanelScore = m_nHiScore[g];
		if (g < 18) setIfNew(m_anita.pAnita(0), kGameAnim[g][0]);
		cKingdomAnita* hi = m_anita.pAnita(1);
		const int a1 = hi->GetAnimNo();
		if (a1 != 0x1e && a1 != 0x1f)
		{
			hi->SetAnimNo(0x1e, true, 0.0f);
			hi->SetState(3u);
		}
		else if (a1 == 0x1e && hi->IsState(0x10000u)) hi->SetAnimNo(0x1f, true, 0.0f);
		uint score = m_nPanelScore;
		for (uint i = 0; i < 9; i++) { setIfNew(m_anita.pAnita(i + 2), DigitAnim(score % 10, 0)); score /= 10; }
	}
	else if (m_nPanelWait < 0x15) m_nPanelWait = 0;
	else
	{
		m_nPanelGame = g;
		cKingdomAnita* plate = m_anita.pAnita(0);
		if (g < 18 && kGameAnim[g][2] != plate->GetAnimNo())
		{
			float rest = plate->GetFrameNum(-1) - plate->GetFrame();
			setMirrored(plate, kGameAnim[g][2], rest);
		}
		cKingdomAnita* hi = m_anita.pAnita(1);
		if (hi->GetAnimNo() != 0x21)
		{
			float rest = hi->GetFrameNum(-1) - hi->GetFrame();
			setMirrored(hi, 0x21, rest);
		}
		uint score = m_nPanelScore;
		float rest = m_anita.pAnita(2)->GetFrameNum(-1) - m_anita.pAnita(2)->GetFrame();
		for (uint i = 0; i < 9; i++)
		{
			cKingdomAnita* a = m_anita.pAnita(i + 2);
			const int anim = DigitAnim(score % 10, 2);
			score /= 10;
			if (anim != a->GetAnimNo()) setMirrored(a, anim, rest);
		}
		m_nPanelWait = 0;
	}
}

// DOL 0x80011ac8: the description of the shown game (CSV_MSG_OLD_INFO row game + 1, the language column), its alpha follows the name
// plate's anim: fading in while it appears, out while it disappears, full once decided
void cAppMenu::ClassicText()
{
	const int row = (int)m_nPanelGame + 1;
	m_text = m_msg.Get((row < 0 ? 0 : row) - 1);   // MsgTable::Get(i) reads CSV row i + 1
	if (m_text.empty()) return;
	cKingdomAnita* plate = m_anita.pAnita(0);
	const float frame = plate->GetFrame(), n = plate->GetFrameNum(-1);
	if (n == 0.0f) return;
	if (m_bDecided) m_nTextAlpha = 0xff;
	else if (m_nPanelWait < 0x15) m_nTextAlpha = frame == n ? 0 : (std::uint8_t)(int)(255.0f - 255.0f * frame / n);
	else if (frame == n) { if (m_nTextAlpha != 0) m_nTextAlpha = 0xff; }
	else m_nTextAlpha = (std::uint8_t)(int)(255.0f * frame / n);
}

// DOL 0x8001292c: the classic text is queued first, then (in an orthographic 640 x 480 projection) the anitas and the font writer draw
void cAppMenu::Draw(int nStatus, cJetTask* pParam)
{
	(void)nStatus; (void)pParam;
	m_text.clear();
	if (m_bPanel) ClassicText();
	m_anita.Draw();
	if (m_text.empty() || m_nTextAlpha == 0) return;
	// font writer +0x718: RESIDENT_FONT_NOFRAME (KingSys +0xa60), size 20 (0x80016dd8 = +0xa8), colour = the alpha byte alone (0x80016dd0 =
	// +0xa4 = 0x000000aa: black), queued by 0x80016e88 in mode 2 (flags 0x311: baseline, centred)
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_NOFRAME";
	const int height = gfx::FontHeight(st.font);
	st.scaleX = st.scaleY = 20.0f / (float)(height > 0 ? height : 32);
	st.color = m_nTextAlpha;
	st.vorigin = gfx::TextStyle::VOrigin::Baseline;
	gfx::DrawText((float)(int)m_vText[0], (float)(int)m_vText[1], m_text, st, gfx::TextAlign::Center);
}

}
