#include "kingdom/game/g07/cTaskCharEnemyBoss05.hpp"

#include <iterator>

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

Vector4 cTaskCharEnemyBoss05::vRING_POSITION = Vector4(0.0f, 0.75f, 0.0f, 1.0f);
const int cTaskCharEnemyBoss05::datalist[11] = {136, 130, 136, 132, 129, 384, 136, 136, 136, 136, 136};
const string cTaskCharEnemyBoss05::node_name[4] = {"m07m_04clyd01", "m07m_07inky01", "m07m_06blin01", "m07m_05pink01"};

cTaskCharEnemyBoss05::cTaskCharEnemyBoss05()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBoss05::Construct()
{
	return new cTaskCharEnemyBoss05();
}

void cTaskCharEnemyBoss05::Init()
{
}

void cTaskCharEnemyBoss05::Term()
{
	cTaskBoss05::Term();
}

bool cTaskCharEnemyBoss05::Create(uint no)
{
	cKingdomModel* array[11] = {};
	m_flashMatLists.assign(11, {});
	m_baseMatColorLists.assign(11, {});
	string array2[11][4] = {{"MDL_CHAR_BOSS_5_CENT", nullptr, nullptr, nullptr}, {"MDL_CHAR_BOSS_5_DDAI", nullptr, nullptr, nullptr}, {"MDL_CHAR_BOSS_5_BODY", "ANM_CHAR_BOSS_5_A00", "ANM_CHAR_BOSS_5_A03", nullptr}, {"MDL_CHAR_BOSS_5_ZBOU", nullptr, nullptr, nullptr}, {"MDL_CHAR_BOSS_5_XBOU", nullptr, nullptr, nullptr}, {"MDL_CHAR_W5BOSS_RING", "MDL_CLA_BOSS_RING_ANM", nullptr, nullptr}, {"MDL_CHAR_BOSS_TUNO", nullptr, nullptr, nullptr}, {"MDL_CHAR_GHOST_CLYDE", "ANM_CHAR_GHORST_A00", nullptr, nullptr}, {"MDL_CHAR_GHOST_INKY", "ANM_CHAR_GHORST_A01", nullptr, nullptr}, {"MDL_CHAR_GHOST_BLINKY", "ANM_CHAR_GHORST_A02", nullptr, nullptr}, {"MDL_CHAR_GHOST_PINKY", "ANM_CHAR_GHORST_A03", nullptr, nullptr}};
	for (uint num = 0u; num < 11; num++)
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
		std::vector<Material*> allMaterial; // TODO(unity): allMaterial = array[num].pScnMdl().GetAllMaterial()
		m_baseMatColorLists[num].clear();
		m_flashMatLists[num].clear();
		for (int j = 0; j < (int)allMaterial.size(); j++)
		{
			// TODO(unity): m_baseMatColorLists[num].Add(allMaterial[j].GetColor(cTaskCharBoss.scEmissionColorPropertyID));
			m_flashMatLists[num].push_back(allMaterial[j]);
		}
	}
	for (uint num3 = 0u; num3 < 11; num3++)
	{
		::Constants::jetGeoVecCopy(pModel(num3)->vTrans(), vMTrans());
		::Constants::jetGeoVecCopy(pModel(num3)->vRotate(), vRotate());
	}
	::Constants::jetGeoVecUnit(pModel()->vTrans());
	pModel()->vTrans()[1] = 60.0f;
	pModel(5u)->SetUpdateRateMatClr(0u, 0.0f);
	pModel(2u)->SetUpdateRateAnmChr(0u, 0.0f);
	m_glassNode = pModel()->GetLocalNode("glass");
	for (int k = 0; k < (int)std::size(node_name); k++)
	{
		m_ghostNode[k] = pModel()->GetLocalNode(node_name[k]);
	}
	return true;
}

void cTaskCharEnemyBoss05::UpdateMatrix()
{
	if (GetMode() == 1)
	{
		ApearUpdateMatrix();
	}
	else
	{
		GameMainUpdateMatrix();
	}
}

