#include "kingdom/cAppDebugMenu.hpp"

#include "GamePad.hpp"
#include "WiiPad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include <cstring>

#include "gfx/DebugText.hpp"

namespace kingdom
{

// DOL 0x800c2414
bool cAppBaseDebugMenu::Execute(cJetTask* pParam)
{
	const GamePad::State& pad = GamePad::Get(0); // FUN_801411d8(0): the raw pad block
	const int count = (int)m_entries.size();
	if (m_chosen == 0xffffffffu)
	{
		if (count == 0) return true;
		// DOL: up on game bit 0 (UP, 0x800c2444), down on bit 1 (DOWN, 0x800c2470). With the Sideways orientation the physical Up arrow arrives as another bit,
		// which made the menu feel inverted, so the cursor follows the PHYSICAL arrows (GamePad::PhysicalBit) whatever the orientation: DELIBERATE DEVIATION.
		if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_UP))        // up
			m_cursor = m_cursor == 0 ? count - 1 : m_cursor - 1;
		else if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_DOWN))   // down, wraps to the first entry
			m_cursor = m_cursor >= count - 1 ? 0 : m_cursor + 1;
		else
		{
			m_selected = m_entries[m_cursor].job;           // 0x800c24a0
			if ((pad.trig & GamePad::TRIG_DECIDE) && m_selected != 0xffffffffu) m_chosen = m_selected;
		}
	}
	else
		KingdomGlobal::SysCore->SetJobNo(m_chosen);          // 0x800c24d8: Jbc_SetProcNo(core, chosen)
	return true;
}

// DOL 0x800c24f8: title at (8, 24), entries at x = 32, y = 48 + 20 * i, the cursor ">" at x = 16 on the current row (default colour)
void cAppBaseDebugMenu::Draw(int nStatus, cJetTask* pParam)
{
	if (m_title == nullptr) return;
	dbgtext::Print(8, 0x18, dbgtext::DEFAULT_COLOR, "%s", m_title);
	for (size_t i = 0; i < m_entries.size(); i++) dbgtext::Print(0x20, 0x30 + 0x14 * (int)i, dbgtext::DEFAULT_COLOR, "%s", m_entries[i].label);
	dbgtext::Print(0x10, 0x30 + 0x14 * m_cursor, dbgtext::DEFAULT_COLOR, ">");
}

// DOL 0x8000e1f8: FUN_800c2404(menu, "Kingdom Final Sep 30 2010 21:51:51" (0x80368718), 17, table 0x80389868)
cAppDebugTopMenu::cAppDebugTopMenu()
{
	SetName("Kingdom Debug Menu");
	const uint SEP = 0xffffffffu;
	SetMenu("Kingdom Final Sep 30 2010 21:51:51", {
		{"----", SEP},
		{"BOOT", 0x08400000u},
		{"ATTRACT", 0x08800000u},
		{"TITLE", 0x08900000u},
		{"----", SEP},
		{"G01 : GALAGA", 0x0b000000u},
		{"G02 : RALLY X", 0x0c000000u},
		{"G03 : PACN ROLL", 0x0d000000u},
		{"G04 : WANIx2 PANIC", 0x0e000000u},
		{"G07 : MOTOS", 0x11000000u},
		{"G11 : GROBDA", 0x15000000u},
		{"RSO : ORIGINAL", 0x0ae00000u},
		{"----", SEP},
		{"Load   Save Data", 0x09000000u},
		{"Save   Save Data", 0x09100000u},
		{"Delete Save Data", 0x09200000u},
		{"----", SEP},
	});
}

}
