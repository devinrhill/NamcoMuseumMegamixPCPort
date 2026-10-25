#pragma once

#include "common.hpp"
#include "cJetPtkHnd.hpp"

namespace kingdom
{

class cKingdomPtkHnd : public cJetPtkHnd
{
public:
	cKingdomPtkHnd();

	cKingdomPtkHnd(uint nSlotNo, bool bOnetime = false, bool bDepend = false, bool bClipping = true, float fDelay = 0.0f, Vector4 vTrans = Vector4(), Vector4 vRotate = Vector4(), Vector4 vScale = Vector4(), int nLightIdx = 0, int nFogIdx = -1);

	virtual ~cKingdomPtkHnd();

	bool Generate(uint nSlotNo, bool bOnetime = false, bool bDepend = false, bool bClipping = true, float fDelay = 0.0f, Nullable<Vector4> vTrans = nullptr, Nullable<Vector4> vRotate = nullptr, Nullable<Vector4> vScale = nullptr, int nLightIdx = 0, int nFogIdx = -1, int drawLayer = 0, float loadFactor = -1.0f);
};

}
