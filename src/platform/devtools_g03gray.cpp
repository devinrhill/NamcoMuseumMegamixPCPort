// Graybox primitives for the Pac-N-Roll editor (see include/platform/devtools_g03gray.hpp): a stage made of boxes, ramps and cylinders. Developer-only,
// NOT part of the 1:1 game; nothing here exists in main.dol.
//
// A primitive is a convex solid (unit box / ramp / 16 sided cylinder) with a position, three Euler angles (R = Rz * Ry * Rx, degrees, like the rest of
// the editor) and a size per local axis. Its surface triangles are made in world space; they are drawn flat shaded (gfx::DrawTriangles3D) and, for a
// solid one, are the collision faces of a cTaskBgHit that sits at the origin of the stage (the same trick as --g03-bg-collision: faces in world space,
// floor = normal within 60 degrees of up). Edits change the faces in place (a rebuild only when a face changes between floor and wall), so Pac-Man
// can be rolled on a primitive that is being moved.
#include "platform/devtools_g03gray.hpp"
#include "platform/devtools.hpp"
#include "platform/devtools_view.hpp"
#include "platform/devui.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/Gfx3D.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskStage.hpp"
#include <Constants.hpp>

namespace devtools
{
namespace
{
namespace fs = std::filesystem;
using kingdom::game::g03::cFaceList;
using kingdom::game::g03::cTaskBgHit;
using kingdom::game::g03::cTaskCharPacman;
using kingdom::game::g03::cTaskStage;
using GConstants = kingdom::game::g03::Constants;
using V3 = std::array<float, 3>;
using Tri = std::array<V3, 3>;

enum Kind { K_BOX, K_RAMP, K_CYLINDER, K_COUNT };
const char* const KIND_NAMES[K_COUNT] = {"Box", "Ramp", "Cylinder"};
const char* const KIND_KEYS[K_COUNT] = {"box", "ramp", "cylinder"};
constexpr int CYL_SIDES = 16;
constexpr float MIN_SIZE = 0.05f;

struct Prim
{
	int kind = K_BOX;
	float pos[3] = {0, 0, 0}, rot[3] = {0, 0, 0}, size[3] = {2, 2, 2};
	bool solid = true;
	std::string name;
	bool operator==(const Prim& o) const
	{
		return kind == o.kind && solid == o.solid && name == o.name && std::equal(pos, pos + 3, o.pos) && std::equal(rot, rot + 3, o.rot) && std::equal(size, size + 3, o.size);
	}
};
using Prims = std::vector<Prim>;

// ------------------------------------------------------------------------------------------------ state

std::map<int, Prims> g_rounds;      // per round
Prims g_default;                    // from --g03-gray: a round without a list of its own starts as a copy
bool g_hasDefault = false;
std::string g_file;
bool g_fileRead = false;
bool g_only = false;                // graybox stage only (the stage's own models, collision and placement data are not made)
int g_sel = -1;
int g_mode = 0;                     // 0 move, 1 rotate, 2 scale
bool g_worldAxes = false, g_place = false;
int g_placeKind = K_BOX;
float g_snapMove = 0.5f, g_snapRot = 15.0f, g_snapSize = 0.5f;
std::string g_msg;
int g_pageFrame = -100;
int g_undoRound = -1;
std::vector<Prims> g_undo, g_redo;
int g_editFrame = -100, g_lastPushFrame = -100, g_lastPushPrim = -1;

// collision
struct Coll
{
	cTaskBgHit* obj = nullptr;
	std::vector<GConstants::sFace*> faces;   // in the order of Faces(prim)
	std::vector<char> floor;                 // the kind each face was filed under
	int kind = -1;
};
std::vector<Coll> g_coll;
cTaskStage* g_stage = nullptr;
bool g_hasStart = false;
float g_start[3] = {0, 5, 0}, g_bottom = -50.0f;

bool InStage()
{
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	return core != nullptr && core->GetJobNo() == kingdom::job::PACNROLL_GAME && KingdomGlobal::PacnRollSys != nullptr;
}
int Round() { return KingdomGlobal::PacnRollSys != nullptr ? (int)KingdomGlobal::PacnRollSys->nRound() : 0; }

void ReadFileOnce();
Prims& Cur()
{
	ReadFileOnce();
	const int r = Round();
	auto it = g_rounds.find(r);
	if (it == g_rounds.end()) it = g_rounds.emplace(r, g_hasDefault ? g_default : Prims()).first;
	return it->second;
}

// ------------------------------------------------------------------------------------------------ geometry

void RotMatrix(const float deg[3], float m[3][3])
{
	const float rx = deg[0] * 0.0174532925f, ry = deg[1] * 0.0174532925f, rz = deg[2] * 0.0174532925f;
	const float cx = std::cos(rx), sx = std::sin(rx), cy = std::cos(ry), sy = std::sin(ry), cz = std::cos(rz), sz = std::sin(rz);
	const float t[3][3] = {{cy * cz, sx * sy * cz - cx * sz, cx * sy * cz + sx * sz}, {cy * sz, sx * sy * sz + cx * cz, cx * sy * sz - sx * cz}, {-sy, sx * cy, cx * cy}};
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) m[r][c] = t[r][c];
}
// R = Rz * Ry * Rx back to the three angles (degrees); at gimbal lock (Y = +-90) the Z turn is put into X
void EulerFromMatrix(const float m[3][3], float deg[3])
{
	const float ry = std::asin(std::clamp(-m[2][0], -1.0f, 1.0f));
	float rx, rz;
	if (std::fabs(std::cos(ry)) > 1e-5f) { rx = std::atan2(m[2][1], m[2][2]); rz = std::atan2(m[1][0], m[0][0]); }
	else { rx = std::atan2(-m[1][2], m[1][1]); rz = 0.0f; }
	deg[0] = rx * 57.2957795f; deg[1] = ry * 57.2957795f; deg[2] = rz * 57.2957795f;
}
void MulMat(const float a[3][3], const float b[3][3], float out[3][3])
{
	float t[3][3];
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) t[r][c] = a[r][0] * b[0][c] + a[r][1] * b[1][c] + a[r][2] * b[2][c];
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) out[r][c] = t[r][c];
}
V3 Mul(const float m[3][3], const V3& v) { return {m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2], m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2], m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]}; }
V3 Sub(const V3& a, const V3& b) { return {a[0] - b[0], a[1] - b[1], a[2] - b[2]}; }
V3 Add(const V3& a, const V3& b) { return {a[0] + b[0], a[1] + b[1], a[2] + b[2]}; }
V3 Scale(const V3& a, float s) { return {a[0] * s, a[1] * s, a[2] * s}; }
V3 Cross(const V3& a, const V3& b) { return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]}; }
float Dot(const V3& a, const V3& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
float Len(const V3& a) { return std::sqrt(Dot(a, a)); }
V3 Norm(const V3& a) { const float l = Len(a); return l > 1e-12f ? Scale(a, 1.0f / l) : V3{0, 1, 0}; }

struct Poly
{
	std::vector<V3> v;
	std::vector<std::vector<int>> f;   // face loops, any winding (the triangles are turned outwards afterwards)
};

const Poly& Unit(int kind)
{
	static Poly polys[K_COUNT];
	static bool made = false;
	if (!made)
	{
		made = true;
		Poly& b = polys[K_BOX];
		for (int i = 0; i < 8; i++) b.v.push_back({(i & 1) ? 0.5f : -0.5f, (i & 2) ? 0.5f : -0.5f, (i & 4) ? 0.5f : -0.5f});
		b.f = {{0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4}, {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 3, 7, 5}};
		// the ramp rises towards +X: a triangular prism, the tall end at x = +0.5
		Poly& r = polys[K_RAMP];
		r.v = {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}};
		r.f = {{0, 1, 4, 3}, {1, 2, 5, 4}, {0, 2, 5, 3}, {0, 1, 2}, {3, 4, 5}};
		Poly& c = polys[K_CYLINDER];
		for (int i = 0; i < CYL_SIDES; i++)
		{
			const float a = 6.28318531f * (float)i / CYL_SIDES;
			c.v.push_back({0.5f * std::cos(a), -0.5f, 0.5f * std::sin(a)});
		}
		for (int i = 0; i < CYL_SIDES; i++) c.v.push_back({c.v[i][0], 0.5f, c.v[i][2]});
		for (int i = 0; i < CYL_SIDES; i++) c.f.push_back({i, (i + 1) % CYL_SIDES, CYL_SIDES + (i + 1) % CYL_SIDES, CYL_SIDES + i});
		std::vector<int> bot, top;
		for (int i = 0; i < CYL_SIDES; i++) { bot.push_back(i); top.push_back(CYL_SIDES + i); }
		c.f.push_back(bot);
		c.f.push_back(top);
	}
	return polys[std::clamp(kind, 0, (int)K_COUNT - 1)];
}

