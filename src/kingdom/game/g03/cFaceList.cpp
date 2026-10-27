#include "kingdom/game/g03/cFaceList.hpp"
#include <algorithm>

#include <iterator>
#include <limits>

#include "CSharpCompat.hpp"
#include "CustomKingdomUtil.hpp"
#include "KingdomGlobal.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <Extentions.hpp>

namespace kingdom::game::g03
{

bool cFaceList::IsAABB()
{
	return m_bAABB;
}

AABB cFaceList::pAABB()
{
	return m_AABB;
}

AABB& cFaceList::ReadOnlyAABB()
{
	return m_AABB;
}

ResMdl* cFaceList::pResMdl()
{
	return m_ResMdl;
}

uint cFaceList::nFaceNum(int n)
{
	return (uint)(int)m_vecFace[n].size();
}

Constants::sFace* cFaceList::pFace(int n, int i)
{
	return m_vecFace[n][i];
}

std::vector<Constants::sFace*>* cFaceList::pAllFace()
{
	return m_vecFace;
}

bool& cFaceList::IsReuse()
{
	return m_isReuse;
}

AABB& cFaceList::CalcedAABB()
{
	return m_calcedAABB;
}

cFaceList::cFaceList(ResMdl* data)
{
	CustomKingdomUtil::Assert(data != nullptr, "nullpo");
	Init();
	RegistFace(data);
	SearchHangEdge();
}

cFaceList::cFaceList(ResMdl* data, const std::vector<Constants::sFace*> faces[4], const AABB& aabb)
{
	Init();
	m_ResMdl = data;
	for (int i = 0; i < (int)std::size(m_vecFace); i++) m_vecFace[i] = faces[i];
	m_AABB = aabb;
	m_bAABB = true;
	SearchHangEdge();
}

cFaceList::~cFaceList()
{
	Term();
}

void cFaceList::Init()
{
	m_bAABB = false;
	for (int i = 0; i < (int)std::size(m_vecFace); i++)
	{
		m_vecFace[i].clear();
	}
}

void cFaceList::Term()
{
}

// Native version (the Unity original, kept in old/BACKUP_unity_headers/cFaceList.cpp.orig, read SkinnedMeshRenderer meshes of the imported model): the "parts"
// are the model's shapes in shape order with the name of the material they draw; vertices are the model's own (the C# mirrored X on import and mirrored it
// back with GetUnityPos, so the raw Wii data is what it ends up with, winding included).
void cFaceList::RegistFace(ResMdl* resMdl)
{
	m_ResMdl = resMdl;
	// DOL 0x800b8860: a shape is classified by the NAME OF ITS POSITION ARRAY (e.g. "jump_10_w1__w", "dash_saka1_w__w13"), not by its material: the hit models use
	// generic material names ("w", "w13"), so the material-name rule of the C# port dropped those jump / dash faces.
	struct Part { std::string mat; const wii::Mdl0Mesh* mesh; };
	std::vector<Part> parts;
	std::vector<int> list4;   // parts whose material is not norm / wall / came (their node supplies the dash direction)
	if (resMdl->Mdl != nullptr)
	{
		std::vector<const wii::Mdl0Draw*> draws;
		for (const auto& d : resMdl->Mdl->draws) draws.push_back(&d);
		std::stable_sort(draws.begin(), draws.end(), [](const wii::Mdl0Draw* a, const wii::Mdl0Draw* b) { return a->shape < b->shape; });
		for (const wii::Mdl0Draw* d : draws)
		{
			const wii::Mdl0Mesh* mesh = resMdl->Mdl->MeshByShapeId(d->shape);
			const wii::Mdl0Material* mat = resMdl->Mdl->MaterialById(d->material);
			if (mesh == nullptr || mat == nullptr) continue;
			parts.push_back({mesh->posName, mesh});
			if (!(mesh->posName == "norm") && !(mesh->posName == "wall") && !(mesh->posName == "came")) list4.push_back((int)parts.size() - 1);
		}
	}
	Vector3 lo(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector3 hi(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	for (const Part& part : parts)
		for (const auto& v : part.mesh->verts)
		{
			lo = Vector3(std::min(lo.x, v.pos[0]), std::min(lo.y, v.pos[1]), std::min(lo.z, v.pos[2]));
			hi = Vector3(std::max(hi.x, v.pos[0]), std::max(hi.y, v.pos[1]), std::max(hi.z, v.pos[2]));
		}
	Bounds bounds2;
	if (!parts.empty()) bounds2.SetMinMax(lo, hi);
	m_AABB = AABB(bounds2);
	m_bAABB = true;
	int num2 = 0;
	bool array[4] = {true, true, true, true};
	for (int i = 0; i < (int)parts.size(); i++)
	{
		const wii::Mdl0Mesh* mesh = parts[i].mesh;
		string name2 = parts[i].mat;
		Constants::eFACE_KIND faceKind = GetFaceKind(name2);
		if (faceKind == Constants::eFACE_KIND::eFACE_KIND_UNKNOWN)
		{
			continue;
		}
		// DOL 0x800b8860: the node whose name is the LONGEST one contained in the position array name supplies the rotation / translation (and fWork); a node
		// that is not longer than the best so far does not replace it. (The C# looked for nodes called dash / dash1..3 and only for dash faces.)
		string text;
		{
			size_t best = 0;
			if (resMdl->Mdl != nullptr)
				for (const auto& n : resMdl->Mdl->nodes)
					if (name2.find(n.name) != string::npos && n.name.size() > best) { best = n.name.size(); text = n.name; }
		}
		Vector4 v = Vector4::zero;
		Vector4 v2 = Vector4::zero;
		::Constants::jetGeoVecUnit(v);
		::Constants::jetGeoVecUnit(v2);
		Matrix4x4 m;
		::Constants::jetGeoMtxIdentity(m);
		if (!text.empty())
		{
			ResNode* resNode3 = resMdl->GetResNode(text);
			Vector4 translate = resNode3->GetTranslate();
			::Constants::jetGeoVecSet(v2, translate.x, translate.y, translate.z, 1.0f);
			translate = resNode3->GetRotate();
			::Constants::jetGeoVecSet(v, ::Constants::JET_DEG2RAD(translate.x), ::Constants::JET_DEG2RAD(translate.y), ::Constants::JET_DEG2RAD(translate.z), 1.0f);
			::Constants::jetGeoMtxIdentity(m);
			::Constants::jetGeoMtxTrans(m, m, v2);
			::Constants::jetGeoMtxRotZYX(m, m, v);
			v[1] -= ::Constants::JET_DEG2RAD(90.0f);
		}
		const int num4 = (int)mesh->tris.size() / 3;
		for (int k = 0; k < num4; k++)
		{
			Vector4 array3[3] = {};
			for (int l = 0; l < 3; l++)
			{
				const float* p = mesh->verts[mesh->tris[k * 3 + l]].pos;
				::Constants::jetGeoVecSet(array3[l], p[0], p[1], p[2], 1.0f);
			}
			Constants::sFace* sFace = new Constants::sFace();
			sFace->fWork = v[1];
			for (uint num6 = 0u; num6 < 3; num6++)
			{
				sFace->vVrt[num6] = array3[num6];
			}
			Vector4 array4[3] = {};
			::Constants::jetGeoVecSub(array4[0], sFace->vVrt[1], sFace->vVrt[0]);
			::Constants::jetGeoVecSub(array4[1], sFace->vVrt[2], sFace->vVrt[0]);
			::Constants::jetGeoVecOuterProduct(sFace->vNorm, array4[1], array4[0]);
			::Constants::jetGeoVecNormalize(sFace->vNorm, sFace->vNorm);
			sFace->nKind = faceKind;
			sFace->nKindEx = (uint)GetFaceKindEx(name2);
			if (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_WALL || sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_WAXX || sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_BOUN)
			{
				m_vecFace[0].push_back(sFace);
			}
			else if (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_DASH || sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_DALL)
			{
				m_vecFace[2].push_back(sFace);
			}
			else if (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_CAME)
			{
				m_vecFace[3].push_back(sFace);
			}
			else if (sFace->nKind != Constants::eFACE_KIND::eFACE_KIND_WATE)
			{
				m_vecFace[1].push_back(sFace);
			}
			AABB aabb = AABB(Vector3(Mathf::Min(sFace->vVrt[0].x, sFace->vVrt[1].x, sFace->vVrt[2].x), Mathf::Min(sFace->vVrt[0].y, sFace->vVrt[1].y, sFace->vVrt[2].y), Mathf::Min(sFace->vVrt[0].z, sFace->vVrt[1].z, sFace->vVrt[2].z)),
				Vector3(Mathf::Max(sFace->vVrt[0].x, sFace->vVrt[1].x, sFace->vVrt[2].x), Mathf::Max(sFace->vVrt[0].y, sFace->vVrt[1].y, sFace->vVrt[2].y), Mathf::Max(sFace->vVrt[0].z, sFace->vVrt[1].z, sFace->vVrt[2].z)), false);
			sFace->aabb = aabb;
		}
	}
}

void cFaceList::SearchHangEdge()
{
	for (Constants::sFace* item : m_vecFace[1])
	{
		Constants::sFace* faceF = item;
		if (faceF->nKind != Constants::eFACE_KIND::eFACE_KIND_NORM && faceF->nKind != Constants::eFACE_KIND::eFACE_KIND_GRAS)
		{
			continue;
		}
		for (Constants::sFace* item2 : m_vecFace[0])
		{
			Constants::sFace* faceW = item2;
			if (faceW->nKind != Constants::eFACE_KIND::eFACE_KIND_WALL)
			{
				continue;
			}
			uint num = 0u;
			uint num2 = 0u;
			for (uint num3 = 0u; num3 < 3; num3++)
			{
				for (uint num4 = 0u; num4 < 3; num4++)
				{
					if (faceF->vVrt[num3][0] == faceW->vVrt[num4][0] && faceF->vVrt[num3][1] == faceW->vVrt[num4][1] && faceF->vVrt[num3][2] == faceW->vVrt[num4][2])
					{
						num2++;
						num |= (uint)(1 << (int)num3);
						break;
					}
				}
				if (num2 != 2)
				{
					continue;
				}
				switch (num)
				{
				case 3u:
					if (CliffCheck(2u, faceF, faceW))
					{
						faceF->nHangFlag |= 1u;
					}
					break;
				case 6u:
					if (CliffCheck(0u, faceF, faceW))
					{
						faceF->nHangFlag |= 2u;
					}
					break;
				case 5u:
					if (CliffCheck(1u, faceF, faceW))
					{
						faceF->nHangFlag |= 4u;
					}
					break;
				}
				break;
			}
		}
	}
}

bool cFaceList::CliffCheck(const uint& noEdgeIdx, Constants::sFace* const& faceF, Constants::sFace* const& faceW)
{
	Vector4 v;
	::Constants::jetGeoVecSub(v, faceF->vVrt[noEdgeIdx], faceF->vVrt[(noEdgeIdx + 1) % 3]);
	if (Constants::IsVecNormalize(v))
	{
		::Constants::jetGeoVecNormalize(v, v);
		if (::Constants::jetGeoVecInnerProduct(v, faceW->vNorm) < 0.0f)
		{
			return true;
		}
	}
	return false;
}

Constants::eFACE_KIND cFaceList::GetFaceKind(string matName)
{
	CustomKingdomUtil::Assert(!matName.empty(), "nullpo");
	if (CustomKingdomUtil::strstr(matName, "wall"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_WALL;
	}
	if (CustomKingdomUtil::strstr(matName, "waxx"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_WAXX;
	}
	if (CustomKingdomUtil::strstr(matName, "boun"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_BOUN;
	}
	if (CustomKingdomUtil::strstr(matName, "norm"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_NORM;
	}
	if (CustomKingdomUtil::strstr(matName, "goal"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_NORM;
	}
	if (CustomKingdomUtil::strstr(matName, "wate"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_WATE;
	}
	if (CustomKingdomUtil::strstr(matName, "swim"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_SWIM;
	}
	if (CustomKingdomUtil::strstr(matName, "gras"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_GRAS;
	}
	if (CustomKingdomUtil::strstr(matName, "dash"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_DASH;
	}
	if (CustomKingdomUtil::strstr(matName, "dall"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_DALL;
	}
	if (CustomKingdomUtil::strstr(matName, "jump"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_JUMP;
	}
	if (CustomKingdomUtil::strstr(matName, "magm"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_MAGM;
	}
	if (CustomKingdomUtil::strstr(matName, "pois"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_POIS;
	}
	if (CustomKingdomUtil::strstr(matName, "came"))
	{
		return Constants::eFACE_KIND::eFACE_KIND_CAME;
	}
	if (KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		if (CustomKingdomUtil::strstr(matName, "mgal"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G00;
		}
		if (CustomKingdomUtil::strstr(matName, "mral"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G01;
		}
		if (CustomKingdomUtil::strstr(matName, "mpac"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G02;
		}
		if (CustomKingdomUtil::strstr(matName, "mwan"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G03;
		}
		if (CustomKingdomUtil::strstr(matName, "mmot"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G04;
		}
		if (CustomKingdomUtil::strstr(matName, "mold"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G05;
		}
		if (CustomKingdomUtil::strstr(matName, "mbac"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G06;
		}
		if (CustomKingdomUtil::strstr(matName, "mgro"))
		{
			return Constants::eFACE_KIND::eFACE_KIND_MENU_G07;
		}
	}
	// TODO(unity): Debug.LogFormat("warning! unknown material name : {0}\n", matName);
	return Constants::eFACE_KIND::eFACE_KIND_UNKNOWN;
}

Constants::eFACE_KIND_EX cFaceList::GetFaceKindEx(string matName)
{
	if (CustomKingdomUtil::strstr(matName, "bo01"))
	{
		return Constants::eFACE_KIND_EX::eFACE_KIND_EX_IRON_BALL_01;
	}
	if (CustomKingdomUtil::strstr(matName, "bo02"))
	{
		return Constants::eFACE_KIND_EX::eFACE_KIND_EX_IRON_BALL_02;
	}
	if (CustomKingdomUtil::strstr(matName, "di01"))
	{
		return Constants::eFACE_KIND_EX::eFACE_KIND_EX_BOUND_GIMMICK_01;
	}
	if (CustomKingdomUtil::strstr(matName, "di02"))
	{
		return Constants::eFACE_KIND_EX::eFACE_KIND_EX_BOUND_GIMMICK_02;
	}
	return Constants::eFACE_KIND_EX::eFACE_KIND_EX_NONE;
}

}
