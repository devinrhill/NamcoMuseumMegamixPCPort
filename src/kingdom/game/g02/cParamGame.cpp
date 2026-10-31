#include "kingdom/game/g02/cParamGame.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float FPS = 60.0f;   // FLOAT_8047bff0
}

// DOL 0x80123fbc [audited]: cJetCSV ctor (FUN_801393c0), vtable slot +0x08 (clear), data lookup FUN_8000bcd4 -> SetData FUN_80139428, then the rows
// in this order; +0x68 is set whether or not the data was found (the DOL does not test SetData)
cParamGame::cParamGame()
{
	RegistData("CSV_GAME");
	m_nLife = S32(0, 1, 0);
	auto wait = [this](int row) { return FPS * F32(row, 1, 0.0f); };
	m_fMissWait = wait(1);
	m_fClearLastWait = wait(3);
	m_fWinnerLastWait = wait(4);
	m_fDrawLastWait = wait(5);
	m_fLoseLastWait = wait(6);
	m_fClearFlagWait = wait(7);
	m_fWinnerFlagWait = wait(8);
	m_fDrawFlagWait = wait(9);
	m_fLoseFlagWait = wait(10);
	auto color = [this](int row) {
		const u32 r = (u32)S32(row, 1, 0), g = (u32)S32(row, 2, 0), b = (u32)S32(row, 3, 0), a = (u32)S32(row, 4, 0);
		return r << 24 | (g & 0xff) << 16 | (b & 0xff) << 8 | (a & 0xff);
	};
	for (int i = 0; i < 4; i++) m_nFontColor[i] = color(0xb + i);
	for (int i = 0; i < 4; i++) m_nFontShadow[i] = color(0xf + i);
	m_bLoaded = true;
}

}
