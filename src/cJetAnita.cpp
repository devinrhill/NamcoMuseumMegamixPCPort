#include <cmath>
#include "cJetAnita.hpp"

#include "Constants.hpp"
#include "CustomKingdomUtil.hpp"
#include "GameDefine.hpp"
#include "WiiSystem.hpp"
#include "gfx/Gfx.hpp"
#include "gfx/TextureCache.hpp"
#include <cstdio>
#include <cstdlib>
#include "jet/JetUtil.hpp"

using namespace jet;

const int cJetAnita::caTRIANGLES[6] = { 0, 1, 2, 1, 3, 2 };

Shader* cJetAnita::g_aShaders[3]{};

int cJetAnita::g_iDrawCount = 0;

const int cJetAnita::scMainTexPropertyID = 0; // TODO(unity): Shader.PropertyToID("_MainTex");

Vector3 cJetAnita::m_tmpVertices[4] = {};

Vector2 cJetAnita::m_tmpUVs[4] = {};

Color32 cJetAnita::m_tmpColors[4] = {};

cJetAnita::MeshInfo* cJetAnita::GetMesh(int index, float w, float h)
{
	if (index < (int)m_vMeshs.size())
	{
		if (m_vMeshs[index]->h != h || m_vMeshs[index]->w != w)
		{
			m_tmpVertices[0] = Vector3(0.0f - w, h, 0.0f);
			m_tmpVertices[1] = Vector3(0.0f - w, 0.0f - h, 0.0f);
			m_tmpVertices[2] = Vector3(w, h, 0.0f);
			m_tmpVertices[3] = Vector3(w, 0.0f - h, 0.0f);
			// TODO(unity): m_vMeshs[index].mesh.SetVertices(m_tmpVertices);
			m_vMeshs[index]->h = h;
			m_vMeshs[index]->w = w;
			// TODO(unity): m_vMeshs[index].mesh.RecalculateBounds();
		}
		return m_vMeshs[index];
	}
	Mesh* mesh = nullptr; // TODO(unity): new Mesh();
	// TODO(unity): mesh.MarkDynamic();
	m_tmpVertices[0] = Vector3(0.0f - w, h, 0.0f);
	m_tmpVertices[1] = Vector3(0.0f - w, 0.0f - h, 0.0f);
	m_tmpVertices[2] = Vector3(w, h, 0.0f);
	m_tmpVertices[3] = Vector3(w, 0.0f - h, 0.0f);
	// TODO(unity): mesh.SetVertices(m_tmpVertices);
	// TODO(unity): mesh.triangles = caTRIANGLES;
	// TODO(unity): mesh.RecalculateNormals();
	// TODO(unity): mesh.RecalculateBounds();
	MeshInfo* meshInfo = new MeshInfo();
	meshInfo->mesh = mesh;
	meshInfo->w = w;
	meshInfo->h = h;
	m_vMeshs.push_back(meshInfo);
	return m_vMeshs[m_vMeshs.size() - 1];
}

Material* cJetAnita::GetMaterial(int index, EAnitaLay blendMode)
{
	Material* material = nullptr;
	if (index < (int)m_vMaterials.size())
	{
		MaterialInfo* materialInfo = m_vMaterials[index];
		material = materialInfo->mat;
		if (materialInfo->blendMode != blendMode)
		{
			// TODO(unity): material.shader = g_aShaders[(int)blendMode];
			materialInfo->blendMode = blendMode;
		}
	}
	else
	{
		material = nullptr; // TODO(unity): new Material(g_aShaders[(int)blendMode]);
		MaterialInfo* materialInfo2 = new MaterialInfo();
		materialInfo2->mat = material;
		materialInfo2->blendMode = blendMode;
		m_vMaterials.push_back(materialInfo2);
	}
	return material;
}

void cJetAnita::ShaderInit()
{
	if (g_aShaders[0] == nullptr)
	{
		g_aShaders[2] = nullptr; // TODO(unity): Shader.Find("Unlit/SimpleVertexColorAddShader");
		g_aShaders[1] = nullptr; // TODO(unity): Shader.Find("Unlit/SimpleVertexColorShader");
		g_aShaders[0] = nullptr; // TODO(unity): Shader.Find("Unlit/SimpleVertexColorShader");
	}
}

void cJetAnita::SetDrawLayer(int layer)
{
	m_iDrawLayer = layer;
}

void cJetAnita::DrawInit()
{
	g_iDrawCount = 0;
}

bool cJetAnita::GXSetAnitaEnv()
{
	return true;
}

