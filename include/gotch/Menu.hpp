#pragma once

#include <cstdint>
#include <string>

// The menus of the gotch frontend (nmwii2.rso): MENUITEM / ITEMLIST (a linked list of texts), CMENU (a MENUITEM that owns items, reads the
// pads, draws a box and its texts through CDraw::PrintfUTF8) and WII_MENU (the per-game set of CMENUs: start, pause, screen adjust, options,
// maniac options, debug). A 1:1 port of the pieces the START, PAUSE and OPTIONS menus use; the other menus are built empty and their jobs are not ported
// (marked at the sites). Field offsets of the original are in the comments.
namespace gotch
{
class CDraw;
class CGameIO;
class MenuItem;
struct SWork;

// the 0x64-byte block the *MenuSet functions fill on the stack (MENUITEM word n + 1 = block word n)
struct MenuItemInfo
{
	int id = -1;                    // +0x04: MenuMM's slot (-1 = none)
	std::uint32_t command = 0;      // +0x08: what the item does (0x20000 continue, 0x20300 screen adjust, 0x2ff00 quit, ...)
	int unk0c = 0;                  // +0x0c
	int x = 0;                      // +0x10
	int y = 0;                      // +0x14
	int textPalette = 0;            // +0x18: RenderText's palette
	int palette = 0;                // +0x1c: normal
	int paletteBlink = 0;           // +0x20: under the cursor, blink phase on
	int paletteCursor = 0;          // +0x24: under the cursor
	int unk28 = 1;                  // +0x28
	std::uint32_t printFlags = 0;   // +0x2c: PrintfUTF8 flags (alignment 0x1000 / 0x2000, ...)
	int lineHeight = 0xc;           // +0x30
	std::uint8_t unk34 = 0;         // +0x34
	std::uint8_t unk35 = 0;         // +0x35
	std::uint32_t decideMask = 1;   // +0x38: 1 = PAD_DECIDE runs `decide`, 2 = left / right run it
	void (*decide)(MenuItem*) = nullptr;   // +0x3c: called with the item (r3)
	int unk40 = 0;                  // +0x40
	int unk44 = 0;                  // +0x44
	void (*enter)() = nullptr;      // +0x48: the cursor arrived on the item
	void (*leave)() = nullptr;      // +0x4c: the cursor left it
	int unk50 = 0;                  // +0x50
	int unk54 = 0;                  // +0x54
	int rect[4] = {};               // +0x58: left, top, right, bottom (ApplyMenuTemplate measures it)
};

// MENUITEM (0x8c bytes; ctor RSO2 0x80d28534). Its vtable is ITEMLIST's (GetText +0xc, IncNumber +0x1c = 1, DecNumber +0x20 = 1, ...)
class MenuItem
{
public:
	MenuItem(const MenuItemInfo& info, const char* text);
	virtual void SetText(const char* t) { text = t; }      // ITEMLIST::SetText RSO2 0x80d3067c (vtable +0x10)
	virtual ~MenuItem();                                   // RSO2 0x80d28684 (DeleteItemAll)
	virtual const char* GetText() const { return text; }  // ITEMLIST::GetText RSO2 0x80d295f4
	virtual bool IncNumber() { return true; }             // ITEMLIST::IncNumber RSO2 0x80d29040
	virtual bool DecNumber() { return true; }             // ITEMLIST::DecNumber RSO2 0x80d291b8

	bool AddItem(MenuItem* item, int at);       // RSO2 0x80d287bc (at -1 = the end)
	bool DeleteItemAll();                       // RSO2 0x80d28940
	MenuItem* GetItemByNum(int n) const;        // RSO2 0x80d28e90
	bool IncItemCursor();                       // RSO2 0x80d28ed0
	bool SetItemCursor(int n);                  // RSO2 0x80d291c0 (true = it wrapped / clamped)
	MenuItem* Current() const { return GetItemByNum(cursor); }

	MenuItemInfo info;                // +0x04
	const char* text = nullptr;       // +0x68
	MenuItem* next = nullptr;         // +0x6c
	MenuItem* prev = nullptr;         // +0x70
	bool enabled = true;              // +0x74: shown and selectable (a CMENU: running)
	bool unk75 = false;               // +0x75: drawn although disabled
	bool wrap = true;                 // +0x76: the cursor wraps
	int count = 0;                    // +0x78
	MenuItem* first = nullptr;        // +0x7c
	int cursor = 0;                   // +0x80
	bool useRepeat = false;           // +0x84: the cursor uses the auto-repeat words
	bool subCursor = true;            // +0x85: left / right move a child list's cursor
	int repeatTable = 0;              // +0x88

