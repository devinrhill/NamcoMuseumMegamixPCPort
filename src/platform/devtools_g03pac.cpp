// The Pac-Man tab of the dev window (see include/platform/devtools_g03pac.hpp). Developer-only, NOT part of the 1:1 game.
// Fields write the members of cTaskCharPacman directly, but only when the value was edited (the game keeps changing them every frame). The form uses
// ChangePacn (the effect and sound of a chocolate item), the power-up uses cAppG03PacnRollGame::IjikeRegist (the real power cookie: Pac-Man, bosses
// and ghosts all start it, and it ends by its own timer).
#include "platform/devtools_g03pac.hpp"
#include "platform/devtools.hpp"
#include "platform/devtools_view.hpp"
#include "platform/devui.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/Gfx3D.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/game/g03/cSaveLoadMan.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include <Constants.hpp>

namespace devtools
{
namespace
{
using kingdom::game::g03::cTaskCharPacman;
using GConstants = kingdom::game::g03::Constants;

int g_panelFrame = -100, g_touchFrame = -100;
bool g_showParams = false;
bool g_scaleModel = true;

bool InStage()
{
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	return core != nullptr && core->GetJobNo() == kingdom::job::PACNROLL_GAME && KingdomGlobal::PacnRollSys != nullptr;
}
cTaskCharPacman* Pac()
{
	kingdom::game::g03::cAppG03PacnRollGame* app = GConstants::PACN_APP();
	return InStage() && app != nullptr ? app->pTaskCharPacman() : nullptr;
}
bool HandleShown() { return FrameNo() - g_panelFrame <= 2 || FrameNo() - g_touchFrame <= 2; }

// the drag
struct Drag
{
	bool on = false;
	float planeY = 0, my0 = 0, startY = 0;
	float grab[3] = {0, 0, 0}, startPos[3] = {0, 0, 0};
} g_drag;

float SnapHalf(float v) { return std::round(v / 0.5f) * 0.5f; }

void Hold(cTaskCharPacman* p)
{
	// where he is dropped he stays: no velocity, no sweep from the old place
	::Constants::jetGeoVecZero(p->vMove());
	::Constants::jetGeoVecCopy(p->vTransOld(), p->vTrans());
	::Constants::jetGeoVecCopy(p->vTransOld2(), p->vTrans());
	::Constants::jetGeoVecZero(p->vForceMove());
}

void SetPos(cTaskCharPacman* p, const float v[3])
{
	p->vTrans() = Vector4(v[0], v[1], v[2], 1.0f);
	Hold(p);
}

// ------------------------------------------------------------------------------------------------ the tab

bool Flag(devui::UI& ui, const devui::Rect& r, const char* label, cTaskCharPacman* p, unsigned bit)
{
	const bool on = p->IsState(bit);
	const bool now = ui.Checkbox(r, label, on);
	if (now != on)
	{
		if (now) p->OnState(bit); else p->OffState(bit);
	}
	return now;
}

void Uint(devui::UI& ui, const char* label, unsigned& v, int id, double maxv)
{
	const double lo = 0.0;
	const double n = ui.Number(ui.Row(), devui::MakeId("g03pac.u", id), label, (double)v, 1, "%.0f", true, &lo);
	const unsigned nv = (unsigned)std::clamp(n, 0.0, maxv);
	if (nv != v) v = nv;
}

void Float(devui::UI& ui, const char* label, float& v, int id, double step)
{
	const double n = ui.Number(ui.Row(), devui::MakeId("g03pac.f", id), label, (double)v, step, "%.3f");
	if ((float)n != v) v = (float)n;
}

void Vec(devui::UI& ui, const char* label, Vector4& v, int id, double step)
{
	float a[3] = {v[0], v[1], v[2]};
	const float o[3] = {a[0], a[1], a[2]};
	ui.Vec3(ui.Row(), devui::MakeId("g03pac.v", id), label, a, step, "%.3f");
	if (a[0] != o[0] || a[1] != o[1] || a[2] != o[2]) v = Vector4(a[0], a[1], a[2], v[3]);
}

void ViewInput(cTaskCharPacman* p);

void PacPanel(devui::UI& ui)
{
	cTaskCharPacman* p = Pac();
	if (p == nullptr) { ui.Label("no Pac-Man here (open a Pac-N-Roll stage)", &devui::Color(devui::DIM)); return; }
	g_panelFrame = FrameNo();
	kingdom::game::g03::cAppG03PacnRollGame* app = GConstants::PACN_APP();
	ui.Heading("Pac-Man");
	ui.Label("Drag him in the game view (the yellow handle) to put him somewhere else; Alt = up / down, Ctrl snaps. He keeps still where you drop him.",
	         &devui::Color(devui::DIM));
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		const bool god = ui.Checkbox(c[0], "No damage", G03NoDamage());
		if (god != G03NoDamage()) SetG03NoDamage(god);
		if (ui.Button(c[1], "Zero velocity")) Hold(p);
	}
	ui.Space();
	ui.Heading("Position");
	Vec(ui, "translation", p->vTrans(), 0, 0.25);
	Vec(ui, "velocity", p->vMove(), 1, 0.05);
	Vec(ui, "last floor", p->vLastFloorPos(), 2, 0.25);
	Float(ui, "radius", p->fRadius(), 3, 0.01);
	g_scaleModel = ui.Checkbox(ui.Row(), "Scale his model with the radius (radius / 0.38)", g_scaleModel);
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
		if (ui.Button(c[0], "Radius 0.38")) p->fRadius() = 0.38f;
		if (ui.Button(c[1], "Up 5")) { float v[3] = {p->vTrans()[0], p->vTrans()[1] + 5.0f, p->vTrans()[2]}; SetPos(p, v); }
		if (ui.Button(c[2], "To last floor")) { float v[3] = {p->vLastFloorPos()[0], p->vLastFloorPos()[1] + 1.0f, p->vLastFloorPos()[2]}; SetPos(p, v); }
	}
	ui.Space();
	ui.Heading("Health");
	Uint(ui, "HP", p->nHP(), 0, cTaskCharPacman::eHP_LIMIT);
	Uint(ui, "max HP", p->nMaxHP(), 1, cTaskCharPacman::eHP_LIMIT);
	Uint(ui, "lives", p->nLife(), 2, cTaskCharPacman::eLIFE_LIMIT);
	if (ui.Button(ui.Row(), "Heal fully")) p->nHP() = p->nMaxHP();
	ui.Space();
	ui.Heading("Form and power-up");
	{
		static const std::vector<std::string> forms = {"Normal", "Wing", "Knight"};
		const int cur = (int)std::min<unsigned>(p->nType(), 2u);
		const int pick = ui.Choice(ui.Row(), devui::MakeId("g03pac.form"), "form", forms, cur);
		if (pick != cur) p->ChangePacn((unsigned)pick);
	}
	{
		const bool power = p->IsState(cTaskCharPacman::STATE_EAT_POWER_COOKIE);
		ui.Label(std::string("power cookie: ") + (power ? "active" : "off") + (app != nullptr && app->IsIjikeTime() ? "  (" + std::to_string(app->IjikeGetCount()) + " frames left)" : ""),
		         &devui::Color(power ? devui::GOOD : devui::DIM));
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		if (ui.Button(c[0], "Start power cookie", app != nullptr)) app->IjikeRegist();
		if (ui.Button(c[1], "End it", app != nullptr && power)) app->IjikeRegist(2);
	}
	Uint(ui, "enemies eaten", p->nEatEnemy(), 3, 9999);
	Uint(ui, "eat combo", p->nEatComboCnt(), 4, 99);
	ui.Space();
	ui.Heading("State flags");
	struct S { const char* name; unsigned bit; };
	static const S states[] = {
	    {"dash", cTaskCharPacman::STATE_DASH}, {"dash floor", cTaskCharPacman::STATE_DASH_FLOOR}, {"power cookie", cTaskCharPacman::STATE_EAT_POWER_COOKIE},
	    {"damage", cTaskCharPacman::STATE_DAMAGE}, {"hang", cTaskCharPacman::STATE_HANG}, {"brake", cTaskCharPacman::STATE_BRAKE},
	    {"hit BG", cTaskCharPacman::STATE_HIT_BG}, {"water up", cTaskCharPacman::STATE_WATER_UP}, {"water in", cTaskCharPacman::STATE_WATER_IN},
	    {"bound", cTaskCharPacman::STATE_BOUND}, {"gears", cTaskCharPacman::STATE_GEARS}, {"jump", cTaskCharPacman::STATE_JUMP},
	    {"climb", cTaskCharPacman::STATE_CLIMB}, {"hell (fell)", cTaskCharPacman::STATE_HELL}, {"miss", cTaskCharPacman::STATE_MISS},
	    {"miss finish", cTaskCharPacman::STATE_MISS_FINISH}};
	for (std::size_t i = 0; i < sizeof states / sizeof states[0]; i += 2)
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		for (std::size_t j = 0; j < 2 && i + j < sizeof states / sizeof states[0]; j++) Flag(ui, c[j], states[i + j].name, p, states[i + j].bit);
	}
	ui.Label("The last three end the run (a miss); the power cookie flag alone does not start the power-up, use the button above.", &devui::Color(devui::DIM));
	ui.Space();
	g_showParams = ui.Checkbox(ui.Row(), "Movement parameters of this form", g_showParams);
	if (g_showParams && p->GetParam() != nullptr)
	{
		cTaskCharPacman::sParam& q = *p->GetParam();
		int id = 100;
		Float(ui, "pad force", q.fPadForce, id++, 0.005);
		Float(ui, "speed max", q.fSpeedMax, id++, 0.01);
		Float(ui, "speed max (air)", q.fSpeedMaxAir, id++, 0.01);
		Float(ui, "gravity (fall)", q.fGravityFall, id++, 0.005);
		Float(ui, "gravity (up)", q.fGravityUp, id++, 0.005);
		Float(ui, "gravity (down)", q.fGravityDown, id++, 0.005);
		Float(ui, "reflect", q.fReflect, id++, 0.05);
		Float(ui, "friction", q.fFriction, id++, 0.005);
		Float(ui, "friction (brake)", q.fFrictionBrake, id++, 0.005);
		Float(ui, "air resist", q.fAirResist, id++, 0.005);
		Float(ui, "air resist (dash)", q.fAirResistDash, id++, 0.005);
		Float(ui, "dash force", q.fDashForce, id++, 0.005);
		Float(ui, "dash speed max", q.fDashSpeedMax, id++, 0.01);
		Float(ui, "dash speed min", q.fDashSpeedMin, id++, 0.01);
		Float(ui, "dash bound", q.fDashBound, id++, 0.05);
		Uint(ui, "dash time", q.nDashTime, id++, 9999);
		Uint(ui, "dash accel time", q.nDashAccelTime, id++, 9999);
		Float(ui, "hang speed", q.fHangSpeed, id++, 0.01);
		Float(ui, "bound speed", q.fBoundSpeed, id++, 0.01);
		Float(ui, "water resist XZ", q.fWaterResistXZ, id++, 0.005);
		Float(ui, "water resist Y", q.fWaterResistY, id++, 0.005);
		Float(ui, "water lift", q.fWaterLift, id++, 0.005);
		Float(ui, "bound floor", q.fBoundFloor, id++, 0.05);
		Float(ui, "bound wall", q.fBoundWall, id++, 0.05);
		Uint(ui, "dash floor time", q.nDashFloorTime, id++, 9999);
		Float(ui, "dash floor accel", q.fDashFloorAcc, id++, 0.005);
		Float(ui, "dash floor speed max", q.fDashFloorSpeedMax, id++, 0.01);
		Float(ui, "seesaw moment", q.fSeesawMoment, id++, 0.005);
		Float(ui, "seesaw accel max", q.fSeesawAccMax, id++, 0.005);
		ui.Label("These are the form's own table (shared by every use of it until the stage is left).", &devui::Color(devui::DIM));
	}
	ViewInput(p);
}

