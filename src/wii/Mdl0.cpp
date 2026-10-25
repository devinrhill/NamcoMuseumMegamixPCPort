#include "wii/Mdl0.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>

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
	// BinString: the offset points at the characters, the u32 length sits 4 bytes before
	std::string str(std::size_t o) const
	{
		if (o < 4 || o > d.size()) return std::string();
		std::uint32_t len = u32(o - 4);
		if (!ok(o, len)) return std::string();
		return std::string(reinterpret_cast<const char*>(&d[o]), len);
	}
};

// ResDic: name -> absolute data offset (node 0 is the trie root and is skipped)
bool ParseDic(const Reader& r, std::size_t off, std::vector<std::pair<std::string, std::size_t>>& out)
{
	out.clear();
	if (off == 0) return true;
	if (!r.ok(off, 8)) return false;
	std::uint32_t num = r.u32(off + 4);
	if (!r.ok(off, 8 + 0x10ull * (num + 1))) return false;
	for (std::uint32_t i = 1; i <= num; i++)
	{
		std::size_t n = off + 8 + 0x10 * i;
		std::int32_t nameOff = r.i32(n + 8), dataOff = r.i32(n + 12);
		out.emplace_back(r.str(off + nameOff), (std::size_t)(off + dataOff));
	}
	return true;
}

// ---- GX display lists ------------------------------------------------------------------------------------------------

enum { ATTR_NONE = 0, ATTR_DIRECT = 1, ATTR_INDEX8 = 2, ATTR_INDEX16 = 3 };

struct VertexDesc
{
	int pnmtxidx{};
	int texmtxidx[8]{};
	int pos{}, nrm{}, clr[2]{}, tex[8]{};

	bool Size(int& size) const
	{
		size = pnmtxidx;
		for (int i = 0; i < 8; i++) size += texmtxidx[i];
		int attrs[12] = {pos, nrm, clr[0], clr[1], tex[0], tex[1], tex[2], tex[3], tex[4], tex[5], tex[6], tex[7]};
		for (int a : attrs)
		{
			if (a == ATTR_DIRECT) return false; // inline vertex data is not used by NW4R models
			size += a == ATTR_INDEX8 ? 1 : (a == ATTR_INDEX16 ? 2 : 0);
		}
		return true;
	}
};

struct DlVertex
{
	int pnmtx{-1};
	int texmtx[8];
	int pos{-1}, nrm{-1}, clr[2]{-1, -1}, tex[8];
	DlVertex()
	{
		for (int i = 0; i < 8; i++) { texmtx[i] = -1; tex[i] = -1; }
	}
};

// CP register writes of the setup display list: {register: value}
bool ParsePrePrim(const std::vector<std::uint8_t>& dl, std::size_t length, bool be, std::map<int, std::uint32_t>& regs)
{
	std::size_t pos = 0, end = std::min(length, dl.size());
	auto u32 = [&](std::size_t o) {
		return be ? ((std::uint32_t)dl[o] << 24) | (dl[o + 1] << 16) | (dl[o + 2] << 8) | dl[o + 3]
		          : ((std::uint32_t)dl[o + 3] << 24) | (dl[o + 2] << 16) | (dl[o + 1] << 8) | dl[o];
	};
	while (pos < end)
	{
		std::uint8_t cmd = dl[pos];
		if (cmd == 0x00) pos += 1;
		else if (cmd == 0x08)
		{
			if (pos + 6 > dl.size()) return false;
			regs[dl[pos + 1]] = u32(pos + 2);
			pos += 6;
		}
		else if (cmd == 0x10)
		{
			if (pos + 5 > dl.size()) return false;
			std::uint32_t hdr = u32(pos + 1);
			pos += 5 + 4 * ((hdr >> 16) + 1);
		}
		else if (cmd == 0x61) pos += 5;
		else return false;
	}
	return true;
}

