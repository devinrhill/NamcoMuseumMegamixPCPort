// Developer-only immediate-mode UI: a port of unpac's ui.py (see include/platform/devui.hpp). NOT part of the 1:1 game.
#include "platform/devui.hpp"

#include <raylib.h>
#include <rlgl.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>

#include <nlohmann/json.hpp>

namespace devui
{
namespace
{
namespace fs = std::filesystem;

std::string EnvOr(const char* name, const std::string& fallback)
{
	const char* v = std::getenv(name);
	return v != nullptr && *v != 0 ? std::string(v) : fallback;
}
std::string Home() { return EnvOr("HOME", "."); }

// ---------------------------------------------------------------------------------------------------------------- scale
constexpr float SCALE_MIN = 0.7f, SCALE_MAX = 2.6f;
float g_scale = 1.0f;                           // Ctrl + = / Ctrl + - / Ctrl + 0 zoom the panel; kept between runs
std::string ScaleFile() { return EnvOr("XDG_CACHE_HOME", Home() + "/.cache") + "/megamix/ui_scale"; }

bool LoadScale()
{
	std::ifstream f(ScaleFile());
	float v;
	if (f >> v) g_scale = std::min(std::max(v, SCALE_MIN), SCALE_MAX);
	return true;
}
bool g_scaleLoaded = LoadScale();

// ---------------------------------------------------------------------------------------------------------------- theme
// Every colour of the panels comes from a theme that a file can change (the first run writes the file with these defaults):
// $MEGAMIX_UI_THEME, else ~/.config/megamix/ui_theme.json. A colour is "#rrggbb", "#rrggbbaa" (aa: 00 clear, ff solid) or [r, g, b, a].
// The file is read again when it changes, so edits show in a running game. Missing or invalid colours keep their default.
const char* const kColorNames[COL_COUNT] = {
	"panel_bg", "panel_border", "text", "dim", "heading", "heading_line", "button", "button_hot", "button_active", "button_border", "button_text",
	"field", "field_hot", "field_active", "field_border", "list_selected", "list_hover", "scrollbar_track", "scrollbar_thumb", "warn", "bad", "good",
	"hint", "hud_text", "hud_dim", "hud_edit", "hud_selected",
};
const Rgba kDefaultTheme[COL_COUNT] = {
	{36, 38, 50, 245},      // panel_bg: the side panel's background (the alpha shows the game through it)
	{90, 94, 120, 255},     // panel_border: the line on the panel's left edge
	{225, 225, 235, 255},   // text: default text
	{140, 140, 160, 255},   // dim: secondary text: field labels, notes, disabled buttons
	{150, 170, 230, 255},   // heading: section heading text
	{70, 74, 100, 255},     // heading_line: the line under a section heading
	{24, 26, 36, 255},      // button
	{58, 62, 84, 255},      // button_hot: under the mouse
	{80, 110, 170, 255},    // button_active: the selected tab / tool
	{70, 74, 100, 255},     // button_border
	{225, 225, 235, 255},   // button_text
	{24, 26, 36, 255},      // field: number boxes, choices, check boxes, lists
	{58, 62, 84, 255},      // field_hot
	{80, 110, 170, 255},    // field_active: a box being typed into
	{70, 74, 100, 255},     // field_border
	{80, 110, 170, 255},    // list_selected
	{58, 62, 84, 255},      // list_hover
	{24, 26, 36, 255},      // scrollbar_track
	{110, 116, 150, 255},   // scrollbar_thumb
	{255, 170, 80, 255},    // warn
	{255, 100, 100, 255},   // bad
	{130, 220, 140, 255},   // good
	{211, 211, 211, 255},   // hint: small hotkey help lines
	{245, 245, 245, 255},   // hud_text: status lines drawn over the game
	{211, 211, 211, 255},   // hud_dim
	{255, 200, 120, 255},   // hud_edit
	{255, 230, 150, 255},   // hud_selected
};
Rgba g_current[COL_COUNT];
std::string g_fontFile;                         // the theme's "font" key ($MEGAMIX_UI_FONT wins); empty = raylib's font
constexpr const char* DEFAULT_FONT = "resources/fonts/Verdana.ttf";   // unpac's res/font/Verdana.ttf, relative to the working directory
struct ThemeState
{
	bool have{false};                           // a stamp was read (else the file was missing)
	fs::file_time_type mtime{};
	std::uintmax_t size{0};
	double checked{-10.0};
	bool warned{false};
	bool first{true};
} g_theme;

std::string ThemeFile() { return EnvOr("MEGAMIX_UI_THEME", EnvOr("XDG_CONFIG_HOME", Home() + "/.config") + "/megamix/ui_theme.json"); }

bool ParseJsonColor(const nlohmann::json& v, Rgba& out)
{
	if (v.is_string()) return ParseColor(v.get<std::string>(), out);
	if (v.is_array() && (v.size() == 3 || v.size() == 4))
	{
		int c[4] = {0, 0, 0, 255};
		for (size_t i = 0; i < v.size(); i++)
		{
			if (!v[i].is_number_integer()) return false;
			int k = v[i].get<int>();
			if (k < 0 || k > 255) return false;
			c[i] = k;
		}
		out = Rgba{(unsigned char)c[0], (unsigned char)c[1], (unsigned char)c[2], (unsigned char)c[3]};
		return true;
	}
	return false;
}

// Read the theme file when it has changed (looked at about once a second). The first run writes it with the defaults.
void LoadTheme(bool force = false)
{
	double now = GetTime();
	if (g_theme.first)
	{
		std::copy(kDefaultTheme, kDefaultTheme + COL_COUNT, g_current);
		g_theme.first = false;
		force = true;
	}
	if (!force && now - g_theme.checked < 1.0) return;
	g_theme.checked = now;
	const std::string path = ThemeFile();
	std::error_code ec;
	bool have = fs::is_regular_file(path, ec);
	fs::file_time_type mtime{};
	std::uintmax_t size = 0;
	if (have)
	{
		mtime = fs::last_write_time(path, ec);
		size = fs::file_size(path, ec);
		have = !ec;
	}
	if (!force && have == g_theme.have && (!have || (mtime == g_theme.mtime && size == g_theme.size))) return;
	g_theme.have = have;
	g_theme.mtime = mtime;
	g_theme.size = size;
	Rgba colors[COL_COUNT];
	std::copy(kDefaultTheme, kDefaultTheme + COL_COUNT, colors);
	std::string font;
	if (!have)
	{
		fs::create_directories(fs::path(path).parent_path(), ec);
		std::ofstream f(path);
		if (f)
		{
			nlohmann::ordered_json doc;
			doc["_help"] = "colours: #rrggbb, #rrggbbaa (aa: 00 clear .. ff solid) or [r, g, b, a]; saved changes show at once; "
			               "\"font\": a .ttf file for the text (empty = resources/fonts/Verdana.ttf like unpac's ui.py, \"raylib\" = raylib's pixel font)";
			doc["font"] = "";
			char hex[16];
			for (int i = 0; i < COL_COUNT; i++)
			{
				const Rgba& c = kDefaultTheme[i];
				std::snprintf(hex, sizeof hex, "#%02x%02x%02x%02x", c.r, c.g, c.b, c.a);
				doc[kColorNames[i]] = hex;
			}
			f << doc.dump(2) << "\n";
		}
	}
	else
	{
		try
		{
			std::ifstream f(path);
			nlohmann::json data = nlohmann::json::parse(f);
			if (!data.is_object()) throw std::runtime_error("not a JSON object");
			for (auto it = data.begin(); it != data.end(); ++it)
			{
				if (it.key() == "font")
				{
					if (it.value().is_string()) font = it.value().get<std::string>();
					continue;
				}
				for (int i = 0; i < COL_COUNT; i++)
				{
					if (it.key() != kColorNames[i]) continue;
					Rgba c;
					if (ParseJsonColor(it.value(), c)) colors[i] = c;
					else if (!g_theme.warned)
						std::fprintf(stderr, "ui theme: %s: %s is not a colour (#rrggbb, #rrggbbaa or [r, g, b, a])\n", kColorNames[i], it.value().dump().c_str());
				}
			}
			g_theme.warned = false;
		}
		catch (const std::exception& e)
		{
			if (!g_theme.warned) std::fprintf(stderr, "ui theme: %s: %s\n", path.c_str(), e.what());
			g_theme.warned = true;
		}
	}
	std::copy(colors, colors + COL_COUNT, g_current);
	g_fontFile = EnvOr("MEGAMIX_UI_FONT", font);
	if (g_fontFile.empty()) g_fontFile = DEFAULT_FONT;   // unpac ui.py FONT_FILE
	else if (g_fontFile == "raylib") g_fontFile.clear();
}

::Color RL(const Rgba& c) { return ::Color{c.r, c.g, c.b, c.a}; }
::Color RL(Col c) { return RL(Color(c)); }

// ---------------------------------------------------------------------------------------------------------------- text
// A TTF (the theme's "font" / $MEGAMIX_UI_FONT, by default resources/fonts/Verdana.ttf) instead of raylib's pixel font, cached per pixel size. Under the panel's zoom matrix the
// glyphs are rasterised at their final pixel size and drawn at px / zoom logical units, so each texel lands on exactly one screen pixel.
std::map<int, ::Font> g_fonts;
std::string g_fontLoaded;                       // the file g_fonts came from
bool g_fontBad = true;                          // no file, or it would not load: raylib's font
float g_zoomScale = 1.0f, g_zoomRight = 0.0f;   // (scale, x it grows from) while the panel is drawn under its zoom matrix, see UI::Begin

const ::Font* FontPx(int px)
{
	if (g_fontFile != g_fontLoaded)
	{
		for (auto& f : g_fonts) UnloadFont(f.second);
		g_fonts.clear();
		g_fontLoaded = g_fontFile;
		g_fontBad = g_fontFile.empty() || !FileExists(g_fontFile.c_str());
	}
	if (g_fontBad) return nullptr;
	auto it = g_fonts.find(px);
	if (it == g_fonts.end())
	{
		::Font f = LoadFontEx(g_fontFile.c_str(), px, nullptr, 0);
		if (f.texture.id == 0 || f.texture.id == GetFontDefault().texture.id)
		{
			std::fprintf(stderr, "ui font: %s did not load, using raylib's font\n", g_fontFile.c_str());
			g_fontBad = true;
			return nullptr;
		}
		SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
		it = g_fonts.emplace(px, f).first;
	}
	return &it->second;
}

// (font, logical size) of a text size under the current zoom; nullptr = raylib's default font
const ::Font* Sized(int size, float& logical)
{
	int px = std::max((int)std::lround(size * g_zoomScale), 1);
	logical = px / g_zoomScale;
	return FontPx(px);
}

Id SubId(Id id, int k) { return (id ^ 0x9e3779b97f4a7c15ull) * 1099511628211ull + (Id)(k + 1); }
}

const Rgba& Color(Col c)
{
	LoadTheme();
	return g_current[c];
}

bool ParseColor(const std::string& s, Rgba& out)
{
	std::string t = s;
	t.erase(0, t.find_first_not_of(" \t"));
	t.erase(t.find_last_not_of(" \t") + 1);
	t.erase(0, t.find_first_not_of('#'));
	if (t.size() != 6 && t.size() != 8) return false;
	for (char c : t)
		if (!std::isxdigit((unsigned char)c)) return false;
	auto hex = [&](int i) { return (unsigned char)std::stoi(t.substr(i, 2), nullptr, 16); };
	out = Rgba{hex(0), hex(2), hex(4), t.size() == 8 ? hex(6) : (unsigned char)255};
	return true;
}

float Scale() { return g_scale; }
void SetScale(float s)
{
	g_scale = std::round(std::min(std::max(s, SCALE_MIN), SCALE_MAX) * 1000.0f) / 1000.0f;
	std::error_code ec;
	fs::create_directories(fs::path(ScaleFile()).parent_path(), ec);
	std::ofstream f(ScaleFile());
	if (f) f << g_scale;
}

Id MakeId(const char* name, int a, int b)
{
	Id h = 1469598103934665603ull;
	for (const char* p = name; *p; p++) h = (h ^ (unsigned char)*p) * 1099511628211ull;
	return SubId(SubId(h, a), b);
}

int MeasureText(const std::string& text, int size)
{
	float logical;
	const ::Font* f = Sized(size, logical);
	if (f == nullptr) return ::MeasureText(text.c_str(), size);
	return (int)std::lround(MeasureTextEx(*f, text.c_str(), logical, 0.0f).x);
}

void DrawText(const std::string& text, int x, int y, int size, const Rgba& c)
{
	float logical;
	const ::Font* f = Sized(size, logical);
	if (f == nullptr)
	{
		::DrawText(text.c_str(), x, y, size, RL(c));
		return;
	}
	float fx = (float)x, fy = (float)y;
	if (g_zoomScale != 1.0f)                    // whole screen pixels: a glyph quad on a fraction is blurred by the filter
	{
		float right = g_zoomRight, s = g_zoomScale;
		fx = right + (std::round(right + (x - right) * s) - right) / s;
		fy = std::round(y * s) / s;
	}
	DrawTextEx(*f, text.c_str(), Vector2{fx, fy}, logical, 0.0f, RL(c));
}

std::vector<std::string> Wrap(const std::string& text, int size, int width)
{
	std::vector<std::string> lines;
	auto split = [](const std::string& s, char sep) {
		std::vector<std::string> out;
		size_t a = 0;
		for (size_t b; (b = s.find(sep, a)) != std::string::npos; a = b + 1) out.push_back(s.substr(a, b - a));
		out.push_back(s.substr(a));
		return out;
	};
	for (const std::string& para : split(text, '\n'))
	{
		std::string line;
		for (std::string word : split(para, ' '))
		{
			std::string trial = line.empty() ? word : line + " " + word;
			if (MeasureText(trial, size) <= width)
			{
				line = trial;
				continue;
			}
			if (!line.empty()) lines.push_back(line);
			while (word.size() > 1 && MeasureText(word, size) > width)
			{
				size_t k = word.size() - 1;
				while (k > 1 && MeasureText(word.substr(0, k), size) > width) k--;
				lines.push_back(word.substr(0, k));
				word = word.substr(k);
			}
			line = word;
		}
		lines.push_back(line);
	}
	return lines;
}

// ------------------------------------------------------------------------------------------------------------------ frame

// Back to plain screen coordinates after the panel (Begin zooms by Scale() about the panel's top right corner). Also where the panel learns how tall
// its content was, draws the scroll bar and takes the wheel nobody else used.
void UI::End()
{
	if (m_open) Finish();
	Close();
}

void UI::Close()
{
	if (m_scissor)
	{
		EndScissorMode();
		m_scissor = false;
	}
	if (m_zoomed)
	{
		rlDrawRenderBatchActive();
		rlPopMatrix();
		m_zoomed = false;
		g_zoomScale = 1.0f;
		g_zoomRight = 0.0f;
	}
	m_open = false;
}

void UI::Finish()
{
	const Rect p = m_panel;
	m_contentH = (float)(m_y - p.y + (int)std::lround(m_scroll) + PAD);   // where the widgets ended, before the scroll moved them
	if (m_scissor)
	{
		EndScissorMode();
		m_scissor = false;
	}
	float room = m_contentH - p.h;
	if (room <= 0)
	{
		m_scroll = 0.0f;
		m_drag = -1.0f;
		return;
	}
	if (m_wheel != 0 && !m_wheelUsed && OverPanel()) m_scroll = std::min(std::max(m_scroll - m_wheel * 48, 0.0f), room);
	Rect track{p.x + p.w - 6, p.y, 6, p.h};
	float thumbH = std::max((float)p.h * p.h / m_contentH, 24.0f);
	float thumbY = p.y + (p.h - thumbH) * m_scroll / room;
	DrawRectangle(track.x, track.y, track.w, track.h, RL(SCROLLBAR_TRACK));
	DrawRectangle(track.x, (int)thumbY, track.w, (int)thumbH, RL(SCROLLBAR_THUMB));
	if (m_click && Inside(track)) m_drag = (thumbY <= m_my && m_my <= thumbY + thumbH) ? m_my - thumbY : thumbH / 2;
	if (m_drag >= 0)
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			m_scroll = std::min(std::max((m_my - m_drag - p.y) / std::max(p.h - thumbH, 1.0f) * room, 0.0f), room);
		else
			m_drag = -1.0f;
	}
}

