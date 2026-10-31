#include "kingdom/cKingdomAnita.hpp"

#include "wii/Archive.hpp"
#include <cstdio>

#include "CustomKingdomUtil.hpp"
#include "jet/JetUtil.hpp"
#include "jet/uDatLayP.hpp"

namespace kingdom
{

void cKingdomAnita::Init()
{
	cJetAnita::Init();
	m_bHBMUpdate = false;
}

void cKingdomAnita::Term()
{
	cJetAnita::Term();
}

bool cKingdomAnita::IsHitPat(int pat, Vector4 trans)
{
	{
		byte* pAnita = m_pAnita.data();
		{
			void* ptr = pAnita;
			CustomKingdomUtil::Assert(0 <= pat && pat < jet::JetUtil::anpPatCount(ptr), "idx error");
			int num = jet::JetUtil::anpPatLayCount(ptr, pat) - 1;
			if (num < 0)
			{
				return false;
			}
			jet::uDatLayP* ptr2 = jet::JetUtil::anpPatLayPtr(ptr, pat, num);
			while (num > -1)
			{
				if (IsHitLayer((jet::sDatLayP*)ptr2, trans))
				{
					return true;
				}
				num--;
				ptr2--;
			}
		}
	}
	return false;
}

bool cKingdomAnita::IsHitPatIntepolate(Vector4 trans)
{
	{
		byte* pAnita = m_pAnita.data();
		{
			void* ptr = pAnita;
			CustomKingdomUtil::Assert(0 <= m_AnimSeq->pat_noA && m_AnimSeq->pat_noA < jet::JetUtil::anpPatCount(ptr), "idx error");
			CustomKingdomUtil::Assert(0 <= m_AnimSeq->pat_noB && m_AnimSeq->pat_noB < jet::JetUtil::anpPatCount(ptr), "idx error");
			int num = jet::JetUtil::anpPatLayCount(ptr, m_AnimSeq->pat_noA) - 1;
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
				return IsHitPat(m_AnimSeq->pat_noA, trans);
			}
			if (num2 >= 1.0f)
			{
				return IsHitPat(m_AnimSeq->pat_noB, trans);
			}
			jet::uDatLayP* ptr2 = jet::JetUtil::anpPatLayPtr(ptr, m_AnimSeq->pat_noA, num);
			jet::uDatLayP* ptr3 = jet::JetUtil::anpPatLayPtr(ptr, m_AnimSeq->pat_noB, num);
			jet::sDatLayP sDatLayP2 = jet::sDatLayP();
			while (num > -1)
			{
				Interpolate(&sDatLayP2, (jet::sDatLayP*)ptr2, (jet::sDatLayP*)ptr3, num2);
				if (IsHitLayer(&sDatLayP2, trans))
				{
					return true;
				}
				num--;
				ptr2--;
				ptr3--;
			}
		}
	}
	return false;
}

bool cKingdomAnita::IsHitLayer(jet::sDatLayP* pLay, Vector4 trans)
{
	LayerAffine(pLay);
	const Matrix4x4 m = m_userAffineMtx * m_mtx; // as cJetAnita::DrawLayer composes it
	Vector4 vector = Vector4(0.0f - pLay->suh, 0.0f - pLay->svh, 0.0f, 1.0f);
	Vector4 vector2 = Vector4(pLay->suh, pLay->svh, 0.0f, 1.0f);
	vector = m * vector;
	vector2 = m * vector2;
	Vector4 array[2][3] = {{Vector4(vector[0], vector[1], 0.0f, 1.0f), Vector4(vector[0], vector2[1], 0.0f, 1.0f), Vector4(vector2[0], vector[1], 0.0f, 1.0f)}, {Vector4(vector2[0], vector[1], 0.0f, 1.0f), Vector4(vector[0], vector2[1], 0.0f, 1.0f), Vector4(vector2[0], vector2[1], 0.0f, 1.0f)}};
	bool result = false;
	for (uint num = 0u; num < 2; num++)
	{
		if (LineSide(array[num][0], array[num][1], trans) <= 0 && LineSide(array[num][1], array[num][2], trans) <= 0 && LineSide(array[num][2], array[num][0], trans) <= 0)
		{
			result = true;
			break;
		}
	}
	return result;
}

int cKingdomAnita::LineSide(Vector4 seg0, Vector4 seg1, Vector4 trans)
{
	Vector4 vector = seg1 - seg0;
	Vector4 vector2 = trans - seg0;
	return (int)(vector[0] * vector2[1] - vector[1] * vector2[0]);
}

bool cKingdomAnita::GetLayerPosPat(Vector4& pos, uint lay, int pat)
{
	{
		byte* pAnita = m_pAnita.data();
		{
			void* ptr = pAnita;
			CustomKingdomUtil::Assert(0 <= pat && pat < jet::JetUtil::anpPatCount(ptr), "idx error");
			int num = jet::JetUtil::anpPatLayCount(ptr, pat) - 1;
			if (num < 0)
			{
				return false;
			}
			if ((long)lay > (long)num)
			{
				return false;
			}
			jet::uDatLayP* ptr2 = jet::JetUtil::anpPatLayPtr(ptr, pat, (int)lay);
			pos[0] = ((jet::sDatLayP*)ptr2)->px;
			pos[1] = ((jet::sDatLayP*)ptr2)->py;
			pos[2] = 0.0f;
			pos[3] = 1.0f;
		}
	}
	return true;
}

