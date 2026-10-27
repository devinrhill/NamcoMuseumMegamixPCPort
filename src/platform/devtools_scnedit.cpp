// The dev window's SCN0 tab (devui, --devui / F9): every SCN0 (AnmScn: lights, ambient lights, light sets, fogs, cameras) the game has loaded
// this run, keyed by entry name + the CRC of its block (wii::ScnAnim::Load registers it). Edits go to a copy that every later sample of that
// resource reads instead (ScnAnim::Sample*, also for objects loaded before the edit); nothing is written anywhere and the edits end with the run.
// Most games sample their scene once when a job starts and copy the result into the models: those see an edit after "Restart the job".
// NOT part of the 1:1 game.
#include "platform/devtools_scn.hpp"
#include "platform/devui.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/KingdomJob.hpp"
#include "wii/Anim.hpp"

namespace devtools
{
// the parsed resource's private parts (wii::ScnAnim befriends this)
struct ScnEditAccess
{
	static auto& Lights(wii::ScnAnim& a) { return a.m_lights; }
	static auto& Ambs(wii::ScnAnim& a) { return a.m_ambs; }
	static auto& Sets(wii::ScnAnim& a) { return a.m_sets; }
	static auto& Fogs(wii::ScnAnim& a) { return a.m_fogs; }
	static auto& Cams(wii::ScnAnim& a) { return a.m_cams; }
	static void ClearKey(wii::ScnAnim& a) { a.m_key.clear(); }   // the copies must not redirect to themselves
};

namespace
{
using A = ScnEditAccess;
using devui::MakeId;
using devui::UI;

struct Entry
{
	std::string key;
	wii::ScnAnim orig, edit;
	bool edited = false;
	int loads = 0;
	long samples = 0;          // samples since the last load (many = the scene is read every frame)
	unsigned job = 0;          // the job that loaded it last
};
std::vector<Entry> g_entries;  // in the order of their first load
int g_sel = -1, g_page = 0, g_item = 0;

unsigned CurrentJob()
{
	cSysCoreStructure* core = KingdomGlobal::SysCore;
	return core != nullptr ? core->GetJobNo() : 0u;
}

Entry* Find(const std::string& key)
{
	for (Entry& e : g_entries)
		if (e.key == key) return &e;
	return nullptr;
}

std::string Hex(unsigned v)
{
	char b[16];
	std::snprintf(b, sizeof b, "0x%08x", v);
	return b;
}

// ---- widgets ----------------------------------------------------------------------------------------------------------------

// an RGBA word (0xRRGGBBAA) as four 0..255 fields
bool ColorField(UI& ui, const char* id, int a, const std::string& label, std::uint32_t& c)
{
	ui.Label(label, &devui::Color(devui::DIM));
	std::vector<devui::Rect> r = UI::Split(ui.Row(), {1.0f, 1.0f, 1.0f, 1.0f});
	static const char* const kCh[4] = {"R", "G", "B", "A"};
	static const double zero = 0.0;
	bool changed = false;
	for (int k = 0; k < 4; k++)
	{
		const int shift = 24 - 8 * k;
		const double v = (double)(c >> shift & 0xff);
		double n = ui.Number(r[k], MakeId(id, a, k), kCh[k], v, 1.0, "%.0f", true, &zero);
		if (n == v) continue;
		n = std::min(255.0, n);
		c = (c & ~(0xffu << shift)) | ((std::uint32_t)n << shift);
		changed = true;
	}
	return changed;
}

// a colour that may be keyed per frame: an edit makes it constant
template <class F>
bool FrameColor(UI& ui, const char* id, int a, const std::string& label, std::uint32_t& c, F& frames)
{
	if (!frames.empty())
	{
		c = frames.front();
		ui.Label("(per frame, " + std::to_string(frames.size()) + " values: an edit makes it constant; frame 0 shown)", &devui::Color(devui::HINT));
	}
	if (!ColorField(ui, id, a, label, c)) return false;
	frames.clear();
	return true;
}

// a slot (constant or keyed): shows its frame 0 value; an edit makes it constant
template <class S>
bool SlotNumber(UI& ui, const devui::Rect& r, const char* id, int a, const std::string& label, S& s, double step)
{
	const double v = s.Eval(0.0f);
	const double n = ui.Number(r, MakeId(id, a), s.constant ? label : label + " (keyed)", v, step, "%.3f");
	if (n == v) return false;
	s.constant = true;
	s.value = (float)n;
	s.keys.clear();
	return true;
}

template <class S>
bool SlotVec3(UI& ui, const char* id, int a, const std::string& label, S* s, double step)
{
	float v[3] = {s[0].Eval(0.0f), s[1].Eval(0.0f), s[2].Eval(0.0f)};
	const float old[3] = {v[0], v[1], v[2]};
	const bool keyed = !s[0].constant || !s[1].constant || !s[2].constant;
	ui.Vec3(ui.Row(), MakeId(id, a), keyed ? label + " (keyed)" : label, v, step);
	if (v[0] == old[0] && v[1] == old[1] && v[2] == old[2]) return false;
	for (int k = 0; k < 3; k++)   // all three axes become constant at the shown values
	{
		s[k].constant = true;
		s[k].value = v[k];
		s[k].keys.clear();
	}
	return true;
}

template <class V, class N>
int Pick(UI& ui, const char* id, const char* label, const V& items, N name)
{
	std::vector<std::string> names;
	for (const auto& it : items) names.push_back(name(it));
	g_item = std::clamp(g_item, 0, (int)names.size() - 1);
	g_item = ui.Choice(ui.Row(), MakeId(id), label, names, g_item);
	return g_item;
}

// ---- pages ------------------------------------------------------------------------------------------------------------------

bool LightsPage(UI& ui, wii::ScnAnim& s)
{
	auto& lights = A::Lights(s);
	if (lights.empty()) { ui.Label("no lights", &devui::Color(devui::DIM)); return false; }
	auto& l = lights[Pick(ui, "scn.light", "light", lights, [](const auto& x) { return x.name; })];
	bool ch = false;
	const int type = (int)(l.flags & 3);
	const int t = ui.Choice(ui.Row(), MakeId("scn.ltype"), "type", {"point", "directional", "spot"}, std::min(type, 2));
	if (t != type) { l.flags = (l.flags & ~3u) | (std::uint32_t)t; ch = true; }
	const bool on = !((l.flags >> 23 & 1) && !(l.flags & 4));
	if (ui.Checkbox(ui.Row(), "enabled", on) != on) { l.flags |= 1u << 23; l.flags = on ? l.flags & ~4u : l.flags | 4u; ch = true; }
	ch |= FrameColor(ui, "scn.lcol", 0, "colour", l.color, l.colorFrames);
	ch |= SlotVec3(ui, "scn.lpos", 0, "position", l.pos, 1.0);
	ch |= SlotVec3(ui, "scn.laim", 0, "aim", l.aim, 1.0);
	const bool spec = (l.flags & 8) != 0;
	if (ui.Checkbox(ui.Row(), "specular", spec) != spec) { l.flags ^= 8u; ch = true; }
	if (l.flags & 8)
	{
		ch |= FrameColor(ui, "scn.lspec", 0, "specular colour", l.specColor, l.specFrames);
		ch |= SlotNumber(ui, ui.Row(), "scn.lshin", 0, "shininess", l.shininess, 1.0);
	}
	if ((l.flags & 3) != 1)
	{
		ui.Heading("Attenuation");
		const int df = ui.Choice(ui.Row(), MakeId("scn.ldist"), "distance", {"off", "gentle", "medium", "steep"}, std::clamp(l.distFunc, 0, 3));
		if (df != l.distFunc) { l.distFunc = df; ch = true; }
		ch |= SlotNumber(ui, ui.Row(), "scn.lrd", 0, "ref distance", l.refDist, 1.0);
		ch |= SlotNumber(ui, ui.Row(), "scn.lrb", 0, "ref brightness", l.refBright, 0.05);
		if ((l.flags & 3) == 2)
		{
			const int sf = ui.Choice(ui.Row(), MakeId("scn.lspot"), "spot", {"off", "flat", "cos", "cos2", "sharp", "ring1", "ring2"}, std::clamp(l.spotFunc, 0, 6));
			if (sf != l.spotFunc) { l.spotFunc = sf; ch = true; }
			ch |= SlotNumber(ui, ui.Row(), "scn.lcut", 0, "cutoff (degrees)", l.cutoff, 1.0);
		}
	}
	return ch;
}

bool AmbPage(UI& ui, wii::ScnAnim& s)
{
	auto& ambs = A::Ambs(s);
	if (ambs.empty()) { ui.Label("no ambient lights", &devui::Color(devui::DIM)); return false; }
	auto& a = ambs[Pick(ui, "scn.amb", "ambient", ambs, [](const auto& x) { return x.name; })];
	ui.Label("multiplies the materials' ambient colours of the models lit by a light set that uses it", &devui::Color(devui::DIM));
	return FrameColor(ui, "scn.acol", 0, "colour", a.color, a.frames);
}

bool SetsPage(UI& ui, wii::ScnAnim& s)
{
	auto& sets = A::Sets(s);
	if (sets.empty()) { ui.Label("no light sets (the port then uses the first light alone)", &devui::Color(devui::DIM)); return false; }
	int i = 0;
	auto& ls = sets[Pick(ui, "scn.set", "light set", sets, [&i](const auto&) { return "set " + std::to_string(i++); })];
	ui.Label("the port lights models with set 0 (SampleLightSet)", &devui::Color(devui::DIM));
	bool ch = false;
	std::vector<std::string> ambNames{"(none)"};
	int cur = 0;
	for (const auto& a : A::Ambs(s))
	{
		if (a.name == ls.ambient) cur = (int)ambNames.size();
		ambNames.push_back(a.name);
	}
	const int n = ui.Choice(ui.Row(), MakeId("scn.setamb"), "ambient", ambNames, cur);
	if (n != cur) { ls.ambient = n == 0 ? std::string() : ambNames[n]; ch = true; }
	ui.Heading("Lights (up to 8; the first 4 enabled ones light the models)");
	for (const auto& l : A::Lights(s))
	{
		auto it = std::find(ls.lights.begin(), ls.lights.end(), l.name);
		const bool in = it != ls.lights.end();
		if (ui.Checkbox(ui.Row(), l.name, in) == in) continue;
		if (in) ls.lights.erase(it);
		else if (ls.lights.size() < 8) ls.lights.push_back(l.name);
		ch = true;
	}
	return ch;
}

bool FogsPage(UI& ui, wii::ScnAnim& s)
{
	auto& fogs = A::Fogs(s);
	if (fogs.empty()) { ui.Label("no fogs", &devui::Color(devui::DIM)); return false; }
	int i = 0;
	auto& f = fogs[Pick(ui, "scn.fog", "fog", fogs, [&i](const auto&) { return "fog " + std::to_string(i++); })];
	ui.Label("the port applies fog 0 (SampleFog)", &devui::Color(devui::DIM));
	static const int kTypes[] = {0, 2, 4, 5, 6, 7};
	int cur = 0;
	for (int k = 0; k < 6; k++) if (kTypes[k] == f.type) cur = k;
	bool ch = false;
	const int t = ui.Choice(ui.Row(), MakeId("scn.ftype"), "type", {"none", "linear", "exp", "exp2", "revexp", "revexp2"}, cur);
	if (t != cur) { f.type = kTypes[t]; ch = true; }
	const double st = ui.Number(ui.Row(), MakeId("scn.fst"), "start", f.start, 10.0, "%.1f");
	const double en = ui.Number(ui.Row(), MakeId("scn.fen"), "end", f.end, 10.0, "%.1f");
	if (st != f.start || en != f.end) { f.start = (float)st; f.end = (float)en; ch = true; }
	ch |= ColorField(ui, "scn.fcol", 0, "colour", f.color);
	return ch;
}

bool CamsPage(UI& ui, wii::ScnAnim& s)
{
	auto& cams = A::Cams(s);
	if (cams.empty()) { ui.Label("no cameras", &devui::Color(devui::DIM)); return false; }
	auto& c = cams[Pick(ui, "scn.cam", "camera", cams, [](const auto& x) { return x.first; })].second;
	const bool aim = c.flags & 1;
	ui.Label(std::string(aim ? "aim" : "rotate") + " camera, " + (c.projType == 0 ? "perspective" : "orthographic"), &devui::Color(devui::DIM));
	bool ch = SlotVec3(ui, "scn.cpos", 0, "position", &c.slot[0], 1.0);
	ch |= aim ? SlotVec3(ui, "scn.caim", 0, "aim", &c.slot[9], 1.0) : SlotVec3(ui, "scn.crot", 0, "rotation (degrees)", &c.slot[6], 1.0);
	if (aim) ch |= SlotNumber(ui, ui.Row(), "scn.ctw", 0, "twist", c.slot[12], 1.0);
	ch |= c.projType == 0 ? SlotNumber(ui, ui.Row(), "scn.cfov", 0, "fovy", c.slot[13], 1.0) : SlotNumber(ui, ui.Row(), "scn.coh", 0, "ortho height", c.slot[14], 1.0);
	ch |= SlotNumber(ui, ui.Row(), "scn.cnear", 0, "near", c.slot[4], 1.0);
	ch |= SlotNumber(ui, ui.Row(), "scn.cfar", 0, "far", c.slot[5], 10.0);
	ch |= SlotNumber(ui, ui.Row(), "scn.casp", 0, "aspect", c.slot[3], 0.01);
	return ch;
}

void ScnPanel(UI& ui)
{
	ui.Label("Every SCN0 loaded this run. Edits are temporary (until you quit) and apply to every sample of that resource.", &devui::Color(devui::DIM));
	if (g_entries.empty()) { ui.Label("nothing loaded yet", &devui::Color(devui::WARN)); return; }
	std::vector<devui::ListRow> rows;
	for (const Entry& e : g_entries)
		rows.push_back(devui::ListRow{(e.edited ? "* " : "") + e.key + "  (job " + Hex(e.job) + ")", devui::Color(e.job == CurrentJob() ? devui::TEXT : devui::DIM)});
	const int sel = ui.ListBox(MakeId("scn.list"), ui.Row(devui::ROW * 8), rows, g_sel, true);
	if (sel != g_sel) { g_sel = sel; g_item = 0; }
	std::vector<devui::Rect> b = UI::Split(ui.Row(), {1.0f, 1.0f, 1.2f});
	const bool pick = g_sel >= 0 && g_sel < (int)g_entries.size();
	if (ui.Button(b[0], "Revert", pick && g_entries[g_sel].edited) && pick) { g_entries[g_sel].edit = g_entries[g_sel].orig; g_entries[g_sel].edited = false; }
	if (ui.Button(b[1], "Revert all"))
		for (Entry& e : g_entries) { e.edit = e.orig; e.edited = false; }
	if (ui.Button(b[2], "Restart the job") && CurrentJob() != 0) kingdom::Kingdom_SetJobNo(CurrentJob(), false, false);
	if (!pick) { ui.Label("pick a scene", &devui::Color(devui::DIM)); return; }
	Entry& e = g_entries[g_sel];
	ui.Label(e.samples > 30 ? "read every frame: edits show at once" : "read when the job started: edits show after Restart the job (or the next load)",
	         &devui::Color(devui::HINT));
	ui.Label("loaded " + std::to_string(e.loads) + "x, " + std::to_string(e.samples) + " samples since the last load", &devui::Color(devui::DIM));
	const std::string pages[] = {"Lights " + std::to_string(A::Lights(e.edit).size()), "Ambient " + std::to_string(A::Ambs(e.edit).size()),
	                             "Sets " + std::to_string(A::Sets(e.edit).size()), "Fogs " + std::to_string(A::Fogs(e.edit).size()),
	                             "Cameras " + std::to_string(A::Cams(e.edit).size())};
	std::vector<devui::Rect> p = UI::Split(ui.Row(), {1.0f, 1.0f, 1.0f});
	std::vector<devui::Rect> q = UI::Split(ui.Row(), {1.0f, 1.0f, 1.0f});
	for (int k = 0; k < 5; k++)
		if (ui.Button(k < 3 ? p[k] : q[k - 3], pages[k], true, g_page == k) && g_page != k) { g_page = k; g_item = 0; }
	ui.Space();
	bool ch = false;
	switch (g_page)
	{
	case 0: ch = LightsPage(ui, e.edit); break;
	case 1: ch = AmbPage(ui, e.edit); break;
	case 2: ch = SetsPage(ui, e.edit); break;
	case 3: ch = FogsPage(ui, e.edit); break;
	default: ch = CamsPage(ui, e.edit); break;
	}
	if (ch) e.edited = true;
}
}

void ScnLoaded(const std::string& key, const wii::ScnAnim& anim)
{
	Entry* e = Find(key);
	if (e == nullptr)
	{
		g_entries.push_back(Entry{key, anim, anim});
		e = &g_entries.back();
		A::ClearKey(e->orig);
		A::ClearKey(e->edit);
	}
	e->loads++;
	e->samples = 0;
	e->job = CurrentJob();
}

const wii::ScnAnim* ScnEdited(const std::string& key)
{
	Entry* e = Find(key);
	if (e == nullptr) return nullptr;
	e->samples++;
	return e->edited ? &e->edit : nullptr;
}

void RegisterScnPanel() { devui::RegisterPanel("SCN0", ScnPanel); }
}