void cTaskCharEnemyBoss05::ApearUpdateMatrix()
{
	float num = fCenterPosY() + 0.0f;
	for (uint num2 = 0u; num2 < 11; num2++)
	{
		cKingdomModel* cKingdomModel2 = pModel(num2);
		if (CustomKingdomUtil::IF(datalist[num2] & 8))
		{
			::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
			cKingdomModel2->vTrans()[1] += num;
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 1))
		{
			cKingdomModel2->vTrans()[0] = 8.0f;
			cKingdomModel2->vTrans()[2] = vMTrans()[2];
			::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), vRotate());
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 4))
		{
			cKingdomModel2->vTrans()[0] = vMTrans()[0];
			cKingdomModel2->vTrans()[2] = 9.0f;
			::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), vRotate());
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 2))
		{
			::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
			cKingdomModel2->vTrans()[1] = 0.0f;
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 0x100))
		{
			::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
			pModel(5u)->vTrans()[0] += vRING_POSITION[0];
			pModel(5u)->vTrans()[1] = vRING_POSITION[1] + pRingInfo()->m_vRingPos[1];
			pModel(5u)->vTrans()[2] += vRING_POSITION[2];
		}
	}
	Vector4 vector = Vector4();
	// TODO(unity): vector = m_glassNode.localPosition;
	::Constants::jetGeoVecAdd(pModel(6u)->vTrans(), vector, vMTrans());
	for (uint num3 = 0u; (long)num3 < (long)(int)std::size(node_name); num3++)
	{
		uint idx = 7 + num3;
		// TODO(unity): vector = m_ghostNode[num3].localPosition;
		::Constants::jetGeoVecRotY(vector, vector, pModel(idx)->vRotate()[1]);
		::Constants::jetGeoVecAdd(pModel(idx)->vTrans(), vector, vMTrans());
	}
}

void cTaskCharEnemyBoss05::GameMainUpdateMatrix()
{
	float num = fCenterPosY() + 0.0f;
	for (uint num2 = 0u; num2 < 11; num2++)
	{
		cKingdomModel* cKingdomModel2 = pModel(num2);
		if (CustomKingdomUtil::IF(datalist[num2] & 8))
		{
			::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
			cKingdomModel2->vTrans()[1] = num;
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 1))
		{
			cKingdomModel2->vTrans()[0] = 8.0f;
			cKingdomModel2->vTrans()[2] = vMTrans()[2];
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 4))
		{
			cKingdomModel2->vTrans()[0] = vMTrans()[0];
			cKingdomModel2->vTrans()[2] = 9.0f;
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 2))
		{
			cKingdomModel2->vTrans()[0] = vMTrans()[0];
			cKingdomModel2->vTrans()[2] = vMTrans()[2];
		}
		else if (CustomKingdomUtil::IF(datalist[num2] & 0x100))
		{
			::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
			pModel(5u)->vTrans()[0] += vRING_POSITION[0];
			pModel(5u)->vTrans()[1] += vRING_POSITION[1] + pRingInfo()->m_vRingPos[1];
			pModel(5u)->vTrans()[2] += vRING_POSITION[2];
		}
		::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), vRotate());
	}
	float num3 = 2.95f;
	num3 = (0.0f - fCenterPosY()) / num3;
	pModel(2u)->SetFrameAnmChr(0u, num3);
	Vector4 vector = Vector4();
	// TODO(unity): vector = m_glassNode.localPosition;
	::Constants::jetGeoVecAdd(pModel(6u)->vTrans(), vector, vMTrans());
	::Constants::jetGeoVecUnit(pModel(6u)->vRotate());
	pModel(6u)->vTrans()[1] += num;
	for (uint num4 = 0u; (long)num4 < (long)(int)std::size(node_name); num4++)
	{
		uint idx = 7 + num4;
		// TODO(unity): vector = m_ghostNode[num4].localPosition;
		::Constants::jetGeoVecRotY(vector, vector, pModel(idx)->vRotate()[1] + ::Constants::JET_DEG2RAD(180.0f));
		::Constants::jetGeoVecAdd(pModel(idx)->vTrans(), vector, vMTrans());
		::Constants::jetGeoVecUnit(pModel(idx)->vRotate());
		pModel(idx)->vTrans()[1] += num;
	}
}

}
