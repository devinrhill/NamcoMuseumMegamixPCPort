// The dev window's Editor tab (devui, --devui / F9), shown inside a Pac-N-Roll stage (job PACNROLL_GAME): a placement editor for the stage's
// sSaveData* files, a C++ port of unpac's kingdom/pacnroll editor.py + savedata.py (first part: no line tools, models, camera CSVs, effects).
// The stage's own entries are read from the attached archive; "Apply" hands the edited bytes to cKingdomSys::SearchDataEx (G03EditOverride, only
// for the edited round, only in PACNROLL_GAME) and restarts the stage through PACNROLL_DEBUG_SELECT like the debug menu. Save / Load keep a
// round's edited files in a folder ($MEGAMIX_G03_EDITS, else $XDG_DATA_HOME/megamix/g03_edits, else ~/.local/share/megamix/g03_edits).
// NOT part of the 1:1 game.
//
// Records keep their raw bytes: only the named fields change (savedata.py: the unused words hold stale memory of the original tool). Files are
// arrays of big-endian records padded to 32 bytes; the ghost files are one big record each (areas / move points / ghosts, route points + rider).
#include "platform/devtools.hpp"
#include "platform/devtools_view.hpp"
#include "platform/devui.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/Gfx3D.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskCollision.hpp"
#include "wii/Archive.hpp"
#include <Constants.hpp>

namespace devtools
{
namespace
{
using Bytes = std::vector<std::uint8_t>;
namespace fs = std::filesystem;

// ------------------------------------------------------------------------------------------------ big-endian fields

std::uint32_t Rd32(const Bytes& b, int o) { return (std::uint32_t)b[o] << 24 | (std::uint32_t)b[o + 1] << 16 | (std::uint32_t)b[o + 2] << 8 | b[o + 3]; }
void Wr32(Bytes& b, int o, std::uint32_t v) { b[o] = (std::uint8_t)(v >> 24); b[o + 1] = (std::uint8_t)(v >> 16); b[o + 2] = (std::uint8_t)(v >> 8); b[o + 3] = (std::uint8_t)v; }
float RdF(const Bytes& b, int o) { std::uint32_t u = Rd32(b, o); float f; std::memcpy(&f, &u, 4); return f; }
void WrF(Bytes& b, int o, float f) { std::uint32_t u; std::memcpy(&u, &f, 4); Wr32(b, o, u); }
int RdI(const Bytes& b, int o) { return (int)Rd32(b, o); }
void WrI(Bytes& b, int o, int v) { Wr32(b, o, (std::uint32_t)v); }

// ------------------------------------------------------------------------------------------------ record kinds (savedata.py)

enum Kind { K_ITEM, K_BOX, K_GATE, K_START, K_GOAL, K_POINT, K_FALL, K_WATER, K_WIND, K_CAM, K_GAREA, K_MOVE, K_GHOST, K_ROUTE, K_RIDER, K_COUNT };
enum FieldType { F_U32, F_I32, F_U8, F_VEC3, F_FIX3, F_INT3 };
enum PosType { P_VEC3, P_FIX3, P_CORNERS };

const char* const ITEM_NAMES[] = {"COOKIE", "COOKIE_POW", "CHOCO_KNIGHT", "CHOCO_WING", "CHOCO_PACN", "APPLE_L", "APPLE_S", "GRAPE_L", "GRAPE_S", "BANANA_L",
                                  "BANANA_S", "HAMBURGER_L", "HAMBURGER_S", "PARFAIT_L", "PARFAIT_S", "MEAT_L", "MEAT_S", "PIZZA", "SFLAG"};
const char* const BOX_NAMES[] = {"WOOD_L", "WOOD_S", "IRON_L", "IRON_S"};
const char* const GHOST_NAMES[] = {"Blinky", "Pinky", "Clyde", "Inky"};
const char* const RIDE_NAMES[] = {"on foot", "car", "ufo", "boat", "plane", "hover"};
const char* const AREA_KINDS[] = {"triangle", "quad"};
const char* const NO_YES[] = {"no", "yes"};
// box treasure: 0 nothing, 1..4 = 1 / 3 / 6 / 9 cookies, 5..22 = item kind t - 4
const std::vector<std::string>& TreasureNames()
{
	static std::vector<std::string> v;
	if (v.empty())
	{
		v = {"nothing", "1 cookie", "3 cookies", "6 cookies", "9 cookies"};
		for (int t = 5; t <= 22; t++) v.push_back(ITEM_NAMES[t - 4]);
	}
	return v;
}

struct Field
{
	const char* label;
	int off;
	FieldType type;
	const char* const* names;   // a Choice when the value is one of them
	int nNames;
	bool treasure;              // the names are TreasureNames()
};

struct KindDef
{
	const char* name;
	const char* plural;
	int size;
	PosType pos;
	int posOff;
	int rotY;                   // I32 degrees, -1 = none
	unsigned rgba;
	float lift;                 // height above the ground a new one is placed at
	std::vector<Field> fields;
};

#define NAMES(a) a, (int)(sizeof a / sizeof a[0]), false
const KindDef KINDS[K_COUNT] = {
	{"item", "items", 48, P_VEC3, 16, -1, 0xffd040ff, 0.5f, {{"kind", 0, F_U32, NAMES(ITEM_NAMES)}, {"flag", 4, F_U32, nullptr, 0, false}, {"position", 16, F_VEC3, nullptr, 0, false}}},
	{"box", "boxes", 48, P_VEC3, 16, 32, 0xb07840ff, 0.0f,
	 {{"kind", 0, F_U32, NAMES(BOX_NAMES)}, {"treasure", 4, F_U32, nullptr, 23, true}, {"position", 16, F_VEC3, nullptr, 0, false}, {"turn (deg)", 32, F_I32, nullptr, 0, false}}},
	{"gate", "gates", 48, P_VEC3, 16, 32, 0xff60c0ff, 0.0f,
	 {{"cookies", 0, F_U32, nullptr, 0, false}, {"position", 16, F_VEC3, nullptr, 0, false}, {"turn (deg)", 32, F_I32, nullptr, 0, false}, {"kind (0..2)", 36, F_U32, nullptr, 0, false}, {"no", 40, F_U32, nullptr, 0, false}}},
	{"start", "starts", 48, P_VEC3, 16, 32, 0x40ff40ff, 0.35f, {{"drop in", 0, F_U8, NAMES(NO_YES)}, {"position", 16, F_VEC3, nullptr, 0, false}, {"turn (deg)", 32, F_I32, nullptr, 0, false}}},
	{"goal", "goals", 32, P_VEC3, 0, 16, 0xff4040ff, 0.0f, {{"position", 0, F_VEC3, nullptr, 0, false}, {"turn (deg)", 16, F_I32, nullptr, 0, false}}},
	{"checkpoint", "checkpoints", 32, P_VEC3, 0, 16, 0x40e0ffff, 0.0f, {{"position", 0, F_VEC3, nullptr, 0, false}, {"turn (deg)", 16, F_I32, nullptr, 0, false}}},
	{"fall area", "fall areas", 64, P_FIX3, 0, -1, 0xff3030ff, 0.0f,
	 {{"centre", 0, F_FIX3, nullptr, 0, false}, {"turn (deg)", 12, F_INT3, nullptr, 0, false}, {"size", 24, F_FIX3, nullptr, 0, false}, {"check kind", 36, F_U32, nullptr, 0, false}}},
	{"water", "waters", 64, P_FIX3, 0, -1, 0x3080ffff, 0.0f,
	 {{"centre", 0, F_FIX3, nullptr, 0, false}, {"turn (deg)", 12, F_INT3, nullptr, 0, false}, {"size", 24, F_FIX3, nullptr, 0, false}, {"flow (deg)", 36, F_INT3, nullptr, 0, false}, {"force", 48, F_I32, nullptr, 0, false}}},
	{"wind", "winds", 64, P_FIX3, 0, -1, 0xc0c0ffff, 0.0f,
	 {{"centre", 0, F_FIX3, nullptr, 0, false}, {"turn (deg)", 12, F_INT3, nullptr, 0, false}, {"size", 24, F_FIX3, nullptr, 0, false}, {"blow (deg)", 36, F_INT3, nullptr, 0, false}, {"force", 48, F_I32, nullptr, 0, false}}},
	{"camera area", "camera areas", 84, P_FIX3, 0, -1, 0xff9020ff, 0.0f,
	 {{"centre", 0, F_FIX3, nullptr, 0, false}, {"turn (deg)", 12, F_INT3, nullptr, 0, false}, {"size", 24, F_FIX3, nullptr, 0, false}, {"area no", 36, F_I32, nullptr, 0, false}}},
	{"ghost area", "ghost areas", 80, P_CORNERS, 16, -1, 0xa060ffff, 0.0f,
	 {{"shape", 0, F_U32, NAMES(AREA_KINDS)}, {"corner 1", 16, F_VEC3, nullptr, 0, false}, {"corner 2", 32, F_VEC3, nullptr, 0, false}, {"corner 3", 48, F_VEC3, nullptr, 0, false}, {"corner 4", 64, F_VEC3, nullptr, 0, false}}},
	{"move point", "move points", 16, P_VEC3, 0, -1, 0xc090ffff, 0.0f, {{"position", 0, F_VEC3, nullptr, 0, false}}},
	{"ghost", "ghosts", 32, P_VEC3, 0, -1, 0xff80ffff, 0.0f,
	 {{"position", 0, F_VEC3, nullptr, 0, false}, {"ghost", 16, F_I32, NAMES(GHOST_NAMES)}, {"option item", 20, F_I32, nullptr, 0, false}, {"ride", 24, F_I32, NAMES(RIDE_NAMES)}}},
	{"route point", "route points", 32, P_VEC3, 16, -1, 0x80ff80ff, 0.0f, {{"speed", 0, F_I32, nullptr, 0, false}, {"position", 16, F_VEC3, nullptr, 0, false}}},
	{"route rider", "route riders", 32, P_VEC3, 0, -1, 0xff80ffff, 0.0f,
	 {{"position", 0, F_VEC3, nullptr, 0, false}, {"ghost", 16, F_I32, NAMES(GHOST_NAMES)}, {"option item", 20, F_I32, nullptr, 0, false}, {"ride", 24, F_I32, NAMES(RIDE_NAMES)}}},
};
#undef NAMES

Bytes NewRecord(Kind k)
{
	Bytes r(KINDS[k].size, 0);
	if (k == K_GAREA)
		for (int c = 0; c < 4; c++) WrF(r, 16 + 16 * c + 12, 1.0f);
	else if (KINDS[k].pos == P_VEC3)
		WrF(r, KINDS[k].posOff + 12, 1.0f);   // Vector4 w
	return r;
}

int Corners(const Bytes& r) { return Rd32(r, 0) == 0 ? 3 : 4; }

void GetPos(Kind k, const Bytes& r, float p[3])
{
	const KindDef& d = KINDS[k];
	if (d.pos == P_VEC3)
		for (int i = 0; i < 3; i++) p[i] = RdF(r, d.posOff + 4 * i);
	else if (d.pos == P_FIX3)
		for (int i = 0; i < 3; i++) p[i] = (float)RdI(r, d.posOff + 4 * i) / 1000.0f;
	else
	{
		const int n = Corners(r);
		p[0] = p[1] = p[2] = 0.0f;
		for (int c = 0; c < n; c++)
			for (int i = 0; i < 3; i++) p[i] += RdF(r, 16 + 16 * c + 4 * i) / (float)n;
	}
}

void SetPos(Kind k, Bytes& r, const float p[3])
{
	const KindDef& d = KINDS[k];
	if (d.pos == P_VEC3)
		for (int i = 0; i < 3; i++) WrF(r, d.posOff + 4 * i, p[i]);
	else if (d.pos == P_FIX3)
		for (int i = 0; i < 3; i++) WrI(r, d.posOff + 4 * i, (int)std::lround(p[i] * 1000.0f));
	else
	{
		float c0[3];
		GetPos(k, r, c0);
		for (int c = 0; c < 4; c++)
			for (int i = 0; i < 3; i++) WrF(r, 16 + 16 * c + 4 * i, RdF(r, 16 + 16 * c + 4 * i) + p[i] - c0[i]);
	}
}

bool HasTurn(Kind k) { return KINDS[k].rotY >= 0 || KINDS[k].pos == P_FIX3; }
void Turn(Kind k, Bytes& r, int deg)
{
	const int off = KINDS[k].rotY >= 0 ? KINDS[k].rotY : (KINDS[k].pos == P_FIX3 ? 16 : -1);   // areas: rotate.y
	if (off < 0) return;
	WrI(r, off, ((RdI(r, off) + deg) % 360 + 360) % 360);
}

// ------------------------------------------------------------------------------------------------ files

constexpr int AREA_FILES = 8, ROUTE_FILES = 6;
constexpr int AREA_SIZE = 16 + 48 * 80 + 64 * 16 + 8 * 32, ROUTE_SIZE = 16 + 32 * 32 + 32;
enum Layout { L_FLAT, L_AREA, L_ROUTE };

struct ListLay { Kind kind; int off, max, countOff; };   // countOff -1 = one fixed record (the route rider)
const ListLay AREA_LISTS[3] = {{K_GAREA, 16, 48, 0}, {K_MOVE, 16 + 48 * 80, 64, 4}, {K_GHOST, 16 + 48 * 80 + 64 * 16, 8, 8}};
const ListLay ROUTE_LISTS[2] = {{K_ROUTE, 16, 32, 0}, {K_RIDER, 16 + 32 * 32, 1, -1}};

struct File
{
	std::string key;
	Layout layout{L_FLAT};
	Kind flat{K_ITEM};
	Bytes base;                 // the bytes it was read from (padding / unused slots come from here)
	bool exists{false};
	std::vector<Bytes> recs[3];

