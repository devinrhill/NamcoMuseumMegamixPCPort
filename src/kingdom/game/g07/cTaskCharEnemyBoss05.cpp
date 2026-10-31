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
// DOL 0x8036a440 [audited: per AUDIT_G03_G07 row] (GameMainUpdateMatrix): model 2 is 0x82 here, 0x88 in the appear table 0x8036a3c0
static const int s_datalistMain[11] = {136, 130, 130, 132, 129, 384, 136, 136, 136, 136, 136};
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

// DOL 0x800534e4 [audited: per AUDIT_G03_G07 row] (CRC table 0x8036a310 compared)
bool cTaskCharEnemyBoss05::Create(uint no)
{
	cKingdomModel* array[11] = {};
	m_flashMatLists.assign(11, {});
	m_baseMatColorLists.assign(11, {});
	string array2[11][4] = {{"MDL_CHAR_BOSS_5_CENT", "", "", ""}, {"MDL_CHAR_BOSS_5_DDAI", "", "", ""}, {"MDL_CHAR_BOSS_5_BODY", "ANM_CHAR_BOSS_5_A00", "ANM_CHAR_BOSS_5_A03", ""}, {"MDL_CHAR_BOSS_5_ZBOU", "", "", ""}, {"MDL_CHAR_BOSS_5_XBOU", "", "", ""}, {"MDL_CHAR_W5BOSS_RING", "MDL_CLA_BOSS_RING_ANM", "", ""}, {"MDL_CHAR_BOSS_TUNO", "", "", ""}, {"MDL_CHAR_GHOST_CLYDE", "ANM_CHAR_GHORST_A00", "", ""}, {"MDL_CHAR_GHOST_INKY", "ANM_CHAR_GHORST_A01", "", ""}, {"MDL_CHAR_GHOST_BLINKY", "ANM_CHAR_GHORST_A02", "", ""}, {"MDL_CHAR_GHOST_PINKY", "ANM_CHAR_GHORST_A03", "", ""}};
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

// DOL 0x800536cc [audited: per AUDIT_G03_G07 row] (mode 1 half)
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
	pModel()->GetLocalNodeTrans(vector, "glass"); // DOL FUN_8001dd54 (the Unity transform lookup left (0,0,0))
	::Constants::jetGeoVecAdd(pModel(6u)->vTrans(), vector, vMTrans());
	for (uint num3 = 0u; (long)num3 < (long)(int)std::size(node_name); num3++)
	{
		uint idx = 7 + num3;
		pModel()->GetLocalNodeTrans(vector, node_name[num3]);
		::Constants::jetGeoVecRotY(vector, vector, pModel(idx)->vRotate()[1]);
		::Constants::jetGeoVecAdd(pModel(idx)->vTrans(), vector, vMTrans());
	}
}

// DOL 0x80053b30 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyBoss05::GameMainUpdateMatrix()
{
	float num = fCenterPosY() + 0.0f;
	for (uint num2 = 0u; num2 < 11; num2++)
	{
		cKingdomModel* cKingdomModel2 = pModel(num2);
		if (CustomKingdomUtil::IF(s_datalistMain[num2] & 8))
		{
			::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
			cKingdomModel2->vTrans()[1] = num;
		}
		else if (CustomKingdomUtil::IF(s_datalistMain[num2] & 1))
		{
			cKingdomModel2->vTrans()[0] = 8.0f;
			cKingdomModel2->vTrans()[2] = vMTrans()[2];
		}
		else if (CustomKingdomUtil::IF(s_datalistMain[num2] & 4))
		{
			cKingdomModel2->vTrans()[0] = vMTrans()[0];
			cKingdomModel2->vTrans()[2] = 9.0f;
		}
		else if (CustomKingdomUtil::IF(s_datalistMain[num2] & 2))
		{
			cKingdomModel2->vTrans()[0] = vMTrans()[0];
			cKingdomModel2->vTrans()[2] = vMTrans()[2];
		}
		else if (CustomKingdomUtil::IF(s_datalistMain[num2] & 0x100))
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
	pModel(2u)->SetFrameAnmChr(0u, 120.0f * num3); // DOL 0x80053b30: 120 frames (FLOAT 0x405e0000), the port passed 0..1
	Vector4 vector = Vector4();
	pModel()->GetLocalNodeTrans(vector, "glass"); // DOL FUN_8001dd54 (the Unity transform lookup left (0,0,0))
	::Constants::jetGeoVecAdd(pModel(6u)->vTrans(), vector, vMTrans());
	::Constants::jetGeoVecUnit(pModel(6u)->vRotate());
	pModel(6u)->vTrans()[1] += num;
	for (uint num4 = 0u; (long)num4 < (long)(int)std::size(node_name); num4++)
	{
		uint idx = 7 + num4;
		pModel()->GetLocalNodeTrans(vector, node_name[num4]);
		::Constants::jetGeoVecRotY(vector, vector, pModel(idx)->vRotate()[1] + ::Constants::JET_DEG2RAD(180.0f));
		::Constants::jetGeoVecAdd(pModel(idx)->vTrans(), vector, vMTrans());
		::Constants::jetGeoVecUnit(pModel(idx)->vRotate());
		pModel(idx)->vTrans()[1] += num;
	}
}

}