bool cJetAnita::DrawPat(int pat)
{
	byte* pAnita = !m_pAnita.empty() ? m_pAnita.data() : nullptr;
	{
		void* ptr = pAnita;
		JetUtil::anpPatCount(ptr);
		int num = JetUtil::anpPatLayCount(ptr, pat) - 1;
		if (num < 0)
		{
			return false;
		}
		uDatLayP* ptr2 = JetUtil::anpPatLayPtr(ptr, pat, num);
		while (num > -1)
		{
			DrawLayer((sDatLayP*)ptr2);
			num--;
			ptr2--;
		}
	}
	return true;
}

bool cJetAnita::DrawPatIntepolate()
{
	byte* pAnita = !m_pAnita.empty() ? m_pAnita.data() : nullptr;
	{
		void* ptr = pAnita;
		int num = JetUtil::anpPatLayCount(ptr, m_AnimSeq->pat_noA) - 1;
		if (num < 0)
		{
			return false;
		}
		float num2 = m_AnimSeq->frm_noB - m_AnimSeq->frm_noA;
		if (num2 != 0.0f)
		{
			num2 = (m_AnimSeq->frm_cnt - m_AnimSeq->frm_noA) / num2;
		}
		if (num2 <= 0.0f)
		{
			return DrawPat(m_AnimSeq->pat_noA);
		}
		if (num2 >= 1.0f)
		{
			return DrawPat(m_AnimSeq->pat_noB);
		}
		uDatLayP* ptr2 = JetUtil::anpPatLayPtr(ptr, m_AnimSeq->pat_noA, num);
		uDatLayP* ptr3 = JetUtil::anpPatLayPtr(ptr, m_AnimSeq->pat_noB, num);
		sDatLayP sDatLayP2 = sDatLayP();
		while (num > -1)
		{
			Interpolate(&sDatLayP2, (sDatLayP*)ptr2, (sDatLayP*)ptr3, num2);
			DrawLayer(&sDatLayP2);
			num--;
			ptr2--;
			ptr3--;
		}
	}
	return true;
}

bool cJetAnita::DrawLayer(sDatLayP* pLay)
{
	if (pLay->a == 0 || pLay->tex == -1)
	{
		return false;
	}
	AnitaTex* anitaTex = m_Textures[pLay->tex];
	const bool textured = anitaTex->pBrres != nullptr && anitaTex->pBrres->gfxId != 0;
	// texture rectangle: pixels of the image (v counted from the top); the Unity build flipped v, the gfx layer does not
	float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
	if (textured && anitaTex->Width != 0 && anitaTex->Height != 0)
	{
		u0 = pLay->u0 / (float)anitaTex->Width;
		v0 = pLay->v0 / (float)anitaTex->Height;
		u1 = pLay->u1 / (float)anitaTex->Width;
		v1 = pLay->v1 / (float)anitaTex->Height;
		if (pLay->filter != 1)
		{
			// platform: layers flagged NEAREST are drawn LINEAR (below). The Wii shows layouts 1:1 at 640 x 480, where the flag makes no
			// visible difference; the port scales them up and nearest turns blocky. Pull the rectangle in by half a texel so linear
			// sampling never reaches the neighbouring sprite of an atlas (u0 > u1 / v0 > v1 = mirrored layer).
			const float hu = 0.5f / anitaTex->Width, hv = 0.5f / anitaTex->Height;
			if (std::fabs(u1 - u0) > 2 * hu) { const float s = u1 > u0 ? hu : -hu; u0 += s; u1 -= s; }
			if (std::fabs(v1 - v0) > 2 * hv) { const float s = v1 > v0 ? hv : -hv; v0 += s; v1 -= s; }
		}
	}
	LayerAffine(pLay);
	const Matrix4x4 m = m_userAffineMtx * m_mtx;
	const float w = pLay->suh;
	const float h = pLay->svh;
	// top-left, bottom-left, top-right, bottom-right of the layer's mesh (see GetMesh)
	const Vector4 corners[4] = {Vector4(-w, h, 0.0f, 1.0f), Vector4(-w, -h, 0.0f, 1.0f), Vector4(w, h, 0.0f, 1.0f), Vector4(w, -h, 0.0f, 1.0f)};
	float xy[8];
	for (int i = 0; i < 4; i++)
	{
		const Vector4 p = m * corners[i];
		xy[i * 2] = p.x;
		xy[i * 2 + 1] = p.y;
	}
	const float uv[8] = {u0, v0, u0, v1, u1, v0, u1, v1};
	if (std::getenv("MEGAMIX_TRACE_ANITA"))
	{
		std::printf("anita quad tex=%d p=(%g,%g) is=(%g,%g,rot %g) oc=(%g,%g) os=(%g,%g,rot %g) half=(%g,%g) -> TL(%.1f,%.1f) BL(%.1f,%.1f) TR(%.1f,%.1f) BR(%.1f,%.1f)\n",
			pLay->tex, pLay->px, pLay->py, pLay->isx, pLay->isy, pLay->irot, pLay->ocx, pLay->ocy, pLay->osx, pLay->osy, pLay->orot, w, h,
			xy[0], xy[1], xy[2], xy[3], xy[4], xy[5], xy[6], xy[7]);
	}
	// DOL 0x80142878: layer +0x50 MUL (1) -> GXSetBlendMode(BLEND, SRCALPHA, INVSRCALPHA), ADD (2) -> (BLEND, SRCALPHA, ONE), NON (0) or
	// anything else -> GX_BM_NONE; the Anita draw setup (FUN_80141eec) sets the alpha compare to ALWAYS, so a NON layer is drawn opaque
	const gfx::Blend blend = pLay->blend == (int)jet::eANITA_LAY_BLEND_MUL   ? gfx::BLEND_ALPHA
	                         : pLay->blend == (int)jet::eANITA_LAY_BLEND_ADD ? gfx::BLEND_ADD
	                                                                         : gfx::BLEND_NONE;
	if (textured)
	{
		gfx::DrawQuad(anitaTex->pBrres->gfxId, xy, uv, pLay->r, pLay->g, pLay->b, pLay->a, blend, true);   // see the NEAREST note above
	}
	else
	{
		// a texture that could not be found: a faint magenta ghost so the layer is still visible
		gfx::DrawQuad(gfx::NO_TEX, xy, uv, 255, 0, 255, pLay->a < 90 ? pLay->a : 90, blend, true);
	}
	m_mtx *= Matrix4x4::Translate(Vector3(0.0f, 0.0f, (float)(-g_iDrawCount) * 0.05f));
	g_iDrawCount++;
	m_iLayerIndex++;
	return true;
}

