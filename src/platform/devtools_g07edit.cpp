// The dev window's "Motos editor" tab (devui, --devui / F9), shown inside a single-player Pac-Motos stage (job MOTOS_GAME): a C++ port of
// unpac's kingdom/motos stage_editor.py + mapfile.py (the MAPOO_OBJ part + the models.py preview: no themes, camera / slope CSVs, shots, live send).
// The stage's MAPOO_OBJ is read from the attached archive; "Apply" hands the edited bytes to cKingdomMapoo::RegistObj (G07EditOverride, only
// for the edited stage, only in MOTOS_GAME) and restarts the stage through MOTOS_DEBUG_SELECT like the debug menu. Save / Load keep a stage's
// edited MAPOO_OBJ in a folder ($MEGAMIX_G07_EDITS, else $XDG_DATA_HOME/megamix/g07_edits, else ~/.local/share/megamix/g07_edits).
// NOT part of the 1:1 game.
//
// MAPOO_OBJ (big endian, mapfile.py): "MAPOOOBJ", version 0x20070419, layer count at 0xc, 16 bytes, then {node count, offset} per layer from
// 0x20, then 0x40-byte nodes: kind u16, variant u16, x u32, y u32 (16 units per panel), four i32 parameters, 0x24 bytes kept as read.
// The game scans every layer for the kinds it wants (cMotosMapoo::SearchObj), so the layer only decides the order nodes are met in.
// Map space: the arena is 16 x 16 panels of size 1.0; a node at (x, y) stands at (x / 16, 0, y / 16), in the panel (x >> 4, y >> 4), and the
// game draws everything through the slope matrix (cTaskChar::UpdateMtx: pSlopeMatrix * translate).

#include "platform/devtools.hpp"
#include "platform/devtools_g07edit.hpp"
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
#include <set>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/Gfx3D.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "wii/Archive.hpp"
#include <Constants.hpp>

