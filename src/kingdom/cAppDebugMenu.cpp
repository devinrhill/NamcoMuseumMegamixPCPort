#include "kingdom/cAppDebugMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

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
		{"BOOT", job::BOOT},
		{"ATTRACT", job::ATTRACT},
		{"TITLE", job::TITLE},
		{"----", SEP},
		{"G01 : GALAGA", job::GALAGA_INIT},
		{"G02 : RALLY X", job::RALLYX_INIT},
		{"G03 : PACN ROLL", job::PACNROLL_INIT},
		{"G04 : WANIx2 PANIC", job::WANIWANI_INIT},
		{"G07 : MOTOS", job::MOTOS_INIT},
		{"G11 : GROBDA", job::GROBDA_INIT},
		{"RSO : ORIGINAL", job::RSO_DEBUG_MENU},
		{"----", SEP},
		{"Load   Save Data", job::DEBUG_LOAD},
		{"Save   Save Data", job::DEBUG_SAVE},
		{"Delete Save Data", job::DEBUG_DELETE},
		{"----", SEP},
	});
}

// DOL 0x800c25c0: new 0x4c, FUN_800c23e0 (cursor 0, selected / chosen -1), FUN_800c2404(menu, "Kingdom Final Sep 30 2010 21:51:53" (0x80370a70),
// 7, table 0x80438418)
cAppRootDebugMenu::cAppRootDebugMenu()
{
	SetName("cJetTask");
	SetMenu("Kingdom Final Sep 30 2010 21:51:53", {
		{"KINGDOM", job::KINGDOM_INIT},
		{"HOSONO     LABO (-A-)", job::MH_DEBUG_MENU},
		{"MOURI      LABO\\(^o^)/", job::YM_DEBUG_MENU},
		{"MIURA      LABO (@-@)", job::SM_DEBUG_MENU},
		{"KANAZAWA   LABO ('w')", job::KZ_DEBUG_MENU},
		{"GOTOU      LABO (/-\\)", job::GT_DEBUG_MENU},
		{"MATSUSHITA LOBO (_ _)", job::JM_DEBUG_MENU},
	});
}

}