void UI::Begin(int x, int y, int w, int h)
{
	LoadTheme();
	Close();
	int h0 = h;
	if (m_focus == 0 && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)))
	{
		if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) SetScale(g_scale * 1.1f);
		else if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) SetScale(g_scale / 1.1f);
		else if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0)) SetScale(1.0f);
	}
	Vector2 m = GetMousePosition();
	float s = g_scale;
	float right = (float)(x + w);
	m_mx = m.x;
	m_my = m.y;
	if (s != 1.0f)
	{
		rlDrawRenderBatchActive();
		rlPushMatrix();
		rlTranslatef(right, 0.0f, 0.0f);
		rlScalef(s, s, 1.0f);
		rlTranslatef(-right, 0.0f, 0.0f);
		g_zoomScale = s;
		g_zoomRight = right;
		m_zoomed = true;
		m_mx = right - (right - m.x) / s;
		m_my = m.y / s;
		h = (int)std::ceil(h / s);
	}
	m_panel = Rect{x, y, w, h};
	m_click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && OverPanel();
	m_rightClick = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && OverPanel();
	m_doubleClick = false;
	if (m_click)
	{
		const double t = GetTime();
		m_doubleClick = m_lastClickT >= 0.0 && t - m_lastClickT < 0.4 && std::fabs(m_mx - m_lastClickX) < 5 && std::fabs(m_my - m_lastClickY) < 5;
		m_lastClickT = m_doubleClick ? -1.0 : t;   // a third click starts a new pair
		m_lastClickX = m_mx;
		m_lastClickY = m_my;
	}
	// whole notches only: smooth or high-resolution wheels report fractions or large values per frame
	if (OverPanel())
	{
		m_wheelAcc += GetMouseWheelMove();
		int steps = (int)m_wheelAcc;
		m_wheelAcc -= steps;
		m_wheel = (float)std::max(-1, std::min(1, steps));
	}
	else
	{
		m_wheelAcc = 0.0f;
		m_wheel = 0.0f;
	}
	m_shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
	m_alt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
	m_chars.clear();
	for (int c; (c = GetCharPressed()) != 0;) m_chars.push_back((unsigned)c);
	m_enter = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
	m_escape = IsKeyPressed(KEY_ESCAPE);
	m_tab = IsKeyPressed(KEY_TAB);
	m_backspace = IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE);
	static const int kNav[8] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_PAGE_UP, KEY_PAGE_DOWN, KEY_HOME, KEY_END};
	for (int i = 0; i < 8; i++)                 // navigation keys, held ones repeating; only while nothing is being typed
		m_nav[i] = m_focus == 0 && (IsKeyPressed(kNav[i]) || IsKeyPressedRepeat(kNav[i]));
	DrawRectangle(x, y, w, h, RL(PANEL_BG));
	DrawLine(x, y, x, y + h, RL(PANEL_BORDER));
	m_open = true;
	m_wheelUsed = false;
	float room = std::max(m_contentH - h, 0.0f);
	m_scroll = std::min(std::max(m_scroll, 0.0f), room);
	if (room > 0)                               // taller than the panel: clip to it (screen pixels, the zoom does not apply)
	{
		BeginScissorMode((int)std::lround(right - w * s), (int)std::lround(y * s), (int)std::lround(w * s), h0);
		m_scissor = true;
	}
	m_y = y + PAD - (int)std::lround(m_scroll);
}