namespace devtools
{
namespace
{
using Bytes = std::vector<std::uint8_t>;
namespace fs = std::filesystem;

// ------------------------------------------------------------------------------------------------ the map file (mapfile.py)

enum { KIND_PANEL = 1, KIND_GIMMICK = 2, KIND_ITEM = 4, KIND_PLAYER = 8, KIND_ENEMY = 16, KIND_PATTERN = 32, KIND_GROUP = 64 };
constexpr std::uint32_t VERSION = 0x20070419;
constexpr int NODE_SIZE = 0x40;

struct Node
{
	int kind = 0, variant = 0, x = 0, y = 0;
	int p[4]{};
	std::array<std::uint8_t, 0x24> tail{};
	int uid = 0;                          // the editor's handle (not in the file)
};
struct MapFile
{
	std::uint32_t version = VERSION;
	std::array<std::uint8_t, 0x10> pad{};
	std::vector<std::vector<Node>> layers;
};

int g_nextUid = 1;

std::uint32_t Rd32(const std::uint8_t* b) { return (std::uint32_t)b[0] << 24 | (std::uint32_t)b[1] << 16 | (std::uint32_t)b[2] << 8 | b[3]; }
void Put32(Bytes& b, std::uint32_t v) { for (int s = 24; s >= 0; s -= 8) b.push_back((std::uint8_t)(v >> s)); }
void Put16(Bytes& b, std::uint32_t v) { b.push_back((std::uint8_t)(v >> 8)); b.push_back((std::uint8_t)v); }

bool Parse(const std::uint8_t* d, std::size_t size, MapFile& out)
{
	if (size < 0x20 || std::memcmp(d, "MAPOOOBJ", 8) != 0) return false;
	MapFile m;
	m.version = Rd32(d + 8);
	std::memcpy(m.pad.data(), d + 0x10, 0x10);
	const std::uint32_t count = Rd32(d + 0xc);
	if (0x20 + 8 * (std::size_t)count > size) return false;
	for (std::uint32_t i = 0; i < count; i++)
	{
		const std::uint32_t n = Rd32(d + 0x20 + 8 * i), off = Rd32(d + 0x24 + 8 * i);
		if (off + (std::size_t)n * NODE_SIZE > size) return false;
		std::vector<Node> layer;
		for (std::uint32_t j = 0; j < n; j++)
		{
			const std::uint8_t* q = d + off + j * NODE_SIZE;
			Node node;
			node.kind = q[0] << 8 | q[1];
			node.variant = q[2] << 8 | q[3];
			node.x = (int)Rd32(q + 4);
			node.y = (int)Rd32(q + 8);
			for (int k = 0; k < 4; k++) node.p[k] = (int)Rd32(q + 12 + 4 * k);
			std::memcpy(node.tail.data(), q + 28, 0x24);
			node.uid = g_nextUid++;
			layer.push_back(node);
		}
		m.layers.push_back(layer);
	}
	out = m;
	return true;
}

Bytes Serialize(const MapFile& m)
{
	std::vector<const std::vector<Node>*> layers;
	for (const auto& l : m.layers)
		if (!l.empty()) layers.push_back(&l);
	Bytes b(reinterpret_cast<const std::uint8_t*>("MAPOOOBJ"), reinterpret_cast<const std::uint8_t*>("MAPOOOBJ") + 8);
	Put32(b, m.version);
	Put32(b, (std::uint32_t)layers.size());
	b.insert(b.end(), m.pad.begin(), m.pad.end());
	std::uint32_t off = 0x20 + 8 * (std::uint32_t)layers.size();
	for (const auto* l : layers)
	{
		Put32(b, (std::uint32_t)l->size());
		Put32(b, off);
		off += (std::uint32_t)l->size() * NODE_SIZE;
	}
	for (const auto* l : layers)
		for (const Node& n : *l)
		{
			Put16(b, (std::uint32_t)n.kind);
			Put16(b, (std::uint32_t)n.variant);
			Put32(b, (std::uint32_t)n.x);
			Put32(b, (std::uint32_t)n.y);
			for (int k = 0; k < 4; k++) Put32(b, (std::uint32_t)n.p[k]);
			b.insert(b.end(), n.tail.begin(), n.tail.end());
		}
	return b;
}

// the order the game's own files list the kinds in (mapfile.py KIND_ORDER): a new kind gets a layer of its own there
int KindRank(int kind) { return kind == KIND_ITEM ? 0 : kind == KIND_ENEMY ? 1 : kind == KIND_PLAYER ? 2 : 3; }

void AddNode(MapFile& m, Node n)
{
	if (n.uid == 0) n.uid = g_nextUid++;
	for (auto& l : m.layers)
		for (const Node& o : l)
			if (o.kind == n.kind) { l.push_back(n); return; }
	std::size_t at = 0;
	for (std::size_t i = 0; i < m.layers.size(); i++)
	{
		int lo = 99;
		for (const Node& o : m.layers[i]) lo = std::min(lo, KindRank(o.kind));
		if (!m.layers[i].empty() && lo <= KindRank(n.kind)) at = i + 1;
	}
	m.layers.insert(m.layers.begin() + (std::ptrdiff_t)at, std::vector<Node>{n});
}

// ------------------------------------------------------------------------------------------------ names (map.py, stage_editor.py)

struct Named { int v; const char* name; };
const std::vector<Named> PANELS = {{1, "a"}, {2, "b"}, {3, "c"}, {4, "floor"}, {5, "wall basic1"}, {6, "wall basic2"}, {7, "wall straight vertical"},
                                   {9, "wall straight horizontal"}, {11, "wall corner left bottom"}, {12, "wall corner right top"},
                                   {13, "wall corner right bottom"}, {14, "wall corner left top"}, {15, "wall fourway"}, {19, "wall threeway bottom"},
                                   {20, "wall threeway top"}, {21, "wall threeway right"}, {22, "wall threeway left"}};
const std::vector<Named> GIMMICKS = {{1, "dash"}, {2, "hatch"}, {3, "switch"}, {4, "blast"}};
const std::vector<Named> ITEMS = {{1, "power"}, {2, "jump"}, {3, "charge"}};
const std::vector<Named> PLAYERS = {{1, "p1"}, {2, "p2"}, {3, "p3"}, {4, "p4"}};
const std::vector<Named> ENEMIES = {{1, "pupa red"}, {2, "pupa blue"}, {3, "pupa black"}, {4, "polar"}, {5, "spirus"}, {6, "fire bug"}, {7, "fire bee"},
                                    {8, "taitorian"}, {9, "lady bug"}, {10, "beetle"}, {11, "mega"}, {12, "giga"}, {14, "beecon"}, {15, "nabicon"},
                                    {16, "boss world1"}, {17, "boss world2"}, {18, "boss world3"}, {19, "boss world4"}, {20, "boss world5"},
                                    {21, "boss world5 last phase"}, {22, "ball blue"}, {23, "ball green"}, {24, "peta"}};
const std::vector<Named> PATTERNS = {{1, "random"}, {2, "flash"}, {4, "wipe bottom"}, {5, "wipe top"}, {6, "wipe left"}, {7, "wipe right"},
                                     {8, "wipe diagonal right top"}, {9, "wipe diagonal right bottom"}, {10, "wipe diagonal left bottom"},
                                     {11, "wipe diagonal left top"}, {12, "interlace vertical"}, {13, "interlace horizontal"}, {14, "weave right"},
                                     {15, "weave left"}, {16, "checkered"}, {17, "wipe center"}};
// formation types of a group node (cTaskCharEnemy: type 1 breaks the formation up, 2..5 are FormMeet / FormMove Snake / Circle / Arrow / Fence)
const std::vector<Named> FORMATIONS = {{1, "release (free)"}, {2, "snake (behind)"}, {3, "circle (orbit)"}, {4, "arrow"}, {5, "fence (tangent)"}};
const std::vector<Named> NONE;

const std::vector<Named>& Variants(int kind)
{
	switch (kind)
	{
	case KIND_PANEL: return PANELS;
	case KIND_GIMMICK: return GIMMICKS;
	case KIND_ITEM: return ITEMS;
	case KIND_PLAYER: return PLAYERS;
	case KIND_ENEMY: return ENEMIES;
	case KIND_PATTERN: return PATTERNS;
	case KIND_GROUP: return FORMATIONS;
	default: return NONE;
	}
}
std::string VariantName(int kind, int variant)
{
	for (const Named& n : Variants(kind))
		if (n.v == variant) return n.name;
	return "variant " + std::to_string(variant);
}
const char* KindName(int kind)
{
	switch (kind)
	{
	case KIND_PANEL: return "panel";
	case KIND_GIMMICK: return "gimmick";
	case KIND_ITEM: return "item";
	case KIND_PLAYER: return "player";
	case KIND_ENEMY: return "enemy";
	case KIND_PATTERN: return "light pattern";
	case KIND_GROUP: return "formation step";
	default: return "node";
	}
}
std::string Describe(const Node& n)
{
	return std::string(KindName(n.kind)) + " " + VariantName(n.kind, n.variant) + " at " + std::to_string(n.x >> 4) + "," + std::to_string(n.y >> 4);
}

const char* const DIRECTIONS[4] = {"up", "down", "left", "right"};    // parameter 0: 0 up (-z), 1 down (+z), 2 left (-x), 3 right (+x)
const std::vector<std::string> ROLES = {"solo", "leader", "follower"};
const std::vector<std::string> PHASES = {"in step", "half a cycle later"};
const int FORMATION_ENEMIES[] = {4, 5, 7, 8, 9, 10, 11, 12};          // SetMMSParam takes a formation role and group
constexpr int NAVICON = 15;

bool IsFormationEnemy(int variant) { return std::find(std::begin(FORMATION_ENEMIES), std::end(FORMATION_ENEMIES), variant) != std::end(FORMATION_ENEMIES); }
bool IsHatch(const Node& n) { return n.kind == KIND_GIMMICK && n.variant == 2; }
bool IsPanelNode(const Node& n) { return n.kind == KIND_PANEL || (n.kind == KIND_GIMMICK && n.variant == 4); }   // the blast panel is a gimmick node
bool IsFormationMember(const Node& n) { return n.kind == KIND_ENEMY && IsFormationEnemy(n.variant) && (n.p[1] == 1 || n.p[1] == 2); }

// what each of a node's four parameters means (stage_editor.py param_specs, from cTaskMap Setup*CB, cTaskCharEnemy*::SetMMSParam, cTaskGimmickHatch)
struct Spec { const char* label; char widget; const std::vector<std::string>* choices; };   // widget 'd' direction, 'i' number, 'f' flag, 'c' choice, '-' unused
std::array<Spec, 4> Specs(int kind, int variant)
{
	const Spec dir{"direction", 'd', nullptr}, unused{"unused", '-', nullptr};
	if (kind == KIND_ENEMY)
	{
		if (IsFormationEnemy(variant)) return {dir, Spec{"role", 'c', &ROLES}, Spec{"group number", 'i', nullptr}, unused};
		if (variant == NAVICON) return {dir, Spec{"interval", 'i', nullptr}, Spec{"start timer", 'i', nullptr}, Spec{"breakable", 'f', nullptr}};
		return {dir, unused, unused, unused};
	}
	if (kind == KIND_PLAYER) return {dir, unused, unused, unused};
	if (kind == KIND_GIMMICK)
	{
		if (variant == 2) return {unused, Spec{"phase", 'c', &PHASES}, Spec{"closed for", 'i', nullptr}, Spec{"open for", 'i', nullptr}};
		if (variant == 1 || variant == 4 || variant == 5 || variant == 6) return {dir, unused, unused, unused};
	}
	if (kind == KIND_GROUP)
		return {Spec{"group number", 'i', nullptr}, Spec{"order", 'i', nullptr}, Spec{"meet frames", 'i', nullptr}, Spec{"move frames", 'i', nullptr}};
	return {unused, unused, unused, unused};
}
const char* SpecHint(int kind, int variant)
{
	if (kind == KIND_ENEMY)
		return IsFormationEnemy(variant) ? "solo ignores the group; a leader needs formation steps"
		                                 : variant == NAVICON ? "60 frames per second; interval 0 = 5 seconds" : "this enemy only uses its direction";
	if (kind == KIND_GIMMICK && variant == 2) return "0 = 180 frames; opening and closing take 40";
	if (kind == KIND_GROUP) return "steps run in order; meet = frames to gather";
	if (kind == KIND_ITEM || kind == KIND_PANEL || kind == KIND_PATTERN) return "this has no parameters";
	return "";
}
bool HasDirection(int kind, int variant) { return Specs(kind, variant)[0].widget == 'd'; }

// cells a node covers: a hatch covers 4 x 4 around its corner point, everything else its own cell
std::vector<std::pair<int, int>> Footprint(const Node& n)
{
	const int cx = n.x >> 4, cz = n.y >> 4;
	std::vector<std::pair<int, int>> out;
	if (IsHatch(n))
		for (int j = 0; j < 4; j++)
			for (int i = 0; i < 4; i++) out.push_back({cx - 2 + i, cz - 2 + j});
	else
		out.push_back({cx, cz});
	return out;
}
bool Overlap(const Node& a, const Node& b)
{
	for (const auto& c : Footprint(a))
		for (const auto& d : Footprint(b))
			if (c == d) return true;
	return false;
}

// ------------------------------------------------------------------------------------------------ editor state

enum Tool { T_SELECT, T_PANEL, T_GIMMICK, T_ITEM, T_ENEMY, T_PLAYER, T_ERASE, T_COUNT };
const char* const TOOL_NAMES[T_COUNT] = {"Select", "Panel", "Gimmick", "Item", "Enemy", "Player", "Erase"};
const int TOOL_KIND[T_COUNT] = {0, KIND_PANEL, KIND_GIMMICK, KIND_ITEM, KIND_ENEMY, KIND_PLAYER, 0};

MapFile g_map;
int g_stage = -1;                           // nStageIdx the map was read for
std::vector<MapFile> g_undo, g_redo;
int g_sel = 0;                              // uid, 0 = none
Bytes g_applied;                            // what RegistObj hands out for g_appliedStage
int g_appliedStage = -1;
bool g_dirty = false;
std::string g_msg;
int g_panelFrame = -100;
int g_page = 0;                             // Tools / Selection / Stage / Check
Tool g_tool = T_PANEL;
int g_brush[T_COUNT] = {0, 1, 1, 1, 1, 1, 0};
int g_brushP[4]{};
bool g_skipItemSet = false;
bool g_preview = true;
std::vector<std::string> g_prevMissing;     // preview models the attached archives do not hold
bool g_free = false, g_showUnused = false, g_editView = true, g_grid = true, g_outlines = true, g_throughWalls = true;
// the cursor on the arena (map units: panels)
bool g_hasHit = false;
float g_hit[2]{};
int g_cell[2]{};
// mouse work
enum Drag { D_NONE, D_PAINT, D_ERASE, D_MOVE };
Drag g_drag = D_NONE;
int g_dragLast[3]{-1, -1, -1};
bool g_dragPushed = false;
float g_grabOff[2]{}, g_pressHit[2]{};
bool g_leftPending = false, g_leftPanning = false, g_rightPending = false, g_rightOrbit = false;
float g_pressX = 0, g_pressY = 0;
// the orbit camera
bool g_camOn = false, g_camInit = false;
float g_yaw = 0, g_pitch = 0.9f, g_dist = 20, g_target[3]{};

bool InStage()
{
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	return core != nullptr && core->GetJobNo() == kingdom::job::MOTOS_GAME && KingdomGlobal::MotosSys != nullptr && KingdomGlobal::g_pMotosGame != nullptr &&
	       !KingdomGlobal::g_pMotosGame->bMultiPlay();
}
bool Active() { return InStage() && FrameNo() - g_panelFrame <= 2; }   // the tab is drawn
int CurrentStage() { return (int)KingdomGlobal::MotosSys->nStageIdx(); }

std::string StageName(int idx)
{
	char buf[64];
	if (idx == CurrentStage())
		std::snprintf(buf, sizeof buf, "stage %d (w%02ds%02d)", idx, (int)KingdomGlobal::MotosSys->nWorld() + 1, (int)KingdomGlobal::MotosSys->nStage() + 1);
	else
		std::snprintf(buf, sizeof buf, "stage %d", idx);
	return buf;
}

std::vector<Node*> AllNodes()
{
	std::vector<Node*> out;
	for (auto& l : g_map.layers)
		for (Node& n : l) out.push_back(&n);
	return out;
}
Node* Find(int uid)
{
	if (uid == 0) return nullptr;
	for (auto& l : g_map.layers)
		for (Node& n : l)
			if (n.uid == uid) return &n;
	return nullptr;
}
void Remove(int uid)
{
	for (auto& l : g_map.layers)
		for (std::size_t i = 0; i < l.size(); i++)
			if (l[i].uid == uid) { l.erase(l.begin() + (std::ptrdiff_t)i); break; }
	if (g_sel == uid) g_sel = 0;
}

void LoadStage(int idx)
{
	g_map = MapFile{};
	g_stage = idx;
	g_undo.clear();
	g_redo.clear();
	g_sel = 0;
	g_dirty = false;
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find("MAPOO_OBJ", size);
	if (d == nullptr || !Parse(d, size, g_map)) g_msg = "this stage has no readable MAPOO_OBJ";
	else if (g_appliedStage == idx && Parse(g_applied.data(), g_applied.size(), g_map)) g_msg = "editing the applied edits of this stage";
}

void Push()
{
	g_undo.push_back(g_map);
	if (g_undo.size() > 200) g_undo.erase(g_undo.begin());
	g_redo.clear();
	g_dirty = true;
}
void Undo()
{
	if (g_undo.empty()) return;
	g_redo.push_back(g_map);
	g_map = g_undo.back();
	g_undo.pop_back();
	g_dirty = true;
}
void Redo()
{
	if (g_redo.empty()) return;
	g_undo.push_back(g_map);
	g_map = g_redo.back();
	g_redo.pop_back();
	g_dirty = true;
}

// ------------------------------------------------------------------------------------------------ edits (stage_editor.py)

std::vector<Node*> NodesAt(int cx, int cz)
{
	std::vector<Node*> out;
	for (Node* n : AllNodes())
	{
		if (n->kind != KIND_PANEL && n->kind != KIND_GIMMICK && n->kind != KIND_ITEM && n->kind != KIND_ENEMY && n->kind != KIND_PLAYER) continue;
		for (const auto& c : Footprint(*n))
			if (c.first == cx && c.second == cz) { out.push_back(n); break; }
	}
	return out;
}
// what a click on a cell means: players, enemies, items, gimmicks, then the panel
Node* TopAt(int cx, int cz)
{
	auto rank = [](const Node* n) { return IsPanelNode(*n) ? 4 : n->kind == KIND_PLAYER ? 0 : n->kind == KIND_ENEMY ? 1 : n->kind == KIND_ITEM ? 2 : 3; };
	Node* best = nullptr;
	for (Node* n : NodesAt(cx, cz))
		if (best == nullptr || rank(n) < rank(best)) best = n;
	return best;
}

// map position (1/16 panels) for a point on the arena: cell centres, the corner point for a hatch; free = the nearest 1/16 (not panels)
void Snap(int kind, int variant, float hx, float hz, bool free, int& x, int& y)
{
	const bool panel = kind == KIND_PANEL || (kind == KIND_GIMMICK && variant == 4), hatch = kind == KIND_GIMMICK && variant == 2;
	if (free && !panel)
	{
		const int lo = hatch ? 32 : 0, hi = hatch ? 224 : 255;
		x = std::clamp((int)std::lround(hx * 16), lo, hi);
		y = std::clamp((int)std::lround(hz * 16), lo, hi);
		return;
	}
	if (hatch)
	{
		x = std::clamp((int)std::lround(hx), 2, 14) * 16;
		y = std::clamp((int)std::lround(hz), 2, 14) * 16;
		return;
	}
	x = std::clamp((int)std::floor(hx), 0, 15) * 16 + 8;
	y = std::clamp((int)std::floor(hz), 0, 15) * 16 + 8;
}

// add a node, removing what it replaces (a panel in the cell, the same kind of object, a player of that number); returns its uid
int Place(int kind, int variant, int x, int y, const int p[4])
{
	Node nn;
	nn.kind = kind;
	nn.variant = variant;
	nn.x = x;
	nn.y = y;
	if (p != nullptr) std::copy(p, p + 4, nn.p);
	std::vector<int> doomed;
	for (Node* n : AllNodes())
	{
		const bool same = Overlap(*n, nn);
		if ((IsPanelNode(nn) && IsPanelNode(*n) && same) ||
		    (kind == KIND_GIMMICK && !IsPanelNode(nn) && n->kind == KIND_GIMMICK && !IsPanelNode(*n) && same) ||
		    ((kind == KIND_ITEM || kind == KIND_ENEMY) && n->kind == kind && same) ||
		    (kind == KIND_PLAYER && n->kind == KIND_PLAYER && (n->variant == variant || same)))
			doomed.push_back(n->uid);
	}
	for (int u : doomed) Remove(u);
	nn.uid = g_nextUid++;
	AddNode(g_map, nn);
	return nn.uid;
}

void DeleteSelection()
{
	if (Find(g_sel) == nullptr) return;
	Push();
	Remove(g_sel);
}

void Duplicate()
{
	const Node* n = Find(g_sel);
	if (n == nullptr || n->kind == KIND_PATTERN) return;
	Push();
	Node c = *n;
	c.x += c.x + 16 < 256 ? 16 : -16;
	c.uid = g_nextUid++;
	AddNode(g_map, c);
	g_sel = c.uid;
}

// move every panel, gimmick, item, enemy and player by whole cells; nothing moves when one would leave the 16 x 16 arena
void ShiftAll(int dx, int dz)
{
	int out = 0, moving = 0;
	for (Node* n : AllNodes())
	{
		if (n->kind == KIND_PATTERN || n->kind == KIND_GROUP) continue;
		moving++;
		if (!(n->x + dx * 16 >= 0 && n->x + dx * 16 < 256 && n->y + dz * 16 >= 0 && n->y + dz * 16 < 256)) out++;
	}
	if (out > 0) { g_msg = "cannot shift: " + std::to_string(out) + " would leave the arena"; return; }
	if (moving == 0) return;
	Push();
	for (Node* n : AllNodes())
		if (n->kind != KIND_PATTERN && n->kind != KIND_GROUP) { n->x += dx * 16; n->y += dz * 16; }
	g_msg = dx < 0 ? "shifted left" : dx > 0 ? "shifted right" : dz < 0 ? "shifted up" : "shifted down";
}

void Turn()
{
	Node* n = Find(g_sel);
	if (n != nullptr && HasDirection(n->kind, n->variant)) { Push(); n->p[0] = (n->p[0] + 1) % 4; }
	else if (n == nullptr && TOOL_KIND[g_tool] != 0 && HasDirection(TOOL_KIND[g_tool], g_brush[g_tool])) g_brushP[0] = (g_brushP[0] + 1) % 4;
}

int Pattern()
{
	int v = -1;
	for (Node* n : AllNodes())
		if (n->kind == KIND_PATTERN) v = n->variant;   // the last one counts
	return v;
}
void SetPattern(int variant)
{
	Push();
	std::vector<int> doomed;
	for (Node* n : AllNodes())
		if (n->kind == KIND_PATTERN) doomed.push_back(n->uid);
	for (int u : doomed) Remove(u);
	Node n;
	n.kind = KIND_PATTERN;
	n.variant = variant;
	n.x = n.y = 248;
	AddNode(g_map, n);
}

void Clear(int what)   // 0 panels, 1 gimmicks, 2 items, 3 enemies (and formation steps)
{
	static const char* const names[] = {"panels", "gimmicks", "items", "enemies"};
	std::vector<int> doomed;
	for (Node* n : AllNodes())
	{
		const bool hit = what == 0 ? IsPanelNode(*n) : what == 1 ? (n->kind == KIND_GIMMICK && !IsPanelNode(*n))
		                                              : what == 2 ? n->kind == KIND_ITEM : (n->kind == KIND_ENEMY || n->kind == KIND_GROUP);
		if (hit) doomed.push_back(n->uid);
	}
	if (doomed.empty()) return;
	Push();
	for (int u : doomed) Remove(u);
	g_msg = "cleared " + std::to_string(doomed.size()) + " " + names[what];
}

std::vector<Node*> FormationSteps(int group)
{
	std::vector<Node*> out;
	for (Node* n : AllNodes())
		if (n->kind == KIND_GROUP && n->p[0] == group) out.push_back(n);
	std::stable_sort(out.begin(), out.end(), [](const Node* a, const Node* b) { return a->p[1] < b->p[1]; });
	return out;
}

// ------------------------------------------------------------------------------------------------ checking (stage_editor.py check)

struct Problem { int level; std::string text; int uid; };
std::vector<Problem> Check()
{
	std::vector<Problem> out;
	const std::vector<Node*> nodes = AllNodes();
	std::map<std::pair<int, int>, const Node*> panels;
	for (const Node* n : nodes)
	{
		if (n->x >= 256 || n->y >= 256 || n->x < 0 || n->y < 0) out.push_back({2, Describe(*n) + " is outside the 16 x 16 map", n->uid});
		if (!IsPanelNode(*n)) continue;
		const std::pair<int, int> c{n->x >> 4, n->y >> 4};
		if (panels.count(c)) out.push_back({2, "two panels in cell " + std::to_string(c.first) + "," + std::to_string(c.second) + " (the game keeps the first)", n->uid});
		panels[c] = n;
		bool known = n->kind != KIND_PANEL;
		for (const Named& v : PANELS) known = known || v.v == n->variant;
		if (!known) out.push_back({2, "panel variant " + std::to_string(n->variant) + " is not one the game knows", n->uid});
	}
	std::set<int> seen;
	bool p1 = false, anyPlayer = false;
	for (const Node* n : nodes)
	{
		if (n->kind != KIND_PLAYER) continue;
		anyPlayer = true;
		p1 = p1 || n->variant == 1;
		if (seen.count(n->variant)) out.push_back({2, "player " + std::to_string(n->variant) + " is placed twice", n->uid});
		seen.insert(n->variant);
	}
	if (!anyPlayer) out.push_back({2, "no player start", 0});
	else if (!p1) out.push_back({1, "no player 1", 0});
	std::map<std::pair<int, int>, int> owner;
	for (const Node* n : nodes)
	{
		if (n->kind == KIND_GIMMICK && !IsPanelNode(*n))
			for (const auto& c : Footprint(*n))
			{
				if (owner.count(c)) out.push_back({2, "gimmicks overlap at " + std::to_string(c.first) + "," + std::to_string(c.second), n->uid});
				owner[c] = n->uid;
			}
		if ((n->kind == KIND_ITEM || n->kind == KIND_ENEMY || n->kind == KIND_PLAYER) && !panels.count({n->x >> 4, n->y >> 4}))
			out.push_back({1, Describe(*n) + " is not on a panel", n->uid});
		if (n->kind == KIND_GIMMICK && (n->variant < 1 || n->variant > 6)) out.push_back({1, "unknown gimmick variant " + std::to_string(n->variant), n->uid});
	}
	std::map<int, std::vector<const Node*>> groups, steps;
	for (const Node* n : nodes)
	{
		if (IsFormationMember(*n)) groups[n->p[2]].push_back(n);
		if (n->kind == KIND_GROUP) steps[n->p[0]].push_back(n);
	}
	for (const auto& [g, members] : groups)
	{
		const std::string gs = std::to_string(g);
		std::vector<const Node*> leaders;
		for (const Node* m : members)
			if (m->p[1] == 1) leaders.push_back(m);
		if (leaders.empty()) out.push_back({1, "formation group " + gs + " has followers but no leader (they will not form up)", members[0]->uid});
		else if (!steps.count(g)) out.push_back({1, "the leader of formation group " + gs + " has no formation step (it will act alone)", leaders[0]->uid});
		for (const Node* m : leaders)
		{
			if (m->variant == 7) out.push_back({2, "Fire Bee cannot lead formation group " + gs + ": its own AI forces the formation's modes and the formation breaks", m->uid});
			else if (m->variant == 9) out.push_back({1, "Lady Bug leads formation group " + gs + ": its AI forces the meet mode at random, which may break the formation", m->uid});
		}
		std::set<int> orders;
		for (const Node* s : steps[g])
			if (!orders.insert(s->p[1]).second) { out.push_back({1, "formation group " + gs + " has two steps with the same order", s->uid}); break; }
	}
	for (const auto& [g, st] : steps)
		if (!groups.count(g)) out.push_back({1, "formation steps for group " + std::to_string(g) + ", but no placed enemy leads or follows it (a boss may use it)", st[0]->uid});
	int patterns = 0;
	for (const Node* n : nodes) patterns += n->kind == KIND_PATTERN ? 1 : 0;
	if (patterns > 1) out.push_back({1, "more than one light pattern node (the last one counts)", 0});
	std::stable_sort(out.begin(), out.end(), [](const Problem& a, const Problem& b) { return a.level > b.level; });
	return out;
}

// ------------------------------------------------------------------------------------------------ apply / save / load

void Restart(int idx)
{
	KingdomGlobal::MotosSys->nStageIdx() = (unsigned)idx;
	kingdom::Kingdom_SetJobNo(kingdom::job::MOTOS_DEBUG_SELECT, false, false);   // the stage restarts and reads MAPOO_OBJ again
}

void Apply()
{
	g_applied = Serialize(g_map);
	g_appliedStage = g_stage;
	g_dirty = false;
	Restart(g_stage);
	g_msg = "applied: the stage restarts with the edits";
}

void Revert()
{
	const int idx = g_stage;
	g_applied.clear();
	g_appliedStage = -1;
	LoadStage(idx);
	Restart(idx);
	g_msg = "back to the disc's data";
}

fs::path EditDir(int idx)
{
	fs::path root;
	if (const char* e = std::getenv("MEGAMIX_G07_EDITS"); e != nullptr && *e) root = e;
	else if (const char* x = std::getenv("XDG_DATA_HOME"); x != nullptr && *x) root = fs::path(x) / "megamix" / "g07_edits";
	else if (const char* h = std::getenv("HOME"); h != nullptr) root = fs::path(h) / ".local" / "share" / "megamix" / "g07_edits";
	else root = "g07_edits";
	char buf[16];
	std::snprintf(buf, sizeof buf, "stage%02d", idx);
	return root / buf;
}

void Save()
{
	const fs::path dir = EditDir(g_stage);
	std::error_code ec;
	fs::create_directories(dir, ec);
	const Bytes b = Serialize(g_map);
	std::ofstream out(dir / "MAPOO_OBJ", std::ios::binary);
	out.write((const char*)b.data(), (std::streamsize)b.size());
	g_msg = out.good() ? "saved " + (dir / "MAPOO_OBJ").string() : "could not write " + (dir / "MAPOO_OBJ").string();
}

void Load()
{
	const fs::path path = EditDir(g_stage) / "MAPOO_OBJ";
	std::ifstream in(path, std::ios::binary);
	if (!in) { g_msg = "nothing saved in " + path.parent_path().string(); return; }
	const Bytes b((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	MapFile m;
	if (!Parse(b.data(), b.size(), m)) { g_msg = path.string() + " is not a MAPOO_OBJ file"; return; }
	Push();
	g_map = m;
	g_sel = 0;
	g_msg = "loaded: Apply to play it";
}

// ------------------------------------------------------------------------------------------------ map space <-> world (the slope matrix)

void Slope(float m[16])
{
	// column-major copy of pSlopeMatrix (row-major, column vectors: translation in m03 / m13 / m23)
	const Matrix4x4& s = KingdomGlobal::g_pMotosGame->pSlopeMatrix();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++) m[c * 4 + r] = s.m[r * 4 + c];
}
void ToWorld(const float slope[16], float x, float y, float z, float out[3])
{
	const float in[4] = {x, y, z, 1.0f};
	float w[4];
	Transform(slope, in, w);
	for (int i = 0; i < 3; i++) out[i] = w[i];
}

// the point of the arena floor (y = 0 in map space) under the mouse
bool ArenaHit(float mx, float my, float hit[2])
{
	float vp[16], o[3], d[3], slope[16], inv[16];
	if (!ViewProj(vp) || !MouseRay(vp, mx, my, o, d)) return false;
	Slope(slope);
	if (!Invert(slope, inv)) return false;
	const float o4[4] = {o[0], o[1], o[2], 1.0f}, d4[4] = {d[0], d[1], d[2], 0.0f};
	float lo[4], ld[4];
	Transform(inv, o4, lo);
	Transform(inv, d4, ld);
	if (std::fabs(ld[1]) < 1e-6f) return false;
	const float t = -lo[1] / ld[1];
	if (t <= 0) return false;
	hit[0] = lo[0] + ld[0] * t;
	hit[1] = lo[2] + ld[2] * t;
	return hit[0] >= -1 && hit[0] <= 17 && hit[1] >= -1 && hit[1] <= 17;
}

// ------------------------------------------------------------------------------------------------ the game view

bool BrushNode(Node& out)
{
	const int kind = TOOL_KIND[g_tool];
	if (kind == 0 || !g_hasHit) return false;
	out = Node{};
	out.kind = kind;
	out.variant = g_brush[g_tool];
	Snap(kind, out.variant, g_hit[0], g_hit[1], g_free != devui::GetInput().shift, out.x, out.y);
	std::copy(g_brushP, g_brushP + 4, out.p);
	return true;
}

void EraseAt(int cx, int cz, bool& pushed)
{
	Node* t = TopAt(cx, cz);
	if (t == nullptr) return;
	if (!pushed) { Push(); pushed = true; }
	Remove(t->uid);
}

void ViewInput()
{
	const devui::Input in = devui::GetInput();
	const devui::Rect gv = devui::GameView();
	const bool over = in.mx >= gv.x && in.mx < gv.x + gv.w && in.my >= gv.y && in.my < gv.y + gv.h;
	if (!g_editView) { g_drag = D_NONE; g_hasHit = false; return; }
	const bool busy = g_drag != D_NONE || g_leftPending || g_rightPending;
	if (over || busy) devui::CaptureInput(true, true);
	g_hasHit = over && ArenaHit(in.mx, in.my, g_hit);
	if (g_hasHit) { g_cell[0] = (int)std::floor(g_hit[0]); g_cell[1] = (int)std::floor(g_hit[1]); }
	const bool inside = g_hasHit && g_cell[0] >= 0 && g_cell[0] < 16 && g_cell[1] >= 0 && g_cell[1] < 16;
	const bool free = g_free != in.shift;

	// the orbit camera: a right drag turns, a middle drag (or a left drag off the things) pans, the wheel zooms
	if ((g_leftPending || g_rightPending) && std::fabs(in.mx - g_pressX) + std::fabs(in.my - g_pressY) > 4.0f)
	{
		if (g_leftPending && g_camOn) g_leftPanning = true;
		if (g_rightPending && g_camOn) g_rightOrbit = true;
	}
	if (g_camOn && (over || g_leftPanning || g_rightOrbit))
	{
		if (g_rightOrbit && in.down[1]) { g_yaw -= in.dx * 0.01f; g_pitch = std::clamp(g_pitch + in.dy * 0.01f, -1.5f, 1.5f); }
		if ((over && in.down[2]) || (g_leftPanning && in.down[0]))
		{
			const float s = g_dist * 0.0025f, cy = std::cos(g_yaw), sy = std::sin(g_yaw);
			g_target[0] -= (in.dx * cy) * s;
			g_target[2] -= (-in.dx * sy) * s;
			g_target[1] += in.dy * s;
		}
		if (over && in.wheel != 0.0f) g_dist = std::clamp(g_dist * std::pow(0.88f, in.wheel), 2.0f, 400.0f);
	}

	// right button: a click removes the topmost thing of the cell; a drag turns the editor camera, or (camera off) swipes things away
	if (over && in.pressed[1] && inside)
	{
		g_rightPending = true;
		g_rightOrbit = false;
		g_dragPushed = false;
		g_dragLast[0] = g_dragLast[1] = -1;
		g_pressX = in.mx;
		g_pressY = in.my;
	}
	if (g_rightPending)
	{
		if (!g_camOn && in.down[1] && inside && (g_cell[0] != g_dragLast[0] || g_cell[1] != g_dragLast[1]))
		{
			g_dragLast[0] = g_cell[0];
			g_dragLast[1] = g_cell[1];
			EraseAt(g_cell[0], g_cell[1], g_dragPushed);
		}
		if (!in.down[1])
		{
			if (g_camOn && !g_rightOrbit && inside) EraseAt(g_cell[0], g_cell[1], g_dragPushed);
			g_rightPending = g_rightOrbit = false;
		}
	}

	if (over && in.pressed[0] && g_hasHit)
	{
		g_dragPushed = false;
		g_dragLast[0] = g_dragLast[1] = g_dragLast[2] = -1;
		g_pressX = in.mx;
		g_pressY = in.my;
		if (g_tool == T_SELECT)
		{
			Node* t = inside ? TopAt(g_cell[0], g_cell[1]) : nullptr;
			if (t == nullptr)
			{
				g_leftPending = true;   // deselect on release, or pan
				g_leftPanning = false;
			}
			else
			{
				g_sel = t->uid;
				g_drag = D_MOVE;
				g_grabOff[0] = t->x / 16.0f - g_hit[0];
				g_grabOff[1] = t->y / 16.0f - g_hit[1];
				g_pressHit[0] = g_hit[0];
				g_pressHit[1] = g_hit[1];
			}
		}
		else if (g_tool == T_ERASE && inside)
			g_drag = D_ERASE;
		else if (inside)
			g_drag = D_PAINT;
		else
		{
			g_leftPending = true;
			g_leftPanning = false;
		}
	}
	if (g_leftPending && !in.down[0])
	{
		if (!g_leftPanning && over && g_tool == T_SELECT) g_sel = 0;
		g_leftPending = g_leftPanning = false;
	}
	if (!in.down[0]) g_drag = D_NONE;
	if (g_drag == D_PAINT && inside)
	{
		Node b;
		if (BrushNode(b) && (b.x != g_dragLast[0] || b.y != g_dragLast[1] || b.variant != g_dragLast[2]) && (g_tool == T_PANEL || g_dragLast[0] < 0))
		{
			Push();
			g_sel = Place(b.kind, b.variant, b.x, b.y, b.p);
			g_dragLast[0] = b.x;
			g_dragLast[1] = b.y;
			g_dragLast[2] = b.variant;
		}
	}
	else if (g_drag == D_ERASE && inside && (g_cell[0] != g_dragLast[0] || g_cell[1] != g_dragLast[1]))
	{
		g_dragLast[0] = g_cell[0];
		g_dragLast[1] = g_cell[1];
		EraseAt(g_cell[0], g_cell[1], g_dragPushed);
	}
	else if (g_drag == D_MOVE && g_hasHit)
	{
		Node* n = Find(g_sel);
		// a click that has not moved yet only selects: it must not pull the thing onto the grid
		if (n != nullptr && (g_dragPushed || std::hypot(g_hit[0] - g_pressHit[0], g_hit[1] - g_pressHit[1]) >= 0.2f))
		{
			int x, y;
			if (free) Snap(n->kind, n->variant, g_hit[0] + g_grabOff[0], g_hit[1] + g_grabOff[1], true, x, y);
			else Snap(n->kind, n->variant, g_hit[0], g_hit[1], false, x, y);
			if (x != n->x || y != n->y)
			{
				if (!g_dragPushed) { Push(); g_dragPushed = true; n = Find(g_sel); }
				n->x = x;
				n->y = y;
			}
		}
	}

	if (!over) return;
	// keys (stage_editor.py hotkeys): 1-7 tools, R turn, Del / X delete, Ctrl Z / Y / D / S, H J K L shift everything, Esc, Enter = apply
	if (in.ctrl)
	{
		if (devui::KeyPressed('Z')) { if (in.shift) Redo(); else Undo(); }
		if (devui::KeyPressed('Y')) Redo();
		if (devui::KeyPressed('D')) Duplicate();
		if (devui::KeyPressed('S')) Save();
		return;
	}
	for (int t = 0; t < T_COUNT; t++)
		if (devui::KeyPressed('1' + t)) g_tool = (Tool)t;
	if (devui::KeyPressed('R')) Turn();
	if (devui::KeyPressed(devui::K_DELETE) || devui::KeyPressed(devui::K_BACKSPACE) || devui::KeyPressed('X')) DeleteSelection();
	if (devui::KeyPressed('H')) ShiftAll(-1, 0);
	if (devui::KeyPressed('J')) ShiftAll(0, 1);
	if (devui::KeyPressed('K')) ShiftAll(0, -1);
	if (devui::KeyPressed('L')) ShiftAll(1, 0);
	if (devui::KeyPressed(devui::K_ESCAPE)) { g_sel = 0; g_tool = T_SELECT; }
	if (devui::KeyPressed(devui::K_ENTER)) Apply();
}

// ------------------------------------------------------------------------------------------------ drawing (3D pass)

std::vector<gfx::LineVertex> g_lines;
float g_slope[16];
void Line(const float a[3], const float b[3], unsigned rgba)   // map space
{
	gfx::LineVertex v{};
	for (int k = 0; k < 4; k++) v.rgba[k] = (std::uint8_t)(rgba >> (24 - 8 * k));
	ToWorld(g_slope, a[0], a[1], a[2], v.pos);
	g_lines.push_back(v);
	ToWorld(g_slope, b[0], b[1], b[2], v.pos);
	g_lines.push_back(v);
}
void Square(float x0, float z0, float x1, float z1, float y, unsigned rgba)
{
	const float p[4][3] = {{x0, y, z0}, {x1, y, z0}, {x1, y, z1}, {x0, y, z1}};
	for (int i = 0; i < 4; i++) Line(p[i], p[(i + 1) % 4], rgba);
}
void Cells(const std::vector<std::pair<int, int>>& cells, unsigned rgba, float inset = 0.06f)
{
	for (const auto& c : cells)
		if (c.first >= 0 && c.first < 16 && c.second >= 0 && c.second < 16)
			Square(c.first + inset, c.second + inset, c.first + 1 - inset, c.second + 1 - inset, 0.04f, rgba);
}
void Marker(const Node& n, unsigned rgba, float r)
{
	const float x = n.x / 16.0f, z = n.y / 16.0f, y = 0.4f;
	const float a[3] = {x - r, y, z}, b[3] = {x + r, y, z}, c[3] = {x, y, z - r}, d[3] = {x, y, z + r}, e[3] = {x, 0.0f, z}, f[3] = {x, y + r, z};
	Line(a, b, rgba);
	Line(c, d, rgba);
	Line(e, f, rgba);
	if (HasDirection(n.kind, n.variant))
	{
		static const float DX[4] = {0, 0, -1, 1}, DZ[4] = {-1, 1, 0, 0};
		const int k = n.p[0] & 3;
		const float tip[3] = {x + DX[k] * 0.6f, y, z + DZ[k] * 0.6f};
		const float s1[3] = {tip[0] - DX[k] * 0.2f + DZ[k] * 0.15f, y, tip[2] - DZ[k] * 0.2f + DX[k] * 0.15f};
		const float s2[3] = {tip[0] - DX[k] * 0.2f - DZ[k] * 0.15f, y, tip[2] - DZ[k] * 0.2f - DX[k] * 0.15f};
		const float o[3] = {x, y, z};
		Line(o, tip, rgba);
		Line(tip, s1, rgba);
		Line(tip, s2, rgba);
	}
}
unsigned KindColor(const Node& n)
{
	if (IsPanelNode(n)) return n.kind == KIND_GIMMICK ? 0xff6040ffu : n.variant <= 3 ? 0xb0b0b0ffu : n.variant == 4 ? 0x60a0ffffu : 0xc08040ffu;
	switch (n.kind)
	{
	case KIND_GIMMICK: return 0xffa000ffu;
	case KIND_ITEM: return 0x40ff40ffu;
	case KIND_PLAYER: return 0x40c0ffffu;
	case KIND_ENEMY: return 0xff4040ffu;
	default: return 0xffffffffu;
	}
}

// ------------------------------------------------------------------------------------------------ the panel

const std::vector<std::string> Labels(const std::vector<Named>& v)
{
	std::vector<std::string> out;
	for (const Named& n : v) out.push_back(n.name);
	return out;
}
int IndexOf(const std::vector<Named>& v, int value)
{
	for (std::size_t i = 0; i < v.size(); i++)
		if (v[i].v == value) return (int)i;
	return -1;
}

// one widget per parameter the node's kind uses; returns true when one changed (p is written)
bool Params(devui::UI& ui, int kind, int variant, int p[4], int idBase)
{
	bool changed = false;
	const std::array<Spec, 4> specs = Specs(kind, variant);
	for (int i = 0; i < 4; i++)
	{
		const Spec& s = specs[i];
		const devui::Id id = devui::MakeId("g07e.param", idBase, i);
		int v = p[i];
		switch (s.widget)
		{
		case '-':
			if (v != 0 || g_showUnused) v = (int)ui.Number(ui.Row(), id, "param " + std::to_string(i) + " (unused)", v, 1, "%.0f", true);
			break;
		case 'd':
		{
			std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
			for (int d = 0; d < 4; d++)
				if (ui.Button(c[d], DIRECTIONS[d], true, v == d)) v = d;
			break;
		}
		case 'c': v = ui.Choice(ui.Row(), id, s.label, *s.choices, std::clamp(v, 0, (int)s.choices->size() - 1)); break;
		case 'f': v = ui.Checkbox(ui.Row(), s.label, v != 0) ? 1 : 0; if ((v != 0) == (p[i] != 0)) v = p[i]; break;
		default:
		{
			static const double zero = 0.0;
			v = (int)ui.Number(ui.Row(), id, s.label, v, 1, "%.0f", true, kind == KIND_GROUP ? &zero : nullptr);
			break;
		}
		}
		if (v != p[i]) { p[i] = v; changed = true; }
	}
	if (*SpecHint(kind, variant)) ui.Label(SpecHint(kind, variant), &devui::Color(devui::DIM));
	g_showUnused = ui.Checkbox(ui.Row(), "Show unused parameters", g_showUnused);
	return changed;
}

// the formation of one enemy group: a row per step (type, then order / meet frames / move frames)
void FormationPanel(devui::UI& ui, int group)
{
	const std::vector<Node*> steps = FormationSteps(group);
	int leaders = 0, members = 0;
	for (Node* n : AllNodes())
		if (IsFormationMember(*n) && n->p[2] == group) { members++; leaders += n->p[1] == 1 ? 1 : 0; }
	ui.Heading("Formation of group " + std::to_string(group));
	ui.Label(std::to_string(leaders) + " leader(s), " + std::to_string(members - leaders) + " follower(s), " + std::to_string(steps.size()) + " step(s)",
	         &devui::Color(leaders > 0 && !steps.empty() ? devui::DIM : devui::WARN));
	const std::vector<std::string> names = Labels(FORMATIONS);
	for (std::size_t i = 0; i < steps.size(); i++)
	{
		const int uid = steps[i]->uid;
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {5, 1});
		const int cur = std::max(0, IndexOf(FORMATIONS, steps[i]->variant));
		const int pick = ui.Choice(c[0], devui::MakeId("g07e.ftype", uid), std::to_string(i + 1), names, cur);
		if (pick != cur) { Push(); Find(uid)->variant = FORMATIONS[pick].v; }
		if (ui.Button(c[1], "x")) { Push(); Remove(uid); return; }
		c = devui::UI::Split(ui.Row(), {1, 1, 1});
		static const char* const labels[] = {"order", "meet", "move"};
		static const double zero = 0.0;
		for (int k = 0; k < 3; k++)
		{
			Node* n = Find(uid);
			const int v = (int)ui.Number(c[k], devui::MakeId("g07e.fnum", uid, k), labels[k], n->p[k + 1], k == 0 ? 1 : 10, "%.0f", true, &zero);
			if (v != n->p[k + 1]) { Push(); Find(uid)->p[k + 1] = v; }
		}
	}
	if (ui.Button(ui.Row(), "Add a formation step"))
	{
		Push();
		Node n;
		n.kind = KIND_GROUP;
		n.variant = 2;
		n.x = n.y = 248;
		n.p[0] = group;
		n.p[1] = steps.empty() ? 0 : steps.back()->p[1] + 1;
		n.p[2] = n.p[3] = 120;
		AddNode(g_map, n);
	}
}

void ToolsPage(devui::UI& ui)
{
	for (int t = 0; t < T_COUNT; t += 4)
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
		for (int j = 0; j < 4 && t + j < T_COUNT; j++)
			if (ui.Button(c[j], std::to_string(t + j + 1) + " " + TOOL_NAMES[t + j], true, g_tool == t + j)) g_tool = (Tool)(t + j);
	}
	ui.Space(4);
	g_free = ui.Checkbox(ui.Row(), "Free 1/16 positions (Shift flips)", g_free);
	const int kind = TOOL_KIND[g_tool];
	if (kind == 0)
	{
		ui.Label(g_tool == T_SELECT ? "Click a thing to select it, drag to move it. A drag off the things pans the editor camera."
		                            : "Click or drag over cells to remove what is on top.",
		         &devui::Color(devui::DIM));
		return;
	}
	ui.Heading(std::string(TOOL_NAMES[g_tool]) + " brush");
	const std::vector<Named>& vars = Variants(kind);
	std::vector<devui::ListRow> rows;
	for (const Named& n : vars) rows.push_back(devui::ListRow{n.name, devui::Color(devui::TEXT)});
	const int cur = std::max(0, IndexOf(vars, g_brush[g_tool]));
	const int pick = ui.ListBox(devui::MakeId("g07e.brush", g_tool), ui.Row(devui::ROW * std::min<int>(8, (int)vars.size())), rows, cur, true);
	if (pick >= 0 && pick < (int)vars.size()) g_brush[g_tool] = vars[pick].v;
	Params(ui, kind, g_brush[g_tool], g_brushP, 1000 + g_tool);
	if (kind == KIND_ENEMY && IsFormationEnemy(g_brush[g_tool]) && (g_brushP[1] == 1 || g_brushP[1] == 2)) FormationPanel(ui, g_brushP[2]);
	if (kind == KIND_GIMMICK && g_brush[g_tool] == 2) ui.Label("Hatches sit on a panel corner and cover 4 x 4.", &devui::Color(devui::DIM));
	ui.Label("Left click / drag places (panels paint), right click removes the top thing. R turns the brush.", &devui::Color(devui::DIM));
}

void SelectionPage(devui::UI& ui)
{
	std::vector<Node*> list = AllNodes();
	std::vector<devui::ListRow> rows;
	int sel = -1;
	for (std::size_t i = 0; i < list.size(); i++)
	{
		rows.push_back(devui::ListRow{Describe(*list[i]), devui::Color(list[i]->uid == g_sel ? devui::TEXT : devui::DIM)});
		if (list[i]->uid == g_sel) sel = (int)i;
	}
	bool activated = false;
	const int picked = ui.ListBox(devui::MakeId("g07e.things"), ui.Row(devui::ROW * 8), rows, sel, true, nullptr, false, &activated);
	if (picked != sel && picked >= 0 && picked < (int)list.size()) g_sel = list[picked]->uid;
	Node* n = Find(g_sel);
	if (activated && n != nullptr)
	{
		// a double click looks at it (the editor camera)
		g_camOn = g_camInit = true;
		Slope(g_slope);
		ToWorld(g_slope, n->x / 16.0f, 0.0f, n->y / 16.0f, g_target);
		g_dist = std::min(g_dist, 10.0f);
	}
	if (n == nullptr)
	{
		ui.Label("nothing selected (Select tool, or pick one above)", &devui::Color(devui::DIM));
		return;
	}
	const int uid = n->uid;
	ui.Space();
	ui.Heading(std::string(KindName(n->kind)) + "  -  " + VariantName(n->kind, n->variant));
	const std::vector<Named>& vars = Variants(n->kind);
	if (!vars.empty())
	{
		const int cur = IndexOf(vars, n->variant);
		std::vector<std::string> names = Labels(vars);
		if (cur < 0) names.push_back(VariantName(n->kind, n->variant));
		const int pick = ui.Choice(ui.Row(), devui::MakeId("g07e.variant", uid), "variant", names, cur < 0 ? (int)vars.size() : cur);
		if (pick < (int)vars.size() && pick != cur) { Push(); Find(uid)->variant = vars[pick].v; }
	}
	static const double zero = 0.0;
	n = Find(uid);
	const int x = (int)ui.Number(ui.Row(), devui::MakeId("g07e.x", uid), "x (1/16 panel)", n->x, 1, "%.0f", true, &zero);
	const int y = (int)ui.Number(ui.Row(), devui::MakeId("g07e.y", uid), "y (1/16 panel)", n->y, 1, "%.0f", true, &zero);
	if (x != n->x || y != n->y) { Push(); n = Find(uid); n->x = x; n->y = y; }
	n = Find(uid);
	char buf[64];
	std::snprintf(buf, sizeof buf, "cell %d, %d   map %.2f, %.2f", n->x >> 4, n->y >> 4, n->x / 16.0f, n->y / 16.0f);
	ui.Label(buf, &devui::Color(devui::DIM));
	int p[4];
	std::copy(n->p, n->p + 4, p);
	if (Params(ui, n->kind, n->variant, p, uid)) { Push(); std::copy(p, p + 4, Find(uid)->p); }
	n = Find(uid);
	if (n != nullptr && IsFormationMember(*n)) FormationPanel(ui, n->p[2]);
	else if (n != nullptr && n->kind == KIND_GROUP) FormationPanel(ui, n->p[0]);
	ui.Space(4);
	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
	if (ui.Button(c[0], "Duplicate")) Duplicate();
	if (ui.Button(c[1], "Delete")) DeleteSelection();
	if (ui.Button(c[2], "Deselect")) g_sel = 0;
}

void StagePage(devui::UI& ui)
{
	ui.Heading("Panel light pattern");
	const int pat = Pattern();
	std::vector<std::string> names = Labels(PATTERNS);
	int cur = IndexOf(PATTERNS, pat);
	if (cur < 0) { names.push_back(pat < 0 ? "(none)" : "variant " + std::to_string(pat)); cur = (int)PATTERNS.size(); }
	const int pick = ui.Choice(ui.Row(), devui::MakeId("g07e.pattern"), "pattern", names, cur);
	if (pick != cur && pick < (int)PATTERNS.size()) SetPattern(PATTERNS[pick].v);
	ui.Heading("View");
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		g_grid = ui.Checkbox(c[0], "Grid", g_grid);
		g_outlines = ui.Checkbox(c[1], "Markers", g_outlines);
	}
	g_throughWalls = ui.Checkbox(ui.Row(), "Markers show through the models", g_throughWalls);
	ui.Heading("Shift everything one cell (H J K L)");
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
		if (ui.Button(c[0], "left")) ShiftAll(-1, 0);
		if (ui.Button(c[1], "down")) ShiftAll(0, 1);
		if (ui.Button(c[2], "up")) ShiftAll(0, -1);
		if (ui.Button(c[3], "right")) ShiftAll(1, 0);
	}
	ui.Heading("Clear");
	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
	static const char* const what[] = {"panels", "gimmicks", "items", "enemies"};
	for (int i = 0; i < 4; i++)
		if (ui.Button(c[i], what[i])) Clear(i);
}