	// the "move the cursor until it sits on an enabled item" loop the module inlines everywhere (IncItemCursor's body after the
	// IncNumber call); true = the cursor wrapped / clamped on the way or no enabled item was found
	bool StepToEnabled(int dir);
};

// the printf the menus run over module formats ("ROUND SELECT %d", "%02d"): only %d with an optional 0 flag and width, and %%
std::string FormatInt(const char* format, int n);

// NUMITEMLIST (ctor RSO2 0x80d2923c): an item whose text is a printf format of one number (the start menu's ROUND SELECT %d). In the module
// the number / max / min overlay MENUITEM's count / first / cursor (+0x78 / +0x7c / +0x80); a NUMITEMLIST never holds items, so they are
// separate fields here.
class NumItemList : public MenuItem
{
public:
	// RSO2 0x80d2923c: info, format, number, min, max (SetNumberEx_Num / _Min / _Max); wraps (+0x76 = 1)
	NumItemList(const MenuItemInfo& info, const char* format, int number, int min, int max);
	const char* GetText() const override;   // RSO2 0x80d295b0
	bool IncNumber() override;              // RSO2 0x80d2963c
	bool DecNumber() override;              // RSO2 0x80d29708
	int GetRange() const { return max - min; }   // RSO2 0x80d296f8
	int GetNumber() const { return number; }     // RSO2 0x80d30688

	int number = 0;   // +0x78
	int max = 0;      // +0x7c (exclusive)
	int min = 0;      // +0x80

private:
	bool Step(int d);
	mutable char buf[64] = {};   // +0x84 (strlen(format) + 0x21 bytes)
};

// CMENU (300 bytes; ctor RSO2 0x80d29900)
class CMenu : public MenuItem
{
public:
	CMenu(CDraw& draw, CGameIO& gameio, const MenuItemInfo& info, const char* title);
	int MenuJob(bool readPads);                 // RSO2 0x80d29d38
	int Render();                               // RSO2 0x80d2a970
	void RenderBox();                           // RSO2 0x80d2ae94
	int ApplyMenuTemplate(int kind);            // RSO2 0x80d2c020 (kinds 2, 4 and 8 only: the others are not ported)
	// RSO2 0x80d2bc94: the rows template 2 pins to the bottom of the screen (DEFAULT, EXIT, ...); moves the value bounds with the label
	void SpecialMenuLocationFix(const MenuItemInfo& item, int* bounds, int* rect) const;
	int OptionsTemplate(int right);             // ApplyMenuTemplate case 2
	bool SetStyleControl(int mode, int value);  // RSO2 0x80d2b9e0
	void SetBlackBox(int l, int t, int r, int b, int unkb8, int colour, bool pixels);   // RSO2 0x80d2b798
	void SetBlinkAtr(int on, int off, bool blink)   // RSO2 0x80d2a91c
	{
		blinkOn = on;
		blinkEnd = on + off;
		blinkCount = 0;
		blinkEnabled = blink;
		blinkPhase = false;
	}
	void SetScreenSpec(int w8, int h8, int rotation)   // RSO2 0x80d2b780
	{
		rot = rotation;
		screenW = w8 << 3;
		screenH = h8 << 3;
	}
	void SetDrawHandle(int tex) { texture = tex; }                     // RSO2 0x80d2a914
	void SetStringConvertTable(const std::uint16_t* t) { conv = t; }   // RSO2 0x80d2b778
	void SetPaletOffset(int p) { paletOffset = p; }                    // RSO2 0x80d2a968
	void ResetBlinkCount() { blinkCount = 0; }                         // RSO2 0x80d2a93c
	void SetMenuTimer(int t) { timer = t; }                            // RSO2 0x80d2a950
	int GetMenuTimer() const { return timer; }                         // CMENU::GetMenuTimer

