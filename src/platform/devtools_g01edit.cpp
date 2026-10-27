// The dev window's "Galaga editor" tab (devui, --devui / F9), shown inside a single-player Galaga course stage (job GALAGA_GAME, stages 1..9;
// not the boss stage 10): a C++ port of unpac's kingdom/galaga editor.py with the parts of scn.py / emy.py / orbit.py / stage.py it uses (the
// records, the orbit keys, the orbit follower for the path, the CourseSim spawn times, the checks). unpac's own playback (its stage view, enemy
// models, Pac-Man course) is not ported: the running game is the playback. While the tab is shown the stage stands still (the game's pause flag,
// G01EditFrozen); the handles live in the game camera's frame at that moment (g01 cTaskCamera::LocalToWorld), like unpac's at its playhead.
// "Play to" lets the stage run until the course reaches a record (restarting it when that is behind). "Apply" hands the edited scn to cTaskScene
// (G01EditOverride) and the edited orbit table to cOrbitDataMan::Load (G01EditOrbits), only for the edited stage, and restarts it (GALAGA_GAME again,
// the retry path). Save / Load keep a stage's scn, orbits and EnemyOrbit csv in a folder ($MEGAMIX_G01_EDITS, else $XDG_DATA_HOME/megamix/g01_edits,
// else ~/.local/share/megamix/g01_edits). Not ported: Export (unpac's .lzs packing), clicking a live enemy to select its record. NOT part of the
// 1:1 game.
//
// scn (scn.py): headerless big endian, 0x30 bytes per record: parent xyz, (kept), list index, course time, course speed, Pac-Man speed, local
// delay, the word (bits 30-31 class: 2 spawn, 0 command; command bits 0-14: 0 advance, 1 ramp, 2 wait, 3 gate; spawn bits 10-19 enemy type,
// bits 0-9 orbit = row of EnemyOrbit_s%x.csv), parameter 1 (gate: last group), parameter 2 (spawn: bits 0-8 group, 9-10 marker).
// emy (emy.py): headerless big endian, 0x40 bytes per key: position, rotation (radians), Hermite tangent (4 floats each), frame, command (bits
// 0-15 interpolation 0 linear / 1 Hermite / 2 jump / 3 hold, 16-27 jump node, 28-31 facing), parameter (fire pattern), zero.
// Records and keys are kept as raw words, so a file written back is byte identical.

#include "platform/devtools.hpp"
#include "platform/devtools_g01edit.hpp"
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
#include <sstream>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/Gfx3D.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskCamera.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "wii/Archive.hpp"

namespace devtools
{
namespace
{
using Bytes = std::vector<std::uint8_t>;
namespace fs = std::filesystem;
namespace g01 = kingdom::game::g01;

std::uint32_t Rd32(const std::uint8_t* b) { return (std::uint32_t)b[0] << 24 | (std::uint32_t)b[1] << 16 | (std::uint32_t)b[2] << 8 | b[3]; }
void Wr32(std::uint8_t* b, std::uint32_t v) { for (int i = 0; i < 4; i++) b[i] = (std::uint8_t)(v >> (24 - 8 * i)); }
float WordF(std::uint32_t w) { float f; std::memcpy(&f, &w, 4); return f; }
std::uint32_t FWord(float f) { std::uint32_t w; std::memcpy(&w, &f, 4); return w; }

// ------------------------------------------------------------------------------------------------ scn records (scn.py)

constexpr int REC_SIZE = 0x30;
enum { CMD_ADVANCE = 0, CMD_RAMP = 1, CMD_WAIT = 2, CMD_GATE = 3 };
const std::vector<std::string> CMD_LABELS = {"ADVANCE", "RAMP", "WAIT", "GATE"};
const std::vector<std::string> INTERP_LABELS = {"linear", "hermite", "jump", "hold"};
const std::vector<std::string> FACING_LABELS = {"node rotation", "face target", "from camera", "to camera"};
const std::vector<std::string> ENEMY_NAMES = {"zako", "goei", "boss", "gyalboss", "root", "queen", "midori", "momiji", "bomb", "queen2", "fighter",
                                              "lastboss", "gate", "spike", "spikemaster"};

struct Rec
{
	std::array<std::uint8_t, REC_SIZE> raw{};
	float F(int off) const { return WordF(Rd32(raw.data() + off)); }
	std::uint32_t U(int off) const { return Rd32(raw.data() + off); }
	void SetF(int off, float v) { Wr32(raw.data() + off, FWord(v)); }
	void SetU(int off, std::uint32_t v) { Wr32(raw.data() + off, v); }

	float Parent(int i) const { return F(4 * i); }
	std::uint32_t Index() const { return U(0x10); }
	float CourseTime() const { return F(0x14); }
	float CourseSpeed() const { return F(0x18); }
	float PacmanSpeed() const { return F(0x1c); }
	float Delay() const { return F(0x20); }
	std::uint32_t Word() const { return U(0x24); }
	std::uint32_t P1() const { return U(0x28); }
	std::uint32_t P2() const { return U(0x2c); }

