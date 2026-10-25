#pragma once

#include "common.hpp"
#include "cJetPtkMan.hpp"

namespace kingdom
{

class cKingdomPtkMan : public cJetPtkMan
{
public:
	virtual ~cKingdomPtkMan();

	void Create(uint nSlot, uint nEmi, uint nGen, uint nPtc) override;

	void Draw(Camera* camera, Vector4 vCamPos, Vector4 vCamTgt, float fFovy = 0.0f) override;

	void DebugDraw(int x, int y, uint w);

	void RegistData(uint nSlotNo, string pName);

	void RegistData(uint nSlotNo, uint nCRC);

	void UpdateFrame(float fStep = 1.0f);
};

}
