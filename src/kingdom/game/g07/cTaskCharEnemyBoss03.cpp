#include "kingdom/game/g07/cTaskCharEnemyBoss03.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBoss03::cTaskCharEnemyBoss03()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBoss03::Construct()
{
	return new cTaskCharEnemyBoss03();
}

void cTaskCharEnemyBoss03::Init()
{
}

void cTaskCharEnemyBoss03::Term()
{
	cTaskBoss03::Term();
}

uint cTaskCharEnemyBoss03::Input()
{
	return 0u;
}

bool cTaskCharEnemyBoss03::Create(uint no)
{
	cKingdomModel* array[5] = {};
	m_flashMatLists.assign(5, {});
	m_baseMatColorLists.assign(5, {});
	string array2[5][4] = {{"MDL_CHAR_BOSS_3_CENT1", nullptr, nullptr, nullptr}, {"MDL_CHAR_BOSS_3_CENT2", "ANM_CHAR_BOSS_3_A00", nullptr, nullptr}, {"MDL_CHAR_BOSS_3_BODY", "ANM_CHAR_BOSS_3_A02", nullptr, nullptr}, {"MDL_CHAR_BOSS_GLAS", nullptr, nullptr, nullptr}, {"MDL_CHAR_GHOST_BLINKY", "ANM_CHAR_GHORST_A02", nullptr, nullptr}};
	for (uint num = 0u; num < 5; num++)
	{
		array[num] = CreateModel(num);
		CustomKingdomUtil::Assert(!array2[num][0].empty(), "error");
		array[num]->RegistModel(array2[num][0]);
		int num2 = 4;
		for (int i = 1; i < num2; i++)
		{
			if (!array2[num][i].empty())
			{
				array[num]->RegistAnm(0u, array2[num][i]);
			}
		}
		array[num]->SetLightSetIdx(2);
		array[num]->SetFogIdx(2);
		array[num]->IsDrawShadowDepth(true);
		if (num == 0 || num == 1 || num == 3 || num == 2)
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
	SetMortionReverse(2u, "ANM_CHAR_BOSS_3_A03", 0u, 2u);
	return true;
}

void cTaskCharEnemyBoss03::UpdateMatrix()
{
	for (uint num = 0u; num < 5; num++)
	{
		::Constants::jetGeoVecCopy(pModel(num)->vTrans(), vMTrans());
	}
	pModel(2u)->vRotate()[1] = vRotate()[1];
	pModel(1u)->vRotate()[1] = vRotate()[1];
	Vector4 v = Vector4();
	pModel()->GetLocalNodeTrans(v, "glass");
	::Constants::jetGeoVecAdd(pModel(3u)->vTrans(), v, vMTrans());
	pModel()->GetLocalNodeTrans(v, "m07m_06blin01");
	::Constants::jetGeoVecAdd(pModel(4u)->vTrans(), v, vMTrans());
}

}
