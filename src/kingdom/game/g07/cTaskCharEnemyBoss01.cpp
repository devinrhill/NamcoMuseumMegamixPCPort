#include "kingdom/game/g07/cTaskCharEnemyBoss01.hpp"

#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBoss01::cTaskCharEnemyBoss01()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBoss01::Construct()
{
	return new cTaskCharEnemyBoss01();
}

void cTaskCharEnemyBoss01::Init()
{
}

void cTaskCharEnemyBoss01::Term()
{
	cTaskBoss01::Term();
}

uint cTaskCharEnemyBoss01::Input()
{
	return 0u;
}

bool cTaskCharEnemyBoss01::Create(uint no)
{
	cKingdomModel* array[5] = {};
	m_flashMatLists.assign(5, {});
	m_baseMatColorLists.assign(5, {});
	string array2[5][4] = {{"MDL_CHAR_BOSS_1_CENT1", nullptr, nullptr, nullptr}, {"MDL_CHAR_BOSS_1_CENT2", "ANM_CHAR_BOSS_1_A00", nullptr, nullptr}, {"MDL_CHAR_BOSS_1_BODY", "ANM_CHAR_BOSS_1_A04", nullptr, nullptr}, {"MDL_CHAR_BOSS_GLAS", nullptr, nullptr, nullptr}, {"MDL_CHAR_GHOST_CLYDE", "ANM_CHAR_GHORST_A00", nullptr, nullptr}};
	for (uint num = 0u; num < 5; num++)
	{
		array[num] = CreateModel(num);
		array[num]->RegistModel(array2[num][0]);
		int num2 = 4;
		for (int i = 1; i < num2; i++)
		{
			if (!array2[num][i].empty())
			{
				SetMortion(num, array2[num][i]);
			}
		}
		array[num]->SetLightSetIdx(2);
		array[num]->SetFogIdx(2);
		::Constants::jetGeoVecUnit(array[num]->vRotate());
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
	pModel(2u)->SetUpdateRateMatClr(0u, 0.0f - pModel(2u)->GetUpdateRateMatClr(0u));
	return true;
}

void cTaskCharEnemyBoss01::UpdateMatrix()
{
	for (uint num = 0u; num < 5; num++)
	{
		::Constants::jetGeoVecCopy(pModel(num)->vTrans(), vMTrans());
	}
	pModel(2u)->vRotate()[1] = vRotate()[1];
	pModel(1u)->vRotate()[1] = vRotate()[1];
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	pModel()->GetLocalNodeTrans(v, "glass");
	::Constants::jetGeoVecAdd(v, v, vMTrans());
	::Constants::jetGeoVecCopy(pModel(3u)->vTrans(), v);
	pModel()->GetLocalNodeTrans(v, "m07m_04clyd01");
	::Constants::jetGeoVecAdd(v, v, vMTrans());
	::Constants::jetGeoVecCopy(pModel(4u)->vTrans(), v);
}

}
