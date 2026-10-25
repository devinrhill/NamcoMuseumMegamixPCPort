#include "kingdom/game/g07/cMotosMapoo.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g07
{

cMotosMapoo::cMotosMapoo()
{
	CustomKingdomUtil::Assert(RegistObj("MAPOO_OBJ"), "fatal error");
}

cMotosMapoo::~cMotosMapoo()
{
}

bool cMotosMapoo::SearchObj(uint mask, cMotosMapoo::SearchCB func, cJetTask* args, uint limit)
{
	uint num = 0u;
	bool result = false;
	for (uint num2 = 0u; num2 < nObjLayer(); num2++)
	{
		for (uint num3 = 0u; num3 < nObjNum(num2); num3++)
		{
			DObjParam::sObj* sObj = pObj(num2, num3);
			if ((sObj->m_type & mask) != 0)
			{
				result = true;
				if (func != nullptr && !func(sObj, args))
				{
					return result;
				}
				if (++num >= limit && limit != 0)
				{
					return result;
				}
			}
		}
	}
	return result;
}

bool cMotosMapoo::SearchObj(uint mask, cMotosMapoo::SearchCB_2 func, Ref<uint>* args, uint limit)
{
	uint num = 0u;
	bool result = false;
	for (uint num2 = 0u; num2 < nObjLayer(); num2++)
	{
		for (uint num3 = 0u; num3 < nObjNum(num2); num3++)
		{
			DObjParam::sObj* sObj = pObj(num2, num3);
			if ((sObj->m_type & mask) != 0)
			{
				result = true;
				if (func != nullptr && !func(sObj, args))
				{
					return result;
				}
				if (++num >= limit && limit != 0)
				{
					return result;
				}
			}
		}
	}
	return result;
}

int cMotosMapoo::SearchFormNum(int nFormGroup)
{
	int num = 0;
	int num2 = 0;
	for (uint num3 = 0u; num3 < nObjLayer(); num3++)
	{
		for (uint num4 = 0u; num4 < nObjNum(num3); num4++)
		{
			DObjParam::sObj* sObj = pObj(num3, num4);
			num = sObj->m_param[0];
			if ((sObj->m_type & 0x400000) != 0 && num == nFormGroup)
			{
				num2++;
			}
		}
	}
	return num2;
}

}
