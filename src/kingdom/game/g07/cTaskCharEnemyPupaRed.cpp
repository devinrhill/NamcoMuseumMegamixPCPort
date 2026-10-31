#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaRed.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyPupaRed::cTaskCharEnemyPupaRed()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyPupaRed::Construct()
{
	return new cTaskCharEnemyPupaRed();
}

// DOL 0x80058dc0 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyPupaRed::Init()
{
	nChangeMoveFrame() = 16;
}

void cTaskCharEnemyPupaRed::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x80058dd0 [audited: per AUDIT_G03_G07 row] (audited 44th: matches; rand() % 100 only on change frames: <15 reverse, then <20 stop, <35 slant)
uint cTaskCharEnemyPupaRed::Input()
{
	pParam();
	float num = 1.0f - HereticRate();
	ushort num2 = nInputDir();
	if (bTestFlag(64u))
	{
		return 0u;
	}
	if (bTestFlag(2097152u))
	{
		return 0u;
	}
	if (bTestFlag(512u))
	{
		return 0u;
	}
	Vector4 v = Vector4::zero;
	cTaskCharPlayer* cTaskCharPlayer2 = SearchNearPlayer();
	if (cTaskCharPlayer2 != nullptr)
	{
		::Constants::jetGeoVecCopy(v, cTaskCharPlayer2->vMTrans());
	}
	else
	{
		::Constants::jetGeoVecUnit(v);
		v[0] = 8.0f;
		v[2] = 8.0f;
	}
	float fDx = v[0] - vMTrans()[0];
	float fDz = v[2] - vMTrans()[2];
	uint nRand = 0u;
	uint nRandBase = 0u;
	auto RATE = [&](uint rate2) {   // C# local function
		if ((nRandBase += rate2) <= nRand) return false;
		return true;
	};
	if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
	{
		nRand = (uint)kingdom::MslRandRange(0, 100);
		nRandBase = 0u;
		if (RATE(15u))
		{
			num2 = nReverseDir(nInputDir());
		}
		else
		{
			nRandBase = 0u;
			num2 = nTargetDir(fDx, fDz);
			if (nInputDir() == num2)
			{
				if (RATE(20u))
				{
					num2 = 0;
				}
				else if (RATE(15u))
				{
					num2 = nSlantDir(num2);
				}
			}
			else if (RATE(20u))
			{
				num2 = 0;
			}
		}
	}
	ushort num3 = nFallPassDirRandom(num2, vSpeed(), fAccelSpeed() * num);
	if (num3 != num2)
	{
		num2 = num3;
		::Constants::jetGeoVecUnit(vSpeed());
	}
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, Constants::motosDir2Vector(num2));
	Constants::VEC3_MUL(v2, v2, fAccelSpeed() * num);
	Constants::VEC3_ADD(vSpeed(), vSpeed(), v2);
	AdjustMaxSpeed();
	if (num2 != 0)
	{
		nModelDir() = num2;
	}
	Dir2Rotate(nModelDir());
	return num2;
}

// DOL 0x8005915c [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyPupaRed::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_PUPA_RED");
	string array[3] = {"TSA_CHAR_ENEMY_PUPA_RED", "CLA_CHAR_ENEMY_PUPA_RED", "CHA_CHAR_ENEMY_PUPA_RED"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	cKingdomModel2->IsDrawShadowDepth(true);
	return true;
}

// DOL 0x80059240 [audited: part = p1, group = p2 only when part != 0, vs disassembly]
void cTaskCharEnemyPupaRed::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
