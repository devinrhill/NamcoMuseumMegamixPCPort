#include "kingdom/game/g03/cTaskWater.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/sSaveDataWater.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ReadFileUtil.hpp>

namespace kingdom::game::g03
{

std::vector<cTaskWater::sWaterArea*>& cTaskWater::GetWaterAreaList()
{
	return m_vecTaskWater;
}

cTaskWater::cTaskWater()
{
	SetName("cTaskWater");
	Init();
}

cTaskWater::~cTaskWater()
{
	Term();
}

// DOL 0x80188ae8 [audited: per AUDIT_G03_G07 row] (BIN_WATER_SET_DAT: 0x40-byte records; position /1000, half extents scale/1000*0.5 (x at +0x18, z at +0x20), force direction (0,0,1,1) turned about Y by +0x28 degrees and scaled by +0x30/1000; stored as 0x30-byte areas)
void cTaskWater::Init()
{
	float num = 1000.0f;
	sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx("BIN_WATER_SET_DAT");
	if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
	{
		std::vector<sSaveDataWater*> records;
		int num3i = 0, num2 = 0;
		ReadFileUtil::Convert<sSaveDataWater>(sKingdomData2->pBin, records, num3i, num2);
		uint num3 = (uint)num3i;
		std::vector<sSaveDataWater> array;
		for (auto* r : records) array.push_back(*r);
		for (uint num5 = 0u; num5 < num3; num5++)
		{
			cTaskWater::sWaterArea* sWaterArea2 = new cTaskWater::sWaterArea();
			sWaterArea2->vTrans[0] = (float)array[num5].vnTrans[0] / num;
			sWaterArea2->vTrans[1] = (float)array[num5].vnTrans[1] / num;
			sWaterArea2->vTrans[2] = (float)array[num5].vnTrans[2] / num;
			sWaterArea2->vTrans[3] = 1.0f;
			sWaterArea2->fLengthX = (float)array[num5].vnScale[0] / num / 2.0f;
			sWaterArea2->fLengthZ = (float)array[num5].vnScale[2] / num / 2.0f;
			::Constants::jetGeoVecUnit(sWaterArea2->vWaterForce);
			sWaterArea2->vWaterForce[2] = 1.0f;
			::Constants::jetGeoVecRotY(sWaterArea2->vWaterForce, sWaterArea2->vWaterForce, ::Constants::JET_DEG2RAD(array[num5].vnRotWater[1]));
			sWaterArea2->fForce = (float)array[num5].nForce / num;
			::Constants::jetGeoVecScale(sWaterArea2->vWaterForce, sWaterArea2->vWaterForce, sWaterArea2->fForce);
			m_vecTaskWater.push_back(sWaterArea2);
		}
	}
}

// DOL 0x80189250 [audited: described by the 43rd-session pass] (blr); dtor 0x80188a38
void cTaskWater::Term()
{
}

// DOL 0x80189254 [audited: per AUDIT_G03_G07 row] (return 1)
bool cTaskWater::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskWater::Draw(int nStatus, cJetTask* pParam)
{
}

// DOL 0x80189260 [audited: per AUDIT_G03_G07 row]
uint cTaskWater::AreaCheck(Vector4 trans, float radius, Vector4& outWaterForce, float& outWaterHeight)
{
	CustomKingdomUtil::Assert("nullpo");
	// DOL 0x80189260: the outputs are written only for the area that matched (the port cleared them first: with no match the DOL leaves the caller's last values)
	for (cTaskWater::sWaterArea* item : m_vecTaskWater)
	{
		if (item->vTrans[0] - item->fLengthX <= trans[0] && trans[0] <= item->vTrans[0] + item->fLengthX && item->vTrans[2] - item->fLengthZ <= trans[2] && trans[2] <= item->vTrans[2] + item->fLengthZ)
		{
			::Constants::jetGeoVecCopy(outWaterForce, item->vWaterForce);
			outWaterHeight = item->vTrans[1];
			if (trans[1] < item->vTrans[1])
			{
				return 2u;
			}
			if (trans[1] <= item->vTrans[1] + radius)
			{
				return 1u;
			}
		}
	}
	return 0u;
}

}