void CheckPage(devui::UI& ui)
{
	const std::vector<Problem> probs = Check();
	if (probs.empty()) { ui.Label("nothing to report", &devui::Color(devui::GOOD)); return; }
	for (std::size_t i = 0; i < probs.size(); i++)
	{
		const Problem& q = probs[i];
		ui.Label((q.level >= 2 ? "! " : "? ") + q.text, &devui::Color(q.level >= 2 ? devui::BAD : devui::WARN));
		if (q.uid != 0 && ui.Button(ui.Row(), "Select it", true, q.uid == g_sel)) { g_sel = q.uid; g_tool = T_SELECT; }
	}
}

void EditorPanel(devui::UI& ui)
{
	if (!InStage()) return;
	g_panelFrame = FrameNo();
	const int stage = CurrentStage();
	if (g_stage < 0 || (g_stage != stage && !g_dirty)) LoadStage(stage);
	ui.Heading("Pac-Motos stage editor");
	ui.Label(StageName(g_stage) + "  -  " + std::to_string(AllNodes().size()) + " nodes" +
	         (g_appliedStage == g_stage ? (g_dirty ? "  -  edited since the last apply" : "  -  edits applied") : (g_dirty ? "  -  edited, not applied" : "")),
	         &devui::Color(g_dirty ? devui::WARN : devui::DIM));
	if (g_stage != stage)
	{
		ui.Label("The game is on " + StageName(stage) + "; the edits are for another stage.", &devui::Color(devui::WARN));
		if (ui.Button(ui.Row(), "Drop the edits and edit this stage")) LoadStage(stage);
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1.4f, 1, 0.9f});
		if (ui.Button(c[0], "Apply + restart (Enter)")) { Apply(); return; }
		if (ui.Button(c[1], "Back to the disc")) { Revert(); return; }
		if (ui.Button(c[2], "Restart")) { Restart(g_stage); g_msg = "restarted"; }
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
		if (ui.Button(c[0], "Undo", !g_undo.empty())) Undo();
		if (ui.Button(c[1], "Redo", !g_redo.empty())) Redo();
		if (ui.Button(c[2], "Save")) Save();
		if (ui.Button(c[3], "Load")) Load();
	}
	ui.Label("file: " + (EditDir(g_stage) / "MAPOO_OBJ").string(), &devui::Color(devui::DIM));
	g_editView = ui.Checkbox(ui.Row(), "Edit in the game view (the game gets no mouse / keys there)", g_editView);
	g_skipItemSet = ui.Checkbox(ui.Row(), "Skip the item settings (start with no items, no READY)", g_skipItemSet);
	g_preview = ui.Checkbox(ui.Row(), "Preview the edited map (models in place of the stage's, no tasks)", g_preview);
	if (g_preview && !g_prevMissing.empty())
	{
		std::string s = "not in this stage's archives (not drawn):";
		for (const std::string& k : g_prevMissing) s += " " + k;
		ui.Label(s, &devui::Color(devui::WARN));
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {2.2f, 1});
		int& level = G07LevelSetting();
		level = ui.Choice(c[0], devui::MakeId("g07e.level"), "Difficulty", {"Normal", "Easy", "Hard"}, level);
		static const char* const kLevel[3] = {"Normal", "Easy", "Hard"};
		const int now = KingdomGlobal::pMotosGameSys->nGameLevel();
		ui.Label(c[1], std::string("playing: ") + kLevel[std::clamp(now, 0, 2)], &devui::Color(now != level ? devui::WARN : devui::DIM));
	}
	ui.Label("The stage stands still while this tab is shown (the panels still animate); switch tabs or hide the window (F9) to play. On the item setup screen you have 99 power / jump / charge to pick from (the game takes at most 5 of each); your real stock comes back at the stage end.", &devui::Color(devui::DIM));
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {2.2f, 1});
		const bool cam = ui.Checkbox(c[0], "Editor camera (right drag turns, middle / left pans, wheel)", g_camOn);
		if (cam != g_camOn) { g_camOn = cam; g_camInit = false; }
		if (ui.Button(c[1], "Top view"))
		{
			// straight down onto the arena's centre (a hair off vertical: the orbit's yaw stays defined)
			g_camOn = g_camInit = true;
			Slope(g_slope);
			ToWorld(g_slope, 8.0f, 0.0f, 8.0f, g_target);
			g_yaw = 0.0f;
			g_pitch = 1.5f;
			g_dist = 24.0f;
		}
	}
	ui.Space();
	static const char* const pages[] = {"Tools", "Selection", "Stage", "Check"};
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1.3f, 1, 1});
		for (int i = 0; i < 4; i++)
			if (ui.Button(c[i], pages[i], true, g_page == i)) g_page = i;
	}
	ui.Space(4);
	switch (g_page)
	{
	case 0: ToolsPage(ui); break;
	case 1: SelectionPage(ui); break;
	case 2: StagePage(ui); break;
	default: CheckPage(ui); break;
	}
	ui.Space();
	if (!g_msg.empty()) ui.Label(g_msg, &devui::Color(devui::HINT));
	ui.Label("Game view: 1-7 tools, R turn, Del / X delete, Ctrl+Z / Y undo / redo, Ctrl+D duplicate, Ctrl+S save, H J K L shift, Enter apply.",
	         &devui::Color(devui::DIM));
	ViewInput();
}
}