VertexDesc DescFromRegs(const std::map<int, std::uint32_t>& regs)
{
	auto get = [&](int r) { auto it = regs.find(r); return it == regs.end() ? 0u : it->second; };
	const std::uint32_t lo = get(0x50), hi = get(0x60);
	VertexDesc d;
	d.pnmtxidx = lo & 1;
	for (int i = 0; i < 8; i++) d.texmtxidx[i] = (lo >> (1 + i)) & 1;
	d.pos = (lo >> 9) & 3;
	d.nrm = (lo >> 11) & 3;
	d.clr[0] = (lo >> 13) & 3;
	d.clr[1] = (lo >> 15) & 3;
	for (int i = 0; i < 8; i++) d.tex[i] = (hi >> (2 * i)) & 3;
	return d;
}

struct DrawCmd
{
	bool isXfLoad{};
	// xf load
	char array{};
	int index{}, address{}, count{};
	// primitive
	int prim{};
	std::vector<DlVertex> verts;
};

bool ParsePrim(const std::vector<std::uint8_t>& dl, std::size_t length, const VertexDesc& desc, bool be, std::vector<DrawCmd>& cmds)
{
	int vsize = 0;
	if (!desc.Size(vsize)) return false;
	std::size_t pos = 0, end = std::min(length, dl.size());
	auto u16 = [&](std::size_t o) {
		return be ? (std::uint16_t)((dl[o] << 8) | dl[o + 1]) : (std::uint16_t)((dl[o + 1] << 8) | dl[o]);
	};
	while (pos < end)
	{
		std::uint8_t op = dl[pos];
		if (op == 0x00) { pos++; continue; }
		if (op == 0x20 || op == 0x28 || op == 0x30 || op == 0x38)
		{
			if (pos + 5 > dl.size()) return false;
			DrawCmd c;
			c.isXfLoad = true;
			c.array = "ABCD"[(op - 0x20) >> 3];
			c.index = u16(pos + 1);
			unsigned word = u16(pos + 3);
			c.address = word & 0xfff;
			c.count = (word >> 12) + 1;
			cmds.push_back(std::move(c));
			pos += 5;
			continue;
		}
		const int prim = op & 0xF8;
		if (prim != 0x80 && prim != 0x90 && prim != 0x98 && prim != 0xA0 && prim != 0xA8 && prim != 0xB0 && prim != 0xB8) return false;
		if (pos + 3 > dl.size()) return false;
		const int count = u16(pos + 1);
		pos += 3;
		if (pos + (std::size_t)count * vsize > dl.size()) return false;
		DrawCmd c;
		c.prim = prim;
		c.verts.resize(count);
		for (int n = 0; n < count; n++)
		{
			DlVertex& v = c.verts[n];
			if (desc.pnmtxidx) v.pnmtx = dl[pos++];
			for (int i = 0; i < 8; i++)
				if (desc.texmtxidx[i]) v.texmtx[i] = dl[pos++];
			auto attr = [&](int kind) -> int {
				if (kind == ATTR_INDEX8) return dl[pos++];
				if (kind == ATTR_INDEX16) { int val = u16(pos); pos += 2; return val; }
				return -1;
			};
			v.pos = attr(desc.pos);
			v.nrm = attr(desc.nrm);
			for (int i = 0; i < 2; i++) v.clr[i] = attr(desc.clr[i]);
			for (int i = 0; i < 8; i++) v.tex[i] = attr(desc.tex[i]);
		}
		cmds.push_back(std::move(c));
	}
	return true;
}

// triangles of a primitive command (quads, strips and fans are split; lines and points give none)
void Triangulate(const DrawCmd& c, std::vector<std::array<const DlVertex*, 3>>& out)
{
	const auto& v = c.verts;
	const int n = (int)v.size();
	switch (c.prim)
	{
	case 0x90:
		for (int i = 0; i + 2 < n; i += 3) out.push_back({&v[i], &v[i + 1], &v[i + 2]});
		break;
	case 0x80:
		for (int i = 0; i + 3 < n; i += 4)
		{
			out.push_back({&v[i], &v[i + 1], &v[i + 2]});
			out.push_back({&v[i], &v[i + 2], &v[i + 3]});
		}
		break;
	case 0x98:
		for (int i = 0; i + 2 < n; i++)
		{
			if (i % 2 == 0) out.push_back({&v[i], &v[i + 1], &v[i + 2]});
			else out.push_back({&v[i + 1], &v[i], &v[i + 2]});
		}
		break;
	case 0xA0:
		for (int i = 1; i + 1 < n; i++) out.push_back({&v[0], &v[i], &v[i + 1]});
		break;
	default: break;
	}
}