// the tab's own game view input: dragging him
void ViewInput(cTaskCharPacman* p)
{
	(void)p;
	const devui::Input in = devui::GetInput();
	const devui::Rect gv = devui::GameView();
	const bool over = in.mx >= gv.x && in.mx < gv.x + gv.w && in.my >= gv.y && in.my < gv.y + gv.h;
	if (over || PacDragging()) devui::CaptureInput(true, false);
	float vp[16], o[3], d[3];
	if (!ViewProj(vp) || !MouseRay(vp, in.mx, in.my, o, d)) { if (!in.down[0]) PacDrag(in, o, d); return; }
	PacDrag(in, o, d);
	if (over && in.pressed[0]) PacPress(in, vp, o, d);
}
}   // namespace

// ------------------------------------------------------------------------------------------------ game view

void PacTouch() { g_touchFrame = FrameNo(); }
float G03PacModelScale(float radius) { return g_scaleModel && radius > 0.0f ? radius / 0.38f : 1.0f; }
bool PacDragging() { return g_drag.on; }

bool PacPress(const devui::Input& in, const float vp[16], const float o[3], const float d[3])
{
	cTaskCharPacman* p = Pac();
	if (p == nullptr || !HandleShown()) return false;
	const float pos[3] = {p->vTrans()[0], p->vTrans()[1], p->vTrans()[2]};
	float sx, sy;
	if (!Project(vp, pos, sx, sy)) return false;
	// his radius on the screen: the pixel distance to a point one radius above him
	const float up[3] = {pos[0], pos[1] + std::max(0.38f, p->fRadius()), pos[2]};
	float ux, uy;
	float rpx = 14.0f;
	if (Project(vp, up, ux, uy)) rpx = std::max(14.0f, std::sqrt((ux - sx) * (ux - sx) + (uy - sy) * (uy - sy)) * 1.3f);
	if ((in.mx - sx) * (in.mx - sx) + (in.my - sy) * (in.my - sy) > rpx * rpx) return false;
	g_drag = Drag{};
	g_drag.on = true;
	g_drag.planeY = pos[1];
	g_drag.my0 = in.my;
	g_drag.startY = pos[1];
	for (int i = 0; i < 3; i++) g_drag.startPos[i] = pos[i];
	if (std::fabs(d[1]) > 1e-6f)
	{
		const float t = (pos[1] - o[1]) / d[1];
		for (int i = 0; i < 3; i++) g_drag.grab[i] = o[i] + d[i] * t;
	}
	else
		for (int i = 0; i < 3; i++) g_drag.grab[i] = pos[i];
	return true;
}