// ----------------------------------------------------------------------------------------------------------------- layout

enum { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT, NAV_PAGEUP, NAV_PAGEDOWN, NAV_HOME, NAV_END };

Rect UI::Row(int height, int gap)
{
	Rect r{m_panel.x + PAD, m_y, m_panel.w - 2 * PAD, height};
	m_y += height + gap;
	return r;
}

std::vector<Rect> UI::Split(const Rect& r, const std::vector<float>& weights, int gap)
{
	float total = 0.0f;
	for (float w : weights) total += w;
	int usable = r.w - gap * ((int)weights.size() - 1);
	std::vector<Rect> out;
	int x = r.x;
	for (float wt : weights)
	{
		int w = (int)(usable * wt / total);
		out.push_back(Rect{x, r.y, w, r.h});
		x += w + gap;
	}
	return out;
}

// ---------------------------------------------------------------------------------------------------------------- widgets

// text shortened with ... to fit width (the default font has no ellipsis glyph)
std::string UI::Fit(const std::string& text, int size, int width)
{
	if (MeasureText(text, size) <= width) return text;
	std::string t = text;
	while (t.size() > 1 && MeasureText(t + "...", size) > width) t.pop_back();
	return t + "...";
}

void UI::Text(const Rect& r, const std::string& text, const Rgba& c, int size, bool center)
{
	std::string t = Fit(text, size, r.w - 8);   // a box never draws past its edge
	int width = MeasureText(t, size);
	int x = center ? r.x + (int)std::floor((r.w - width) / 2.0) : r.x + 4;
	DrawText(t, x, r.y + (int)std::floor((r.h - size) / 2.0), size, c);
}