bool cKingdomAnita::GetLayerPosPatIntepolate(Vector4& pos, uint lay)
{
	{
		byte* pAnita = m_pAnita.data();
		{
			void* ptr = pAnita;
			CustomKingdomUtil::Assert(0 <= m_AnimSeq->pat_noA && m_AnimSeq->pat_noA < jet::JetUtil::anpPatCount(ptr), "idx error");
			CustomKingdomUtil::Assert(0 <= m_AnimSeq->pat_noB && m_AnimSeq->pat_noB < jet::JetUtil::anpPatCount(ptr), "idx error");
			int num = jet::JetUtil::anpPatLayCount(ptr, m_AnimSeq->pat_noA) - 1;
			if (num < 0)
			{
				return false;
			}
			if ((long)lay > (long)num)
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
				return GetLayerPosPat(pos, lay, m_AnimSeq->pat_noA);
			}
			if (num2 >= 1.0f)
			{
				return GetLayerPosPat(pos, lay, m_AnimSeq->pat_noB);
			}
			jet::sDatLayP* ptr2 = (jet::sDatLayP*)jet::JetUtil::anpPatLayPtr(ptr, m_AnimSeq->pat_noA, (int)lay);
			jet::sDatLayP* ptr3 = (jet::sDatLayP*)jet::JetUtil::anpPatLayPtr(ptr, m_AnimSeq->pat_noB, (int)lay);
			pos[0] = Interpolate(ptr2->px, ptr3->px, num2);
			pos[1] = Interpolate(ptr2->py, ptr3->py, num2);
			pos[2] = 0.0f;
			pos[3] = 1.0f;
		}
	}
	return true;
}

// DOL 0x80013dd0 [audited]: needs data and the visible bit; pat < 0 uses the animation's current pattern (0x80013f70 when interpolating between two
// patterns, else the layers of the pattern at +0x20 / +0x24 of the sequence), otherwise just that pattern (FUN_8001417c per layer).
bool cKingdomAnita::IsHit(Vector4 trans, int pat)
{
	if (m_pAnita.empty() || !IsState(JET_ANITA_STATE_VISIBLE)) return false;
	LayerAffineUser();
	if (pat >= 0) return IsHitPat(pat, trans);
	if (m_AnimSeq == nullptr) return false;
	if (m_AnimSeq->pat_noA == m_AnimSeq->pat_noB) return IsHitPat(m_AnimSeq->pat_noA, trans);
	return IsHitPatIntepolate(trans);
}

cKingdomAnita::cKingdomAnita()
{
	Init();
}

cKingdomAnita::~cKingdomAnita()
{
	Term();
}

bool cKingdomAnita::RegistData(string fileName, int drawLayer, bool checkLanguage)
{
	// The Unity build loaded a TextAsset by name; the Wii build finds the entry in the attached disc archives.
	std::size_t size = 0;
	const std::uint8_t* data = wii::ArchiveStore::Find(fileName, size);
	CustomKingdomUtil::Assert(data != nullptr, "could not load " + fileName);
	if (data == nullptr)
	{
		return false;
	}
	SetDrawLayer(drawLayer);
	return cJetAnita::SetData(std::vector<byte>(data, data + size), checkLanguage);
}

bool cKingdomAnita::UpdateFrame()
{
	return cJetAnita::AnimUpdate();
}

bool cKingdomAnita::UpdateFrame(float step)
{
	return cJetAnita::AnimUpdate(step);
}

bool& cKingdomAnita::IsHBMUpdate()
{
	return m_bHBMUpdate;
}

void cKingdomAnita::UpdateMatrix()
{
}

bool cKingdomAnita::SetTexture(int idx, Texture* tex)
{
	CustomKingdomUtil::Assert(idx >= 0 && idx < 16, "idx error");
	CustomKingdomUtil::Assert(tex != nullptr, "nullpo");
	m_Textures[idx]->pBrres = tex;
	// TODO(unity): m_Textures[idx].Width = (uint)tex.width;
	// TODO(unity): m_Textures[idx].Height = (uint)tex.height;
	return true;
}

bool cKingdomAnita::GetLayerPos(Vector4& pos, uint lay, int pat)
{
	if (m_pAnita.empty())
	{
		return false;
	}
	if (pat < 0)
	{
		if (m_AnimSeq->pat_noA == m_AnimSeq->pat_noB)
		{
			return GetLayerPosPat(pos, lay, m_AnimSeq->pat_noA);
		}
		return GetLayerPosPatIntepolate(pos, lay);
	}
	return GetLayerPosPat(pos, lay, pat);
}

}