bool cJetAnita::Interpolate(sDatLayP* pLay0, sDatLayP* pLay1, sDatLayP* pLay2, float r)
{
	pLay0->u0 = Interpolate(pLay1->u0, pLay2->u0, r);
	pLay0->v0 = Interpolate(pLay1->v0, pLay2->v0, r);
	pLay0->u1 = Interpolate(pLay1->u1, pLay2->u1, r);
	pLay0->v1 = Interpolate(pLay1->v1, pLay2->v1, r);
	pLay0->su = Interpolate(pLay1->su, pLay2->su, r);
	pLay0->sv = Interpolate(pLay1->sv, pLay2->sv, r);
	pLay0->suh = Interpolate(pLay1->suh, pLay2->suh, r);
	pLay0->svh = Interpolate(pLay1->svh, pLay2->svh, r);
	pLay0->px = Interpolate(pLay1->px, pLay2->px, r);
	pLay0->py = Interpolate(pLay1->py, pLay2->py, r);
	pLay0->isx = Interpolate(pLay1->isx, pLay2->isx, r);
	pLay0->isy = Interpolate(pLay1->isy, pLay2->isy, r);
	pLay0->irot = InterpolateRot(pLay1->irot, pLay2->irot, r);
	pLay0->ocx = Interpolate(pLay1->ocx, pLay2->ocx, r);
	pLay0->ocy = Interpolate(pLay1->ocy, pLay2->ocy, r);
	pLay0->osx = Interpolate(pLay1->osx, pLay2->osx, r);
	pLay0->osy = Interpolate(pLay1->osy, pLay2->osy, r);
	pLay0->orot = InterpolateRot(pLay1->orot, pLay2->orot, r);
	pLay0->r = (byte)Interpolate((float)(int)pLay1->r, (float)(int)pLay2->r, r);
	pLay0->g = (byte)Interpolate((float)(int)pLay1->g, (float)(int)pLay2->g, r);
	pLay0->b = (byte)Interpolate((float)(int)pLay1->b, (float)(int)pLay2->b, r);
	pLay0->a = (byte)Interpolate((float)(int)pLay1->a, (float)(int)pLay2->a, r);
	pLay0->tex = pLay1->tex;
	pLay0->blend = pLay1->blend;
	pLay0->filter = pLay1->filter;
	pLay0->flag = pLay1->flag;
	return true;
}

