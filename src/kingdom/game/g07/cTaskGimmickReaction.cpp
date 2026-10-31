#include "kingdom/game/g07/cTaskGimmickReaction.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskGimmickReaction::cTaskGimmickReaction()
{
	Init();
}

cTaskGimmickBase* cTaskGimmickReaction::Construct()
{
	return new cTaskGimmickReaction();
}

void cTaskGimmickReaction::Init()
{
}

void cTaskGimmickReaction::Term()
{
	cTaskGimmickBase::Term();
}

// DOL 0x8012ce54 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskGimmickReaction::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_GIMMICK_REACTION");
	string array[4] = {"TSA_GIMMICK_REACTION", "CLA_GIMMICK_REACTION", "CHA_GIMMICK_REACTION", "TPA_GIMMICK_REACTION"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	return true;
}

// DOL 0x8012cf10 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskGimmickReaction::Execute(cJetTask* pParam)
{
	UpdateFrame();
	return true;
}

// DOL 0x8012cf34 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskGimmickReaction::IsAction(cTaskChar* pChar)
{
	if (IsStepOnCnt(pChar))
	{
		if (nHitChar() != pChar->GrobalID() || nHitCounter() != Constants::get_GAME_COUNTER() - 1)
		{
			nHitChar() = pChar->GrobalID();
			nTimer() = 0u;
		}
		else if (++nTimer() == 5)
		{
			return true;
		}
		nHitCounter() = Constants::get_GAME_COUNTER();
	}
	return false;
}

}