	bool IsSpawn() const { return Word() >> 30 == 2; }
	bool IsCommand() const { return Word() >> 30 == 0; }
	int Cmd() const { return (int)(Word() & 0x7fff); }
	bool IsRamp() const { return IsCommand() && Cmd() == CMD_RAMP; }
	bool IsWait() const { return IsCommand() && Cmd() == CMD_WAIT; }
	bool IsGate() const { return IsCommand() && Cmd() == CMD_GATE; }
	int Kind() const { return (int)(Word() >> 10 & 0x3ff); }
	int OrbitId() const { return (int)(Word() & 0x3ff); }
	int Group() const { return (int)(P2() & 0x1ff); }
	int Marker() const { return (int)(P2() >> 9 & 3); }
	void SetKind(int k) { SetU(0x24, (Word() & ~(0x3ffu << 10)) | ((std::uint32_t)k & 0x3ff) << 10); }
	void SetOrbit(int o) { SetU(0x24, (Word() & ~0x3ffu) | ((std::uint32_t)o & 0x3ff)); }
	void SetGroup(int g) { SetU(0x2c, (P2() & ~0x1ffu) | ((std::uint32_t)g & 0x1ff)); }
	void SetMarker(int m) { SetU(0x2c, (P2() & ~(3u << 9)) | ((std::uint32_t)m & 3) << 9); }
};

Rec NewSpawn(int kind, int orbit, const float parent[3], float ct, float delay, int group)
{
	Rec r;
	for (int i = 0; i < 3; i++) r.SetF(4 * i, parent[i]);
	r.SetF(0x14, ct);
	r.SetF(0x20, delay);
	r.SetU(0x24, 2u << 30 | ((std::uint32_t)kind & 0x3ff) << 10 | ((std::uint32_t)orbit & 0x3ff));
	r.SetU(0x2c, (std::uint32_t)group & 0x1ff);
	return r;
}
Rec NewCommand(int cmd, float ct, float speed, float delay)
{
	Rec r;
	r.SetF(0x14, ct);
	r.SetF(0x18, speed);
	r.SetF(0x1c, speed);
	r.SetF(0x20, delay);
	r.SetU(0x24, (std::uint32_t)cmd & 0x7fff);
	return r;
}

// ------------------------------------------------------------------------------------------------ emy keys (emy.py)

constexpr int KEY_SIZE = 0x40;
struct Key
{
	std::array<std::uint32_t, 16> w{};
	float Pos(int i) const { return WordF(w[i]); }
	float Rot(int i) const { return WordF(w[4 + i]); }
	float Tan(int i) const { return WordF(w[8 + i]); }
	float Frame() const { return WordF(w[12]); }
	int Interp() const { return (int)(w[13] & 0xffff); }
	int JumpTo() const { return (int)(w[13] >> 16 & 0xfff); }
	int Facing() const { return (int)(w[13] >> 28); }
	std::uint32_t Param() const { return w[14]; }
	void SetPos(const float p[3]) { for (int i = 0; i < 3; i++) w[i] = FWord(p[i]); }
	void SetRot(const float r[3]) { for (int i = 0; i < 3; i++) w[4 + i] = FWord(r[i]); }
	void SetTan(const float t[3]) { for (int i = 0; i < 3; i++) w[8 + i] = FWord(t[i]); }
	void SetFrame(float f) { w[12] = FWord(f); }
	void SetCommand(int interp, int facing, int jump) { w[13] = ((std::uint32_t)interp & 0xffff) | ((std::uint32_t)jump & 0xfff) << 16 | ((std::uint32_t)facing & 0xf) << 28; }
};

Key MakeKey(const float pos[3], const float rot[3], const float tan[3], float frame, int interp, int facing)
{
	Key k;
	k.SetPos(pos);
	k.SetRot(rot);
	k.SetTan(tan);
	k.SetFrame(frame);
	k.SetCommand(interp, facing, 0);
	return k;
}

struct Orbit
{
	std::string name;           // the .emy file of the csv row
	bool exists = false;        // the file was found (the game loads an empty orbit otherwise)
	std::vector<Key> keys;
};

std::vector<Key> ParseEmy(const std::uint8_t* d, std::size_t size)
{
	std::vector<Key> out;
	for (std::size_t o = 0; d != nullptr && o + KEY_SIZE <= size; o += KEY_SIZE)
	{
		Key k;
		for (int i = 0; i < 16; i++) k.w[i] = Rd32(d + o + 4 * i);
		out.push_back(k);
	}
	return out;
}
Bytes EmyBytes(const Orbit& o)
{
	Bytes b(o.keys.size() * KEY_SIZE);
	for (std::size_t i = 0; i < o.keys.size(); i++)
		for (int j = 0; j < 16; j++) Wr32(b.data() + i * KEY_SIZE + 4 * j, o.keys[i].w[j]);
	return b;
}

// a node after node `after`, halfway to the next one in time and position (or a step past the last); returns its index (Emy.add_node)
int AddKey(Orbit& o, int after)
{
	const float zero[3] = {0, 0, 0}, turn[3] = {0, 3.14159265f, 0};
	if (o.keys.empty()) { o.keys.push_back(MakeKey(zero, turn, zero, 0.0f, 0, 0)); return 0; }
	after = std::clamp(after, 0, (int)o.keys.size() - 1);
	const Key& a = o.keys[(std::size_t)after];
	float pos[3], rot[3], tan[3];
	for (int i = 0; i < 3; i++) rot[i] = a.Rot(i);
	Key n;
	if (after + 1 < (int)o.keys.size())
	{
		const Key& b = o.keys[(std::size_t)after + 1];
		for (int i = 0; i < 3; i++) { pos[i] = (a.Pos(i) + b.Pos(i)) / 2; tan[i] = (a.Tan(i) + b.Tan(i)) / 2; }
		n = MakeKey(pos, rot, tan, (a.Frame() + b.Frame()) / 2, a.Interp(), a.Facing());
	}
	else
	{
		for (int i = 0; i < 3; i++) { pos[i] = a.Pos(i); tan[i] = a.Tan(i); }
		pos[2] += 5.0f;
		n = MakeKey(pos, rot, tan, a.Frame() + 30.0f, a.Interp(), a.Facing());
	}
	o.keys.insert(o.keys.begin() + after + 1, n);
	return after + 1;
}

// a two node orbit: from the spawn point straight ahead for 120 frames (Emy.new)
std::vector<Key> NewOrbitKeys()
{
	const float a[3] = {0, 0, 0}, b[3] = {0, 0, 20}, turn[3] = {0, 3.14159265f, 0}, zero[3] = {0, 0, 0};
	return {MakeKey(a, turn, zero, 0.0f, 0, 1), MakeKey(b, turn, zero, 120.0f, 0, 1)};
}

// ------------------------------------------------------------------------------------------------ the orbit follower (orbit.py, cTaskEnemy::UpdateOrbit)

// the local positions along an orbit, one per `every` game frames (sample_orbit; position only: the facing needs the camera)
std::vector<std::array<float, 3>> SampleOrbit(const Orbit& o, int every = 2, int limit = 3000)
{
	std::vector<std::array<float, 3>> out;
	const std::vector<Key>& k = o.keys;
	if (k.size() < 2) return out;
	auto node = [&](int i) -> const Key* { return i >= 0 && i < (int)k.size() ? &k[(std::size_t)i] : nullptr; };
	float time = 0.0f, pos[3] = {k[0].Pos(0), k[0].Pos(1), k[0].Pos(2)};
	int idx = 1;
	const Key* cur = node(0);
	const Key* nxt = node(1);
	for (int n = 0; n < limit; n++)
	{
		bool finished = false;
		for (int guard = 0; guard < 4096; guard++)
		{
			bool passed = false, again = false;
			const int interp = cur->Interp();
			if (interp == 2)
			{
				idx = cur->JumpTo();
				cur = node(idx);
				idx++;
				nxt = node(idx);
				if (cur == nullptr || nxt == nullptr) { finished = true; break; }
				for (int i = 0; i < 3; i++) pos[i] = cur->Pos(i);
				time = cur->Frame();
			}
			else if (interp == 0 || interp == 1)
			{
				float t = nxt->Frame() - cur->Frame();
				if (t != 0.0f) t = (time - cur->Frame()) / t;
				if (interp == 0)
					for (int i = 0; i < 3; i++) pos[i] = cur->Pos(i) + (nxt->Pos(i) - cur->Pos(i)) * t;
				else
				{
					// OrbitHermite (801c1be8): the tangents are used as stored
					const float t2 = t * t, t3 = t2 * t;
					const float h00 = 1.0f + (2.0f * t3 - 3.0f * t2), h10 = t - (2.0f * t2 - t3), h01 = 3.0f * t2 - 2.0f * t3, h11 = t3 - t2;
					for (int i = 0; i < 3; i++) pos[i] = h11 * nxt->Tan(i) + h10 * cur->Tan(i) + h00 * cur->Pos(i) + h01 * nxt->Pos(i);
				}
				passed = nxt->Frame() <= time;
			}
			else if (interp == 3)
			{
				for (int i = 0; i < 3; i++) pos[i] = cur->Pos(i);
				passed = nxt->Frame() <= time;
			}
			if (passed)
			{
				const Key* was = nxt;
				idx++;
				cur = nxt;
				nxt = node(idx);
				if (nxt == nullptr) { finished = true; break; }
				if (was->Frame() <= time) again = true;
			}
			if (!again) break;
		}
		if (finished) break;
		time += 1.0f;
		if (n % every == 0) out.push_back({pos[0], pos[1], pos[2]});
	}
	return out;
}

// ------------------------------------------------------------------------------------------------ the course (stage.py CourseSim, cTaskScene::CourseStep)

constexpr int MAX_GAME_FRAMES = 60 * 60 * 20;   // a course that never finishes (speed 0) stops after this
constexpr float MIN_SPEED = 0.01f;              // a ramp speed in [-1, 0.01) is raised to this

struct Sim
{
	std::map<std::uint32_t, int> spawnFrame;    // record index -> game frame of its spawn
	std::map<std::uint32_t, int> commandFrame;  // record index of a command -> the game frame it starts running
	int frames = 0;
	bool stalled = false;
};

Sim RunSim(const std::vector<Rec>& recs)
{
	Sim s;
	std::vector<const Rec*> cmds;
	for (const Rec& r : recs)
		if (r.IsCommand()) cmds.push_back(&r);
	if (cmds.empty()) { s.frames = 1; return s; }
	std::map<int, std::vector<const Rec*>> bySeg;
	for (const Rec& r : recs)
	{
		if (!r.IsSpawn()) continue;
		int seg = -1;
		for (int i = 0; i < (int)cmds.size(); i++)
			if (cmds[(std::size_t)i]->Index() < r.Index()) seg = i;
		if (seg >= 0) bySeg[seg].push_back(&r);
	}
	int ci = 0;
	const Rec* cur = cmds[0];
	const Rec* nxt = cmds.size() > 1 ? cmds[1] : nullptr;
	float speed = cur->CourseSpeed(), t = 0.0f, timer = 0.0f, prevTimer = -1.0f, prevT = -1.0f;
	s.commandFrame[cur->Index()] = 0;
	bool ended = false;
	int g = 0;
	for (; g < MAX_GAME_FRAMES; g++)
	{
		const float tNow = t, speedNow = speed;
		const Rec* oldCur = cur;
		const Rec* oldNxt = nxt;
		const int oldCi = ci;
		const int cmd = cur->Cmd();
		bool done = false;
		if (cmd == CMD_WAIT)
			done = cur->Delay() <= timer;
		else if (cmd == CMD_GATE)
		{
			if (speedNow == 0.0f || nxt == nullptr) done = cur->Delay() <= timer;   // the player is assumed to clear the gate at once
			else done = nxt->CourseTime() - cur->CourseTime() >= 0 ? nxt->CourseTime() <= tNow : tNow <= nxt->CourseTime();
		}
		else if (cmd == CMD_RAMP)
		{
			if (nxt != nullptr)
			{
				const float span = nxt->CourseTime() - cur->CourseTime();
				const float f = span != 0.0f ? (tNow - cur->CourseTime()) / span : 1.0f;
				speed = f * (nxt->CourseSpeed() - cur->CourseSpeed()) + cur->CourseSpeed();
				if (-1.0f <= speed && speed < MIN_SPEED) speed = MIN_SPEED;
				done = nxt->CourseTime() <= tNow;
			}
		}
		else
			done = nxt != nullptr && nxt->CourseTime() <= tNow;

		if (done && nxt != nullptr)
		{
			timer = 0.0f;
			cur = nxt;
			ci++;
			nxt = ci + 1 < (int)cmds.size() ? cmds[(std::size_t)ci + 1] : nullptr;
			speed = cur->CourseSpeed();
			if (!s.commandFrame.count(cur->Index())) s.commandFrame[cur->Index()] = g + 1;
		}
		else if (!done)
			timer += 1.0f;

		if (oldNxt != nullptr)   // the spawns of the segment that was running at the frame's start
		{
			for (const Rec* sp : bySeg[oldCi])
			{
				if (s.spawnFrame.count(sp->Index()) || !(oldCur->Index() < sp->Index() && sp->Index() < oldNxt->Index())) continue;
				const bool hit = speedNow == 0.0f ? (prevTimer < sp->Delay() && sp->Delay() <= timer) : (prevT < sp->CourseTime() && sp->CourseTime() <= tNow);
				if (hit) s.spawnFrame[sp->Index()] = g;
			}
			prevTimer = timer;
			prevT = tNow;
		}
		if (nxt == nullptr && !done && cur == cmds.back() && ci == (int)cmds.size() - 1) { ended = true; break; }
		t += speed;
	}
	s.frames = g + 1;
	s.stalled = !ended;
	return s;
}

// ------------------------------------------------------------------------------------------------ editor state

struct Data
{
	std::vector<Rec> recs;
	std::vector<Orbit> orbits;   // index = row of EnemyOrbit_s%x.csv = the orbit number of a spawn
};

Data g_data;
int g_stage = -1;               // cG01GalagaSys::nStage the data was read for
std::string g_scnName;          // scn_s%xp%d.scn
std::string g_csvRaw;           // the disc's EnemyOrbit csv, kept for Save (its other lines stay)
std::vector<Data> g_undo, g_redo;
Sim g_sim;
Data g_applied;
Bytes g_appliedScn;
std::string g_appliedName;
int g_appliedStage = -1;
bool g_dirty = false;
std::string g_msg;
int g_panelFrame = -100;
int g_page = 0;                 // Scene / Orbit / Check
int g_filter = 0;               // all, spawns, commands
int g_sel = 0;                  // the selected record (position in the file)
int g_node = 0;                 // the selected node of the selected spawn's orbit
bool g_reveal = true;
bool g_editView = true, g_freeze = true;
float g_runTo = -1.0f;          // "Play to": the course time the stage runs to before it stands still again (< 0 = none)
// the drag of a handle
enum HKind { H_PARENT, H_NODE, H_TAN };
bool g_dragging = false, g_dragMoved = false, g_leftPending = false, g_leftPanning = false;
HKind g_dragKind = H_PARENT;
float g_dragW0[3]{}, g_dragOff[3]{}, g_dragLocal0[3]{};
int g_axis = 0;                 // 0 free, 1..3 = only the local x / y / z moves (X / Y / Z during a drag)
float g_pressX = 0, g_pressY = 0;
// the editor camera (only while the stage stands still: the enemies live in the game camera's frame)
bool g_camOn = false, g_camInit = false, g_camApplied = false;
float g_yaw = 0, g_pitch = 0.3f, g_dist = 30, g_target[3]{};
float g_saveEye[3]{}, g_saveTarget[3]{}, g_saveFovy = 45, g_saveNear = 1, g_saveFar = 1000;

g01::cTaskScene* Scene()
{
	g01::cTaskEnemyMan* man = g01::cTaskEnemyMan::s_pInstance;
	return man != nullptr ? man->pScene() : nullptr;
}
g01::cTaskCamera* Camera() { return KingdomGlobal::GalagaSys != nullptr ? KingdomGlobal::GalagaSys->pTaskCamera() : nullptr; }

bool InStage()
{
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	g01::cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	return core != nullptr && core->GetJobNo() == kingdom::job::GALAGA_GAME && sys != nullptr && !sys->IsMulti() && sys->nStage() >= 1 &&
	       sys->nStage() <= 9 && Scene() != nullptr && Camera() != nullptr;
}
bool Active() { return InStage() && FrameNo() - g_panelFrame <= 2; }   // the tab is drawn
bool Frozen() { return Active() && g_freeze && g_runTo < 0.0f; }

std::string ScnName(int stage)
{
	char buf[64];
	std::snprintf(buf, sizeof buf, "scn_s%xp%d.scn", stage, KingdomGlobal::GalagaSys->nPlayer());
	return buf;
}
std::string CsvName(int stage)
{
	char buf[64];
	std::snprintf(buf, sizeof buf, "EnemyOrbit_s%x.csv", stage);
	return buf;
}

std::vector<Rec> ParseScn(const std::uint8_t* d, std::size_t size)
{
	std::vector<Rec> out;
	for (std::size_t o = 0; d != nullptr && o + REC_SIZE <= size; o += REC_SIZE)
	{
		Rec r;
		std::memcpy(r.raw.data(), d + o, REC_SIZE);
		out.push_back(r);
	}
	return out;
}
Bytes ScnBytes(const std::vector<Rec>& recs)
{
	Bytes b;
	for (const Rec& r : recs) b.insert(b.end(), r.raw.begin(), r.raw.end());
	return b;
}

// the .emy name of every row, the way cOrbitDataMan::Load reads them (cKingdomCSV rows = lines, column 1; the first empty one ends the table)
std::vector<std::string> CsvNames(const std::string& text)
{
	std::vector<std::string> out;
	std::istringstream in(text);
	std::string line;
	while (std::getline(in, line))
	{
		if (!line.empty() && line.back() == '\r') line.pop_back();
		const std::size_t c = line.find(',');
		std::string name = c == std::string::npos ? std::string() : line.substr(c + 1);
		const std::size_t c2 = name.find(',');
		if (c2 != std::string::npos) name.resize(c2);
		while (!name.empty() && (name.back() == '\r' || name.back() == ' ')) name.pop_back();
		if (name.empty()) break;
		out.push_back(name);
	}
	return out;
}

Orbit LoadOrbit(const std::string& name, const fs::path& dir)
{
	Orbit o;
	o.name = name;
	if (!dir.empty())
	{
		std::ifstream in(dir / name, std::ios::binary);
		if (in)
		{
			const Bytes b((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			o.keys = ParseEmy(b.data(), b.size());
			o.exists = true;
			return o;
		}
	}
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(name, size);
	if (d != nullptr) { o.keys = ParseEmy(d, size); o.exists = true; }
	return o;
}

void Renumber(std::vector<Rec>& recs)
{
	for (std::size_t i = 0; i < recs.size(); i++)
		if (recs[i].Index() != (std::uint32_t)i) recs[i].SetU(0x10, (std::uint32_t)i);
}

Rec* Sel() { return g_sel >= 0 && g_sel < (int)g_data.recs.size() ? &g_data.recs[(std::size_t)g_sel] : nullptr; }
Orbit* SelOrbit()
{
	const Rec* r = Sel();
	if (r == nullptr || !r->IsSpawn() || r->OrbitId() >= (int)g_data.orbits.size()) return nullptr;
	Orbit& o = g_data.orbits[(std::size_t)r->OrbitId()];
	return o.exists ? &o : nullptr;
}

void Rebuild()
{
	Renumber(g_data.recs);
	g_sim = RunSim(g_data.recs);
	g_sel = std::clamp(g_sel, 0, std::max(0, (int)g_data.recs.size() - 1));
	const Orbit* o = SelOrbit();
	g_node = o != nullptr ? std::clamp(g_node, 0, std::max(0, (int)o->keys.size() - 1)) : 0;
}

void LoadStage(int stage)
{
	g_data = Data{};
	g_stage = stage;
	g_scnName = ScnName(stage);
	g_undo.clear();
	g_redo.clear();
	g_sel = g_node = 0;
	g_reveal = true;
	g_dirty = false;
	g_msg.clear();
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(g_scnName, size);
	g_data.recs = ParseScn(d, size);
	if (d == nullptr) g_msg = g_scnName + " is not in the attached archives";
	const std::uint8_t* c = wii::ArchiveStore::Find(CsvName(stage), size);
	g_csvRaw = c != nullptr ? std::string((const char*)c, size) : std::string();
	for (const std::string& n : CsvNames(g_csvRaw)) g_data.orbits.push_back(LoadOrbit(n, {}));
	if (g_appliedStage == stage && g_appliedName == g_scnName)
	{
		g_data = g_applied;
		g_msg = "editing the applied edits of this stage";
	}
	Rebuild();
}

void Push()
{
	g_undo.push_back(g_data);
	if (g_undo.size() > 100) g_undo.erase(g_undo.begin());
	g_redo.clear();
	g_dirty = true;
}
void Undo()
{
	if (g_undo.empty()) return;
	g_redo.push_back(g_data);
	g_data = g_undo.back();
	g_undo.pop_back();
	g_dirty = true;
	Rebuild();
	g_msg = "undo";
}
void Redo()
{
	if (g_redo.empty()) return;
	g_undo.push_back(g_data);
	g_data = g_redo.back();
	g_redo.pop_back();
	g_dirty = true;
	Rebuild();
	g_msg = "redo";
}

std::string KindLabel(int kind) { return kind >= 0 && kind < (int)ENEMY_NAMES.size() ? ENEMY_NAMES[(std::size_t)kind] : "kind" + std::to_string(kind); }

// ------------------------------------------------------------------------------------------------ the playhead (the live course)

struct Playhead { float ct; float speed; bool game; };
Playhead Now()
{
	const g01::cTaskScene* s = Scene();
	if (s == nullptr || !s->IsGame()) return {Sel() != nullptr ? Sel()->CourseTime() : 0.0f, 1.0f, false};
	return {s->fFrame(), s->fRate(), true};
}

// the command whose segment the playhead is in: the last one at or before its course time (unpac asks its own sim for the game frame)
int SegmentAt(float ct)
{
	int seg = -1;
	for (int i = 0; i < (int)g_data.recs.size(); i++)
		if (g_data.recs[(std::size_t)i].IsCommand() && g_data.recs[(std::size_t)i].CourseTime() <= ct + 1e-3f) seg = i;
	return seg;
}

// ------------------------------------------------------------------------------------------------ record edits (editor.py)

void Select(int i)
{
	g_sel = std::clamp(i, 0, std::max(0, (int)g_data.recs.size() - 1));
	g_node = 0;
	g_reveal = true;
}

void AddSpawn()
{
	Push();
	const Rec* base = Sel() != nullptr && Sel()->IsSpawn() ? Sel() : nullptr;
	const Playhead p = Now();
	const int seg = SegmentAt(p.ct);
	const bool moving = p.speed > 0.0f;
	const float zero[3] = {0, 0, 0};
	float parent[3];
	for (int i = 0; i < 3; i++) parent[i] = base != nullptr ? base->Parent(i) : zero[i];
	// a spawn while the course stands (wait / gate) runs on its delay: unpac takes the frames since the command started from its sim, the live
	// game does not tell them here, so it gets 1 (UNVERIFIED stand-in: set the delay in the fields)
	const float ct = moving ? std::round(p.ct * 10.0f) / 10.0f : (seg >= 0 ? g_data.recs[(std::size_t)seg].CourseTime() : 0.0f);
	const Rec r = NewSpawn(base != nullptr ? base->Kind() : 0, base != nullptr ? base->OrbitId() : 0, parent, ct, moving ? 0.0f : 1.0f,
	                       base != nullptr ? base->Group() : 0);
	// after the selected record when that is in the playhead's segment, otherwise right after the segment's command
	int at = g_sel;
	if (seg >= 0)
	{
		int next = (int)g_data.recs.size();
		for (int i = seg + 1; i < (int)g_data.recs.size(); i++)
			if (g_data.recs[(std::size_t)i].IsCommand()) { next = i; break; }
		at = seg <= g_sel && g_sel < next ? g_sel : seg;
	}
	at = std::clamp(at, -1, (int)g_data.recs.size() - 1);
	g_data.recs.insert(g_data.recs.begin() + at + 1, r);
	g_sel = at + 1;
	g_reveal = true;
	Rebuild();
	g_msg = "spawn added";
}

void AddCommand(int cmd)
{
	Push();
	const Playhead p = Now();
	const float ct = std::round(p.ct * 10.0f) / 10.0f, spd = std::max(p.speed, 0.1f);
	const Rec r = NewCommand(cmd, ct, cmd == CMD_WAIT || cmd == CMD_GATE ? 0.0f : spd, cmd == CMD_WAIT ? 60.0f : cmd == CMD_GATE ? 1.0f : 0.0f);
	const int at = std::min(g_sel + 1, (int)g_data.recs.size());
	g_data.recs.insert(g_data.recs.begin() + at, r);
	g_sel = at;
	g_reveal = true;
	Rebuild();
	std::string l = CMD_LABELS[(std::size_t)cmd];
	std::transform(l.begin(), l.end(), l.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	g_msg = l + " added";
}

void Duplicate()
{
	if (Sel() == nullptr) return;
	Push();
	const Rec r = *Sel();
	g_data.recs.insert(g_data.recs.begin() + g_sel + 1, r);
	g_sel++;
	g_reveal = true;
	Rebuild();
	g_msg = "duplicated";
}

void Delete()
{
	if (Sel() == nullptr || g_data.recs.size() <= 1) return;
	Push();
	g_data.recs.erase(g_data.recs.begin() + g_sel);
	Rebuild();
	g_msg = "deleted";
}

void Move(int delta)
{
	const int j = g_sel + delta;
	if (Sel() == nullptr || j < 0 || j >= (int)g_data.recs.size()) return;
	Push();
	std::swap(g_data.recs[(std::size_t)g_sel], g_data.recs[(std::size_t)j]);
	g_sel = j;
	g_reveal = true;
	Rebuild();
}

void SetTimeToPlayhead()
{
	Rec* r = Sel();
	if (r == nullptr || !r->IsSpawn()) return;
	const Playhead p = Now();
	if (!p.game) { g_msg = "the course has not started"; return; }
	Push();
	r->SetF(0x14, std::round(p.ct * 10.0f) / 10.0f);
	Rebuild();
	g_msg = "course time set to the playhead";
}

// ------------------------------------------------------------------------------------------------ orbit edits

// the next free orbit row and an attackN.emy name no row uses yet (editor.py next_orbit_slot; unpac also looks at the folder's files)
std::string NextOrbitName()
{
	int used = 0;
	auto take = [&](const std::string& n) {
		int v = 0;
		if (std::sscanf(n.c_str(), "attack%d.emy", &v) == 1) used = std::max(used, v);
	};
	for (const Orbit& o : g_data.orbits) take(o.name);
	return "attack" + std::to_string(used + 1) + ".emy";
}

void NewOrbit(bool copy)
{
	Rec* r = Sel();
	if (r == nullptr || !r->IsSpawn()) return;
	const Orbit* src = SelOrbit();
	Push();
	Orbit o;
	o.name = NextOrbitName();
	o.exists = true;
	o.keys = copy && src != nullptr ? src->keys : NewOrbitKeys();
	const int idx = (int)g_data.orbits.size();
	g_data.orbits.push_back(o);
	Sel()->SetOrbit(idx);
	g_node = 0;
	Rebuild();
	g_msg = "orbit " + std::to_string(idx) + " = " + o.name;
}

void AddNode()
{
	Orbit* o = SelOrbit();
	if (o == nullptr) return;
	Push();
	g_node = AddKey(*SelOrbit(), g_node);
}

void DeleteNode()
{
	Orbit* o = SelOrbit();
	if (o == nullptr) return;
	if (o->keys.size() <= 2) { g_msg = "an orbit needs two nodes"; return; }
	Push();
	o = SelOrbit();
	o->keys.erase(o->keys.begin() + std::clamp(g_node, 0, (int)o->keys.size() - 1));
	g_node = std::min(g_node, (int)o->keys.size() - 1);
}

// ------------------------------------------------------------------------------------------------ checks (editor.py _check)

struct Problem { std::string text; int rec; };   // rec = position of the record to select, -1 = none
std::vector<Problem> Check()
{
	std::vector<Problem> out;
	std::vector<const Rec*> cmds;
	for (const Rec& r : g_data.recs)
		if (r.IsCommand()) cmds.push_back(&r);
	char buf[256];
	if (cmds.size() < 2) out.push_back({"needs at least two course commands (a spawn only fires between two)", -1});
	if (!g_data.recs.empty() && !g_data.recs[0].IsCommand()) out.push_back({"the first record should be a course command", 0});
	for (std::size_t i = 0; i + 1 < cmds.size(); i++)
	{
		const Rec& a = *cmds[i];
		const Rec& b = *cmds[i + 1];
		if (b.CourseTime() < a.CourseTime())
		{
			std::snprintf(buf, sizeof buf, "command #%u goes back in course time (%g < %g)", b.Index(), b.CourseTime(), a.CourseTime());
			out.push_back({buf, (int)b.Index()});
		}
		if (a.IsRamp() && b.CourseTime() == a.CourseTime())
		{
			std::snprintf(buf, sizeof buf, "ramp #%u has no course time to ramp over", a.Index());
			out.push_back({buf, (int)a.Index()});
		}
	}
	for (int i = 0; i < (int)g_data.recs.size(); i++)
	{
		const Rec& r = g_data.recs[(std::size_t)i];
		if (!r.IsSpawn()) continue;
		if (r.OrbitId() >= (int)g_data.orbits.size() || !g_data.orbits[(std::size_t)r.OrbitId()].exists)
		{
			std::snprintf(buf, sizeof buf, "spawn #%d: orbit %d has no file", i, r.OrbitId());
			out.push_back({buf, i});
		}
		if (g_sim.spawnFrame.count(r.Index())) continue;
		const Rec* prev = nullptr;
		const Rec* next = nullptr;
		for (const Rec* c : cmds)
		{
			if (c->Index() < r.Index()) prev = c;
			else if (c->Index() > r.Index() && next == nullptr) next = c;
		}
		std::string why;
		if (prev == nullptr) why = "it is before the first command";
		else if (next == nullptr) why = "it is after the last command";
		else if (prev->IsWait() || prev->IsGate())
		{
			if (r.Delay() < 1) why = "a spawn in a wait or gate needs a delay of at least 1 frame";
			else
			{
				std::snprintf(buf, sizeof buf, "its delay %g is after the %s ends", r.Delay(), prev->IsWait() ? "wait" : "gate");
				why = buf;
			}
		}
		else
		{
			std::snprintf(buf, sizeof buf, "its course time %g is outside %g to %g", r.CourseTime(), prev->CourseTime(), next->CourseTime());
			why = buf;
		}
		out.push_back({"spawn #" + std::to_string(i) + " never triggers: " + why, i});
	}
	for (std::size_t i = 0; i < g_data.orbits.size(); i++)
	{
		const Orbit& o = g_data.orbits[i];
		for (std::size_t k = 0; k + 1 < o.keys.size(); k++)
			if (o.keys[k + 1].Frame() < o.keys[k].Frame())
			{
				out.push_back({"orbit " + std::to_string(i) + " (" + o.name + "): node frames go backwards", -1});
				break;
			}
	}
	if (g_sim.stalled) out.push_back({"the course never finishes (a speed of zero that is never left)", -1});
	return out;
}

// ------------------------------------------------------------------------------------------------ apply / save / load

void Restart()
{
	kingdom::Kingdom_SetJobNo(kingdom::job::GALAGA_GAME, false, false);   // the stage again (the retry path): cTaskScene / cOrbitDataMan read their data
}

void Apply()
{
	Renumber(g_data.recs);
	g_applied = g_data;
	g_appliedScn = ScnBytes(g_data.recs);
	g_appliedName = g_scnName;
	g_appliedStage = g_stage;
	g_dirty = false;
	Restart();
	g_msg = "applied: the stage restarts with the edits";
}

void Revert()
{
	g_applied = Data{};
	g_appliedScn.clear();
	g_appliedStage = -1;
	LoadStage(g_stage);
	Restart();
	g_msg = "back to the disc's data";
}

// "Play to": run until the course reaches the record (restart when that is behind the playhead)
void PlayTo(int i)
{
	if (i < 0 || i >= (int)g_data.recs.size()) return;
	const Rec& r = g_data.recs[(std::size_t)i];
	if (r.IsSpawn() && !g_sim.spawnFrame.count(r.Index())) { g_msg = "that record never happens"; return; }
	float target = r.CourseTime();
	if (r.IsSpawn())
	{
		const int seg = SegmentAt(r.CourseTime());
		int s = -1;
		for (int k = 0; k < i; k++)
			if (g_data.recs[(std::size_t)k].IsCommand()) s = k;
		if (s >= 0 && (g_data.recs[(std::size_t)s].IsWait() || g_data.recs[(std::size_t)s].IsGate())) target = g_data.recs[(std::size_t)s].CourseTime();
		(void)seg;
	}
	const Playhead p = Now();
	if (!p.game || target < p.ct - 0.05f) Restart();
	g_runTo = target;
	g_msg = "playing to course time " + std::to_string((int)target) + " (gates wait for their enemies to be shot)";
}

fs::path EditDir(int stage)
{
	fs::path root;
	if (const char* e = std::getenv("MEGAMIX_G01_EDITS"); e != nullptr && *e) root = e;
	else if (const char* x = std::getenv("XDG_DATA_HOME"); x != nullptr && *x) root = fs::path(x) / "megamix" / "g01_edits";
	else if (const char* h = std::getenv("HOME"); h != nullptr) root = fs::path(h) / ".local" / "share" / "megamix" / "g01_edits";
	else root = "g01_edits";
	char buf[16];
	std::snprintf(buf, sizeof buf, "stage%02d", stage);
	return root / buf;
}

// EnemyOrbit csv listing every orbit: the disc's lines with the rows replaced or added (editor.py _write_csv)
std::string CsvText()
{
	const std::string eol = g_csvRaw.find("\r\n") != std::string::npos ? "\r\n" : "\n";
	std::vector<std::string> lines;
	if (!g_csvRaw.empty())
	{
		std::size_t at = 0;
		for (;;)
		{
			const std::size_t e = g_csvRaw.find(eol, at);
			lines.push_back(g_csvRaw.substr(at, e == std::string::npos ? std::string::npos : e - at));
			if (e == std::string::npos) break;
			at = e + eol.size();
		}
	}
	std::map<int, std::size_t> rows;
	for (std::size_t n = 0; n < lines.size(); n++)
	{
		const std::size_t c = lines[n].find(',');
		if (c == std::string::npos || c == 0) continue;
		const std::string head = lines[n].substr(0, c);
		if (head.find_first_not_of("0123456789 ") == std::string::npos) rows[std::atoi(head.c_str())] = n;
	}
	std::vector<std::string> pending;
	for (std::size_t i = 0; i < g_data.orbits.size(); i++)
	{
		const std::string& name = g_data.orbits[i].name;
		auto it = rows.find((int)i);
		if (it != rows.end())
		{
			std::string& l = lines[it->second];
			const std::size_t c = l.find(',');
			const std::size_t c2 = l.find(',', c + 1);
			std::string cur = l.substr(c + 1, c2 == std::string::npos ? std::string::npos : c2 - c - 1);
			while (!cur.empty() && cur.back() == ' ') cur.pop_back();
			if (cur != name) l = l.substr(0, c + 1) + name + (c2 == std::string::npos ? std::string() : l.substr(c2));
		}
		else
			pending.push_back(std::to_string(i) + "," + name);
	}
	if (!pending.empty())
	{
		std::size_t at = 0;
		for (const auto& [idx, n] : rows) at = std::max(at, n + 1);
		lines.insert(lines.begin() + (std::ptrdiff_t)at, pending.begin(), pending.end());
	}
	if (lines.empty() || !lines.back().empty()) lines.push_back("");
	std::string out;
	for (std::size_t i = 0; i < lines.size(); i++) out += (i ? eol : "") + lines[i];
	return out;
}

bool WriteFile(const fs::path& p, const std::string& data)
{
	std::ofstream out(p, std::ios::binary);
	out.write(data.data(), (std::streamsize)data.size());
	return out.good();
}

void Save()
{
	const fs::path dir = EditDir(g_stage);
	std::error_code ec;
	fs::create_directories(dir, ec);
	Renumber(g_data.recs);
	const Bytes scn = ScnBytes(g_data.recs);
	bool ok = WriteFile(dir / g_scnName, std::string(scn.begin(), scn.end()));
	int orbits = 0;
	for (const Orbit& o : g_data.orbits)
		if (o.exists)
		{
			const Bytes b = EmyBytes(o);
			ok = WriteFile(dir / o.name, std::string(b.begin(), b.end())) && ok;
			orbits++;
		}
	ok = WriteFile(dir / CsvName(g_stage), CsvText()) && ok;
	g_msg = ok ? "saved " + g_scnName + ", " + std::to_string(orbits) + " orbits and " + CsvName(g_stage) + " in " + dir.string()
	           : "could not write everything in " + dir.string();
}

void Load()
{
	const fs::path dir = EditDir(g_stage);
	std::ifstream in(dir / g_scnName, std::ios::binary);
	if (!in) { g_msg = "nothing saved in " + dir.string(); return; }
	const Bytes b((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	if (b.size() % REC_SIZE != 0) { g_msg = (dir / g_scnName).string() + " is not a scn file (its size is not a multiple of 0x30)"; return; }
	Push();
	g_data.recs = ParseScn(b.data(), b.size());
	std::ifstream csv(dir / CsvName(g_stage), std::ios::binary);
	if (csv)
	{
		const std::string text((std::istreambuf_iterator<char>(csv)), std::istreambuf_iterator<char>());
		g_data.orbits.clear();
		for (const std::string& n : CsvNames(text)) g_data.orbits.push_back(LoadOrbit(n, dir));
	}
	g_sel = g_node = 0;
	g_reveal = true;
	Rebuild();
	g_msg = "loaded: Apply to play it";
}

// ------------------------------------------------------------------------------------------------ the course frame (cTaskCamera) and handles

void LocalToWorld(const float local[3], float out[3]) { Camera()->LocalToWorld(out, local); }

struct Handle { HKind kind; int node; float w[3]; unsigned rgba; float r; };
std::vector<Handle> Handles()
{
	std::vector<Handle> out;
	const Rec* r = Sel();
	if (r == nullptr || !r->IsSpawn() || Camera() == nullptr) return out;
	const float parent[3] = {r->Parent(0), r->Parent(1), r->Parent(2)};
	Handle h{H_PARENT, -1, {}, 0xfff078ffu, 0.35f};
	LocalToWorld(parent, h.w);
	out.push_back(h);
	const Orbit* o = SelOrbit();
	if (o == nullptr) return out;
	for (int i = 0; i < (int)o->keys.size(); i++)
	{
		const Key& k = o->keys[(std::size_t)i];
		const float base[3] = {parent[0] + k.Pos(0), parent[1] + k.Pos(1), parent[2] + k.Pos(2)};
		Handle n{H_NODE, i, {}, i == g_node ? 0xffaa3cffu : 0x5aaaffffu, i == g_node ? 0.38f : 0.3f};
		LocalToWorld(base, n.w);
		out.push_back(n);
		if (k.Interp() == 1)
		{
			const float t[3] = {k.Tan(0), k.Tan(1), k.Tan(2)};
			float tw[3];
			Camera()->LocalDirToWorld(tw, t);
			Handle th{H_TAN, i, {n.w[0] + tw[0], n.w[1] + tw[1], n.w[2] + tw[2]}, 0xff8232ffu, 0.2f};
			out.push_back(th);
		}
	}
	return out;
}

void CameraForward(float f[3])
{
	float eye[3], target[3], fovy;
	kingdom::NativeScene::GetCamera(eye, target, fovy);
	float l = 0;
	for (int i = 0; i < 3; i++) { f[i] = target[i] - eye[i]; l += f[i] * f[i]; }
	l = std::sqrt(l);
	for (int i = 0; i < 3; i++) f[i] = l > 0 ? f[i] / l : 0.0f;
}

// where a ray meets the plane through p facing n
bool PlaneHit(const float o[3], const float d[3], const float p[3], const float n[3], float out[3])
{
	const float den = n[0] * d[0] + n[1] * d[1] + n[2] * d[2];
	if (std::fabs(den) < 1e-6f) return false;
	const float t = (n[0] * (p[0] - o[0]) + n[1] * (p[1] - o[1]) + n[2] * (p[2] - o[2])) / den;
	for (int i = 0; i < 3; i++) out[i] = o[i] + d[i] * t;
	return true;
}

void ViewInput()
{
	const devui::Input in = devui::GetInput();
	const devui::Rect gv = devui::GameView();
	const bool over = in.mx >= gv.x && in.mx < gv.x + gv.w && in.my >= gv.y && in.my < gv.y + gv.h;
	if (!g_editView || Camera() == nullptr) { g_dragging = false; return; }
	if (over || g_dragging || g_leftPending) devui::CaptureInput(true, true);
	float vp[16];
	if (!ViewProj(vp)) return;

	// the editor camera: right drag turns, middle drag (or a left drag off the handles) pans, the wheel zooms
	if (g_leftPending && in.down[0] && g_camApplied && std::fabs(in.mx - g_pressX) + std::fabs(in.my - g_pressY) > 4.0f) g_leftPanning = true;
	if (g_camApplied && (over || g_leftPanning))
	{
		if (in.down[1]) { g_yaw -= in.dx * 0.01f; g_pitch = std::clamp(g_pitch + in.dy * 0.01f, -1.5f, 1.5f); }
		if (in.down[2] || (g_leftPanning && in.down[0]))
		{
			const float s = g_dist * 0.0025f, cy = std::cos(g_yaw), sy = std::sin(g_yaw);
			g_target[0] -= (in.dx * cy) * s;
			g_target[2] -= (-in.dx * sy) * s;
			g_target[1] += in.dy * s;
		}
		if (in.wheel != 0.0f) g_dist = std::clamp(g_dist * std::pow(0.88f, in.wheel), 1.0f, 4000.0f);
	}

	float o[3], d[3];
	const bool ray = MouseRay(vp, in.mx, in.my, o, d);
	if (over && in.pressed[0] && ray)
	{
		// the nearest handle within 14 pixels
		const std::vector<Handle> hs = Handles();
		int best = -1;
		float bestD = 14.0f * 14.0f;
		for (int i = 0; i < (int)hs.size(); i++)
		{
			float sx, sy;
			if (!Project(vp, hs[(std::size_t)i].w, sx, sy)) continue;
			const float dd = (sx - in.mx) * (sx - in.mx) + (sy - in.my) * (sy - in.my);
			if (dd < bestD) { bestD = dd; best = i; }
		}
		if (best < 0)
		{
			g_leftPending = true;   // pan
			g_leftPanning = false;
			g_pressX = in.mx;
			g_pressY = in.my;
		}
		else
		{
			const Handle& h = hs[(std::size_t)best];
			if (h.kind != H_PARENT) g_node = h.node;
			float n[3], grab[3];
			CameraForward(n);
			if (!PlaneHit(o, d, h.w, n, grab)) std::copy(h.w, h.w + 3, grab);
			g_dragging = true;
			g_dragMoved = false;
			g_dragKind = h.kind;
			g_axis = 0;
			for (int i = 0; i < 3; i++) { g_dragW0[i] = h.w[i]; g_dragOff[i] = grab[i] - h.w[i]; }
			Camera()->WorldToLocal(g_dragLocal0, h.w);
		}
	}
	if (g_leftPending && !in.down[0]) g_leftPending = g_leftPanning = false;
	if (g_dragging)
	{
		for (int a = 0; a < 3; a++)
			if (devui::KeyPressed('X' + a)) g_axis = g_axis == a + 1 ? 0 : a + 1;   // X / Y / Z: only that local axis moves (again = free)
		float n[3], hit[3];
		CameraForward(n);
		Rec* r = Sel();
		if (!in.down[0] || r == nullptr) g_dragging = false;
		else if (ray && PlaneHit(o, d, g_dragW0, n, hit))
		{
			float world[3], local[3];
			for (int i = 0; i < 3; i++) world[i] = hit[i] - g_dragOff[i];
			Camera()->WorldToLocal(local, world);
			if (g_axis != 0)
				for (int a = 0; a < 3; a++)
					if (a != g_axis - 1) local[a] = g_dragLocal0[a];
			float moved = 0;
			for (int i = 0; i < 3; i++) moved += std::fabs(world[i] - g_dragW0[i]);
			if (!g_dragMoved && moved > 1e-4f) { Push(); g_dragMoved = true; r = Sel(); }
			if (g_dragMoved)
			{
				if (g_dragKind == H_PARENT)
				{
					for (int i = 0; i < 3; i++) r->SetF(4 * i, local[i]);
					Rebuild();
				}
				else if (Orbit* orb = SelOrbit(); orb != nullptr && g_node < (int)orb->keys.size())
				{
					Key& k = orb->keys[(std::size_t)g_node];
					if (g_dragKind == H_NODE)
					{
						const float p[3] = {local[0] - r->Parent(0), local[1] - r->Parent(1), local[2] - r->Parent(2)};
						k.SetPos(p);
					}
					else
					{
						// the tangent: the tip relative to its node, turned back into the frame
						const float base[3] = {r->Parent(0) + k.Pos(0), r->Parent(1) + k.Pos(1), r->Parent(2) + k.Pos(2)};
						float nw[3], dw[3], t[3];
						LocalToWorld(base, nw);
						for (int i = 0; i < 3; i++) dw[i] = world[i] - nw[i];
						Camera()->WorldDirToLocal(t, dw);
						k.SetTan(t);
					}
				}
			}
		}
	}

	if (!over || devui::Typing()) return;
	// keys (editor.py): Ctrl+S / Z / Y, N new spawn at the playhead, T course time = playhead, Delete deletes the record, Enter applies
	if (in.ctrl)
	{
		if (devui::KeyPressed('Z')) { if (in.shift) Redo(); else Undo(); }
		if (devui::KeyPressed('Y')) Redo();
		if (devui::KeyPressed('S')) Save();
		if (devui::KeyPressed('D')) Duplicate();
		return;
	}
	if (devui::KeyPressed('N')) AddSpawn();
	if (devui::KeyPressed('T')) SetTimeToPlayhead();
	if (devui::KeyPressed(devui::K_DELETE)) Delete();
	if (devui::KeyPressed(devui::K_ENTER)) Apply();
}

// the editor camera: only while the stage stands still (nothing else sets the camera then); the course camera comes back when it is switched off,
// the tab is left or the stage runs again (G01EditFrozen, before the game's camera task reads it)
void RestoreCamera()
{
	if (!g_camApplied) return;
	kingdom::NativeScene::SetCamera(g_saveEye, g_saveTarget, g_saveFovy, g_saveNear, g_saveFar);
	g_camApplied = false;
}
void ApplyEditorCamera()
{
	if (!g_camApplied)
	{
		kingdom::NativeScene::GetCamera(g_saveEye, g_saveTarget, g_saveFovy);
		kingdom::NativeScene::GetClip(g_saveNear, g_saveFar);
		if (!g_camInit)
		{
			// start where the course camera is
			float d[3];
			for (int i = 0; i < 3; i++) d[i] = g_saveEye[i] - g_saveTarget[i];
			g_dist = std::max(1.0f, std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]));
			g_yaw = std::atan2(d[0], d[2]);
			g_pitch = std::asin(std::clamp(d[1] / g_dist, -1.0f, 1.0f));
			std::copy(g_saveTarget, g_saveTarget + 3, g_target);
			g_camInit = true;
		}
	}
	const float cp = std::cos(g_pitch);
	const float eye[3] = {g_target[0] + g_dist * cp * std::sin(g_yaw), g_target[1] + g_dist * std::sin(g_pitch), g_target[2] + g_dist * cp * std::cos(g_yaw)};
	kingdom::NativeScene::SetCamera(eye, g_target, g_saveFovy, g_saveNear, std::max(g_saveFar, g_dist + 1000.0f));
	g_camApplied = true;
}

// ------------------------------------------------------------------------------------------------ the panel (editor.py panel)

std::string RowText(int i, const Rec& r, devui::Rgba& col)
{
	char buf[160];
	if (r.IsSpawn())
	{
		auto it = g_sim.spawnFrame.find(r.Index());
		const std::string when = it != g_sim.spawnFrame.end() ? "f" + std::to_string(it->second) : "never";
		std::snprintf(buf, sizeof buf, "%3d %6.1f %-8s o%-3d g%-3d %s", i, r.CourseTime(), KindLabel(r.Kind()).c_str(), r.OrbitId(), r.Group(), when.c_str());
		col = devui::Color(it != g_sim.spawnFrame.end() ? devui::TEXT : devui::BAD);
		return buf;
	}
	const std::string label = r.Cmd() < 4 ? CMD_LABELS[(std::size_t)r.Cmd()] : "cmd" + std::to_string(r.Cmd());
	char extra[64] = "";
	if (r.Cmd() == CMD_RAMP || r.Cmd() == CMD_ADVANCE) std::snprintf(extra, sizeof extra, "spd %.3f", r.CourseSpeed());
	else if (r.Cmd() == CMD_WAIT) std::snprintf(extra, sizeof extra, "%g frames", r.Delay());
	else if (r.Cmd() == CMD_GATE) std::snprintf(extra, sizeof extra, "groups 0-%u / %gf", r.P1(), r.Delay());
	std::snprintf(buf, sizeof buf, "%3d %6.1f %s %s", i, r.CourseTime(), label.c_str(), extra);
	col = devui::Rgba{150, 200, 255, 255};
	return buf;
}

void SpawnFields(devui::UI& ui)
{
	const Rec cur = *Sel();
	Rec r = cur;
	static const double zero = 0.0;
	r.SetKind(ui.Choice(ui.Row(), devui::MakeId("g01e.kind"), "enemy", ENEMY_NAMES, std::clamp(cur.Kind(), 0, (int)ENEMY_NAMES.size() - 1)));
	if (cur.Kind() >= (int)ENEMY_NAMES.size() && r.Kind() == (int)ENEMY_NAMES.size() - 1) r.SetKind(cur.Kind());   // an unknown kind stays until picked
	const int orbit = (int)ui.Number(ui.Row(), devui::MakeId("g01e.orbit"), "orbit index", cur.OrbitId(), 1, "%.0f", true, &zero);
	r.SetOrbit(orbit);
	{
		std::string s = "   ";
		if (orbit < (int)g_data.orbits.size())
		{
			const Orbit& o = g_data.orbits[(std::size_t)orbit];
			s += o.name + (o.exists ? "   " + std::to_string(o.keys.size()) + " nodes, " + std::to_string((int)(o.keys.empty() ? 0.0f : o.keys.back().Frame())) + " frames" : "   (no file)");
		}
		else
			s += "(no file)";
		ui.Label(s, &devui::Color(devui::DIM));
	}
	r.SetF(0x14, (float)ui.Number(ui.Row(), devui::MakeId("g01e.ct"), "course time", cur.CourseTime(), 1.0, "%.2f"));
	r.SetF(0x20, (float)ui.Number(ui.Row(), devui::MakeId("g01e.delay"), "delay (frames)", cur.Delay(), 1.0, "%.1f"));
	float par[3] = {cur.Parent(0), cur.Parent(1), cur.Parent(2)};
	ui.Vec3(ui.Row(), devui::MakeId("g01e.parent"), "position", par, 0.5);
	for (int i = 0; i < 3; i++) r.SetF(4 * i, par[i]);
	r.SetGroup((int)ui.Number(ui.Row(), devui::MakeId("g01e.group"), "group", cur.Group(), 1, "%.0f", true, &zero));
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		r.SetU(0x28, (std::uint32_t)ui.Number(c[0], devui::MakeId("g01e.p1"), "p1", cur.P1(), 1, "%.0f", true, &zero));
		r.SetMarker((int)ui.Number(c[1], devui::MakeId("g01e.marker"), "marker", cur.Marker(), 1, "%.0f", true, &zero));
	}
	auto it = g_sim.spawnFrame.find(cur.Index());
	char buf[96];
	if (it != g_sim.spawnFrame.end()) std::snprintf(buf, sizeof buf, "triggers at frame %d (%.1f s)", it->second, it->second / 60.0f);
	else std::snprintf(buf, sizeof buf, "NEVER TRIGGERS (see Check)");
	ui.Label(buf, &devui::Color(it != g_sim.spawnFrame.end() ? devui::DIM : devui::BAD));
	if (r.raw != cur.raw)
	{
		Push();
		*Sel() = r;
		Rebuild();
	}
}

void CommandFields(devui::UI& ui)
{
	const Rec cur = *Sel();
	Rec r = cur;
	static const double zero = 0.0;
	const int cmd = ui.Choice(ui.Row(), devui::MakeId("g01e.cmd"), "command", CMD_LABELS, std::min(cur.Cmd(), 3));
	if (cmd != std::min(cur.Cmd(), 3)) r.SetU(0x24, (cur.Word() & ~0x7fffu) | (std::uint32_t)cmd);
	r.SetF(0x14, (float)ui.Number(ui.Row(), devui::MakeId("g01e.cct"), "course time", cur.CourseTime(), 1.0, "%.2f"));
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1});
		r.SetF(0x18, (float)ui.Number(c[0], devui::MakeId("g01e.cs"), "course spd", cur.CourseSpeed(), 0.01, "%.3f"));
		r.SetF(0x1c, (float)ui.Number(c[1], devui::MakeId("g01e.ps"), "pacman spd", cur.PacmanSpeed(), 0.01, "%.3f"));
	}
	r.SetF(0x20, (float)ui.Number(ui.Row(), devui::MakeId("g01e.cdelay"), "delay (frames)", cur.Delay(), 1.0, "%.1f"));
	r.SetU(0x28, (std::uint32_t)ui.Number(ui.Row(), devui::MakeId("g01e.cp1"), "gate groups (last)", cur.P1(), 1, "%.0f", true, &zero));
	static const char* const hints[4] = {"advance at this speed to the next command's course time",
	                                     "speeds ramp to the next command's over the course time between them",
	                                     "the course stops for the delay; spawns here use their delay as time",
	                                     "the course stops until groups 0..p1 are cleared (the check assumes at once)"};
	if (cur.Cmd() < 4) ui.Label(hints[cur.Cmd()], &devui::Color(devui::DIM));
	if (r.raw != cur.raw)
	{
		Push();
		*Sel() = r;
		Rebuild();
	}
}

void ScenePage(devui::UI& ui)
{
	g_filter = ui.Choice(ui.Row(), devui::MakeId("g01e.filter"), "show", {"all records", "spawns only", "commands only"}, g_filter);
	std::vector<devui::ListRow> rows;
	std::vector<int> mapping;
	for (int i = 0; i < (int)g_data.recs.size(); i++)
	{
		const Rec& r = g_data.recs[(std::size_t)i];
		if ((g_filter == 1 && !r.IsSpawn()) || (g_filter == 2 && !r.IsCommand())) continue;
		devui::Rgba col{};
		const std::string t = RowText(i, r, col);
		rows.push_back(devui::ListRow{t, col});
		mapping.push_back(i);
	}
	const auto at = std::find(mapping.begin(), mapping.end(), g_sel);
	const int shown = at != mapping.end() ? (int)(at - mapping.begin()) : -1;
	bool activated = false;
	const int picked = ui.ListBox(devui::MakeId("g01e.records"), ui.Row(devui::ROW * 11, 4), rows, shown, g_reveal, nullptr, false, &activated);
	g_reveal = false;
	if (picked != shown && picked >= 0 && picked < (int)mapping.size()) { Select(mapping[(std::size_t)picked]); g_reveal = false; }
	if (activated) PlayTo(g_sel);

	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
	if (ui.Button(c[0], "+ Spawn")) AddSpawn();
	if (ui.Button(c[1], "+ Ramp")) AddCommand(CMD_RAMP);
	if (ui.Button(c[2], "+ Wait")) AddCommand(CMD_WAIT);
	if (ui.Button(c[3], "+ Gate")) AddCommand(CMD_GATE);
	c = devui::UI::Split(ui.Row(), {1, 1, 1, 1, 1.2f});
	if (ui.Button(c[0], "Dup")) Duplicate();
	if (ui.Button(c[1], "Delete")) Delete();
	if (ui.Button(c[2], "Up")) Move(-1);
	if (ui.Button(c[3], "Down")) Move(1);
	if (ui.Button(c[4], "Play to it")) PlayTo(g_sel);

	if (Sel() == nullptr) return;
	ui.Heading("record #" + std::to_string(g_sel) + (Sel()->IsSpawn() ? " spawn" : " course command"));
	if (Sel()->IsSpawn()) SpawnFields(ui);
	else CommandFields(ui);
}

void OrbitPage(devui::UI& ui)
{
	Rec* r = Sel();
	if (r == nullptr || !r->IsSpawn()) { ui.Label("select a spawn in the Scene page to edit its orbit", &devui::Color(devui::DIM)); return; }
	const int oi = r->OrbitId();
	Orbit* o = SelOrbit();
	ui.Label("orbit " + std::to_string(oi) + ": " + (oi < (int)g_data.orbits.size() ? g_data.orbits[(std::size_t)oi].name : std::string("(no row)")) +
	         (o != nullptr ? "  (" + std::to_string(o->keys.size()) + " nodes)" : std::string("  (no file)")));
	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
	if (ui.Button(c[0], "New orbit")) { NewOrbit(false); return; }
	if (ui.Button(c[1], "Copy orbit", o != nullptr)) { NewOrbit(true); return; }
	if (ui.Button(c[2], "Play to spawn")) PlayTo(g_sel);
	if (o == nullptr) { ui.Label("this orbit has no file yet; New orbit makes one", &devui::Color(devui::WARN)); return; }
	ui.Label("drag the blue nodes (orange tips are tangents) in the game view; X / Y / Z during a drag keep it on that local axis", &devui::Color(devui::DIM));

	std::vector<devui::ListRow> rows;
	for (int i = 0; i < (int)o->keys.size(); i++)
	{
		const Key& k = o->keys[(std::size_t)i];
		std::string ip = k.Interp() < 4 ? INTERP_LABELS[(std::size_t)k.Interp()] : "?" + std::to_string(k.Interp());
		if (k.Interp() == 2) ip += " " + std::to_string(k.JumpTo());
		char buf[160];
		std::snprintf(buf, sizeof buf, "%2d f%7.1f  %-9s %s  (%.1f, %.1f, %.1f)", i, k.Frame(), ip.c_str(),
		              k.Facing() < 4 ? FACING_LABELS[(std::size_t)k.Facing()].c_str() : "?", k.Pos(0), k.Pos(1), k.Pos(2));
		rows.push_back(devui::ListRow{buf, devui::Color(i == g_node ? devui::GOOD : devui::TEXT)});
	}
	const int pick = ui.ListBox(devui::MakeId("g01e.nodes", oi), ui.Row(devui::ROW * 7, 4), rows, g_node);
	if (pick != g_node && pick >= 0 && pick < (int)o->keys.size()) g_node = pick;
	c = devui::UI::Split(ui.Row(), {1, 1, 1});
	if (ui.Button(c[0], "+ Node")) { AddNode(); return; }
	if (ui.Button(c[1], "Delete node")) { DeleteNode(); return; }
	if (ui.Button(c[2], "Sort nodes"))
	{
		Push();
		o = SelOrbit();
		std::stable_sort(o->keys.begin(), o->keys.end(), [](const Key& a, const Key& b) { return a.Frame() < b.Frame(); });
	}
	o = SelOrbit();
	if (o == nullptr || g_node < 0 || g_node >= (int)o->keys.size()) return;
	const Key cur = o->keys[(std::size_t)g_node];
	Key k = cur;
	ui.Heading("node " + std::to_string(g_node));
	static const double zero = 0.0;
	k.SetFrame((float)ui.Number(ui.Row(), devui::MakeId("g01e.nframe", oi, g_node), "frame", cur.Frame(), 1.0, "%.1f"));
	float pos[3] = {cur.Pos(0), cur.Pos(1), cur.Pos(2)};
	ui.Vec3(ui.Row(), devui::MakeId("g01e.npos", oi, g_node), "position", pos, 0.5);
	k.SetPos(pos);
	float deg[3], rot[3];
	for (int i = 0; i < 3; i++) deg[i] = cur.Rot(i) * 57.2957795f;
	ui.Vec3(ui.Row(), devui::MakeId("g01e.nrot", oi, g_node), "rotation (deg)", deg, 5.0, "%.0f");
	bool rotChanged = false;
	for (int i = 0; i < 3; i++) { rot[i] = deg[i] * 0.0174532925f; rotChanged = rotChanged || std::fabs(rot[i] - cur.Rot(i)) > 1e-4f; }
	if (rotChanged) k.SetRot(rot);
	float tan[3] = {cur.Tan(0), cur.Tan(1), cur.Tan(2)};
	ui.Vec3(ui.Row(), devui::MakeId("g01e.ntan", oi, g_node), "tangent", tan, 0.5);
	k.SetTan(tan);
	int interp = cur.Interp(), facing = cur.Facing(), jump = cur.JumpTo();
	const int ip = ui.Choice(ui.Row(), devui::MakeId("g01e.ninterp", oi, g_node), "interpolation", INTERP_LABELS, std::min(interp, 3));
	if (ip != std::min(interp, 3)) interp = ip;
	const int fc = ui.Choice(ui.Row(), devui::MakeId("g01e.nface", oi, g_node), "facing", FACING_LABELS, std::min(facing, 3));
	if (fc != std::min(facing, 3)) facing = fc;
	if (cur.Interp() == 2) jump = (int)ui.Number(ui.Row(), devui::MakeId("g01e.njump", oi, g_node), "jump to node", jump, 1, "%.0f", true, &zero);
	if (interp != cur.Interp() || facing != cur.Facing() || jump != cur.JumpTo()) k.SetCommand(interp, facing, jump);
	if (cur.Interp() == 3) k.w[14] = (std::uint32_t)ui.Number(ui.Row(), devui::MakeId("g01e.nparam", oi, g_node), "fire pattern", cur.Param(), 1, "%.0f", true, &zero);
	if (k.w != cur.w)
	{
		Push();
		SelOrbit()->keys[(std::size_t)g_node] = k;
	}
}

void CheckPage(devui::UI& ui)
{
	int spawns = 0;
	float last = 0;
	for (const Rec& r : g_data.recs) { spawns += r.IsSpawn() ? 1 : 0; last = std::max(last, r.CourseTime()); }
	char buf[160];
	std::snprintf(buf, sizeof buf, "%d records, %d spawns (%d trigger)", (int)g_data.recs.size(), spawns, (int)g_sim.spawnFrame.size());
	ui.Label(buf);
	std::snprintf(buf, sizeof buf, "%d game frames = %.1f s at 60 fps, course to %.1f", g_sim.frames, g_sim.frames / 60.0f, last);
	ui.Label(buf);
	const std::vector<Problem> probs = Check();
	if (probs.empty()) ui.Label("no problems found", &devui::Color(devui::GOOD));
	for (const Problem& p : probs)
	{
		ui.Label("? " + p.text, &devui::Color(devui::WARN));
		if (p.rec >= 0 && ui.Button(ui.Row(), "Select it", true, p.rec == g_sel)) { Select(p.rec); g_page = 0; }
	}
}

void EditorPanel(devui::UI& ui)
{
	if (!InStage()) return;
	g_panelFrame = FrameNo();
	const int stage = KingdomGlobal::GalagaSys->nStage();
	if (g_stage < 0 || ((g_stage != stage || g_scnName != ScnName(stage)) && !g_dirty)) LoadStage(stage);
	// "Play to": stand still again once the course is there (or it ended)
	if (g_runTo >= 0.0f)
	{
		const g01::cTaskScene* s = Scene();
		if (s->IsCourseEnd() || (s->IsGame() && s->fFrame() >= g_runTo)) { g_runTo = -1.0f; g_msg = "there: course time " + std::to_string((int)s->fFrame()); }
	}
	ui.Heading("Galaga stage editor");
	ui.Label(g_scnName + "  -  " + std::to_string(g_data.recs.size()) + " records, " + std::to_string(g_data.orbits.size()) + " orbits" +
	         (g_appliedStage == g_stage ? (g_dirty ? "  -  edited since the last apply" : "  -  edits applied") : (g_dirty ? "  -  edited, not applied" : "")),
	         &devui::Color(g_dirty ? devui::WARN : devui::DIM));
	if (g_stage != stage)
	{
		ui.Label("The game is on stage " + std::to_string(stage) + "; the edits are for stage " + std::to_string(g_stage) + ".", &devui::Color(devui::WARN));
		if (ui.Button(ui.Row(), "Drop the edits and edit this stage")) LoadStage(stage);
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1.4f, 1, 0.9f});
		if (ui.Button(c[0], "Apply + restart (Enter)")) { Apply(); return; }
		if (ui.Button(c[1], "Back to the disc")) { Revert(); return; }
		if (ui.Button(c[2], "Restart")) { Restart(); g_msg = "restarted"; }
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
		if (ui.Button(c[0], "Undo", !g_undo.empty())) Undo();
		if (ui.Button(c[1], "Redo", !g_redo.empty())) Redo();
		if (ui.Button(c[2], "Save")) Save();
		if (ui.Button(c[3], "Load")) Load();
	}
	ui.Label("folder: " + EditDir(g_stage).string(), &devui::Color(devui::DIM));
	g_editView = ui.Checkbox(ui.Row(), "Edit in the game view (the game gets no mouse / keys there)", g_editView);
	g_freeze = ui.Checkbox(ui.Row(), "Stand still while this tab is shown (the handles need it)", g_freeze);
	{
		const Playhead p = Now();
		char buf[128];
		if (p.game) std::snprintf(buf, sizeof buf, "playhead: course time %.1f, speed %.3f%s", p.ct, p.speed, g_runTo >= 0 ? "  (playing to it)" : "");
		else std::snprintf(buf, sizeof buf, "the course has not started yet (opening / READY)");
		ui.Label(buf, &devui::Color(g_runTo >= 0 ? devui::WARN : devui::DIM));
		if (g_runTo >= 0 && ui.Button(ui.Row(), "Stop here")) g_runTo = -1.0f;
	}
	{
		const bool cam = ui.Checkbox(ui.Row(), "Editor camera (right drag turns, middle / left pans, wheel; only while standing still)", g_camOn);
		if (cam != g_camOn) { g_camOn = cam; g_camInit = false; }
	}
	if (g_camOn && Frozen()) ApplyEditorCamera();
	else RestoreCamera();
	ui.Space();
	const std::string pages[3] = {"Scene", "Orbit", "Check (" + std::to_string(Check().size()) + ")"};
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1.3f});
		for (int i = 0; i < 3; i++)
			if (ui.Button(c[(std::size_t)i], pages[i], true, g_page == i)) g_page = i;
	}
	ui.Space(4);
	switch (g_page)
	{
	case 0: ScenePage(ui); break;
	case 1: OrbitPage(ui); break;
	default: CheckPage(ui); break;
	}
	ui.Space();
	if (!g_msg.empty()) ui.Label(g_msg, &devui::Color(devui::HINT));
	ui.Label("Game view: drag handles (X / Y / Z lock an axis), N new spawn at the playhead, T its course time = the playhead, Delete deletes the record, "
	         "Ctrl+Z / Y undo / redo, Ctrl+D duplicate, Ctrl+S save, Enter apply. A double click on a record plays to it.",
	         &devui::Color(devui::DIM));
	ViewInput();
}