	MenuItem* changed = nullptr;      // +0x8c
	MenuItem* last = nullptr;         // +0x90: the item under the cursor before this MenuJob
	std::uint32_t result = 0;         // +0x9c: the pad bits this frame (what the WII_MENU jobs test)
	bool box = false;                 // +0xa4
	bool crossHatch = false;          // +0xa5
	int boxRect[4] = {};              // +0xa8: pixels
	int unkb8 = 0;                    // +0xb8
	int blinkOn = 0;                  // +0xbc
	int blinkEnd = 0;                 // +0xc0
	int blinkCount = 0;               // +0xc4
	bool blinkEnabled = false;        // +0xc8
	bool blinkPhase = false;          // +0xc9
	int timer = 0;                    // +0xcc
	int paletOffset = 0;              // +0xd0
	int boxColour = 0;                // +0xd8
	int texture = 0;                  // +0xdc
	int rot = 0;                      // +0xe0
	int screenW = 0;                  // +0xe4
	int screenH = 0;                  // +0xe8
	const std::uint16_t* conv = nullptr;   // +0xec
	int styleMode = 0;                // +0xf0: 1 fade in, 2 fade out, 3 callback
	bool styleActive = false;         // +0xf4
	void (*styleFn)(CMenu*) = nullptr;     // +0xf8
	int styleAlpha = 0;               // +0xfc: 16.16
	int styleStep = 0;                // +0x100
	bool display = true;              // +0x104
	std::uint32_t moveMask = ~0u;     // +0x114
	std::uint32_t padDecide = 0;      // the module's PAD_DECIDE (a global there; WII_MENU passes it in)

private:
	CDraw& draw;     // +0x98 machine -> +0x11c
	CGameIO& gameio; // +0x98 machine -> +0x118
};

// WII_MENU (0x334 bytes; ctor RSO2 0x80d48a4c). Built with the game's swork and machine; PACMAN's subclass GAMEMENU only overrides empty /
// save-data hooks (vtable 0x81148538)
class WiiMenu
{
public:
	struct Strings
	{
		const char* pause = "";          // MenuStr_PAUSE (0x811dcff8)
		const char* cont = "";           // MenuStr_CONTINUE (0x811dce8c)
		const char* screenAdjust = "";   // MenuStr_NMREMIX_WII_SCREEN_ADJUST_TITLE (0x811dd104)
		const char* quit = "";           // MenuStr_QUIT (0x811dce88)
		// the start menu (StartMenuSet)
		const char* start1P = "";        // MenuStr_STARTMENU_1UP_START
		const char* start2P = "";        // MenuStr_STARTMENU_2UP_START
		const char* roundSelect = "";    // MenuStr_NMREMIX_WII_STARTMENU_ROUND_SELECT (a printf format)
		const char* options = "";        // MenuStr_STARTMENU_OPTIONS
		const char* startScreenAdjust = "";   // MenuStr_STARTMENU_SCREEN_ADJUST
		const char* retMainMenu = "";    // MenuStr_STARTMENU_RET_MAINMENU (only its length is used)
		const char* returnToArcade = ""; // MenuStr_NMREMIX_WII_STARTMENU_RETURN_TO_ARCADE
		const char* gameName = "";       // MenuStr_PAC_MAN (the title's x)
		const char* soundTest = "";      // MenuStr_SOUND_TEST (ChangeMenu / the options job compare an item's text pointer with it)
	};
	// machine: draw, gameio, the screen size and rotation (machine +0x20 / +0x24 / +0x40); pad words PAD_DECIDE / PAD_CANCEL (CommonResourceSet)
	WiiMenu(SWork& sw, CDraw& draw, CGameIO& gameio, int screenW, int screenH, int rotation, const Strings& str, std::uint32_t padDecide,
	        std::uint32_t padCancel);
	~WiiMenu();

	void SetStringConvertTable(const std::uint16_t* t) { stringConv = t; }   // RSO2 0x80d4943c
	void SetPaletStatus(int offset, int stride);                            // RSO2 0x80d4a4b4
	void SetMenuYoffset(int start, int pause, int screen, int options, int maniac, int debug);   // RSO2 0x80d4a5a0
	void InitMenu();                                // RSO2 0x80d4a3c4
	void RebuildMenu();                             // RSO2 0x80d4a42c
	void EnableMenu();                              // RSO2 0x80d4a490
	void MenuIn(int menu, bool resetCursor);        // RSO2 0x80d49450
	void ChangeMenu(int menu, bool resetCursor);    // RSO2 0x80d494fc
	void PushAndChangeMenu(int menu, bool resetCursor);   // RSO2 0x80d494bc
	void MenuJob();                                 // RSO2 0x80d49a78
	void MenuRender();                              // RSO2 0x80d4da04
	void PauseMenuSet();                            // RSO2 0x80d4b304
	void PauseMenuJob();                            // RSO2 0x80d4e078
	void OptionMenuTemplateSet();                   // RSO2 0x80d4c9f0
	void OptionsMenuLeave();                        // the "back" MenuJob case 4 inlines twice
	CMenu& Options() { return *options; }
	void StartMenuSet();                            // RSO2 0x80d4a694
	void StartMenuJob();                            // RSO2 0x80d4dc00
	// RSO2 0x80d4f388 (-1 = keep): the start menu's title, its x / y, RenderText palette and palette (in palette steps)
	void SetStartMenuTitle(const char* text, int x, int y, int textPalette, int palette);
	int GetContinueRound(int player) const;         // RSO2 0x80d4f500: the round the player's ROUND SELECT shows, from 0

