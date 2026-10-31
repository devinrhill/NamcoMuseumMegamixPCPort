#include "kingdom/cKingdomModelBase.hpp"
#include "Constants.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>

#include "CustomKingdomUtil.hpp"
#include "gfx/ModelDraw.hpp"
#include "kingdom/NativeModel.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/ResHandle.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

// Native backend of the Wii model wrapper (kingdom::cKingdomModelBase / cKingdomModel). The Unity port kept GameObjects + Animation clips; here a model is
// a shared gfx::ModelDraw (one per archive entry) plus per-instance animation slots (CHR0 bone clips blended by weight, one SRT0 and one PAT0 texture
// animation), a transform (vTrans / vRotate / vScale) and a light set. Frames advance by `rate` per UpdateFrame (one game frame; UNVERIFIED clip rate,
// as in the title scene). The original Unity version of this file is kept in old/BACKUP_unity_headers/.
namespace kingdom
{

namespace
{
// asset caches, keyed by archive entry; the archives change per stage, so they are dropped with ClearAssetCache()
std::map<std::string, std::shared_ptr<gfx::ModelDraw>> g_models;
std::map<std::string, std::shared_ptr<wii::ChrAnim>> g_chr;
std::map<std::string, std::shared_ptr<wii::SrtAnim>> g_srt;
std::map<std::string, std::shared_ptr<wii::PatAnim>> g_pat;
std::map<std::string, std::shared_ptr<wii::VisAnim>> g_vis;
std::map<std::string, std::shared_ptr<wii::ClrAnim>> g_clr;

bool LoadBrresEntry(const std::string& entry, wii::Brres& b)
{
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(entry, size);
	return d != nullptr && b.Load(d, size);
}
}

void cKingdomModelBase::ClearAssetCache()
{
	g_models.clear(); g_chr.clear(); g_srt.clear(); g_pat.clear(); g_vis.clear(); g_clr.clear();
}

std::shared_ptr<gfx::ModelDraw> LoadNativeModel(const std::string& entry)
{
	auto it = g_models.find(entry);
	if (it != g_models.end()) return it->second;
	wii::Brres b;
	auto md = std::make_shared<gfx::ModelDraw>();
	if (!LoadBrresEntry(entry, b) || b.ModelNames().empty() || !md->Load(b, b.ModelNames().front())) md.reset();
	g_models[entry] = md;
	return md;
}

string cKingdomModelBase::AnimationDic(int index)
{
	if (m_dicAnimationList.contains(index)) return m_dicAnimationList[index].c_str();
	return string();
}

cKingdomModelBase::cKingdomModelBase(uint lodSize)
{
	Init();
	m_nLod = lodSize;
	m_pLodObjs.assign(m_nLod, {});
	for (int i = 0; (long)i < (long)m_nLod; i++) m_pLodObjs[i] = new cKingdomModelBase::sLodObj();
}

cKingdomModelBase::~cKingdomModelBase()
{
	Term();
}

bool cKingdomModelBase::IsDrawShadowDepth() { return m_bDrawDepth; }
void cKingdomModelBase::IsDrawShadowDepth(bool b) { m_bDrawDepth = b; }
Vector4& cKingdomModelBase::vTrans() { return m_vTrans; }
Vector4& cKingdomModelBase::vRotate() { return m_vRotate; }
Vector4& cKingdomModelBase::vScale() { return m_vScale; }
float cKingdomModelBase::vTrans(uint n) { return m_vTrans[(int)n]; }
float cKingdomModelBase::vRotate(uint n) { return m_vRotate[(int)n]; }
float cKingdomModelBase::vScale(uint n) { return m_vScale[(int)n]; }
uint cKingdomModelBase::nLod() { return m_nLod; }

cKingdomModelBase::sLodObj* cKingdomModelBase::pLodObj(uint n)
{
	CustomKingdomUtil::Assert(n < m_nLod, "error");
	return m_pLodObjs[n];
}

bool cKingdomModelBase::IsVisible() { return m_bVisible; }
bool cKingdomModelBase::IsUpdateExpand() { return m_bUpdateExpand; }

void cKingdomModelBase::Init()
{
	m_vTrans = Vector4::zero;
	m_vRotate = Vector4::zero;
	m_vScale = Vector4::one;
	m_nLod = 0u;
	m_nLightSetIdx = 0;
	m_nFogIdx = -1;
	m_bVisible = true;
	m_bUpdateExpand = false;
	m_bDrawDepth = false;
	m_native = nullptr;
	for (int i = 0; i < 16; i++) m_world[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

void cKingdomModelBase::Term()
{
	NativeScene::Remove(this);
	delete m_native;
	m_native = nullptr;
}

void cKingdomModelBase::Visible() { m_bVisible = true; }
void cKingdomModelBase::Invisible() { m_bVisible = false; }

Object* cKingdomModelBase::SearchData(string key)
{
	return ResourceDatabase::Get<Object>(key);
}

void cKingdomModelBase::RegistAnm(uint blend_idx, string key, string node)
{
	RegistAnm(blend_idx, SearchData(key), node);
}

void cKingdomModelBase::RegistAnm(uint blend_idx, Object* data, string node)
{
	_RegistAnm((int)blend_idx, data, node);
}

int cKingdomModelBase::RegistAnmNoIdx(string key, string node)
{
	return RegistAnmNoIdx(SearchData(key), node);
}

int cKingdomModelBase::RegistAnmNoIdx(Object* data, string node)
{
	int idx = 0;
	if (m_native != nullptr)
		while (idx < BLEND_MAX - 1 && (m_native->chr[idx].anim != nullptr || m_native->srt[idx].anim != nullptr || m_native->pat[idx].anim != nullptr)) idx++;
	return _RegistAnm(idx, data, node);
}

// the animation file (CHR0 / SRT0 / PAT0 folder of a BRRES) is read once; a file can hold several kinds
int cKingdomModelBase::_RegistAnm(int blend_idx, Object* data, string node)
{
	if (data == nullptr || m_native == nullptr || blend_idx < 0 || blend_idx >= BLEND_MAX) return blend_idx;
	const ResHandle* h = dynamic_cast<const ResHandle*>(data);
	if (h == nullptr || h->entry.empty()) return blend_idx;
	wii::Brres b;
	if (!LoadBrresEntry(h->entry, b)) return blend_idx;
	auto c = g_chr.find(h->entry);
	if (c == g_chr.end())
	{
		auto a = std::make_shared<wii::ChrAnim>();
		if (!a->Load(b)) a.reset();
		c = g_chr.emplace(h->entry, a).first;
	}
	auto s = g_srt.find(h->entry);
	if (s == g_srt.end())
	{
		auto a = std::make_shared<wii::SrtAnim>();
		if (!a->Load(b)) a.reset();
		s = g_srt.emplace(h->entry, a).first;
	}
	auto p = g_pat.find(h->entry);
	if (p == g_pat.end())
	{
		auto a = std::make_shared<wii::PatAnim>();
		if (!a->Load(b)) a.reset();
		p = g_pat.emplace(h->entry, a).first;
	}
	auto v = g_vis.find(h->entry);
	if (v == g_vis.end())
	{
		auto a = std::make_shared<wii::VisAnim>();
		if (!a->Load(b)) a.reset();
		v = g_vis.emplace(h->entry, a).first;
	}
	auto cl = g_clr.find(h->entry);
	if (cl == g_clr.end())
	{
		auto a = std::make_shared<wii::ClrAnim>();
		if (!a->Load(b)) a.reset();
		cl = g_clr.emplace(h->entry, a).first;
	}
	if (cl->second) { m_native->clr[blend_idx].anim = cl->second; m_native->clr[blend_idx].frame = 0; }
	if (v->second) { m_native->vis[blend_idx].anim = v->second; m_native->vis[blend_idx].frame = 0; }
	if (c->second) { m_native->chr[blend_idx].anim = c->second; m_native->chr[blend_idx].frame = 0; }
	if (s->second) { m_native->srt[blend_idx].anim = s->second; m_native->srt[blend_idx].frame = 0; }
	if (p->second) { m_native->pat[blend_idx].anim = p->second; m_native->pat[blend_idx].frame = 0; }
	m_dicAnimationList[blend_idx] = h->entry;
	return blend_idx;
}

std::shared_ptr<wii::ChrAnim> cKingdomModelBase::ChrAnimOf(const AnimationClip* clip)
{
	const ResHandle* h = dynamic_cast<const ResHandle*>(clip);
	if (h == nullptr || h->entry.empty()) return nullptr;
	auto c = g_chr.find(h->entry);
	if (c == g_chr.end())
	{
		wii::Brres b;
		auto a = std::make_shared<wii::ChrAnim>();
		if (!LoadBrresEntry(h->entry, b) || !a->Load(b)) a.reset();
		c = g_chr.emplace(h->entry, a).first;
	}
	return c->second;
}

std::shared_ptr<wii::ChrAnim> cKingdomModelBase::AnmChrOf(uint idx)
{
	return (m_native != nullptr && idx < (uint)BLEND_MAX) ? m_native->chr[idx].anim : nullptr;
}

Matrix4x4 cKingdomModelBase::BoneRt(const wii::BoneKey& k, bool keepTranslation)
{
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	if (keepTranslation) ::Constants::jetGeoMtxTrans(m, m, Vector4(k.trans[0], k.trans[1], k.trans[2], 1.0f));
	::Constants::jetGeoMtxRotZDeg(m, m, k.rotate[2]);
	::Constants::jetGeoMtxRotYDeg(m, m, k.rotate[1]);
	::Constants::jetGeoMtxRotXDeg(m, m, k.rotate[0]);
	return m;
}

void cKingdomModelBase::ReleaseAnm(AnimationClip*) {}
void cKingdomModelBase::ReleaseAnmChr(uint i) { if (m_native && i < BLEND_MAX) m_native->chr[i].anim.reset(); }
void cKingdomModelBase::ReleaseAnmVis(uint i) { if (m_native && i < BLEND_MAX) m_native->vis[i].anim.reset(); }
void cKingdomModelBase::ReleaseAnmClr(uint i) { if (m_native && i < BLEND_MAX) m_native->clr[i].anim.reset(); }
void cKingdomModelBase::ReleaseAnmTexPat(uint i) { if (m_native && i < BLEND_MAX) m_native->pat[i].anim.reset(); }
void cKingdomModelBase::ReleaseAnmTexSrt(uint i) { if (m_native && i < BLEND_MAX) m_native->srt[i].anim.reset(); }
void cKingdomModelBase::ReleaseAnmShp(uint) {}

int cKingdomModelBase::AnmBlendAttach(cKingdomModel*, Animation*, int blend_idx, uint, AnimationClip*, string)
{
	return blend_idx;
}

#define NATIVE_OK(i) (m_native != nullptr && (i) < (uint)BLEND_MAX)

void cKingdomModelBase::SetFrameAnmChr(uint i, float frame) { if (NATIVE_OK(i)) m_native->chr[i].frame = frame; }
void cKingdomModelBase::SetFrameAnmVis(uint i, float frame) { if (NATIVE_OK(i)) m_native->vis[i].frame = frame; }
void cKingdomModelBase::SetFrameMatClr(uint i, float frame) { if (NATIVE_OK(i)) m_native->clr[i].frame = frame; }
void cKingdomModelBase::SetFrameAnmTexPat(uint i, float frame) { if (NATIVE_OK(i)) m_native->pat[i].frame = frame; }
void cKingdomModelBase::SetFrameAnmTexSrt(uint i, float frame) { if (NATIVE_OK(i)) m_native->srt[i].frame = frame; }
void cKingdomModelBase::SetFrameAnmShp(uint, float) {}

void cKingdomModelBase::SetFrameAnmAll(uint i, float frame)
{
	SetFrameAnmChr(i, frame);
	SetFrameAnmTexPat(i, frame);
	SetFrameAnmTexSrt(i, frame);
	SetFrameAnmVis(i, frame);
	SetFrameMatClr(i, frame);
}

// -1 = "no such animation" (what the C# returns without a clip)
float cKingdomModelBase::GetFrameAnmChr(uint i) { return NATIVE_OK(i) && m_native->chr[i].anim ? m_native->chr[i].frame : -1.0f; }
float cKingdomModelBase::GetFrameAnmVis(uint i) { return NATIVE_OK(i) && m_native->vis[i].anim ? m_native->vis[i].frame : -1.0f; }
float cKingdomModelBase::GetFrameMatClr(uint i) { return NATIVE_OK(i) && m_native->clr[i].anim ? m_native->clr[i].frame : -1.0f; }
float cKingdomModelBase::GetFrameAnmTexPat(uint i) { return NATIVE_OK(i) && m_native->pat[i].anim ? m_native->pat[i].frame : -1.0f; }
float cKingdomModelBase::GetFrameAnmTexSrt(uint i) { return NATIVE_OK(i) && m_native->srt[i].anim ? m_native->srt[i].frame : -1.0f; }
float cKingdomModelBase::GetFrameAnmShp(uint) { return -1.0f; }

float cKingdomModelBase::GetFrameMaxAnmChr(uint i) { return NATIVE_OK(i) && m_native->chr[i].anim ? (float)m_native->chr[i].anim->numFrame : -1.0f; }
float cKingdomModelBase::GetFrameMaxAnmVis(uint i) { return NATIVE_OK(i) && m_native->vis[i].anim ? (float)m_native->vis[i].anim->numFrame : -1.0f; }
float cKingdomModelBase::GetFrameMaxMatClr(uint i) { return NATIVE_OK(i) && m_native->clr[i].anim ? (float)m_native->clr[i].anim->numFrame : -1.0f; }
float cKingdomModelBase::GetFrameMaxAnmTexPat(uint i) { return NATIVE_OK(i) && m_native->pat[i].anim ? (float)m_native->pat[i].anim->numFrame : -1.0f; }
float cKingdomModelBase::GetFrameMaxAnmTexSrt(uint i) { return NATIVE_OK(i) && m_native->srt[i].anim ? (float)m_native->srt[i].anim->numFrame : -1.0f; }
float cKingdomModelBase::GetFrameMaxAnmShp(uint) { return -1.0f; }

void cKingdomModelBase::SetUpdateRateAnmChr(uint i, float rate) { if (NATIVE_OK(i)) m_native->chr[i].rate = rate; }
void cKingdomModelBase::SetUpdateRateAnmVis(uint, float) {}
void cKingdomModelBase::SetUpdateRateMatClr(uint i, float rate) { if (NATIVE_OK(i)) m_native->clr[i].rate = rate; }
void cKingdomModelBase::SetUpdateRateAnmTexPat(uint i, float rate) { if (NATIVE_OK(i)) m_native->pat[i].rate = rate; }
void cKingdomModelBase::SetUpdateRateAnmTexSrt(uint i, float rate) { if (NATIVE_OK(i)) m_native->srt[i].rate = rate; }
void cKingdomModelBase::SetUpdateRateAnmShp(uint, float) {}

void cKingdomModelBase::SetUpdateRateAll(uint i, float rate)
{
	SetUpdateRateAnmChr(i, rate);
	SetUpdateRateAnmTexPat(i, rate);
	SetUpdateRateAnmTexSrt(i, rate);
	SetUpdateRateMatClr(i, rate);
}

float cKingdomModelBase::GetUpdateRateAnmChr(uint i) { return NATIVE_OK(i) ? m_native->chr[i].rate : 0.0f; }
float cKingdomModelBase::GetUpdateRateAnmVis(uint) { return 0.0f; }
float cKingdomModelBase::GetUpdateRateMatClr(uint i) { return NATIVE_OK(i) ? m_native->clr[i].rate : 0.0f; }
float cKingdomModelBase::GetUpdateRateAnmTexPat(uint i) { return NATIVE_OK(i) ? m_native->pat[i].rate : 0.0f; }
float cKingdomModelBase::GetUpdateRateAnmTexSrt(uint i) { return NATIVE_OK(i) ? m_native->srt[i].rate : 0.0f; }
float cKingdomModelBase::GetUpdateRateAnmShp(uint) { return 0.0f; }

void cKingdomModelBase::SetWeightAnmChr(uint i, float weight) { if (NATIVE_OK(i)) m_native->chr[i].weight = weight; }
float cKingdomModelBase::GetWeightAnmChr(uint i) { return NATIVE_OK(i) ? m_native->chr[i].weight : 0.0f; }
void cKingdomModelBase::SetWeightAnmShp(uint, float) {}
float cKingdomModelBase::GetWeightAnmShp(uint) { return 0.0f; }

namespace
{
// current pose of a node (3x4 row major, model space) of the model's own draw, posed with the model's CHR layers
bool PosedNode(cKingdomModelBase::Native* n, const std::string& name, float m[12]);
}

void cKingdomModelBase::GetLocalNodeScale(Vector4& v, string node)
{
	v = Vector4::one;
	float m[12];
	if (PosedNode(m_native, node, m))
	{
		const float sx = std::sqrt(m[0] * m[0] + m[4] * m[4] + m[8] * m[8]), sy = std::sqrt(m[1] * m[1] + m[5] * m[5] + m[9] * m[9]), sz = std::sqrt(m[2] * m[2] + m[6] * m[6] + m[10] * m[10]);
		v = Vector4(sx, sy, sz, 1.0f);
	}
}
void cKingdomModelBase::GetLocalNodeScale(Vector4 v, uint node) {}

void cKingdomModelBase::GetLocalNodeRotate(Vector4& v, string node)
{
	v = Vector4::zero;
	float m[12];
	if (PosedNode(m_native, node, m))
	{
		// XYZ euler (radians) of the rotation part, for a Z*Y*X composed matrix: m02 = -sin y
		const float sy = -m[8] / (std::sqrt(m[0] * m[0] + m[4] * m[4] + m[8] * m[8]) > 1e-6f ? std::sqrt(m[0] * m[0] + m[4] * m[4] + m[8] * m[8]) : 1.0f);
		v = Vector4(std::atan2(m[9], m[10]), std::asin(sy < -1 ? -1 : (sy > 1 ? 1 : sy)), std::atan2(m[4], m[0]), 1.0f);
	}
}
void cKingdomModelBase::GetLocalNodeRotate(Vector4 v, uint node) {}

void cKingdomModelBase::GetLocalNodeTrans(Vector4& v, string node)
{
	v = Vector4::zero;
	float m[12];
	if (PosedNode(m_native, node, m)) v = Vector4(m[3], m[7], m[11], 1.0f);
}
void cKingdomModelBase::GetLocalNodeTrans(Vector4 v, uint node) {}

Transform* cKingdomModelBase::GetLocalNode(string node) { return nullptr; }

// one frame of every animation slot
void cKingdomModelBase::UpdateFrame()
{
	if (m_native == nullptr) return;
	for (int i = 0; i < BLEND_MAX; i++)
	{
		auto& c = m_native->chr[i];
		if (c.anim) { c.frame += c.rate; if (c.anim->loops) { if (c.anim->numFrame > 0) c.frame = std::fmod(std::fmod(c.frame, (float)c.anim->numFrame) + c.anim->numFrame, (float)c.anim->numFrame); } else if (c.frame > c.anim->numFrame) c.frame = (float)c.anim->numFrame; }
		auto& s = m_native->srt[i];
		if (s.anim) { s.frame += s.rate; if (s.anim->loops && s.anim->numFrame > 0) s.frame = std::fmod(s.frame, (float)s.anim->numFrame); }
		auto& cl = m_native->clr[i];
		if (cl.anim) { cl.frame += cl.rate; if (cl.anim->loops && cl.anim->numFrame > 0) cl.frame = std::fmod(cl.frame, (float)cl.anim->numFrame); else if (cl.frame > cl.anim->numFrame) cl.frame = (float)cl.anim->numFrame; }
		auto& vs = m_native->vis[i];
		if (vs.anim) { vs.frame += vs.rate; if (vs.anim->loops && vs.anim->numFrame > 0) vs.frame = std::fmod(vs.frame, (float)vs.anim->numFrame); else if (vs.frame > vs.anim->numFrame) vs.frame = (float)vs.anim->numFrame; }
		auto& p = m_native->pat[i];
		if (p.anim) { p.frame += p.rate; if (p.anim->loops && p.anim->numFrame > 0) p.frame = std::fmod(p.frame, (float)p.anim->numFrame); }
	}
}

void cKingdomModelBase::SetNativeMtx(const Matrix4x4& m)
{
	m_useMtx = true;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++) m_mtxCM[c * 4 + r] = m.m[r * 4 + c];
	for (int c = 0; c < 3; c++)
	{
		const float l = std::sqrt(m_mtxCM[c * 4] * m_mtxCM[c * 4] + m_mtxCM[c * 4 + 1] * m_mtxCM[c * 4 + 1] + m_mtxCM[c * 4 + 2] * m_mtxCM[c * 4 + 2]);
		if (l > 1e-6f) for (int r = 0; r < 3; r++) m_mtxCM[c * 4 + r] /= l;
	}
}

void cKingdomModelBase::SetNativeScale(float x, float y, float z)
{
	m_mtxScale[0] = x; m_mtxScale[1] = y; m_mtxScale[2] = z;
}

void cKingdomModelBase::SetNativeMtxRaw(const float cm[16])
{
	m_useMtx = true;
	for (int i = 0; i < 16; i++) m_mtxCM[i] = cm[i];
	m_mtxScale[0] = m_mtxScale[1] = m_mtxScale[2] = 1.0f;
}

void cKingdomModelBase::GetNativeMtx(Matrix4x4& out) const
{
	float w[16];
	CalcWorld(w);
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++) out.m[r * 4 + c] = w[c * 4 + r];
}

void cKingdomModelBase::CalcWorld(float out[16]) const
{
	if (m_useMtx)
	{
		for (int c = 0; c < 3; c++)
			for (int r = 0; r < 4; r++) out[c * 4 + r] = m_mtxCM[c * 4 + r] * m_mtxScale[c];
		out[3] = out[7] = out[11] = 0.0f;
		out[12] = m_mtxCM[12]; out[13] = m_mtxCM[13]; out[14] = m_mtxCM[14]; out[15] = 1.0f;
		return;
	}
	const float rx = m_vRotate.x, ry = m_vRotate.y, rz = m_vRotate.z;
	const float cx = std::cos(rx), sx = std::sin(rx), cy = std::cos(ry), sy = std::sin(ry), cz = std::cos(rz), sz = std::sin(rz);
	// R = Rz * Ry * Rx (column vectors); rows of R:
	const float r[3][3] = {{cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx}, {sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx}, {-sy, cy * sx, cy * cx}};
	const float s[3] = {m_vScale.x, m_vScale.y, m_vScale.z};
	for (int c = 0; c < 3; c++)
		for (int row = 0; row < 3; row++) out[c * 4 + row] = r[row][c] * s[c];
	out[3] = out[7] = out[11] = 0.0f;
	out[12] = m_vTrans.x; out[13] = m_vTrans.y; out[14] = m_vTrans.z; out[15] = 1.0f;
}

// computed when the task draws (cTaskModelBase::Draw): the world matrix and the submission to this frame's 3D pass (DOL 0x8009c3f4 [audited: submission to the per-frame list])
void cKingdomModelBase::UpdateMatrix()
{
	CalcWorld(m_world);
	if (!m_bExpandChild) NativeScene::Submit(this);
}

void cKingdomModelBase::AttachShadow() {}
void cKingdomModelBase::SetLightSetIdx(int idx) { m_nLightSetIdx = idx; }
int cKingdomModelBase::GetLightSetIdx() { return m_nLightSetIdx; }
void cKingdomModelBase::SetFogIdx(int idx) { m_nFogIdx = idx; }
int cKingdomModelBase::GetFogIdx() { return m_nFogIdx; }

void cKingdomModelBase::GarbageModel(cKingdomModelBase*) {}
void cKingdomModelBase::EnableGarbageModel() {}
void cKingdomModelBase::ClearGarbageModel() {}
void cKingdomModelBase::ForceClearGarbageModel() {}

// ---- drawing ----------------------------------------------------------------------------------------------------------------------

namespace
{
void PoseNative(cKingdomModelBase::Native* n)
{
	std::vector<gfx::ModelDraw::ChrLayer> layers;
	for (int i = 0; i < cKingdomModelBase::BLEND_MAX; i++)
		if (n->chr[i].anim && n->chr[i].weight > 0.0f && n->chr[i].anim->numFrame > 0)
			layers.push_back({n->chr[i].anim.get(), wii::ClipFrame(n->chr[i].frame, n->chr[i].anim->numFrame, n->chr[i].anim->loops), n->chr[i].weight});
	static const bool noPose = std::getenv("MEGAMIX_NOPOSE") != nullptr;   // debug: bind pose
	if (noPose) layers.clear();
	n->draw->SetPose(layers);
	// every slot's SRT0 / PAT0 clip (a BG model registers one per "_rtsa" / "_rtpa" user data entry; only the first was drawn: w4 magma, w2s2 pads)
	n->draw->SetTexSrt(nullptr, 0);
	for (int i = 0; i < cKingdomModelBase::BLEND_MAX; i++)
		if (n->srt[i].anim) n->draw->AddTexSrt(n->srt[i].anim.get(), n->srt[i].frame);
	n->draw->SetClrAnim(nullptr, 0);
	for (int i = 0; i < cKingdomModelBase::BLEND_MAX; i++)
		if (n->clr[i].anim) { n->draw->SetClrAnim(n->clr[i].anim.get(), n->clr[i].frame); break; }
	n->draw->SetVisAnim(nullptr, 0);
	for (int i = 0; i < cKingdomModelBase::BLEND_MAX; i++)
		if (n->vis[i].anim) { n->draw->SetVisAnim(n->vis[i].anim.get(), n->vis[i].frame); break; }
	n->draw->SetTexPat(nullptr, 0);
	for (int i = 0; i < cKingdomModelBase::BLEND_MAX; i++)
		if (n->pat[i].anim) n->draw->AddTexPat(n->pat[i].anim.get(), n->pat[i].frame);
}

bool PosedNode(cKingdomModelBase::Native* n, const std::string& name, float m[12])
{
	if (n == nullptr || !n->draw) return false;
	PoseNative(n);
	return n->draw->NodeMatrix(name, m);
}
}

// the shared ModelDraw is posed for this instance right before it draws
void DrawNativeModel(cKingdomModelBase::Native* n, const float world[16], int lightSet, int fogIdx)
{
	if (n == nullptr || !n->draw) return;
	if (gfx::ModelDraw::Pass() == 2 && !n->draw->HasXlu()) return;   // the translucent pass: nothing to draw, no pose
	PoseNative(n);
	if (const wii::LightResult* l = NativeScene::Light(lightSet))
	{
		n->draw->SetLightRig(*l);   // the single light, or the whole light set when the scene sampled one (numRig > 0)
		// fog: only the background (0) and gimmick (3) scenes: the character scene's fog0 (REVEXP2, 0..130) is UNVERIFIED, it is not applied
		if (lightSet == 0 || lightSet == 3) n->draw->SetFog(l->fogType, l->fogStart, l->fogEnd, l->fogColor);
		else n->draw->SetFog(0, 0, 0, l->fogColor);
	}
	// SetFogIdx(i): the model is fogged by the scene's fog slot i (DOL kingdom 0x8001e400 -> ScnRoot::GetFog(i)); ModelDraw drops it for materials
	// whose own fog index is < 0
	if (fogIdx >= 0)
		if (const NativeScene::FogSlot* f = NativeScene::GetFogSlot(fogIdx)) n->draw->SetFog(f->type, f->start, f->end, f->color);
	n->draw->Draw(world);
}

bool NativeNodeMatrix(cKingdomModelBase::Native* n, const std::string& node, float m[12]) { return PosedNode(n, node, m); }
bool cKingdomModelBase::NodeMatrix(const std::string& node, float m3x4[12]) { return PosedNode(m_native, node, m3x4); }

bool cKingdomModelBase::PosedSkin(std::vector<float>& out)
{
	out.clear();
	if (m_native == nullptr || !m_native->draw) return false;
	PoseNative(m_native);
	out = m_native->draw->Skin();
	return true;
}

bool cKingdomModelBase::HasChrPose() const
{
	if (m_native == nullptr) return false;
	for (int i = 0; i < BLEND_MAX; i++)
		if (m_native->chr[i].anim && m_native->chr[i].weight > 0.0f && m_native->chr[i].anim->numFrame > 0) return true;
	return false;
}
bool NativeHasModel(const cKingdomModelBase::Native* n) { return n != nullptr && n->draw != nullptr; }

}