// ------------------------------------------------------------------------------------------------ drawing (3D pass)

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
// a wire octahedron (unpac draws spheres)
void Diamond(const float p[3], float r, unsigned rgba)
{
	float v[6][3];
	for (int i = 0; i < 6; i++)
	{
		for (int k = 0; k < 3; k++) v[i][k] = p[k];
		v[i][i / 2] += (i & 1) ? r : -r;
	}
	for (int a = 0; a < 6; a++)
		for (int b = a + 1; b < 6; b++)
			if (a / 2 != b / 2) Line(v[a], v[b], rgba);
}
void Box(const float p[3], float r, unsigned rgba)
{
	for (int a = 0; a < 3; a++)
		for (int s1 = -1; s1 <= 1; s1 += 2)
			for (int s2 = -1; s2 <= 1; s2 += 2)
			{
				float u[3], w[3];
				const int b = (a + 1) % 3, c = (a + 2) % 3;
				u[a] = p[a] - r; w[a] = p[a] + r;
				u[b] = w[b] = p[b] + s1 * r;
				u[c] = w[c] = p[c] + s2 * r;
				Line(u, w, rgba);
			}
}
}

void G01EditOverride(const std::string& key, const std::uint8_t*& data, std::size_t& size)
{
	if (g_appliedStage < 0 || g_appliedScn.empty() || key != g_appliedName) return;
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	g01::cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	if (core == nullptr || core->GetJobNo() != kingdom::job::GALAGA_GAME || sys == nullptr || sys->nStage() != g_appliedStage || sys->IsMulti()) return;
	data = g_appliedScn.data();
	size = g_appliedScn.size();
}

