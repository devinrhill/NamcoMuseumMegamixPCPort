#include "gotch/Menu.hpp"

#include "gotch/Draw.hpp"
#include "gotch/GameIO.hpp"
#include "gotch/SWork.hpp"

#include <cstdio>
#include <cstring>
#include <string>

namespace gotch
{
namespace
{
// the pad words a CMENU reads: per pad hi | lo << 4 of a CONTROLLER_VALUE pair (high word +0, low word +4), folded with >> 16
std::uint32_t MenuWord(std::uint64_t v)
{
	std::uint32_t w = (std::uint32_t)(v >> 32) | (std::uint32_t)v << 4;
	return w | w >> 16;
}

// the pad clear WII_MENU inlines into ChangeMenu / MenuJob (CGAMEIO +0x20 = 3, every pad's words and analog values zeroed)
void ClearPads(CGameIO& io)
{
	io.invalidFrames = 3;
	for (int i = 0; i < io.PadCount(); i++)
	{
		ControllerValue& p = io.Pad(i);
		p.way4 = 0;
		p.edge = 0;
		p.value = 0;
		p.raw = 0;
		p.oldWay4 = 0;
		p.old = 0;
		p.edgeWay4 = 0;
		p.repeat = 0;
		std::memset(p.analog, 0, sizeof(p.analog));
	}
}
}

// ---------------------------------------------------------------------------------------------------------------------------- MENUITEM

// RSO2 0x80d28534 (prev / next = 0 at every call site)
MenuItem::MenuItem(const MenuItemInfo& info_, const char* text_) : info(info_), text(text_)
{
}

// RSO2 0x80d28684
MenuItem::~MenuItem()
{
	DeleteItemAll();
}

// RSO2 0x80d287bc
bool MenuItem::AddItem(MenuItem* item, int at)
{
	if (count == 0)
	{
		first = item;
		item->next = nullptr;
		first->prev = nullptr;
		count++;
		return true;
	}
	MenuItem* p = first;
	for (int i = 0; i < count; i++)
	{
		if (i == at)
		{
			item->prev = p->prev;
			item->next = p;
			if (p->prev == nullptr)
				first = item;
			else
			{
				p->prev->next = item;
				// the module stores into p->next's prev here, not p's (kept)
				if (p->next != nullptr) p->next->prev = item;
			}
			count++;
			return true;
		}
		if (p->next != nullptr) p = p->next;
	}
	item->prev = p;
	item->next = nullptr;
	p->next = item;
	count++;
	return true;
}

// RSO2 0x80d28940 (MenuMM's slot table is not kept: no ported item has an id)
bool MenuItem::DeleteItemAll()
{
	MenuItem* p = first;
	for (int i = 0; i < count; i++)
	{
		MenuItem* n = p->next != nullptr ? p->next : p;
		delete p;
		p = n;
	}
	count = 0;
	first = nullptr;
	cursor = 0;
	return true;
}

// RSO2 0x80d28e90 (stays on the last node when the list is shorter than n; null when n >= count)
MenuItem* MenuItem::GetItemByNum(int n) const
{
	MenuItem* p = first;
	for (int i = 0; i < count; i++)
	{
		if (i == n) return p;
		if (p->next != nullptr) p = p->next;
	}
	return nullptr;
}

// the inlined "step until enabled" loop (see Menu.hpp)
bool MenuItem::StepToEnabled(int dir)
{
	bool wrapped = false;
	int i;
	for (i = 0; i < count; i++)
	{
		if (SetItemCursor(cursor + dir)) wrapped = true;
		MenuItem* p = GetItemByNum(cursor);
		if (p == nullptr) break;
		if (p->enabled) break;
	}
	if (i == count) wrapped = true;
	return wrapped;
}

// RSO2 0x80d28ed0
bool MenuItem::IncItemCursor()
{
	if (!Current()->IncNumber()) return false;
	return StepToEnabled(1);
}

// RSO2 0x80d291c0
bool MenuItem::SetItemCursor(int n)
{
	if (count == 0)
	{
		cursor = 0;
		return false;
	}
	bool r;
	if (!wrap)
	{
		r = n < 0;
		if (r) n = 0;
		if (count <= n)
		{
			n = count - 1;
			r = true;
		}
	}
	else
	{
		r = n < 0;
		if (r) n = count - 1;
		if (count <= n)
		{
			n = 0;
			r = true;
		}
	}
	cursor = n;
	return r;
}

// ------------------------------------------------------------------------------------------------------------------------- NUMITEMLIST

// RSO2 0x80d2923c (+0x74 = 1, +0x75 = 0, +0x76 = 1; vtable +0x30 / +0x34 / +0x38 = SetNumberEx_Min / _Max / _Num)
NumItemList::NumItemList(const MenuItemInfo& info_, const char* format, int number_, int min_, int max_) : MenuItem(info_, format)
{
	enabled = true;
	unk75 = false;
	wrap = true;
	min = min_;
	max = max_;
	number = number_;
}

std::string FormatInt(const char* format, int n)
{
	std::string out;
	for (const char* c = format != nullptr ? format : ""; *c != 0; c++)
	{
		if (c[0] != '%')
		{
			out += *c;
			continue;
		}
		if (c[1] == '%')
		{
			out += '%';
			c++;
			continue;
		}
		const char* d = c + 1;
		const bool zero = *d == '0';
		if (zero) d++;
		int width = 0;
		while (*d >= '0' && *d <= '9') width = width * 10 + (*d++ - '0');
		if (*d != 'd')
		{
			out += *c;   // not a conversion this port knows: kept as text
			continue;
		}
		std::string v = std::to_string(n < 0 ? -n : n);
		const std::size_t sign = n < 0 ? 1 : 0;
		if ((int)(v.size() + sign) < width) v.insert(0, (std::size_t)width - v.size() - sign, zero ? '0' : ' ');
		if (n < 0) v.insert(zero ? 0 : v.find_first_not_of(' '), 1, '-');
		out += v;
		c = d;
	}
	return out;
}

// RSO2 0x80d295b0: sprintf_s(buf, size, format, number); the format is module data (FormatInt)
const char* NumItemList::GetText() const
{
	std::snprintf(buf, sizeof(buf), "%s", FormatInt(text, number).c_str());
	return buf;
}

// the body IncNumber (RSO2 0x80d2963c) and DecNumber (RSO2 0x80d29708) share: true = no range or the number wrapped / clamped
bool NumItemList::Step(int d)
{
	if (GetRange() == 0) return true;
	bool r = false;
	number += d;
	if (max <= number)
	{
		number = wrap ? min : max - 1;
		r = true;
	}
	if (number < min)
	{
		number = wrap ? max - 1 : min;
		r = true;
	}
	return r;
}

bool NumItemList::IncNumber() { return Step(1); }
bool NumItemList::DecNumber() { return Step(-1); }

// ------------------------------------------------------------------------------------------------------------------------------- CMENU

// RSO2 0x80d29900 (the info words are copied as by MENUITEM's ctor; prev / next = 0)
CMenu::CMenu(CDraw& draw_, CGameIO& gameio_, const MenuItemInfo& info_, const char* title) : MenuItem(info_, title), draw(draw_), gameio(gameio_)
{
	blinkOn = 0x1e;
	blinkEnd = 0x3c;
}

// RSO2 0x80d29d38
int CMenu::MenuJob(bool readPads)
{
	if (styleActive)
	{
		result = 0;
		return 0;
	}
	if (!blinkEnabled)
	{
		blinkPhase = true;
		blinkCount = 0;
	}
	else
	{
		if (blinkOn < blinkCount)
		{
			if (blinkEnd < blinkCount)
				blinkCount = 0;
			else
				blinkPhase = false;
		}
		else
			blinkPhase = true;
		blinkCount++;
	}
	// pads 0 and 1: edge (4-way) and auto-repeat words; nothing while CGAMEIO's invalid frames count down
	std::uint32_t edge[2], rep[2];
	for (int i = 0; i < 2; i++)
	{
		edge[i] = gameio.invalidFrames == 0 ? MenuWord(gameio.Pad(i).edgeWay4) : 0;
		rep[i] = gameio.invalidFrames == 0 ? MenuWord(gameio.Pad(i).repeat) : 0;
		if (!readPads) edge[i] = rep[i] = 0;
	}
	std::uint32_t w = edge[0] | edge[1];
	if (useRepeat)
	{
		for (int i = 0; i < gameio.PadCount(); i++) gameio.Pad(i).repeatTable = repeatTable;
		w = rep[0] | rep[1];
	}
	result = 0;
	last = GetItemByNum(cursor);
	for (std::uint32_t bit : {0x10u, 0x40u, 0x80u, 0x20u})
		if (w & bit) result |= bit;
	// up: the item's DecNumber, then back to the previous enabled item
	if (moveMask & w & 0x10)
	{
		MenuItem* p = GetItemByNum(cursor);
		if (p != nullptr && p->DecNumber()) StepToEnabled(-1);
		timer = 0;
	}
	// down
	if (moveMask & w & 0x40)
	{
		MenuItem* p = GetItemByNum(cursor);
		if (p != nullptr && p->IncNumber()) StepToEnabled(1);
		timer = 0;
	}
	for (std::uint32_t bit : {0x1000u, 0x2000u, 0x4000u, 0x8000u, 0x100u, 0x200u, 0x400u, 0x800u, 8u, 1u})
		if (w & bit) result |= bit;
	MenuItem* cur = GetItemByNum(cursor);
	if (cur == nullptr) return 0;   // the module would read through null (never: every ported menu has items)
	// the cursor sits on a disabled item: move on to the next enabled one
	if (!cur->enabled)
	{
		MenuItem* p = GetItemByNum(cursor);
		if (p->IncNumber()) StepToEnabled(1);
	}
	changed = cur;
	if (last != cur)
	{
		if (last != nullptr && last->info.leave != nullptr) last->info.leave();
		if (changed->info.enter != nullptr) changed->info.enter();
	}
	changed = nullptr;
	// an item that holds a list of its own (left / right change its value)
	if (cur->count != 0)
	{
		std::uint32_t v = edge[0] | edge[1];
		if (cur->useRepeat)
		{
			for (int i = 0; i < gameio.PadCount(); i++) gameio.Pad(i).repeatTable = cur->repeatTable;
			v = rep[0] | rep[1];
		}
		if (moveMask & v & 0x50) timer = 0;
		if (cur->subCursor)
		{
			if (moveMask & v & 0x80)
			{
				MenuItem* p = cur->GetItemByNum(cur->cursor);
				if (p != nullptr && p->DecNumber()) cur->StepToEnabled(-1);
				timer = 0;
			}
			if (moveMask & v & 0x20)
			{
				MenuItem* p = cur->GetItemByNum(cur->cursor);
				if (p != nullptr && p->IncNumber()) cur->StepToEnabled(1);
				timer = 0;
			}
			MenuItem* s = cur->GetItemByNum(cur->cursor);
			if ((moveMask & v & 0xa0) && (s->info.decideMask & 2) && s->info.decide != nullptr) s->info.decide(s);
		}
		MenuItem* s = cur->GetItemByNum(cur->cursor);
		changed = s;
		if ((padDecide & v & moveMask) && (s->info.decideMask & 1) && s->info.decide != nullptr) s->info.decide(s);
	}
	return 0;
}

// RSO2 0x80d2a970
int CMenu::Render()
{
	if (styleActive && (unsigned)(styleMode - 1) < 2)
	{
		std::uint32_t c = draw.GetPalet(paletOffset, boxColour);
		c = ((std::uint32_t)styleAlpha & 0xff0000u) << 8 | (c & 0xffffff);
		draw.SetPalet(paletOffset, boxColour, &c, 1);
	}
	if (display)
	{
		timer++;
		draw.Begin(6);
		if (box) RenderBox();
		// crossHatch: CMENU::RenderCrossHatch RSO2 0x80d2b0d4 (the screen adjust menus only): not ported
		draw.End();
		if (!styleActive)
		{
			draw.Begin(5);
			draw.PrintfUTF8(info.rect, 10, texture, info.palette, GetText(), (std::uint32_t)rot | info.printFlags, info.lineHeight, nullptr, nullptr);
			MenuItem* p = GetItemByNum(0);
			for (int i = 0; i < count; i++)
			{
				if (p->enabled || p->unk75)
				{
					int pal = p->info.palette;
					if (i == cursor) pal = blinkPhase ? p->info.paletteBlink : p->info.paletteCursor;
					draw.PrintfUTF8(p->info.rect, 10, texture, pal, p->GetText(), (std::uint32_t)rot | p->info.printFlags, p->info.lineHeight, nullptr,
					                nullptr);
					if (p->count != 0)
					{
						MenuItem* s = p->GetItemByNum(p->cursor);
						int spal = s->info.palette;
						if (i == cursor) spal = blinkPhase ? s->info.paletteBlink : s->info.paletteCursor;
						if (s->enabled)
							draw.PrintfUTF8(s->info.rect, 10, texture, spal, s->GetText(), (std::uint32_t)rot | s->info.printFlags, s->info.lineHeight,
							                nullptr, nullptr);
					}
				}
				p = p->next;
			}
			draw.End();
		}
	}
	if (styleActive)
	{
		if ((unsigned)(styleMode - 1) < 2)
		{
			styleAlpha += styleStep;
			if (((std::uint32_t)styleAlpha & 0xff000000u) != 0)
			{
				styleActive = false;
				if (styleMode == 2) enabled = false;
			}
		}
		else if (styleMode == 3)
			styleFn(this);
	}
	return 0;
}

// RSO2 0x80d2ae94: the box corners turned with the screen (sin table RSO2 0x80dce058 = 0, 1, 0, -1), one quad of palette entry boxColour
void CMenu::RenderBox()
{
	static const int kSin[4] = {0, 1, 0, -1};
	const int q = rot / 0x5a;
	const int s0 = kSin[q & 3], c0 = kSin[(q + 1) & 3];
	int x0 = boxRect[0] * c0 - boxRect[1] * s0;
	int y0 = boxRect[0] * s0 + boxRect[1] * c0;
	int x1 = boxRect[2] * c0 - boxRect[3] * s0;
	int y1 = boxRect[2] * s0 + boxRect[3] * c0;
	if (rot == 0x10e)
	{
		y0 += screenH;
		y1 += screenH;
	}
	else if (rot == 0xb4)
	{
		x0 += screenW;
		x1 += screenW;
		y0 += screenH;
		y1 += screenH;
	}
	else if (rot == 0x5a)
	{
		x0 += screenW;
		x1 += screenW;
	}
	const int xy[8] = {x0, y0, x1, y0, x0, y1, x1, y1};
	draw.DrawPoints(xy, 4, paletOffset, boxColour);
}

// RSO2 0x80d2c020, cases 4 (the pause menu: every text centred on the screen axis, the box around the title and the items) and 8 (the start
// menu: the same, but the title is placed without being measured and stays outside the box; an item holding a list is measured by the
// entry under its cursor)
int CMenu::ApplyMenuTemplate(int kind)
{
	if (kind != 2 && kind != 4 && kind != 8) return 1;   // cases 0, 1, 3, 5..7 (the other menus' templates): not ported
	const int right = (rot == 0x5a || rot == 0x10e) ? screenH : screenW;
	if (kind == 2) return OptionsTemplate(right);
	// the bounds start at RSO2 0x80d98260..6c = 0x400, 0x400, -0x400, -0x400
	int bound[4] = {0x400, 0x400, -0x400, -0x400};
	auto measure = [&](MenuItem* m) {
		const int r[4] = {0, m->info.y, right, m->info.y + 0x200};
		int out[4] = {};
		draw.PrintfUTF8(r, 0, 0, 0, m->GetText(), (std::uint32_t)rot | 0x12000, 0xc, nullptr, out);
		std::memcpy(m->info.rect, out, sizeof(out));
		m->info.printFlags = 0x2000;
		if (out[0] < bound[0]) bound[0] = out[0];
		if (out[1] < bound[1]) bound[1] = out[1];
		if (bound[2] < out[2]) bound[2] = out[2];
		if (bound[3] < out[3]) bound[3] = out[3];
	};
	if (kind == 4)
		measure(this);
	else
	{
		const int r[4] = {0, info.y, right, info.y + 0x200};
		std::memcpy(info.rect, r, sizeof(r));
		info.printFlags = 0x2000;
	}
	const int keep = cursor;
	SetItemCursor(0);
	bool wrapped;
	do
	{
		MenuItem* m = GetItemByNum(cursor);
		if (kind == 8 && m->count != 0) m = m->GetItemByNum(m->cursor);
		measure(m);
		wrapped = false;
		if (GetItemByNum(cursor)->IncNumber()) wrapped = StepToEnabled(1);
	} while (!wrapped);
	SetItemCursor(keep);
	boxRect[0] = bound[0] - 8;
	boxRect[2] = bound[2] + 8;
	boxRect[1] = bound[1] - 8;
	boxRect[3] = bound[3] + 4;
	return 1;
}

// RSO2 0x80d2c020 case 2 (the options menus): the title centred at y 0x18 (bounds start RSO2 0x80d98220 = 8); each row's label at x 8
// (RSO2 0x80d98230), its values right-aligned on the same line, or one line lower when they would overlap the label; rows go down from y 0x28
int CMenu::OptionsTemplate(int right)
{
	const int top = cursor;
	const int t[4] = {8, 0x18, right, 0x18};
	std::memcpy(info.rect, t, sizeof(t));
	info.printFlags = 0x2000;
	SetItemCursor(0);
	int y = 0x28;
	const int r8 = right - 8;
	bool wrapped;
	do
	{
		MenuItem* it = GetItemByNum(cursor);
		const int cmd = (int)it->info.command;
		int lh = it->info.lineHeight;
		switch (cmd)
		{
		case 0x30000: case 0x30700: case 0x30800: case 0x30900: case 0x50000: case 0x50100: case 0x50800:
		case 0x58000: case 0x58100: case 0x58200: case 0x58300: case 0x58400: case 0x58500: case 0x58600: case 0x58700:
			lh = 8;
			break;
		case 0x40000: case 0x40100: case 0x40200: case 0x40300:
			lh = 0x10;
			break;
		default:
			break;
		}
		it->info.lineHeight = lh;
		const int keepSub = it->cursor;
		it->SetItemCursor(0);
		int b[4] = {0x400, 0x400, -0x400, -0x400};
		// the values: the full text's top / bottom, the first line's left / right
		bool done = false;
		do
		{
			MenuItem* v = it->GetItemByNum(it->cursor);
			if (v == nullptr) break;
			v->info.lineHeight = lh;
			const int r[4] = {0, y, r8, y + 0x200};
			int o1[4] = {}, o2[4] = {};
			draw.PrintfUTF8(r, 0, 0, 0, v->GetText(), (std::uint32_t)rot | 0x11000, v->info.lineHeight, nullptr, o1);
			std::string line = v->GetText();
			const std::size_t nl = line.find('\n');
			if (nl != std::string::npos) line.resize(nl);
			draw.PrintfUTF8(r, 0, 0, 0, line.c_str(), (std::uint32_t)rot | 0x11000, v->info.lineHeight, nullptr, o2);
			if (o2[0] < b[0]) b[0] = o2[0];
			if (o1[1] < b[1]) b[1] = o1[1];
			if (b[2] < o2[2]) b[2] = o2[2];
			if (b[3] < o1[3]) b[3] = o1[3];
			done = false;
			if (it->GetItemByNum(it->cursor)->IncNumber()) done = it->StepToEnabled(1);
		} while (!done);
		// the label
		const int r[4] = {8, y, r8, y + 0x200};
		int l[4] = {};
		draw.PrintfUTF8(r, 0, 0, 0, it->GetText(), (std::uint32_t)rot | 0x10000, it->info.lineHeight, nullptr, l);
		if (b[0] <= l[2])
		{
			b[1] += 8;
			b[3] += 8;
		}
		SpecialMenuLocationFix(it->info, b, l);
		std::memcpy(it->info.rect, l, sizeof(l));
		it->info.printFlags = 0;
		it->SetItemCursor(0);
		if (b[3] < l[3])
		{
			const int d = ((l[3] - l[1]) - (b[3] - b[1])) / 2;
			b[1] += d;
			b[3] += d;
		}
		done = false;
		do
		{
			MenuItem* v = it->GetItemByNum(it->cursor);
			if (v == nullptr) break;
			v->info.printFlags = 0x1000;
			std::memcpy(v->info.rect, b, sizeof(b));
			done = false;
			if (it->GetItemByNum(it->cursor)->IncNumber()) done = it->StepToEnabled(1);
		} while (!done);
		it->SetItemCursor(keepSub);
		if (b[0] == 0x400) std::memcpy(b, l, sizeof(b));
		if (b[3] < l[3]) b[3] = l[3];
		y = b[3];
		if (cmd == 0x30000 || cmd == 0x30700 || cmd == 0x30800 || cmd == 0x30900) y += 4;
		wrapped = false;
		if (GetItemByNum(cursor)->IncNumber()) wrapped = StepToEnabled(1);
	} while (!wrapped);
	SetItemCursor(top);
	return 1;
}

// RSO2 0x80d2bc94. v = the screen's vertical extent, h = its width (both as the turned screen shows them)
void CMenu::SpecialMenuLocationFix(const MenuItemInfo& item, int* bounds, int* rect) const
{
	const int y0 = rect[1];
	const bool turned = rot == 0x5a || rot == 0x10e;
	const int v = turned ? screenW : screenH;
	const int h = turned ? screenH : screenW;
	auto place = [&](int t, int b) {
		rect[0] = 8;
		rect[2] = h - 8;
		rect[1] = v - t;
		rect[3] = v - b;
	};
	switch ((int)item.command)
	{
	case 0x58000: place(100, 0x5c); break;
	case 0x58100: place(0x54, 0x4c); break;
	case 0x58200: place(0x4c, 0x44); break;
	case 0x58300: place(0x44, 0x3c); break;
	case 0x58400: place(0x3c, 0x34); break;
	case 0x58500: place(0x34, 0x2c); break;
	case 0x58600: place(0x2c, 0x24); break;
	case 0x58700: place(0x24, 0x1c); break;
	case 0x38000: place(0x30, 0x28); break;
	case 0x39000: case 0x62000: case 0x7f000: place(0x24, 0x1c); break;
	case 0x5a000: case 0x5a100: case 0x5a200: case 0x5a300: case 0x5a400: place(0x6c, 100); break;
	case 0x3ff00: case 0x6ff00: case 0x7ff00: case 0xfff00: place(0x18, 0x10); break;
	default: break;
	}
	bounds[1] += rect[1] - y0;
	bounds[3] += rect[1] - y0;
}

// RSO2 0x80d2b9e0 (CDraw +0x180, the palette index mode, is never set for these games: the GetPalet / SetPalet branch)
bool CMenu::SetStyleControl(int mode, int value)
{
	styleActive = true;
	styleMode = mode;
	if (mode == 1)
	{
		if (value == -1) value = 0x1e;
		styleAlpha = 0;
		styleStep = 0xff0000 / value;
	}
	else if (mode == 2)
	{
		if (value == -1) value = 0x1e;
		styleAlpha = 0xff0000;
		styleStep = -(0xff0000 / value);
	}
	else if (mode == 3)
		styleFn = nullptr;   // the callback (value) is a module function: no ported menu uses mode 3
	else
	{
		if (mode != 0) return false;
		std::uint32_t c = draw.GetPalet(paletOffset, boxColour) | (std::uint32_t)value << 24;
		draw.SetPalet(paletOffset, boxColour, &c, 1);
		enabled = true;
		styleActive = false;
	}
	return true;
}

// RSO2 0x80d2b798
void CMenu::SetBlackBox(int l, int t, int r, int b, int unkb8_, int colour, bool pixels)
{
	unkb8 = unkb8_;
	box = true;
	boxColour = colour;
	if (b + t + r + l == 0) box = false;
	const int k = pixels ? 0 : 3;
	boxRect[0] = l << k;
	boxRect[1] = t << k;
	boxRect[2] = r << k;
	boxRect[3] = b << k;
}

// ---------------------------------------------------------------------------------------------------------------------------- WII_MENU

namespace
{
// the info block every *MenuSet starts from (WII_MENU ctor / PauseMenuSet locals)
MenuItemInfo BlankInfo()
{
	MenuItemInfo i;
	i.id = -1;
	i.unk28 = 1;
	i.lineHeight = 0xc;
	i.decideMask = 1;
	return i;
}
}

// RSO2 0x80d48a4c. MENU_KEYCONV_Init (the menu key conversion of the machine) is not ported; the screen adjust / options / maniac /
// debug menus are built empty (ScreenAdjustMenuSet 0x80d4b8d4, ScreenAdjustV2MenuSet 0x80d4bc5c, DebugMenuSet
// 0x80d4cde8, OptionMenuTemplateSet 0x80d4c9f0 and SCREENADJUST: not ported). CDraw +0x180 (palette index mode) is never set: stride 1.
WiiMenu::WiiMenu(SWork& sw_, CDraw& draw_, CGameIO& gameio_, int screenW, int screenH, int rotation, const Strings& str_, std::uint32_t padDecide_,
                 std::uint32_t padCancel_)
    : screenW(screenW), screenH(screenH), rotation(rotation), sw(sw_), draw(draw_), gameio(gameio_), str(str_), padDecide(padDecide_),
      padCancel(padCancel_)
{
	const MenuItemInfo info = BlankInfo();
	auto make = [&](const char* title, bool blink) {
		CMenu* m = new CMenu(draw, gameio, info, title);
		m->SetScreenSpec(screenW / 8, screenH / 8, rotation);
		if (blink) m->SetBlinkAtr(blinkOn, blinkOff, true);
		m->padDecide = padDecide;
		return m;
	};
	pause = make(str.pause, false);
	start = make("", false);
	screenAdjust = make("", false);
	options = make("OPTIONS", true);
	maniac = make("MANIAC OPTIONS", true);
	debug = make("DEBUG MENU", true);
	stringConv = nullptr;   // DefaultMenuStringConvertTable: CMENU::Render passes no table, nothing ported reads it
	quitWait = 0;
	wait = 0;
	depth = 0;
	continueRound = 0;
	StartMenuSet();
	PauseMenuSet();
	mode = 0;
}

// RSO2 0x80d492ac
WiiMenu::~WiiMenu()
{
	for (CMenu* m : {start, pause, screenAdjust, options, maniac, debug}) delete m;
}

// RSO2 0x80d4a4b4 (+0xd0's options block fields: not ported)
void WiiMenu::SetPaletStatus(int offset, int stride)
{
	paletOffset = offset;
	paletStride = stride;
	pause->SetPaletOffset(offset + stride * 0x22);
	start->SetPaletOffset(paletOffset + paletStride * 0x20);
	screenAdjust->SetPaletOffset(paletOffset + paletStride * 0x20);
	maniac->SetPaletOffset(paletOffset + paletStride * 0x23);
	options->SetPaletOffset(paletOffset + paletStride * 0x21);
	debug->SetPaletOffset(paletOffset + paletStride * 0x21);
}

// RSO2 0x80d4a5a0
void WiiMenu::SetMenuYoffset(int start_, int pause_, int screen, int options_, int maniac_, int debug_)
{
	const int v[6] = {start_, pause_, screen, options_, maniac_, debug_};
	for (int i = 0; i < 6; i++)
		if (v[i] != -1) y[i] = v[i];
}

// RSO2 0x80d4a3c4
void WiiMenu::InitMenu()
{
	depth = 0;
	StartMenuSet();
	PauseMenuSet();   // ScreenAdjust(V2)MenuSet / DebugMenuSet: not ported
	OptionMenuTemplateSet();
	mode = 0;
}

// RSO2 0x80d4a42c (ScreenAdjust(V2)MenuSet / DebugMenuSet and vtable +0x1c, PACMAN's ReflectSaveDataToItemCursorExtend 0x80d817a4 = the
// options items' cursors: not ported with those menus)
void WiiMenu::RebuildMenu()
{
	StartMenuSet();
	PauseMenuSet();
}

void WiiMenu::SetStartPauseScreenSpec(int w8, int h8, int rot)
{
	start->SetScreenSpec(w8, h8, rot);
	pause->SetScreenSpec(w8, h8, rot);
}

// RSO2 0x80d4a490
void WiiMenu::EnableMenu()
{
	if (mode != 0) return;
	mode = 1;
	sw.set32(0x10, 10);
}

// RSO2 0x80d49450 (vtable +0x10 = PauseIn: empty in PACMAN's GAMEMENU 0x80d8184c)
void WiiMenu::MenuIn(int menu, bool reset)
{
	depth = 0;
	ChangeMenu(menu, reset);
}

// RSO2 0x80d494fc. Leaving the options menu (mode 4: CMENU +0x3c, the sound test stop) and the screen adjust items' values (SetNumberEx_Num of
// the screen adjust lists from the machine's view) are not ported.
void WiiMenu::ChangeMenu(int menu, bool reset)
{
	if (mode == 0) return;
	if (mode != menu)
	{
		// leaving the options menu: its own decide (PACMAN: OptionsSoundStop) unless the cursor sits on SOUND TEST (the module's first test,
		// mode != 4 inside mode == 4, is dead)
		if (mode == 4)
		{
			MenuItem* cur = options->GetItemByNum(options->cursor);
			if (cur != nullptr && cur->GetText() != str.soundTest && options->info.decide != nullptr) options->info.decide(options);
		}
		resetCursor = reset;
		mode = menu;
		for (CMenu* m : {start, pause, screenAdjust, options, maniac, debug})
		{
			m->enabled = true;
			m->ResetBlinkCount();
			m->SetStyleControl(0, 0xff);
			if (m == start) m->SetMenuTimer(0);
		}
		ClearPads(gameio);
	}
	wait = 3;
}

// RSO2 0x80d494bc
void WiiMenu::PushAndChangeMenu(int menu, bool reset)
{
	if (mode == 0 || mode == menu) return;
	resetCursor = reset;
	if (depth < 0x80) stack[depth++] = mode;
	ChangeMenu(menu, reset);
}

// RSO2 0x80d49a78. The start (2), options (4) and pause (6) menus run; maniac (5), screen adjust (7..9) and debug (10) are not ported.
void WiiMenu::MenuJob()
{
	if (wait != 0)
	{
		wait--;
		return;
	}
	if ((sw.b(0xaf) >> 1) & 1)
	{
		RebuildMenu();
		sw.b(0xaf) &= 0xfd;
	}
	for (CMenu* m : {start, pause, screenAdjust, options, maniac, debug}) m->result = 0;
	switch (mode)
	{
	case 2:
		if (resetCursor)
		{
			start->SetItemCursor(0);
			resetCursor = false;
		}
		start->MenuJob(true);
		StartMenuJob();
		break;
	case 4:
	{
		if (resetCursor)
		{
			options->SetItemCursor(0);
			resetCursor = false;
		}
		options->MenuJob(true);
		bool idle = true;
		if ((padDecide & options->result) != 0)
		{
			idle = false;
			const std::uint32_t cmd = options->GetItemByNum(options->cursor)->info.command;
			if (cmd == 0x39000)
				sw.b(0xd9) = 1;   // DEFAULT (the game's MenuExec resets the export data)
			else if (cmd == 0x3ff00)
				OptionsMenuLeave();
		}
		if (idle && (options->result & 8) != 0) OptionsMenuLeave();
		// every frame: the options' decide (PACMAN: OptionsSoundStop) once the mode left 4, and while the cursor is not on SOUND TEST
		MenuItem* cur = options->GetItemByNum(options->cursor);
		if (mode != 4 && options->info.decide != nullptr) options->info.decide(options);
		if (cur->GetText() != str.soundTest && options->info.decide != nullptr) options->info.decide(options);
		break;
	}
	case 6:
		if (resetCursor)
		{
			pause->SetItemCursor(0);
			resetCursor = false;
		}
		pause->MenuJob(true);
		PauseMenuJob();
		break;
	default:
		break;
	}
}

// RSO2 0x80d4da04 (only the start and pause menus are drawn; see MenuJob)
void WiiMenu::MenuRender()
{
	for (CMenu* m : {start, pause, screenAdjust, options, maniac, debug}) m->SetDrawHandle(drawHandle);
	screenAdjust->crossHatch = false;
	if (mode == 2) start->Render();
	if (mode == 4) options->Render();
	if (mode == 6) pause->Render();
}

// RSO2 0x80d4b304
void WiiMenu::PauseMenuSet()
{
	const int keep = pause->cursor;
	pause->DeleteItemAll();
	MenuItemInfo i = BlankInfo();
	const int titlePal = paletOffset + paletStride * 9;
	i.paletteCursor = paletOffset + paletStride;
	i.palette = paletOffset + paletStride * 0xf;
	i.paletteBlink = paletOffset + paletStride * 3;
	const int cx = x[1];
	i.y = y[1] + 4;
	auto centred = [&](const char* s) { return cx - (int)(std::strlen(s) * 8) / 2; };
	i.command = 0x20000;
	i.x = centred(str.cont);
	itemContinue = new MenuItem(i, str.cont);
	i.command = 0x20300;
	i.y += 0xc;
	i.x = centred(str.screenAdjust);
	itemScreenAdjust = new MenuItem(i, str.screenAdjust);
	i.command = 0x2ff00;
	i.y += 0xc;
	i.x = centred(str.quit);
	itemQuit = new MenuItem(i, str.quit);
	// the menu itself: the title PAUSE above the items
	i.command = 0;
	i.x = centred(str.pause);
	i.y = y[1] - 0xc;
	i.palette = titlePal;
	i.paletteBlink = titlePal;
	pause->SetText(str.pause);
	pause->info = i;
	pause->SetStringConvertTable(stringConv);
	pause->SetBlinkAtr(blinkOn, blinkOff, true);
	pause->AddItem(itemContinue, -1);
	pause->AddItem(itemScreenAdjust, -1);
	pause->AddItem(itemQuit, -1);
	pause->SetBlackBox(8, 0xe, 0x14, 0x17, 0x40, 0, false);
	pause->ApplyMenuTemplate(4);
	// vtable +0x28 AutoCalcBlackBoxSize(6, ...): WII_MENU's 0x80d4b300 is empty and PACMAN's GAMEMENU does not override it
	pause->SetItemCursor(keep);
}

// RSO2 0x80d4e078. PAD_CANCEL / B, MINUS or "1" close the menu (fade out); decide on CONTINUE fades out, SCREEN ADJUST pushes mode 8
// (PushAndChangeMenu: the screen adjust menu is not ported, so it only changes the mode), QUIT GAME resets the game. The faded-out menu
// returns to the game.
void WiiMenu::PauseMenuJob()
{
	const std::uint32_t r = pause->result;
	if ((padCancel | 8) & r)
	{
		pause->SetStyleControl(2, -1);
		return;
	}
	if (r & 0x2001)
	{
		pause->SetStyleControl(2, -1);
		return;
	}
	if ((padDecide | 8) & r)
	{
		const std::uint32_t cmd = pause->GetItemByNum(pause->cursor)->info.command;
		if (cmd == 0x20000)
			pause->SetStyleControl(2, -1);
		else if (cmd == 0x20300)
		{
			// the screen adjust menu (mode 8) is not ported: the item does nothing natively
		}
		else if (cmd == 0x2ff00)
		{
			sw.b(0x2f) = 0;
			sw.b(0x21) = 0;
			sw.b(0x51) = 1;
			gameio.GameReset(sw);
			sw.set32(0x68, 0xe0);
			sw.set32(0x14, quitWait);
			ChangeMenu(1, false);
			if (soundBufferClear != nullptr) soundBufferClear();
			// vtable +0x14 PauseOut: empty in PACMAN's GAMEMENU (0x80d81850)
			return;
		}
	}
	if (!pause->enabled)
	{
		if (sw.b(6) != 0) sw.b(6) = 0;
		if (sw.b(7) != 0) sw.b(7) = 0;
		ChangeMenu(1, false);
	}
}

// RSO2 0x80d4a694. The rows: 1P START, a holder with the 1P ROUND SELECT, 2P START, a holder with the 2P ROUND SELECT (both holders hidden
// while continueRound is 0), OPTIONS, SCREEN ADJUSTMENT (never shown), RETURN TO ARCADE (on the screen adjust row); the title is the game's
// name (SetStartMenuTitle). Template 8 then centres every text and sizes the box.
void WiiMenu::StartMenuSet()
{
	const int stride = paletStride, off = paletOffset;
	const int keep = start->cursor;
	start->DeleteItemAll();
	MenuItemInfo i = BlankInfo();
	i.paletteCursor = off + stride;
	i.palette = off + stride * 0xf;
	i.paletteBlink = off + stride * 3;
	const int right = (rotation == 0x5a || rotation == 0x10e) ? screenH : screenW;
	auto centred = [&](const char* s) { return x[0] - (int)(std::strlen(s) * 8) / 2; };
	auto row = [&](int y_) {
		i.y = y_;
		i.rect[0] = i.x;
		i.rect[1] = y_;
		i.rect[2] = right;
	};
	i.y = y[0];
	i.x = 0;
	i.printFlags = 0x2000;
	i.command = 0x10000;
	row(i.y);
	item1P = new MenuItem(i, str.start1P);
	i.command = 0x18100;
	row(i.y + 8);
	hold1P = new MenuItem(i, "");   // DAT_80dc86f0 = ""
	hold1P->AddItem(new NumItemList(i, str.roundSelect, continueRound + 1, 1, continueRound + 2), -1);
	i.y += continueRound != 0 ? 0x10 : 8;
	i.command = 0x10100;
	row(i.y);
	item2P = new MenuItem(i, str.start2P);
	i.command = 0x18200;
	row(i.y + 8);
	hold2P = new MenuItem(i, "");
	hold2P->AddItem(new NumItemList(i, str.roundSelect, continueRound + 1, 1, continueRound + 2), -1);
	i.y += continueRound != 0 ? 0x10 : 8;
	// from here the rect keeps the 2P holder's words (the module stops updating them; template 8 measures again)
	i.printFlags = 0;
	i.x = centred(str.options);
	i.command = 0x10200;
	itemOptions = new MenuItem(i, str.options);
	i.y += 0x10;
	i.x = centred(str.startScreenAdjust);
	i.command = 0x10700;
	itemStartScreenAdjust = new MenuItem(i, str.startScreenAdjust);
	itemStartScreenAdjust->enabled = false;
	itemStartScreenAdjust->unk75 = false;
	i.x = centred(str.retMainMenu);   // CUTF8::GetLength of RET_MAINMENU, the text is RETURN TO ARCADE (kept)
	i.command = 0x1ff00;
	itemReturn = new MenuItem(i, str.returnToArcade);
	// the menu itself: the title
	i.command = 0;
	i.y = 0x38;
	i.x = screenW / 2 - (int)(std::strlen(str.gameName) * 8) / 2;
	const int titlePal = titlePalette != -1 ? titlePalette : off + stride * 9;
	i.palette = titlePal;
	i.paletteBlink = titlePal;
	i.decide = nullptr;
	start->info = i;
	start->SetStringConvertTable(stringConv);
	start->SetBlinkAtr(startBlinkOn, startBlinkOff, true);
	for (MenuItem* m : {item1P, hold1P, item2P, hold2P, itemOptions, itemStartScreenAdjust, itemReturn}) start->AddItem(m, -1);
	if (continueRound == 0)
	{
		hold1P->enabled = false;
		hold2P->enabled = false;
	}
	start->SetBlackBox(4, 0xe, 0x18, 0x1a, 0x40, 0, false);
	start->ApplyMenuTemplate(8);
	if (startBoxW8 != 0) start->SetBlackBox(0, 0, startBoxW8, startBoxH8, 0x40, 0, false);   // GAMEMENU's AutoCalcBlackBoxSize(2, ...)
	// vtable +0x28 AutoCalcBlackBoxSize(2, ...): empty for PACMAN (see PauseMenuSet)
	start->SetItemCursor(keep);
}

// the back step of MenuJob case 4 (EXIT, or A / PLUS): a save is requested when leaving an options / screen adjust menu, the previous mode
// comes off the stack, the pads are cleared and MenuJob waits 3 frames
void WiiMenu::OptionsMenuLeave()
{
	resetCursor = false;
	if ((unsigned)(mode - 7) < 3 || mode == 4)
	{
		sw.set32(0x58, 1);
		sw.b(0x54) = 1;
	}
	if (depth == 0)
		mode = 1;
	else
		mode = stack[--depth];
	ClearPads(gameio);
	start->enabled = true;
	start->SetMenuTimer(0);
	pause->enabled = true;
	screenAdjust->enabled = true;
	wait = 3;
}

// RSO2 0x80d4c9f0: the options menu's frame (a full-screen box, the title block the game's OptionsMenuBuild copies, auto-repeat on);
// its one EXIT item (text DAT_80dc8718) is replaced by the game's OptionsMenuBuild
void WiiMenu::OptionMenuTemplateSet()
{
	const int off = paletOffset, stride = paletStride;
	options->DeleteItemAll();
	MenuItemInfo i = BlankInfo();
	i.palette = off + stride * 0xf;
	i.paletteCursor = off + stride;
	i.paletteBlink = off + stride * 3;
	i.command = 0x3ff00;
	i.y = 0x100;
	MenuItem* exitItem = new MenuItem(i, "");   // DAT_80dc8718 (the game replaces the item)
	options->useRepeat = true;
	options->repeatTable = 0;
	i.command = 0;
	i.y = 0x28;
	i.x = x[3];   // - strlen("") * 8 / 2
	i.palette = off + stride * 9;
	i.paletteBlink = off + stride * 9;
	options->SetText("");
	options->info = i;
	options->SetStringConvertTable(stringConv);
	options->SetBlinkAtr(blinkOn, blinkOff, true);
	options->AddItem(exitItem, -1);
	// vtable +0x28 AutoCalcBlackBoxSize(4, ...) is empty: the box is the whole (turned) screen in characters
	const bool turned = rotation == 0x5a || rotation == 0x10e;
	options->SetBlackBox(0, 0, (turned ? screenH : screenW) / 8, (turned ? screenW : screenH) / 8, 0x40, 0, false);
	options->ApplyMenuTemplate(2);
}

// RSO2 0x80d4f388 (vtable +0x10 = SetText)
void WiiMenu::SetStartMenuTitle(const char* text, int x_, int y_, int textPalette, int palette)
{
	start->SetText(text);
	if (x_ != -1) start->info.x = x_;
	if (y_ != -1) start->info.y = y_;
	if (textPalette != -1) start->info.textPalette = textPalette;
	if (palette != -1)
	{
		start->info.palette = paletOffset + palette * paletStride;
		titlePalette = paletOffset + palette * paletStride;
	}
}

// RSO2 0x80d4f500 (vtable +0x28 of the NUMITEMLIST = GetNumber)
int WiiMenu::GetContinueRound(int player) const
{
	const MenuItem* h = player == 0 ? hold1P : hold2P;
	return static_cast<const NumItemList*>(h->GetItemByNum(h->cursor))->GetNumber() - 1;
}

// RSO2 0x80d4dc00. PAD_CANCEL, MINUS or "1" back out to the attract (game reset); decide on a START row starts the game (vtable +0xc =
// GameStartInit), OPTIONS pushes mode 4, SCREEN ADJUSTMENT mode 9 (not ported: the row is always disabled), RETURN TO
// ARCADE asks the console to exit (vtable +0x24, MenuExitExtend: empty). 0x708 frames without a move go back to the attract.
void WiiMenu::StartMenuJob()
{
	const std::uint32_t r = start->result;
	bool idle = true;
	if ((padCancel & r) != 0 || (r & 0x2001) != 0)
	{
		idle = false;
		sw.b(0x51) = 1;
		gameio.GameReset(sw);
		sw.set32(0x14, quitWait);
		ChangeMenu(1, false);
	}
	if (idle && ((padDecide | 8) & r) != 0)
	{
		idle = false;
		const std::uint32_t cmd = start->GetItemByNum(start->cursor)->info.command;
		if (cmd == 0x10000 || cmd == 0x10100 || cmd == 0x18100 || cmd == 0x18200)
		{
			const int p = (cmd == 0x10100 || cmd == 0x18200) ? 1 : 0;
			swork_SetWork_GameStart(sw, p, (cmd & 0x8000) != 0 ? 1 : 0);
			sw.b(0xa3) = 0;
			sw.b(0xa4) = 0;
			sw.b(0x54) = 1;
			gameio.SetPadInvalidUntilReleaseEdgeDetect(0, padDecide | 8);
			sw.set32(0x14, startWait);
			ChangeMenu(1, false);
			players = p;
			if (gameStartInit != nullptr) gameStartInit((int)cmd);
		}
		else if (cmd == 0x10200)
		{
			// the options menu (inlined PushAndChangeMenu(4), cursor kept)
			if (mode != 0 && mode != 4)
			{
				resetCursor = false;
				if (depth < 0x80) stack[depth++] = mode;
				ChangeMenu(4, false);
			}
		}
		else if (cmd == 0x10700)
		{
			// PushAndChangeMenu(9): the screen adjust menu is not ported (the row is never enabled anyway)
		}
		else if (cmd == 0x1ff00)
		{
			if (menuExitExtend != nullptr) menuExitExtend();   // UNVERIFIED: the call's place relative to ExitRequest
			gameio.ExitRequest(sw);
			ChangeMenu(1, false);
		}
	}
	if (idle && start->GetMenuTimer() > 0x707)
	{
		gameio.GameReset(sw);
		sw.set32(0x14, quitWait);
		ChangeMenu(1, false);
	}
}
}