	int Lists() const { return layout == L_FLAT ? 1 : (layout == L_AREA ? 3 : 2); }
	Kind KindOf(int list) const { return layout == L_FLAT ? flat : (layout == L_AREA ? AREA_LISTS[list].kind : ROUTE_LISTS[list].kind); }
	int Max(int list) const { return layout == L_FLAT ? (flat == K_START || flat == K_GOAL ? 1 : 100000) : (layout == L_AREA ? AREA_LISTS[list].max : ROUTE_LISTS[list].max); }
	bool Used() const
	{
		if (exists) return true;
		for (int l = 0; l < Lists(); l++)
			if (!recs[l].empty()) return true;
		return false;
	}
};

struct Data
{
	int round{-1};
	std::vector<File> files;
};

void Pad(Bytes& body, const Bytes& tail)
{
	const std::size_t need = (32 - body.size() % 32) % 32;
	if (tail.size() == need)
	{
		body.insert(body.end(), tail.begin(), tail.end());
		return;
	}
	static const char fill[] = "LTEJLTEJ";
	for (std::size_t i = 0; i < need; i++) body.push_back(i < 8 ? (std::uint8_t)fill[i] : 0);
}

void Parse(File& f, const Bytes& data)
{
	f.base = data;
	for (auto& l : f.recs) l.clear();
	if (f.layout == L_FLAT)
	{
		const int size = KINDS[f.flat].size;
		for (std::size_t p = 0; p + size <= data.size(); p += size) f.recs[0].emplace_back(data.begin() + p, data.begin() + p + size);
		return;
	}
	const int total = f.layout == L_AREA ? AREA_SIZE : ROUTE_SIZE;
	Bytes d = data;
	d.resize(std::max<std::size_t>(d.size(), total), 0);
	for (int l = 0; l < f.Lists(); l++)
	{
		const ListLay& lay = f.layout == L_AREA ? AREA_LISTS[l] : ROUTE_LISTS[l];
		const int size = KINDS[lay.kind].size;
		const int n = lay.countOff < 0 ? (data.empty() ? 0 : 1) : std::clamp(RdI(d, lay.countOff), 0, lay.max);
		for (int i = 0; i < n; i++) f.recs[l].emplace_back(d.begin() + lay.off + i * size, d.begin() + lay.off + (i + 1) * size);
	}
}

Bytes Serialize(const File& f)
{
	Bytes out;
	if (f.layout == L_FLAT)
	{
		for (const Bytes& r : f.recs[0]) out.insert(out.end(), r.begin(), r.end());
		const std::size_t size = KINDS[f.flat].size, n0 = f.base.size() / size;
		Pad(out, Bytes(f.base.begin() + n0 * size, f.base.end()));
		return out;
	}
	const int total = f.layout == L_AREA ? AREA_SIZE : ROUTE_SIZE;
	out.assign(f.base.begin(), f.base.begin() + std::min<std::size_t>(f.base.size(), total));
	out.resize(total, 0);
	for (int l = 0; l < f.Lists(); l++)
	{
		const ListLay& lay = f.layout == L_AREA ? AREA_LISTS[l] : ROUTE_LISTS[l];
		const int size = KINDS[lay.kind].size;
		if (lay.countOff >= 0) WrI(out, lay.countOff, (int)f.recs[l].size());
		for (std::size_t i = 0; i < f.recs[l].size() && (int)i < lay.max; i++) std::copy(f.recs[l][i].begin(), f.recs[l][i].end(), out.begin() + lay.off + i * size);
	}
	Pad(out, f.base.size() > (std::size_t)total ? Bytes(f.base.begin() + total, f.base.end()) : Bytes());
	return out;
}

// ------------------------------------------------------------------------------------------------ editor state

using kingdom::game::g03::cG03PacnRollSys;
struct Ref
{
	int file, list, rec;
	bool operator==(const Ref& o) const { return file == o.file && list == o.list && rec == o.rec; }
};

enum Tool { T_SELECT, T_ITEM, T_BOX, T_GATE, T_POINT, T_START, T_GOAL, T_FALL, T_WATER, T_WIND, T_CAM, T_GAREA, T_MOVE, T_GHOST, T_ROUTE, T_TEST, T_COUNT };
const char* const TOOL_NAMES[T_COUNT] = {"Select", "Item", "Box", "Gate", "Checkpoint", "Start", "Goal", "Fall area", "Water", "Wind", "Camera area",
                                         "Ghost area", "Move point", "Ghost", "Route point", "Test start"};
const Kind TOOL_KIND[T_COUNT] = {K_COUNT, K_ITEM, K_BOX, K_GATE, K_POINT, K_START, K_GOAL, K_FALL, K_WATER, K_WIND, K_CAM, K_GAREA, K_MOVE, K_GHOST, K_ROUTE, K_COUNT};

Data g_data;
std::vector<Data> g_undo, g_redo;
std::vector<Ref> g_sel;
std::map<std::string, Bytes> g_applied;   // the bytes SearchDataEx hands out for g_appliedRound
int g_appliedRound = -1;
bool g_dirty = false;
std::string g_msg;
int g_panelFrame = -100;
int g_page = 0;                           // Tools / Selection / Show / Delete / Check
Tool g_tool = T_SELECT;
int g_addItem = 0, g_addBox = 0, g_addTreasure = 0, g_addGate = 0, g_addGhost = 0, g_addRide = 0, g_addAreaKind = 1;
int g_areaFile = 0, g_routeFile = 0;
bool g_editView = true, g_snapGround = false, g_throughWalls = true, g_hideHud = false, g_noCull = true;
// the test start: editor only, never written to the stage files
bool g_testOn = false;
float g_test[3]{};
constexpr float START_LIFT = 0.35f;   // Pac-Man's centre above the floor (editor.py START_LIFT)
float g_grid = 1.0f;
bool g_show[K_COUNT];
int g_filter = 0;
Ref g_lastEdit{-1, -1, -1};
int g_lastEditFrame = -100;
// the orbit camera
bool g_camOn = false, g_camInit = false;
float g_yaw = 0, g_pitch = 0.6f, g_dist = 30, g_target[3]{}, g_gameEye[3]{}, g_gameTarget[3]{};
// the drag
bool g_dragging = false, g_dragMoved = false;
float g_dragY = 0, g_dragStart[3]{}, g_dragStartMy = 0;
std::vector<std::array<float, 3>> g_dragOrig;
// a left press that is not on a handle: a click (deselect / place, on release) unless the mouse moves, then it pans like the middle button (trackpads)
bool g_leftPending = false, g_leftPanning = false;
float g_leftX = 0, g_leftY = 0;

bool InStage()
{
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	return core != nullptr && core->GetJobNo() == kingdom::job::PACNROLL_GAME && KingdomGlobal::PacnRollSys != nullptr;
}
bool Active() { return InStage() && FrameNo() - g_panelFrame <= 2; }   // the Editor tab is drawn
int CurrentRound() { return (int)KingdomGlobal::PacnRollSys->nRound(); }

std::string RoundName(int r)
{
	cG03PacnRollSys* sys = KingdomGlobal::PacnRollSys;
	char buf[64];
	const int w = (int)sys->nWorld((unsigned)r) + 1, s = (int)sys->nStage((unsigned)r) + 1;
	if (cG03PacnRollSys::IsBossStage((unsigned)r)) std::snprintf(buf, sizeof buf, "round %d: world %d boss", r, w);
	else std::snprintf(buf, sizeof buf, "round %d: world %d stage %d", r, w, s);
	return buf;
}

std::string CamKey(int r)
{
	cG03PacnRollSys* sys = KingdomGlobal::PacnRollSys;   // cTaskCameraSet::LoadData: set<world + 1><stage + 1>.cam
	return "set" + std::to_string(sys->nWorld((unsigned)r) + 1) + std::to_string(sys->nStage((unsigned)r) + 1) + ".cam";
}

// the stage's own data from the attached archives (not the edited copy)
void LoadStage(int round)
{
	Data d;
	d.round = round;
	auto add = [&](const std::string& key, Layout lay, Kind flat) {
		File f;
		f.key = key;
		f.layout = lay;
		f.flat = flat;
		std::size_t size = 0;
		const std::uint8_t* p = wii::ArchiveStore::Find(key, size);
		f.exists = p != nullptr;
		Parse(f, p != nullptr ? Bytes(p, p + size) : Bytes());
		d.files.push_back(std::move(f));
	};
	add("BIN_ITEM_SET_DAT", L_FLAT, K_ITEM);
	add("BIN_BOX_SET_DAT", L_FLAT, K_BOX);
	add("BIN_GATE_SET_DAT", L_FLAT, K_GATE);
	add("BIN_START_SET_DAT", L_FLAT, K_START);
	add("BIN_GOAL_SET_DAT", L_FLAT, K_GOAL);
	add("BIN_POINT_SET_DAT", L_FLAT, K_POINT);
	add("BIN_FALL_AREA_SET_DAT", L_FLAT, K_FALL);
	add("BIN_WATER_SET_DAT", L_FLAT, K_WATER);
	add("BIN_WIND_SET_DAT", L_FLAT, K_WIND);
	add(CamKey(round), L_FLAT, K_CAM);
	for (int i = 0; i < AREA_FILES; i++) add("BIN_GHOST_AREA_" + std::to_string(i) + "_SET_DAT", L_AREA, K_COUNT);
	for (int i = 0; i < ROUTE_FILES; i++) add("BIN_GHOST_ROUTE_" + std::to_string(i) + "_SET_DAT", L_ROUTE, K_COUNT);
	g_data = std::move(d);
	g_undo.clear();
	g_redo.clear();
	g_sel.clear();
	g_dirty = false;
}

File* FlatFile(Kind k)
{
	for (File& f : g_data.files)
		if (f.layout == L_FLAT && f.flat == k) return &f;
	return nullptr;
}
File& AreaFile(int i) { return g_data.files[10 + i]; }
File& RouteFile(int i) { return g_data.files[10 + AREA_FILES + i]; }

Kind KindOf(const Ref& r) { return g_data.files[r.file].KindOf(r.list); }
Bytes& Rec(const Ref& r) { return g_data.files[r.file].recs[r.list][r.rec]; }
bool Valid(const Ref& r)
{
	return r.file >= 0 && r.file < (int)g_data.files.size() && r.list >= 0 && r.list < 3 && r.rec >= 0 && r.rec < (int)g_data.files[r.file].recs[r.list].size();
}
std::vector<Ref> Things()
{
	std::vector<Ref> out;
	for (int f = 0; f < (int)g_data.files.size(); f++)
		for (int l = 0; l < g_data.files[f].Lists(); l++)
			for (int i = 0; i < (int)g_data.files[f].recs[l].size(); i++) out.push_back(Ref{f, l, i});
	return out;
}
bool Selected(const Ref& r) { return std::find(g_sel.begin(), g_sel.end(), r) != g_sel.end(); }

std::string Describe(const Ref& r)
{
	const Kind k = KindOf(r);
	const Bytes& b = Rec(r);
	std::string s = KINDS[k].name;
	const File& f = g_data.files[r.file];
	if (f.layout == L_AREA) s += " (area file " + std::to_string(r.file - 10) + ")";
	if (f.layout == L_ROUTE) s += " (route " + std::to_string(r.file - 10 - AREA_FILES) + ")";
	s += " " + std::to_string(r.rec);
	if (k == K_ITEM && Rd32(b, 0) < 19) s += std::string(": ") + ITEM_NAMES[Rd32(b, 0)];
	if (k == K_BOX && Rd32(b, 0) < 4) s += std::string(": ") + BOX_NAMES[Rd32(b, 0)];
	if (k == K_GATE) s += ": " + std::to_string(Rd32(b, 0)) + " cookies";
	if ((k == K_GHOST || k == K_RIDER) && (unsigned)RdI(b, 16) < 4) s += std::string(": ") + GHOST_NAMES[RdI(b, 16)];
	if (k == K_CAM) s += ": area " + std::to_string(RdI(b, 36));
	float p[3];
	GetPos(k, b, p);
	char buf[64];
	std::snprintf(buf, sizeof buf, "  (%.1f, %.1f, %.1f)", p[0], p[1], p[2]);
	return s + buf;
}

void PushUndo(const Ref* coalesce = nullptr)
{
	if (coalesce != nullptr && *coalesce == g_lastEdit && FrameNo() - g_lastEditFrame < 30)
	{
		g_lastEditFrame = FrameNo();
		return;
	}
	g_lastEdit = coalesce != nullptr ? *coalesce : Ref{-1, -1, -1};
	g_lastEditFrame = FrameNo();
	g_undo.push_back(g_data);
	if (g_undo.size() > 200) g_undo.erase(g_undo.begin());
	g_redo.clear();
	g_dirty = true;
}
void Undo()
{
	if (g_undo.empty()) return;
	g_redo.push_back(std::move(g_data));
	g_data = std::move(g_undo.back());
	g_undo.pop_back();
	g_sel.erase(std::remove_if(g_sel.begin(), g_sel.end(), [](const Ref& r) { return !Valid(r); }), g_sel.end());
	g_lastEdit = Ref{-1, -1, -1};
	g_dirty = true;
}
void Redo()
{
	if (g_redo.empty()) return;
	g_undo.push_back(std::move(g_data));
	g_data = std::move(g_redo.back());
	g_redo.pop_back();
	g_sel.erase(std::remove_if(g_sel.begin(), g_sel.end(), [](const Ref& r) { return !Valid(r); }), g_sel.end());
	g_lastEdit = Ref{-1, -1, -1};
	g_dirty = true;
}

void DeleteRefs(std::vector<Ref> refs)
{
	refs.erase(std::remove_if(refs.begin(), refs.end(), [](const Ref& r) { return !Valid(r) || KindOf(r) == K_RIDER; }), refs.end());
	if (refs.empty()) return;
	PushUndo();
	std::sort(refs.begin(), refs.end(), [](const Ref& a, const Ref& b) { return a.file != b.file ? a.file < b.file : (a.list != b.list ? a.list < b.list : a.rec > b.rec); });
	for (const Ref& r : refs)
	{
		auto& l = g_data.files[r.file].recs[r.list];
		l.erase(l.begin() + r.rec);
	}
	g_sel.clear();
	g_msg = std::to_string(refs.size()) + " deleted";
}

void Duplicate()
{
	std::vector<Ref> made;
	bool pushed = false;
	for (const Ref& r : g_sel)
	{
		if (!Valid(r)) continue;
		File& f = g_data.files[r.file];
		if ((int)f.recs[r.list].size() >= f.Max(r.list)) continue;   // start / goal / rider / full ghost lists
		if (!pushed) { PushUndo(); pushed = true; }
		Bytes copy = f.recs[r.list][r.rec];
		float p[3];
		GetPos(f.KindOf(r.list), copy, p);
		p[0] += 1.0f;
		SetPos(f.KindOf(r.list), copy, p);
		f.recs[r.list].push_back(copy);
		made.push_back(Ref{r.file, r.list, (int)f.recs[r.list].size() - 1});
	}
	if (!made.empty()) g_sel = made;
	g_msg = made.empty() ? "nothing to duplicate (the start, goal and route riders are single)" : std::to_string(made.size()) + " duplicated";
}

// ------------------------------------------------------------------------------------------------ apply / save / load

void Apply()
{
	g_applied.clear();
	for (const File& f : g_data.files)
		if (f.Used()) g_applied[f.key] = Serialize(f);
	g_appliedRound = g_data.round;
	g_dirty = false;
	KingdomGlobal::PacnRollSys->nRound() = (unsigned)g_data.round;
	kingdom::Kingdom_SetJobNo(kingdom::job::PACNROLL_DEBUG_SELECT, false, false);   // the stage restarts and reads the edited data
	g_msg = "applied: the stage restarts with the edits";
}

void Revert()
{
	const int r = g_data.round;
	g_applied.clear();
	g_appliedRound = -1;
	LoadStage(r);
	KingdomGlobal::PacnRollSys->nRound() = (unsigned)r;
	kingdom::Kingdom_SetJobNo(kingdom::job::PACNROLL_DEBUG_SELECT, false, false);
	g_msg = "back to the disc's data";
}

void Restart()
{
	KingdomGlobal::PacnRollSys->nRound() = (unsigned)g_data.round;
	kingdom::Kingdom_SetJobNo(kingdom::job::PACNROLL_DEBUG_SELECT, false, false);   // applied edits stay (they are keyed by the round)
	g_msg = g_testOn ? "restarted at the test start" : "restarted";
}

fs::path EditDir(int round)
{
	fs::path root;
	if (const char* e = std::getenv("MEGAMIX_G03_EDITS"); e != nullptr && *e) root = e;
	else if (const char* x = std::getenv("XDG_DATA_HOME"); x != nullptr && *x) root = fs::path(x) / "megamix" / "g03_edits";
	else if (const char* h = std::getenv("HOME"); h != nullptr) root = fs::path(h) / ".local" / "share" / "megamix" / "g03_edits";
	else root = "g03_edits";
	char buf[16];
	std::snprintf(buf, sizeof buf, "round%02d", round);
	return root / buf;
}

void Save()
{
	const fs::path dir = EditDir(g_data.round);
	std::error_code ec;
	fs::create_directories(dir, ec);
	int n = 0;
	for (const File& f : g_data.files)
	{
		if (!f.Used()) continue;
		const Bytes b = Serialize(f);
		std::ofstream out(dir / f.key, std::ios::binary);
		out.write((const char*)b.data(), (std::streamsize)b.size());
		n += out.good() ? 1 : 0;
	}
	g_msg = "saved " + std::to_string(n) + " files to " + dir.string();
}

void Load()
{
	const fs::path dir = EditDir(g_data.round);
	int n = 0;
	bool pushed = false;
	for (File& f : g_data.files)
	{
		std::ifstream in(dir / f.key, std::ios::binary);
		if (!in) continue;
		if (!pushed) { PushUndo(); pushed = true; }
		Bytes b((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		f.exists = true;
		Parse(f, b);
		n++;
	}
	g_sel.clear();
	g_msg = n == 0 ? "nothing saved in " + dir.string() : "loaded " + std::to_string(n) + " files: Apply to play them";
}

// ------------------------------------------------------------------------------------------------ the view: matrices, rays, the stage's faces

// the nearest collision face turned towards the ray, camera-only faces skipped (what Pac-Man rolls on: cTaskCollision's objects, world = pMtx * vertex)
bool RayStage(const float o[3], const float d[3], float& tBest)
{
	using kingdom::game::g03::cTaskBgHit;
	using kingdom::game::g03::cFaceList;
	kingdom::game::g03::cAppG03PacnRollGame* app = kingdom::game::g03::Constants::PACN_APP();
	if (app == nullptr || app->pTaskCollision() == nullptr) return false;
	bool hit = false;
	tBest = 1e30f;
	for (cTaskBgHit* obj : app->pTaskCollision()->pVecObj())
	{
		cFaceList* list = obj->pFaceList();
		if (list == nullptr) continue;
		for (int n = 0; n < 4; n++)
			for (unsigned i = 0; i < list->nFaceNum(n); i++)
			{
				kingdom::game::g03::Constants::sFace* f = list->pFace(n, (int)i);
				if (f->nKind == kingdom::game::g03::Constants::eFACE_KIND_CAME) continue;   // camera-only faces
				{
					// one-sided: only faces turned towards the ray (their normal, rotated like devtools_g03's damaging-wall test)
					Vector4 nrm;
					::Constants::jetGeoVecTrans(nrm, obj->pMtxRotate(), f->vNorm);
					if (nrm[0] * d[0] + nrm[1] * d[1] + nrm[2] * d[2] >= 0.0f) continue;
				}
				float v[3][3];
				for (int k = 0; k < 3; k++)
				{
					Vector4 p;
					::Constants::jetGeoVecTrans(p, obj->pMtx(), f->vVrt[k]);
					v[k][0] = p[0]; v[k][1] = p[1]; v[k][2] = p[2];
				}
				// Moller-Trumbore, both sides
				float e1[3], e2[3], s[3], h[3], q[3];
				for (int k = 0; k < 3; k++) { e1[k] = v[1][k] - v[0][k]; e2[k] = v[2][k] - v[0][k]; s[k] = o[k] - v[0][k]; }
				h[0] = d[1] * e2[2] - d[2] * e2[1]; h[1] = d[2] * e2[0] - d[0] * e2[2]; h[2] = d[0] * e2[1] - d[1] * e2[0];
				const float a = e1[0] * h[0] + e1[1] * h[1] + e1[2] * h[2];
				if (std::fabs(a) < 1e-9f) continue;
				const float fa = 1.0f / a, u = fa * (s[0] * h[0] + s[1] * h[1] + s[2] * h[2]);
				if (u < 0.0f || u > 1.0f) continue;
				q[0] = s[1] * e1[2] - s[2] * e1[1]; q[1] = s[2] * e1[0] - s[0] * e1[2]; q[2] = s[0] * e1[1] - s[1] * e1[0];
				const float w = fa * (d[0] * q[0] + d[1] * q[1] + d[2] * q[2]);
				if (w < 0.0f || u + w > 1.0f) continue;
				const float t = fa * (e2[0] * q[0] + e2[1] * q[1] + e2[2] * q[2]);
				if (t > 1e-4f && t < tBest) { tBest = t; hit = true; }
			}
	}
	return hit;
}

bool RayPlaneY(const float o[3], const float d[3], float y, float out[3])
{
	if (std::fabs(d[1]) < 1e-6f) return false;
	const float t = (y - o[1]) / d[1];
	if (t <= 0.0f) return false;
	for (int i = 0; i < 3; i++) out[i] = o[i] + d[i] * t;
	return true;
}

float PacmanY()
{
	kingdom::game::g03::cAppG03PacnRollGame* app = kingdom::game::g03::Constants::PACN_APP();
	if (app != nullptr && app->pTaskCharPacman() != nullptr) return app->pTaskCharPacman()->vTrans()[1];
	return 0.0f;
}

// where a click lands: the stage under the mouse, else the plane at Pac-Man's height
bool Ground(const float o[3], const float d[3], float out[3])
{
	float t;
	if (RayStage(o, d, t))
	{
		for (int i = 0; i < 3; i++) out[i] = o[i] + d[i] * t;
		return true;
	}
	return RayPlaneY(o, d, PacmanY(), out);
}

float Snap(float v) { return g_grid > 0.0f ? std::round(v / g_grid) * g_grid : v; }

// ------------------------------------------------------------------------------------------------ adding

void Add(Tool tool, const float at[3])
{
	const Kind k = TOOL_KIND[tool];
	float p[3] = {at[0], at[1] + KINDS[k].lift, at[2]};
	File* f = nullptr;
	int list = 0;
	if (k == K_GAREA || k == K_MOVE || k == K_GHOST)
	{
		f = &AreaFile(g_areaFile);
		list = k == K_GAREA ? 0 : (k == K_MOVE ? 1 : 2);
	}
	else if (k == K_ROUTE)
		f = &RouteFile(g_routeFile);
	else
		f = FlatFile(k);
	if (f == nullptr) return;
	// the start and goal are single: placing one moves it
	if ((k == K_START || k == K_GOAL) && !f->recs[0].empty())
	{
		PushUndo();
		SetPos(k, f->recs[0][0], p);
		g_sel = {Ref{(int)(f - g_data.files.data()), 0, 0}};
		g_msg = std::string(KINDS[k].name) + " moved";
		return;
	}
	if ((int)f->recs[list].size() >= f->Max(list))
	{
		g_msg = std::string("this file holds ") + std::to_string(f->Max(list)) + " " + KINDS[k].plural;
		return;
	}
	PushUndo();
	Bytes r = NewRecord(k);
	switch (k)
	{
	case K_ITEM: Wr32(r, 0, (unsigned)g_addItem); break;
	case K_BOX: Wr32(r, 0, (unsigned)g_addBox); Wr32(r, 4, (unsigned)g_addTreasure); break;
	case K_GATE: Wr32(r, 0, 10); Wr32(r, 36, (unsigned)g_addGate); Wr32(r, 40, (unsigned)f->recs[0].size()); break;
	case K_FALL: case K_WATER: case K_WIND: case K_CAM:
		WrI(r, 24, 10000); WrI(r, 28, (k == K_FALL || k == K_WATER) ? 0 : 10000); WrI(r, 32, 10000);   // editor.py: 10 x (0 or 10) x 10
		if (k == K_CAM) WrI(r, 36, 1);
		break;
	case K_GAREA:
		Wr32(r, 0, (unsigned)g_addAreaKind);
		{
			const float off[4][2] = {{-3, -3}, {3, -3}, {3, 3}, {-3, 3}};
			for (int c = 0; c < 4; c++) { WrF(r, 16 + 16 * c, p[0] + off[c][0]); WrF(r, 16 + 16 * c + 4, p[1]); WrF(r, 16 + 16 * c + 8, p[2] + off[c][1]); }
		}
		break;
	case K_GHOST: WrI(r, 16, g_addGhost); WrI(r, 24, g_addRide); break;
	case K_ROUTE: WrI(r, 0, f->recs[0].empty() ? 60 : RdI(f->recs[0].back(), 0)); break;
	default: break;
	}
	if (k != K_GAREA) SetPos(k, r, p);
	if (k == K_ROUTE && f->recs[1].empty())
	{
		// a new route gets its rider (editor.py _new_route_file: ghost type 0)
		Bytes rider = NewRecord(K_RIDER);
		WrI(rider, 16, g_addGhost);
		WrI(rider, 24, g_addRide);
		SetPos(K_RIDER, rider, p);
		f->recs[1].push_back(rider);
	}
	f->recs[list].push_back(r);
	g_sel = {Ref{(int)(f - g_data.files.data()), list, (int)f->recs[list].size() - 1}};
	g_msg = std::string(KINDS[k].name) + " placed";
}

// ------------------------------------------------------------------------------------------------ the game view

Ref Pick(const float vp[16], float mx, float my)
{
	Ref best{-1, -1, -1};
	float bestD = 14.0f * 14.0f;
	for (const Ref& r : Things())
	{
		const Kind k = KindOf(r);
		if (!g_show[k]) continue;
		float p[3], sx, sy;
		GetPos(k, Rec(r), p);
		if (!Project(vp, p, sx, sy)) continue;
		const float dd = (sx - mx) * (sx - mx) + (sy - my) * (sy - my);
		if (dd < bestD) { bestD = dd; best = r; }
	}
	return best;
}

void ViewInput()
{
	const devui::Input in = devui::GetInput();
	const devui::Rect gv = devui::GameView();
	const bool over = in.mx >= gv.x && in.mx < gv.x + gv.w && in.my >= gv.y && in.my < gv.y + gv.h;   // the panel lies outside GameView
	if (!g_editView) { g_dragging = false; return; }
	if (over || g_dragging || g_leftPending) devui::CaptureInput(true, true);
	float vp[16];
	if (!ViewProj(vp)) return;

	// the orbit camera: right drag turns, middle drag (or a left drag off the handles) pans, the wheel zooms
	if (g_leftPending && in.down[0] && g_camOn && std::fabs(in.mx - g_leftX) + std::fabs(in.my - g_leftY) > 4.0f) g_leftPanning = true;
	if (g_camOn && (over || g_leftPanning))
	{
		if (in.down[1]) { g_yaw -= in.dx * 0.01f; g_pitch = std::clamp(g_pitch + in.dy * 0.01f, -1.5f, 1.5f); }
		if (in.down[2] || (g_leftPanning && in.down[0]))
		{
			const float s = g_dist * 0.0025f, cy = std::cos(g_yaw), sy = std::sin(g_yaw);
			g_target[0] -= (in.dx * cy) * s;
			g_target[2] -= (-in.dx * sy) * s;
			g_target[1] += in.dy * s;
		}
		if (in.wheel != 0.0f) g_dist = std::clamp(g_dist * std::pow(0.88f, in.wheel), 1.0f, 2000.0f);
	}

	float o[3], d[3];
	const bool ray = MouseRay(vp, in.mx, in.my, o, d);
	if (over && in.pressed[0] && ray)
	{
		if (g_tool == T_SELECT)
		{
			const Ref r = Pick(vp, in.mx, in.my);
			if (r.file < 0)
			{
				g_leftPending = true;   // deselect on release, or pan
				g_leftPanning = false;
				g_leftX = in.mx;
				g_leftY = in.my;
			}
			else
			{
				if (in.shift)
				{
					auto it = std::find(g_sel.begin(), g_sel.end(), r);
					if (it != g_sel.end()) g_sel.erase(it);
					else g_sel.push_back(r);
				}
				else if (!Selected(r))
					g_sel = {r};
				if (Selected(r))
				{
					float p[3];
					GetPos(KindOf(r), Rec(r), p);
					g_dragY = p[1];
					g_dragging = RayPlaneY(o, d, g_dragY, g_dragStart);
					g_dragMoved = false;
					g_dragStartMy = in.my;
					g_dragOrig.clear();
					for (const Ref& s : g_sel)
					{
						float q[3];
						GetPos(KindOf(s), Rec(s), q);
						g_dragOrig.push_back({q[0], q[1], q[2]});
					}
				}
			}
		}
		else
		{
			g_leftPending = true;   // place on release, or pan
			g_leftPanning = false;
			g_leftX = in.mx;
			g_leftY = in.my;
		}
	}
	if (g_leftPending && !in.down[0])
	{
		g_leftPending = false;
		if (!g_leftPanning && over && ray && g_tool == T_SELECT)
		{
			if (!in.shift) g_sel.clear();
		}
		else if (!g_leftPanning && over && ray)
		{
			float at[3];
			if (Ground(o, d, at))
			{
				if (in.ctrl) { at[0] = Snap(at[0]); at[2] = Snap(at[2]); }
				if (g_tool == T_TEST)
				{
					g_test[0] = at[0]; g_test[1] = at[1] + START_LIFT; g_test[2] = at[2];
					g_testOn = true;
					g_msg = "test start set: Restart to roll from there";
				}
				else
					Add(g_tool, at);
			}
		}
	}
	if (g_dragging)
	{
		if (!in.down[0]) g_dragging = false;
		else if (ray && g_dragOrig.size() == g_sel.size())
		{
			float p[3], delta[3] = {0, 0, 0};
			if (in.alt)
			{
				// Alt: up / down by the mouse's height on the screen (world units per pixel at the thing's distance)
				float v[16], pr[16];
				kingdom::NativeScene::GetMatrices(v, pr);
				float dist = 0;
				for (int i = 0; i < 3; i++) dist += (g_dragOrig[0][i] - o[i]) * (g_dragOrig[0][i] - o[i]);
				delta[1] = (g_dragStartMy - in.my) * 2.0f * std::sqrt(dist) / (pr[5] * gv.h);
			}
			else if (RayPlaneY(o, d, g_dragY, p))
			{
				delta[0] = p[0] - g_dragStart[0];
				delta[2] = p[2] - g_dragStart[2];
			}
			if (!g_dragMoved && (delta[0] != 0 || delta[1] != 0 || delta[2] != 0)) { PushUndo(); g_dragMoved = true; }
			if (g_dragMoved)
				for (std::size_t i = 0; i < g_sel.size(); i++)
				{
					if (!Valid(g_sel[i])) continue;
					const Kind k = KindOf(g_sel[i]);
					float q[3] = {g_dragOrig[i][0] + delta[0], g_dragOrig[i][1] + delta[1], g_dragOrig[i][2] + delta[2]};
					if (in.ctrl) { q[0] = Snap(q[0]); q[2] = Snap(q[2]); }
					if (g_snapGround && !in.alt && g_sel.size() == 1)
					{
						// on the stage under the mouse (the thing's own lift above it)
						float t;
						if (RayStage(o, d, t))
						{
							for (int j = 0; j < 3; j++) q[j] = o[j] + d[j] * t;
							if (in.ctrl) { q[0] = Snap(q[0]); q[2] = Snap(q[2]); }
							q[1] += KINDS[k].lift;
						}
					}
					SetPos(k, Rec(g_sel[i]), q);
				}
		}
	}

	if (!over) return;
	// keys (editor.py): Delete / X / Backspace, Ctrl Z / Y / D / S, R turns 15 degrees (Shift back), Page Up / Down = up / down, Esc, Enter = apply
	if (devui::KeyPressed(devui::K_DELETE) || devui::KeyPressed(devui::K_BACKSPACE) || (!in.ctrl && devui::KeyPressed('X'))) DeleteRefs(g_sel);
	if (in.ctrl && devui::KeyPressed('Z')) { if (in.shift) Redo(); else Undo(); }
	if (in.ctrl && devui::KeyPressed('Y')) Redo();
	if (in.ctrl && devui::KeyPressed('D')) Duplicate();
	if (in.ctrl && devui::KeyPressed('S')) Save();
	if (devui::KeyPressed(devui::K_ESCAPE)) { g_sel.clear(); g_tool = T_SELECT; }
	if (devui::KeyPressed(devui::K_ENTER)) Apply();
	if (!in.ctrl && devui::KeyPressed('R') && !g_sel.empty())
	{
		PushUndo();
		for (const Ref& r : g_sel)
			if (Valid(r) && HasTurn(KindOf(r))) Turn(KindOf(r), Rec(r), in.shift ? -15 : 15);
	}
	const int updown = devui::KeyPressed(devui::K_PAGE_UP) ? 1 : (devui::KeyPressed(devui::K_PAGE_DOWN) ? -1 : 0);
	if (updown != 0 && !g_sel.empty())
	{
		PushUndo();
		for (const Ref& r : g_sel)
		{
			if (!Valid(r)) continue;
			float p[3];
			GetPos(KindOf(r), Rec(r), p);
			p[1] += updown * (in.shift ? 0.1f : 0.5f);
			SetPos(KindOf(r), Rec(r), p);
		}
	}
}

// ------------------------------------------------------------------------------------------------ handles (3D pass)

std::vector<gfx::LineVertex> g_lines;
void Line(const float p[3], const float q[3], unsigned rgba)
{
	gfx::LineVertex v{};
	for (int k = 0; k < 4; k++) v.rgba[k] = (std::uint8_t)(rgba >> (24 - 8 * k));
	for (int k = 0; k < 3; k++) v.pos[k] = p[k];
	g_lines.push_back(v);
	for (int k = 0; k < 3; k++) v.pos[k] = q[k];
	g_lines.push_back(v);
}
void Cross(const float p[3], float r, unsigned rgba)
{
	for (int a = 0; a < 3; a++)
	{
		float u[3] = {p[0], p[1], p[2]}, w[3] = {p[0], p[1], p[2]};
		u[a] -= r;
		w[a] += r;
		Line(u, w, rgba);
	}
}
// srt_matrix (unpac revolution/brres/pose.py): R = Rz * Ry * Rx, degrees
void Rotate(const float deg[3], const float v[3], float out[3])
{
	const float rx = deg[0] * 0.0174532925f, ry = deg[1] * 0.0174532925f, rz = deg[2] * 0.0174532925f;
	const float cx = std::cos(rx), sx = std::sin(rx), cy = std::cos(ry), sy = std::sin(ry), cz = std::cos(rz), sz = std::sin(rz);
	const float m[3][3] = {{cy * cz, sx * sy * cz - cx * sz, cx * sy * cz + sx * sz}, {cy * sz, sx * sy * sz + cx * cz, cx * sy * sz - sx * cz}, {-sy, sx * cy, cx * cy}};
	for (int r = 0; r < 3; r++) out[r] = m[r][0] * v[0] + m[r][1] * v[1] + m[r][2] * v[2];
}

void DrawThing(const Ref& r, bool sel)
{
	const Kind k = KindOf(r);
	const Bytes& b = Rec(r);
	const unsigned c = sel ? 0xffffffffu : KINDS[k].rgba;
	float p[3];
	GetPos(k, b, p);
	if (KINDS[k].pos == P_FIX3)
	{
		float rot[3], sc[3], corner[8][3];
		for (int i = 0; i < 3; i++) { rot[i] = (float)RdI(b, 12 + 4 * i); sc[i] = (float)RdI(b, 24 + 4 * i) / 1000.0f; }
		for (int i = 0; i < 8; i++)
		{
			const float l[3] = {(i & 4 ? 0.5f : -0.5f) * sc[0], (i & 2 ? 0.5f : -0.5f) * sc[1], (i & 1 ? 0.5f : -0.5f) * sc[2]};
			float w[3];
			Rotate(rot, l, w);
			for (int j = 0; j < 3; j++) corner[i][j] = p[j] + w[j];
		}
		static const int E[12][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
		for (const auto& e : E) Line(corner[e[0]], corner[e[1]], c);
		if (k == K_WATER || k == K_WIND)
		{
			// the flow / blow direction: the area's local +Z turned by its own angles (UNVERIFIED convention)
			float dir[3], fwd[3] = {0, 0, 2.0f}, q[3];
			const float fr[3] = {(float)RdI(b, 36), (float)RdI(b, 40), (float)RdI(b, 44)};
			Rotate(fr, fwd, dir);
			for (int j = 0; j < 3; j++) q[j] = p[j] + dir[j];
			Line(p, q, c);
		}
		Cross(p, sel ? 0.5f : 0.3f, c);
		return;
	}
	if (k == K_GAREA)
	{
		const int n = Corners(b);
		float q[4][3];
		for (int i = 0; i < n; i++)
			for (int j = 0; j < 3; j++) q[i][j] = RdF(b, 16 + 16 * i + 4 * j);
		for (int i = 0; i < n; i++) Line(q[i], q[(i + 1) % n], c);
		Cross(p, sel ? 0.5f : 0.3f, c);
		return;
	}
	Cross(p, sel ? 0.6f : 0.35f, c);
	if (KINDS[k].rotY >= 0)
	{
		// facing: +Z turned by rotY around Y (UNVERIFIED which way the models face)
		const float a = (float)RdI(b, KINDS[k].rotY) * 0.0174532925f;
		const float q[3] = {p[0] + std::sin(a) * 1.2f, p[1], p[2] + std::cos(a) * 1.2f};
		Line(p, q, c);
	}
}

// ------------------------------------------------------------------------------------------------ the panel

struct Problem { int level; std::string text; };
std::vector<Problem> Check()
{
	std::vector<Problem> out;
	if (FlatFile(K_START)->recs[0].empty()) out.push_back({2, "no start (BIN_START_SET_DAT is empty)"});
	if (FlatFile(K_GOAL)->recs[0].empty()) out.push_back({2, "no goal (BIN_GOAL_SET_DAT is empty)"});
	unsigned cookies = 0;
	for (const Bytes& r : FlatFile(K_ITEM)->recs[0]) cookies += Rd32(r, 0) <= 1 ? 1 : 0;
	const auto& gates = FlatFile(K_GATE)->recs[0];
	for (std::size_t i = 0; i < gates.size(); i++)
	{
		if (Rd32(gates[i], 36) > 2) out.push_back({2, "gate " + std::to_string(i) + ": kind " + std::to_string(Rd32(gates[i], 36)) + " (0 to 2)"});
		if (Rd32(gates[i], 0) > cookies) out.push_back({1, "gate " + std::to_string(i) + " needs " + std::to_string(Rd32(gates[i], 0)) + " cookies, the stage has " + std::to_string(cookies)});
	}
	const auto& items = FlatFile(K_ITEM)->recs[0];
	for (std::size_t i = 0; i < items.size(); i++)
		if (Rd32(items[i], 0) > 18) out.push_back({2, "item " + std::to_string(i) + ": kind " + std::to_string(Rd32(items[i], 0)) + " has no model (0 to 18)"});
	const auto& boxes = FlatFile(K_BOX)->recs[0];
	for (std::size_t i = 0; i < boxes.size(); i++)
	{
		if (Rd32(boxes[i], 0) > 3) out.push_back({2, "box " + std::to_string(i) + ": kind " + std::to_string(Rd32(boxes[i], 0)) + " (0 to 3)"});
		if (Rd32(boxes[i], 4) > 22) out.push_back({2, "box " + std::to_string(i) + ": treasure " + std::to_string(Rd32(boxes[i], 4)) + " (0 to 22)"});
	}
	for (int a = 0; a < AREA_FILES; a++)
	{
		const File& f = AreaFile(a);
		if (!f.recs[2].empty() && f.recs[0].empty()) out.push_back({1, "ghost area " + std::to_string(a) + ": ghosts but no areas to move in"});
		if (!f.recs[0].empty() && f.recs[1].size() < 2) out.push_back({1, "ghost area " + std::to_string(a) + ": needs move points inside its areas"});
		for (std::size_t e = 0; e < f.recs[2].size(); e++)
			if ((unsigned)RdI(f.recs[2][e], 16) > 3) out.push_back({2, "ghost area " + std::to_string(a) + " ghost " + std::to_string(e) + ": type is not 0 to 3"});
	}
	for (int r = 0; r < ROUTE_FILES; r++)
	{
		const File& f = RouteFile(r);
		if (!f.Used()) continue;
		if (f.recs[0].size() < 2) out.push_back({1, "route " + std::to_string(r) + ": fewer than two points"});
		if (!f.recs[1].empty() && (unsigned)RdI(f.recs[1][0], 24) > 5) out.push_back({2, "route " + std::to_string(r) + ": ride " + std::to_string(RdI(f.recs[1][0], 24)) + " (0 to 5)"});
	}
	if (FlatFile(K_FALL)->recs[0].empty()) out.push_back({1, "no fall area: Pac-Man cannot fall out of the stage"});
	std::stable_sort(out.begin(), out.end(), [](const Problem& a, const Problem& b) { return a.level > b.level; });
	return out;
}

// one record's fields (a change is written back as one undo step per field edit)
void Fields(devui::UI& ui, const Ref& ref)
{
	const Kind k = KindOf(ref);
	Bytes r = Rec(ref);
	const int idBase = ref.file * 4 + ref.list;
	int fi = 0;
	for (const Field& f : KINDS[k].fields)
	{
		const devui::Id id = devui::MakeId(f.label, idBase * 100 + fi++, ref.rec);
		switch (f.type)
		{
		case F_U32: case F_I32: case F_U8:
		{
			long v = f.type == F_U8 ? r[f.off] : (f.type == F_U32 ? (long)Rd32(r, f.off) : (long)RdI(r, f.off));
			std::vector<std::string> names;
			if (f.treasure) names = TreasureNames();
			else for (int i = 0; i < f.nNames; i++) names.push_back(f.names[i]);
			if (k == K_GAREA && f.off == 0) v = v == 0 ? 0 : 1;   // any non-zero kind is a quad
			if (!names.empty() && v >= 0 && v < (long)names.size())
				v = ui.Choice(ui.Row(), id, f.label, names, (int)v);
			else
				v = (long)ui.Number(ui.Row(), id, f.label, (double)v, 1, "%.0f", true);
			if (k == K_GAREA && f.off == 0 && (v != 0) == (Rd32(r, 0) != 0)) break;   // keep the stored non-zero value
			if (f.type == F_U8) r[f.off] = (std::uint8_t)v;
			else Wr32(r, f.off, (std::uint32_t)v);
			break;
		}
		case F_VEC3:
		{
			float v[3] = {RdF(r, f.off), RdF(r, f.off + 4), RdF(r, f.off + 8)};
			ui.Vec3(ui.Row(), id, f.label, v, 0.1, "%.2f");
			for (int i = 0; i < 3; i++) WrF(r, f.off + 4 * i, v[i]);
			break;
		}
		case F_FIX3: case F_INT3:
		{
			const float s = f.type == F_FIX3 ? 1000.0f : 1.0f;
			float v[3];
			for (int i = 0; i < 3; i++) v[i] = (float)RdI(r, f.off + 4 * i) / s;
			ui.Vec3(ui.Row(), id, f.label, v, f.type == F_FIX3 ? 0.1 : 5.0, f.type == F_FIX3 ? "%.2f" : "%.0f");
			for (int i = 0; i < 3; i++) WrI(r, f.off + 4 * i, (int)std::lround(v[i] * s));
			break;
		}
		}
	}
	if (r != Rec(ref))
	{
		PushUndo(&ref);
		Rec(ref) = r;
	}
}

void ToolsPage(devui::UI& ui)
{
	for (int t = 0; t < T_COUNT; t += 3)
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
		for (int j = 0; j < 3 && t + j < T_COUNT; j++)
			if (ui.Button(c[j], TOOL_NAMES[t + j], true, g_tool == t + j)) g_tool = (Tool)(t + j);
	}
	ui.Space();
	std::vector<std::string> names;
	switch (g_tool)
	{
	case T_ITEM:
		for (const char* n : ITEM_NAMES) names.push_back(n);
		g_addItem = ui.Choice(ui.Row(), devui::MakeId("g03e.additem"), "item", names, g_addItem);
		break;
	case T_BOX:
		for (const char* n : BOX_NAMES) names.push_back(n);
		g_addBox = ui.Choice(ui.Row(), devui::MakeId("g03e.addbox"), "box", names, g_addBox);
		g_addTreasure = ui.Choice(ui.Row(), devui::MakeId("g03e.addtreasure"), "treasure", TreasureNames(), g_addTreasure);
		break;
	case T_GATE:
		g_addGate = std::clamp((int)ui.Number(ui.Row(), devui::MakeId("g03e.addgate"), "gate kind", g_addGate, 1, "%.0f", true), 0, 2);
		break;
	case T_GAREA: case T_MOVE: case T_GHOST:
		g_areaFile = std::clamp((int)ui.Number(ui.Row(), devui::MakeId("g03e.areafile"), "ghost area file", g_areaFile, 1, "%.0f", true), 0, AREA_FILES - 1);
		if (g_tool == T_GAREA)
			g_addAreaKind = ui.Choice(ui.Row(), devui::MakeId("g03e.addarea"), "shape", std::vector<std::string>(AREA_KINDS, AREA_KINDS + 2), g_addAreaKind);
		if (g_tool == T_GHOST)
		{
			g_addGhost = ui.Choice(ui.Row(), devui::MakeId("g03e.addghost"), "ghost", std::vector<std::string>(GHOST_NAMES, GHOST_NAMES + 4), g_addGhost);
			g_addRide = ui.Choice(ui.Row(), devui::MakeId("g03e.addride"), "ride", std::vector<std::string>(RIDE_NAMES, RIDE_NAMES + 6), g_addRide);
		}
		break;
	case T_ROUTE:
		g_routeFile = std::clamp((int)ui.Number(ui.Row(), devui::MakeId("g03e.routefile"), "route", g_routeFile, 1, "%.0f", true), 0, ROUTE_FILES - 1);
		g_addGhost = ui.Choice(ui.Row(), devui::MakeId("g03e.addghost"), "new route's ghost", std::vector<std::string>(GHOST_NAMES, GHOST_NAMES + 4), g_addGhost);
		g_addRide = ui.Choice(ui.Row(), devui::MakeId("g03e.addride"), "new route's ride", std::vector<std::string>(RIDE_NAMES, RIDE_NAMES + 6), g_addRide);
		break;
	default: break;
	}
	ui.Label(g_tool == T_SELECT ? "Click a handle to select it (Shift adds), drag to move it on its height plane; Alt drag = up / down, Ctrl = grid. A left drag off the handles pans the editor camera."
	                            : "Click the stage to place one (Ctrl = on the grid). Esc goes back to Select.",
	         &devui::Color(devui::DIM));
	ui.Space();
	g_snapGround = ui.Checkbox(ui.Row(), "Drag along the stage's floor (one thing)", g_snapGround);
	static const double zero = 0.0;
	g_grid = (float)ui.Number(ui.Row(), devui::MakeId("g03e.grid"), "grid (Ctrl)", g_grid, 0.25, "%.2f", false, &zero);
	ui.Space();
	ui.Heading("Test start (editor only, never saved)");
	g_testOn = ui.Checkbox(ui.Row(), "Start Pac-Man at the test position", g_testOn);
	ui.Vec3(ui.Row(), devui::MakeId("g03e.test"), "position", g_test, 0.1, "%.2f");
	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1.2f, 1, 1});
	if (ui.Button(c[0], "Pac-Man's place"))
	{
		kingdom::game::g03::cAppG03PacnRollGame* app = kingdom::game::g03::Constants::PACN_APP();
		if (app != nullptr && app->pTaskCharPacman() != nullptr)
		{
			for (int i = 0; i < 3; i++) g_test[i] = app->pTaskCharPacman()->vTrans()[i];
			g_testOn = true;
		}
	}
	if (ui.Button(c[1], "Selected", !g_sel.empty() && Valid(g_sel[0])))
	{
		GetPos(KindOf(g_sel[0]), Rec(g_sel[0]), g_test);
		g_test[1] += START_LIFT;
		g_testOn = true;
	}
	if (ui.Button(c[2], "Restart")) Restart();
	ui.Label("The Test start tool sets it with a click. Restart keeps the applied edits; Apply also starts there.", &devui::Color(devui::DIM));
}

void SelectionPage(devui::UI& ui)
{
	std::vector<std::string> filters = {"everything"};
	for (int k = 0; k < K_COUNT; k++) filters.push_back(KINDS[k].plural);
	g_filter = ui.Choice(ui.Row(), devui::MakeId("g03e.filter"), "show", filters, g_filter);
	std::vector<Ref> list;
	for (const Ref& r : Things())
		if (g_filter == 0 || KindOf(r) == g_filter - 1) list.push_back(r);
	std::vector<devui::ListRow> rows;
	int sel = -1;
	for (std::size_t i = 0; i < list.size(); i++)
	{
		rows.push_back(devui::ListRow{Describe(list[i]), devui::Color(Selected(list[i]) ? devui::TEXT : devui::DIM)});
		if (!g_sel.empty() && list[i] == g_sel[0]) sel = (int)i;
	}
	bool activated = false;
	const int picked = ui.ListBox(devui::MakeId("g03e.things"), ui.Row(devui::ROW * 9), rows, sel, true, nullptr, false, &activated);
	if (picked != sel && picked >= 0 && picked < (int)list.size()) g_sel = {list[picked]};
	if (activated && picked >= 0 && picked < (int)list.size())
	{
		// a double click looks at it (the editor camera)
		float p[3];
		GetPos(KindOf(list[picked]), Rec(list[picked]), p);
		g_camOn = g_camInit = true;
		std::copy(p, p + 3, g_target);
		g_dist = std::min(g_dist, 25.0f);
	}
	if (g_sel.empty() || !Valid(g_sel[0]))
	{
		ui.Label("nothing selected", &devui::Color(devui::DIM));
		return;
	}
	ui.Space();
	if (g_sel.size() > 1) ui.Label(std::to_string(g_sel.size()) + " selected; the fields are the first one's", &devui::Color(devui::DIM));
	ui.Heading(Describe(g_sel[0]).substr(0, Describe(g_sel[0]).find("  (")));
	Fields(ui, g_sel[0]);
	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
	if (ui.Button(c[0], "Duplicate")) Duplicate();
	if (ui.Button(c[1], "Turn 15", HasTurn(KindOf(g_sel[0]))))
	{
		PushUndo();
		for (const Ref& r : g_sel)
			if (Valid(r) && HasTurn(KindOf(r))) Turn(KindOf(r), Rec(r), 15);
	}
	if (ui.Button(c[2], "Delete")) DeleteRefs(g_sel);
}

void ShowPage(devui::UI& ui)
{
	g_throughWalls = ui.Checkbox(ui.Row(), "Handles show through walls", g_throughWalls);
	for (int k = 0; k < K_COUNT; k += 2)
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		for (int j = 0; j < 2 && k + j < K_COUNT; j++) g_show[k + j] = ui.Checkbox(c[j], KINDS[k + j].plural, g_show[k + j]);
	}
	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
	if (ui.Button(c[0], "Show all")) std::fill(g_show, g_show + K_COUNT, true);
	if (ui.Button(c[1], "Hide all")) std::fill(g_show, g_show + K_COUNT, false);
}

void DeletePage(devui::UI& ui)
{
	if (ui.Button(ui.Row(), "Delete the selection (" + std::to_string(g_sel.size()) + ")", !g_sel.empty())) DeleteRefs(g_sel);
	ui.Space();
	for (int k = 0; k < K_COUNT; k++)
	{
		if (k == K_RIDER) continue;
		std::vector<Ref> all;
		for (const Ref& r : Things())
			if (KindOf(r) == k) all.push_back(r);
		if (ui.Button(ui.Row(), "Delete every " + std::string(KINDS[k].name) + " (" + std::to_string(all.size()) + ")", !all.empty())) DeleteRefs(all);
	}
}

void CheckPage(devui::UI& ui)
{
	const std::vector<Problem> p = Check();
	if (p.empty()) ui.Label("no problems found", &devui::Color(devui::GOOD));
	for (const Problem& q : p) ui.Label(q.text, &devui::Color(q.level >= 2 ? devui::BAD : devui::WARN));
}

void EditorPanel(devui::UI& ui)
{
	if (!InStage()) return;
	g_panelFrame = FrameNo();
	static bool once = false;
	if (!once) { std::fill(g_show, g_show + K_COUNT, true); once = true; }
	const int round = CurrentRound();
	if (g_data.round < 0 || (g_data.round != round && !g_dirty)) LoadStage(round);
	ui.Heading("Pac-N-Roll stage editor");
	ui.Label(RoundName(g_data.round) + "  -  " + std::to_string(Things().size()) + " things" +
	         (g_appliedRound == g_data.round ? (g_dirty ? "  -  edited since the last apply" : "  -  edits applied") : (g_dirty ? "  -  edited, not applied" : "")),
	         &devui::Color(g_dirty ? devui::WARN : devui::DIM));
	if (g_data.round != round)
	{
		ui.Label("The game is on " + RoundName(round) + "; the edits are for another round.", &devui::Color(devui::WARN));
		if (ui.Button(ui.Row(), "Drop the edits and edit this round")) LoadStage(round);
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1.4f, 1});
		if (ui.Button(c[0], "Apply + restart (Enter)")) { Apply(); return; }
		if (ui.Button(c[1], "Back to the disc")) { Revert(); return; }
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
		if (ui.Button(c[0], "Undo", !g_undo.empty())) Undo();
		if (ui.Button(c[1], "Redo", !g_redo.empty())) Redo();
		if (ui.Button(c[2], "Save")) Save();
		if (ui.Button(c[3], "Load")) Load();
	}
	ui.Label("files: " + EditDir(g_data.round).string(), &devui::Color(devui::DIM));
	g_editView = ui.Checkbox(ui.Row(), "Edit in the game view (the game gets no mouse / keys there)", g_editView);
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		const bool skip = ui.Checkbox(c[0], "No title card", G03SkipTitle());   // the same switch as --g03-skip-title
		if (skip != G03SkipTitle()) SetG03SkipTitle(skip);
		g_hideHud = ui.Checkbox(c[1], "Hide the HUD", g_hideHud);
	}
	g_noCull = ui.Checkbox(ui.Row(), "Draw everything (no culling by the game camera)", g_noCull);
	const bool cam = ui.Checkbox(ui.Row(), "Editor camera: right drag turns, middle / left drag pans, wheel", g_camOn);
	if (cam != g_camOn) { g_camOn = cam; g_camInit = false; }
	ui.Space();
	static const char* const pages[] = {"Tools", "Selection", "Show", "Delete", "Check"};
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1.3f, 1, 1, 1});
		for (int i = 0; i < 5; i++)
			if (ui.Button(c[i], pages[i], true, g_page == i)) g_page = i;
	}
	ui.Space(4);
	switch (g_page)
	{
	case 0: ToolsPage(ui); break;
	case 1: SelectionPage(ui); break;
	case 2: ShowPage(ui); break;
	case 3: DeletePage(ui); break;
	default: CheckPage(ui); break;
	}
	ui.Space();
	if (!g_msg.empty()) ui.Label(g_msg, &devui::Color(devui::HINT));
	ui.Label("Game view: Del / X delete, Ctrl+Z / Y undo / redo, Ctrl+D duplicate, R turn (Shift back), PgUp / PgDn up / down, Ctrl+S save, Enter apply.",
	         &devui::Color(devui::DIM));
	ViewInput();
}
}

