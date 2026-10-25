#include "kingdom/game/g07/cTaskCharEnemyBoss02.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBoss02::cTaskCharEnemyBoss02()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBoss02::Construct()
{
	return new cTaskCharEnemyBoss02();
}

void cTaskCharEnemyBoss02::Init()
{
}

void cTaskCharEnemyBoss02::Term()
{
	cTaskBoss02::Term();
}

uint cTaskCharEnemyBoss02::Input()
{
	return 0u;
}

bool cTaskCharEnemyBoss02::Create(uint no)
{
	cKingdomModel* array[7] = {};
	m_flashMatLists.assign(7, {});
	m_baseMatColorLists.assign(7, {});
	string array2[7][4] = {{"MDL_CHAR_BOSS_2_CENT", "ANM_CHAR_BOSS_2_A00", nullptr, nullptr}, {"MDL_CHAR_BOSS_2_HASHI", "ANM_CHAR_BOSS_2_A04", "ANM_CHAR_BOSS_2_A02", nullptr}, {"MDL_CHAR_BOSS_2_HASHI", "ANM_CHAR_BOSS_2_A04", "ANM_CHAR_BOSS_2_A02", nullptr}, {"MDL_CHAR_BOSS_RING", "MDL_CLA_BOSS_RING_ANM", nullptr, nullptr}, {"MDL_CHAR_BOSS_RING", "MDL_CLA_BOSS_RING_ANM", nullptr, nullptr}, {"MDL_CHAR_BOSS_GLAS", nullptr, nullptr, nullptr}, {"MDL_CHAR_GHOST_INKY", "ANM_CHAR_GHORST_A01", nullptr, nullptr}};
	for (uint num = 0u; num < 7; num++)
	{
		array[num] = CreateModel(num);
		CustomKingdomUtil::Assert(!array2[num][0].empty(), "error");
		array[num]->RegistModel(array2[num][0]);
		int num2 = 4;
		uint num3 = 0u;
		for (int i = 1; i < num2; i++)
		{
			if (!array2[num][i].empty())
			{
				SetMortion(num, array2[num][i], num3);
				num3++;
			}
		}
		array[num]->SetLightSetIdx(2);
		array[num]->SetFogIdx(2);
		array[num]->IsDrawShadowDepth(true);
		if (num == 0 || num == 5 || num == 1 || num == 2 || num == 3 || num == 4)
		{
			std::vector<Material*> allMaterial; // TODO(unity): allMaterial = array[num].pScnMdl().GetAllMaterial()
			m_baseMatColorLists[num].clear();
			m_flashMatLists[num].clear();
			for (int j = 0; j < (int)allMaterial.size(); j++)
			{
				// TODO(unity): m_baseMatColorLists[num].Add(allMaterial[j].GetColor(cTaskCharBoss.scEmissionColorPropertyID));
				m_flashMatLists[num].push_back(allMaterial[j]);
			}
		}
	}
	pModel(1u)->SetUpdateRateMatClr(1u, 0.0f);
	pModel(2u)->SetUpdateRateMatClr(1u, 0.0f);
	pModel(3u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(4u)->SetUpdateRateMatClr(0u, 0.0f);
	return true;
}

void cTaskCharEnemyBoss02::UpdateMatrix()
{
	for (uint num = 0u; num < 7; num++)
	{
		::Constants::jetGeoVecCopy(pModel(num)->vTrans(), vMTrans());
		::Constants::jetGeoVecCopy(pModel(num)->vRotate(), vRotate());
	}
	fCenterPosY();
	Vector4 v = Vector4();
	float num2 = fCenterPosY() + 2.0f;
	pModel()->vTrans()[1] = fCenterPosY() + 2.0f;
	pModel()->GetLocalNodeTrans(v, "glass");
	v.x = 0.0f;
	v[1] += num2;
	::Constants::jetGeoVecAdd(pModel(5u)->vTrans(), v, vMTrans());
	pModel()->GetLocalNodeTrans(v, "m07m_07inky01");
	v[1] += num2;
	::Constants::jetGeoVecAdd(pModel(6u)->vTrans(), v, vMTrans());
	Vector4 v2 = Vector4::zero;
	::Constants::jetGeoVecUnit(v2);
	v2[1] = pModel(1u)->vRotate()[1] + ::Constants::JET_DEG2RAD(180.0f);
	::Constants::jetGeoVecCopy(pModel(1u)->vRotate(), v2);
	v2[1] = pModel(3u)->vRotate()[1] + ::Constants::JET_DEG2RAD(180.0f);
	pModel(4u)->vTrans()[1] = pRingInfo(0u)->m_vRingPos[1];
	pModel(3u)->vTrans()[1] = pRingInfo(1u)->m_vRingPos[1];
	::Constants::jetGeoVecCopy(pModel(3u)->vRotate(), v2);
}

}
