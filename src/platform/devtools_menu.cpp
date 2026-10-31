// The dev window's Menu tab (devui, --devui / F9): shows only while a game's own menu job runs (Galaga, Rally-X, Pac-N-Roll, Pac-Motos, Grobda).
// Unlock stages (in memory; "Write save" stores them where the port saves that game) and start any stage the way each game's debug menu does
// (set the sys' stage, then its GAME / DEBUG_SELECT job at once). NOT part of the 1:1 game.
#include "platform/devtools.hpp"
#include "platform/devui.hpp"
#include "platform/devtools_g07edit.hpp"

#include <cstdio>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/cSaveLoadMan.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"

namespace devtools
{
namespace
{
using namespace kingdom;

// one game's hooks: its stages (list order), whether each is open, how to open it / all of them, how to start one
struct Game
{
	uint menuJob;
	const char* name;
	bool (*ready)();                          // the game's sys exists
	int (*count)();
	std::string (*label)(int i);
	bool (*isOpen)(int i);                    // nullptr = the game has no locked stages
	void (*setOpen)(int i, bool open);
	void (*openAll)();
	void (*play)(int i);
	void (*save)();                           // nullptr = the port keeps this game's save in memory only
};

std::string Fmt(const char* f, int a, int b = 0)
{
	char buf[48];
	std::snprintf(buf, sizeof buf, f, a, b);
	return buf;
}

void Start(uint job) { Kingdom_SetJobNo(job, false, false); }   // at once, like the debug menus (SysCore->SetJobNo)

// Galaga: stages 1..10 (10 = the world 3 boss), cRecordMan stage records (+0x24 bOpen); AllOpen = the debug menu's save (DOL 0x800b0074 [audited: AllOpen via dev tool])
using game::g01::cG01GalagaSys;
cG01GalagaSys* Galaga() { return KingdomGlobal::GalagaSys; }

// Rally-X: stage index 1..28 = world * 6 + stage + 1 (6th = the boss stage), the record's open word (cTaskConsMenuConsoles 0x8010b050)
using game::g02::cG02RallyXRecord;
constexpr int RALLYX_STAGES = 28;

// Pac-N-Roll: rounds 0..25, the save's per-round bFree (cSaveLoadMan::ForceAllClear opens 0..24)
using game::g03::cG03PacnRollSys;
constexpr int PACNROLL_ROUNDS = 26;
SSaveDataPacnRoll::sStageRecord PacnRollRecord(int r)
{
	SSaveDataPacnRoll::sStageRecord rec{};
	KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(rec, (uint)r);
	return rec;
}

// Pac-Motos: stage index 1..35 (the vs stages 36.. are multiplayer only), pScoreData(idx).bOpen
using game::g07::cG07MotosSys;
constexpr int MOTOS_STAGES = 35;

const Game kGames[] = {
	{job::GALAGA_MENU, "Galaga", [] { return Galaga() != nullptr; }, [] { return 10; },
	 [](int i) { const int s = i + 1; return s == 10 ? std::string("World 3 boss") : Fmt("World %d stage %d", (s - 1) / 3 + 1, (s - 1) % 3 + 1); },
	 [](int i) { return Galaga()->pStageRecord(i + 1)->bOpen != 0; },
	 [](int i, bool open) { Galaga()->pStageRecord(i + 1)->bOpen = open ? 1 : 0; },
	 [] { Galaga()->AllOpen(); },
	 [](int i) { Galaga()->nPlayer(1); Galaga()->nStage(i + 1); Start(job::GALAGA_GAME); },
	 [] { Galaga()->SaveData(); }},
	{job::RALLYX_MENU, "Rally-X", [] { return KingdomGlobal::RallyXSys != nullptr; }, [] { return RALLYX_STAGES; },
	 [](int i) { return i % 6 == 5 ? Fmt("World %d boss", i / 6 + 1) : Fmt("World %d stage %d", i / 6 + 1, i % 6 + 1); },
	 [](int i) { return cG02RallyXRecord::Get().IsOpen(i + 1); },
	 [](int i, bool open) { cG02RallyXRecord::Get().Stage(i + 1).nOpen = open ? 1u : 0u; },
	 [] { for (int i = 1; i <= RALLYX_STAGES; i++) cG02RallyXRecord::Get().SetOpen(i); cG02RallyXRecord::Get().SetAllClear(); },
	 [](int i) { KingdomGlobal::RallyXSys->nStageIdx(i + 1); Start(job::RALLYX_GAME); },
	 [] { cG02RallyXRecord::Get().Save(); }},
	{job::PACNROLL_MENU, "Pac-N-Roll", [] { return KingdomGlobal::PacnRollSys != nullptr; }, [] { return PACNROLL_ROUNDS; },
	 [](int i) {
		 cG03PacnRollSys* sys = KingdomGlobal::PacnRollSys;
		 return Fmt(cG03PacnRollSys::IsBossStage((uint)i) ? "World %d boss" : "World %d stage %d", (int)sys->nWorld((uint)i) + 1, (int)sys->nStage((uint)i) + 1);
	 },
	 [](int i) { return PacnRollRecord(i).bFree; },
	 [](int i, bool open) { SSaveDataPacnRoll::sStageRecord rec = PacnRollRecord(i); rec.bFree = open; KingdomGlobal::PacnRollSys->pSaveLoadMan()->SetStageRecord(rec, (uint)i); },
	 [] { for (int i = 0; i < 25; i++) { SSaveDataPacnRoll::sStageRecord rec = PacnRollRecord(i); rec.bFree = true; KingdomGlobal::PacnRollSys->pSaveLoadMan()->SetStageRecord(rec, (uint)i); } },
	 [](int i) { KingdomGlobal::PacnRollSys->nRound() = (uint)i; Start(job::PACNROLL_DEBUG_SELECT); },
	 [] { KingdomGlobal::PacnRollSys->pSaveLoadMan()->SaveData(); }},
	{job::MOTOS_MENU, "Pac-Motos", [] { return KingdomGlobal::MotosSys != nullptr; }, [] { return MOTOS_STAGES; },
	 [](int i) {
		 cG07MotosSys* sys = KingdomGlobal::MotosSys;
		 const int w = (int)sys->Idx2World((uint)i + 1), st = (int)sys->Idx2Stage((uint)i + 1);
		 return st == 7 ? Fmt("World %d boss", w) : Fmt("World %d stage %d", w, st);   // 7 = the menu's BOSS panel
	 },
	 [](int i) { return KingdomGlobal::MotosSys->IsStageOpen((uint)i + 1); },
	 [](int i, bool open) { KingdomGlobal::MotosSys->SetStageOpen((uint)i + 1, open); },
	 [] { KingdomGlobal::MotosSys->StageAllOpen(); },
	 [](int i) { KingdomGlobal::MotosSys->nStageIdx() = (uint)i + 1; Start(job::MOTOS_DEBUG_SELECT); },
	 [] { KingdomGlobal::MotosSys->SetTitleData(); }},
	{job::GROBDA_MENU, "Grobda", [] { return game::g11::cG11GrobdaSys::Instance() != nullptr; }, [] { return 2; },
	 [](int i) { return Fmt("Stage %02d", i); },
	 nullptr, nullptr, nullptr,
	 [](int i) { game::g11::cG11GrobdaSys::Instance()->nStage(i); Start(job::GROBDA_GAME); },
	 nullptr},
};

const Game* Current()
{
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	if (core == nullptr) return nullptr;
	const uint cur = core->GetJobNo();
	for (const Game& g : kGames)
		if (g.menuJob == cur) return &g;
	return nullptr;
}

void MenuPanel(devui::UI& ui)
{
	static int selected = -1;
	static bool reload = true;
	static const Game* last = nullptr;
	const Game* g = Current();
	if (g == nullptr) return;
	if (g != last) selected = -1;
	last = g;
	ui.Heading(g->name);
	if (!g->ready())
	{
		ui.Label("the game's system object does not exist (entered by a job jump?)", &devui::Color(devui::WARN));
		return;
	}
	bool changed = false;
	if (g->isOpen != nullptr)
	{
		if (ui.Button(ui.Row(), "Unlock every stage"))
		{
			g->openAll();
			changed = true;
		}
		reload = ui.Checkbox(ui.Row(), "Reload the menu after a change", reload);
	}
	ui.Space();

	ui.Heading("Stages");
	std::vector<devui::ListRow> rows;
	for (int i = 0; i < g->count(); i++)
	{
		const bool open = g->isOpen == nullptr || g->isOpen(i);
		rows.push_back(devui::ListRow{(g->isOpen ? (open ? "[x] " : "[ ] ") : "") + g->label(i), devui::Color(open ? devui::TEXT : devui::DIM)});
	}
	bool activated = false;
	selected = ui.ListBox(devui::MakeId("menu.stages"), ui.Row(devui::ROW * 12), rows, selected, true, nullptr, false, &activated);
	const bool pick = selected >= 0 && selected < g->count();
	std::vector<devui::Rect> c = devui::UI::Split(ui.Row(), {1.0f, 1.0f});
	if (g->isOpen != nullptr)
	{
		const bool open = pick && g->isOpen(selected);
		if (ui.Button(c[0], open ? "Lock" : "Unlock", pick) && pick)
		{
			g->setOpen(selected, !open);
			changed = true;
		}
	}
	if ((ui.Button(c[1], "Play", pick) || activated) && pick)
	{
		g->play(selected);
		return;
	}
	ui.Label("Play (or a double click) starts the stage at once, like the game's debug menu.", &devui::Color(devui::DIM));
	if (g->menuJob == job::MOTOS_MENU)
	{
		int& level = devtools::G07LevelSetting();
		level = ui.Choice(ui.Row(), devui::MakeId("menu.g07level"), "Difficulty", {"Normal", "Easy", "Hard"}, level);
		ui.Label("Single-player stages started from now on (Play or the game's own menu) use the CSV parameters of that difficulty (the game itself always plays Normal).", &devui::Color(devui::DIM));
	}
	if (g->save != nullptr && ui.Button(ui.Row(), "Write save (keep the unlocks)")) g->save();
	if (g->isOpen != nullptr && g->save == nullptr) ui.Label("this game's save stays in memory in the port: unlocks last until you quit", &devui::Color(devui::DIM));
	if (changed && reload) Start(g->menuJob);   // the menu reads the records when it is built
}
}

void RegisterMenuPanel() { devui::RegisterPanel("Menu", MenuPanel, [] { return Current() != nullptr; }); }
}