float cJetAnita::Interpolate(float s, float e, float r)
{
	if (s != e)
	{
		return s + (e - s) * r;
	}
	return s;
}

float cJetAnita::InterpolateRot(float s, float e, float r)
{
	if (s == e)
	{
		return s;
	}
	if (e - s > 180.0f || e - s < -180.0f)
	{
		s += 360.0f;
	}
	float num;
	for (num = Interpolate(s, e, r); num > 360.0f; num -= 360.0f)
	{
	}
	return num;
}

bool cJetAnita::LayerAffine(sDatLayP* pLay)
{
	m_mtx = Constants::scMtxIdentity;
	Vector4 v = Vector4::zero;
	Constants::jetGeoVecSet(v, pLay->ocx, pLay->ocy, 0.0f, 1.0f);
	if (!CustomKingdomUtil::IsEqualFloat(v.x, 0.0f) || !CustomKingdomUtil::IsEqualFloat(v.y, 0.0f) || !CustomKingdomUtil::IsEqualFloat(v.z, 0.0f))
	{
		m_mtx *= Matrix4x4::Translate(v);
	}
	if (!CustomKingdomUtil::IsEqualFloat(0.0f - pLay->orot, 0.0f))
	{
		m_mtx *= Matrix4x4::Rotate(Quaternion::Euler(0.0f, 0.0f, 0.0f - pLay->orot));
	}
	Constants::jetGeoVecSet(v, pLay->osx, pLay->osy, 1.0f, 1.0f);
	if (!CustomKingdomUtil::IsEqualFloat(v.x, 1.0f) || !CustomKingdomUtil::IsEqualFloat(v.y, 1.0f))
	{
		m_mtx *= Matrix4x4::Scale(v);
	}
	Constants::jetGeoVecSet(v, 0.0f - pLay->ocx, 0.0f - pLay->ocy, 0.0f, 1.0f);
	if (!CustomKingdomUtil::IsEqualFloat(v.x, 0.0f) || !CustomKingdomUtil::IsEqualFloat(v.y, 0.0f))
	{
		m_mtx *= Matrix4x4::Translate(v);
	}
	Vector3 pos = Vector3(pLay->px, pLay->py, 0.0f);
	Vector3 s = Vector3(pLay->isx, pLay->isy, 1.0f);
	Matrix4x4 scMtxIdentity = Constants::scMtxIdentity;
	scMtxIdentity.SetTRS(pos, Quaternion::Euler(0.0f, 0.0f, 0.0f - pLay->irot), s);
	m_mtx *= scMtxIdentity;
	return true;
}

bool cJetAnita::LayerAffineUser()
{
	m_userAffineMtx.SetTRS(m_vTrans, Quaternion::Euler(0.0f, 0.0f, m_vRot[2]), m_vScale);
	return true;
}

cJetAnita::cJetAnita()
{
	Init();
}

cJetAnita::cJetAnita(const std::vector<byte>& anita)
{
	Init();
	SetData(anita);
}

cJetAnita::~cJetAnita()
{
	Term();
}

void cJetAnita::Init()
{
	m_pAnita.clear();
	m_nState = 0u;
	m_AnimSeq = new Seq();
	m_AnimSeq->frm_rate = 60.0f;
	std::fill(std::begin(m_Textures), std::end(m_Textures), nullptr);
	for (int i = 0; i < (int)std::size(m_Textures); i++)
	{
		m_Textures[i] = new AnitaTex();
		m_Textures[i]->pBrres = nullptr;
		m_Textures[i]->IsClut = false;
	}
	m_vTrans = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	m_vRot = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	m_vScale = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	ShaderInit();
}

void cJetAnita::Term()
{
	if (!m_vMeshs.empty())
	{
		for (int i = 0; i < (int)m_vMeshs.size(); i++)
		{
			// TODO(unity): ReleaseObjectManager.Regist(m_vMeshs[i].mesh);
		}
		m_vMeshs.clear();
	}
	if (!m_vMaterials.empty())
	{
		for (int j = 0; j < (int)m_vMaterials.size(); j++)
		{
			// TODO(unity): ReleaseObjectManager.Regist(m_vMaterials[j].mat);
		}
		m_vMaterials.clear();
	}
}