V3 World(const Prim& p, const V3& local, const float rm[3][3])
{
	const V3 s = {local[0] * p.size[0], local[1] * p.size[1], local[2] * p.size[2]};
	return Add({p.pos[0], p.pos[1], p.pos[2]}, Mul(rm, s));
}

// the surface triangles in world space, wound counter-clockwise seen from outside (turned by the polyhedron's centre, so a negative scale is fine)
std::vector<Tri> Tris(const Prim& p)
{
	const Poly& u = Unit(p.kind);
	float rm[3][3];
	RotMatrix(p.rot, rm);
	std::vector<V3> w;
	V3 cen = {0, 0, 0};
	for (const V3& l : u.v)
	{
		w.push_back(World(p, l, rm));
		cen = Add(cen, w.back());
	}
	cen = Scale(cen, 1.0f / (float)w.size());
	std::vector<Tri> out;
	for (const std::vector<int>& f : u.f)
		for (std::size_t i = 1; i + 1 < f.size(); i++)
		{
			V3 a = w[f[0]], b = w[f[i]], c = w[f[i + 1]];
			const V3 n = Cross(Sub(b, a), Sub(c, a));
			if (Len(n) < 1e-9f) continue;
			if (Dot(n, Sub(cen, a)) > 0.0f) std::swap(b, c);
			out.push_back({a, b, c});
		}
	return out;
}

V3 Normal(const Tri& t) { return Norm(Cross(Sub(t[1], t[0]), Sub(t[2], t[0]))); }

// ------------------------------------------------------------------------------------------------ collision

class cTaskGrayCollider : public cTaskBgHit
{
public:
	bool Execute(cJetTask* pParam) override
	{
		UpdateMatrix();
		return cTaskBgHit::Execute(pParam);
	}
};

AABB FacesAABB(const std::vector<GConstants::sFace*>& faces)
{
	const float big = std::numeric_limits<float>::max();
	Vector3 lo(big, big, big), hi(-big, -big, -big);
	for (const GConstants::sFace* f : faces)
		for (int l = 0; l < 3; l++)
		{
			lo = Vector3(std::min(lo.x, f->vVrt[l].x), std::min(lo.y, f->vVrt[l].y), std::min(lo.z, f->vVrt[l].z));
			hi = Vector3(std::max(hi.x, f->vVrt[l].x), std::max(hi.y, f->vVrt[l].y), std::max(hi.z, f->vVrt[l].z));
		}
	Bounds b;
	if (!faces.empty()) b.SetMinMax(lo, hi);
	return AABB(b);
}

// fills a face from a triangle; the game's winding for an outward normal is (a, c, b) (normal = (v2 - v0) x (v1 - v0), cFaceList::RegistFace)
bool FillFace(GConstants::sFace* f, const Tri& t)
{
	const V3 ord[3] = {t[0], t[2], t[1]};
	for (int l = 0; l < 3; l++) ::Constants::jetGeoVecSet(f->vVrt[l], ord[l][0], ord[l][1], ord[l][2], 1.0f);
	Vector4 e0, e1, n;
	::Constants::jetGeoVecSub(e0, f->vVrt[1], f->vVrt[0]);
	::Constants::jetGeoVecSub(e1, f->vVrt[2], f->vVrt[0]);
	::Constants::jetGeoVecOuterProduct(n, e1, e0);
	if (n[0] * n[0] + n[1] * n[1] + n[2] * n[2] < 1e-12f) return false;
	::Constants::jetGeoVecNormalize(f->vNorm, n);
	const Vector4* v = f->vVrt;
	f->aabb = AABB(Vector3(Mathf::Min(v[0].x, v[1].x, v[2].x), Mathf::Min(v[0].y, v[1].y, v[2].y), Mathf::Min(v[0].z, v[1].z, v[2].z)),
		Vector3(Mathf::Max(v[0].x, v[1].x, v[2].x), Mathf::Max(v[0].y, v[1].y, v[2].y), Mathf::Max(v[0].z, v[1].z, v[2].z)), false);
	return true;
}

bool IsFloor(const Tri& t) { return Normal(t)[1] >= 0.5f; }   // up to 60 degrees from up

