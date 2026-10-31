#include "kingdom/game/g03/cTaskBgGimmickPillar.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include <cmath>
#include <limits>

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include "platform/devtools.hpp"

namespace kingdom::game::g03
{

cTaskBgGimmickPillar::cTaskBgGimmickPillar(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickPillar");
	Init();
}

void cTaskBgGimmickPillar::Init()
{
	m_fWidth = 2.0f;
	for (uint num = 0u; num < 2; num++)
	{
		::Constants::jetGeoVecUnit(m_vEffectTransL[num]);
	}
	m_nEff = 0u;
	IsDrawShadowDepth(true);
	Constants::PACN_APP();
	ResMdl* resMdl = pFaceList()->pResMdl();
	// DOL 0x801517dc [audited: per AUDIT_G03_G07 row]: every node of the hit model by its index (nw4r ResMdl::GetResNode(i), i < GetResNodeNumEntries). The converted loop only
	// looked at the children of the first child, missed effect1_1 / effect1_2 (x +-5 in m03o_42_wo02_h) and kept the default half width 2:
	// the long w4s2 logs hurt only in their middle. (The port's ResMdl::GetResNode(int) counts root children, so the node table is read here.)
	const int nodeNum = resMdl->Mdl != nullptr ? (int)resMdl->Mdl->nodes.size() : 0;
	for (int num2 = 0; num2 < nodeNum; num2++)
	{
		ResNode node(resMdl->Mdl, resMdl->Mdl->nodes[num2].id);
		ResNode* firstChildChildResNode = &node;
		if (firstChildChildResNode->IsValid() && CustomKingdomUtil::strstr(firstChildChildResNode->GetName(), "effect"))
		{
			Vector3 vector = firstChildChildResNode->GetTranslate();
			::Constants::jetGeoVecSet(m_vEffectTransL[m_nEff], vector.x, vector.y, vector.z, 1.0f);
			if (++m_nEff >= 2)
			{
				break;
			}
		}
	}
	if (m_nEff != 0)
	{
		m_fWidth = ::Constants::fabs(m_vEffectTransL[0][0]);
	}
}

void cTaskBgGimmickPillar::Term()
{
	cTaskBgGimmick::Term();
}

// DOL 0x801519f0 [audited: per AUDIT_G03_G07 row]
bool cTaskBgGimmickPillar::Execute(cJetTask* pParam)
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if (cTaskCharPacman2 == nullptr)
	{
		return true;
	}
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACN_APP()->IsNakajimaPause(this))
	{
		if (!cTaskCharPacman2->IsState(8u))
		{
			IsCollision() = true;
		}
		else
		{
			IsCollision() = false;
		}
		const uint slot = AnmRootSlot() >= 0 ? (uint)AnmRootSlot() : 0u;   // natively the clip may not sit in slot 0 (see AnmRootSlot)
		float frameMaxAnmChr = this->Model->GetFrameMaxAnmChr(slot);
		float num = std::fmod(this->Model->GetFrameAnmChr(slot), frameMaxAnmChr);
		this->Model->UpdateFrame();
		float num2 = std::fmod(this->Model->GetFrameAnmChr(slot), frameMaxAnmChr);
		::Constants::jetGeoMtxCopy(pMtxOld(), pMtx());
		::Constants::jetGeoMtxCopy(pMtxInvOld(), pMtxInv());
		::Constants::jetGeoMtxCopy(pMtxRotateOld(), pMtxRotate());
		::Constants::jetGeoMtxCopy(pMtxRotateInvOld(), pMtxRotateInv());
		::Constants::jetGeoMtxIdentity(pMtxRotate());
		::Constants::jetGeoMtxRotZYX(pMtxRotate(), pMtxRotate(), this->Model->vRotate());
		::Constants::jetGeoMtxIdentity(pMtxRotTrans());
		::Constants::jetGeoMtxTrans(pMtxRotTrans(), pMtxRotTrans(), this->Model->vTrans());
		::Constants::jetGeoMtxMult(pMtxRotTrans(), pMtxRotTrans(), pMtxRotate());
		::Constants::jetGeoMtxScale(pMtx(), pMtxRotTrans(), this->Model->vScale());
		// DOL 0x801519f0: the "root" bone of the clip folds into the hit matrices as in cTaskBgHit::UpdateMatrix (0x80154e64); the converted code had it
		// behind "false /* TODO(unity) */", so the rolling pillar's hit box never left its start (no damage, w4s2)
		ApplyAnmRoot();
		::Constants::jetGeoMtxInverse(pMtxRotateInv(), pMtxRotate());
		::Constants::jetGeoMtxInverse(pMtxInv(), pMtx());
		if (num > num2)
		{
			SetMaterialAlpha(std::numeric_limits<byte>::max());
		}
		// DOL 0x801519f0: the dust effect when the clip frame crosses 60 (old <= 60 <= new); the converted "num2 * 60 >= 60" was Unity seconds
		// (natively it fired at frame 1)
		if (num < num2 && num <= 60.0f && num2 >= 60.0f)
		{
			Vector4 v = Vector4::zero;
			Vector4 v2 = Vector4::zero;
			::Constants::jetGeoVecUnit(v);
			::Constants::jetGeoVecUnit(v2);
			::Constants::jetGeoVecTrans(v, pMtx(), v);
			for (uint num3 = 0u; num3 < m_nEff; num3++)
			{
				::Constants::jetGeoVecAdd(v2, v, m_vEffectTransL[num3]);
				Constants::PACN_APP()->SetEffect(29u, v2, Vector4());
			}
			m_playEffect = true;
		}
		else
		{
			float num4 = frameMaxAnmChr - num2;
			if (num4 <= 60.0f)
			{
				byte materialAlpha = (byte)(num4 * 255.0f / 60.0f);
				SetMaterialAlpha(materialAlpha);
			}
			if (num > num2)
			{
				m_playEffect = false;
			}
		}
		cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
		if (!cTaskCharPacman2->IsState(cTaskCharPacman::STATE_DAMAGE | cTaskCharPacman::STATE_HANG | cTaskCharPacman::STATE_MISS) && HitCheck())
		{
			Vector4 v3;
			::Constants::jetGeoVecTrans(v3, pMtxInv(), cTaskCharPacman2->vTransOld());
			if (::Constants::fabs(v3[0]) >= m_fWidth)
			{
				::Constants::jetGeoVecSet(v3, v3[0], 0.0f, 0.0f, 1.0f);
			}
			else
			{
				::Constants::jetGeoVecSet(v3, 0.0f, 0.0f, v3[2], 1.0f);
			}
			::Constants::jetGeoVecTrans(v3, pMtxRotate(), v3);
			cTaskCharPacman2->DamageProcess(1u, v3, 0.15f);
		}
	}
	return true;
}

void cTaskBgGimmickPillar::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

bool cTaskBgGimmickPillar::HitCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector3 min = Vector3();
	min.x = 0.0f - m_fWidth;
	min.y = -1.0f;
	min.z = -1.0f;
	Vector3 max = Vector3();
	max.x = m_fWidth;
	max.y = 1.0f;
	max.z = 1.0f;
	Matrix4x4 m;
	::Constants::jetGeoMtx44to34(m, pMtx());
	AABB target = AABB(min, max, false);
	target.Set(target, m);
	{ const Vector3 a = target.This.min(), b = target.This.max(); const float mn[3] = {a.x, a.y, a.z}, mx[3] = {b.x, b.y, b.z}; devtools::G03HitBox(mn, mx, 0xffffffffu); } // devtools: --g03-show-gimmick-hit (not 1:1)
	Vector3 trans = Vector3();
	trans.x = cTaskCharPacman2->vTrans()[0];
	trans.y = cTaskCharPacman2->vTrans()[1];
	trans.z = cTaskCharPacman2->vTrans()[2];
	float _discard1;
	if (DummyNW4R::DistSqPoint3ToAABB(trans, target, _discard1) != 0.0f)
	{
		return false;
	}
	return true;
}

}