// A line of text; on its own row it wraps onto more rows when it is wider than the panel.
void UI::Label(const std::string& text, const Rgba* color)
{
	std::vector<std::string> lines = Wrap(text, 14, m_panel.w - 2 * PAD - 8);
	for (const std::string& line : lines) Text(Row(lines.size() == 1 ? ROW : 18), line, color ? *color : Color(TEXT));
}
void UI::Label(const Rect& r, const std::string& text, const Rgba* color) { Text(r, text, color ? *color : Color(TEXT)); }

void UI::Heading(const std::string& text)
{
	Rect r = Row(20);
	std::string up = text;
	for (char& c : up) c = (char)std::toupper((unsigned char)c);
	Text(r, up, Color(HEADING), 12);
	DrawLine(r.x, r.y + r.h - 1, r.x + r.w, r.y + r.h - 1, RL(HEADING_LINE));
}

bool UI::Button(const Rect& r, const std::string& text, bool enabled, bool active)
{
	bool hot = enabled && Inside(r);
	DrawRectangle(r.x, r.y, r.w, r.h, RL(active ? BUTTON_ACTIVE : hot ? BUTTON_HOT : BUTTON));
	DrawRectangleLines(r.x, r.y, r.w, r.h, RL(BUTTON_BORDER));
	int size = 13;
	while (size > 10 && MeasureText(text, size) > r.w - 8) size--;   // a label too wide for its button gets smaller before it is cut
	Text(r, text, Color(enabled ? BUTTON_TEXT : DIM), size, true);
	return hot && m_click;
}

