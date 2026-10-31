#include "kingdom/game/g03/cTaskBox.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/cTaskBgBox.hpp"
#include "kingdom/game/g03/sSaveDataBox.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ReadFileUtil.hpp>

namespace kingdom::game::g03
{

// DOL 0x801553cc [audited: per AUDIT_G03_G07 row]
cTaskBox::cTaskBox()
{
	SetName("cTaskBox");
	Init();
}

// DOL 0x801555bc [audited: per AUDIT_G03_G07 row] (not a retry: BIN_BOX_SET_DAT 0x30-byte records; a retry: the stored boxes)
void cTaskBox::Init()
{
	if (!KingdomGlobal::PacnRollSys->IsRetry())
	{
		sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx("BIN_BOX_SET_DAT");
		if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
		{
			std::vector<sSaveDataBox*> outParams;
			int outParamNum;
			int _discard1;
			ReadFileUtil::Convert<sSaveDataBox>(sKingdomData2->pBin, outParams, outParamNum, _discard1);
			for (uint num = 0u; (long)num < (long)outParamNum; num++)
			{
				cTaskBgBox* cTaskBgBox2 = new cTaskBgBox();
				CustomKingdomUtil::Assert(cTaskBgBox2 != nullptr, "nullpo");
				cTaskBgBox2->RegistData(*outParams[num]);
				m_vecBox.push_back(cTaskBgBox2);
				ConnectChild(cTaskBgBox2);
			}
		}
		return;
	}
	for (cG03PacnRollSys::sRetryBox* item : KingdomGlobal::PacnRollSys->pRetryData()->vecBox)
	{
		cTaskBgBox* cTaskBgBox3 = new cTaskBgBox();
		CustomKingdomUtil::Assert(cTaskBgBox3 != nullptr, "nullpo");
		cTaskBgBox3->RegistData(item->nKind, item->nTreasure, item->vTrans, item->vRotate, item->bBreak);
		m_vecBox.push_back(cTaskBgBox3);
		ConnectChild(cTaskBgBox3);
	}
}

// dtor 0x80155508
void cTaskBox::Term()
{
	for (cTaskBgBox* item : m_vecBox)
	{
		if (item != nullptr)
		{
			item->TermOnce();
		}
	}
}

// DOL 0x80155ee4 [audited: constant getter vs disassembly] (return 1)
bool cTaskBox::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskBox::Draw(int nStatus, cJetTask* pParam)
{
}

// DOL 0x80155ef0 [audited: per AUDIT_G03_G07 row] (kind, treasure, broken, position, rotation per box)
void cTaskBox::SetRetryData()
{
	for (cTaskBgBox* item : m_vecBox)
	{
		cG03PacnRollSys::sRetryBox* sRetryBox = new cG03PacnRollSys::sRetryBox();
		sRetryBox->nKind = item->nKind();
		sRetryBox->bBreak = item->IsBreak();
		sRetryBox->nTreasure = item->nTreasure();
		::Constants::jetGeoVecCopy(sRetryBox->vTrans, item->Model->vTrans());
		::Constants::jetGeoVecCopy(sRetryBox->vRotate, item->Model->vRotate());
		KingdomGlobal::PacnRollSys->pRetryData()->vecBox.push_back(sRetryBox);
	}
}

}
