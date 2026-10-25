#pragma once

#include "common.hpp"
#include "kingdom/cTaskPBModel.hpp"
#include "kingdom/game/g02/cParamShadow.hpp"

// Rally-X Remix (g02) car shadow: kingdom::game::g02::cRallyModelShadow (0x2b0 bytes; a cTaskPBModel built with the g04 cTaskEnemyShadow
// base ctor 0x8009a730, vtables 0x80393df0 at +0x50 / 0x80393e14 at +0xfc; deleting dtor 0x8007c6d4). The car models FUN_8008120c make one
// per car: CAR_PAC_SH for every colour 1..6, CLR clip SHADOW_COLOR_WORLDn (table 0x80371188 by FUN_800c5ce0) in slot 0, cParamShadow +0x298,
// light vector +0x2a0, connected as a child of the car. Every frame (0x8007c838) it is squashed onto the ground along the light: a planar
// projected shadow placed at the car (y + CSV row 3) and turned like it (Z, Y, X).
namespace kingdom::game::g02
{
class cTaskUnit;

class cRallyModelShadow : public kingdom::cTaskPBModel
{
public:
	explicit cRallyModelShadow(cTaskUnit* pOwner);   // the shadow part of 0x8008120c
	bool Execute(cJetTask* pParam) override;         // 0x8007c838 (vtable +0x50 slot +0x18)
	// 0x80088718 mode 0: the 4x4 projecting along `light` onto the plane through the origin with normal `normal` (row major)
	static void ShadowMatrix(float m[4][4], const float light[3], const float normal[3]);
	int nColour{-1};                                 // +0x290 (the chameleon paint's colour index, -1 off; read by the PB draw: NOT PORTED)

private:
	cTaskUnit* m_pOwner{};                           // the parent node (+0xec, dynamic_cast to cTaskUnit)
	cParamShadow m_Param;                            // +0x298
	float m_vLight[4]{0.0f, 0.0f, 0.0f, 1.0f};       // +0x2a0
};
}
