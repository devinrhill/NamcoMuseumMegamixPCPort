// Developer-only (NOT 1:1, nothing here exists in main.dol): Pac-N-Roll dev tools. Hit overlays (--g03-show-hit / --g03-show-gimmick-hit) and the
// fly hack (--g03-fly, G03FlyStep), collision from the drawn models (--g03-bg-collision, at the end of the file).
// The collision faces are read from the objects cTaskCollision tests (cTaskBgHit::pFaceList, world = pMtx * local vertex), so the wireframe is
// exactly what Pac-Man collides with. The gimmicks' own damage / area tests (pillar box, arrow / iron ball spheres, wall area) are recorded
// where they run (one-line "devtools:" hooks) and drawn in the same frame.
#include "platform/devtools.hpp"
#include "platform/devtools_g01edit.hpp"
#include "platform/devtools_g07edit.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <vector>

#include "GamePad.hpp"
#include "gfx/Gfx3D.hpp"
#include "kingdom/Constants.hpp"
#include "InGamePadManager.hpp"
#include "PadDefine.hpp"
#include "kingdom/game/g03/cTaskCamera.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/game/g03/cTaskBg.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "kingdom/game/g03/cTaskBgGimmickWall.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCollision.hpp"
#include "kingdom/game/g03/cTaskStage.hpp"
#include "kingdom/game/g07/cParamBG.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Mdl0.hpp"
#include <Constants.hpp>