// ---- vertex arrays ---------------------------------------------------------------------------------------------------

struct VtxArray
{
	int id{};
	std::string name;              // the dictionary name of the array (the DOL's cFaceList classifies shapes by the name of their position array)
	int kind{};                    // 0 pos, 1 nrm, 2 clr, 3 tex
	std::vector<std::array<float, 3>> f; // pos / nrm / tex (z unused)
	std::vector<std::array<std::uint8_t, 4>> c;
};

bool DecodeColor(const Reader& r, std::size_t off, int kind, std::array<std::uint8_t, 4>& out)
{
	auto ex6 = [](unsigned v) { return (std::uint8_t)((v << 2) | (v >> 4)); };
	switch (kind)
	{
	case 0: // RGB565
	{
		unsigned v = r.u16(off), R = (v >> 11) & 31, G = (v >> 5) & 63, B = v & 31;
		out = {(std::uint8_t)((R << 3) | (R >> 2)), (std::uint8_t)((G << 2) | (G >> 4)), (std::uint8_t)((B << 3) | (B >> 2)), 255};
		return true;
	}
	case 1:
	case 2: out = {r.u8(off), r.u8(off + 1), r.u8(off + 2), 255}; return true;
	case 3: // RGBA4
	{
		unsigned v = r.u16(off);
		out = {(std::uint8_t)(((v >> 12) & 15) * 17), (std::uint8_t)(((v >> 8) & 15) * 17), (std::uint8_t)(((v >> 4) & 15) * 17), (std::uint8_t)((v & 15) * 17)};
		return true;
	}
	case 4: // RGBA6
	{
		unsigned v = (r.u8(off) << 16) | (r.u8(off + 1) << 8) | r.u8(off + 2);
		out = {ex6((v >> 18) & 63), ex6((v >> 12) & 63), ex6((v >> 6) & 63), ex6(v & 63)};
		return true;
	}
	case 5: out = {r.u8(off), r.u8(off + 1), r.u8(off + 2), r.u8(off + 3)}; return true;
	default: return false;
	}
}

bool LoadArray(const Reader& r, std::size_t pos, int kind, VtxArray& a)
{
	a.kind = kind;
	a.id = (int)r.u32(pos + 0x10);
	const std::int32_t toArray = r.i32(pos + 8);
	const unsigned compCount = r.u32(pos + 0x14), compType = r.u32(pos + 0x18);
	const std::size_t start = pos + toArray;
	if (kind == 2)
	{
		const unsigned stride = r.u8(pos + 0x1c), count = r.u16(pos + 0x1e);
		if (!r.ok(start, (std::size_t)count * stride)) return false;
		a.c.resize(count);
		for (unsigned i = 0; i < count; i++)
			if (!DecodeColor(r, start + (std::size_t)i * stride, (int)compType, a.c[i])) return false;
		return true;
	}
	const unsigned frac = r.u8(pos + 0x1c), stride = r.u8(pos + 0x1d), count = r.u16(pos + 0x1e);
	if (!r.ok(start, (std::size_t)count * stride)) return false;
	int per = kind == 0 ? (compCount ? 3 : 2) : (kind == 1 ? 3 : (compCount ? 2 : 1));
	const float scale = compType == 4 ? 1.0f : 1.0f / (float)(1u << frac);
	a.f.assign(count, {0, 0, 0});
	for (unsigned i = 0; i < count; i++)
	{
		for (int k = 0; k < per; k++)
		{
			std::size_t o = start + (std::size_t)i * stride;
			float v;
			switch (compType)
			{
			case 0: v = (float)r.u8(o + k) * scale; break;
			case 1: v = (float)(std::int8_t)r.u8(o + k) * scale; break;
			case 2: v = (float)r.u16(o + k * 2) * scale; break;
			case 3: v = (float)r.i16(o + k * 2) * scale; break;
			case 4: v = r.f32(o + k * 4); break;
			default: return false;
			}
			a.f[i][k] = v;
		}
	}
	return true;
}

