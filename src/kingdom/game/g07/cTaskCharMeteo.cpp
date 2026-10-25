#include "kingdom/game/g07/cTaskCharMeteo.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharMeteo::cTaskCharMeteo()
{
	Init();
}

Vector4& cTaskCharMeteo::vTarget()
{
	return m_vTarget;
}

float& cTaskCharMeteo::fSpeed()
{
	return m_fSpeed;
}

void cTaskCharMeteo::Init()
{
	::Constants::jetGeoVecUnit(m_vTarget);
	m_fSpeed = 0.0f;
	::Constants::jetGeoMtxIdentity(m_Ptkmtx);
	m_phEffTail = nullptr;
}

void cTaskCharMeteo::Term()
{
	cTaskChar::Term();
}

bool cTaskCharMeteo::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_OTHER_METEOR");
	string array[2] = {"TSA_CHAR_OTHER_METEOR", "CLA_CHAR_OTHER_METEOR"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	cKingdomModel2->IsDrawShadowDepth(true);
	m_phEffTail = Constants::SpawnEffect(37, Constants::VEC3_GET(0.0f, 0.0f, 0.0f), nullptr, 0, false);
	CustomKingdomUtil::Assert(m_phEffTail != nullptr, "error");
	return true;
}

void cTaskCharMeteo::UpdateMatrix()
{
	Vector4 _discard1;
	::Constants::jetGeoVecNormalize(_discard1, vSpeed());
	cKingdomModel* cKingdomModel2;
	if ((cKingdomModel2 = pModel()) != nullptr)
	{
		::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
		::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), vRotate());
	}
}

bool cTaskCharMeteo::Execute(cJetTask* pParam)
{
	ActionLocal();
	Move();
	if (!KingdomGlobal::g_pMotosGame->IsPause())
	{
		UpdateFrame();
	}
	return true;
}

bool cTaskCharMeteo::Move()
{
	if (KingdomGlobal::g_pMotosGame->IsPause())
	{
		return true;
	}
	::Constants::jetGeoVecCopy(vMTransOLD(), vMTrans());
	Constants::VEC3_ADD(vMTrans(), vMTrans(), vSpeed());
	bMtxUsed() = true;
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	Matrix4x4 m2;
	::Constants::jetGeoMtxIdentity(m2);
	Matrix4x4 m3;
	::Constants::jetGeoMtxIdentity(m3);
	::Constants::jetGeoMtxTrans(m3, m3, vMTrans());
	if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(vSpeed())))
	{
		Vector4 v;
		::Constants::jetGeoVecNormalize(v, vSpeed());
		float r = Mathf::Acos(::Constants::jetGeoVecInnerProduct(Constants::VEC3_GET(0.0f, -1.0f, 0.0f), v));
		Vector4 v2;
		::Constants::jetGeoVecOuterProduct(v2, Constants::VEC3_GET(0.0f, -1.0f, 0.0f), v);
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v2)))
		{
			::Constants::jetGeoVecNormalize(v2, v2);
		}
		::Constants::jetGeoMtxRotAxis(m, m, v2, r);
		Vector4 v3;
		if (KingdomGlobal::g_pMotosGame->pMap() != nullptr)
		{
			KingdomGlobal::g_pMotosGame->pMap()->Map2World(v3, vMTrans());
		}
		else
		{
			::Constants::jetGeoVecCopy(v3, vMTrans());
		}
		Constants::VEC3_SUB(v3, KingdomGlobal::g_pMotosGame->pCamera()->vPosition(), v3);
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v3)))
		{
			::Constants::jetGeoVecNormalize(v3, v3);
		}
		::Constants::jetGeoVecTrans(v2, m, Constants::VEC3_GET(0.0f, 0.0f, 1.0f));
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v2)))
		{
			::Constants::jetGeoVecNormalize(v2, v2);
		}
		::Constants::jetGeoVecOuterProduct(v3, v, v3);
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v3)))
		{
			::Constants::jetGeoVecNormalize(v3, v3);
		}
		::Constants::jetGeoVecOuterProduct(v3, v3, v);
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v3)))
		{
			::Constants::jetGeoVecNormalize(v3, v3);
		}
		r = Mathf::Acos(::Constants::jetGeoVecInnerProduct(v2, v3));
		if (vSpeed()[0] < 0.0f)
		{
			r = 0.0f - r;
		}
		::Constants::jetGeoMtxRotY(m2, m2, r);
		::Constants::jetGeoMtxMult(m, m, m2);
		::Constants::jetGeoMtxMult(m, m3, m);
		::Constants::jetGeoMtxCopy(pMatrix(), m);
	}
	CustomKingdomUtil::Assert(m_phEffTail != nullptr, "nullpo");
	return true;
}

void cTaskCharMeteo::Draw(int nStatus, cJetTask* pParam)
{
	uint num = 0u;
	while (true)
	{
		cTaskModel::cPBModel* cPBModel = pPBModel(num);
		if (cPBModel != nullptr)
		{
			Matrix4x4 m;
			::Constants::jetGeoMtxIdentity(m);
			::Constants::jetGeoMtxCopy(m, pMatrix());
			Matrix4x4 _discard2;
			::Constants::jetGeoMtx44to34(_discard2, m);
			DummyNW4R::SetMtx(cPBModel->pScnMdl(), m);
			DummyNW4R::SetScale(cPBModel->pScnMdl(), cPBModel->vScale()[0], cPBModel->vScale()[1], cPBModel->vScale()[2]);
			m_Ptkmtx = m;   // Unity: SetTRS(position, rotation, localScale) of the model transform: the matrix with its columns scaled
			for (int c = 0; c < 3; c++)
				for (int r = 0; r < 3; r++) m_Ptkmtx[r][c] *= cPBModel->vScale()[c];
			m_phEffTail->UpdateMatrix(m_Ptkmtx);
			num++;
			continue;
		}
		break;
	}
}

bool cTaskCharMeteo::ActionLocal()
{
	if (vMTrans()[1] < 0.0f && vMTransOLD()[1] >= 0.0f)
	{
		bool flag = true;
		CustomKingdomUtil::Assert(KingdomGlobal::g_pMotosGame != nullptr, "nullpo");
		int num = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->StrikePanel(0, 2, 0.1f, vMTrans());
		cTaskGimmickBase* cTaskGimmickBase2 = KingdomGlobal::g_pMotosGame->pMap()->SearchGimmick(vMTrans()[0], vMTrans()[2]);
		if (cTaskGimmickBase2 != nullptr)
		{
			flag = cTaskGimmickBase2->IsFall(vMTrans());
		}
		if (CustomKingdomUtil::IF(num & 2) || !flag)
		{
			Constants::SpawnEffect(30, vMTrans());
			vMTrans()[1] = 1000.0f;
		}
		else if (CustomKingdomUtil::IF(num & 4))
		{
			Constants::SpawnEffect(29, vMTrans());
		}
	}
	return true;
}

bool cTaskCharMeteo::ActionOther()
{
	return true;
}

}