void G01EditOrbits(int stage, std::vector<g01::cOrbit>& orbits)
{
	if (g_appliedStage < 0 || stage != g_appliedStage || g_appliedScn.empty()) return;
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	g01::cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	if (core == nullptr || core->GetJobNo() != kingdom::job::GALAGA_GAME || sys == nullptr || sys->IsMulti()) return;
	// the rows the way cOrbitDataMan::Load makes them (a row whose file is missing is an empty orbit)
	orbits.clear();
	for (const Orbit& o : g_applied.orbits)
	{
		g01::cOrbit c;
		for (const Key& k : o.keys)
		{
			g01::sEmyNode n{};
			for (int i = 0; i < 4; i++)
			{
				n.vPos[i] = WordF(k.w[(std::size_t)i]);
				n.vRot[i] = WordF(k.w[4 + (std::size_t)i]);
				n.vTangent[i] = WordF(k.w[8 + (std::size_t)i]);
			}
			n.fFrame = WordF(k.w[12]);
			n.nCmd = k.w[13];
			n.nParam = k.w[14];
			n.n3c = k.w[15];
			c.nodes.push_back(n);
		}
		orbits.push_back(std::move(c));
	}
}

bool G01EditFrozen()
{
	const bool f = Frozen();
	if (!f || !g_camOn) RestoreCamera();
	return f;
}

