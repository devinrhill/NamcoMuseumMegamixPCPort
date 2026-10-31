#include "kingdom/game/g07/cTaskCharEnemyBoss04.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBoss04::cTaskCharEnemyBoss04()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBoss04::Construct()
{
	return new cTaskCharEnemyBoss04();
}

// DOL 0x80052dd8 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyBoss04::Init()
{
	m_bBoss04FirstInit = false;
}

void cTaskCharEnemyBoss04::Term()
{
	cTaskBoss04::Term();
}

uint cTaskCharEnemyBoss04::Input()
{
	return 0u;
}

// DOL 0x80052df0 [audited: per AUDIT_G03_G07 row] (CRC table 0x8036a200 compared)
bool cTaskCharEnemyBoss04::Create(uint no)
{
	cKingdomModel* array[11] = {};
	m_flashMatLists.assign(11, {});
	m_baseMatColorLists.assign(11, {});
	string array2[11][4] = {{"MDL_CHAR_BOSS_4_CENT", "ANM_CHAR_BOSS_4_A00", "", ""}, {"MDL_CHAR_BOSS_4_HASHI", "ANM_CHAR_BOSS_4_A03", "ANM_CHAR_BOSS_4_A02", ""}, {"MDL_CHAR_BOSS_4_HASHI", "ANM_CHAR_BOSS_4_A03", "ANM_CHAR_BOSS_4_A02", ""}, {"MDL_CHAR_BOSS_4_HASHI", "ANM_CHAR_BOSS_4_A03", "ANM_CHAR_BOSS_4_A02", ""}, {"MDL_CHAR_BOSS_4_HASHI", "ANM_CHAR_BOSS_4_A03", "ANM_CHAR_BOSS_4_A02", ""}, {"MDL_CHAR_BOSS_RING", "MDL_CLA_BOSS_RING_ANM", "", ""}, {"MDL_CHAR_BOSS_RING", "MDL_CLA_BOSS_RING_ANM", "", ""}, {"MDL_CHAR_BOSS_RING", "MDL_CLA_BOSS_RING_ANM", "", ""}, {"MDL_CHAR_BOSS_RING", "MDL_CLA_BOSS_RING_ANM", "", ""}, {"MDL_CHAR_BOSS_GLAS", "", "", ""}, {"MDL_CHAR_GHOST_PINKY", "ANM_CHAR_GHORST_A03", "", ""}};
	for (uint num = 0u; num < 11; num++)
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
		if (num == 0 || num == 9 || num == 5 || num == 6 || num == 7 || num == 8 || num == 1 || num == 2 || num == 3 || num == 4)
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
	pModel(1u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(2u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(3u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(4u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(1u)->SetUpdateRateMatClr(1u, 1.0f);
	pModel(2u)->SetUpdateRateMatClr(1u, 1.0f);
	pModel(3u)->SetUpdateRateMatClr(1u, 1.0f);
	pModel(4u)->SetUpdateRateMatClr(1u, 1.0f);
	pModel(5u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(6u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(7u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(8u)->SetUpdateRateMatClr(0u, 0.0f);
	return true;
}

// DOL 0x80053040 [audited: per AUDIT_G03_G07 row] (the C# also zeroed the glass node's x: not in the DOL)
void cTaskCharEnemyBoss04::UpdateMatrix()
{
	if (GetMode() == 1 && !m_bBoss04FirstInit)
	{
		m_bBoss04FirstInit = true;
		::Constants::jetGeoVecUnit(vRotate());
	}
	for (uint num = 0u; num < 11; num++)
	{
		::Constants::jetGeoVecCopy(pModel(num)->vTrans(), vMTrans());
		::Constants::jetGeoVecCopy(pModel(num)->vRotate(), vRotate());
	}
	Vector4 v = Vector4();
	float num2 = fCenterPosY() + 2.0f;
	pModel()->vTrans()[1] = fCenterPosY() + 2.0f;
	pModel()->GetLocalNodeTrans(v, "glass");
	v[1] += num2;
	::Constants::jetGeoVecAdd(pModel(9u)->vTrans(), v, vMTrans());
	pModel()->GetLocalNodeTrans(v, "m07m_05pink01");
	v[1] += num2;
	::Constants::jetGeoVecAdd(pModel(10u)->vTrans(), v, vMTrans());
	uint array[4] = {0u, 1u, 2u, 3u};
	Vector4 v2 = Vector4::zero;
	float num3 = 0.0f;
	for (uint num4 = 0u; num4 < 4; num4++)
	{
		uint idx = 1 + num4;
		uint idx2 = 5 + num4;
		::Constants::jetGeoVecUnit(v2);
		v2[1] = pModel(idx)->vRotate()[1] + ::Constants::JET_DEG2RAD(num3);
		::Constants::jetGeoVecCopy(pModel(idx)->vRotate(), v2);
		v2[1] = pModel(idx2)->vRotate()[1] + ::Constants::JET_DEG2RAD(num3);
		(void)(num4 % 4);
		pModel(idx2)->vTrans()[1] += pRingInfo(array[num4])->m_vRingPos[1];
		::Constants::jetGeoVecCopy(pModel(idx2)->vRotate(), v2);
		num3 += 90.0f;
	}
}

}