bool UI::Checkbox(const Rect& r, const std::string& text, bool value)
{
	Rect box{r.x + 2, r.y + 3, 16, 16};
	DrawRectangle(box.x, box.y, box.w, box.h, RL(FIELD));
	DrawRectangleLines(box.x, box.y, box.w, box.h, RL(FIELD_BORDER));
	if (value) DrawRectangle(box.x + 4, box.y + 4, 8, 8, RL(GOOD));
	Text(Rect{r.x + 22, r.y, r.w - 22, r.h}, text, Color(TEXT));
	if (Inside(r) && m_click) return !value;
	return value;
}

// Click or scroll to step through options; right click steps back.
int UI::Choice(const Rect& r, Id id, const std::string& label, const std::vector<std::string>& options, int index)
{
	(void)id;
	Rect box = r;
	if (!label.empty())
	{
		std::vector<Rect> cells = Split(r, {0.4f, 0.6f});
		Text(cells[0], label, Color(DIM));
		box = cells[1];
	}
	bool hot = Inside(box);
	DrawRectangle(box.x, box.y, box.w, box.h, RL(hot ? FIELD_HOT : FIELD));
	DrawRectangleLines(box.x, box.y, box.w, box.h, RL(FIELD_BORDER));
	int n = (int)options.size();
	index = std::min(std::max(index, 0), std::max(n - 1, 0));
	Text(box, n ? options[index] : "", Color(TEXT), 13);
	if (hot && n)
	{
		auto wrap = [n](int i) { return ((i % n) + n) % n; };
		if (m_click) return wrap(index + 1);
		if (m_rightClick) return wrap(index - 1);
		if (m_wheel != 0)
		{
			m_wheelUsed = true;
			return wrap(index + (m_wheel > 0 ? 1 : -1));
		}
		if (m_nav[NAV_RIGHT]) return wrap(index + 1);
		if (m_nav[NAV_LEFT]) return wrap(index - 1);
	}
	return index;
}

double UI::Number(const Rect& r, Id id, const std::string& label, double value, double step, const char* fmt, bool integer, const double* minimum)
{
	Rect box = r;
	if (!label.empty())
	{
		std::vector<Rect> cells = Split(r, {0.42f, 0.58f});
		Text(cells[0], label, Color(DIM));
		box = cells[1];
	}
	bool hot = Inside(box);
	bool editing = m_focus == id;
	DrawRectangle(box.x, box.y, box.w, box.h, RL(editing ? FIELD_ACTIVE : hot ? FIELD_HOT : FIELD));
	DrawRectangleLines(box.x, box.y, box.w, box.h, RL(FIELD_BORDER));
	char shown[64];
	if (editing) std::snprintf(shown, sizeof shown, "%s_", m_buf.c_str());
	else if (integer) std::snprintf(shown, sizeof shown, "%ld", std::lround(value));
	else std::snprintf(shown, sizeof shown, fmt, value);
	Text(box, shown, Color(TEXT), 13);
	auto clampMin = [&](double v) { return minimum != nullptr ? std::max(v, *minimum) : v; };

	if (editing)
	{
		if (!m_chars.empty() || m_backspace)
		{
			if (m_fresh)                        // the first key replaces the whole text
			{
				m_buf.clear();
				m_fresh = false;
			}
			for (unsigned c : m_chars)
				if (c < 128 && std::string("0123456789.-+eE").find((char)c) != std::string::npos && m_buf.size() < 14) m_buf += (char)c;
			if (m_backspace && !m_buf.empty()) m_buf.pop_back();
		}
		if (m_escape)
		{
			m_focus = 0;
			return value;
		}
		if (m_enter || m_tab || (m_click && !hot))   // Enter, Tab or a click elsewhere applies it
		{
			m_focus = 0;
			char* end = nullptr;
			double v = std::strtod(m_buf.c_str(), &end);
			if (m_buf.empty() || end == m_buf.c_str() || *end != 0) return value;
			if (integer) v = (double)std::lround(v);
			return clampMin(v);
		}
		return value;
	}

	if (hot)
	{
		if (m_click)
		{
			char buf[64];
			if (integer) std::snprintf(buf, sizeof buf, "%ld", std::lround(value));
			else std::snprintf(buf, sizeof buf, "%g", value);
			m_focus = id;
			m_buf = buf;
			m_fresh = true;
		}
		else if (m_wheel != 0 || m_nav[NAV_UP] || m_nav[NAV_DOWN] || m_nav[NAV_LEFT] || m_nav[NAV_RIGHT])
		{
			m_wheelUsed = m_wheelUsed || m_wheel != 0;
			double mult = m_shift ? 10.0 : m_alt ? 0.1 : 1.0;
			bool up = m_wheel > 0 || m_nav[NAV_UP] || m_nav[NAV_RIGHT];
			double v = value + (up ? 1 : -1) * step * mult;
			if (integer) v = (double)std::lround(v);
			return clampMin(v);
		}
	}
	return value;
}

void UI::Vec3(const Rect& r, Id id, const std::string& label, float v[3], double step, const char* fmt)
{
	std::vector<Rect> cells = label.empty() ? Split(r, {1, 1, 1}) : Split(r, {0.34f, 0.22f, 0.22f, 0.22f});
	int base = label.empty() ? 0 : 1;
	if (!label.empty()) Text(cells[0], label, Color(DIM));
	for (int k = 0; k < 3; k++) v[k] = (float)Number(cells[base + k], SubId(id, k), "", v[k], step, fmt);
}

