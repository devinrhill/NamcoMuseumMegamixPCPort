#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom { class cTaskModel; }

// Rally-X Remix (g02) the mud on the screen: kingdom::game::g02::cTaskConsMud (0x3bc bytes, vtable 0x80391494, a kingdom::cTaskPBAnita; ctor
// 0x800632ac, Execute 0x80068bac, dtor 0x800689e8; push-back draw = vtable +0x40 0x801235cc). ANITA_MUD (0x6db9f578): anitas 0..0x17 the big
// splats, 0x18..0x1b the small ones, each placed on a random layer of pattern 0 (big: layers 0..24, small: 25..37, shuffled with the console's
// cKingdomRand). While the player's mud timer runs the splats play anim 0/3/6/9 (+0 in, +1 stay, +2 out by kind), then fade out over
// CSV_CAR row 0x19 * 60 frames.
// The port has no cTaskPBAnita (push-back anita drawn inside the 3D scene's pass): this is a cTaskAnita drawn right after the scene
// (priority 0x80000000; the DOL's push-back priority is 0x8000001, UNVERIFIED that the result looks the same: below the HUD, over the 3D view).
namespace kingdom::game::g02
{
class cTaskConsole;

class cTaskConsMud : public kingdom::cTaskAnita
{
public:
	cTaskConsMud(cTaskConsole* pConsole, int nPlayer);         // DOL 0x800632ac
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80068bac
	void Draw(int nStatus, cJetTask* pParam) override;         // DOL 0x801235cc (the push-back draw: only while +0x1d4 is set)
	void SetCar(kingdom::cTaskModel* pCar);                    // the mud half of DOL 0x80065db0

private:
	kingdom::cKingdomAnita* Regist(uint idx, const char* name, int anim);   // DOL 0x80123418 (cTaskPBAnita's: RegistData + SetAnimNo + vTrans 0)
	void SetAnim(uint idx, int anim);                          // DOL 0x801233d8 (SetAnimNo, reset = false)
	bool SetPosLayer(uint idx, int pat, uint lay);             // DOL 0x8012351c
	void HideAll();

public:

	struct sSplat
	{
		int nLayer{};                                          // +0x1ec + i * 0xc (big) / +0x318 + i * 0xc (small)
		u32 nKey{};                                            // +0x1f0 / +0x31c (the random sort key)
		int nKind{};                                           // +0x1f4 / +0x320 (0..3 big, 0..1 small: the clip triple)
	};

	bool m_b1b4{};                                             // +0x1b4 (cTaskPBAnita, 0)
	int m_nPlayer{};                                           // +0x1b8
	// +0x1c0 the anita vector (cTaskAnita::AnitaVector here)
	bool m_bDraw{true};                                        // +0x1d4
	kingdom::cTaskModel* m_pCar{};                             // +0x1d8
	int m_nMode{};                                             // +0x1dc
	uint m_nBig{};                                             // +0x1e0 (the car's param +0x78 = CSV_CAR row 0x1a, at most 24)
	uint m_nSmall{};                                           // +0x1e4 (the car's param +0x7c = CSV_CAR row 0x1b, at most 4)
	uint m_nCarBig{};                                          // (port) CSV_CAR row 0x1a / 0x1b as read by the mud's own param: SetCar uses
	uint m_nCarSmall{};                                        //   them for the car's param +0x78 / +0x7c (the same file; UNVERIFIED equal)
	float m_fMaxFrame{};                                       // +0x1e8 (the longest "out" clip; computed, never read)
	sSplat m_Big[25];                                          // +0x1ec
	sSplat m_Small[13];                                        // +0x318
	// +0x3b4 a cParamCar of its own (0x100 bytes, CSV_CAR read in full by the ctor); the mud reads only these:
	float m_fMudTime{};                                        // cParamCar +0x70 = CSV_CAR row 0x18 * 60, at least 13 (FLOAT_8047a908)
	float m_fFadeTime{};                                       // cParamCar +0x74 = CSV_CAR row 0x19 * 60
	cTaskConsole* m_pConsole{};                                // +0x3b8
};
}