void BuildList(Coll& c, const Prim& p)
{
	std::vector<GConstants::sFace*> lists[4];
	c.faces.clear();
	c.floor.clear();
	if (p.solid)
		for (const Tri& t : Tris(p))
		{
			GConstants::sFace* f = new GConstants::sFace();
			if (!FillFace(f, t)) { delete f; continue; }
			const bool fl = IsFloor(t);
			f->nKind = fl ? GConstants::eFACE_KIND_NORM : GConstants::eFACE_KIND_WALL;
			lists[fl ? 1 : 0].push_back(f);
			c.faces.push_back(f);
			c.floor.push_back(fl);
		}
	// the old list stays allocated: Pac-Man may still hold one of its faces as the floor under him
	c.obj->pFaceList() = c.faces.empty() ? nullptr : new cFaceList(nullptr, lists, FacesAABB(c.faces));
	c.obj->IsCollision() = !c.faces.empty();
	c.kind = p.kind;
	c.obj->UpdateMatrix();
}

// new geometry for the same kind: in place when no face changes between floor and wall
void Refill(Coll& c, const Prim& p)
{
	if (!p.solid || c.kind != p.kind || c.obj->pFaceList() == nullptr) { BuildList(c, p); return; }
	const std::vector<Tri> tris = Tris(p);
	if (tris.size() != c.faces.size()) { BuildList(c, p); return; }
	for (std::size_t i = 0; i < tris.size(); i++)
		if ((char)IsFloor(tris[i]) != c.floor[i]) { BuildList(c, p); return; }
	for (std::size_t i = 0; i < tris.size(); i++)
		if (!FillFace(c.faces[i], tris[i])) { BuildList(c, p); return; }
	c.obj->pFaceList()->ReadOnlyAABB() = FacesAABB(c.faces);
}