	int Mode() const { return mode; }
	int players = 0;                 // +0x0c: 0 = 1P, 1 = 2P (the start menu's choice)
	int startWait = 0;               // +0x14: sw+0x14 after a start (PACMAN::MenuConstruct sets 0x10)
	int continueRound = 0;           // +0x300: the highest round the ROUND SELECT offers (0 = no ROUND SELECT rows)
	void (*gameStartInit)(int command) = nullptr;   // vtable +0xc (PACMAN::GAMEMENU::GameStartInit)
	int drawHandle = 0;              // +0xdc: the font texture (the game sets it every frame)
	int quitWait = 0;                // +0x18: sw+0x14 (CallRender's blank frames) after QUIT (PACMAN::MenuConstruct sets 0x10)
	void (*soundBufferClear)() = nullptr;   // CWiiSound::BufferClear of the machine's sound (+0x120 -> +0xc0)
	void (*menuExitExtend)() = nullptr;     // vtable +0x24 (RALLYX clears its video RAM; PACMAN's is WII_MENU's empty one)
	// RALLYX's AutoCalcBlackBoxSize override (vtable +0x28, RSO2 0x80d727d0): the start menu's box is (0, 0, w8, h8) instead of the default; 0 = none
	int startBoxW8 = 0, startBoxH8 = 0;
	// CMENU::SetScreenSpec of the start and pause menus (RALLYX: 0x1c x 0x1c, RSO2 0x80d72964's tail)
	void SetStartPauseScreenSpec(int w8, int h8, int rotation);

private:
	SWork& sw;                       // +0xd8
	CDraw& draw;
	CGameIO& gameio;                 // +0xd4 machine -> +0x118
	Strings str;
	std::uint32_t padDecide, padCancel;
	int wait = 0;                    // +0x10: frames MenuJob waits after a change
	bool resetCursor = false;        // +0x09
	CMenu* start = nullptr;          // +0x1c
	CMenu* pause = nullptr;          // +0x4c
	CMenu* screenAdjust = nullptr;   // +0x70
	CMenu* options = nullptr;        // +0x98
	CMenu* maniac = nullptr;         // +0xa4
	CMenu* debug = nullptr;          // +0xb0
	MenuItem* item1P = nullptr;             // +0x30
	MenuItem* hold1P = nullptr;             // +0x34: holds the 1P ROUND SELECT
	MenuItem* item2P = nullptr;             // +0x38
	MenuItem* hold2P = nullptr;             // +0x3c
	MenuItem* itemOptions = nullptr;        // +0x40
	MenuItem* itemStartScreenAdjust = nullptr;   // +0x44
	MenuItem* itemReturn = nullptr;         // +0x48
	int titlePalette = -1;                  // +0x2c
	int screenW = 0, screenH = 0, rotation = 0;   // the machine's +0x20 / +0x24 / +0x40
	MenuItem* itemContinue = nullptr;       // +0x5c
	MenuItem* itemScreenAdjust = nullptr;   // +0x60
	MenuItem* itemQuit = nullptr;           // +0x6c
	int x[6] = {0x70, 0x70, 0x70, 0x70, 0x70, 0x70};   // +0x20 / +0x50 / +0x74 / +0x9c / +0xa8 / +0xb4
	int y[6] = {0x90, 0x84, 0x80, 0x80, 0x80, 0x80};   // +0x24 / +0x54 / +0x78 / +0xa0 / +0xac / +0xb8
	int mode = 0;                    // +0xe0: 0 off, 1 in game, 2 start, 4 options, 5 maniac, 6 pause, 7..9 screen adjust, 10 debug
	int depth = 0;                   // +0xe4
	int stack[0x80] = {};            // +0xe8
	int startBlinkOn = 0xf, startBlinkOff = 0x2d;   // +0x2e8 / +0x2ec
	int blinkOn = 0xf, blinkOff = 0x2d;   // +0x2f8 / +0x2fc
	int paletOffset = 0;             // +0x304
	int paletStride = 1;             // +0x308
	const std::uint16_t* stringConv = nullptr;   // +0x330
};
}
