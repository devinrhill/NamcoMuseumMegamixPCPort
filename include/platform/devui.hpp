#pragma once

// Developer-only immediate-mode UI (a port of unpac's ui.py: panels, buttons, number fields you can click and type into or scroll over, choices,
// check boxes, list boxes, tabs, text inputs) for in-game dev tools. NOT part of the 1:1 game. Raylib-free header: the implementation is
// src/platform/devui_raylib.cpp.
//
// Call UI::Begin once a frame, lay widgets out top to bottom with Row() / Split() and draw them; every widget returns its new value. While a field is
// being typed into, Typing() is true and the host should ignore its hotkeys. The panel is anchored to the screen's right edge, Ctrl + = / - / 0 zoom it
// (kept between runs).
//
// Config (docs/OPTIONS.md): the theme file ($MEGAMIX_UI_THEME, else ~/.config/megamix/ui_theme.json; written with the defaults on the first run, re-read
// when it changes), the zoom file (~/.cache/megamix/ui_scale), an optional TTF font ($MEGAMIX_UI_FONT, else the theme file's "font" key).
//
// Registered panels (RegisterPanel) show as tabs of the dev window that Frame() draws; MEGAMIX_DEVUI=1 or --devui turns it on, F9 shows / hides it.
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace devui
{
struct Rect { int x, y, w, h; };
struct Rgba { unsigned char r, g, b, a; };

// the theme colours (names = the keys of the theme file, lower case with underscores, see kColorNames in the .cpp)
enum Col
{
	PANEL_BG, PANEL_BORDER, TEXT, DIM, HEADING, HEADING_LINE, BUTTON, BUTTON_HOT, BUTTON_ACTIVE, BUTTON_BORDER, BUTTON_TEXT,
	FIELD, FIELD_HOT, FIELD_ACTIVE, FIELD_BORDER, LIST_SELECTED, LIST_HOVER, SCROLLBAR_TRACK, SCROLLBAR_THUMB, WARN, BAD, GOOD,
	HINT, HUD_TEXT, HUD_DIM, HUD_EDIT, HUD_SELECTED, COL_COUNT
};
const Rgba& Color(Col c);                       // the current theme (reloads the theme file when it changed, about once a second)
bool ParseColor(const std::string& s, Rgba& out);   // "#rrggbb" or "#rrggbbaa"

constexpr int ROW = 22, PAD = 6;
float Scale();                                  // the panel zoom
void SetScale(float s);                         // clamped to 0.7 .. 2.6, saved

using Id = std::uint64_t;                       // a widget's identity (0 = none)
Id MakeId(const char* name, int a = 0, int b = 0);

// text (the configured font, or raylib's default; both measure and draw through the zoom of the open panel)
int MeasureText(const std::string& text, int size);
void DrawText(const std::string& text, int x, int y, int size, const Rgba& c);
std::vector<std::string> Wrap(const std::string& text, int size, int width);   // lines no wider than width px, "\n" always breaks

struct ListRow { std::string text; Rgba color; };
struct Texture { unsigned id; int width, height; };   // a raylib Texture2D's id / size (id 0 = no icon), for ListBox icons

class UI
{
public:
	bool Typing() const { return m_focus != 0; }
	void Begin(int x, int y, int w, int h);     // the panel at x, y, w units wide, h pixels high (its right edge x + w stays on the screen's edge)
	void End();                                 // back to screen coordinates; draws the scroll bar and takes the wheel nobody used

	Rect Row(int height = ROW, int gap = 3);
	static std::vector<Rect> Split(const Rect& r, const std::vector<float>& weights, int gap = 4);
	void Space(int h = 6) { m_y += h; }
	bool Inside(const Rect& r) const { return r.x <= m_mx && m_mx < r.x + r.w && r.y <= m_my && m_my < r.y + r.h; }
	bool OverPanel() const { return Inside(m_panel); }

	void Label(const std::string& text, const Rgba* color = nullptr);         // wraps onto more rows when wider than the panel
	void Label(const Rect& r, const std::string& text, const Rgba* color = nullptr);
	void Heading(const std::string& text);
	bool Button(const Rect& r, const std::string& text, bool enabled = true, bool active = false);
	bool Checkbox(const Rect& r, const std::string& text, bool value);
	int Choice(const Rect& r, Id id, const std::string& label, const std::vector<std::string>& options, int index);   // click / scroll steps, right click back
	// A number box: scroll over it (or the arrow keys) to nudge (shift x10, alt x0.1); click to type, Enter / Tab / click elsewhere applies, Esc cancels.
	double Number(const Rect& r, Id id, const std::string& label, double value, double step = 0.1, const char* fmt = "%.3f", bool integer = false,
	              const double* minimum = nullptr);
	void Vec3(const Rect& r, Id id, const std::string& label, float v[3], double step = 0.1, const char* fmt = "%.2f");
	// Up / Down / PageUp / PageDown / Home / End move through the list the mouse is over; with confirm Enter selects (the keys only move a cursor).
	// A double click on a row selects it and sets *activated (when given): the caller runs the row's action.
	int ListBox(Id id, const Rect& r, const std::vector<ListRow>& rows, int selected, bool reveal = false,
	            const std::vector<Texture>* icons = nullptr, bool confirm = false, bool* activated = nullptr);
	// a row of tabs; when they do not fit the panel's width it scrolls sideways (the < / > ends or the wheel over it), keeping the active one in view
	int Tabs(const std::vector<std::string>& labels, int active);
	std::string TextInput(const Rect& r, Id id, const std::string& label, const std::string& text, const std::string& hint = "");

	const Rect& PanelRect() const { return m_panel; }

private:
	void Finish();
	void Close();
	void Text(const Rect& r, const std::string& text, const Rgba& c, int size = 14, bool center = false);
	std::string Fit(const std::string& text, int size, int width);

	Id m_focus{0};                              // the field being typed into
	std::string m_buf;
	bool m_fresh{false};
	std::unordered_map<Id, int> m_top, m_shown, m_cursor;   // per list box: first row, last revealed selection, keyboard cursor (ui.py state)
	Id m_activeList{0};
	Rect m_panel{0, 0, 0, 0};
	int m_y{0};
	float m_mx{0}, m_my{0};
	bool m_click{false}, m_rightClick{false}, m_doubleClick{false};
	double m_lastClickT{-1.0};                  // double click: a second click within 0.4 s, a few pixels from the first
	float m_lastClickX{0}, m_lastClickY{0};
	float m_wheel{0}, m_wheelAcc{0};
	bool m_zoomed{false}, m_scissor{false}, m_open{false}, m_wheelUsed{false};
	bool m_shift{false}, m_alt{false};
	std::vector<unsigned> m_chars;
	bool m_enter{false}, m_escape{false}, m_tab{false}, m_backspace{false};
	bool m_nav[8]{};                            // up down left right pageup pagedown home end
	float m_scroll{0}, m_contentH{0};
	float m_drag{-1};                           // scroll bar thumb grab offset, < 0 = none
	int m_tabFirst{0}, m_tabActive{-1};         // Tabs: first tab shown while the row scrolls, the active tab it last revealed
};

// the dev window: registered panels as tabs on the right edge of the window
// visible (optional): the tab only shows while it returns true (e.g. only in some jobs); the active tab is remembered by title, so it comes back
void RegisterPanel(const std::string& title, std::function<void(UI&)> fn, std::function<bool()> visible = {});
void SetEnabled(bool on);
bool Enabled();
bool Typing();                                  // a dev window field has the keyboard (the game's keys read as up meanwhile)
bool WantsMouse();                              // the mouse is over the dev window (the game's mouse buttons read as up)
int ReservedWidth();                            // window pixels on the right kept for the dev window (0 when hidden): the game area is the rest
void Frame();                                   // inside the frame (platform::EndFrame's drawing): F9 toggles, draws the active panel

// raw mouse / keyboard for tools that work in the game view (a level editor): not gated by the panel; window pixels
struct Input
{
	float mx, my, dx, dy, wheel;
	bool down[3], pressed[3], released[3];      // 0 left, 1 right, 2 middle
	bool ctrl, shift, alt;
};
Input GetInput();
enum Key { K_DELETE = 256, K_BACKSPACE, K_ENTER, K_ESCAPE, K_PAGE_UP, K_PAGE_DOWN };
bool KeyPressed(int key);                       // 'A'..'Z', '0'..'9' or a Key
Rect GameView();                                // window pixels of the 4:3 game picture (the 3D pass' viewport)
// a tool uses the game view this frame: the game reads its mouse buttons (mouse) / keys (keys) as up from the next frame on. Asked again every frame.
void CaptureInput(bool mouse, bool keys);
}