// ------------------------------------------------------------------------------------------------ model preview (unpac motos models.py / stage.py)

// SetupPanelCB (cTaskPanelMan): panel variant -> model family (N panel A / B / C, X floor, W wall A..F) and turn in degrees
struct PanelModel { int v; char family; int sub; int turn; };
const PanelModel PANEL_MODELS[] = {{1, 'N', 0, 0}, {2, 'N', 1, 0}, {3, 'N', 2, 0}, {4, 'X', 0, 0}, {5, 'W', 0, 0}, {6, 'W', 1, 0}, {7, 'W', 2, 0},
                                   {9, 'W', 2, 90}, {11, 'W', 3, 0}, {12, 'W', 3, 180}, {13, 'W', 3, 90}, {14, 'W', 3, 270}, {15, 'W', 4, 0},
                                   {19, 'W', 5, 0}, {20, 'W', 5, 180}, {21, 'W', 5, 90}, {22, 'W', 5, 270}};
const int BLAST_TURN[4] = {0, 180, 90, 270};          // the blast panel (gimmick 4) by parameter 0
const int DIRECTION_TURN[4] = {180, 0, 270, 90};      // parameter 0 of players, enemies and gimmicks
const char* EnemyModel(int v)
{
	static const char* const k[25] = {nullptr, "MDL_CHAR_ENEMY_PUPA_RED", "MDL_CHAR_ENEMY_PUPA_BLUE", "MDL_CHAR_ENEMY_PUPA_BLACK", "MDL_CHAR_ENEMY_POLAR",
	                                  "MDL_CHAR_ENEMY_SPIRUS", "MDL_CHAR_ENEMY_FIRE_BUG", "MDL_CHAR_ENEMY_FIRE_BEE", "MDL_CHAR_ENEMY_TAITORIAN",
	                                  "MDL_CHAR_ENEMY_LADY_BUG", "MDL_CHAR_ENEMY_BEETLE", "MDL_CHAR_ENEMY_MEGA", "MDL_CHAR_ENEMY_GIGA", nullptr,
	                                  "MDL_CHAR_ENEMY_BEACON", "MDL_CHAR_ENEMY_NAVICON", "MDL_CHAR_BOSS_1_BODY", "MDL_CHAR_BOSS_2_CENT", "MDL_CHAR_BOSS_3_BODY",
	                                  "MDL_CHAR_BOSS_4_CENT", "MDL_CHAR_BOSS_5_BODY", "MDL_CHAR_GHOST_BLINKY", "MDL_CHAR_ENEMY_BALL_A", "MDL_CHAR_ENEMY_BALL_B",
	                                  "MDL_CHAR_ENEMY_PETA"};
	return v >= 0 && v < 25 ? k[v] : nullptr;
}

