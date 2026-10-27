// The dev window's Save tab (devui, --devui / F9): opens a save folder (museum.bin + banner.bin), shows the records of every
// game as named fields, edits them in memory and writes the result to ANOTHER folder (museum.bin's CRC-32 recomputed, banner.bin copied as is, or
// the game's own banner (cKingdomSave::InitBanner) when the source has none or an empty one, as older port builds wrote).
// The running game keeps its own copy of the save: start the port with MEGAMIX_SAVE=<that folder> to play with the edited one. The files are the
// Wii's own (NAND 00010000/534e4d45/data): copied into a Dolphin user folder's Wii/title/00010000/534e4d45/data they load in the real game.
// Layouts are the port's Load / Save code (each field names the function that reads it). NOT part of the 1:1 game.
#include "platform/devtools.hpp"
#include "platform/devui.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "WiiSystem.hpp"
#include "kingdom/cKingdomSave.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "wii/Archive.hpp"   // Crc32

namespace devtools
{
namespace
{
using u8 = std::uint8_t;
using u32 = std::uint32_t;

// cKingdomSave: museum.bin = [BE CRC-32 of the data][0xe000 data = 7 blocks of 0x2000]; banner.bin 0xf0a0 bytes
constexpr std::size_t BLOCK_SIZE = 0x2000, BLOCKS = 7, DATA_SIZE = BLOCK_SIZE * BLOCKS, MUSEUM_SIZE = DATA_SIZE + 4, BANNER_SIZE = 0xf0a0;

struct State
{
	std::string src, dst, msg;
	bool msgBad = false;
	bool loaded = false;
	std::vector<u8> museum, banner;   // empty = not in the source folder
	u32 crcLoaded = 0;                     // the computed CRC when the file was read
	int page = 0, galagaStage = 1, rallyStage = 1, pnrRound = 0, motosStage = 1, rawBlock = 0, rawOffset = 0;
	bool firstFrame = true;
};
State g;

std::string Trim(std::string s)
{
	while (s.size() > 1 && s.back() == '/') s.pop_back();
	return s;
}

bool ReadFile(const std::string& path, std::vector<u8>& out)
{
	out.clear();
	std::ifstream f(path, std::ios::binary);
	if (!f) return false;
	out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
	return true;
}

bool WriteFile(const std::string& path, const std::vector<u8>& data)
{
	std::ofstream f(path, std::ios::binary | std::ios::trunc);
	if (!f) return false;
	f.write(reinterpret_cast<const char*>(data.data()), (std::streamsize)data.size());
	return (bool)f;
}

u32 GetBe(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
void PutBe(u8* p, u32 v) { p[0] = (u8)(v >> 24); p[1] = (u8)(v >> 16); p[2] = (u8)(v >> 8); p[3] = (u8)v; }

u8* Block(int b) { return g.museum.data() + 4 + (std::size_t)b * BLOCK_SIZE; }
u32 Crc() { return wii::Crc32(g.museum.data() + 4, DATA_SIZE); }

void Say(const std::string& m, bool bad) { g.msg = m; g.msgBad = bad; }

void Open()
{
	g.src = Trim(g.src);
	g.loaded = false;
	std::vector<u8> m;
	if (!ReadFile(g.src + "/museum.bin", m)) { Say("no museum.bin in " + g.src, true); return; }
	if (m.size() != MUSEUM_SIZE) { Say("museum.bin is " + std::to_string(m.size()) + " bytes, expected 57348", true); return; }
	g.museum = std::move(m);
	if (!ReadFile(g.src + "/banner.bin", g.banner) || g.banner.size() != BANNER_SIZE) g.banner.clear();
	g.crcLoaded = Crc();
	g.loaded = true;
	const bool crcOk = GetBe(g.museum.data()) == g.crcLoaded;
	Say(std::string("opened ") + g.src + (crcOk ? "" : " (the stored CRC does not match: the game would call it broken)") +
	        (g.banner.empty() ? "; no banner.bin (Write adds the game's)" : GetBe(g.banner.data()) != 0x5749424e ? "; banner.bin is blank (Write puts the game's in)" : ""),
	    !crcOk);
}

void Write()
{
	namespace fs = std::filesystem;
	g.dst = Trim(g.dst);
	if (g.dst.empty()) { Say("give a folder to write to", true); return; }
	std::error_code ec;
	fs::create_directories(g.dst, ec);
	// writing over the running game's own save folder: it keeps the save it loaded in memory and writes that back on its next save
	const std::string live = Trim(WiiSystem::SaveDir());
	const bool running = fs::exists(live, ec) && fs::equivalent(g.dst, live, ec);
	std::vector<u8> m = g.museum;
	PutBe(m.data(), wii::Crc32(m.data() + 4, DATA_SIZE));
	bool ok = WriteFile(g.dst + "/museum.bin", m);
	std::vector<u8> b = g.banner;
	if ((b.empty() || GetBe(b.data()) != 0x5749424e) && KingdomGlobal::KingSys != nullptr && KingdomGlobal::KingSys->pSave() != nullptr)   // "WIBN"
		b.assign(KingdomGlobal::KingSys->pSave()->Banner(), KingdomGlobal::KingSys->pSave()->Banner() + BANNER_SIZE);
	if (ok && !b.empty()) ok = WriteFile(g.dst + "/banner.bin", b);
	if (!ok) Say("could not write to " + g.dst, true);
	else if (running) Say("wrote the game's own save folder: restart the game to load it (its next save would overwrite these edits)", false);
	else Say("wrote the save (play it with MEGAMIX_SAVE=<that folder>)", false);
}

// ---- fields ---------------------------------------------------------------------------------------------------------------------

using devui::MakeId;
using devui::UI;

// a 32-bit word at p (big-endian unless host); signed words may go negative
void Word(UI& ui, const devui::Rect& r, const char* id, int a, const std::string& label, u8* p, bool sign = false)
{
	const u32 raw = GetBe(p);
	const double v = sign ? (double)(std::int32_t)raw : (double)raw;
	static const double zero = 0.0;
	double n = ui.Number(r, MakeId(id, a), label, v, 1.0, "%.0f", true, sign ? nullptr : &zero);
	if (n == v) return;
	if (sign) n = n < -2147483648.0 ? -2147483648.0 : (n > 2147483647.0 ? 2147483647.0 : n);
	else n = n > 4294967295.0 ? 4294967295.0 : n;
	const u32 w = sign ? (u32)(std::int32_t)n : (u32)n;
	PutBe(p, w);
}

// a word used as a flag (0 / 1); other values show as set and are kept until the box is clicked
void Flag(UI& ui, const devui::Rect& r, const std::string& label, u8* p, u32 bit = 0)
{
	const u32 raw = GetBe(p);
	const bool on = bit == 0 ? raw != 0 : (raw & bit) != 0;
	const bool now = ui.Checkbox(r, label, on);
	if (now == on) return;
	const u32 w = bit == 0 ? (now ? 1u : 0u) : (now ? raw | bit : raw & ~bit);
	PutBe(p, w);
}

void Words(UI& ui, const char* id, int a, const char* l0, u8* p0, const char* l1, u8* p1, bool sign = false)
{
	Word(ui, ui.Row(), id, a * 2, l0, p0, sign);   // one per row: the labels do not fit two columns
	if (p1 != nullptr) Word(ui, ui.Row(), id, a * 2 + 1, l1, p1, sign);
}

std::string Time(u32 ms)
{
	char b[32];
	std::snprintf(b, sizeof b, "%u:%02u.%03u", ms / 60000, ms / 1000 % 60, ms % 1000);
	return b;
}

// Galaga, block 0 (cRecordMan, 0x454 bytes: cG01GalagaSys::Init / SaveData; stage records +0x2c each, multiplayer records at +0x1e4, +0x38 each)
void GalagaPage(UI& ui)
{
	u8* b = Block(0);
	Words(ui, "save.g01v", 0, "version (2)", b + 0x44c, nullptr, nullptr, true);
	{
		const bool on = b[0x450] != 0;
		if (ui.Checkbox(ui.Row(), "all open (+0x450)", on) != on) b[0x450] = on ? 0 : 1;
	}
	std::vector<std::string> st;
	for (int s = 0; s <= 10; s++) st.push_back(s == 0 ? "0 (unused)" : s == 10 ? "10 world 3 boss" : std::to_string(s) + " world " + std::to_string((s - 1) / 3 + 1));
	g.galagaStage = ui.Choice(ui.Row(), MakeId("save.g01st"), "stage", st, g.galagaStage);
	u8* r = b + 0x2c * g.galagaStage;
	ui.Heading("Single player record");   // cAppG01GalagaGame StoreResult
	Flag(ui, ui.Row(), "open (+0x24)", r + 0x24);
	Words(ui, "save.g01s", 0, "best score", r + 0x10, "best kills", r + 0x00, true);
	Words(ui, "save.g01s", 1, "hit rate (1/1000)", r + 0x08, "lives left + 1", r + 0x0c, true);
	std::vector<devui::Rect> c = UI::Split(ui.Row(), {1.0f, 1.0f, 1.0f, 1.0f});
	Flag(ui, c[0], "score", r + 0x1c);
	Flag(ui, c[1], "kills", r + 0x14);
	Flag(ui, c[2], "rate", r + 0x18);
	Flag(ui, c[3], "life", r + 0x20);
	ui.Label("the four boxes: the stage's ScoreNorma.csv goal reached", &devui::Color(devui::DIM));
	Words(ui, "save.g01s", 2, "+0x04", r + 0x04, "+0x28", r + 0x28, true);
	ui.Heading("Multiplayer record");   // StoreMultiRec
	u8* m = b + 0x1e4 + 0x38 * g.galagaStage;
	Words(ui, "save.g01m", 0, "winner mask", m + 0x00, "lives + 1", m + 0x04, true);
	for (int p = 0; p < 4; p++)
	{
		ui.Label("player " + std::to_string(p + 1), &devui::Color(devui::DIM));
		std::vector<devui::Rect> q = UI::Split(ui.Row(), {1.0f, 1.0f, 1.0f});
		Word(ui, q[0], "save.g01mp", p * 3, "score", m + 0x28 + 4 * p, true);
		Word(ui, q[1], "save.g01mp", p * 3 + 1, "kills", m + 0x08 + 4 * p, true);
		Word(ui, q[2], "save.g01mp", p * 3 + 2, "rate", m + 0x18 + 4 * p, true);
	}
}

// Rally-X, block 1 (cG02RallyXRecord Load / Save: valid, version 0x20070824, all clear, then 29 stages x 0x14 from +0xc)
void RallyXPage(UI& ui)
{
	u8* b = Block(1);
	Words(ui, "save.g02", 0, "valid", b + 0, "version", b + 4);
	Flag(ui, ui.Row(), "all clear (+0x8)", b + 8);
	if (GetBe(b + 4) != 0x20070824u) ui.Label("version is not 0x20070824 (537331748): the game resets the record", &devui::Color(devui::WARN));
	std::vector<std::string> st{"0 (unused)"};
	for (int i = 1; i <= 28; i++)
		st.push_back(std::to_string(i) + (i > 24 ? " multiplayer" : (i - 1) % 6 == 5 ? " world " + std::to_string((i - 1) / 6 + 1) + " boss"
		                                                                              : " world " + std::to_string((i - 1) / 6 + 1) + " stage " + std::to_string((i - 1) % 6 + 1)));
	g.rallyStage = ui.Choice(ui.Row(), MakeId("save.g02st"), "stage", st, g.rallyStage);
	u8* r = b + 0xc + 0x14 * g.rallyStage;
	Flag(ui, ui.Row(), "open", r + 0x0);
	Words(ui, "save.g02s", 0, "best score", r + 0x8, "best time (ms)", r + 0x4);
	ui.Label("time " + Time(GetBe(r + 0x4)), &devui::Color(devui::DIM));
	std::vector<devui::Rect> c = UI::Split(ui.Row(), {1.0f, 1.0f});
	Flag(ui, c[0], "new score mark", r + 0xc);
	Flag(ui, c[1], "new time mark", r + 0x10);
}

// Pac-N-Roll, block 2 (cSaveLoadMan, 0x3c0 bytes: the manager's data object, LoadData 0x8012ad64 / SaveData 0x8012ad20): validate, default
// lives / HP, lives, max HP, all clear, then 26 rounds x 0x24 from +0x18
void PacnRollPage(UI& ui)
{
	u8* h = Block(2);
	Words(ui, "save.g03", 0, "validate (3)", h + 0, "lives", h + 0xc);
	Words(ui, "save.g03", 1, "max HP", h + 0x10, "default lives", h + 4);
	Words(ui, "save.g03", 2, "default HP", h + 8, nullptr, nullptr);
	if (GetBe(h) != 3u) ui.Label("validate is not 3: the game starts Pac-N-Roll over (0 = no Pac-N-Roll save yet)", &devui::Color(devui::WARN));
	Flag(ui, ui.Row(), "all clear (+0x14)", h + 0x14);
	std::vector<std::string> st;
	for (int i = 0; i < 26; i++) st.push_back("round " + std::to_string(i) + (i == 8 || i == 13 || i == 19 || i >= 24 ? " (boss)" : ""));
	g.pnrRound = ui.Choice(ui.Row(), MakeId("save.g03r"), "round", st, g.pnrRound);
	u8* r = h + 0x18 + 0x24 * g.pnrRound;
	Flag(ui, ui.Row(), "open (bFree, +0x20)", r + 0x20);
	Words(ui, "save.g03s", 0, "best time (ms, 599999 = none)", r + 0, "ghosts", r + 4);
	ui.Label("time " + Time(GetBe(r)), &devui::Color(devui::DIM));
	Words(ui, "save.g03s", 1, "cookies", r + 8, "cookies max", r + 0xc);
	std::vector<devui::Rect> c = UI::Split(ui.Row(), {1.0f, 1.0f});
	Flag(ui, c[0], "pizza eaten", r + 0x10);
	Flag(ui, c[1], "time mark", r + 0x14);
	c = UI::Split(ui.Row(), {1.0f, 1.0f});
	Flag(ui, c[0], "ghost mark", r + 0x18);
	Flag(ui, c[1], "cookie mark", r + 0x1c);
}

// Pac-Motos, block 4 (cG07MotosSys save data, 0x46c bytes: SaveDataLoad 0x800a0280 / SaveDataSave 0x800a0438): version 3, 46 stages x 0x18 from
// +0x4, item stock +0x454, all clear +0x464, player stock +0x468
void MotosPage(UI& ui)
{
	u8* b = Block(4);
	Words(ui, "save.g07", 0, "version (3)", b + 0, nullptr, nullptr);
	if (GetBe(b) != 3u) ui.Label("version is not 3: the game starts Pac-Motos over", &devui::Color(devui::WARN));
	Flag(ui, ui.Row(), "all clear (+0x464)", b + 0x464);
	Words(ui, "save.g07i", 0, "power stock", b + 0x458, "jump stock", b + 0x45c);
	Words(ui, "save.g07i", 1, "charge stock", b + 0x460, "player stock (+0x468)", b + 0x468);
	std::vector<std::string> st{"0 (unused)"};
	for (int i = 1; i <= 45; i++)
		st.push_back(i > 35 ? "vs " + std::to_string(i - 35) : "world " + std::to_string((i - 1) / 7 + 1) + ((i - 1) % 7 == 6 ? " boss" : " stage " + std::to_string((i - 1) % 7 + 1)));
	g.motosStage = ui.Choice(ui.Row(), MakeId("save.g07st"), "stage", st, g.motosStage);
	u8* r = b + 4 + 0x18 * g.motosStage;
	std::vector<devui::Rect> c = UI::Split(ui.Row(), {1.0f, 1.0f});
	Flag(ui, c[0], "open", r + 0x14);
	Flag(ui, c[1], "clear", r + 0x10);
	Words(ui, "save.g07s", 0, "hi score", r + 0, "record time (0 = none)", r + 4);
	c = UI::Split(ui.Row(), {1.0f, 1.0f});
	Flag(ui, c[0], "hi score mark", r + 0x8);
	Flag(ui, c[1], "time mark", r + 0xc);
}

// Gator Panic, block 3 {flag, version 0x20070723, score} (cG04WaniWaniSys LoadRecord / SaveRecord); Grobda, block 5 {flag 1, best}
void OthersPage(UI& ui)
{
	u8* w = Block(3);
	ui.Heading("Gator Panic (block 3)");
	Words(ui, "save.g04", 0, "high score", w + 8, "flag", w + 0, true);
	Words(ui, "save.g04", 1, "version", w + 4, nullptr, nullptr);
	if (GetBe(w) == 0 || GetBe(w + 4) != 0x20070723u) ui.Label("flag 0 or version not 0x20070723 (537331491): the game starts at 0", &devui::Color(devui::WARN));
	u8* gr = Block(5);
	ui.Heading("Grobda (block 5)");
	Words(ui, "save.g11", 0, "best", gr + 4, "flag", gr + 0, true);
	if (GetBe(gr) != 1u) ui.Label("flag is not 1: the game ignores the best score", &devui::Color(devui::WARN));
}

// Block 6: the classic hall's high scores (cAppMenu::LoadHiScores: "NMRM" / "NMMX" + 9 words per volume slice of 0x800); the rest of each
// slice is the arcade module's own save (cAppRSOOriginal), edit it on the Raw page
void ArcadePage(UI& ui)
{
	u8* b = Block(6);
	for (int vol = 0; vol < 2; vol++)
	{
		u8* s = b + 0x800 * vol;
		const char* magic = vol == 0 ? "NMRM" : "NMMX";
		ui.Heading(vol == 0 ? "Volume 1 (nmwii.rso)" : "Volume 2 (nmwii2.rso)");
		const bool ok = std::memcmp(s, magic, 4) == 0;
		std::vector<devui::Rect> c = UI::Split(ui.Row(), {2.0f, 1.0f});
		ui.Label(c[0], ok ? std::string("magic ") + magic : "no magic: the hall shows its default scores", &devui::Color(ok ? devui::GOOD : devui::WARN));
		if (!ok && ui.Button(c[1], std::string("Write ") + magic)) std::memcpy(s, magic, 4);
		for (int i = 0; i < 9; i += 2)
			Words(ui, "save.rso", vol * 9 + i, ("game " + std::to_string(vol * 9 + i) + (vol == 1 && i == 0 ? " PAC-MAN" : "")).c_str(), s + 4 + 4 * i,
			       ("game " + std::to_string(vol * 9 + i + 1)).c_str(), i + 1 < 9 ? s + 4 + 4 * (i + 1) : nullptr);
	}
	ui.Label("0 = the hall's default score; more than 999999999 shows as 999999999", &devui::Color(devui::DIM));
}

// any block as hex, plus one big-endian word editor
void RawPage(UI& ui)
{
	std::vector<std::string> bl;
	for (int i = 0; i < (int)BLOCKS; i++)
		bl.push_back(std::to_string(i) + (i == 0 ? " Galaga" : i == 1 ? " Rally-X" : i == 2 ? " Pac-N-Roll" : i == 3 ? " Gator Panic" : i == 4 ? " Pac-Motos" : i == 5 ? " Grobda" : " hub / arcade"));
	g.rawBlock = ui.Choice(ui.Row(), MakeId("save.rawb"), "block", bl, g.rawBlock);
	static const double zero = 0.0;
	g.rawOffset = (int)ui.Number(ui.Row(), MakeId("save.rawo"), "offset (decimal)", g.rawOffset, 4.0, "%.0f", true, &zero);
	g.rawOffset = (g.rawOffset & ~3) % (int)BLOCK_SIZE;
	u8* b = Block(g.rawBlock);
	char line[96];
	std::snprintf(line, sizeof line, "word at +0x%04x", g.rawOffset);
	Word(ui, ui.Row(), "save.raww", g.rawBlock * 0x10000 + g.rawOffset, line, b + g.rawOffset);
	const int start = g.rawOffset & ~0xf;
	for (int row = 0; row < 16 && start + row * 16 < (int)BLOCK_SIZE; row++)
	{
		const u8* p = b + start + row * 16;
		int n = std::snprintf(line, sizeof line, "%04x ", start + row * 16);
		for (int k = 0; k < 16; k++) n += std::snprintf(line + n, sizeof line - n, k % 4 == 0 ? " %02x" : "%02x", p[k]);
		ui.Label(line, &devui::Color(start + row * 16 == (g.rawOffset & ~0xf) ? devui::TEXT : devui::DIM));
	}
}

void SavePanel(UI& ui)
{
	if (g.firstFrame)
	{
		g.firstFrame = false;
		g.src = Trim(WiiSystem::SaveDir());
		g.dst = g.src + "_edit";
	}
	g.src = ui.TextInput(ui.Row(), MakeId("save.src"), "open", g.src, "a save folder (museum.bin)");
	g.dst = ui.TextInput(ui.Row(), MakeId("save.dst"), "write to", g.dst, "another folder");
	std::vector<devui::Rect> c = UI::Split(ui.Row(), {1.0f, 1.0f});
	if (ui.Button(c[0], g.loaded ? "Reopen (drops edits)" : "Open")) Open();
	if (ui.Button(c[1], "Write", g.loaded) && g.loaded) Write();
	if (!g.msg.empty()) ui.Label(g.msg, &devui::Color(g.msgBad ? devui::WARN : devui::GOOD));
	if (!g.loaded)
	{
		ui.Label("Edits stay in this tab until written; the running game is not changed. Writing to the opened folder overwrites it.", &devui::Color(devui::DIM));
		return;
	}
	if (Crc() != g.crcLoaded) ui.Label("edited (the CRC is recomputed on write)", &devui::Color(devui::HINT));
	ui.Space();
	static const char* const kPages[] = {"Galaga", "Rally-X", "Pac-N-Roll", "Pac-Motos", "Gator / Grobda", "Arcade", "Raw", ""};   // button rows (Tabs scroll state is the window's)
	for (int row = 0; row < 2; row++)
	{
		std::vector<devui::Rect> p = UI::Split(ui.Row(), {1.0f, 1.0f, 1.0f, 1.0f});
		for (int k = 0; k < 4; k++)
			if (*kPages[row * 4 + k] && ui.Button(p[k], kPages[row * 4 + k], true, g.page == row * 4 + k)) g.page = row * 4 + k;
	}
	ui.Space();
	switch (g.page)
	{
	case 0: GalagaPage(ui); break;
	case 1: RallyXPage(ui); break;
	case 2: PacnRollPage(ui); break;
	case 3: MotosPage(ui); break;
	case 4: OthersPage(ui); break;
	case 5: ArcadePage(ui); break;
	default: RawPage(ui); break;
	}
}
}

void RegisterSavePanel() { devui::RegisterPanel("Save", SavePanel); }
}