bool cJetAnita::SetData(const std::vector<byte>& anita, bool checkLanguage)
{
	m_pAnita = anita;
	if (!jet::JetUtil::anpToHostEndian(m_pAnita))
	{
		m_pAnita.clear();
		return false;
	}
	SetTexture(true, checkLanguage);
	m_nState |= 1u;
	m_nState |= 2u;
	SetAnimNo(0);
	return true;
}

std::vector<byte>& cJetAnita::GetData()
{
	return m_pAnita;
}

bool cJetAnita::SetTexture(bool overwrite, bool checkLanguage)
{
	byte* pAnita = !m_pAnita.empty() ? m_pAnita.data() : nullptr;
	{
		void* ptr = pAnita;
		for (int num = JetUtil::anpTexCount(ptr) - 1; num > -1; num--)
		{
			if (overwrite || GetTexture(num) == nullptr)
			{
				string text = JetUtil::anpTexTexName(ptr, num);
				if (!text.empty())
				{
					// the Wii build finds "<name>.brres" in the attached archives (the Unity build used ResourceCache)
					Texture* brres = gfx::FindTexture(text);
					if (brres == nullptr) std::fprintf(stderr, "cJetAnita: texture %s not found\n", text.c_str());
					SetTexture(num, brres);
				}
			}
		}
	}
	return true;
}

bool cJetAnita::SetTexture(int idx, Texture* brres)
{
	if (brres != nullptr)
	{
		m_Textures[idx]->pBrres = brres;
		m_Textures[idx]->Width = (uint)brres->width;
		m_Textures[idx]->Height = (uint)brres->height;
	}
	else
	{
		m_Textures[idx]->pBrres = nullptr;
		m_Textures[idx]->IsClut = false;
	}
	return true;
}

Texture* cJetAnita::GetTexture(int idx)
{
	return m_Textures[idx]->pBrres;
}

bool cJetAnita::AnimUpdate(float step)
{
	byte* pAnita = !m_pAnita.empty() ? m_pAnita.data() : nullptr;
	{
		void* ptr = pAnita;
		if (m_pAnita.empty())
		{
			return false;
		}
		if (JetUtil::anpAnmCount(ptr) == 0)
		{
			return false;
		}
		if ((m_nState & 2) == 0)
		{
			step = 0.0f;
		}
		step = ((m_AnimSeq->frm_rate == 0.0f) ? 0.0f : (step / m_AnimSeq->frm_rate));
		float frm_cnt = m_AnimSeq->frm_cnt;
		float num = frm_cnt + 60.0f * step;
		float num2 = 0.0f;
		float frm_end = m_AnimSeq->frm_end;
		if ((num >= frm_end && frm_cnt <= frm_end && num != frm_cnt) || (num <= num2 && frm_cnt <= frm_end && num != frm_cnt))
		{
			uDatAnmP* ptr2 = JetUtil::anpAnmPtr(ptr, m_AnimSeq->anm_no);
			if (0 < (ptr2->anm.flag & 1))
			{
				num = ((step < 0.0f) ? (frm_end + num) : (num - frm_end));
			}
			else
			{
				if (0 < (ptr2->anm.flag & 2))
				{
					num = ((step < 0.0f) ? (num2 - num) : (num - frm_end));
					return SetAnimNo(ptr2->anm.end_jmp, true, num);
				}
				if (0 < (m_nState & 4))
				{
					num = ((step < 0.0f) ? (frm_end + num) : (num - frm_end));
				}
				else
				{
					num = ((step < 0.0f) ? num2 : frm_end);
					m_nState |= 65536u;
				}
			}
		}
		m_AnimSeq->frm_cnt = num;
		uDatSeqP* ptr3 = JetUtil::anpAnmSeqBSPtr(ptr, m_AnimSeq->anm_no, m_AnimSeq->frm_cnt);
		switch (ptr3->seq.cmd)
		{
		case 0:
			m_AnimSeq->pat_noA = ptr3->seq.para0;
			m_AnimSeq->pat_noB = ptr3->seq.para0;
			m_AnimSeq->frm_noA = ptr3->seq.frm;
			m_AnimSeq->frm_noB = (float)ptr3->seq.frm + (float)ptr3->seq.para1;
			break;
		case 1:
		{
			uDatSeqP* ptr4 = ptr3 - 1;
			m_AnimSeq->pat_noA = ptr4->seq.para0;
			m_AnimSeq->frm_noA = ptr3->seq.frm;
			ptr4 = ptr3 + 1;
			m_AnimSeq->pat_noB = ptr4->seq.para0;
			m_AnimSeq->frm_noB = ptr4->seq.frm;
			break;
		}
		}
	}
	return true;
}