void PacDrag(const devui::Input& in, const float o[3], const float d[3])
{
	if (!g_drag.on) return;
	cTaskCharPacman* p = Pac();
	if (p == nullptr || !in.down[0]) { g_drag.on = false; return; }
	float v[3] = {g_drag.startPos[0], g_drag.startPos[1], g_drag.startPos[2]};
	if (in.alt)
	{
		float eye[3] = {o[0], o[1], o[2]};
		const devui::Rect gv = devui::GameView();
		float dist = 0;
		for (int i = 0; i < 3; i++) dist += (v[i] - eye[i]) * (v[i] - eye[i]);
		v[1] = g_drag.startY - (in.my - g_drag.my0) * 2.0f * std::sqrt(dist) * 0.9f / (float)gv.h;
	}
	else if (std::fabs(d[1]) > 1e-6f)
	{
		const float t = (g_drag.planeY - o[1]) / d[1];
		if (t > 0.0f)
		{
			v[0] += o[0] + d[0] * t - g_drag.grab[0];
			v[2] += o[2] + d[2] * t - g_drag.grab[2];
		}
	}
	if (in.ctrl) for (int i = 0; i < 3; i++) v[i] = SnapHalf(v[i]);
	SetPos(p, v);
}

void G03PacDraw3D()
{
	cTaskCharPacman* p = Pac();
	if (p == nullptr || !HandleShown()) return;
	std::vector<gfx::LineVertex> lines;
	auto line = [&](const float a[3], const float b[3], unsigned rgba) {
		gfx::LineVertex v{};
		for (int k = 0; k < 4; k++) v.rgba[k] = (std::uint8_t)(rgba >> (24 - 8 * k));
		for (int k = 0; k < 3; k++) v.pos[k] = a[k];
		lines.push_back(v);
		for (int k = 0; k < 3; k++) v.pos[k] = b[k];
		lines.push_back(v);
	};
	const float c[3] = {p->vTrans()[0], p->vTrans()[1], p->vTrans()[2]};
	const float r = std::max(0.2f, p->fRadius());
	const unsigned col = g_drag.on ? 0xffffffffu : 0xffe030ffu;
	for (int ax = 0; ax < 3; ax++)   // three rings: his collision sphere
	{
		float prev[3];
		for (int k = 0; k <= 24; k++)
		{
			const float a = 6.2831853f * (float)k / 24.0f, ca = std::cos(a) * r, sa = std::sin(a) * r;
			float q[3] = {c[0], c[1], c[2]};
			q[(ax + 1) % 3] += ca;
			q[(ax + 2) % 3] += sa;
			if (k > 0) line(prev, q, col);
			for (int i = 0; i < 3; i++) prev[i] = q[i];
		}
	}
	const float lo[3] = {c[0], c[1] - 2.0f, c[2]}, hi[3] = {c[0], c[1] + 2.0f, c[2]};
	line(lo, hi, col);
	if (!lines.empty()) gfx::DrawLines3D(lines.data(), lines.size(), false);
}