// icons: a texture (id 0 = none) per row, drawn left-aligned before the text.
// Up / Down (PageUp / PageDown, Home / End) move through the list the mouse is over, or the one it was last over while it stays on the panel. With
// confirm the keys only move a cursor and Enter selects the row (for lists where selecting does something heavy); a click always selects.
int UI::ListBox(Id id, const Rect& r, const std::vector<ListRow>& rows, int selected, bool reveal, const std::vector<Texture>* icons, bool confirm,
                bool* activated)
{
	if (activated != nullptr) *activated = false;
	int count = (int)rows.size();
	int n = std::max(r.h / ROW, 1);
	int top = m_top.count(id) ? m_top[id] : 0;
	top = std::min(std::max(top, 0), std::max(count - n, 0));
	auto shown = m_shown.find(id);
	if (reveal && selected >= 0 && (shown == m_shown.end() || shown->second != selected))   // only when the selection changed:
	{
		m_shown[id] = selected;                                                              // scrolling away must not snap back
		if (selected < top) top = selected;
		else if (selected >= top + n) top = selected - n + 1;
	}
	bool hot = Inside(r);
	if (hot && m_wheel != 0)
	{
		m_wheelUsed = true;
		top = std::min(std::max(top - (int)m_wheel * 2, 0), std::max(count - n, 0));
	}
	if (hot) m_activeList = id;
	int cursor = -1;
	if (confirm)
	{
		cursor = m_cursor.count(id) ? m_cursor[id] : -1;
		if (!(0 <= cursor && cursor < count)) cursor = selected;
	}
	bool keys = m_nav[NAV_UP] || m_nav[NAV_DOWN] || m_nav[NAV_PAGEUP] || m_nav[NAV_PAGEDOWN] || m_nav[NAV_HOME] || m_nav[NAV_END];
	if (keys && m_activeList == id && OverPanel() && count)
	{
		int moved = confirm ? cursor : selected;
		if (m_nav[NAV_DOWN]) moved += 1;
		if (m_nav[NAV_UP]) moved -= 1;
		if (m_nav[NAV_PAGEDOWN]) moved += n;
		if (m_nav[NAV_PAGEUP]) moved -= n;
		if (m_nav[NAV_HOME]) moved = 0;
		if (m_nav[NAV_END]) moved = count - 1;
		moved = std::min(std::max(moved, 0), count - 1);
		if (moved != (confirm ? cursor : selected))
		{
			if (confirm) cursor = moved;
			else selected = moved;
			if (moved < top) top = moved;   // keep the row the keys reached in view
			else if (moved >= top + n) top = moved - n + 1;
			m_shown[id] = selected;         // the reveal rule above must not undo it
		}
	}
	if (confirm && m_enter && m_focus == 0 && m_activeList == id && OverPanel() && 0 <= cursor && cursor < count) selected = cursor;
	if (confirm) m_cursor[id] = cursor;
	DrawRectangle(r.x, r.y, r.w, r.h, RL(FIELD));
	DrawRectangleLines(r.x, r.y, r.w, r.h, RL(FIELD_BORDER));
	for (int k = 0; k < n; k++)
	{
		int i = top + k;
		if (i >= count) break;
		Rect cell{r.x, r.y + k * ROW, r.w - 6, ROW};
		if (i == selected) DrawRectangle(cell.x, cell.y, cell.w, cell.h, RL(LIST_SELECTED));
		else if (Inside(cell)) DrawRectangle(cell.x, cell.y, cell.w, cell.h, RL(LIST_HOVER));
		if (confirm && i == cursor) DrawRectangleLines(cell.x, cell.y, cell.w, cell.h, RL(TEXT));
		const Texture* tex = icons != nullptr && i < (int)icons->size() && (*icons)[i].id != 0 ? &(*icons)[i] : nullptr;
		if (tex != nullptr)
		{
			float side = ROW - 2;
			Texture2D t{tex->id, tex->width, tex->height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
			DrawTexturePro(t, Rectangle{0, 0, (float)t.width, (float)t.height}, Rectangle{(float)cell.x + 2, (float)cell.y + 1, side, side},
			               Vector2{0, 0}, 0.0f, ::Color{255, 255, 255, 255});
			Text(Rect{cell.x + ROW + 2, cell.y, cell.w - ROW - 2, cell.h}, rows[i].text, rows[i].color, 13);
		}
		else
			Text(cell, rows[i].text, rows[i].color, 13);
		if (Inside(cell) && m_click)
		{
			selected = i;
			if (confirm) m_cursor[id] = i;
			if (m_doubleClick && activated != nullptr) *activated = true;
		}
	}
	if (count > n)                              // scroll bar
	{
		int barH = std::max(r.h * n / count, 12);
		int barY = r.y + (r.h - barH) * top / std::max(count - n, 1);
		DrawRectangle(r.x + r.w - 5, barY, 4, barH, RL(SCROLLBAR_THUMB));
	}
	m_top[id] = top;
	return selected;
}

int UI::Tabs(const std::vector<std::string>& labels, int active)
{
	Rect r = Row(24);
	const int count = (int)labels.size(), gap = 2;
	auto width = [&](int i) { return std::max(MeasureText(labels[i], 13) + 16, 48); };
	int total = 0;
	for (int i = 0; i < count; i++) total += width(i) + (i ? gap : 0);
	if (total <= r.w)
	{
		m_tabFirst = 0;
		std::vector<Rect> cells = Split(r, std::vector<float>(count, 1.0f), gap);
		for (int i = 0; i < count; i++)
			if (Button(cells[i], labels[i], true, i == active)) active = i;
		return active;
	}
	// too wide: < and > at the ends, between them as many whole tabs from m_tabFirst as fit (stretched to fill)
	const int arrow = 18;
	Rect mid{r.x + arrow + gap, r.y, r.w - 2 * (arrow + gap), r.h};
	auto lastFrom = [&](int first) {   // the last tab that fits when the row starts at first
		int used = 0, last = first;
		for (int i = first; i < count; i++)
		{
			used += width(i) + (i > first ? gap : 0);
			if (used > mid.w && i > first) break;
			last = i;
		}
		return last;
	};
	m_tabFirst = std::min(std::max(m_tabFirst, 0), count - 1);
	if (active != m_tabActive && 0 <= active && active < count)   // a newly active tab scrolls into view (scrolling away stays put)
	{
		m_tabActive = active;
		if (active < m_tabFirst) m_tabFirst = active;
		while (lastFrom(m_tabFirst) < active) m_tabFirst++;
	}
	while (m_tabFirst > 0 && lastFrom(m_tabFirst - 1) == count - 1) m_tabFirst--;   // no empty space at the end
	int step = 0;
	if (Inside(r) && m_wheel != 0)
	{
		m_wheelUsed = true;
		step = m_wheel > 0 ? -1 : 1;
	}
	const int last = lastFrom(m_tabFirst);
	if (Button(Rect{r.x, r.y, arrow, r.h}, "<", m_tabFirst > 0)) step = -1;
	if (Button(Rect{r.x + r.w - arrow, r.y, arrow, r.h}, ">", last < count - 1)) step = 1;
	if (step < 0 && m_tabFirst > 0) m_tabFirst--;
	else if (step > 0 && last < count - 1) m_tabFirst++;
	std::vector<float> weights;
	for (int i = m_tabFirst; i <= last; i++) weights.push_back((float)width(i));
	std::vector<Rect> cells = Split(mid, weights, gap);
	for (int i = m_tabFirst; i <= last; i++)
		if (Button(cells[i - m_tabFirst], labels[i], true, i == active)) active = i;
	return active;
}

// A text box in the UI's style (click to type, Enter / Tab / click elsewhere applies, Esc cancels). Returns the (new) text.
std::string UI::TextInput(const Rect& r, Id id, const std::string& label, const std::string& text, const std::string& hint)
{
	Rect box = r;
	if (!label.empty())
	{
		std::vector<Rect> cells = Split(r, {0.3f, 0.7f});
		Text(cells[0], label, Color(DIM));
		box = cells[1];
	}
	bool editing = m_focus == id;
	bool hot = Inside(box);
	bool ascii = std::all_of(text.begin(), text.end(), [](char c) { return (unsigned char)c < 128; });
	DrawRectangle(box.x, box.y, box.w, box.h, RL(editing ? FIELD_ACTIVE : hot ? FIELD_HOT : FIELD));
	DrawRectangleLines(box.x, box.y, box.w, box.h, RL(FIELD_BORDER));
	std::string shown = editing ? m_buf + "_" : text;
	if (!editing && !ascii)
		for (char& c : shown)
			if ((unsigned char)c >= 128) c = '?';
	Text(box, shown, Color(TEXT), 13);
	if (editing)
	{
		for (unsigned c : m_chars)
		{
			if (32 <= c && c < 127 && m_buf.size() < 120)
			{
				if (m_fresh)
				{
					m_buf.clear();
					m_fresh = false;
				}
				m_buf += (char)c;
			}
		}
		if (m_backspace)
		{
			m_fresh = false;
			if (!m_buf.empty()) m_buf.pop_back();
		}
		if (m_escape) m_focus = 0;
		else if (m_enter || m_tab || (m_click && !hot))
		{
			m_focus = 0;
			return m_buf;
		}
	}
	else if (hot && m_click)
	{
		m_focus = id;
		m_buf = ascii ? text : "";
		m_fresh = true;
	}
	if (!hint.empty() && hot && !editing) DrawText(hint, box.x, box.y + box.h + 2, 12, Rgba{255, 230, 160, 255});
	return text;
}

// ------------------------------------------------------------------------------------------------------------- dev window
namespace
{
struct Panel { std::string title; std::function<void(UI&)> fn; std::function<bool()> visible; };
std::vector<Panel> g_panels;
UI g_ui;
bool g_enabled = std::getenv("MEGAMIX_DEVUI") != nullptr && std::string(std::getenv("MEGAMIX_DEVUI")) != "0";
bool g_wantsMouse = false;
int g_reserved = 0;   // screen pixels the window was widened by for the panel (ReservedWidth)
std::string g_tab;   // the active tab's title (the one the user picked; the first tab shows while it is hidden)
constexpr int WINDOW_W = 300;                   // panel units

// the dev window's own tab: theme / zoom, and every widget once (to try a theme)
void UiPanel(UI& ui)
{
	ui.Heading("Theme");
	ui.Label(ThemeFile(), &Color(DIM));
	{
		std::vector<Rect> c = UI::Split(ui.Row(), {1, 1});
		if (ui.Button(c[0], "Reload")) LoadTheme(true);
		float z = (float)ui.Number(c[1], MakeId("devui.zoom"), "Zoom", Scale(), 0.1, "%.2f");
		if (z != Scale()) SetScale(z);
	}
	ui.Label("Ctrl + = / - / 0 zoom, F9 hides this window", &Color(DIM));
	ui.Space();
	ui.Heading("Widgets");
	static bool check = true;
	static int choice = 0, list = 3;
	static double num = 1.5, count = 4;
	static float vec[3] = {0, 1, 2};
	static std::string text = "text";
	check = ui.Checkbox(ui.Row(), "Check box", check);
	choice = ui.Choice(ui.Row(), MakeId("devui.choice"), "Choice", {"one", "two", "three"}, choice);
	num = ui.Number(ui.Row(), MakeId("devui.num"), "Number", num);
	static const double zero = 0;
	count = ui.Number(ui.Row(), MakeId("devui.count"), "Integer >= 0", count, 1, "%.0f", true, &zero);
	ui.Vec3(ui.Row(), MakeId("devui.vec"), "Vec3", vec);
	text = ui.TextInput(ui.Row(), MakeId("devui.text"), "Text", text, "click to type");
	ui.Space(16);
	std::vector<ListRow> rows;
	for (int i = 0; i < 20; i++)
		rows.push_back(ListRow{"row " + std::to_string(i), Color(i % 5 == 0 ? WARN : TEXT)});
	list = ui.ListBox(MakeId("devui.list"), ui.Row(ROW * 6), rows, list, true);
	std::vector<Rect> c = UI::Split(ui.Row(), {1, 1, 1});
	ui.Label(c[0], "good", &Color(GOOD));
	ui.Label(c[1], "warn", &Color(WARN));
	ui.Label(c[2], "bad", &Color(BAD));
}
}

void RegisterPanel(const std::string& title, std::function<void(UI&)> fn, std::function<bool()> visible)
{
	for (Panel& p : g_panels)
		if (p.title == title)
		{
			p.fn = std::move(fn);
			p.visible = std::move(visible);
			return;
		}
	g_panels.push_back(Panel{title, std::move(fn), std::move(visible)});
}
void SetEnabled(bool on) { g_enabled = on; }
bool Enabled() { return g_enabled; }
bool g_captureMouse = false, g_captureKeys = false;     // asked by a tool this frame (CaptureInput) / in force (read by the game)
bool g_wantMouse = false, g_wantKeys = false;
bool Typing() { return g_enabled && (g_ui.Typing() || g_captureKeys); }
bool WantsMouse() { return g_enabled && (g_wantsMouse || g_captureMouse); }
void CaptureInput(bool mouse, bool keys) { g_wantMouse = g_wantMouse || mouse; g_wantKeys = g_wantKeys || keys; }

Input GetInput()
{
	Input in{};
	const Vector2 m = GetMousePosition(), d = GetMouseDelta();
	in.mx = m.x; in.my = m.y; in.dx = d.x; in.dy = d.y;
	in.wheel = GetMouseWheelMove();
	const int buttons[3] = {MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE};
	for (int i = 0; i < 3; i++)
	{
		in.down[i] = IsMouseButtonDown(buttons[i]);
		in.pressed[i] = IsMouseButtonPressed(buttons[i]);
		in.released[i] = IsMouseButtonReleased(buttons[i]);
	}
	in.ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
	in.shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
	in.alt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
	return in;
}

bool KeyPressed(int key)
{
	if (g_ui.Typing()) return false;
	switch (key)
	{
	case K_DELETE: return IsKeyPressed(KEY_DELETE);
	case K_BACKSPACE: return IsKeyPressed(KEY_BACKSPACE);
	case K_ENTER: return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
	case K_ESCAPE: return IsKeyPressed(KEY_ESCAPE);
	case K_PAGE_UP: return IsKeyPressed(KEY_PAGE_UP);
	case K_PAGE_DOWN: return IsKeyPressed(KEY_PAGE_DOWN);
	default: return key > 0 && key < 256 && IsKeyPressed(key);   // raylib's letter / digit codes are their ASCII upper case
	}
}

Rect GameView()
{
	const int w = GetScreenWidth() - g_reserved, h = GetScreenHeight();
	const int vw = std::min(w, h * 4 / 3), vh = vw * 3 / 4;
	return Rect{(w - vw) / 2, (h - vh) / 2, vw, vh};
}
int ReservedWidth() { return g_reserved; }

void Frame()
{
	if (!g_ui.Typing() && IsKeyPressed(KEY_F9)) g_enabled = !g_enabled;
	SetExitKey(Typing() ? KEY_NULL : KEY_ESCAPE);   // Esc cancels a field instead of closing the window
	// the window grows by the panel's width (F9, zoom) so the game keeps its own size beside it; takes effect next frame
	const int want = g_enabled ? (int)std::ceil(WINDOW_W * Scale()) : 0;
	if (want != g_reserved && !IsWindowFullscreen())
	{
		SetWindowSize(GetScreenWidth() - g_reserved + want, GetScreenHeight());
		g_reserved = want;
	}
	g_captureMouse = g_wantMouse;   // what the tools asked for last frame holds until they draw again
	g_captureKeys = g_wantKeys;
	g_wantMouse = g_wantKeys = false;
	if (!g_enabled)
	{
		g_wantsMouse = false;
		return;
	}
	int sw = GetScreenWidth(), sh = GetScreenHeight();
	std::vector<std::string> labels;
	std::vector<const Panel*> shown;
	for (const Panel& p : g_panels)
		if (!p.visible || p.visible())
		{
			labels.push_back(p.title);
			shown.push_back(&p);
		}
	labels.push_back("UI");
	int tab = 0;
	for (size_t i = 0; i < labels.size(); i++)
		if (labels[i] == g_tab) tab = (int)i;
	g_ui.Begin(sw - WINDOW_W, 0, WINDOW_W, sh);
	const int picked = g_ui.Tabs(labels, tab);
	if (picked != tab) g_tab = labels[picked];
	g_ui.Space(4);
	if (picked < (int)shown.size()) shown[picked]->fn(g_ui);
	else UiPanel(g_ui);
	g_ui.End();
	g_wantsMouse = GetMousePosition().x >= sw - WINDOW_W * Scale();
}
}