struct BlendEntry { int node; float weight; };
}

const Mdl0Node* Mdl0::NodeById(int id) const
{
	for (const auto& n : nodes) if (n.id == id) return &n;
	return nullptr;
}

const Mdl0Material* Mdl0::MaterialById(int id) const
{
	for (const auto& m : materials) if (m.id == id) return &m;
	return nullptr;
}

const Mdl0Mesh* Mdl0::MeshByShapeId(int id) const
{
	for (const auto& m : meshes) if (m.shapeId == id) return &m;
	return nullptr;
}

bool Mdl0::Load(const Brres& brres, const std::string& modelName)
{
	const long blockL = brres.ModelOffset(modelName);
	if (blockL < 0) return false;
	const std::size_t block = (std::size_t)blockL;
	Reader r{brres.Data(), brres.BigEndian()};
	if (!r.ok(block, 0x80) || std::memcmp(&brres.Data()[block], "MDL0", 4) != 0) return false;
	if (r.u32(block + 8) != 11) // revision
	{
		std::fprintf(stderr, "mdl0 %s: unsupported revision %u\n", modelName.c_str(), r.u32(block + 8));
		return false;
	}
	name = modelName;

	const std::size_t H = block + 8; // header fields
	auto dicOf = [&](int field) { return (std::size_t)r.i32(H + field); };
	auto off = [&](std::size_t rel) { return rel ? block + rel : 0; };

	const std::size_t INFO = block + 0x4c;
	for (int i = 0; i < 3; i++) volumeMin[i] = r.f32(INFO + 0x28 + 4 * i), volumeMax[i] = r.f32(INFO + 0x34 + 4 * i);

	std::vector<std::pair<std::string, std::size_t>> dic;

	// --- bytecode (node tree, matrix mixes, draws)
	std::map<int, std::vector<BlendEntry>> mixes;
	std::vector<std::pair<int, int>> nodeDesc; // node id, parent id
	if (!ParseDic(r, off(dicOf(8)), dic)) return false;
	for (const auto& e : dic)
	{
		std::size_t pos = e.second;
		for (;;)
		{
			if (!r.ok(pos, 1)) return false;
			const std::uint8_t op = r.u8(pos);
			if (op == 0 || op == 1) { pos++; if (op == 1) break; }
			else if (op == 2) { nodeDesc.emplace_back(r.u16(pos + 1), r.u16(pos + 3)); pos += 5; }
			else if (op == 3)
			{
				int mtx = r.u16(pos + 1), n = r.u8(pos + 3);
				pos += 4;
				std::vector<BlendEntry> b;
				for (int i = 0; i < n; i++) { b.push_back({(int)r.u16(pos), r.f32(pos + 2)}); pos += 6; }
				mixes.emplace(mtx, std::move(b)); // first definition wins (setdefault)
			}
			else if (op == 4)
			{
				Mdl0Draw d;
				d.material = r.u16(pos + 1);
				d.shape = r.u16(pos + 3);
				d.node = r.u16(pos + 5);
				d.priority = r.u8(pos + 7);
				d.stream = e.first;
				draws.push_back(d);
				pos += 8;
			}
			else if (op == 5 || op == 6) pos += 5;
			else return false;
		}
	}

	// --- nodes
	if (!ParseDic(r, off(dicOf(12)), dic)) return false;
	std::map<std::size_t, int> idByRecord;
	std::vector<std::size_t> recOf;
	for (const auto& e : dic)
	{
		const std::size_t p = e.second;
		Mdl0Node n;
		n.name = e.first;
		n.id = (int)r.u32(p + 0xc);
		n.mtxId = (int)r.u32(p + 0x10);
		n.flags = r.u32(p + 0x14);
		n.billboard = (int)r.u32(p + 0x18);
		if (const std::int32_t toUser = r.i32(p + 0x6c))
		{
			// ResUserDataData: u32 size, ResDicData; item: u32 size, s32 toData, u32 count, u32 type, s32 name, u32 id (+ data)
			const std::size_t ud = p + toUser;
			std::vector<std::pair<std::string, std::size_t>> items;
			if (ParseDic(r, ud + 4, items))
				for (const auto& it : items)
				{
					Mdl0UserItem u;
					const std::size_t ip = it.second;
					const std::int32_t toData = r.i32(ip + 4);
					const std::uint32_t count = r.u32(ip + 8);
					u.type = (int)r.u32(ip + 12);
					u.name = r.str(ip + r.i32(ip + 16));
					const std::size_t dp = ip + toData;
					if (u.type == 0) for (std::uint32_t k = 0; k < count && k < 4096; k++) u.ints.push_back(r.i32(dp + 4 * k));
					else if (u.type == 1) for (std::uint32_t k = 0; k < count && k < 4096; k++) u.floats.push_back(r.f32(dp + 4 * k));
					else if (u.type == 2 && r.ok(dp, count)) { u.str.assign(reinterpret_cast<const char*>(&r.d[dp]), count); while (!u.str.empty() && u.str.back() == 0) u.str.pop_back(); }
					n.userData.push_back(std::move(u));
				}
		}
		for (int i = 0; i < 3; i++)
		{
			n.scale[i] = r.f32(p + 0x20 + 4 * i);
			n.rotate[i] = r.f32(p + 0x2c + 4 * i);
			n.translate[i] = r.f32(p + 0x38 + 4 * i);
			n.volMin[i] = r.f32(p + 0x44 + 4 * i);
			n.volMax[i] = r.f32(p + 0x50 + 4 * i);
		}
		for (int i = 0; i < 12; i++)
		{
			n.model[i] = r.f32(p + 0x70 + 4 * i);
			n.inv[i] = r.f32(p + 0xa0 + 4 * i);
		}
		idByRecord[p] = n.id;
		recOf.push_back(p);
		nodes.push_back(std::move(n));
	}
	for (std::size_t i = 0; i < nodes.size(); i++)
	{
		const std::int32_t toParent = r.i32(recOf[i] + 0x5c);
		if (toParent)
		{
			auto it = idByRecord.find(recOf[i] + toParent);
			if (it != idByRecord.end()) nodes[i].parent = it->second;
		}
	}
	std::sort(nodes.begin(), nodes.end(), [](const Mdl0Node& a, const Mdl0Node& b) { return a.id < b.id; });

	// matrix id -> node id
	std::vector<int> mtxToNode;
	if (r.i32(INFO + 0x24))
	{
		std::size_t p = INFO + r.i32(INFO + 0x24);
		std::uint32_t n = r.u32(p);
		for (std::uint32_t i = 0; i < n; i++) mtxToNode.push_back(r.i32(p + 4 + 4 * i));
	}

	// --- vertex arrays (by id)
	std::map<int, VtxArray> pos, nrm, clr, tex;
	struct Src { int field; int kind; std::map<int, VtxArray>* store; };
	for (const Src& s : {Src{16, 0, &pos}, Src{20, 1, &nrm}, Src{24, 2, &clr}, Src{28, 3, &tex}})
	{
		if (!ParseDic(r, off(dicOf(s.field - 8 + 8)), dic)) return false;
		for (const auto& e : dic)
		{
			VtxArray a;
			if (!LoadArray(r, e.second, s.kind, a))
			{
				std::fprintf(stderr, "mdl0 %s: bad vertex array %s\n", modelName.c_str(), e.first.c_str());
				return false;
			}
			a.name = e.first;
			(*s.store)[a.id] = std::move(a);
		}
	}

	// --- materials
	if (!ParseDic(r, off(dicOf(40)), dic)) return false;
	for (const auto& e : dic)
	{
		const std::size_t p = e.second;
		Mdl0Material m;
		m.name = e.first;
		m.id = (int)r.u32(p + 0xc);
		m.flag = r.u32(p + 0x10);
		m.nTexGens = r.u8(p + 0x14);
		m.nChans = r.u8(p + 0x15);
		m.nTevs = r.u8(p + 0x16);
		m.nIndStages = r.u8(p + 0x17);
		m.lightSetIdx = (std::int8_t)r.u8(p + 0x1d);
		m.fogIdx = (std::int8_t)r.u8(p + 0x1e);
		m.cullMode = (int)r.u32(p + 0x18);
		const unsigned nInfo = r.u32(p + 0x2c);
		const std::int32_t toInfo = r.i32(p + 0x30);
		for (unsigned i = 0; i < nInfo; i++)
		{
			const std::size_t ip = p + toInfo + 52ull * i;
			Mdl0TexSlot t;
			if (r.i32(ip)) t.texName = r.str(ip + r.i32(ip));
			if (r.i32(ip + 4)) t.plttName = r.str(ip + r.i32(ip + 4));
			t.mapId = (int)r.u32(ip + 0x10);
			t.wrapS = (int)r.u32(ip + 0x18);
			t.wrapT = (int)r.u32(ip + 0x1c);
			t.minFilter = (int)r.u32(ip + 0x20);
			t.magFilter = (int)r.u32(ip + 0x24);
			t.lodBias = r.f32(ip + 0x28);
			m.tex.push_back(std::move(t));
		}
		std::size_t q = p + 0x1b0;
		for (int i = 0; i < 8; i++, q += 20)
		{
			Mdl0TexSrt s;
			s.su = r.f32(q); s.sv = r.f32(q + 4); s.rot = r.f32(q + 8); s.tu = r.f32(q + 12); s.tv = r.f32(q + 16);
			m.srt.push_back(s);
		}
		q += 8 * 52; // texture matrix effects (not used yet)
		for (int c = 0; c < 2; c++, q += 20)
		{
			m.chanFlag[c] = r.u32(q);
			for (int k = 0; k < 4; k++)
			{
				m.matColor[c][k] = r.u8(q + 4 + k);
				m.ambColor[c][k] = r.u8(q + 8 + k);
			}
			m.chanCtrlColor[c] = r.u32(q + 12);
			m.chanCtrlAlpha[c] = r.u32(q + 16);
		}
		m.texSrtFlag = r.u32(p + 0x1a8);
		m.texMtxMode = (int)r.u32(p + 0x1ac);
		if (const std::int32_t toTev = r.i32(p + 0x28))
		{
			const std::size_t tp = p + toTev;
			const std::uint32_t tsize = r.u32(tp);
			if (tsize > 0x20 && r.ok(tp, tsize))
			{
				m.tevName = ""; // the name lives in the dictionary of TEV objects; the program is identified by the material
				m.tev.assign(brres.Data().begin() + tp + 0x20, brres.Data().begin() + tp + tsize);
			}
		}
		const std::int32_t toDl = r.i32(p + 0x3c);
		if (r.ok(p + toDl, 384)) m.dl.assign(brres.Data().begin() + p + toDl, brres.Data().begin() + p + toDl + 384);
		materials.push_back(std::move(m));
	}

	// --- shapes -> meshes
	std::map<int, int> drawNode; // shape id -> node of its first draw
	for (const auto& d : draws) drawNode.emplace(d.shape, d.node);

	auto nodeModel = [&](int nodeId) -> const float* {
		for (const auto& n : nodes) if (n.id == nodeId) return n.model;
		return nullptr;
	};
	auto mtxForId = [&](int mtxId) -> const float* {
		if (mixes.count(mtxId)) return nullptr; // enveloped: identity
		if (mtxId >= 0 && mtxId < (int)mtxToNode.size()) return nodeModel(mtxToNode[mtxId]);
		return nullptr;
	};
	auto skinForId = [&](int mtxId, std::vector<BlendEntry>& out) {
		out.clear();
		auto it = mixes.find(mtxId);
		if (it != mixes.end())
		{
			for (const auto& b : it->second)
				if (b.node >= 0 && b.node < (int)mtxToNode.size()) out.push_back({mtxToNode[b.node], b.weight});
		}
		else if (mtxId >= 0 && mtxId < (int)mtxToNode.size()) out.push_back({mtxToNode[mtxId], 1.0f});
	};

	if (!ParseDic(r, off(dicOf(48)), dic)) return false;
	for (const auto& e : dic)
	{
		const std::size_t p = e.second;
		Mdl0Mesh mesh;
		mesh.name = e.first;
		mesh.shapeId = (int)r.u32(p + 0x3c);
		const int curMtx = r.i32(p + 8);

		auto readDl = [&](std::size_t tag, std::int32_t rel, std::uint32_t size, std::vector<std::uint8_t>& out) {
			const std::size_t at = tag + rel;
			if (!r.ok(at, size)) return false;
			out.assign(brres.Data().begin() + at, brres.Data().begin() + at + size);
			return true;
		};
		std::vector<std::uint8_t> prePrim, prim;
		if (!readDl(p + 0x18, r.i32(p + 0x20), r.u32(p + 0x18), prePrim) || !readDl(p + 0x24, r.i32(p + 0x2c), r.u32(p + 0x24), prim))
		{
			std::fprintf(stderr, "mdl0 %s: shape %s display list out of bounds\n", modelName.c_str(), e.first.c_str());
			continue;
		}
		std::vector<int> mtxSet;
		if (r.i32(p + 0x64))
		{
			std::size_t mp = p + r.i32(p + 0x64);
			std::uint32_t n = r.u32(mp);
			for (std::uint32_t i = 0; i < n; i++) mtxSet.push_back(r.u16(mp + 4 + 2 * i));
		}
		std::map<int, std::uint32_t> regs;
		std::vector<DrawCmd> cmds;
		if (!ParsePrePrim(prePrim, r.u32(p + 0x1c), r.be, regs) || !ParsePrim(prim, r.u32(p + 0x28), DescFromRegs(regs), r.be, cmds))
		{
			std::fprintf(stderr, "mdl0 %s: shape %s uses an unsupported vertex format\n", modelName.c_str(), e.first.c_str());
			continue;
		}

		const int idPos = r.i16(p + 0x48), idNrm = r.i16(p + 0x4a), idClr = r.i16(p + 0x4c);
		int idTex[8];
		for (int i = 0; i < 8; i++) idTex[i] = r.i16(p + 0x50 + 2 * i);
		auto find = [](std::map<int, VtxArray>& m, int id) -> VtxArray* { auto it = m.find(id); return it == m.end() ? nullptr : &it->second; };
		VtxArray* posArr = find(pos, idPos);
		VtxArray* nrmArr = find(nrm, idNrm);
		VtxArray* clrArr = find(clr, idClr);
		VtxArray* texArr[8];
		for (int i = 0; i < 8; i++) texArr[i] = find(tex, idTex[i]);
		if (posArr == nullptr)
		{
			std::fprintf(stderr, "mdl0 %s: shape %s has no position array\n", modelName.c_str(), e.first.c_str());
			continue;
		}
		mesh.posName = posArr->name;
		const int dn = drawNode.count(mesh.shapeId) ? drawNode[mesh.shapeId] : -1;

		std::map<int, int> slots; // position matrix slot -> model matrix id (indexed XF loads)
		std::map<std::string, std::uint32_t> seen;
		auto emit = [&](const DlVertex& v) -> std::uint32_t {
			char key[160];
			std::snprintf(key, sizeof key, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d|%d|%d", v.pos, v.nrm, v.clr[0], v.tex[0], v.tex[1], v.tex[2], v.tex[3],
			              v.tex[4], v.tex[5], v.tex[6], v.tex[7], v.pnmtx, v.pnmtx >= 0 && slots.count(v.pnmtx) ? slots[v.pnmtx] : -1);
			auto it = seen.find(key);
			if (it != seen.end()) return it->second;

			const float* m = nullptr;
			std::vector<BlendEntry> skin;
			int mtxId = -1;
			if (v.pnmtx >= 0)
			{
				if (slots.count(v.pnmtx)) mtxId = slots[v.pnmtx];
				else if (v.pnmtx / 3 < (int)mtxSet.size()) mtxId = mtxSet[v.pnmtx / 3];
				else mtxId = v.pnmtx / 3;
				m = mtxForId(mtxId);
				skinForId(mtxId, skin);
			}
			else if (curMtx >= 0)
			{
				m = mtxForId(curMtx);
				skinForId(curMtx, skin);
			}
			else if (dn >= 0)
			{
				m = nodeModel(dn);
				skin.push_back({dn, 1.0f});
			}

			Mdl0Vertex out;
			float px = 0, py = 0, pz = 0;
			if (v.pos >= 0 && v.pos < (int)posArr->f.size()) { px = posArr->f[v.pos][0]; py = posArr->f[v.pos][1]; pz = posArr->f[v.pos][2]; }
			float nx = 0, ny = 0, nz = 0;
			bool hasN = nrmArr && v.nrm >= 0 && v.nrm < (int)nrmArr->f.size();
			if (hasN) { nx = nrmArr->f[v.nrm][0]; ny = nrmArr->f[v.nrm][1]; nz = nrmArr->f[v.nrm][2]; }
			if (m)
			{
				const float x = m[0] * px + m[1] * py + m[2] * pz + m[3];
				const float y = m[4] * px + m[5] * py + m[6] * pz + m[7];
				const float z = m[8] * px + m[9] * py + m[10] * pz + m[11];
				px = x; py = y; pz = z;
				if (hasN)
				{
					const float ax = m[0] * nx + m[1] * ny + m[2] * nz, ay = m[4] * nx + m[5] * ny + m[6] * nz, az = m[8] * nx + m[9] * ny + m[10] * nz;
					nx = ax; ny = ay; nz = az;
				}
			}
			out.pos[0] = px; out.pos[1] = py; out.pos[2] = pz;
			out.hasNrm = hasN;
			out.nrm[0] = nx; out.nrm[1] = ny; out.nrm[2] = nz;
			for (int i = 0; i < 8; i++)
			{
				if (texArr[i] && v.tex[i] >= 0 && v.tex[i] < (int)texArr[i]->f.size())
				{
					out.hasUv[i] = true;
					out.uv[i][0] = texArr[i]->f[v.tex[i]][0];
					out.uv[i][1] = texArr[i]->f[v.tex[i]][1];
				}
			}
			if (clrArr && v.clr[0] >= 0 && v.clr[0] < (int)clrArr->c.size())
			{
				out.hasClr = true;
				std::memcpy(out.clr, clrArr->c[v.clr[0]].data(), 4);
			}
			// keep the four strongest weights, renormalised
			std::sort(skin.begin(), skin.end(), [](const BlendEntry& a, const BlendEntry& b) { return a.weight > b.weight; });
			float sum = 0;
			for (int k = 0; k < (int)skin.size() && k < 4; k++) sum += skin[k].weight;
			for (int k = 0; k < (int)skin.size() && k < 4; k++)
			{
				out.skinNode[k] = skin[k].node;
				out.skinWeight[k] = sum > 0 ? skin[k].weight / sum : 0;
				out.skinCount = k + 1;
			}
			mesh.verts.push_back(out);
			return seen[key] = (std::uint32_t)mesh.verts.size() - 1;
		};

		for (const DrawCmd& c : cmds)
		{
			if (c.isXfLoad)
			{
				if (c.array == 'A') slots[c.address / 4] = c.index;
				continue;
			}
			std::vector<std::array<const DlVertex*, 3>> tris;
			Triangulate(c, tris);
			for (const auto& t : tris)
			{
				const std::uint32_t a = emit(*t[0]), b = emit(*t[1]), cc = emit(*t[2]);
				mesh.tris.push_back(a);
				mesh.tris.push_back(b);
				mesh.tris.push_back(cc);
			}
		}
		meshes.push_back(std::move(mesh));
	}
	return true;
}

}
