#include "kingdom/game/g03/cTaskCameraSet.hpp"
#include <cstdio>
#include <cstdlib>
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include <string>

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ReadFileUtil.hpp>

namespace kingdom::game::g03
{

cTaskCameraSet::cTaskCameraSet()
{
	SetName("cTaskCameraSet");
	Init();
}

cTaskCameraSet::~cTaskCameraSet()
{
	Term();
}

std::vector<sCameraSetData>& cTaskCameraSet::GetCameraSetData()
{
	return m_cameraSetData;
}

void cTaskCameraSet::Init()
{
	m_pLoadData = sSaveDataCameraArea();
	m_bValid = false;
	m_nDataNum = 0u;
	m_nSelectArea = 999;
	m_nSelectIndex = -1;
}

void cTaskCameraSet::Term()
{
}

bool cTaskCameraSet::LoadData(uint world, uint stage)
{
	float num = 1000.0f;
	string pKey = string("set") + std::to_string(world + 1) + std::to_string(stage + 1) + ".cam";
	sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx(pKey);
	if (std::getenv("MEGAMIX_SCENE_DEBUG")) std::fprintf(stderr, "cameraset: %s data %p size %d\n", pKey.c_str(), (void*)sKingdomData2, sKingdomData2 ? (int)sKingdomData2->nSize : -1);
	if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
	{
		std::vector<sSaveDataCameraArea*> outParams;
		int outParamNum;
		int _discard1;
		ReadFileUtil::Convert<sSaveDataCameraArea>(sKingdomData2->pBin, outParams, outParamNum, _discard1);
		for (uint num2 = 0u; (long)num2 < (long)outParamNum; num2++)
		{
			m_nDataNum = (uint)outParamNum;
			m_bValid = true;
			sCameraSetData item = sCameraSetData();
			// the converted object initializer was dropped; ported from the C# in the comment below
			item.vTrans = Vector4((float)outParams[num2]->vnTrans[0] / num, (float)outParams[num2]->vnTrans[1] / num, (float)outParams[num2]->vnTrans[2] / num, 1.0f);
			item.vRotate = Vector4(::Constants::JET_DEG2RAD((float)outParams[num2]->vnRotate[0]), ::Constants::JET_DEG2RAD((float)outParams[num2]->vnRotate[1]), ::Constants::JET_DEG2RAD((float)outParams[num2]->vnRotate[2]), 1.0f);
			item.vScale = Vector4((float)outParams[num2]->vnScale[0] / num, (float)outParams[num2]->vnScale[1] / num, (float)outParams[num2]->vnScale[2] / num, 1.0f);
			item.nAreaNo = (uint)outParams[num2]->nArea;
			// TODO(unity): sCameraSetData item = new sCameraSetData
			// 				{
			// 					vTrans =
			// 					{
			// 						[0] = (float)outParams[num2].vnTrans[0] / num,
			// 						[1] = (float)outParams[num2].vnTrans[1] / num,
			// 						[2] = (float)outParams[num2].vnTrans[2] / num,
			// 						[3] = 1f
			// 					},
			// 					vRotate =
			// 					{
			// 						[0] = global::Constants.JET_DEG2RAD(outParams[num2].vnRotate[0]),
			// 						[1] = global::Constants.JET_DEG2RAD(outParams[num2].vnRotate[1]),
			// 						[2] = global::Constants.JET_DEG2RAD(outParams[num2].vnRotate[2]),
			// 						[3] = 1f
			// 					},
			// 					vScale =
			// 					{
			// 						[0] = (float)outParams[num2].vnScale[0] / num,
			// 						[1] = (float)outParams[num2].vnScale[1] / num,
			// 						[2] = (float)outParams[num2].vnScale[2] / num,
			// 						[3] = 1f
			// 					},
			// 					nAreaNo = (uint)outParams[num2].nArea
			// 				};
			m_cameraSetData.push_back(item);
		}
	}
	return true;
}

bool cTaskCameraSet::Execute(cJetTask* pParam)
{
	if (m_bValid)
	{
		if (m_nSelectIndex != -1 && !AreaCheck(m_cameraSetData[m_nSelectIndex]))
		{
			m_nSelectIndex = -1;
		}
		if (m_nSelectIndex == -1)
		{
			int i;
			for (i = 0; (long)i < (long)m_nDataNum && !AreaCheck(m_cameraSetData[i]); i++)
			{
			}
			int num;
			if ((long)i == (long)m_nDataNum)
			{
				num = 0;
				m_nSelectIndex = -1;
			}
			else
			{
				num = (int)m_cameraSetData[i].nAreaNo;
				m_nSelectIndex = i;
			}
			if (num != m_nSelectArea)
			{
				m_nSelectArea = num;
				Constants::PACN_APP()->pTaskCamera()->ChangeArea(m_nSelectArea);
			}
		}
	}
	else if (m_nSelectArea != 0)
	{
		m_nSelectArea = 0;
		Constants::PACN_APP()->pTaskCamera()->ChangeArea(m_nSelectArea);
	}
	return cJetTask::Execute(pParam);
}

void cTaskCameraSet::Draw(int nState, cJetTask* pParam)
{
}

bool cTaskCameraSet::AreaCheck(sCameraSetData pData)
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v = Vector4();
	Vector4 vector = Vector4();
	Vector4 v2 = Vector4();
	for (int i = 0; i < 4; i++)
	{
		vector[i] = pData.vScale[i];
		v[i] = pData.vRotate[i];
		v2[i] = pData.vTrans[i];
	}
	Vector3 min = Vector3((0.0f - vector[0]) * 0.5f, (0.0f - vector[1]) * 0.5f, (0.0f - vector[2]) * 0.5f);
	Vector3 max = Vector3(vector[0] * 0.5f, vector[1] * 0.5f, vector[2] * 0.5f);
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, v2);
	::Constants::jetGeoMtxRotZYX(m, m, v);
	Matrix4x4 m2;
	::Constants::jetGeoMtx44to34(m2, m);
	AABB target = AABB(min, max, false);
	target.Set(target, m2);
	Vector3 trans = Vector3(cTaskCharPacman2->vTrans()[0], cTaskCharPacman2->vTrans()[1], cTaskCharPacman2->vTrans()[2]);
	float _discard3;
	if (DummyNW4R::DistSqPoint3ToAABB(trans, target, _discard3) != 0.0f)
	{
		return false;
	}
	return true;
}

}
