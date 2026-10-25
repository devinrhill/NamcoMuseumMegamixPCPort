#pragma once

#include "common.hpp"
#include "DObjParam.hpp"

namespace kingdom
{

class cKingdomMapoo
{
private:
	DObjParam* m_Obj{};

public:
	bool RegistObj(string obj);

	uint nObjLayer();

	uint nObjNum(uint layer);

	DObjParam::sObj* pObj(uint layer, uint num);

	DObjParam::sObj* pObj(uint layer, int x, int y);
};

}