// cTaskPanelMan::FrameSet_*: the frame a panel's animation starts on, by light pattern (stage.py frame_set)
int PanelFrame(int pattern, int x, int z)
{
	const int mx = 240, w = 16, d = 16;
	switch (pattern)
	{
	case 0: case 1: return std::rand() % mx;
	case 2: case 3: return 0;
	case 4: return z * mx / d;
	case 5: return mx - z * mx / d;
	case 6: return x * mx / w;
	case 7: return mx - x * mx / w;
	case 8: return x * mx / w + z * mx / d;
	case 9: return x * mx / w + (mx - z * mx / d);
	case 10: return mx - x * mx / w + z * mx / d;
	case 11: return mx - x * mx / w + (mx - z * mx / d);
	case 12: return (x & 1) ? z * mx / d : mx - z * mx / d;
	case 13: return (z & 1) ? x * mx / w : mx - x * mx / w;
	case 14: return ((x + z) & 1) ? x * mx / w + (mx - z * mx / d) : mx - x * mx / w + z * mx / d;
	case 15: return ((x + z) & 1) ? x * mx / w + z * mx / d : mx - x * mx / w + (mx - z * mx / d);
	case 16: return ((x + z) & 1) ? 0 : mx >> 1;
	case 17:
		if (x < w / 2) { x += 1; return z >= d / 2 ? mx - x * mx / w + z * mx / d + 60 : mx - x * mx / w + (mx - z * mx / d) + 60; }
		return z >= d / 2 ? x * mx / w + z * mx / d + 60 : x * mx / w + (mx - z * mx / d) + 60;
	case 18:
		if (x < w / 2) return z >= d / 2 ? x * mx / w + (mx - z * mx / d) : x * mx / w + z * mx / d;
		return z >= d / 2 ? mx - x * mx / w + (mx - z * mx / d) : mx - x * mx / w + z * mx / d;
	}
	return 0;
}

