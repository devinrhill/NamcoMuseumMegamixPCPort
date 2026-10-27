// The dev window's Jobs tab (devui, --devui / F9): every job of the master table, filtered by game; Go switches to it at run time. NOT part of the 1:1
// game: the DOL only changes jobs from its own apps (the debug menus, the hub doors).
#include "platform/devtools.hpp"
#include "platform/devui.hpp"
#include "platform/boot.hpp"

#include <cstdio>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace devtools
{
namespace
{
using namespace kingdom;

struct JobRow { uint no; const char* name; const char* game; const char* what; };

// from include/kingdom/KingdomJobTable.hpp (the constants keep it honest: a renamed job breaks the build; add new ones here)
const JobRow kJobs[] = {
	{job::DEBUG_MENU_ROOT, "DEBUG_MENU_ROOT", "Debug", "PRJ_JOBNO_DEBUGMENU (the developers' project menu)"},
	{job::MH_DEBUG_MENU, "MH_DEBUG_MENU", "Debug", "PRJ_JOBNO_MH_DEBUG_MENU (HOSONO LABO)"},
	{job::SM_DEBUG_MENU, "SM_DEBUG_MENU", "Debug", "PRJ_JOBNO_SM_DEBUG_MENU (MIURA LABO)"},
	{job::YM_DEBUG_MENU, "YM_DEBUG_MENU", "Debug", "PRJ_JOBNO_YM_DEBUG_MENU (MOURI LABO)"},
	{job::KZ_DEBUG_MENU, "KZ_DEBUG_MENU", "Debug", "PRJ_JOBNO_KZ_DEBUG_MENU (KANAZAWA LABO)"},
	{job::GT_DEBUG_MENU, "GT_DEBUG_MENU", "Debug", "PRJ_JOBNO_GT_DEBUG_MENU (GOTOU LABO)"},
	{job::JM_DEBUG_MENU, "JM_DEBUG_MENU", "Debug", "PRJ_JOBNO_JM_DEBUG_MENU (MATSUSHITA LOBO)"},
	{job::KINGDOM_INIT, "KINGDOM_INIT", "Kingdom", "creates cKingdomSys, -> MASTER"},
	{job::KINGDOM_TERM, "KINGDOM_TERM", "Kingdom", "Jbc_SetProcNo(0)"},
	{job::MASTER, "MASTER", "Kingdom", "-> BOOT"},
	{job::DEBUG_MENU, "DEBUG_MENU", "Kingdom", "cAppDebugTopMenu"},
	{job::BOOT, "BOOT", "Kingdom", "cAppBoot"},
	{job::LOAD, "LOAD", "Kingdom", "cAppSaveLoad(true): job-change transition with fade"},
	{job::SAVE, "SAVE", "Kingdom", "cAppSaveLoad(false): job-change transition without fade"},
	{job::ATTRACT, "ATTRACT", "Kingdom", "cAppAttract"},
	{job::TITLE, "TITLE", "Kingdom", "cAppTitle"},
	{job::MENU, "MENU", "Kingdom", "the hub (g03::cAppMenu, castle garden)"},
	{job::STAFFROLL, "STAFFROLL", "Kingdom", "the collection's staff roll (cAppG10StaffRoll)"},
	{job::MENU_CLASSIC, "MENU_CLASSIC", "Kingdom", "classic menu (the arcade hall, g03::cAppMenu)"},
	{job::RSO_INIT, "RSO_INIT", "Kingdom", ""},
	{job::RSO_EXEC, "RSO_EXEC", "Kingdom", ""},
	{job::RSO_TERM, "RSO_TERM", "Kingdom", ""},
	{job::DEBUG_LOAD, "DEBUG_LOAD", "Kingdom", ""},
	{job::DEBUG_SAVE, "DEBUG_SAVE", "Kingdom", ""},
	{job::DEBUG_DELETE, "DEBUG_DELETE", "Kingdom", ""},
	{job::DEBUG_WII_MENU, "DEBUG_WII_MENU", "Kingdom", "empty"},
	{job::RSO_DEBUG_MENU, "RSO_DEBUG_MENU", "Kingdom", "cAppRSODebugMenu"},
	{job::DUMMY_INIT, "DUMMY_INIT", "Dummy (g00)", "-> DUMMY_161"},
	{job::DUMMY_161, "DUMMY_161", "Dummy (g00)", ""},
	{job::DUMMY_MENU, "DUMMY_MENU", "Dummy (g00)", "cAppG00DummyMenu"},
	{job::DUMMY_GAME, "DUMMY_GAME", "Dummy (g00)", "cAppG00DummyGame"},
	{job::DUMMY_TERM, "DUMMY_TERM", "Dummy (g00)", ""},
	{job::GALAGA_INIT, "GALAGA_INIT", "Galaga", ""},
	{job::GALAGA_DEBUG_MENU, "GALAGA_DEBUG_MENU", "Galaga", ""},
	{job::GALAGA_MENU, "GALAGA_MENU", "Galaga", ""},
	{job::GALAGA_GAME, "GALAGA_GAME", "Galaga", ""},
	{job::GALAGA_DEBUG_SELECT, "GALAGA_DEBUG_SELECT", "Galaga", "empty"},
	{job::GALAGA_GAMEOVER, "GALAGA_GAMEOVER", "Galaga", ""},
	{job::GALAGA_MATCHING, "GALAGA_MATCHING", "Galaga", ""},
	{job::GALAGA_STAFFROLL, "GALAGA_STAFFROLL", "Galaga", ""},
	{job::GALAGA_NUNCHUK, "GALAGA_NUNCHUK", "Galaga", ""},
	{job::GALAGA_SCREENSHOOT, "GALAGA_SCREENSHOOT", "Galaga", ""},
	{job::GALAGA_SETTING, "GALAGA_SETTING", "Galaga", ""},
	{job::GALAGA_TERM, "GALAGA_TERM", "Galaga", ""},
	{job::RALLYX_INIT, "RALLYX_INIT", "Rally-X", ""},
	{job::RALLYX_DEBUG_MENU, "RALLYX_DEBUG_MENU", "Rally-X", "empty in the DOL (the port has a native menu)"},
	{job::RALLYX_MENU, "RALLYX_MENU", "Rally-X", ""},
	{job::RALLYX_GAME, "RALLYX_GAME", "Rally-X", ""},
	{job::RALLYX_DEBUG_SELECT, "RALLYX_DEBUG_SELECT", "Rally-X", "empty"},
	{job::RALLYX_TITLE, "RALLYX_TITLE", "Rally-X", ""},
	{job::RALLYX_GAMEOVER, "RALLYX_GAMEOVER", "Rally-X", ""},
	{job::RALLYX_MATCHING, "RALLYX_MATCHING", "Rally-X", ""},
	{job::RALLYX_MULTI_MENU, "RALLYX_MULTI_MENU", "Rally-X", ""},
	{job::RALLYX_NUNCHUK, "RALLYX_NUNCHUK", "Rally-X", ""},
	{job::RALLYX_STAFFROLL, "RALLYX_STAFFROLL", "Rally-X", ""},
	{job::RALLYX_TERM, "RALLYX_TERM", "Rally-X", ""},
	{job::PACNROLL_INIT, "PACNROLL_INIT", "Pac-N-Roll", ""},
	{job::PACNROLL_DEBUG_MENU, "PACNROLL_DEBUG_MENU", "Pac-N-Roll", ""},
	{job::PACNROLL_MENU, "PACNROLL_MENU", "Pac-N-Roll", ""},
	{job::PACNROLL_GAME, "PACNROLL_GAME", "Pac-N-Roll", ""},
	{job::PACNROLL_DEBUG_SELECT, "PACNROLL_DEBUG_SELECT", "Pac-N-Roll", "-> PACNROLL_GAME"},
	{job::PACNROLL_GAMEOVER, "PACNROLL_GAMEOVER", "Pac-N-Roll", ""},
	{job::PACNROLL_EDIT, "PACNROLL_EDIT", "Pac-N-Roll", "-> PACNROLL_GAME"},
	{job::PACNROLL_EDIT2, "PACNROLL_EDIT2", "Pac-N-Roll", "-> PACNROLL_GAME"},
	{job::PACNROLL_NUNCHUK, "PACNROLL_NUNCHUK", "Pac-N-Roll", ""},
	{job::PACNROLL_EDIT_ENEMY, "PACNROLL_EDIT_ENEMY", "Pac-N-Roll", "-> PACNROLL_GAME (name from the debug menu row; not in Constants.cs)"},
	{job::PACNROLL_STAFFROLL, "PACNROLL_STAFFROLL", "Pac-N-Roll", ""},
	{job::PACNROLL_TERM, "PACNROLL_TERM", "Pac-N-Roll", ""},
	{job::WANIWANI_INIT, "WANIWANI_INIT", "Gator Panic", ""},
	{job::WANIWANI_DEBUG_MENU, "WANIWANI_DEBUG_MENU", "Gator Panic", ""},
	{job::WANIWANI_GAME, "WANIWANI_GAME", "Gator Panic", ""},
	{job::WANIWANI_MATCHING, "WANIWANI_MATCHING", "Gator Panic", ""},
	{job::WANIWANI_GAME_MULTI, "WANIWANI_GAME_MULTI", "Gator Panic", ""},
	{job::WANIWANI_NUNCHUK, "WANIWANI_NUNCHUK", "Gator Panic", ""},
	{job::WANIWANI_TERM, "WANIWANI_TERM", "Gator Panic", ""},
	{job::MOTOS_INIT, "MOTOS_INIT", "Pac-Motos", "-> MOTOS_MENU"},
	{job::MOTOS_DEBUG_MENU, "MOTOS_DEBUG_MENU", "Pac-Motos", ""},
	{job::MOTOS_MENU, "MOTOS_MENU", "Pac-Motos", ""},
	{job::MOTOS_GAME, "MOTOS_GAME", "Pac-Motos", ""},
	{job::MOTOS_DEBUG_SELECT, "MOTOS_DEBUG_SELECT", "Pac-Motos", "-> MOTOS_GAME"},
	{job::MOTOS_GAMEOVER, "MOTOS_GAMEOVER", "Pac-Motos", ""},
	{job::MOTOS_MATCHING, "MOTOS_MATCHING", "Pac-Motos", ""},
	{job::MOTOS_MULTI_MENU, "MOTOS_MULTI_MENU", "Pac-Motos", ""},
	{job::MOTOS_NUNCHUK, "MOTOS_NUNCHUK", "Pac-Motos", ""},
	{job::MOTOS_STAFFROLL, "MOTOS_STAFFROLL", "Pac-Motos", "(Constants.cs spells it KINGODM_...)"},
	{job::MOTOS_TERM, "MOTOS_TERM", "Pac-Motos", ""},
	{job::GROBDA_INIT, "GROBDA_INIT", "Grobda", ""},
	{job::GROBDA_DEBUG_MENU, "GROBDA_DEBUG_MENU", "Grobda", ""},
	{job::GROBDA_MENU, "GROBDA_MENU", "Grobda", ""},
	{job::GROBDA_GAME, "GROBDA_GAME", "Grobda", ""},
	{job::GROBDA_DEBUG_SELECT, "GROBDA_DEBUG_SELECT", "Grobda", "empty (the debug menu's MODE row)"},
	{job::GROBDA_GAMEOVER, "GROBDA_GAMEOVER", "Grobda", ""},
	{job::GROBDA_MATCHING, "GROBDA_MATCHING", "Grobda", ""},
	{job::GROBDA_NUNCHUK, "GROBDA_NUNCHUK", "Grobda", ""},
	{job::GROBDA_STAFFROLL, "GROBDA_STAFFROLL", "Grobda", ""},
	{job::GROBDA_TERM, "GROBDA_TERM", "Grobda", ""},
};

const JobRow* Find(uint no)
{
	for (const JobRow& j : kJobs)
		if (j.no == no) return &j;
	return nullptr;
}

std::string JobText(uint no)
{
	char buf[96];
	const JobRow* j = Find(no);
	std::snprintf(buf, sizeof buf, "0x%08x  %3u  %s", no, job::Index(no), j ? j->name : "?");
	return buf;
}

// Kingdom_SetJobNo (DOL 0x8000db5c) the three ways the game uses it
void Go(uint no, int how)
{
	if (how == 0 || KingdomGlobal::KingSys == nullptr) Kingdom_SetJobNo(no, false, false);   // at once (SetForceJobNo)
	else Kingdom_SetJobNo(no, true, how == 2);                                               // through SAVE (no fade) / LOAD (fade)
}

void JobsPanel(devui::UI& ui)
{
	static int game = 0, how = 0, selected = -1;
	static bool wiredOnly = true;
	static std::vector<std::string> games;
	if (games.empty())
	{
		games.push_back("All");
		for (const JobRow& j : kJobs)
		{
			bool seen = false;
			for (const std::string& g : games) seen = seen || g == j.game;
			if (!seen) games.push_back(j.game);
		}
	}

	cSysCoreStructure* core = KingdomGlobal::SysCore;
	ui.Heading("Current");
	if (core == nullptr)
	{
		ui.Label("no job system (arcade mode?)", &devui::Color(devui::WARN));
		return;
	}
	const uint cur = core->GetJobNo();
	ui.Label(JobText(cur));
	ui.Label("last  " + JobText(core->GetLastJobNo()), &devui::Color(devui::DIM));
	if (ui.Button(ui.Row(), "Restart this job")) Go(cur, how);
	ui.Space();

	ui.Heading("Switch to");
	game = ui.Choice(ui.Row(), devui::MakeId("jobs.game"), "Game", games, game);
	wiredOnly = ui.Checkbox(ui.Row(), "Only jobs the port has an init for", wiredOnly);
	std::vector<const JobRow*> shown;
	for (const JobRow& j : kJobs)
		if ((game == 0 || games[game] == j.game) && (!wiredOnly || boot::JobAvailable(j.no))) shown.push_back(&j);
	std::vector<devui::ListRow> rows;
	int sel = -1;
	for (size_t i = 0; i < shown.size(); i++)
	{
		const JobRow* j = shown[i];
		devui::Col c = j->no == cur ? devui::GOOD : boot::JobAvailable(j->no) ? devui::TEXT : devui::DIM;
		rows.push_back(devui::ListRow{JobText(j->no), devui::Color(c)});
		if (selected >= 0 && j->no == (uint)selected) sel = (int)i;
	}
	// a click or Up / Down (mouse over the list) only selects; Go or a double click switches
	bool activated = false;
	int picked = ui.ListBox(devui::MakeId("jobs.list"), ui.Row(devui::ROW * 12), rows, sel, true, nullptr, false, &activated);
	if (picked >= 0 && picked < (int)shown.size()) selected = (int)shown[picked]->no;
	const JobRow* pick = selected >= 0 ? Find((uint)selected) : nullptr;
	if (pick != nullptr)
	{
		ui.Label(pick->what[0] ? pick->what : "(no note)", &devui::Color(devui::DIM));
		if (!boot::JobAvailable(pick->no)) ui.Label("not wired in the port: the task tree stays empty", &devui::Color(devui::WARN));
	}
	how = ui.Choice(ui.Row(), devui::MakeId("jobs.how"), "Switch", {"at once", "via SAVE", "via LOAD (fade)"}, how);
	if ((ui.Button(ui.Row(), pick ? "Go to " + std::string(pick->name) : "Go (pick a job)", pick != nullptr) || activated) && pick != nullptr) Go(pick->no, how);
	ui.Label("A jump skips the games' INIT / TERM jobs (like --job): a game entered from another one may miss its system object.",
	         &devui::Color(devui::DIM));
}
}

void RegisterJobsPanel() { devui::RegisterPanel("Jobs", JobsPanel); }
}