void G03EditOverride(const std::string& key, const std::uint8_t*& data, std::size_t& size)
{
	if (g_appliedRound < 0 || !InStage() || CurrentRound() != g_appliedRound) return;
	auto it = g_applied.find(key);
	if (it == g_applied.end()) return;
	static const std::uint8_t empty = 0;
	data = it->second.empty() ? &empty : it->second.data();
	size = it->second.size();
}

void G03EditCamera(float eye[3], float target[3], float& farZ)
{
	if (!g_camOn || !Active()) return;
	if (!g_camInit)
	{
		// start where the game's camera is
		float d[3];
		for (int i = 0; i < 3; i++) d[i] = eye[i] - target[i];
		g_dist = std::max(1.0f, std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]));
		g_yaw = std::atan2(d[0], d[2]);
		g_pitch = std::asin(std::clamp(d[1] / g_dist, -1.0f, 1.0f));
		std::copy(target, target + 3, g_target);
		g_camInit = true;
	}
	farZ = std::max(farZ, g_dist + 5000.0f);   // the stage's far clip is meant for the game camera
	const float cp = std::cos(g_pitch);
	for (int i = 0; i < 3; i++) target[i] = g_target[i];
	eye[0] = g_target[0] + g_dist * cp * std::sin(g_yaw);
	eye[1] = g_target[1] + g_dist * std::sin(g_pitch);
	eye[2] = g_target[2] + g_dist * cp * std::cos(g_yaw);
}