struct PreviewObj
{
	kingdom::cKingdomModel* model = nullptr;
	float local[16];                                  // map space (column major), the slope matrix goes in front each frame
};
std::vector<PreviewObj> g_prev;
std::size_t g_prevHash = 0;
const void* g_prevOwner = nullptr;

bool HasEntry(const std::string& key)
{
	std::size_t size = 0;
	return wii::ArchiveStore::Find(key, size) != nullptr;
}

// a model with the animations of its name (TSA_ / CLA_ / CHA_ / TPA_ + the name after MDL_), as the game's Create functions register them
kingdom::cKingdomModel* PreviewModel(const std::string& key, int lightSet, const char* chr = nullptr)
{
	if (!HasEntry(key))
	{
		if (std::find(g_prevMissing.begin(), g_prevMissing.end(), key) == g_prevMissing.end()) g_prevMissing.push_back(key);
		return nullptr;
	}
	kingdom::cKingdomModel* m = new kingdom::cKingdomModel();
	m->RegistModel(key);
	const std::string name = key.substr(4);
	for (const char* prefix : {"TSA_", "CLA_", "CHA_", "TPA_"})
		if (chr == nullptr || std::strcmp(prefix, "CHA_") != 0)
			if (HasEntry(prefix + name)) m->RegistAnm(0u, prefix + name);
	if (chr != nullptr && HasEntry(chr)) m->RegistAnm(0u, chr);
	m->SetLightSetIdx(lightSet);
	m->SetFogIdx(lightSet);
	m->IsDrawShadowDepth(lightSet == 2);
	m->Visible();
	return m;
}

