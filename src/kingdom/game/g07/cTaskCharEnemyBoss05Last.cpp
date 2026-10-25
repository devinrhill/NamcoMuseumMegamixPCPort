#include "kingdom/game/g07/cTaskCharEnemyBoss05Last.hpp"

#include <iterator>

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

string cTaskCharEnemyBoss05Last::node_name[4] = {"m07m_04clyd01", "m07m_07inky01", "m07m_06blin01", "m07m_05pink01"};

cTaskCharEnemyBoss05Last::cTaskCharEnemyBoss05Last()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBoss05Last::Construct()
{
	return new cTaskCharEnemyBoss05Last();
}

void cTaskCharEnemyBoss05Last::Init()
{
}

void cTaskCharEnemyBoss05Last::Term()
{
	cTaskBoss05Last::Term();
}

bool cTaskCharEnemyBoss05Last::Create(uint no)
{
	cKingdomModel* array[6] = {};
	m_flashMatLists.assign(6, {});
	m_baseMatColorLists.assign(6, {});
	string array2[6][4] = {{"MDL_CHAR_GHOST_CLYDE", "ANM_CHAR_GHORST_A00", nullptr, nullptr}, {"MDL_CHAR_GHOST_INKY", "ANM_CHAR_GHORST_A01", nullptr, nullptr}, {"MDL_CHAR_GHOST_BLINKY", "ANM_CHAR_GHORST_A02", nullptr, nullptr}, {"MDL_CHAR_GHOST_PINKY", "ANM_CHAR_GHORST_A03", nullptr, nullptr}, {"MDL_CHAR_BOSS_TUNO", nullptr, nullptr, nullptr}, {"MDL_CHAR_BOSS_5_LAST", "ANM_CHAR_BOSS_5_A09", nullptr, nullptr}};
	for (uint num = 0u; num < 6; num++)
	{
		array[num] = CreateModel(num);
		CustomKingdomUtil::Assert(!array2[num][0].empty(), "error");
		array[num]->RegistModel(array2[num][0]);
		int num2 = 4;
		for (int i = 1; i < num2; i++)
		{
			if (!array2[num][i].empty() && !array2[num][i].empty())
			{
				array[num]->RegistAnm(0u, array2[num][i]);
			}
		}
		array[num]->SetLightSetIdx(2);
		array[num]->SetFogIdx(2);
		array[num]->IsDrawShadowDepth(true);
		::Constants::jetGeoVecUnit(array[num]->vRotate());
		array[num]->vScale()[0] = 1.0f;
		array[num]->vScale()[1] = 1.0f;
		array[num]->vScale()[2] = 1.0f;
		array[num]->vScale()[3] = 1.0f;
		if (num == 5 || num == 4)
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
	::Constants::jetGeoVecUnit(vRotate());
	for (uint num3 = 0u; num3 < 6; num3++)
	{
		::Constants::jetGeoVecCopy(pModel(num3)->vTrans(), vMTrans());
		::Constants::jetGeoVecCopy(pModel(num3)->vRotate(), vRotate());
	}
	m_glassNode = pModel(5u)->GetLocalNode("glass");
	for (int k = 0; k < (int)std::size(node_name); k++)
	{
		m_ghostNode[k] = pModel(5u)->GetLocalNode(node_name[k]);
	}
	return true;
}

void cTaskCharEnemyBoss05Last::UpdateMatrix()
{
	if (GetMode() == 1)
	{
		::Constants::jetGeoVecUnit(vRotate());
	}
	for (uint num = 0u; num < 6; num++)
	{
		::Constants::jetGeoVecCopy(pModel(num)->vTrans(), vMTrans());
		::Constants::jetGeoVecCopy(pModel(num)->vRotate(), vRotate());
	}
	Vector4 vector = Vector4();
	// TODO(unity): vector = m_glassNode.localPosition;
	::Constants::jetGeoVecAdd(pModel(4u)->vTrans(), vector, vMTrans());
	for (uint num2 = 0u; (long)num2 < (long)(int)std::size(node_name); num2++)
	{
		uint idx = num2;
		// TODO(unity): vector = m_ghostNode[num2].localPosition;
		::Constants::jetGeoVecRotY(vector, vector, pModel(idx)->vRotate()[1]);
		::Constants::jetGeoVecAdd(pModel(idx)->vTrans(), vector, vMTrans());
	}
}

}
