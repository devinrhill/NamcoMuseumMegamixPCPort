#pragma once

#include "common.hpp"
#include "DObjParam.hpp"
#include "kingdom/cKingdomMapoo.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cMotosMapoo : public cKingdomMapoo
{
public:
	using SearchCB = std::function<bool(DObjParam::sObj* pObj, cJetTask* pArgs)>;

	using SearchCB_2 = std::function<bool(DObjParam::sObj* pObj, Ref<uint>* pArgs)>;

	cMotosMapoo();

	virtual ~cMotosMapoo();

	bool SearchObj(uint mask, cMotosMapoo::SearchCB func, cJetTask* args = nullptr, uint limit = 0u);

	bool SearchObj(uint mask, cMotosMapoo::SearchCB_2 func, Ref<uint>* args = nullptr, uint limit = 0u);

	int SearchFormNum(int nFormGroup);
};

}