// column-major a * b
void Mul4(const float a[16], const float b[16], float out[16])
{
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
		{
			float s = 0;
			for (int k = 0; k < 4; k++) s += a[k * 4 + r] * b[c * 4 + k];
			out[c * 4 + r] = s;
		}
}

void LocalMatrix(float x, float z, int turnDeg, float out[16])
{
	const float r = (float)turnDeg * 3.14159265f / 180.0f, c = std::cos(r), s = std::sin(r);
	const float m[16] = {c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, x, 0, z, 1};
	std::memcpy(out, m, sizeof m);
}

void AddPreview(kingdom::cKingdomModel* m, float x, float z, int turn)
{
	if (m == nullptr) return;
	PreviewObj o;
	o.model = m;
	LocalMatrix(x, z, turn, o.local);
	g_prev.push_back(o);
}

void ClearPreview()
{
	for (PreviewObj& o : g_prev) delete o.model;
	g_prev.clear();
	g_prevHash = 0;
	g_prevMissing.clear();
}

std::size_t MapHash()
{
	std::size_t h = (std::size_t)g_stage * 1000003u;
	for (Node* n : AllNodes())
	{
		const int v[8] = {n->kind, n->variant, n->x, n->y, n->p[0], n->p[1], n->p[2], n->p[3]};
		for (int i : v) h = h * 31u + (std::size_t)(unsigned)i;
	}
	return h | 1u;
}