bool cJetAnita::AnimUpdate()
{
	return AnimUpdate(1.0f);
}

bool cJetAnita::Draw(int pat)
{
	if (m_pAnita.empty())
	{
		return false;
	}
	if (!IsState(1u))
	{
		return true;
	}
	m_iLayerIndex = 0;
	GXSetAnitaEnv();
	m_mtx = Constants::scMtxIdentity;
	m_userAffineMtx = Constants::scMtxIdentity;
	LayerAffineUser();
	if (pat < 0)
	{
		if (m_AnimSeq->pat_noA == m_AnimSeq->pat_noB)
		{
			DrawPat(m_AnimSeq->pat_noA);
		}
		else
		{
			DrawPatIntepolate();
		}
	}
	else
	{
		DrawPat(pat);
	}
	return true;
}

void cJetAnita::SetZMode(uint compare, uint func, uint update)
{
	m_nCompare = compare;
	m_nFunc = func;
	m_nUpdate = update;
}

bool cJetAnita::SetAnimNo(int no, bool reset, float frm)
{
	byte* pAnita = !m_pAnita.empty() ? m_pAnita.data() : nullptr;
	{
		void* ptr = pAnita;
		if (m_pAnita.empty())
		{
			return false;
		}
		if (JetUtil::anpAnmCount(ptr) == 0)
		{
			return false;
		}
		if (no == m_AnimSeq->anm_no && !reset)
		{
			return true;
		}
		if (no < 0)
		{
			return false;
		}
		JetUtil::anpAnmCount(ptr);
		uDatAnmP* ptr2 = JetUtil::anpAnmPtr(ptr, no);
		m_AnimSeq->anm_no = no;
		m_AnimSeq->frm_cnt = ((m_AnimSeq->frm_rate < 0.0f) ? ((float)ptr2->anm.end_frm - frm) : frm);
		m_AnimSeq->frm_end = ptr2->anm.end_frm;
		m_nState &= 4294901759u;
		m_nState &= 4294705151u;
		if (0 < (ptr2->anm.flag & 1))
		{
			m_nState |= 262144u;
		}
		AnimUpdate(0.0f);
	}
	return true;
}

int cJetAnita::GetAnimNo()
{
	return m_AnimSeq->anm_no;
}

bool cJetAnita::IsState(uint flag)
{
	return (m_nState & flag) != 0;
}

uint cJetAnita::GetState()
{
	return m_nState;
}

bool cJetAnita::SetState(uint flag, bool bit)
{
	if (bit)
	{
		m_nState |= flag & 0xFFFF;
	}
	else
	{
		m_nState &= ~(flag & 0xFFFF);
	}
	return (flag & 0xFFFF0000u) == 0;
}

float cJetAnita::GetFrameRate()
{
	return m_AnimSeq->frm_rate;
}

void cJetAnita::SetFrameRate(float rate)
{
	if (rate < 0.0f)
	{
		m_nState |= 131072u;
	}
	else
	{
		m_nState &= 4294836223u;
	}
	m_AnimSeq->frm_rate = rate;
}

float cJetAnita::GetFrame()
{
	return m_AnimSeq->frm_cnt;
}

void cJetAnita::SetFrame(float frm)
{
	m_AnimSeq->frm_cnt = frm;
	AnimUpdate(0.0f);
}

float cJetAnita::GetFrameNum(int no)
{
	if (m_pAnita.empty())
	{
		return 0.0f;
	}
	if (no < 0)
	{
		return m_AnimSeq->frm_end;
	}
	byte* pAnita = m_pAnita.data();
	{
		void* ptr = pAnita;
		return JetUtil::anpAnmPtr(ptr, no)->anm.end_frm;
	}
}

uint cJetAnita::GetPatNum()
{
	if (m_pAnita.empty())
	{
		return 0u;
	}
	byte* pAnita = m_pAnita.data();
	{
		void* ptr = pAnita;
		return (uint)JetUtil::anpPatCount(ptr);
	}
}

uint cJetAnita::GetAnimNum()
{
	if (m_pAnita.empty())
	{
		return 0u;
	}
	byte* pAnita = m_pAnita.data();
	{
		void* ptr = pAnita;
		return (uint)JetUtil::anpAnmCount(ptr);
	}
}

Vector4& cJetAnita::vTrans()
{
	return m_vTrans;
}

Vector4& cJetAnita::vRot()
{
	return m_vRot;
}

Vector4& cJetAnita::vScale()
{
	return m_vScale;
}
