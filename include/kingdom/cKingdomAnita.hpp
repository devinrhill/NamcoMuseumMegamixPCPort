#pragma once

#include "common.hpp"
#include "cJetAnita.hpp"
#include "jet/sDatLayP.hpp"

namespace kingdom
{

class cKingdomAnita : public cJetAnita
{
private:
	bool m_bHBMUpdate{};

public:
	void Init() override;

	void Term() override;

private:
	bool IsHitPat(int pat, Vector4 trans);

	bool IsHitPatIntepolate(Vector4 trans);

	bool IsHitLayer(jet::sDatLayP* pLay, Vector4 trans);

	int LineSide(Vector4 seg0, Vector4 seg1, Vector4 trans);

	bool GetLayerPosPat(Vector4& pos, uint lay, int pat);

	bool GetLayerPosPatIntepolate(Vector4& pos, uint lay);

public:
	cKingdomAnita();

	virtual ~cKingdomAnita();

	bool RegistData(string fileName, int drawLayer = 5, bool checkLanguage = false);

	bool UpdateFrame();

	bool UpdateFrame(float step);

	bool& IsHBMUpdate();

	static void UpdateMatrix();

	virtual bool SetTexture(int idx, Texture* tex);

	bool GetLayerPos(Vector4& pos, uint lay, int pat = -1);

	// DOL 0x80013dd0: is the point (layout space: x right, y UP, origin screen centre) inside a layer of pattern `pat` (-1 = the pattern(s) the
	// current animation shows)? False for an empty or hidden (state bit 1 clear) layout.
	bool IsHit(Vector4 trans, int pat = -1);
};

}