void G01EditDraw3D()
{
	if (!Active() || !g_editView || g_stage < 0 || g_stage != KingdomGlobal::GalagaSys->nStage()) return;
	const Rec* r = Sel();
	if (r == nullptr || !r->IsSpawn()) return;
	g_lines.clear();
	if (const Orbit* o = SelOrbit())
	{
		bool have = false;
		float prev[3];
		for (const auto& p : SampleOrbit(*o))
		{
			const float l[3] = {r->Parent(0) + p[0], r->Parent(1) + p[1], r->Parent(2) + p[2]};
			float w[3];
			LocalToWorld(l, w);
			if (have) Line(prev, w, 0x78dcffffu);
			std::copy(w, w + 3, prev);
			have = true;
		}
	}
	float eye[3], target[3], fovy;
	kingdom::NativeScene::GetCamera(eye, target, fovy);
	const std::vector<Handle> hs = Handles();
	for (const Handle& h : hs)
	{
		const float dist = std::sqrt((h.w[0] - eye[0]) * (h.w[0] - eye[0]) + (h.w[1] - eye[1]) * (h.w[1] - eye[1]) + (h.w[2] - eye[2]) * (h.w[2] - eye[2]));
		const float rad = std::max(h.r, dist * 0.008f);
		if (h.kind == H_TAN)
		{
			for (const Handle& n : hs)
				if (n.kind == H_NODE && n.node == h.node) Line(n.w, h.w, h.rgba);
			Box(h.w, rad * 0.7f, h.rgba);
		}
		else
			Diamond(h.w, rad, h.rgba);
	}
	if (!g_lines.empty()) gfx::DrawLines3D(g_lines.data(), g_lines.size(), false);
}

void RegisterG01EditPanel() { devui::RegisterPanel("Galaga editor", EditorPanel, [] { return InStage(); }); }
}