Coll MakeCollider(cTaskStage* stage)
{
	Coll c;
	cTaskGrayCollider* obj = new cTaskGrayCollider();
	::Constants::jetGeoVecSet(obj->Model->vTrans(), 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(obj->Model->vRotate(), 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(obj->Model->vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
	obj->IsMove() = true;
	obj->nDrawPriority(65536u);
	stage->ConnectChild(obj);
	c.obj = obj;
	return c;
}

// the collision objects follow the list of the current round: `only` = just that primitive (its geometry changed), -1 = all
void Sync(int only = -1)
{
	kingdom::game::g03::cAppG03PacnRollGame* app = GConstants::PACN_APP();
	if (app == nullptr || app->pTaskStage() == nullptr) return;
	if (app->pTaskStage() != g_stage) { g_stage = app->pTaskStage(); g_coll.clear(); only = -1; }
	Prims& P = Cur();
	while (g_coll.size() < P.size()) { g_coll.push_back(MakeCollider(g_stage)); only = -1; }
	for (std::size_t i = 0; i < g_coll.size(); i++)
	{
		if (i >= P.size())
		{
			if (g_coll[i].obj->pFaceList() != nullptr) { g_coll[i].obj->pFaceList() = nullptr; g_coll[i].obj->IsCollision() = false; g_coll[i].kind = -1; }
			continue;
		}
		if (only >= 0 && (int)i != only) continue;
		Refill(g_coll[i], P[i]);
	}
}

void Bounds3(const Prims& P, float lo[3], float hi[3])
{
	const float big = std::numeric_limits<float>::max();
	for (int i = 0; i < 3; i++) { lo[i] = big; hi[i] = -big; }
	for (const Prim& p : P)
	{
		if (!p.solid) continue;
		for (const Tri& t : Tris(p))
			for (const V3& v : t)
				for (int i = 0; i < 3; i++) { lo[i] = std::min(lo[i], v[i]); hi[i] = std::max(hi[i], v[i]); }
	}
}

// ------------------------------------------------------------------------------------------------ file

void Parse(const nlohmann::json& j, Prims& out, bool& only)
{
	out.clear();
	if (j.contains("only") && j["only"].is_boolean()) only = j["only"].get<bool>();
	if (!j.contains("prims") || !j["prims"].is_array()) return;
	for (const auto& e : j["prims"])
	{
		Prim p;
		const std::string k = e.value("kind", "box");
		for (int i = 0; i < K_COUNT; i++)
			if (k == KIND_KEYS[i]) p.kind = i;
		p.name = e.value("name", std::string());
		p.solid = e.value("solid", true);
		const char* const keys[3] = {"pos", "rot", "size"};
		float* const dst[3] = {p.pos, p.rot, p.size};
		for (int a = 0; a < 3; a++)
			if (e.contains(keys[a]) && e[keys[a]].is_array() && e[keys[a]].size() == 3)
				for (int i = 0; i < 3; i++) dst[a][i] = e[keys[a]][i].get<float>();
		for (int i = 0; i < 3; i++) p.size[i] = std::max(p.size[i], MIN_SIZE);
		out.push_back(p);
	}
}

nlohmann::json ToJson(const Prims& P, int round)
{
	nlohmann::json j;
	j["version"] = 1;
	j["round"] = round;
	j["only"] = g_only;
	j["prims"] = nlohmann::json::array();
	for (const Prim& p : P)
	{
		nlohmann::json e;
		e["name"] = p.name;
		e["kind"] = KIND_KEYS[p.kind];
		e["solid"] = p.solid;
		e["pos"] = {p.pos[0], p.pos[1], p.pos[2]};
		e["rot"] = {p.rot[0], p.rot[1], p.rot[2]};
		e["size"] = {p.size[0], p.size[1], p.size[2]};
		j["prims"].push_back(e);
	}
	return j;
}

void ReadFileOnce()
{
	if (g_fileRead || g_file.empty()) return;
	g_fileRead = true;
	std::ifstream in(g_file);
	if (!in) { std::fprintf(stderr, "devtools: --g03-gray: cannot read %s\n", g_file.c_str()); return; }
	try
	{
		Parse(nlohmann::json::parse(in), g_default, g_only);
		g_hasDefault = true;
		std::fprintf(stderr, "devtools: --g03-gray: %zu primitives from %s%s\n", g_default.size(), g_file.c_str(), g_only ? " (graybox stage only)" : "");
	}
	catch (const std::exception& e) { std::fprintf(stderr, "devtools: --g03-gray: %s: %s\n", g_file.c_str(), e.what()); }
}

// ------------------------------------------------------------------------------------------------ undo

void PushUndo(int prim = -1)
{
	if (g_undoRound != Round()) { g_undo.clear(); g_redo.clear(); g_undoRound = Round(); }
	g_editFrame = FrameNo();
	if (prim >= 0 && prim == g_lastPushPrim && FrameNo() - g_lastPushFrame < 30) { g_lastPushFrame = FrameNo(); return; }   // one step per edit burst
	g_lastPushPrim = prim;
	g_lastPushFrame = FrameNo();
	g_undo.push_back(Cur());
	if (g_undo.size() > 200) g_undo.erase(g_undo.begin());
	g_redo.clear();
}

std::string NameOf(int i, const Prim& p) { return p.name.empty() ? std::string(KIND_NAMES[p.kind]) + " " + std::to_string(i + 1) : p.name; }

// ------------------------------------------------------------------------------------------------ drawing

std::vector<gfx::LineVertex> g_lines;
std::vector<gfx::TriVertex> g_tris;

void Line(const V3& p, const V3& q, unsigned rgba)
{
	gfx::LineVertex v{};
	for (int k = 0; k < 4; k++) v.rgba[k] = (std::uint8_t)(rgba >> (24 - 8 * k));
	for (int k = 0; k < 3; k++) v.pos[k] = p[k];
	g_lines.push_back(v);
	for (int k = 0; k < 3; k++) v.pos[k] = q[k];
	g_lines.push_back(v);
}

void Eye(float e[3])
{
	float v[16], pr[16];
	kingdom::NativeScene::GetMatrices(v, pr);
	for (int i = 0; i < 3; i++) e[i] = -(v[i * 4 + 0] * v[12] + v[i * 4 + 1] * v[13] + v[i * 4 + 2] * v[14]);
}

// the gizmo: the three handle axes of the current mode at the selected primitive's centre, `len` long
struct Gizmo
{
	V3 c, axis[3];
	float len;
};
bool MakeGizmo(Gizmo& g)
{
	const Prims& P = Cur();
	if (g_sel < 0 || g_sel >= (int)P.size()) return false;
	const Prim& p = P[g_sel];
	g.c = {p.pos[0], p.pos[1], p.pos[2]};
	float e[3];
	Eye(e);
	g.len = std::max(0.5f, Len(Sub({e[0], e[1], e[2]}, g.c)) * 0.14f);
	float rm[3][3];
	RotMatrix(p.rot, rm);
	const V3 ex = {1, 0, 0}, ey = {0, 1, 0}, ez = {0, 0, 1};
	if (g_mode == 2) { g.axis[0] = Mul(rm, ex); g.axis[1] = Mul(rm, ey); g.axis[2] = Mul(rm, ez); }   // scale: always the primitive's own axes
	else if (g_worldAxes) { g.axis[0] = ex; g.axis[1] = ey; g.axis[2] = ez; }
	else if (g_mode == 1)
	{
		// the axes the three Euler angles turn about: R = Rz * Ry * Rx, so X turns about the primitive's own X, Z about the world's Z
		const float rz[3] = {0, 0, p.rot[2]};
		float mz[3][3];
		RotMatrix(rz, mz);
		g.axis[0] = Mul(rm, ex);
		g.axis[1] = Mul(mz, ey);
		g.axis[2] = ez;
	}
	else
	{
		g.axis[0] = Mul(rm, ex); g.axis[1] = Mul(rm, ey); g.axis[2] = Mul(rm, ez);
	}
	return true;
}
const unsigned AXIS_RGBA[3] = {0xff4040ffu, 0x40e040ffu, 0x4080ffffu};

void Ring(const Gizmo& g, int a, int k, V3& pt, V3& tan)
{
	const V3 n = g.axis[a];
	const V3 helper = std::fabs(n[1]) < 0.9f ? V3{0, 1, 0} : V3{1, 0, 0};
	const V3 u = Norm(Cross(n, helper)), v = Cross(n, u);
	const float th = 6.28318531f * (float)k / 64.0f;
	pt = Add(g.c, Scale(Add(Scale(u, std::cos(th)), Scale(v, std::sin(th))), g.len));
	tan = Add(Scale(u, -std::sin(th)), Scale(v, std::cos(th)));
}

void DrawGizmo()
{
	Gizmo g;
	if (!MakeGizmo(g)) return;
	for (int a = 0; a < 3; a++)
	{
		if (g_mode == 1)
		{
			V3 prev, t;
			Ring(g, a, 0, prev, t);
			const V3 first = prev;
			for (int k = 1; k <= 64; k++)
			{
				V3 pt;
				Ring(g, a, k % 64, pt, t);
				Line(prev, pt, AXIS_RGBA[a]);
				prev = pt;
			}
			(void)first;
		}
		else
		{
			const V3 tip = Add(g.c, Scale(g.axis[a], g.len));
			Line(g.c, tip, AXIS_RGBA[a]);
			// a cube-ish tip for scale, an arrow head for move
			const V3 s = g.axis[a == 0 ? 1 : 0], t = Cross(g.axis[a], s);
			const float h = g.len * 0.08f;
			if (g_mode == 0)
			{
				const V3 base = Add(g.c, Scale(g.axis[a], g.len * 0.85f));
				for (const V3& side : {s, Scale(s, -1.0f), t, Scale(t, -1.0f)}) Line(tip, Add(base, Scale(side, h)), AXIS_RGBA[a]);
			}
			else
			{
				for (const V3& d : {Add(s, t), Sub(s, t), Sub(t, s), Scale(Add(s, t), -1.0f)})
				{
					const V3 q = Add(tip, Scale(d, h));
					Line(tip, q, AXIS_RGBA[a]);
				}
			}
		}
	}
}

void Shade(const Tri& t, bool solid, bool sel)
{
	static const V3 light = Norm({0.35f, 0.85f, 0.40f});
	const V3 n = Normal(t);
	const float k = 0.38f + 0.62f * std::max(0.0f, Dot(n, light));
	const float base = sel ? 0.72f : 0.60f;
	float r = base * k, gc = base * k, b = base * k;
	if (sel) { r *= 1.12f; b *= 0.82f; }
	auto c8 = [](float x) { return (std::uint8_t)std::clamp((int)(x * 255.0f + 0.5f), 0, 255); };
	gfx::TriVertex v{};
	v.rgba[0] = c8(r); v.rgba[1] = c8(gc); v.rgba[2] = c8(b); v.rgba[3] = solid ? 255 : 90;
	for (int i = 0; i < 3; i++)
	{
		for (int k2 = 0; k2 < 3; k2++) v.pos[k2] = t[i][k2];
		g_tris.push_back(v);
	}
}

// the silhouette and crease lines of a primitive: every triangle edge of the polyhedron's faces that is not interior to a flat face
void Outline(const Prim& p, unsigned rgba)
{
	const Poly& u = Unit(p.kind);
	float rm[3][3];
	RotMatrix(p.rot, rm);
	std::vector<V3> w;
	for (const V3& l : u.v) w.push_back(World(p, l, rm));
	for (const std::vector<int>& f : u.f)
		for (std::size_t i = 0; i < f.size(); i++) Line(w[f[i]], w[f[(i + 1) % f.size()]], rgba);
}

}   // namespace

// ------------------------------------------------------------------------------------------------ game side

void SetG03GrayFile(const std::string& path) { g_file = path; g_fileRead = false; }
void SetG03GrayOnly(bool on) { g_only = on; }
bool G03GrayOnly() { return g_only && InStage(); }

bool G03GrayStart(float out[3])
{
	if (!G03GrayOnly() || !g_hasStart) return false;
	for (int i = 0; i < 3; i++) out[i] = g_start[i];
	return true;
}

void G03GrayStage(cTaskStage* stage, std::vector<cTaskBgHit*>& hits)
{
	g_stage = stage;
	g_coll.clear();
	g_hasStart = false;
	Prims& P = Cur();
	for (std::size_t i = 0; i < P.size(); i++)
	{
		g_coll.push_back(MakeCollider(stage));
		BuildList(g_coll.back(), P[i]);
		hits.push_back(g_coll.back().obj);
	}
	if (!G03GrayOnly()) return;
	// Pac-Man starts above the middle of the first solid primitive, and is put back there when he falls 30 units below all of them
	float lo[3], hi[3];
	Bounds3(P, lo, hi);
	g_bottom = lo[0] > hi[0] ? -50.0f : lo[1];
	for (const Prim& p : P)
		if (p.solid)
		{
			float top = -std::numeric_limits<float>::max();
			for (const Tri& t : Tris(p))
				for (const V3& v : t) top = std::max(top, v[1]);
			g_start[0] = p.pos[0]; g_start[1] = top + 2.0f; g_start[2] = p.pos[2];
			break;
		}
	if (P.empty() || lo[0] > hi[0]) { g_start[0] = 0; g_start[1] = 5; g_start[2] = 0; }
	g_hasStart = true;
	std::fprintf(stderr, "devtools: graybox stage: %zu primitives, start (%g %g %g)\n", P.size(), g_start[0], g_start[1], g_start[2]);
}

bool G03GrayFell(cTaskCharPacman* pac)
{
	if (!G03GrayOnly() || !g_hasStart || pac->vTrans()[1] >= g_bottom - 30.0f) return false;
	pac->vTrans() = Vector4(g_start[0], g_start[1], g_start[2], 1.0f);
	::Constants::jetGeoVecZero(pac->vMove());
	return true;
}

void G03GrayDraw3D()
{
	if (!InStage()) return;
	const Prims& P = Cur();
	if (P.empty()) return;
	g_tris.clear();
	g_lines.clear();
	const bool editing = FrameNo() - g_pageFrame <= 2;
	for (std::size_t i = 0; i < P.size(); i++)
	{
		const bool sel = editing && (int)i == g_sel;
		for (const Tri& t : Tris(P[i])) Shade(t, P[i].solid, sel);
		if (editing || !P[i].solid) Outline(P[i], sel ? 0xffd040ffu : (P[i].solid ? 0x303030ffu : 0x40e0ffffu));
	}
	// solid surfaces first (they hide what is behind them), the see-through ones after, without writing depth
	std::vector<gfx::TriVertex> solid, ghost;
	for (std::size_t i = 0; i < g_tris.size(); i += 3) (g_tris[i].rgba[3] == 255 ? solid : ghost).insert((g_tris[i].rgba[3] == 255 ? solid : ghost).end(), g_tris.begin() + (long)i, g_tris.begin() + (long)i + 3);
	if (!solid.empty()) gfx::DrawTriangles3D(solid.data(), solid.size(), true, true);
	if (!ghost.empty()) gfx::DrawTriangles3D(ghost.data(), ghost.size(), true, false);
	if (!g_lines.empty() && !editing) gfx::DrawLines3D(g_lines.data(), g_lines.size(), true);
	if (editing)
	{
		if (!g_lines.empty()) gfx::DrawLines3D(g_lines.data(), g_lines.size(), true);
		g_lines.clear();
		DrawGizmo();
		if (!g_lines.empty()) gfx::DrawLines3D(g_lines.data(), g_lines.size(), false);
	}
}

// ------------------------------------------------------------------------------------------------ editor: view input

namespace
{
// the drag of a gizmo handle or of a primitive's body
struct DragState
{
	bool on = false;
	int mode = 0, axis = -1, prim = -1;
	bool body = false, world = false;
	Prim start;
	float mx0 = 0, my0 = 0;
	float sdx = 0, sdy = 0;            // the handle's direction on the screen (unit for rotate, tip - centre for the others)
	float rpx = 1;                     // rotate: the ring's radius on the screen
	V3 axisW{0, 1, 0};
	float len = 1;
	float planeY = 0;
	V3 grab{0, 0, 0};
	bool moved = false;
} g_drag;

float DistSeg(float px, float py, float ax, float ay, float bx, float by)
{
	const float dx = bx - ax, dy = by - ay, l2 = dx * dx + dy * dy;
	float t = l2 > 0 ? ((px - ax) * dx + (py - ay) * dy) / l2 : 0.0f;
	t = std::clamp(t, 0.0f, 1.0f);
	const float cx = ax + t * dx - px, cy = ay + t * dy - py;
	return std::sqrt(cx * cx + cy * cy);
}

bool RayTri(const float o[3], const float d[3], const Tri& tr, float& t)
{
	const V3 e1 = Sub(tr[1], tr[0]), e2 = Sub(tr[2], tr[0]), s = Sub({o[0], o[1], o[2]}, tr[0]), dv = {d[0], d[1], d[2]};
	const V3 h = Cross(dv, e2);
	const float a = Dot(e1, h);
	if (std::fabs(a) < 1e-9f) return false;
	const float f = 1.0f / a, u = f * Dot(s, h);
	if (u < 0.0f || u > 1.0f) return false;
	const V3 q = Cross(s, e1);
	const float v = f * Dot(dv, q);
	if (v < 0.0f || u + v > 1.0f) return false;
	t = f * Dot(e2, q);
	return t > 1e-4f;
}

int PickPrim(const float o[3], const float d[3])
{
	const Prims& P = Cur();
	int best = -1;
	float bt = 1e30f;
	for (std::size_t i = 0; i < P.size(); i++)
		for (const Tri& t : Tris(P[i]))
		{
			float x;
			if (RayTri(o, d, t, x) && x < bt) { bt = x; best = (int)i; }
		}
	return best;
}

float SnapTo(float v, float step) { return step > 0.0f ? std::round(v / step) * step : v; }

void Touch(int i)
{
	Sync(i);
	g_msg.clear();
}

Prim NewPrim(int kind, const float at[3], bool ground)
{
	Prim p;
	p.kind = kind;
	static const float sizes[K_COUNT][3] = {{2, 2, 2}, {4, 2, 4}, {2, 3, 2}};
	for (int i = 0; i < 3; i++) { p.size[i] = sizes[kind][i]; p.pos[i] = at[i]; }
	if (ground) p.pos[1] += p.size[1] * 0.5f;
	return p;
}

int AddPrim(const Prim& src)
{
	Prims& P = Cur();
	PushUndo();
	g_lastPushPrim = -1;
	P.push_back(src);
	g_sel = (int)P.size() - 1;
	Sync();
	return g_sel;
}

void DeletePrim(int i)
{
	Prims& P = Cur();
	if (i < 0 || i >= (int)P.size()) return;
	PushUndo();
	g_lastPushPrim = -1;
	P.erase(P.begin() + i);
	g_sel = -1;
	Sync();
	g_msg = "primitive deleted";
}
}

bool GrayCanUndo() { return !g_undo.empty() && g_undoRound == Round(); }
bool GrayCanRedo() { return !g_redo.empty() && g_undoRound == Round(); }
bool GrayHasSelection() { return g_sel >= 0 && InStage() && g_sel < (int)Cur().size(); }
bool GrayShown() { return FrameNo() - g_pageFrame <= 2; }
void GrayDeselect() { g_sel = -1; }
bool GrayDragging() { return g_drag.on; }
int GrayLastEditFrame() { return g_editFrame; }

bool GrayPressGizmo(const devui::Input& in, const float vp[16], const float o[3], const float d[3])
{
	(void)o; (void)d;
	Gizmo g;
	if (!GrayShown() || !MakeGizmo(g)) return false;
	float cx, cy;
	if (!Project(vp, &g.c[0], cx, cy)) return false;
	int bestA = -1, bestK = 0;
	float bestD = 1e30f;
	for (int a = 0; a < 3; a++)
	{
		if (g_mode == 1)
		{
			float px, py, lastx = 0, lasty = 0;
			bool have = false;
			for (int k = 0; k <= 64; k++)
			{
				V3 pt, t;
				Ring(g, a, k % 64, pt, t);
				if (!Project(vp, &pt[0], px, py)) { have = false; continue; }
				if (have)
				{
					const float dd = DistSeg(in.mx, in.my, lastx, lasty, px, py);
					if (dd < bestD) { bestD = dd; bestA = a; bestK = k % 64; }
				}
				lastx = px; lasty = py; have = true;
			}
		}
		else
		{
			const V3 tip = Add(g.c, Scale(g.axis[a], g.len));
			float tx, ty;
			if (!Project(vp, &tip[0], tx, ty)) continue;
			const float dd = std::min(DistSeg(in.mx, in.my, cx, cy, tx, ty), std::sqrt((in.mx - tx) * (in.mx - tx) + (in.my - ty) * (in.my - ty)) - 5.0f);
			if (dd < bestD) { bestD = dd; bestA = a; }
		}
	}
	if (bestA < 0 || bestD > 9.0f) return false;
	PushUndo(g_sel);
	g_lastPushPrim = -1;                  // one undo step per drag, whatever the burst rule says
	g_drag = DragState{};
	g_drag.on = true;
	g_drag.mode = g_mode;
	g_drag.world = g_worldAxes && g_mode == 1;
	g_drag.axis = bestA;
	g_drag.prim = g_sel;
	g_drag.start = Cur()[g_sel];
	g_drag.mx0 = in.mx;
	g_drag.my0 = in.my;
	g_drag.axisW = g.axis[bestA];
	g_drag.len = g.len;
	if (g_mode == 1)
	{
		V3 pt, tan;
		Ring(g, bestA, bestK, pt, tan);
		float ax, ay, bx, by;
		const V3 pt2 = Add(pt, Scale(tan, g.len * 0.02f));
		Project(vp, &pt[0], ax, ay);
		Project(vp, &pt2[0], bx, by);
		const float l = std::max(1e-6f, std::sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
		g_drag.sdx = (bx - ax) / l;
		g_drag.sdy = (by - ay) / l;
		g_drag.rpx = std::max(10.0f, std::sqrt((ax - cx) * (ax - cx) + (ay - cy) * (ay - cy)));
	}
	else
	{
		const V3 tip = Add(g.c, Scale(g.axis[bestA], g.len));
		float tx, ty;
		Project(vp, &tip[0], tx, ty);
		g_drag.sdx = tx - cx;
		g_drag.sdy = ty - cy;
	}
	return true;
}

bool GrayPressBody(const devui::Input& in, const float o[3], const float d[3], const std::function<bool(float[3])>& ground)
{
	(void)in;
	if (!GrayShown()) return false;
	const int hit = PickPrim(o, d);
	if (hit < 0)
	{
		if (!g_place) return false;
		float at[3];
		if (!ground(at)) return false;
		AddPrim(NewPrim(g_placeKind, at, true));
		g_msg = std::string(KIND_NAMES[g_placeKind]) + " placed";
		return true;
	}
	g_sel = hit;
	if (g_mode != 0) return true;
	// move: drag the body on the plane at its centre's height (Alt: up / down)
	PushUndo(hit);
	g_lastPushPrim = -1;
	g_drag = DragState{};
	g_drag.on = true;
	g_drag.body = true;
	g_drag.mode = 0;
	g_drag.prim = hit;
	g_drag.start = Cur()[hit];
	g_drag.mx0 = in.mx;
	g_drag.my0 = in.my;
	g_drag.planeY = g_drag.start.pos[1];
	if (std::fabs(d[1]) > 1e-6f)
	{
		const float t = (g_drag.planeY - o[1]) / d[1];
		for (int i = 0; i < 3; i++) g_drag.grab[i] = o[i] + d[i] * t;
	}
	return true;
}

void GrayDrag(const devui::Input& in, const float o[3], const float d[3])
{
	if (!g_drag.on) return;
	Prims& P = Cur();
	if (g_drag.prim < 0 || g_drag.prim >= (int)P.size() || !in.down[0]) { g_drag.on = false; return; }
	Prim p = g_drag.start;
	const float dx = in.mx - g_drag.mx0, dy = in.my - g_drag.my0;
	if (g_drag.body)
	{
		if (in.alt)
		{
			float v[16], pr[16];
			kingdom::NativeScene::GetMatrices(v, pr);
			const devui::Rect gv = devui::GameView();
			float e[3];
			Eye(e);
			const float dist = Len(Sub({e[0], e[1], e[2]}, {p.pos[0], p.pos[1], p.pos[2]}));
			p.pos[1] = g_drag.start.pos[1] - dy * 2.0f * dist / (pr[5] * (float)gv.h);
		}
		else if (std::fabs(d[1]) > 1e-6f)
		{
			const float t = (g_drag.planeY - o[1]) / d[1];
			if (t > 0.0f)
			{
				p.pos[0] = g_drag.start.pos[0] + (o[0] + d[0] * t - g_drag.grab[0]);
				p.pos[2] = g_drag.start.pos[2] + (o[2] + d[2] * t - g_drag.grab[2]);
			}
		}
		if (in.ctrl) for (int i = 0; i < 3; i++) p.pos[i] = SnapTo(p.pos[i], g_snapMove);
	}
	else if (g_drag.mode == 1)
	{
		float deg = (dx * g_drag.sdx + dy * g_drag.sdy) / g_drag.rpx * 57.2957795f;
		if (in.ctrl) deg = SnapTo(deg, g_snapRot);
		if (g_drag.world)
		{
			// about a world axis: R = R(axis, deg) * R(start), then the three angles of that
			float rs[3][3], ra[3][3], out[3][3];
			RotMatrix(g_drag.start.rot, rs);
			float axisDeg[3] = {0, 0, 0};
			axisDeg[g_drag.axis] = deg;
			RotMatrix(axisDeg, ra);
			MulMat(ra, rs, out);
			EulerFromMatrix(out, p.rot);
		}
		else
			p.rot[g_drag.axis] = g_drag.start.rot[g_drag.axis] + deg;
	}
	else
	{
		const float l2 = std::max(1e-6f, g_drag.sdx * g_drag.sdx + g_drag.sdy * g_drag.sdy);
		float amount = (dx * g_drag.sdx + dy * g_drag.sdy) / l2 * g_drag.len;
		if (g_drag.mode == 0)
		{
			if (in.ctrl) amount = SnapTo(amount, g_snapMove);
			for (int i = 0; i < 3; i++) p.pos[i] = g_drag.start.pos[i] + g_drag.axisW[i] * amount;
			if (in.ctrl) for (int i = 0; i < 3; i++) p.pos[i] = SnapTo(p.pos[i], g_snapMove);
		}
		else if (in.shift)
		{
			const float f = std::max(0.05f, 1.0f + amount / g_drag.len);
			for (int i = 0; i < 3; i++) p.size[i] = std::max(MIN_SIZE, g_drag.start.size[i] * f);
		}
		else
		{
			float s = g_drag.start.size[g_drag.axis] + amount;
			if (in.ctrl) s = SnapTo(s, g_snapSize);
			p.size[g_drag.axis] = std::max(MIN_SIZE, s);
		}
	}
	if (!(p == P[g_drag.prim]))
	{
		P[g_drag.prim] = p;
		g_drag.moved = true;
		Touch(g_drag.prim);
	}
}

bool GrayKeys(const devui::Input& in)
{
	if (!GrayShown()) return false;
	bool used = false;
	if (!in.ctrl && devui::KeyPressed('1')) { g_mode = 0; used = true; }
	if (!in.ctrl && devui::KeyPressed('2')) { g_mode = 1; used = true; }
	if (!in.ctrl && devui::KeyPressed('3')) { g_mode = 2; used = true; }
	if (!in.ctrl && devui::KeyPressed('Q')) { g_worldAxes = !g_worldAxes; used = true; }
	if (!GrayHasSelection()) return used;
	Prims& P = Cur();
	if (devui::KeyPressed(devui::K_DELETE) || devui::KeyPressed(devui::K_BACKSPACE) || (!in.ctrl && devui::KeyPressed('X'))) { DeletePrim(g_sel); return true; }
	if (in.ctrl && devui::KeyPressed('D'))
	{
		Prim c = P[g_sel];
		c.name.clear();
		c.pos[0] += 1.0f;
		c.pos[2] += 1.0f;
		AddPrim(c);
		g_msg = "duplicated";
		return true;
	}
	const int updown = devui::KeyPressed(devui::K_PAGE_UP) ? 1 : (devui::KeyPressed(devui::K_PAGE_DOWN) ? -1 : 0);
	if (updown != 0)
	{
		PushUndo(g_sel);
		P[g_sel].pos[1] += updown * (in.shift ? 0.1f : g_snapMove);
		Touch(g_sel);
		return true;
	}
	return used;
}

void GrayUndo()
{
	if (g_undo.empty() || g_undoRound != Round()) return;
	g_redo.push_back(Cur());
	Cur() = g_undo.back();
	g_undo.pop_back();
	g_sel = std::min(g_sel, (int)Cur().size() - 1);
	g_editFrame = FrameNo();
	Sync();
}

void GrayRedo()
{
	if (g_redo.empty() || g_undoRound != Round()) return;
	g_undo.push_back(Cur());
	Cur() = g_redo.back();
	g_redo.pop_back();
	g_sel = std::min(g_sel, (int)Cur().size() - 1);
	g_editFrame = FrameNo();
	Sync();
}

void GraySave(const std::string& dir)
{
	const fs::path p = fs::path(dir) / "graybox.json";
	std::error_code ec;
	fs::create_directories(p.parent_path(), ec);
	std::ofstream out(p);
	out << ToJson(Cur(), Round()).dump(1) << "\n";
	g_msg = out.good() ? "saved " + std::to_string(Cur().size()) + " primitives to " + p.string() : "could not write " + p.string();
}

void GrayLoad(const std::string& dir)
{
	const fs::path p = fs::path(dir) / "graybox.json";
	std::ifstream in(p);
	if (!in) { g_msg = "no graybox.json in " + dir; return; }
	try
	{
		Prims loaded;
		bool only = g_only;
		Parse(nlohmann::json::parse(in), loaded, only);
		PushUndo();
		g_lastPushPrim = -1;
		Cur() = loaded;
		g_only = only;
		g_sel = -1;
		Sync();
		g_msg = "loaded " + std::to_string(loaded.size()) + " primitives" + (g_only ? " (graybox stage only: restart to apply)" : "");
	}
	catch (const std::exception& e) { g_msg = std::string("graybox.json: ") + e.what(); }
}

// ------------------------------------------------------------------------------------------------ editor: the page

void GrayPage(devui::UI& ui, const float placeAt[3], const std::string& dir, const std::function<void()>& restart)
{
	g_pageFrame = FrameNo();
	Prims& P = Cur();
	if (g_sel >= (int)P.size()) g_sel = -1;
	ui.Label("Boxes, ramps and cylinders that are drawn flat grey and are solid for Pac-Man at once (no restart). Click one in the game view to select it.",
	         &devui::Color(devui::DIM));
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
		for (int k = 0; k < K_COUNT; k++)
			if (ui.Button(c[k], std::string("+ ") + KIND_NAMES[k])) { AddPrim(NewPrim(k, placeAt, true)); g_msg = std::string(KIND_NAMES[k]) + " added"; }
	}
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1.2f, 1});
		g_place = ui.Checkbox(c[0], "Place on click", g_place);
		g_placeKind = ui.Choice(c[1], devui::MakeId("g03gray.pk"), "kind", {KIND_NAMES[0], KIND_NAMES[1], KIND_NAMES[2]}, g_placeKind);
	}
	if (ui.Button(ui.Row(), "Add a floor (20 x 1 x 20 under Pac-Man)"))
	{
		Prim f;
		f.name = "Floor";
		f.size[0] = 20; f.size[1] = 1; f.size[2] = 20;
		f.pos[0] = placeAt[0]; f.pos[1] = placeAt[1] - 0.9f; f.pos[2] = placeAt[2];   // the top 0.4 under Pac-Man's centre
		AddPrim(f);
	}
	g_only = ui.Checkbox(ui.Row(), "Graybox only (hides the stage; Restart)", g_only);
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1, 1});
		if (ui.Button(c[0], "Restart")) restart();
		if (ui.Button(c[1], "Undo", !g_undo.empty() && g_undoRound == Round())) GrayUndo();
		if (ui.Button(c[2], "Redo", !g_redo.empty() && g_undoRound == Round())) GrayRedo();
		if (ui.Button(c[3], "Save")) GraySave(dir);
	}
	if (ui.Button(ui.Row(), "Load graybox.json")) GrayLoad(dir);
	ui.Space();
	static const char* const modes[] = {"1 Move", "2 Rotate", "3 Scale"};
	{
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
		for (int i = 0; i < 3; i++)
			if (ui.Button(c[i], modes[i], true, g_mode == i)) g_mode = i;
	}
	g_worldAxes = ui.Choice(ui.Row(), devui::MakeId("g03gray.axes"), "axes (Q)", {"Local (the primitive's own)", "World"}, g_worldAxes ? 1 : 0) == 1;
	ui.Label("Move and Rotate follow the axes above; Scale is always along the primitive's own.", &devui::Color(devui::DIM));
	{
		ui.Label("Ctrl snaps to: move / turn (deg) / size");
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
		g_snapMove = (float)ui.Number(c[0], devui::MakeId("g03gray.sm"), "", g_snapMove, 0.25, "%.2f");
		g_snapRot = (float)ui.Number(c[1], devui::MakeId("g03gray.sr"), "", g_snapRot, 5, "%.0f");
		g_snapSize = (float)ui.Number(c[2], devui::MakeId("g03gray.ss"), "", g_snapSize, 0.25, "%.2f");
	}
	ui.Space();
	std::vector<devui::ListRow> rows;
	for (std::size_t i = 0; i < P.size(); i++)
		rows.push_back(devui::ListRow{NameOf((int)i, P[i]) + "  (" + KIND_NAMES[P[i].kind] + (P[i].solid ? "" : ", no collision") + ")",
		                              devui::Color((int)i == g_sel ? devui::TEXT : devui::DIM)});
	if (rows.empty()) ui.Label("no primitives in this round", &devui::Color(devui::DIM));
	else
	{
		bool activated = false;
		const int picked = ui.ListBox(devui::MakeId("g03gray.list"), ui.Row(devui::ROW * std::min<int>(7, (int)rows.size() + 1)), rows, g_sel, true, nullptr, false, &activated);
		if (picked != g_sel && picked >= 0 && picked < (int)P.size()) g_sel = picked;
	}
	if (g_sel >= 0 && g_sel < (int)P.size())
	{
		Prim p = P[g_sel];
		ui.Space();
		ui.Heading(NameOf(g_sel, p));
		p.name = ui.TextInput(ui.Row(), devui::MakeId("g03gray.name", g_sel), "name", p.name, "optional");
		p.kind = ui.Choice(ui.Row(), devui::MakeId("g03gray.kind", g_sel), "shape", {KIND_NAMES[0], KIND_NAMES[1], KIND_NAMES[2]}, p.kind);
		ui.Vec3(ui.Row(), devui::MakeId("g03gray.pos", g_sel), "position", p.pos, 0.25, "%.2f");
		ui.Vec3(ui.Row(), devui::MakeId("g03gray.rot", g_sel), "turn (deg X Y Z)", p.rot, 5.0, "%.1f");
		ui.Vec3(ui.Row(), devui::MakeId("g03gray.size", g_sel), "size", p.size, 0.25, "%.2f");
		p.solid = ui.Checkbox(ui.Row(), "Collision (Pac-Man rolls on it)", p.solid);
		for (int i = 0; i < 3; i++) p.size[i] = std::max(p.size[i], MIN_SIZE);
		if (!(p == P[g_sel]))
		{
			PushUndo(g_sel);
			const bool kindChanged = p.kind != P[g_sel].kind || p.solid != P[g_sel].solid;
			P[g_sel] = p;
			if (kindChanged) Sync(); else Touch(g_sel);
		}
		std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1, 1, 1});
		if (ui.Button(c[0], "Duplicate"))
		{
			Prim d = P[g_sel];
			d.name.clear();
			d.pos[0] += 1.0f;
			d.pos[2] += 1.0f;
			AddPrim(d);
		}
		if (ui.Button(c[1], "Reset turn"))
		{
			PushUndo();
			g_lastPushPrim = -1;
			P[g_sel].rot[0] = P[g_sel].rot[1] = P[g_sel].rot[2] = 0;
			Touch(g_sel);
		}
		if (ui.Button(c[2], "Delete")) DeletePrim(g_sel);
	}
	if (!g_msg.empty()) ui.Label(g_msg, &devui::Color(devui::HINT));
	ui.Label("Game view: 1 / 2 / 3 = move / rotate / scale, Q = local / world axes, drag a handle (Ctrl snaps, Shift on scale = all axes), drag the body to slide it (Alt = up / down), "
	         "Del / X delete, Ctrl+D duplicate, PgUp / PgDn up / down, Ctrl+Z / Y undo / redo.", &devui::Color(devui::DIM));
	ui.Label(std::string("file: ") + dir + "/graybox.json", &devui::Color(devui::DIM));
}
}
