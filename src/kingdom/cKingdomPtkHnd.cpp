#include "kingdom/cKingdomPtkHnd.hpp"

#include "CustomKingdomUtil.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom
{

cKingdomPtkHnd::cKingdomPtkHnd()
{
}

cKingdomPtkHnd::cKingdomPtkHnd(uint nSlotNo, bool bOnetime, bool bDepend, bool bClipping, float fDelay, Vector4 vTrans, Vector4 vRotate, Vector4 vScale, int nLightIdx, int nFogIdx)
{
	Generate(nSlotNo, bOnetime, bDepend, bClipping, fDelay, vTrans, vRotate, vScale, nLightIdx, nFogIdx);
}

cKingdomPtkHnd::~cKingdomPtkHnd()
{
}

bool cKingdomPtkHnd::Generate(uint nSlotNo, bool bOnetime, bool bDepend, bool bClipping, float fDelay, Nullable<Vector4> vTrans, Nullable<Vector4> vRotate, Nullable<Vector4> vScale, int nLightIdx, int nFogIdx, int drawLayer, float loadFactor)
{
	CustomKingdomUtil::Assert(KingdomGlobal::KingSys->pAppMain() != nullptr, "nullpo");
	CustomKingdomUtil::Assert(KingdomGlobal::KingSys->pAppMain()->pTaskPtkMan() != nullptr, "nullpo");
	if (KingdomGlobal::KingSys->pAppMain()->pTaskPtkMan()->get_KingdomPtkMan()->Generate(this, nSlotNo, bOnetime, bDepend, bClipping, fDelay, nLightIdx, nFogIdx, drawLayer, loadFactor))
	{
		if (vTrans.has_value())
		{
			vSetTrans(vTrans.value());
		}
		if (vRotate.has_value())
		{
			vSetRotate(vRotate.value());
		}
		if (vScale.has_value())
		{
			vSetScale(vScale.value());
		}
		UpdateMatrix();
		return true;
	}
	// TODO(unity): Debug.LogFormat("can not ptk generate !! {0}\n", nSlotNo);
	return false;
}

}
