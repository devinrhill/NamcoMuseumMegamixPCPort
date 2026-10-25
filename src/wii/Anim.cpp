#include <cstdio>
#include "wii/Anim.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "wii/Brres.hpp"

namespace wii
{
namespace
{
struct Reader
{
	const std::vector<std::uint8_t>& d;
	bool be;

	bool ok(std::size_t o, std::size_t n) const { return o + n <= d.size(); }
	std::uint8_t u8(std::size_t o) const { return ok(o, 1) ? d[o] : 0; }
	std::uint16_t u16(std::size_t o) const
	{
		if (!ok(o, 2)) return 0;
		return be ? (std::uint16_t)((d[o] << 8) | d[o + 1]) : (std::uint16_t)((d[o + 1] << 8) | d[o]);
	}
	std::int16_t i16(std::size_t o) const { return (std::int16_t)u16(o); }
	std::uint32_t u32(std::size_t o) const
	{
		if (!ok(o, 4)) return 0;
		return be ? ((std::uint32_t)d[o] << 24) | (d[o + 1] << 16) | (d[o + 2] << 8) | d[o + 3]
		          : ((std::uint32_t)d[o + 3] << 24) | (d[o + 2] << 16) | (d[o + 1] << 8) | d[o];
	}
	std::int32_t i32(std::size_t o) const { return (std::int32_t)u32(o); }
	float f32(std::size_t o) const
	{
		std::uint32_t v = u32(o);
		float f;
		std::memcpy(&f, &v, 4);
		return f;
	}
	std::string str(std::size_t o) const
	{
		if (o < 4 || o > d.size()) return std::string();
		std::uint32_t len = u32(o - 4);
		if (!ok(o, len)) return std::string();
		return std::string(reinterpret_cast<const char*>(&d[o]), len);
	}
};

using Entries = std::vector<std::pair<std::string, std::size_t>>;

bool ParseDic(const Reader& r, std::size_t off, Entries& out)
{
	out.clear();
	if (off == 0) return true;
	if (!r.ok(off, 8)) return false;
	std::uint32_t num = r.u32(off + 4);
	if (!r.ok(off, 8 + 0x10ull * (num + 1))) return false;
	for (std::uint32_t i = 1; i <= num; i++)
	{
		std::size_t n = off + 8 + 0x10 * i;
		out.emplace_back(r.str(off + r.i32(n + 8)), (std::size_t)(off + r.i32(n + 12)));
	}
	return true;
}

// first block of a folder, checked against its magic
bool FirstBlock(const Brres& b, const char* folder, const char* magic, std::size_t& off)
{
	const auto* f = b.Folder(folder);
	if (f == nullptr || f->empty()) return false;
	off = f->front().second;
	return off + 8 <= b.Data().size() && std::memcmp(&b.Data()[off], magic, 4) == 0;
}

// ResKeyFrameAnmData at `pos`: u16 n, pad, f32 invRange, { f32 frame, f32 value, f32 slope }[n]
bool ReadKeys(const Reader& r, std::size_t pos, std::vector<std::array<float, 3>>& keys)
{
	if (!r.ok(pos, 8)) return false;
	const std::uint32_t n = r.u16(pos);
	if (!r.ok(pos + 8, 12ull * n)) return false;
	keys.clear();
	for (std::uint32_t i = 0; i < n; i++) keys.push_back({r.f32(pos + 8 + 12 * i), r.f32(pos + 12 + 12 * i), r.f32(pos + 16 + 12 * i)});
	return true;
}

float Hermite(float v0, float t0, float v1, float t1, float p, float d)
{
	const float s = p / d, s1 = s - 1.0f;
	return v0 + (v0 - v1) * (2.0f * s - 3.0f) * s * s + p * s1 * (s1 * t0 + s * t1);
}

float EvalKeys(const std::vector<std::array<float, 3>>& k, float frame)
{
	if (k.empty()) return 0.0f;
	if (frame <= k.front()[0]) return k.front()[1];
	if (k.back()[0] <= frame) return k.back()[1];
	size_t i = 0;
	while (i + 1 < k.size() && k[i + 1][0] <= frame) i++;
	const auto& a = k[i];
	const auto& b = k[i + 1];
	if (a[0] == frame) return a[1];
	return Hermite(a[1], a[2], b[1], b[2], frame - a[0], b[0] - a[0]);
}
}

float ClipFrame(float frame, int numFrame, bool loops)
{
	if (numFrame <= 0) return 0.0f;
	if (loops)
	{
		frame = std::fmod(frame, (float)numFrame);
		return frame < 0 ? frame + numFrame : frame;
	}
	return frame <= 0.0f ? 0.0f : frame >= (float)numFrame ? (float)numFrame : frame;
}

// ---- VIS0 (revision 4) -----------------------------------------------------------------------------------------------------
// block: +0x10 dictionary, +0x20 u16 frames, +0x22 u16 nodes, +0x24 u32 policy; node: i32 name offset, u32 flags (bit 0 constant value, bit 1 constant),
// then (num_frame + 1 + 31) / 32 big-endian words with frame 0 in the MSB of the first word (unpac revolution/brres/vis0.py)
bool VisAnim::Load(const Brres& brres)
{
	m_nodes.clear();
	std::size_t blk;
	if (!FirstBlock(brres, "AnmVis(NW4R)", "VIS0", blk)) return false;
	const Reader r{brres.Data(), brres.BigEndian()};
	if (r.u32(blk + 8) != 4) return false;
	numFrame = r.u16(blk + 0x20);
	loops = r.u32(blk + 0x24) == 1;
	Entries dic;
	if (!ParseDic(r, blk + r.i32(blk + 0x10), dic)) return false;
	const int words = (numFrame + 1 + 31) / 32;
	for (const auto& e : dic)
	{
		Node n;
		n.flags = r.u32(e.second + 4);
		if (!(n.flags & 2))
			for (int i = 0; i < words; i++) n.words.push_back(r.u32(e.second + 8 + 4 * i));
		m_nodes[e.first] = std::move(n);
	}
	return true;
}

bool VisAnim::Sample(const std::string& bone, float frame, bool& visible) const
{
	auto it = m_nodes.find(bone);
	if (it == m_nodes.end()) return false;
	const Node& n = it->second;
	if (n.flags & 2) { visible = (n.flags & 1) != 0; return true; }
	int f = (int)std::max(0.0f, std::min(frame, (float)numFrame));
	size_t w = (size_t)f / 32;
	if (w >= n.words.size()) { visible = !n.words.empty() && (n.words.back() & 1); return true; }
	visible = (n.words[w] & (0x80000000u >> (f % 32))) != 0;
	return true;
}

// ---- CLR0 (revision 4): ResAnmClrData header like VIS0 (dictionary +0x10, numFrame +0x20, policy +0x24); a material entry is
// {name, flags (2 bits per target: exists, constant), packed {mask, colour or self-relative offset to numFrame + 1 RGBA8 values}}
bool ClrAnim::Load(const Brres& brres)
{
	m_mats.clear();
	std::size_t blk;
	if (!FirstBlock(brres, "AnmClr(NW4R)", "CLR0", blk)) return false;
	const Reader r{brres.Data(), brres.BigEndian()};
	if (r.u32(blk + 8) != 4) return false;
	numFrame = r.u16(blk + 0x20);
	loops = r.u32(blk + 0x24) == 1;
	Entries dic;
	if (!ParseDic(r, blk + r.i32(blk + 0x10), dic)) return false;
	for (const auto& e : dic)
	{
		Mat m;
		const std::uint32_t flags = r.u32(e.second + 4);
		std::size_t p = e.second + 8;
		for (int i = 0; i < 11; i++)
		{
			const std::uint32_t f = flags >> (2 * i);
			if (!(f & 1)) continue;
			Target& t = m.t[i];
			m.exist |= 1u << i;
			t.mask = r.u32(p);
			t.constant = (f & 2) != 0;
			if (t.constant) t.value = r.u32(p + 4);
			else
			{
				const std::size_t frames = p + 4 + (std::size_t)r.i32(p + 4);
				for (int k = 0; k <= numFrame; k++) t.frames.push_back(r.u32(frames + 4 * (std::size_t)k));
			}
			p += 8;
		}
		m_mats[e.first] = std::move(m);
	}
	return true;
}

// nw4r ResAnmClr::GetAnmResult: frame clipped to [0, numFrame], linear interpolation per 8 bit component between whole frames
bool ClrAnim::Sample(const std::string& material, float frame, ClrResult& out) const
{
	auto it = m_mats.find(material);
	if (it == m_mats.end()) return false;
	const Mat& m = it->second;
	out = ClrResult{};
	out.exist = m.exist;
	const float f = std::max(0.0f, std::min(frame, (float)numFrame));
	for (int i = 0; i < 11; i++)
	{
		if (!(m.exist & (1u << i))) continue;
		const Target& t = m.t[i];
		out.mask[i] = t.mask;
		if (t.constant || t.frames.empty()) { out.rgba[i] = t.value; continue; }
		const int i0 = std::min((int)f, (int)t.frames.size() - 1), i1 = std::min(i0 + 1, (int)t.frames.size() - 1);
		const float fr = f - (float)(int)f;
		const std::uint32_t a = t.frames[(size_t)i0], b = t.frames[(size_t)i1];
		std::uint32_t c = 0;
		for (int s = 0; s < 32; s += 8)
		{
			const float ca = (float)((a >> s) & 0xff), cb = (float)((b >> s) & 0xff);
			c |= (std::uint32_t)(ca + (cb - ca) * fr + 0.5f) << s;
		}
		out.rgba[i] = fr == 0.0f ? a : c;
	}
	return true;
}

// ---- CHR0 (revision 5) -----------------------------------------------------------------------------------------------------

float ChrAnim::Track::Eval(float frame) const
{
	if (enc == 0) return constant;
	if (enc >= 4) // one sample per frame
	{
		if (samples.empty()) return 0.0f;
		if (frame <= 0) return samples.front();
		const int i = (int)frame;
		if (i >= (int)samples.size() - 1) return samples.back();
		const float f = frame - i;
		return f == 0 ? samples[i] : samples[i] + (samples[i + 1] - samples[i]) * f;
	}
	if (keys.empty()) return 0.0f;
	if (frame <= keys.front().frame) return keys.front().value;
	if (keys.back().frame <= frame) return keys.back().value;
	size_t i = 0;
	while (i + 1 < keys.size() && keys[i + 1].frame <= frame) i++;
	const Key& a = keys[i];
	const Key& b = keys[i + 1];
	if (a.frame == frame) return a.value;
	return Hermite(a.value, a.slope, b.value, b.slope, frame - a.frame, b.frame - a.frame);
}

bool ChrAnim::Load(const Brres& brres)
{
	m_nodes.clear();
	std::size_t blk;
	if (!FirstBlock(brres, "AnmChr(NW4R)", "CHR0", blk)) return false;
	const Reader r{brres.Data(), brres.BigEndian()};
	if (r.u32(blk + 8) != 5) return false;
	name = r.str(blk + r.i32(blk + 0x18));
	numFrame = r.u16(blk + 0x20);
	loops = r.u32(blk + 0x24) == 1;

	Entries dic;
	if (!ParseDic(r, blk + r.i32(blk + 0x10), dic)) return false;
	enum { SCALE_X_CONST = 13, ROT_X_CONST = 16, TRANS_X_CONST = 19 }; // flag bit of the x channel; y, z follow
	for (const auto& e : dic)
	{
		const std::size_t p = e.second;
		Node n;
		n.flags = r.u32(p + 4);
		const std::uint32_t fl = n.flags;
		struct Slot { int target; int enc; };
		std::vector<Slot> slots;
		if (fl & (1u << 22))
		{
			const int enc = (fl >> 25) & 3;
			const int cnt = (fl & (1u << 4)) ? 1 : 3;
			for (int i = 0; i < cnt; i++) slots.push_back({i, enc});
		}
		if (fl & (1u << 23)) { const int enc = (fl >> 27) & 7; for (int i = 0; i < 3; i++) slots.push_back({3 + i, enc}); }
		if (fl & (1u << 24)) { const int enc = (fl >> 30) & 3; for (int i = 0; i < 3; i++) slots.push_back({6 + i, enc}); }

		for (size_t i = 0; i < slots.size(); i++)
		{
			const int t = slots[i].target;
			const std::size_t slot = p + 8 + 4 * i;
			const int constBit = t < 3 ? SCALE_X_CONST + t : t < 6 ? ROT_X_CONST + (t - 3) : TRANS_X_CONST + (t - 6);
			Track& tr = n.track[t];
			n.has[t] = true;
			if (slots[i].enc == 0 || (fl >> constBit & 1)) { tr.enc = 0; tr.constant = r.f32(slot); continue; }
			tr.enc = slots[i].enc;
			std::size_t pos = p + r.i32(slot);
			if (tr.enc <= 3)
			{
				const int cnt = r.u16(pos);
				pos += 8;
				float scale = 1, bias = 0;
				if (tr.enc != 3) { scale = r.f32(pos); bias = r.f32(pos + 4); pos += 8; }
				for (int k = 0; k < cnt; k++)
				{
					Track::Key key;
					if (tr.enc == 1)
					{
						const std::uint32_t w = r.u32(pos + 4 * k);
						int slope = w & 0xfff;
						if (slope & 0x800) slope -= 0x1000;
						key = {(float)(w >> 24), ((w >> 12) & 0xfff) * scale + bias, slope / 32.0f};
					}
					else if (tr.enc == 2)
						key = {r.i16(pos + 6 * k) / 32.0f, r.u16(pos + 6 * k + 2) * scale + bias, r.i16(pos + 6 * k + 4) / 256.0f};
					else
						key = {r.f32(pos + 12 * k), r.f32(pos + 12 * k + 4), r.f32(pos + 12 * k + 8)};
					tr.keys.push_back(key);
				}
			}
			else
			{
				const int cnt = numFrame + 1;
				float scale = 1, bias = 0;
				if (tr.enc != 6) { scale = r.f32(pos); bias = r.f32(pos + 4); pos += 8; }
				for (int k = 0; k < cnt; k++)
				{
					if (tr.enc == 4) tr.samples.push_back(r.u8(pos + k) * scale + bias);
					else if (tr.enc == 5) tr.samples.push_back(r.u16(pos + 2 * k) * scale + bias);
					else tr.samples.push_back(r.f32(pos + 4 * k));
				}
			}
		}
		m_nodes[e.first] = std::move(n);
	}
	return true;
}

bool ChrAnim::Sample(const std::string& bone, float frame, BoneKey& out) const
{
	auto it = m_nodes.find(bone);
	if (it == m_nodes.end()) return false;
	const Node& n = it->second;
	out = BoneKey{};
	for (int i = 0; i < 3; i++)
	{
		if (n.has[i]) { out.hasScale = true; out.scale[i] = n.track[i].Eval(frame); }
		if (n.has[3 + i]) { out.hasRotate = true; out.rotate[i] = n.track[3 + i].Eval(frame); }
		if (n.has[6 + i]) { out.hasTrans = true; out.trans[i] = n.track[6 + i].Eval(frame); }
	}
	if (out.hasScale && (n.flags & (1u << 4))) out.scale[1] = out.scale[2] = out.scale[0]; // uniform: one slot
	return true;
}

// ---- SRT0 (revision 5) -----------------------------------------------------------------------------------------------------

float SrtAnim::Chan::Eval(float f) const { return constant ? value : EvalKeys(keys, f); }

bool SrtAnim::Load(const Brres& brres)
{
	m_mats.clear();
	std::size_t blk;
	if (!FirstBlock(brres, "AnmTexSrt(NW4R)", "SRT0", blk)) return false;
	const Reader r{brres.Data(), brres.BigEndian()};
	if (r.u32(blk + 8) != 5) return false;
	numFrame = r.u16(blk + 0x20);
	texMtxMode = (int)r.u32(blk + 0x24);
	loops = r.u32(blk + 0x28) == 1;

	Entries dic;
	if (!ParseDic(r, blk + r.i32(blk + 0x10), dic)) return false;
	for (const auto& e : dic)
	{
		const std::size_t m = e.second;
		const std::uint32_t flags = r.u32(m + 4); // regular texture matrices (indirect ones are not used)
		std::size_t slotPos = m + 12;
		const std::uint32_t indFlags = r.u32(m + 8);
		std::vector<int> mtx;
		for (int i = 0; i < 8; i++) if (flags >> i & 1) mtx.push_back(i);
		const std::size_t indCount = (indFlags & 1) + (indFlags >> 1 & 1) + (indFlags >> 2 & 1);
		(void)indCount;
		for (int idx : mtx)
		{
			const std::size_t t = m + r.i32(slotPos);
			slotPos += 4;
			Tex tex;
			tex.flags = r.u32(t);
			const std::uint32_t f = tex.flags;
			std::size_t pos = t + 4;
			auto read = [&](Chan& c, bool constant) {
				c.constant = constant;
				if (constant) c.value = r.f32(pos);
				else ReadKeys(r, pos + r.i32(pos), c.keys);
				pos += 4;
			};
			tex.su.value = tex.sv.value = 1.0f;
			if (!(f & 0x2))
			{
				read(tex.su, f & 0x20);
				if (f & 0x10) tex.sv = tex.su; else read(tex.sv, f & 0x40);
			}
			if (!(f & 0x4)) read(tex.rot, f & 0x80);
			if (!(f & 0x8)) { read(tex.tu, f & 0x100); read(tex.tv, f & 0x200); }
			m_mats[e.first][idx] = std::move(tex);
		}
	}
	return true;
}

bool SrtAnim::Sample(const std::string& material, int mtx, float frame, TexSrt& out) const
{
	auto m = m_mats.find(material);
	if (m == m_mats.end()) return false;
	auto t = m->second.find(mtx);
	if (t == m->second.end()) return false;
	const Tex& x = t->second;
	out.su = x.su.Eval(frame);
	out.sv = x.sv.Eval(frame);
	out.rot = x.rot.Eval(frame);
	out.tu = x.tu.Eval(frame);
	out.tv = x.tv.Eval(frame);
	return true;
}

// texmtx.py
void TexMatrix(int mode, const TexSrt& s, float m[6])
{
	const float r = s.rot * 3.14159265358979f / 180.0f;
	const float sr = std::sin(r), cr = std::cos(r);
	if (mode == 0)
	{
		const float sinPart = 0.5f * sr - 0.5f, cosPart = -0.5f * cr;
		m[0] = s.su * cr; m[1] = s.su * sr; m[2] = s.su * (cosPart - sinPart - s.tu);
		m[3] = -s.sv * sr; m[4] = s.sv * cr; m[5] = s.sv * (cosPart + sinPart + s.tv) + 1.0f;
		return;
	}
	const float sucr = s.su * cr, susr = s.su * sr, svcr = s.sv * cr, svsr = s.sv * sr;
	if (mode == 1)
	{
		m[0] = sucr; m[1] = -susr; m[2] = susr - sucr * s.tu - susr * s.tv;
		m[3] = svsr; m[4] = svcr; m[5] = -svcr - svsr * s.tu + svcr * s.tv + 1.0f;
		return;
	}
	m[0] = sucr; m[1] = susr; m[2] = -sucr * (s.tu + 0.5f) + susr * (s.tv - 0.5f) + 0.5f;
	m[3] = -svsr; m[4] = svcr; m[5] = svsr * (s.tu + 0.5f) + svcr * (s.tv - 0.5f) + 0.5f;
}

// ---- PAT0 (revision 4) -----------------------------------------------------------------------------------------------------

bool PatAnim::Load(const Brres& brres)
{
	m_mats.clear();
	m_texNames.clear();
	std::size_t blk;
	if (!FirstBlock(brres, "AnmTexPat(NW4R)", "PAT0", blk)) return false;
	const Reader r{brres.Data(), brres.BigEndian()};
	if (r.u32(blk + 8) != 4) return false;
	const std::int32_t toDic = r.i32(blk + 0x10), toTexNames = r.i32(blk + 0x14);
	numFrame = r.u16(blk + 0x30);
	const int numTex = r.u16(blk + 0x34);
	loops = r.u32(blk + 0x38) == 1;
	const std::size_t names = blk + toTexNames;
	for (int i = 0; i < numTex; i++) m_texNames.push_back(r.str(names + r.i32(names + 4 * i)));

	Entries dic;
	if (!ParseDic(r, blk + toDic, dic)) return false;
	for (const auto& e : dic)
	{
		const std::size_t m = e.second;
		std::uint32_t flags = r.u32(m + 4);
		std::size_t pos = m + 8;
		for (int i = 0; i < 8; i++, flags >>= 4)
		{
			if (!(flags & 1)) continue;
			Map mp;
			mp.hasTex = flags & 4;
			mp.constant = flags & 2;
			if (mp.constant) mp.constTex = r.u16(pos);
			else
			{
				const std::size_t d = m + r.i32(pos);
				const int n = r.u16(d);
				for (int k = 0; k < n; k++) mp.frames.push_back({r.f32(d + 8 + 8 * k), (int)r.u16(d + 12 + 8 * k)});
			}
			m_mats[e.first][i] = std::move(mp);
			pos += 4;
		}
	}
	return true;
}

std::string PatAnim::Sample(const std::string& material, int map, float frame) const
{
	auto m = m_mats.find(material);
	if (m == m_mats.end()) return std::string();
	auto it = m->second.find(map);
	if (it == m->second.end() || !it->second.hasTex) return std::string();
	const Map& mp = it->second;
	int tex = mp.constTex;
	if (!mp.constant)
	{
		if (mp.frames.empty()) return std::string();
		tex = mp.frames.front().tex;
		for (const Frame& f : mp.frames) if (f.frame <= frame) tex = f.tex;
	}
	return tex >= 0 && tex < (int)m_texNames.size() ? m_texNames[tex] : std::string();
}

// ---- SCN0 cameras (revisions 4 / 5) ----------------------------------------------------------------------------------------

float ScnAnim::Slot::Eval(float f) const { return constant ? value : EvalKeys(keys, f); }

bool ScnAnim::Load(const Brres& brres)
{
	m_cams.clear();
	m_lights.clear();
	m_ambs.clear();
	m_sets.clear();
	m_fogs.clear();
	std::size_t blk;
	if (!FirstBlock(brres, "AnmScn(NW4R)", "SCN0", blk)) return false;
	const Reader r{brres.Data(), brres.BigEndian()};
	const std::uint32_t rev = r.u32(blk + 8);
	if (rev != 4 && rev != 5) return false;
	std::size_t pos = blk + 0x28;
	if (rev >= 5) pos += 4; // user data offset
	pos += 8;               // name, path
	numFrame = r.u16(pos);
	loops = r.u32(pos + 4) == 1;

	Entries folders;
	if (!ParseDic(r, blk + r.i32(blk + 0x10), folders)) return false;
	for (const auto& f : folders)
	{
		if (f.first == "Lights(NW4R)")
		{
			Entries lights;
			if (!ParseDic(r, f.second, lights)) return false;
			for (const auto& l : lights)
			{
				const std::size_t p = l.second;
				Light li;
				li.name = l.first;
				li.flags = r.u32(p + 28);
				auto slotAt = [&](int i, int constBit, Slot& s) {
					const std::size_t at = p + 32 + 4 * i;
					s.constant = li.flags >> constBit & 1;
					if (s.constant) s.value = r.f32(at); else ReadKeys(r, at + r.i32(at), s.keys);
				};
				for (int k = 0; k < 3; k++) { slotAt(1 + k, 19 + k, li.pos[k]); slotAt(5 + k, 24 + k, li.aim[k]); }
				auto colorAt = [&](int i, int constBit) {
					const std::size_t at = p + 32 + 4 * i;
					return (li.flags >> constBit & 1) ? r.u32(at) : r.u32(at + r.i32(at)); // frame 0 of a per-frame colour
				};
				// a per-frame colour slot points at numFrame + 1 RGBA words
				auto framesAt = [&](int i, int constBit, std::vector<std::uint32_t>& out) {
					if (li.flags >> constBit & 1) return;
					const std::size_t at = p + 32 + 4 * i, base = at + r.i32(at);
					for (int k = 0; k <= numFrame; k++) out.push_back(r.u32(base + 4 * k));
				};
				li.color = colorAt(4, 22);
				framesAt(4, 22, li.colorFrames);
				if (li.flags & 8) { li.specColor = colorAt(13, 30); framesAt(13, 30, li.specFrames); slotAt(14, 31, li.shininess); }
				m_lights.push_back(std::move(li));
			}
			continue;
		}
		if (f.first == "AmbLights(NW4R)")
		{
			// ResAmbLightData: 20 byte prefix, flags +20, colour slot +24 (constant when flags bit 31)
			Entries ambs;
			if (!ParseDic(r, f.second, ambs)) return false;
			for (const auto& a : ambs)
			{
				AmbLight al;
				al.name = a.first;
				const std::uint32_t flags = r.u32(a.second + 20);
				const std::size_t at = a.second + 24;
				if (flags >> 31 & 1) al.color = r.u32(at);
				else
				{
					const std::size_t base = at + r.i32(at);
					for (int k = 0; k <= numFrame; k++) al.frames.push_back(r.u32(base + 4 * k));
					al.color = al.frames.empty() ? 0xffffffff : al.frames.front();
				}
				m_ambs.push_back(std::move(al));
			}
			continue;
		}
		if (f.first == "LightSet(NW4R)")
		{
			// ResLightSetData (76 bytes): name offsets relative to the item: ambient +20; lights: 8 offsets at +28 relative to +28,
			// count u8 at +26 (unpac revolution/brres/scn0.py)
			Entries sets;
			if (!ParseDic(r, f.second, sets)) return false;
			for (const auto& e : sets)
			{
				const std::size_t p = e.second;
				LightSet ls;
				const std::int32_t ambOff = r.i32(p + 20);
				if (ambOff != 0) ls.ambient = r.str(p + ambOff);
				const int n = std::min<int>(r.u8(p + 26), 8);
				for (int i = 0; i < n; i++)
				{
					const std::int32_t o = r.i32(p + 28 + 4 * i);
					if (o != 0) ls.lights.push_back(r.str(p + 28 + o));
				}
				m_sets.push_back(std::move(ls));
			}
			continue;
		}
		if (f.first == "Fogs(NW4R)")
		{
			// item: 20 byte prefix, flags +20, type +24, start z +28, end z +32, colour RGBA +36 (the slots are constants in every disc file; a keyed
			// slot would need its frame list: frame 0 is read as the constant word, UNVERIFIED)
			Entries fogs;
			if (!ParseDic(r, f.second, fogs)) return false;
			for (const auto& fg : fogs)
			{
				const std::size_t p = fg.second;
				Fog fo;
				const std::uint32_t flags = r.u32(p + 20);
				fo.type = (int)r.u32(p + 24);
				fo.start = r.f32(p + 28);
				fo.end = r.f32(p + 32);
				fo.color = r.u32(p + 36);
				if (!(flags >> 29 & 1) || !(flags >> 30 & 1) || !(flags >> 31 & 1)) std::fprintf(stderr, "SCN0 fog %s: animated fog is not supported\n", fg.first.c_str());
				m_fogs.push_back(fo);
			}
			continue;
		}
		if (f.first != "Cameras(NW4R)") continue;
		Entries cams;
		if (!ParseDic(r, f.second, cams)) return false;
		for (const auto& c : cams)
		{
			const std::size_t p = c.second;
			Cam cam;
			cam.projType = (int)r.u32(p + 20);
			cam.flags = r.u32(p + 24);
			const bool aim = cam.flags & 1;
			static const int kConstBit[15] = {17, 18, 19, 20, 21, 22, 29, 30, 31, 25, 26, 27, 28, 23, 24};
			for (int i = 0; i < 15; i++)
			{
				const bool used = i < 6 || (i < 9 && !aim) || (i >= 9 && i < 13 && aim) || (i == 13 && cam.projType == 0) || (i == 14 && cam.projType == 1);
				if (!used) continue;
				const std::size_t slot = p + 32 + 4 * i;
				Slot& s = cam.slot[i];
				s.constant = cam.flags >> kConstBit[i] & 1;
				if (s.constant) s.value = r.f32(slot);
				else ReadKeys(r, slot + r.i32(slot), s.keys);
			}
			m_cams.emplace_back(c.first, std::move(cam));
		}
	}
	return !m_cams.empty() || !m_lights.empty();
}

// g3d: a directional light shines along aim - pos (LightObj: position -(aim - pos) * 1e10); the surface -> light vector is pos - aim.
// The enable bit is the constant flag bit 2 (an animated enable is read as on, UNVERIFIED: none seen). Colours are per frame when keyed.
bool ScnAnim::SampleLightSet(float frame, LightResult& out) const
{
	if (m_sets.empty()) return SampleLight(frame, out);
	const LightSet& ls = m_sets.front();
	const int fi = std::max(0, (int)frame);
	auto pick = [fi](std::uint32_t c, const std::vector<std::uint32_t>& frames) { return frames.empty() ? c : frames[std::min<std::size_t>(fi, frames.size() - 1)]; };
	auto unit = [](std::uint32_t c, float o[3]) { for (int k = 0; k < 3; k++) o[k] = (c >> (24 - 8 * k) & 0xff) / 255.0f; };
	out = LightResult{};
	for (const AmbLight& a : m_ambs)
		if (a.name == ls.ambient) unit(pick(a.color, a.frames), out.ambient);
	bool first = true;
	for (const std::string& name : ls.lights)
	{
		const auto it = std::find_if(m_lights.begin(), m_lights.end(), [&](const Light& l) { return l.name == name; });
		if (it == m_lights.end()) continue;
		const Light& l = *it;
		if ((l.flags >> 23 & 1) && !(l.flags & 4)) continue;   // disabled
		float pos[3], aim[3];
		for (int k = 0; k < 3; k++) { pos[k] = l.pos[k].Eval(frame); aim[k] = l.aim[k].Eval(frame); }
		if (first)
		{
			out.directional = (l.flags & 3) == 1;
			for (int k = 0; k < 3; k++) { out.pos[k] = pos[k]; out.aim[k] = aim[k]; }
			unit(pick(l.color, l.colorFrames), out.color);
			if (l.flags & 8) { unit(pick(l.specColor, l.specFrames), out.specColor); out.shininess = l.shininess.Eval(frame); }
			else for (float& c : out.specColor) c = 0.0f;
			first = false;
		}
		if (out.numRig >= 4) continue;
		LightResult::RigLight& g = out.rig[out.numRig++];
		float d[3] = {pos[0] - aim[0], pos[1] - aim[1], pos[2] - aim[2]};
		const float len = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
		for (int k = 0; k < 3; k++) g.dir[k] = len > 0.0f ? d[k] / len : 0.0f;
		unit(pick(l.color, l.colorFrames), g.color);
		g.hasSpec = (l.flags & 8) != 0;
		if (g.hasSpec) { unit(pick(l.specColor, l.specFrames), g.spec); g.shininess = l.shininess.Eval(frame); }
	}
	return !first;
}

bool ScnAnim::SampleFog(LightResult& out) const
{
	if (m_fogs.empty()) return false;
	const Fog& f = m_fogs.front();
	out.fogType = f.type;
	out.fogStart = f.start;
	out.fogEnd = f.end;
	for (int i = 0; i < 3; i++) out.fogColor[i] = (float)(f.color >> (24 - 8 * i) & 0xff) / 255.0f;
	return true;
}

bool ScnAnim::SampleCamera(const std::string& name, float frame, CameraResult& out) const
{
	const Cam* cam = nullptr;
	for (const auto& c : m_cams) if (name.empty() || c.first == name) { cam = &c.second; break; }
	if (cam == nullptr) return false;
	auto v = [&](int i) { return cam->slot[i].Eval(frame); };
	out = CameraResult{};
	out.aim = cam->flags & 1;
	out.projType = cam->projType;
	for (int i = 0; i < 3; i++) out.pos[i] = v(i);
	out.aspect = v(3); out.nearZ = v(4); out.farZ = v(5);
	for (int i = 0; i < 3; i++) out.aimPos[i] = out.aim ? v(9 + i) : v(6 + i);
	if (out.aim) out.twist = v(12);
	out.fovy = cam->projType == 0 ? v(13) : v(14);
	return true;
}
}

namespace wii
{
bool ScnAnim::SampleLight(float frame, LightResult& out) const
{
	if (m_lights.empty()) return false;
	const Light& l = m_lights.front();
	out = LightResult{};
	out.directional = (l.flags & 3) == 1;
	for (int k = 0; k < 3; k++) { out.pos[k] = l.pos[k].Eval(frame); out.aim[k] = l.aim[k].Eval(frame); }
	for (int k = 0; k < 3; k++)
	{
		out.color[k] = (l.color >> (24 - 8 * k) & 0xff) / 255.0f;       // 0xRRGGBBAA
		out.specColor[k] = (l.flags & 8) ? (l.specColor >> (24 - 8 * k) & 0xff) / 255.0f : 0.0f;
	}
	if (l.flags & 8) out.shininess = l.shininess.Eval(frame);
	return true;
}
}