namespace devtools
{
namespace
{
struct Shape
{
	bool sphere;
	float a[3], b[3];   // box min / max, or sphere centre / (radius, -, -)
	unsigned rgba;
	int frame;
};
std::vector<Shape> g_shapes;
std::vector<gfx::LineVertex> g_lines;

const unsigned kWhite = 0xffffffffu;   // hurts Pac-Man

void Line(const float p[3], const float q[3], unsigned rgba)
{
	gfx::LineVertex v{};
	for (int k = 0; k < 4; k++) v.rgba[k] = (std::uint8_t)(rgba >> (24 - 8 * k));
	for (int k = 0; k < 3; k++) v.pos[k] = p[k];
	g_lines.push_back(v);
	for (int k = 0; k < 3; k++) v.pos[k] = q[k];
	g_lines.push_back(v);
}

void Box(const float mn[3], const float mx[3], unsigned rgba)
{
	float c[8][3];
	for (int i = 0; i < 8; i++)
	{
		c[i][0] = (i & 1) ? mx[0] : mn[0];
		c[i][1] = (i & 2) ? mx[1] : mn[1];
		c[i][2] = (i & 4) ? mx[2] : mn[2];
	}
	for (int i = 0; i < 8; i++)
		for (int bit = 1; bit < 8; bit <<= 1)
			if (!(i & bit)) Line(c[i], c[i | bit], rgba);
}

void Sphere(const float c[3], float r, unsigned rgba)
{
	const int seg = 24;
	for (int axis = 0; axis < 3; axis++)
		for (int i = 0; i < seg; i++)
		{
			const float a0 = 6.2831853f * i / seg, a1 = 6.2831853f * (i + 1) / seg;
			float p[3] = {c[0], c[1], c[2]}, q[3] = {c[0], c[1], c[2]};
			const int u = (axis + 1) % 3, w = (axis + 2) % 3;
			p[u] += r * std::cos(a0); p[w] += r * std::sin(a0);
			q[u] += r * std::cos(a1); q[w] += r * std::sin(a1);
			Line(p, q, rgba);
		}
}

using kingdom::game::g03::Constants;

// colour of a collision face (0xRRGGBBAA): docs/OPTIONS.md lists the same table
unsigned FaceColor(Constants::eFACE_KIND kind)
{
	switch (kind)
	{
	case Constants::eFACE_KIND_WALL:
	case Constants::eFACE_KIND_WAXX: return 0x9a9a9aff;   // wall: grey
	case Constants::eFACE_KIND_BOUN: return 0xffdc00ff;   // bounce wall: yellow
	case Constants::eFACE_KIND_NORM: return 0x3cc83cff;   // floor: green
	case Constants::eFACE_KIND_WATE:
	case Constants::eFACE_KIND_SWIM: return 0x3c78ffff;   // water: blue
	case Constants::eFACE_KIND_GRAS: return 0x1e781eff;   // grass: dark green
	case Constants::eFACE_KIND_DASH:
	case Constants::eFACE_KIND_DALL: return 0x00dcffff;   // dash pad: cyan
	case Constants::eFACE_KIND_JUMP: return 0xff8c00ff;   // jump pad: orange
	case Constants::eFACE_KIND_MAGM: return 0xff2800ff;   // magma: red
	case Constants::eFACE_KIND_POIS: return 0xaa00ffff;   // poison: purple
	case Constants::eFACE_KIND_CAME: return 0x783c50ff;   // camera only: dark pink
	default: return 0xc8aa3cff;                           // hub doors / unknown: gold
	}
}

void Faces(kingdom::game::g03::cTaskBgHit* obj, bool damagingWalls)
{
	kingdom::game::g03::cFaceList* list = obj->pFaceList();
	if (list == nullptr) return;
	for (int n = 0; n < 4; n++)
		for (uint i = 0; i < list->nFaceNum(n); i++)
		{
			Constants::sFace* f = list->pFace(n, (int)i);
			unsigned rgba = FaceColor(f->nKind);
			if (damagingWalls && n == 0)
			{
				// cTaskBgGimmickWall::HitCallbackWall: a wall face hurts when its rotated normal points down
				Vector4 nrm;
				::Constants::jetGeoVecTrans(nrm, obj->pMtxRotate(), f->vNorm);
				if (nrm[1] < 0.0f) rgba = kWhite;
			}
			float w[3][3];
			for (int k = 0; k < 3; k++)
			{
				Vector4 p;
				::Constants::jetGeoVecTrans(p, obj->pMtx(), f->vVrt[k]);
				w[k][0] = p[0]; w[k][1] = p[1]; w[k][2] = p[2];
			}
			Line(w[0], w[1], rgba);
			Line(w[1], w[2], rgba);
			Line(w[2], w[0], rgba);
		}
}
}

void G03HitBox(const float mn[3], const float mx[3], unsigned rgba)
{
	if (!G03ShowGimmickHit()) return;
	Shape s{false, {mn[0], mn[1], mn[2]}, {mx[0], mx[1], mx[2]}, rgba, FrameNo()};
	g_shapes.push_back(s);
}

void G03HitSphere(const float c[3], float r, unsigned rgba)
{
	if (!G03ShowGimmickHit()) return;
	Shape s{true, {c[0], c[1], c[2]}, {r, 0, 0}, rgba, FrameNo()};
	g_shapes.push_back(s);
}

void DrawOverlay3D()
{
	G03EditDraw3D();
	G07EditDraw3D();
	G01EditDraw3D();
	const bool hit = G03ShowHit(), gim = G03ShowGimmickHit();
	if (!hit && !gim) return;
	g_lines.clear();
	// the faces are drawn with the depth test (a whole stage over itself is unreadable), the gimmick test shapes over everything (they sit
	// inside their models)
	kingdom::game::g03::cAppG03PacnRollGame* app = Constants::PACN_APP();
	if (app != nullptr && app->pTaskCollision() != nullptr)
	{
		for (kingdom::game::g03::cTaskBgHit* obj : app->pTaskCollision()->pVecObj())
		{
			const bool isGimmick = dynamic_cast<kingdom::game::g03::cTaskBgGimmick*>(obj) != nullptr;
			if (isGimmick ? gim : hit) Faces(obj, dynamic_cast<kingdom::game::g03::cTaskBgGimmickWall*>(obj) != nullptr);
		}
		gfx::DrawLines3D(g_lines.data(), g_lines.size(), true);
	}
	// the shapes of the newest frame that recorded any (the gimmicks' Execute may run before or after the scene pass), if it is this frame or
	// the last one; then all are dropped (a paused game records none, so they vanish while paused)
	g_lines.clear();
	int newest = -1;
	for (const Shape& s : g_shapes) if (s.frame > newest) newest = s.frame;
	if (newest >= FrameNo() - 1)
		for (const Shape& s : g_shapes)
		{
			if (s.frame != newest) continue;
			if (s.sphere) Sphere(s.a, s.b[0], s.rgba);
			else Box(s.a, s.b, s.rgba);
		}
	g_shapes.clear();
	if (app != nullptr) gfx::DrawLines3D(g_lines.data(), g_lines.size(), false);
}

// One play frame of Pac-Man in fly mode, called instead of his Control / gravity / collision push (cTaskCharPacman::Execute play mode). The stick is
// read the way Control reads it (camera-relative: up = away from the camera), but it sets the speed directly; C / Z = up / down, B = 3x speed, A = 2x (both = 6x).
// No collision runs (the BG push needs state 64, which this frame does not set), so the move is added to vTrans here.
void G03FlyStep(kingdom::game::g03::cTaskCharPacman* pac)
{
	kingdom::game::g03::cAppG03PacnRollGame* app = Constants::PACN_APP();
	const int pad = InGamePadManager::GetPad1No();
	float speed = InGamePadManager::GetKey(pad, PadDefine::eKEY_CROSS) ? 0.75f : 0.25f;
	if (InGamePadManager::GetKey(pad, PadDefine::eKEY_CIRCLE)) speed *= 2.0f;   // A held: double (stacks with B)
	Vector2 stick = InGamePadManager::GetStick(pad, PadDefine::scLSTICK);
	if (std::fabs(stick.x) <= 0.08f) stick.x = 0.0f;
	if (std::fabs(stick.y) <= 0.08f) stick.y = 0.0f;
	Vector4 move = Vector4::zero;
	::Constants::jetGeoVecUnit(move);
	Vector4 back, right, up;
	::Constants::jetGeoVecSub(back, app->pTaskCamera()->vPos(), app->pTaskCamera()->vTarget());
	back[1] = 0.0f;
	back[3] = 1.0f;
	::Constants::jetGeoVecSet(up, 0.0f, 1.0f, 0.0f, 1.0f);
	if (back[0] * back[0] + back[2] * back[2] > 1e-8f)
	{
		::Constants::jetGeoVecNormalize(back, back);
		::Constants::jetGeoVecOuterProduct(right, back, up);
		::Constants::jetGeoVecNormalize(right, right);
		for (int k = 0; k < 3; k++) move[k] = (-right[k] * stick.x - back[k] * stick.y) * speed;
	}
	if (InGamePadManager::GetKey(pad, PadDefine::eKEY_R1)) move[1] += speed;   // C
	if (InGamePadManager::GetKey(pad, PadDefine::eKEY_L1)) move[1] -= speed;   // Z
	for (int k = 0; k < 3; k++) pac->vMove()[k] = move[k];
	::Constants::jetGeoVecAdd(pac->vTrans(), pac->vTrans(), pac->vMove());
}

// --g03-fly: Wiimote 1 (eKEY_SQUARE, unused by Pac-N-Roll) switches flying on / off; read once per frame however often G03Fly is asked
bool G03Fly()
{
	static bool flying = true;
	static int polled = -1;
	if (!G03FlyEnabled()) return false;
	if (polled != FrameNo())
	{
		polled = FrameNo();
		if (InGamePadManager::GetKeyDown(InGamePadManager::GetPad1No(), PadDefine::eKEY_SQUARE))
		{
			flying = !flying;
			std::fprintf(stderr, "devtools: --g03-fly: %s\n", flying ? "flying" : "rolling");
		}
	}
	return flying;
}

// --g03-freecam: right stick Y (up = look higher) or D-pad up / down moves the look-at height, scaled by the camera distance, read once per frame
float G03FreecamTargetY(float distance)
{
	static float offset = 0.0f;
	static int polled = -1;
	if (!G03Freecam()) return 0.0f;
	if (polled != FrameNo())
	{
		polled = FrameNo();
		const int pad = InGamePadManager::GetPad1No();
		float y = InGamePadManager::GetStick(pad, PadDefine::scRSTICK, false).y;
		if (std::fabs(y) < 0.15f) y = 0.0f;
		const unsigned hold = GamePad::Get(pad).hold;
		if (hold & GamePad::PAD_UP) y = 1.0f;
		else if (hold & GamePad::PAD_DOWN) y = -1.0f;
		if (!kingdom::Constants::IS_PAUSE()) offset += y * 0.008f * distance;
		offset = std::clamp(offset, -1.0f * distance, 1.0f * distance);
	}
	return offset;
}

// --g03-jump: C pressed on a floor -> the strength times fly mode's multipliers (B = 3x, A = 2x, both = 6x)
float G03JumpTrigger(bool onFloor)
{
	if (!G03Jump() || !onFloor) return 0.0f;
	const int pad = InGamePadManager::GetPad1No();
	if (!InGamePadManager::GetKeyDown(pad, PadDefine::eKEY_R1)) return 0.0f;   // C
	float s = G03JumpStrengthValue();
	if (InGamePadManager::GetKey(pad, PadDefine::eKEY_CROSS)) s *= 3.0f;    // B
	if (InGamePadManager::GetKey(pad, PadDefine::eKEY_CIRCLE)) s *= 2.0f;   // A
	return s;
}

// ---- --g03-bg-collision: collision faces from the drawn models ----------------------------------------------------------------------------------
// A face keeps the triangle it came from (mesh + vertex indices), so an animated model's faces are re-skinned in place every frame (the lists keep
// their size; a face keeps the kind it got when it was built). Vertices are skinned the way gfx::ModelDraw's CPU path does it (bind pose = the
// raw positions, which are model space). A gimmick whose CHR clip animates "root" already moves its collision with that bone (cTaskBgHit::ApplyAnmRoot
// folds it into pMtx), so its faces are taken relative to the root: local = (S * rt(root))^-1 * skin * v.
namespace
{
using kingdom::game::g03::cFaceList;
using kingdom::game::g03::cTaskBgHit;
using kingdom::game::g03::Constants;

struct FaceSrc { const wii::Mdl0Mesh* mesh; int drawNode; std::uint32_t idx[3]; };
struct BgFaces
{
	kingdom::cKingdomModel* model;
	std::vector<Constants::sFace*> faces;   // in build order, parallel to src
	std::vector<FaceSrc> src;
	bool posed;                             // the faces were last built from a CHR pose (rebuild once more when it stops)
	float bake[3];                          // a BG collider's placement scale, applied to the vertices (the collider itself has scale 1)
};
std::unordered_map<cFaceList*, BgFaces> g_bgFaces;

// a collision object for one BG model: placed like the model (cTaskBgHit::UpdateMatrix reads its own Model's vTrans / vRotate / vScale, copied
// from the BG, except the scale: cTaskCollision moves Pac-Man / his radius into the object with pMtxRotateInv (rotation only) but his position with
// the full inverse, so a scaled collision object pushes him by the wrong amounts (jitter). The scale is baked into the faces instead.
class cTaskBgCollider : public cTaskBgHit
{
public:
	bool Execute(cJetTask* pParam) override
	{
		UpdateMatrix();
		return cTaskBgHit::Execute(pParam);
	}
};

void SkinPos(const wii::Mdl0Vertex& v, int drawNode, const std::vector<float>& skin, float out[3])
{
	auto xf = [](const float* m, const float* p, float* o) {
		for (int r = 0; r < 3; r++) o[r] = m[r * 4 + 0] * p[0] + m[r * 4 + 1] * p[1] + m[r * 4 + 2] * p[2] + m[r * 4 + 3];
	};
	auto valid = [&](int n) { return n >= 0 && (size_t)(n + 1) * 12 <= skin.size(); };
	for (int c = 0; c < 3; c++) out[c] = v.pos[c];
	if (skin.empty()) return;
	float sp[3] = {0, 0, 0};
	int used = 0;
	for (int k = 0; k < v.skinCount; k++)
	{
		if (!valid(v.skinNode[k])) continue;
		float tp[3];
		xf(&skin[v.skinNode[k] * 12], v.pos, tp);
		for (int c = 0; c < 3; c++) sp[c] += v.skinWeight[k] * tp[c];
		used++;
	}
	if (used == 0 && valid(drawNode)) { xf(&skin[drawNode * 12], v.pos, sp); used = 1; }
	if (used) for (int c = 0; c < 3; c++) out[c] = sp[c];
}

// (S * rt(root))^-1 of the CHR slot that moves obj's collision (cTaskBgHit::ApplyAnmRoot); false when no slot does (or no obj: a BG collider)
bool RootInv(cTaskBgHit* obj, Matrix4x4& inv)
{
	if (obj == nullptr) return false;
	const int s = obj->AnmRootSlot();
	if (s < 0) return false;
	std::shared_ptr<wii::ChrAnim> anm = obj->Model->AnmChrOf((uint)s);
	wii::BoneKey root;
	if (!anm->Sample("root", wii::ClipFrame(obj->Model->GetFrameAnmChr((uint)s), anm->numFrame, anm->loops), root)) return false;
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxScale(m, m, Vector4(root.scale[0], root.scale[1], root.scale[2], 1.0f));
	::Constants::jetGeoMtxMult(m, m, kingdom::cKingdomModelBase::BoneRt(root, true));
	::Constants::jetGeoMtxInverse(inv, m);
	return true;
}

// sets the face's vertices / normal / box; false for a degenerate triangle
bool FillFace(Constants::sFace* f, const FaceSrc& src, const std::vector<float>& skin, const Matrix4x4* rootInv, const float bake[3])
{
	for (int l = 0; l < 3; l++)
	{
		float p[3];
		SkinPos(src.mesh->verts[src.idx[l]], src.drawNode, skin, p);
		::Constants::jetGeoVecSet(f->vVrt[l], p[0], p[1], p[2], 1.0f);
		if (rootInv != nullptr) ::Constants::jetGeoVecTrans(f->vVrt[l], *rootInv, f->vVrt[l]);
		for (int c = 0; c < 3; c++) f->vVrt[l][c] *= bake[c];
	}
	// the normal as cFaceList::RegistFace makes it
	Vector4 e0, e1, n;
	::Constants::jetGeoVecSub(e0, f->vVrt[1], f->vVrt[0]);
	::Constants::jetGeoVecSub(e1, f->vVrt[2], f->vVrt[0]);
	::Constants::jetGeoVecOuterProduct(n, e1, e0);
	if (n[0] * n[0] + n[1] * n[1] + n[2] * n[2] < 1e-12f) return false;
	::Constants::jetGeoVecNormalize(f->vNorm, n);
	const Vector4* v = f->vVrt;
	f->aabb = AABB(Vector3(Mathf::Min(v[0].x, v[1].x, v[2].x), Mathf::Min(v[0].y, v[1].y, v[2].y), Mathf::Min(v[0].z, v[1].z, v[2].z)),
		Vector3(Mathf::Max(v[0].x, v[1].x, v[2].x), Mathf::Max(v[0].y, v[1].y, v[2].y), Mathf::Max(v[0].z, v[1].z, v[2].z)), false);
	f->calcedAABB = AABB();
	return true;
}

AABB FacesAABB(const std::vector<Constants::sFace*>& faces)
{
	const float big = std::numeric_limits<float>::max();
	Vector3 lo(big, big, big), hi(-big, -big, -big);
	for (const Constants::sFace* f : faces)
		for (int l = 0; l < 3; l++)
		{
			lo = Vector3(std::min(lo.x, f->vVrt[l].x), std::min(lo.y, f->vVrt[l].y), std::min(lo.z, f->vVrt[l].z));
			hi = Vector3(std::max(hi.x, f->vVrt[l].x), std::max(hi.y, f->vVrt[l].y), std::max(hi.z, f->vVrt[l].z));
		}
	Bounds b;
	if (!faces.empty()) b.SetMinMax(lo, hi);
	return AABB(b);
}

// the faces of `obj`'s collision from `model` (the visual model); `hitMdl` stays the list's ResMdl. nullptr when the model has no triangles to use
cFaceList* BuildFaces(cTaskBgHit* obj, kingdom::cKingdomModel* model, ResMdl* hitMdl, const float bake[3])
{
	const wii::Mdl0* mdl = model != nullptr ? model->pMdl0() : nullptr;
	if (mdl == nullptr) return nullptr;
	std::vector<float> skin;
	model->PosedSkin(skin);
	Matrix4x4 rootInv;
	const bool hasRoot = RootInv(obj, rootInv);
	// floor or wall by the normal in world orientation (the model's placement rotation; up to 60 degrees from up = floor)
	float w[16];
	model->CalcWorld(w);
	BgFaces bf{model, {}, {}, !skin.empty(), {bake[0], bake[1], bake[2]}};
	std::vector<Constants::sFace*> lists[4];
	for (const wii::Mdl0Draw& d : mdl->draws)
	{
		if (d.stream == "DrawXlu") continue;
		const wii::Mdl0Mesh* mesh = mdl->MeshByShapeId(d.shape);
		const wii::Mdl0Material* mat = mdl->MaterialById(d.material);
		const wii::Mdl0Node* node = mdl->NodeById(d.node);
		if (mesh == nullptr || mat == nullptr || mat->cullMode == 3 || (node != nullptr && node->billboard != 0)) continue;
		// GX culls front (1) / back (2) / none (0): the side that is drawn is the solid one, both for none
		const bool keep = mat->cullMode != 1, flip = mat->cullMode != 2;
		for (size_t t = 0; t + 2 < mesh->tris.size(); t += 3)
			for (int side = 0; side < 2; side++)
			{
				if (side == 0 ? !keep : !flip) continue;
				FaceSrc src{mesh, d.node, {mesh->tris[t], mesh->tris[t + (side ? 2 : 1)], mesh->tris[t + (side ? 1 : 2)]}};
				Constants::sFace* f = new Constants::sFace();
				if (!FillFace(f, src, skin, hasRoot ? &rootInv : nullptr, bake)) { delete f; continue; }
				const Vector4& n = f->vNorm;
				const float wx = w[0] * n[0] + w[4] * n[1] + w[8] * n[2], wy = w[1] * n[0] + w[5] * n[1] + w[9] * n[2], wz = w[2] * n[0] + w[6] * n[1] + w[10] * n[2];
				const float len = std::sqrt(wx * wx + wy * wy + wz * wz);
				const bool floor = len > 0.0f && wy / len >= 0.5f;
				f->nKind = floor ? Constants::eFACE_KIND_NORM : Constants::eFACE_KIND_WALL;
				lists[floor ? 1 : 0].push_back(f);
				bf.faces.push_back(f);
				bf.src.push_back(src);
			}
	}
	if (bf.faces.empty()) return nullptr;
	cFaceList* list = new cFaceList(hitMdl, lists, FacesAABB(bf.faces));
	g_bgFaces.emplace(list, std::move(bf));
	return list;
}
}

void G03BgCollisionStage(kingdom::game::g03::cTaskStage* stage, const std::vector<kingdom::game::g03::cTaskBg*>& bgs, std::vector<cTaskBgHit*>& out)
{
	size_t faces = 0;
	for (kingdom::game::g03::cTaskBg* bg : bgs)
	{
		const Vector4& sc = bg->Model->vScale();
		const float bake[3] = {sc[0], sc[1], sc[2]};
		cFaceList* list = BuildFaces(nullptr, bg->Model, nullptr, bake);
		if (list == nullptr) continue;
		cTaskBgCollider* obj = new cTaskBgCollider();
		::Constants::jetGeoVecCopy(obj->Model->vTrans(), bg->Model->vTrans());
		::Constants::jetGeoVecCopy(obj->Model->vRotate(), bg->Model->vRotate());
		::Constants::jetGeoVecSet(obj->Model->vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
		obj->pFaceList() = list;
		obj->IsMove() = bg->Model->HasChrPose();   // the boxes of re-skinned faces are recomputed every frame
		obj->nDrawPriority(65536u);
		obj->UpdateMatrix();
		stage->ConnectChild(obj);
		out.push_back(obj);
		faces += g_bgFaces[obj->pFaceList()].faces.size();
	}
	std::fprintf(stderr, "devtools: --g03-bg-collision: %zu BG models -> %zu collision objects, %zu faces\n", bgs.size(), out.size(), faces);
}

void G03BgCollisionReplace(cTaskBgHit* obj)
{
	if (obj->pFaceList() == nullptr) return;   // no hit model: not solid in the game either
	static const float one[3] = {1.0f, 1.0f, 1.0f};
	if (cFaceList* list = BuildFaces(obj, obj->Model, obj->pFaceList()->pResMdl(), one)) obj->pFaceList() = list;
}

void G03BgCollisionRefresh(cTaskBgHit* obj)
{
	if (obj->pFaceList() == nullptr) return;
	auto it = g_bgFaces.find(obj->pFaceList());
	if (it == g_bgFaces.end()) return;
	BgFaces& bf = it->second;
	const bool posed = bf.model->HasChrPose();
	if (!posed && !bf.posed) return;
	std::vector<float> skin;
	if (posed) bf.model->PosedSkin(skin);
	bf.posed = posed;
	Matrix4x4 rootInv;
	const bool hasRoot = RootInv(obj, rootInv);
	for (size_t i = 0; i < bf.faces.size(); i++)
		if (!FillFace(bf.faces[i], bf.src[i], skin, hasRoot ? &rootInv : nullptr, bf.bake))
		{
			// squashed flat this frame: a point (never hit) until it opens again
			for (int l = 1; l < 3; l++) bf.faces[i]->vVrt[l] = bf.faces[i]->vVrt[0];
		}
	obj->pFaceList()->ReadOnlyAABB() = FacesAABB(bf.faces);
	obj->pFaceList()->CalcedAABB() = AABB();
}

// ---- --g03-explore: Pac-Man in arbitrary models -------------------------------------------------------------------------------------------------
namespace
{
std::string g_exploreArc;
std::vector<std::string> g_exploreModels, g_exploreAnims;
float g_exploreScale = 1.0f;
float g_exploreStart[3]{};
float g_exploreBottom = -1e30f;
bool g_exploreHasStart = false;

std::vector<std::string> SplitList(const std::string& s)
{
	std::vector<std::string> out;
	size_t p = 0;
	while (p <= s.size())
	{
		const size_t q = s.find(',', p);
		const std::string t = s.substr(p, q == std::string::npos ? std::string::npos : q - p);
		if (!t.empty()) out.push_back(t);
		if (q == std::string::npos) break;
		p = q + 1;
	}
	return out;
}

// the attached entry name for `e` (as given, or with ".brres"); empty when neither is there
std::string FindEntry(const std::string& e)
{
	std::size_t size = 0;
	if (wii::ArchiveStore::Find(e, size) != nullptr) return e;
	if (wii::ArchiveStore::Find(e + ".brres", size) != nullptr) return e + ".brres";
	return std::string();
}
}

bool G03Explore() { return !g_exploreArc.empty(); }
void SetG03Explore(const std::string& archive, const std::string& entries) { g_exploreArc = archive; g_exploreModels = SplitList(entries); }
void SetG03ExploreAnims(const std::string& entries) { g_exploreAnims = SplitList(entries); }
void SetG03ExploreScale(float s) { if (s > 0.0f) g_exploreScale = s; }

void G03ExploreAttach()
{
	if (!G03Explore()) return;
	if (!wii::ArchiveStore::Attach(g_exploreArc)) std::fprintf(stderr, "devtools: --g03-explore: cannot attach %s\n", g_exploreArc.c_str());
}

bool G03ExploreHidesData(const std::string& key)
{
	if (!G03Explore() || key == "BIN_START_SET_DAT") return false;   // the start entry stays: --g03-start / the explore start replace its position
	const auto ends = [&](const char* t) { const size_t n = std::strlen(t); return key.size() >= n && key.compare(key.size() - n, n, t) == 0; };
	return ends("_SET_DAT") || ends(".cam") || key.rfind("BIN_GHOST_", 0) == 0 || key.rfind("BIN_BOSS_COOKIE_", 0) == 0;
}

void G03ExploreStage(kingdom::game::g03::cTaskStage* stage, std::vector<kingdom::game::g03::cTaskBg*>& bgs, std::vector<cTaskBgHit*>& hits)
{
	std::vector<kingdom::game::g03::cTaskBg*> added;
	// a Pac-Motos stage archive places its MDL_BG_1..4 with CSV_BG (g07 cTaskBG::Init): position, rotation, scale, animation rate per model
	kingdom::game::g07::cParamBG* motos = !FindEntry("CSV_BG").empty() ? new kingdom::game::g07::cParamBG("CSV_BG") : nullptr;
	for (const std::string& spec : g_exploreModels)
	{
		// MODEL[+ANIM+ANIM...]: animations for this model only
		std::vector<std::string> parts;
		for (size_t p = 0, q; p <= spec.size(); p = q + 1)
		{
			q = spec.find('+', p);
			if (q == std::string::npos) q = spec.size();
			if (q > p) parts.push_back(spec.substr(p, q - p));
		}
		if (parts.empty()) continue;
		const std::string& m = parts[0];
		const std::string key = FindEntry(m);
		if (key.empty()) { std::fprintf(stderr, "devtools: --g03-explore: no entry %s in the attached archives\n", m.c_str()); continue; }
		kingdom::game::g03::cTaskBg* bg = new kingdom::game::g03::cTaskBg();
		bg->Model->RegistModel(key);
		if (bg->Model->pMdl0() == nullptr) { std::fprintf(stderr, "devtools: --g03-explore: %s holds no model\n", m.c_str()); continue; }
		std::vector<std::string> anims(parts.begin() + 1, parts.end());
		anims.insert(anims.end(), g_exploreAnims.begin(), g_exploreAnims.end());
		for (const std::string& a : anims)
		{
			const std::string ak = FindEntry(a);
			if (!ak.empty()) bg->Model->RegistAnm(0u, ak);
			else std::fprintf(stderr, "devtools: --g03-explore: no animation entry %s\n", a.c_str());
		}
		float rate = 1.0f;
		if (motos != nullptr && m.size() == 8 && m.compare(0, 7, "MDL_BG_") == 0 && m[7] >= '1' && m[7] <= '4')
		{
			const uint n = (uint)(m[7] - '1');
			bg->Model->vTrans() = motos->vGetTrans(n);
			bg->Model->vRotate() = motos->vGetRotate(n);
			bg->Model->vScale() = motos->vGetScale(n);
			rate = motos->fGetFrameRate(n);
		}
		for (int c = 0; c < 3; c++) { bg->Model->vTrans()[c] *= g_exploreScale; bg->Model->vScale()[c] *= g_exploreScale; }
		bg->Model->vTrans()[3] = bg->Model->vScale()[3] = 1.0f;
		bg->Model->SetUpdateRateAll(0u, rate);
		bg->Model->SetLightSetIdx(0);
		bg->Model->SetFogIdx(0);
		bg->Initialize();
		bg->nDrawPriority(65536u);
		bg->Model->AttachShadow();
		stage->ConnectChild(bg);
		bgs.push_back(bg);
		added.push_back(bg);
	}
	const size_t first = hits.size();
	G03BgCollisionStage(stage, added, hits);
	const float big = std::numeric_limits<float>::max();
	float lo[3] = {big, big, big}, hi[3] = {-big, -big, -big};
	struct W { float c[3]; float p[3][3]; };
	std::vector<W> floors;
	for (size_t h = first; h < hits.size(); h++)
	{
		cTaskBgHit* obj = hits[h];
		for (int n = 0; n < 2; n++)
			for (uint i = 0; i < obj->pFaceList()->nFaceNum(n); i++)
			{
				Constants::sFace* f = obj->pFaceList()->pFace(n, (int)i);
				W w{};
				for (int k = 0; k < 3; k++)
				{
					Vector4 p;
					::Constants::jetGeoVecTrans(p, obj->pMtx(), f->vVrt[k]);
					for (int c = 0; c < 3; c++) { lo[c] = std::min(lo[c], p[c]); hi[c] = std::max(hi[c], p[c]); w.c[c] += p[c] / 3.0f; w.p[k][c] = p[c]; }
				}
				if (n == 1) floors.push_back(w);
			}
	}
	if (floors.empty()) { std::fprintf(stderr, "devtools: --g03-explore: no floor faces\n"); return; }
	// dropped from above the middle of the models: the highest floor under the centre (X / Z), else the floor centre nearest to it
	const float cx = (lo[0] + hi[0]) * 0.5f, cz = (lo[2] + hi[2]) * 0.5f;
	float top[3] = {0, -big, 0};
	for (const W& w : floors)
	{
		const float (*q)[3] = w.p;
		const float d = (q[1][0] - q[0][0]) * (q[2][2] - q[0][2]) - (q[2][0] - q[0][0]) * (q[1][2] - q[0][2]);
		if (std::fabs(d) < 1e-9f) continue;
		const float u = ((cx - q[0][0]) * (q[2][2] - q[0][2]) - (q[2][0] - q[0][0]) * (cz - q[0][2])) / d;
		const float v = ((q[1][0] - q[0][0]) * (cz - q[0][2]) - (cx - q[0][0]) * (q[1][2] - q[0][2])) / d;
		if (u < 0.0f || v < 0.0f || u + v > 1.0f) continue;
		const float y = q[0][1] + u * (q[1][1] - q[0][1]) + v * (q[2][1] - q[0][1]);
		if (y > top[1]) { top[0] = cx; top[1] = y; top[2] = cz; }
	}
	if (top[1] == -big)
	{
		float bestD = big;
		for (const W& w : floors)
		{
			const float d = (w.c[0] - cx) * (w.c[0] - cx) + (w.c[2] - cz) * (w.c[2] - cz);
			if (d < bestD) { bestD = d; for (int c = 0; c < 3; c++) top[c] = w.c[c]; }
		}
	}
	float user[3];
	if (G03StartOverride(user)) { for (int c = 0; c < 3; c++) g_exploreStart[c] = user[c]; }
	else
	{
		for (int c = 0; c < 3; c++) g_exploreStart[c] = top[c];
		g_exploreStart[1] += 2.0f;
		SetG03Start(g_exploreStart[0], g_exploreStart[1], g_exploreStart[2]);
	}
	g_exploreHasStart = true;
	g_exploreBottom = lo[1];
	std::fprintf(stderr, "devtools: --g03-explore: bounds (%g %g %g) .. (%g %g %g), start (%g %g %g)\n", lo[0], lo[1], lo[2], hi[0], hi[1], hi[2],
	             g_exploreStart[0], g_exploreStart[1], g_exploreStart[2]);
}

bool G03ExploreFell(kingdom::game::g03::cTaskCharPacman* pac)
{
	if (!G03Explore() || !g_exploreHasStart || pac->vTrans()[1] >= g_exploreBottom - 30.0f) return false;
	pac->vTrans() = Vector4(g_exploreStart[0], g_exploreStart[1], g_exploreStart[2], 1.0f);
	::Constants::jetGeoVecZero(pac->vMove());
	return true;
}
}
