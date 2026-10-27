#pragma once

#include <vector>

#include "common.hpp"
#include "cJetTask.hpp"

namespace kingdom
{

// DOL kingdom::cAppBaseDebugMenu (RTTI 0x80476818, vtable 0x804383d8; Execute 0x800c2414, Draw 0x800c24f8): a list of {label, job number}
// entries drawn with the engine's debug text. A plain cJetTask in the DOL (RTTI bases cJetNode / cJetTask), NOT a cAppKingdom: it needs no
// cKingdomSys (job 0 runs it before job 128 creates one) and leaves cKingdomSys::pAppMain alone. Fields (offsets from this): +0x34 cursor, +0x38 job under the cursor, +0x3c chosen job
// (-1 none), +0x40 title string, +0x44 entry table, +0x48 entry count.
//   up/down: pad word 2 (auto-repeat bits) bit 0 / bit 1 = KPAD LEFT / RIGHT in the DOL (up / down). This port swaps them (see Execute) so the
//   keyboard Up arrow moves the cursor up with the default Sideways orientation;
//   decide: pad word 1 (trig) bit 0x20 on an entry whose job is not -1 -> Jbc_SetProcNo(job) on the next Execute.
class cAppBaseDebugMenu : public cJetTask
{
public:
	struct Entry
	{
		const char* label;      // DOL pointer in the table at 0x80389868
		uint job;               // 0xffffffff = separator, not selectable
	};

	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

protected:
	void SetMenu(const char* title, const std::vector<Entry>& entries)  // FUN_800c2404
	{
		m_title = title;
		m_entries = entries;
	}

	int m_cursor{0};             // +0x34
	uint m_selected{0xffffffffu};  // +0x38
	uint m_chosen{0xffffffffu};    // +0x3c
	const char* m_title{nullptr};  // +0x40
	std::vector<Entry> m_entries;  // +0x44 / +0x48
};

// DOL kingdom::cAppDebugTopMenu (job 131 = 0x08300000; init 0x8000e1f8, RTTI 0x80476808, vtable 0x803898f0; derived through
// cAppGameDebugMenu 0x80476810 whose Execute 0x800951f0 is the base's). The top menu of the debug build: boot / attract / title, the
// games, the save data tools.
class cAppDebugTopMenu : public cAppBaseDebugMenu
{
public:
	cAppDebugTopMenu();
};

// DOL job 0 (PRJ_JOBNO_DEBUGMENU, init 0x800c25c0, term 0x800c26bc = ReleaseMainTask): a plain cAppBaseDebugMenu (vtable 0x804383d8, task name
// "cJetTask") listing KINGDOM (job 128) and the six developers' lab menus (jobs 16..96, not in the retail job table).
class cAppRootDebugMenu : public cAppBaseDebugMenu
{
public:
	cAppRootDebugMenu();
};

}