// ------------------------------------------------------------------------------------------------ the Pac-N-Roll sys tab

void SysPanel(devui::UI& ui)
{
	kingdom::game::g03::cG03PacnRollSys* sys = KingdomGlobal::PacnRollSys;
	if (sys == nullptr) { ui.Label("no Pac-N-Roll system object", &devui::Color(devui::DIM)); return; }
	kingdom::game::g03::cAppG03PacnRollGame* app = GConstants::PACN_APP();
	ui.Heading("Pac-N-Roll sys");
	ui.Label("The cG03PacnRollSys the game keeps between its menus and the stage. The stage reads the round when it starts: change it, then restart.",
	         &devui::Color(devui::DIM));
	if (app != nullptr && InStage())
		ui.Label("the running stage was built from round " + std::to_string(app->nRound()) + " (the sys's round may have changed since)", &devui::Color(devui::HINT));
	// the round: 0 .. 27 (26 / 27 = the extra stages g03wXXs03 / s04)
	{
		const double lo = 0.0;
		unsigned round = sys->nRound();
		const unsigned nr = (unsigned)std::clamp(ui.Number(ui.Row(), devui::MakeId("g03sys.round"), "round", (double)round, 1, "%.0f", true, &lo), 0.0, 27.0);
		if (nr != round) sys->nRound() = nr;
		const unsigned w = sys->nWorld(), st = sys->nStage();
		ui.Label(std::string(kingdom::game::g03::cG03PacnRollSys::IsBossStage(sys->nRound()) ? "world " : "world ") + std::to_string(w + 1) + (sys->IsBossStage() ? " boss" : " stage " + std::to_string(st + 1)) +
		         "  (of " + std::to_string(sys->nStageMax(w)) + " stages)", &devui::Color(devui::DIM));
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		unsigned w1 = w + 1, s1 = st + 1;
		const unsigned nw = (unsigned)std::clamp(ui.Number(c[0], devui::MakeId("g03sys.world"), "world", (double)w1, 1, "%.0f", true, &lo), 1.0, 7.0);
		const unsigned ns = (unsigned)std::clamp(ui.Number(c[1], devui::MakeId("g03sys.stage"), "stage", (double)s1, 1, "%.0f", true, &lo), 1.0, 8.0);
		if (nw != w1 || ns != s1) sys->nSetRound(nw - 1, ns - 1);
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
		if (ui.Button(c[0], "Previous")) sys->nRound() = sys->nRound() == 0 ? 27 : sys->nRound() - 1;
		if (ui.Button(c[1], "Next")) sys->nRound() = sys->nRound() < 27 ? sys->nRound() + 1 : 0;
		if (ui.Button(c[2], "Restart stage")) kingdom::Kingdom_SetJobNo(kingdom::job::PACNROLL_DEBUG_SELECT, false, false);   // the stage job, on the sys's round
	}
	ui.Space();
	ui.Heading("Flags");
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		sys->IsRetry() = ui.Checkbox(c[0], "retry (a retry start)", sys->IsRetry());
		const bool clear = ui.Checkbox(c[1], "all clear", sys->IsAllClear());
		if (clear != sys->IsAllClear()) sys->bAllClear(clear);
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		const bool open = ui.Checkbox(c[0], "stage newly opened", sys->IsOpenStage());
		if (open != sys->IsOpenStage()) sys->bOpenStage(open);
		sys->DebugStageAllUnlock = ui.Checkbox(c[1], "debug: all stages open", sys->DebugStageAllUnlock);
	}
	ui.Label(std::string("top menu mode: ") + (sys->IsTopMenuMode() ? "yes" : "no") + "   free stage: " + std::to_string(sys->nFreeStage()), &devui::Color(devui::DIM));
	{
		const double lo = 0.0;
		sys->DeadEnemyCnt = (int)ui.Number(ui.Row(), devui::MakeId("g03sys.dead"), "enemies defeated", (double)sys->DeadEnemyCnt, 1, "%.0f", true, &lo);
		sys->GetItemCnt = (int)ui.Number(ui.Row(), devui::MakeId("g03sys.items"), "items got", (double)sys->GetItemCnt, 1, "%.0f", true, &lo);
	}
	if (auto* save = sys->pSaveLoadMan())
	{
		ui.Space();
		ui.Heading("Saved progress (cSaveLoadMan)");
		Uint(ui, "lives", save->nLife(), 200, kingdom::game::g03::cTaskCharPacman::eLIFE_LIMIT);
		Uint(ui, "max HP", save->nMaxHP(), 201, kingdom::game::g03::cTaskCharPacman::eHP_LIMIT);
		ui.Label("(kept in memory; the stage writes them to the save at its end)", &devui::Color(devui::DIM));
	}
	if (auto* retry = sys->pRetryData())
	{
		ui.Space();
		ui.Heading("Retry data (what a retry restores)");
		Uint(ui, "lives", retry->nLife, 210, 99);
		Uint(ui, "max HP", retry->nMaxHP, 211, 7);
		Uint(ui, "game time (frames)", retry->nGameTimeCnt, 212, 4000000.0);
		retry->nRetryPointIdx = (int)ui.Number(ui.Row(), devui::MakeId("g03sys.rp"), "checkpoint index", (double)retry->nRetryPointIdx, 1, "%.0f", true);
		Uint(ui, "cookies eaten", retry->nAteCookie, 213, 99999);
		Uint(ui, "cookies held", retry->nHaveCookie, 214, 99999);
		ui.Label(std::to_string(retry->vecItem.size()) + " items, " + std::to_string(retry->vecPoint.size()) + " checkpoints, " + std::to_string(retry->vecGate.size()) + " gates, " +
		         std::to_string(retry->vecBox.size()) + " boxes kept", &devui::Color(devui::DIM));
	}
	else
		ui.Label("no retry data (made when Pac-Man misses and retries)", &devui::Color(devui::DIM));
}

void RegisterG03PacPanel()
{
	devui::RegisterPanel("Pac-Man", PacPanel, [] { return InStage(); });
	devui::RegisterPanel("Pac-N-Roll", SysPanel, [] { return KingdomGlobal::PacnRollSys != nullptr; });
}
}