// stage.py rebuild + models.py: what the map's nodes would put into the arena
void BuildPreview()
{
	ClearPreview();
	const int pattern = std::max(Pattern(), 1);
	std::set<std::pair<int, int>> seen;
	for (Node* np : AllNodes())
	{
		const Node& n = *np;
		const int cx = n.x >> 4, cz = n.y >> 4;
		const int dir = n.p[0] >= 0 && n.p[0] < 4 ? n.p[0] : 0;
		if (IsPanelNode(n))
		{
			if (cx < 0 || cx >= 16 || cz < 0 || cz >= 16 || !seen.insert({cx, cz}).second) continue;   // SetupPanel keeps a cell's first panel
			std::string key;
			int turn = 0;
			if (n.kind == KIND_GIMMICK) { key = "MDL_GIMMICK_EXPLODED"; turn = BLAST_TURN[dir]; }
			else
			{
				const PanelModel* pm = nullptr;
				for (const PanelModel& p : PANEL_MODELS)
					if (p.v == n.variant) pm = &p;
				if (pm == nullptr) continue;
				turn = pm->turn;
				key = pm->family == 'N' ? std::string("MDL_PANEL_") + "ABC"[pm->sub] : pm->family == 'X' ? std::string("MDL_PANEL_X") : std::string("MDL_PANEL_WALL_") + "ABCDEF"[pm->sub];
			}
			kingdom::cKingdomModel* m = PreviewModel(key, 1);
			if (m != nullptr) m->SetFrameAnmAll(0u, (float)PanelFrame(pattern, cx, cz));
			AddPreview(m, cx + 0.5f, cz + 0.5f, turn);
		}
		else if (n.kind == KIND_GIMMICK)
		{
			const int v = n.variant == 5 || n.variant == 6 ? 1 : n.variant;
			static const char* const g[4] = {nullptr, "MDL_GIMMICK_REACTION", "MDL_GIMMICK_HATCH", "MDL_GIMMICK_SWITCH"};
			if (v >= 1 && v <= 3) AddPreview(PreviewModel(g[v], 1), n.x / 16.0f, n.y / 16.0f, DIRECTION_TURN[dir]);
		}
		else if (n.kind == KIND_ITEM)
		{
			static const char* const k[4] = {nullptr, "MDL_ITEM_POWER", "MDL_ITEM_MOBILITY", "MDL_ITEM_FORCE"};
			if (n.variant >= 1 && n.variant <= 3) AddPreview(PreviewModel(k[n.variant], 2), n.x / 16.0f, n.y / 16.0f, 0);
		}
		else if (n.kind == KIND_ENEMY)
		{
			if (const char* key = EnemyModel(n.variant)) AddPreview(PreviewModel(key, 2), n.x / 16.0f, n.y / 16.0f, DIRECTION_TURN[dir]);
		}
		else if (n.kind == KIND_PLAYER)
		{
			const int p = std::clamp(n.variant, 1, 4);
			kingdom::cKingdomModel* m = PreviewModel("MDL_CHAR_PACMAN_STANDARD_" + std::to_string(p) + "P", 2, "CHA_CHAR_PACMAN_01_STANDARD");
			AddPreview(m, n.x / 16.0f, n.y / 16.0f, DIRECTION_TURN[dir]);
			// Pac-Man's origin is the middle of the ball; the game stands his node "root" on the panel (models.py player_matrix)
			float root[12];
			if (m != nullptr && m->NodeMatrix("root", root))
			{
				float* l = g_prev.back().local;
				for (int r = 0; r < 3; r++) l[12 + r] -= l[r] * root[3] + l[4 + r] * root[7] + l[8 + r] * root[11];
			}
		}
	}
	g_prevHash = MapHash();
}

void G07EditOverride(const std::string& key, const std::uint8_t*& data, std::size_t& size)
{
	if (g_appliedStage < 0 || key != "MAPOO_OBJ" || g_applied.empty()) return;
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	if (core == nullptr || core->GetJobNo() != kingdom::job::MOTOS_GAME || KingdomGlobal::MotosSys == nullptr || CurrentStage() != g_appliedStage) return;
	if (KingdomGlobal::g_pMotosGame != nullptr && KingdomGlobal::g_pMotosGame->bMultiPlay()) return;
	data = g_applied.data();
	size = g_applied.size();
}

void G07EditCamera(float eye[3], float target[3], float& farZ)
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
	farZ = std::max(farZ, g_dist + 500.0f);
	const float cp = std::cos(g_pitch);
	for (int i = 0; i < 3; i++) target[i] = g_target[i];
	eye[0] = g_target[0] + g_dist * cp * std::sin(g_yaw);
	eye[1] = g_target[1] + g_dist * std::sin(g_pitch);
	eye[2] = g_target[2] + g_dist * cp * std::cos(g_yaw);
}

void G07EditDraw3D()
{
	if (!Active() || g_stage < 0 || !g_editView) return;
	g_lines.clear();
	Slope(g_slope);
	if (g_grid)
		for (int i = 0; i <= 16; i++)
		{
			const float a[3] = {(float)i, 0.02f, 0.0f}, b[3] = {(float)i, 0.02f, 16.0f}, c[3] = {0.0f, 0.02f, (float)i}, d[3] = {16.0f, 0.02f, (float)i};
			const unsigned col = i % 4 == 0 ? 0xffffffa0u : 0xffffff50u;
			Line(a, b, col);
			Line(c, d, col);
		}
	const Node* sel = Find(g_sel);
	if (g_outlines)
		for (const Node* n : AllNodes())
		{
			if (n->kind == KIND_PATTERN || n->kind == KIND_GROUP || n == sel) continue;
			if (IsPanelNode(*n)) Cells(Footprint(*n), KindColor(*n), 0.12f);
			else if (IsHatch(*n)) { Cells(Footprint(*n), KindColor(*n), 0.2f); Marker(*n, KindColor(*n), 0.3f); }
			else Marker(*n, KindColor(*n), 0.3f);
		}
	if (sel != nullptr)
	{
		Cells(Footprint(*sel), 0xffe650ffu, 0.03f);
		if (!IsPanelNode(*sel)) Marker(*sel, 0xffe650ffu, 0.5f);
		if (IsFormationMember(*sel))
		{
			// the selected enemy's formation: lines from the leader to every follower
			const Node* lead = nullptr;
			for (const Node* m : AllNodes())
				if (IsFormationMember(*m) && m->p[2] == sel->p[2] && m->p[1] == 1) lead = m;
			for (const Node* m : AllNodes())
				if (lead != nullptr && m != lead && IsFormationMember(*m) && m->p[2] == sel->p[2])
				{
					const float a[3] = {lead->x / 16.0f, 0.6f, lead->y / 16.0f}, b[3] = {m->x / 16.0f, 0.6f, m->y / 16.0f};
					Line(a, b, 0x50dcffffu);
				}
		}
	}
	// the cursor: what the brush would place, the cell an erase hits, or the cell a click selects
	if (g_hasHit && g_cell[0] >= 0 && g_cell[0] < 16 && g_cell[1] >= 0 && g_cell[1] < 16 && g_drag != D_MOVE)
	{
		Node b;
		if (BrushNode(b)) { Cells(Footprint(b), 0x5aff8cffu, 0.0f); if (!IsPanelNode(b)) Marker(b, 0x5aff8cffu, 0.35f); }
		else Cells({{g_cell[0], g_cell[1]}}, g_tool == T_ERASE ? 0xff5a5affu : 0x78aaffffu, 0.0f);
	}
	if (!g_lines.empty()) gfx::DrawLines3D(g_lines.data(), g_lines.size(), !g_throughWalls);
}

bool G07EditFrozen() { return Active(); }

void G07EditPreviewFrame(std::vector<kingdom::cKingdomModelBase*>& frame)
{
	const void* owner = InStage() ? (const void*)KingdomGlobal::g_pMotosGame : nullptr;
	if (owner != g_prevOwner) { ClearPreview(); g_prevOwner = owner; }   // a new stage: the old models go (before anything draws them)
	if (!g_preview || !Active() || g_stage != CurrentStage() || g_map.layers.empty()) return;
	if (MapHash() != g_prevHash) BuildPreview();
	frame.erase(std::remove_if(frame.begin(), frame.end(), [](kingdom::cKingdomModelBase* m) { const int l = m->GetLightSetIdx(); return l == 1 || l == 2; }),
	            frame.end());
	float slope[16];
	Slope(slope);
	for (PreviewObj& o : g_prev)
	{
		float w[16];
		Mul4(slope, o.local, w);
		o.model->SetNativeMtxRaw(w);
		o.model->UpdateFrame();
		o.model->UpdateMatrix();
	}
}
bool G07SkipItemSet() { return g_skipItemSet && InStage(); }

int& G07LevelSetting()
{
	static int level = 0;
	return level;
}

int G07Level()
{
	if (KingdomGlobal::MotosSys == nullptr || KingdomGlobal::MotosSys->nStageIdx() >= 36) return 0;
	return std::clamp(G07LevelSetting(), 0, 2);
}

std::string G07LevelCsv(const char* normalName)
{
	static const char* const kNames[3] = {"NORMAL", "EASY", "HARD"};
	std::string s = normalName;
	const std::size_t at = s.find("NORMAL");
	if (at != std::string::npos) s.replace(at, 6, kNames[G07Level()]);
	return s;
}

namespace
{
bool g_itemSwapped = false;
unsigned char g_realItems[4] = {};
}

void G07EditItemStock(unsigned char* itemNum)
{
	if (!Active()) return;
	if (!g_itemSwapped)
	{
		for (int i = 1; i < 4; i++) g_realItems[i] = itemNum[i];
		g_itemSwapped = true;
	}
	for (int i = 1; i < 4; i++) itemNum[i] = 99;
}

bool G07EditItemRestore(unsigned char* itemNum)
{
	if (!g_itemSwapped) return false;
	for (int i = 1; i < 4; i++) itemNum[i] = g_realItems[i];
	g_itemSwapped = false;
	return true;
}

void RegisterG07EditPanel() { devui::RegisterPanel("Motos editor", EditorPanel, [] { return InStage(); }); }
}