bool G03HideHud() { return g_hideHud && InStage(); }
bool G03EditNoCull() { return g_noCull && Active(); }

bool G03EditStart(float out[3])
{
	if (!g_testOn || !InStage() || CurrentRound() != g_data.round) return false;
	std::copy(g_test, g_test + 3, out);
	return true;
}

void G03EditDraw3D()
{
	if (!Active() || g_data.round < 0) return;
	g_lines.clear();
	// the ghost routes as lines, then every handle
	for (int r = 0; r < ROUTE_FILES; r++)
	{
		const File& f = RouteFile(r);
		if (!g_show[K_ROUTE]) break;
		for (std::size_t i = 1; i < f.recs[0].size(); i++)
		{
			float a[3], b[3];
			GetPos(K_ROUTE, f.recs[0][i - 1], a);
			GetPos(K_ROUTE, f.recs[0][i], b);
			Line(a, b, KINDS[K_ROUTE].rgba);
		}
	}
	for (const Ref& r : Things())
		if (g_show[KindOf(r)] && !Selected(r)) DrawThing(r, false);
	for (const Ref& r : g_sel)
		if (Valid(r)) DrawThing(r, true);
	if (g_testOn)
	{
		// the test start: a big cross with a post above it
		const float top[3] = {g_test[0], g_test[1] + 3.0f, g_test[2]};
		Cross(g_test, 0.9f, 0x00ff80ffu);
		Line(g_test, top, 0x00ff80ffu);
	}
	if (!g_lines.empty()) gfx::DrawLines3D(g_lines.data(), g_lines.size(), !g_throughWalls);
}

void RegisterG03EditPanel() { devui::RegisterPanel("Editor", EditorPanel, [] { return InStage(); }); }
}
