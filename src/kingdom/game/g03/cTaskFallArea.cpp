#include "kingdom/game/g03/cTaskFallArea.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/sSaveDataFallArea.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"
#include <ReadFileUtil.hpp>

namespace kingdom::game::g03
{

cTaskFallArea::cTaskFallArea()
{
	SetName("cTaskFallArea");
	Init();
}

cTaskFallArea::~cTaskFallArea()
{
	Term();
}

void cTaskFallArea::Init()
{
	float num = 1000.0f;
	sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx("BIN_FALL_AREA_SET_DAT");
	if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
	{
		std::vector<sSaveDataFallArea*> records;
		int num3i = 0, num2 = 0;
		ReadFileUtil::Convert<sSaveDataFallArea>(sKingdomData2->pBin, records, num3i, num2);
		uint num3 = (uint)num3i;
		std::vector<sSaveDataFallArea> array;
		for (auto* r : records) array.push_back(*r);
		for (uint num5 = 0u; num5 < num3; num5++)
		{
			cTaskFallArea::sFallAreaArea* sFallAreaArea2 = new cTaskFallArea::sFallAreaArea();
			sFallAreaArea2->vTrans[0] = (float)array[num5].vnTrans[0] / num;
			sFallAreaArea2->vTrans[1] = (float)array[num5].vnTrans[1] / num;
			sFallAreaArea2->vTrans[2] = (float)array[num5].vnTrans[2] / num;
			sFallAreaArea2->vTrans[3] = 1.0f;
			sFallAreaArea2->fLengthX = (float)array[num5].vnScale[0] / num / 2.0f;
			sFallAreaArea2->fLengthZ = (float)array[num5].vnScale[2] / num / 2.0f;
			sFallAreaArea2->nAreaCheckKind = array[num5].nAreaCheckKind;
			m_vecTaskFallArea.push_back(sFallAreaArea2);
		}
	}
}

void cTaskFallArea::Term()
{
}

bool cTaskFallArea::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskFallArea::Draw(int nStatus, cJetTask* pParam)
{
}

bool cTaskFallArea::AreaCheck(Vector4 trans, Vector4 last)
{
	CustomKingdomUtil::Assert("nullpo");
	CustomKingdomUtil::Assert("nullpo");
	for (cTaskFallArea::sFallAreaArea* item : m_vecTaskFallArea)
	{
		if (item->nAreaCheckKind == 0)
		{
			if (last[1] > item->vTrans[1] && item->vTrans[0] - item->fLengthX <= trans[0] && trans[0] <= item->vTrans[0] + item->fLengthX && item->vTrans[2] - item->fLengthZ <= trans[2] && trans[2] <= item->vTrans[2] + item->fLengthZ && trans[1] < item->vTrans[1])
			{
				return true;
			}
		}
		else if (item->nAreaCheckKind == 1 && trans[1] < item->vTrans[1])
		{
			return true;
		}
	}
	return false;
}

}
