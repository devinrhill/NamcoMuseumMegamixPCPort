#include "gfx/ModelDraw.hpp"

#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "gfx/TextureCache.hpp"

namespace gfx
{
namespace
{
// 3x4 row major * point
inline void Xform(const float m[12], const float p[3], float out[3])
{
	out[0] = m[0] * p[0] + m[1] * p[1] + m[2] * p[2] + m[3];
	out[1] = m[4] * p[0] + m[5] * p[1] + m[6] * p[2] + m[7];
	out[2] = m[8] * p[0] + m[9] * p[1] + m[10] * p[2] + m[11];
}
inline void XformDir(const float m[12], const float p[3], float out[3])
{
	out[0] = m[0] * p[0] + m[1] * p[1] + m[2] * p[2];
	out[1] = m[4] * p[0] + m[5] * p[1] + m[6] * p[2];
	out[2] = m[8] * p[0] + m[9] * p[1] + m[10] * p[2];
}
// column-major 4x4 * point (w = 1)
inline void XformW(const float m[16], const float p[3], float out[3])
{
	for (int r = 0; r < 3; r++) out[r] = m[r] * p[0] + m[4 + r] * p[1] + m[8 + r] * p[2] + m[12 + r];
}

using Mtx = std::array<float, 12>; // 3x4 row major

Mtx Mul(const float* a, const float* b)
{
	Mtx o;
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++) o[r * 4 + c] = a[r * 4] * b[c] + a[r * 4 + 1] * b[4 + c] + a[r * 4 + 2] * b[8 + c];
		o[r * 4 + 3] = a[r * 4] * b[3] + a[r * 4 + 1] * b[7] + a[r * 4 + 2] * b[11] + a[r * 4 + 3];
	}
	return o;
}

using Quat = std::array<float, 4>; // x y z w

Quat QMul(const Quat& a, const Quat& b)
{
	return {a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1], a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0],
	        a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3], a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2]};
}

// rotation Rz * Ry * Rx (degrees) as a quaternion: the bone matrix convention of unpac pose.py
Quat EulerQuat(const float deg[3])
{
	const float k = 3.14159265358979f / 360.0f; // half angle in radians
	const Quat qx = {std::sin(deg[0] * k), 0, 0, std::cos(deg[0] * k)};
	const Quat qy = {0, std::sin(deg[1] * k), 0, std::cos(deg[1] * k)};
	const Quat qz = {0, 0, std::sin(deg[2] * k), std::cos(deg[2] * k)};
	return QMul(qz, QMul(qy, qx));
}

// T * R(q) * S as a 3x4 row major matrix
Mtx SrtMatrix(const float s[3], Quat q, const float t[3])
{
	const float l = std::sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	if (l > 0) for (float& v : q) v /= l;
	const float x = q[0], y = q[1], z = q[2], w = q[3];
	const float r[3][3] = {{1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)},
	                       {2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)},
	                       {2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)}};
	Mtx o;
	for (int i = 0; i < 3; i++) o[i * 4] = r[i][0] * s[0], o[i * 4 + 1] = r[i][1] * s[1], o[i * 4 + 2] = r[i][2] * s[2], o[i * 4 + 3] = t[i];
	return o;
}
}

// ---- materials -------------------------------------------------------------------------------------------------------------
// GX BP register writes (0x61, reg, val24) of a display list / TEV blob; 0xfe sets the bit mask of the next write. Follows unpac
// revolution/brres/tev.py bp_regs: stops at the first command that is neither a BP write nor a NOP.
using Regs = std::map<int, std::uint32_t>;
using BpLog = std::vector<std::pair<int, std::uint32_t>>;

void ApplyBp(const std::vector<std::uint8_t>& buf, Regs& regs, BpLog* log = nullptr)
{
	std::uint32_t mask = 0xffffff;
	for (size_t i = 0; i + 5 <= buf.size();)
	{
		if (buf[i] == 0) { i++; continue; }
		if (buf[i] != 0x61) break;
		const int reg = buf[i + 1];
		const std::uint32_t val = (buf[i + 2] << 16) | (buf[i + 3] << 8) | buf[i + 4];
		i += 5;
		if (reg == 0xfe) { mask = val; continue; }
		regs[reg] = (regs[reg] & ~mask & 0xffffff) | (val & mask);
		mask = 0xffffff;
		if (log) log->emplace_back(reg, val);
	}
}

// XF texgen setup in the display list: texgen index -> (source row, type); row 5 + k is texture coordinate set k
std::map<int, std::pair<int, int>> TexGenInfo(const std::vector<std::uint8_t>& buf)
{
	std::map<int, std::pair<int, int>> out;
	for (size_t i = 0; i + 5 <= buf.size();)
	{
		if (buf[i] == 0x10)
		{
			const std::uint32_t hdr = (buf[i + 1] << 24) | (buf[i + 2] << 16) | (buf[i + 3] << 8) | buf[i + 4];
			const int n = (hdr >> 16) + 1, addr = hdr & 0xffff;
			for (int k = 0; k < n; k++)
			{
				const size_t at = i + 5 + 4 * k;
				if (addr + k >= 0x1040 && addr + k < 0x1048 && at + 4 <= buf.size())
				{
					const std::uint32_t v = (buf[at] << 24) | (buf[at + 1] << 16) | (buf[at + 2] << 8) | buf[at + 3];
					out[addr + k - 0x1040] = {(int)(v >> 7 & 0x1f), (int)(v >> 4 & 7)};
				}
			}
			i += 5 + 4 * n;
		}
		else if (buf[i] == 0x61) i += 5;
		else if (buf[i] == 0) i++;
		else break;
	}
	return out;
}

int TevColor(std::uint32_t v)
{
	const int x = v & 0x7ff;
	return (v & 0x400) == 0 ? x : x - 0x800;
}

// The static part of a draw's material: TEV stages, constants and pixel state. Textures and texture matrices are filled per frame.
void BuildMaterial(const wii::Mdl0Material& mat, gfx::MaterialDraw& md, int texgenSrc[4], int& numTexGens)
{
	using namespace gfx;
	Regs tevRegs, dl;
	BpLog log;
	ApplyBp(mat.tev, tevRegs);
	ApplyBp(mat.dl, dl, &log);
	const auto tg = TexGenInfo(mat.dl);
	numTexGens = std::min(mat.nTexGens, 4);
	for (int t = 0; t < 4; t++)
	{
		auto it = tg.find(t);
		texgenSrc[t] = (it != tg.end() && it->second.first >= 5) ? std::min(it->second.first - 5, 3) : 0;
		if (t < numTexGens && it != tg.end() && it->second.first == 1) md.envMask |= 1 << t;   // GX_TG_NRM: environment mapping
	}
	if (std::getenv("MEGAMIX_TEXGEN_LOG"))   // debug: materials whose texgens do not read a UV set (row 1 = normal: environment maps)
		for (const auto& g : tg)
			if (g.second.first < 5) std::fprintf(stderr, "texgen %s: gen %d source row %d type %d\n", mat.name.c_str(), g.first, g.second.first, g.second.second);

	// TEV colour registers (PREV, C0..C2) and konstants K0..K3: written as pairs (R,A) / (B,G); the first write of each wins
	std::set<std::pair<int, int>> seen;
	for (const auto& w : log)
	{
		const int reg = w.first;
		const std::uint32_t val = w.second;
		if (reg < 0xe0 || reg > 0xe7 || seen.count({reg, (int)(val >> 23 & 1)})) continue;
		seen.insert({reg, (int)(val >> 23 & 1)});
		const int p = (reg - 0xe0) / 2;
		float (*tgt)[4] = (val & 0x800000) ? md.konst : md.color;
		const int a = TevColor(val), b = TevColor(val >> 12);
		if (reg & 1) { tgt[p][2] = (float)a; tgt[p][1] = (float)b; } // B, G
		else { tgt[p][0] = (float)a; tgt[p][3] = (float)b; }         // R, A
	}

	md.numStages = std::min(mat.nTevs, 16);
	for (int s = 0; s < md.numStages; s++)
	{
		const std::uint32_t c = tevRegs[0xc0 + 2 * s], a = tevRegs[0xc1 + 2 * s];
		const std::uint32_t order = tevRegs[0x28 + (s >> 1)] >> ((s & 1) ? 12 : 0);
		const std::uint32_t ksel = tevRegs[0xf6 + (s >> 1)];
		const int sh = (s & 1) ? 14 : 4;
		TevStage& t = md.stages[s];
		t.cd = c & 15; t.cc = c >> 4 & 15; t.cb = c >> 8 & 15; t.ca = c >> 12 & 15;
		t.cbias = c >> 16 & 3; t.csub = c >> 18 & 1; t.cclamp = c >> 19 & 1; t.cscale = c >> 20 & 3; t.cdest = c >> 22 & 3;
		t.ad = a >> 4 & 7; t.ac = a >> 7 & 7; t.ab = a >> 10 & 7; t.aa = a >> 13 & 7;
		t.abias = a >> 16 & 3; t.asub = a >> 18 & 1; t.aclamp = a >> 19 & 1; t.ascale = a >> 20 & 3; t.adest = a >> 22 & 3;
		t.texmap = order & 7; t.texcoord = std::min<int>(order >> 3 & 7, 3); t.texen = order >> 6 & 1; t.chan = order >> 7 & 7;
		t.kc = ksel >> sh & 31; t.ka = ksel >> (sh + 5) & 31;
	}

	// indirect texturing (GX BP IND_CMD / IND_MTX / RAS1_SS / RAS1_IREF; decoded as Dolphin's BPMemory). The TEV blob and the material
	// display list both write some of these: the TEV blob wins.
	auto bp = [&](int reg, std::uint32_t& v) { if (tevRegs.count(reg)) { v = tevRegs[reg]; return true; } if (dl.count(reg)) { v = dl[reg]; return true; } return false; };
	md.numInd = std::clamp(mat.nIndStages, 0, 4);
	if (md.numInd > 0)
	{
		std::uint32_t iref = 0, ss[2] = {0, 0};
		bp(0x27, iref); bp(0x25, ss[0]); bp(0x26, ss[1]);
		for (int i = 0; i < md.numInd; i++)
		{
			md.indMap[i] = iref >> (6 * i) & 7;
			md.indCoord[i] = std::min<int>(iref >> (6 * i + 3) & 7, 3);
			md.indShift[i][0] = ss[i >> 1] >> (8 * (i & 1)) & 15;
			md.indShift[i][1] = ss[i >> 1] >> (8 * (i & 1) + 4) & 15;
		}
		for (int k = 0; k < 3; k++)
		{
			std::uint32_t a = 0, b = 0, c = 0;
			bp(0x06 + 3 * k, a); bp(0x07 + 3 * k, b); bp(0x08 + 3 * k, c);
			auto s11 = [](std::uint32_t v) { return (float)((int)(v & 0x7ff) - ((v & 0x400) ? 0x800 : 0)) / 1024.0f; };
			const int scale = (int)(a >> 22 & 3) | (int)(b >> 22 & 3) << 2 | (int)(c >> 22 & 3) << 4;
			const float f = std::ldexp(1.0f, scale - 17);
			md.indMtx[k][0][0] = s11(a) * f;       md.indMtx[k][1][0] = s11(a >> 11) * f;   // ma, mb (column 0)
			md.indMtx[k][0][1] = s11(b) * f;       md.indMtx[k][1][1] = s11(b >> 11) * f;   // mc, md
			md.indMtx[k][0][2] = s11(c) * f;       md.indMtx[k][1][2] = s11(c >> 11) * f;   // me, mf
		}
	}
	for (int s = 0; s < md.numStages; s++)
	{
		TevStage& t = md.stages[s];
		std::uint32_t cmd = 0;
		bp(0x10 + s, cmd);
		const int bt = cmd & 3, mtx = cmd >> 9 & 15;
		// a stage uses its indirect stage when it has one and something to do with it (a matrix, a wrap or adding the previous offset)
		const bool used = bt < md.numInd && (mtx != 0 || (cmd >> 13 & 0x3f) != 0 || (cmd >> 20 & 1));
		t.ind = used ? (std::int8_t)(bt + 1) : 0;
		t.ifmt = cmd >> 2 & 3; t.ibias = cmd >> 4 & 7; t.imtx = (std::int8_t)mtx; t.iwrap = cmd >> 13 & 0x3f; t.iaddprev = cmd >> 20 & 1;
		if (!used && (cmd >> 20 & 1)) { t.ind = 0; t.iaddprev = 1; }
	}
	if (md.numInd > 0 && std::getenv("MEGAMIX_IND_LOG"))   // debug: materials with indirect texture stages
	{
		std::fprintf(stderr, "ind %s: %d ind stages (map %d coord %d shift %d,%d), mtx0 [%g %g %g / %g %g %g]:", mat.name.c_str(), md.numInd, md.indMap[0], md.indCoord[0],
		             md.indShift[0][0], md.indShift[0][1], md.indMtx[0][0][0], md.indMtx[0][0][1], md.indMtx[0][0][2], md.indMtx[0][1][0], md.indMtx[0][1][1], md.indMtx[0][1][2]);
		for (int s = 0; s < md.numStages; s++)
			std::fprintf(stderr, " s%d(ind %d fmt %d bias %d mtx %d wrap %#x prev %d map %d coord %d)", s, md.stages[s].ind, md.stages[s].ifmt, md.stages[s].ibias, md.stages[s].imtx,
			             md.stages[s].iwrap, md.stages[s].iaddprev, md.stages[s].texmap, md.stages[s].texcoord);
		std::fprintf(stderr, "\n");
	}

	// pixel engine: alpha compare, blend mode, z mode (defaults of an untouched display list as in tev.py)
	const std::uint32_t ac = dl.count(0xf3) ? dl[0xf3] : 0x3f0000, bm = dl.count(0x41) ? dl[0x41] : 0, zm = dl.count(0x40) ? dl[0x40] : 0x17;
	md.alphaRef[0] = ac & 255; md.alphaRef[1] = ac >> 8 & 255;
	md.alphaComp[0] = ac >> 16 & 7; md.alphaComp[1] = ac >> 19 & 7; md.alphaOp = ac >> 22 & 3;
	md.blend = bm & 1; md.blendDst = bm >> 5 & 7; md.blendSrc = bm >> 8 & 7;
	md.depthTest = zm & 1; md.depthWrite = zm >> 4 & 1;
	if (const char* dump = std::getenv("MEGAMIX_DUMP_TEV"); dump != nullptr && mat.name == dump)   // debug: print the TEV program of this material
	{
		std::fprintf(stderr, "TEV %s: %d stages, texgens %d, alpha comp %d/%d ref %d/%d op %d, blend %d src %d dst %d, z %d/%d, cull %d\n", mat.name.c_str(), md.numStages, numTexGens,
		             md.alphaComp[0], md.alphaComp[1], md.alphaRef[0], md.alphaRef[1], md.alphaOp, (int)md.blend, md.blendSrc, md.blendDst, (int)md.depthTest, (int)md.depthWrite, (int)md.cull);
		for (int s2 = 0; s2 < md.numStages; s2++)
		{
			const TevStage& t = md.stages[s2];
			std::fprintf(stderr, "  s%d color a%d b%d c%d d%d bias%d sub%d clamp%d scale%d dest%d | alpha a%d b%d c%d d%d bias%d sub%d clamp%d scale%d dest%d | map%d coord%d texen%d chan%d kc%d ka%d\n", s2,
			             t.ca, t.cb, t.cc, t.cd, t.cbias, t.csub, t.cclamp, t.cscale, t.cdest, t.aa, t.ab, t.ac, t.ad, t.abias, t.asub, t.aclamp, t.ascale, t.adest, t.texmap, t.texcoord, t.texen, t.chan, t.kc, t.ka);
		}
		for (const auto& g : tg) std::fprintf(stderr, "  texgen %d: source row %d type %d\n", g.first, g.second.first, g.second.second);
		std::fprintf(stderr, "  texSrtFlag %#x texMtxMode %d srt:", mat.texSrtFlag, mat.texMtxMode);
		for (const auto& sr : mat.srt) std::fprintf(stderr, " [s %g %g r %g t %g %g]", sr.su, sr.sv, sr.rot, sr.tu, sr.tv);
		std::fprintf(stderr, "\n");
		for (int c2 = 0; c2 < 2; c2++)
			std::fprintf(stderr, "  chan%d mat %g %g %g %g amb %g %g %g %g ctrlColor %#x ctrlAlpha %#x\n", c2, md.chanMat[c2][0], md.chanMat[c2][1], md.chanMat[c2][2], md.chanMat[c2][3],
			             md.chanAmb[c2][0], md.chanAmb[c2][1], md.chanAmb[c2][2], md.chanAmb[c2][3], md.chanCtrlColor[c2], md.chanCtrlAlpha[c2]);
		for (int c2 = 0; c2 < 4; c2++) std::fprintf(stderr, "  reg%d %g %g %g %g | konst%d %g %g %g %g\n", c2, md.color[c2][0], md.color[c2][1], md.color[c2][2], md.color[c2][3], c2, md.konst[c2][0], md.konst[c2][1], md.konst[c2][2], md.konst[c2][3]);
		// indirect texturing: IND_CMD per stage (0x10..), IND_MTX (0x06..0x0e), RAS1_SS0/1 (0x25 / 0x26), RAS1_IREF (0x27)
		for (int s2 = 0; s2 < md.numStages; s2++) std::fprintf(stderr, "  indcmd s%d %#08x (dl %#08x)\n", s2, tevRegs[0x10 + s2], dl[0x10 + s2]);
		for (int r2 : {0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x25, 0x26, 0x27}) std::fprintf(stderr, "  bp %#04x tev %#08x dl %#08x\n", r2, tevRegs[r2], dl[r2]);
		std::fprintf(stderr, "  genmode(0x00) tev %#08x dl %#08x, mat flag %#x\n", tevRegs[0x00], dl[0x00], mat.flag);
	}

	for (int c = 0; c < 2; c++)
	{
		for (int k = 0; k < 4; k++) md.chanMat[c][k] = mat.matColor[c][k];
		for (int k = 0; k < 4; k++) md.chanAmb[c][k] = mat.ambColor[c][k];
		md.chanCtrlColor[c] = (int)mat.chanCtrlColor[c];
		md.chanCtrlAlpha[c] = (int)mat.chanCtrlAlpha[c];
	}
	if (std::getenv("MEGAMIX_CHAN_LOG"))   // debug: colour channels of every material
		std::fprintf(stderr, "chan %s: c0 %#x mat %g %g %g amb %g %g %g | c1 %#x mat %g %g %g amb %g %g %g | stages %d\n", mat.name.c_str(), md.chanCtrlColor[0], md.chanMat[0][0],
		             md.chanMat[0][1], md.chanMat[0][2], md.chanAmb[0][0], md.chanAmb[0][1], md.chanAmb[0][2], md.chanCtrlColor[1], md.chanMat[1][0], md.chanMat[1][1], md.chanMat[1][2],
		             md.chanAmb[1][0], md.chanAmb[1][1], md.chanAmb[1][2], md.numStages);
	// GX_CULL_FRONT = 1, GX_CULL_BACK = 2. The triangles Mdl0 emits are clockwise for the faces GX treats as front (GX's screen y runs
	// down), so the cull direction is swapped against Gfx3D's counter-clockwise-front convention. Checked on Pac-Man (eyes / mouth).
	md.cull = mat.cullMode == 1 ? CULL_BACK : mat.cullMode == 2 ? CULL_FRONT : CULL_NONE;
}

// texture matrix `index` of a material as (u', v') = (m0 u + m1 v + m2, m3 u + m4 v + m5); false = identity (unused matrix)
bool StaticTexMatrix(const wii::Mdl0Material& mat, int index, float m[6])
{
	const unsigned flag = mat.texSrtFlag >> (4 * index) & 0xf;
	if (!(flag & 1) || ((flag >> 1) & 7) == 7 || index >= (int)mat.srt.size()) return false;
	const wii::Mdl0TexSrt& s = mat.srt[index];
	wii::TexSrt t;
	if (!(flag & 2)) { t.su = s.su; t.sv = s.sv; }
	if (!(flag & 4)) t.rot = s.rot;
	if (!(flag & 8)) { t.tu = s.tu; t.tv = s.tv; }
	wii::TexMatrix(mat.texMtxMode, t, m);
	return true;
}

bool ModelDraw::Load(const wii::Brres& brres, const std::string& modelName)
{
	m_brres = brres;
	m_tex.clear();
	m_skin.clear();
	m_nodeWorld.clear();
	return m_mdl.Load(m_brres, modelName);
}

void ModelDraw::SetPose(const std::vector<ChrLayer>& layers)
{
	m_skin.clear();
	m_nodeWorld.clear();
	if (layers.empty()) return;
	const auto& nodes = m_mdl.nodes;
	std::vector<Mtx> world(nodes.size());
	m_skin.resize(nodes.size() * 12);
	m_nodeWorld.resize(nodes.size() * 12);
	for (size_t i = 0; i < nodes.size(); i++) // parents come before children (ids are assigned in tree order)
	{
		const wii::Mdl0Node& n = nodes[i];
		float scale[3] = {n.scale[0], n.scale[1], n.scale[2]}, trans[3] = {n.translate[0], n.translate[1], n.translate[2]};
		Quat q = EulerQuat(n.rotate);
		float sSum[3] = {0, 0, 0}, tSum[3] = {0, 0, 0}, wS = 0, wR = 0, wT = 0;
		Quat qSum = {0, 0, 0, 0};
		bool anyS = false, anyR = false, anyT = false;
		for (const ChrLayer& l : layers)
		{
			wii::BoneKey k;
			if (l.anim == nullptr || l.weight <= 0 || !l.anim->Sample(n.name, l.frame, k)) continue;
			// channels a layer does not animate contribute the bind value
			const float* sc = k.hasScale ? k.scale : n.scale;
			const float* tr = k.hasTrans ? k.trans : n.translate;
			Quat lq = k.hasRotate ? EulerQuat(k.rotate) : EulerQuat(n.rotate);
			float dot = lq[0] * qSum[0] + lq[1] * qSum[1] + lq[2] * qSum[2] + lq[3] * qSum[3];
			if (dot < 0) for (float& v : lq) v = -v; // shortest way round
			for (int c = 0; c < 3; c++) { sSum[c] += sc[c] * l.weight; tSum[c] += tr[c] * l.weight; }
			for (int c = 0; c < 4; c++) qSum[c] += lq[c] * l.weight;
			wS += l.weight; wT += l.weight; wR += l.weight;
			anyS = anyT = anyR = true;
		}
		if (anyS && wS > 0) for (int c = 0; c < 3; c++) scale[c] = sSum[c] / wS;
		if (anyT && wT > 0) for (int c = 0; c < 3; c++) trans[c] = tSum[c] / wT;
		if (anyR && wR > 0) q = qSum;
		const Mtx local = SrtMatrix(scale, q, trans);
		world[i] = (n.parent >= 0 && (size_t)n.parent < i) ? Mul(world[n.parent].data(), local.data()) : local;
		std::copy(world[i].begin(), world[i].end(), m_nodeWorld.begin() + i * 12);
		const Mtx skin = Mul(world[i].data(), n.inv);
		std::copy(skin.begin(), skin.end(), m_skin.begin() + i * 12);
	}
}

bool ModelDraw::NodeMatrix(const std::string& name, float out[12]) const
{
	for (size_t i = 0; i < m_mdl.nodes.size(); i++)
	{
		if (m_mdl.nodes[i].name != name) continue;
		const float* src = m_nodeWorld.size() == m_mdl.nodes.size() * 12 ? m_nodeWorld.data() + i * 12 : m_mdl.nodes[i].model;
		std::copy(src, src + 12, out);
		return true;
	}
	return false;
}

TexId ModelDraw::TextureFor(const std::string& name)
{
	if (name.empty()) return NO_TEX;
	Tex& t = m_tex[name];
	if (t.loaded) return t.id;
	wii::Image img;
	if (m_brres.HasTexture(name) && m_brres.DecodeTexture(name, img))
		t.id = CreateTexture(img.width, img.height, img.rgba.data());
	else if (Texture* tex = FindTexture(name))
		t.id = tex->gfxId;
	else
	{
		if (!t.warned) { t.warned = true; std::fprintf(stderr, "ModelDraw: texture %s not found\n", name.c_str()); }
		return NO_TEX; // not found yet: an archive attached later may have it, so don't cache the miss
	}
	t.loaded = true;
	return t.id;
}

namespace
{
// column-major 4x4 helpers for the billboard math
void Mul4(const float* a, const float* b, float* out)
{
	float r[16];
	for (int c = 0; c < 4; c++)
		for (int row = 0; row < 4; row++)
		{
			float s = 0;
			for (int k = 0; k < 4; k++) s += a[k * 4 + row] * b[c * 4 + k];
			r[c * 4 + row] = s;
		}
	std::copy(r, r + 16, out);
}

// inverse of an affine matrix (last row 0 0 0 1)
bool InvAffine(const float* m, float* o)
{
	const float a = m[0], b = m[4], c = m[8], d = m[1], e = m[5], f = m[9], g = m[2], h = m[6], i = m[10];
	const float det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
	if (std::fabs(det) < 1e-12f) return false;
	const float id = 1.0f / det;
	const float r[9] = {(e * i - f * h) * id, (c * h - b * i) * id, (b * f - c * e) * id, (f * g - d * i) * id, (a * i - c * g) * id, (c * d - a * f) * id, (d * h - e * g) * id, (b * g - a * h) * id, (a * e - b * d) * id};
	// r is row-major inverse of the 3x3 (row r, col c = r[r*3+c])
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++) o[col * 4 + row] = r[row * 3 + col];
	for (int row = 0; row < 3; row++) o[12 + row] = -(o[0 + row] * m[12] + o[4 + row] * m[13] + o[8 + row] * m[14]);
	o[3] = o[7] = o[11] = 0; o[15] = 1;
	return true;
}
}

ModelDraw::GpuCache::~GpuCache()
{
	for (const GpuDraw& g : draws) if (g.mesh != 0) DestroyMesh(g.mesh);
}

// object-space vertex for the TEV path: texture coordinate set t = the material's source for t (0 when the vertex has none)
static void RawVertex(const wii::Mdl0Vertex& v, const int texgenSrc[4], Vertex3D& o)
{
	for (int c = 0; c < 3; c++) { o.pos[c] = v.pos[c]; o.nrm[c] = v.nrm[c]; }
	for (int t = 0; t < 4; t++)
	{
		const int src = texgenSrc[t];
		o.uv[t][0] = v.hasUv[src] ? v.uv[src][0] : 0.0f;
		o.uv[t][1] = v.hasUv[src] ? v.uv[src][1] : 0.0f;
	}
	for (int c = 0; c < 4; c++) o.rgba[c] = v.hasClr ? v.clr[c] : 255;
}

bool ModelDraw::HasXlu() const
{
	for (const auto& d : m_mdl.draws)
		if (d.stream == "DrawXlu") return true;
	return false;
}

void ModelDraw::Draw(const float world[16])
{
	// Billboard nodes (ResNodeData::bbmode, e.g. the cookie quads) turn to face the camera: g3d's Calc_BILLBOARD_STD keeps the node's y axis as it is in view
	// space (projected on the screen plane) and makes z the line of sight. Modes 2..6 (perspective / rotate / y) use the same rule here (UNVERIFIED).
	std::vector<float> savedSkin;
	bool billboardSkin = false;
	for (const auto& n : m_mdl.nodes) if (n.billboard != 0) { billboardSkin = true; break; }
	if (billboardSkin)
	{
		const float* V = CurrentView();
		float invV[16], invW[16];
		if (V != nullptr && InvAffine(V, invV) && InvAffine(world, invW))
		{
			savedSkin = m_skin;
			const size_t nn = m_mdl.nodes.size();
			if (m_skin.size() != nn * 12)
			{
				m_skin.assign(nn * 12, 0.0f);
				for (size_t i = 0; i < nn; i++) { m_skin[i * 12] = 1; m_skin[i * 12 + 5] = 1; m_skin[i * 12 + 10] = 1; }
			}
			for (size_t i = 0; i < nn; i++)
			{
				const wii::Mdl0Node& n = m_mdl.nodes[i];
				if (n.billboard == 0) continue;
				const float* nm = m_nodeWorld.size() == nn * 12 ? &m_nodeWorld[i * 12] : n.model;
				const float N4[16] = {nm[0], nm[4], nm[8], 0, nm[1], nm[5], nm[9], 0, nm[2], nm[6], nm[10], 0, nm[3], nm[7], nm[11], 1};
				float NW[16], MV[16];
				Mul4(world, N4, NW);
				Mul4(V, NW, MV);
				float vy[2] = {MV[4], MV[5]};
				const float len = std::sqrt(vy[0] * vy[0] + vy[1] * vy[1]);
				if (len < 1e-5f) continue;
				vy[0] /= len; vy[1] /= len;
				const float sx = std::sqrt(NW[0] * NW[0] + NW[1] * NW[1] + NW[2] * NW[2]), sy = std::sqrt(NW[4] * NW[4] + NW[5] * NW[5] + NW[6] * NW[6]), sz = std::sqrt(NW[8] * NW[8] + NW[9] * NW[9] + NW[10] * NW[10]);
				float MVn[16] = {vy[1] * sx, -vy[0] * sx, 0, 0, vy[0] * sy, vy[1] * sy, 0, 0, 0, 0, sz, 0, MV[12], MV[13], MV[14], 1};
				float NWn[16], Nn[16];
				Mul4(invV, MVn, NWn);
				Mul4(invW, NWn, Nn);
				// model-space 3x4 row major, then skin = node * inverse bind
				const float node34[12] = {Nn[0], Nn[4], Nn[8], Nn[12], Nn[1], Nn[5], Nn[9], Nn[13], Nn[2], Nn[6], Nn[10], Nn[14]};
				const Mtx sk = Mul(node34, n.inv);
				std::copy(sk.begin(), sk.end(), m_skin.begin() + i * 12);
			}
		}
		else billboardSkin = false;
	}
	struct Restore { std::vector<float>& skin; std::vector<float>& saved; bool on; ~Restore() { if (on) skin = saved; } } restore{m_skin, savedSkin, billboardSkin};

	// opaque stream first, then translucent; each by priority (stable, so the bytecode order breaks ties)
	std::vector<const wii::Mdl0Draw*> order;
	for (const auto& d : m_mdl.draws) order.push_back(&d);
	auto rank = [](const wii::Mdl0Draw* d) { return d->stream == "DrawXlu" ? 1 : 0; };
	std::stable_sort(order.begin(), order.end(), [&](const wii::Mdl0Draw* a, const wii::Mdl0Draw* b) {
		if (rank(a) != rank(b)) return rank(a) < rank(b);
		return a->priority < b->priority;
	});

	if (m_matCache.size() != m_mdl.materials.size()) m_matCache.assign(m_mdl.materials.size(), MatCache{});
	static const char* skip = std::getenv("MEGAMIX_SKIP_SHAPE"); // debug: hide one shape by name
	static const bool noCull = std::getenv("MEGAMIX_NOCULL") != nullptr;

	for (const wii::Mdl0Draw* d : order)
	{
		if ((s_pass == 1 && rank(d) == 1) || (s_pass == 2 && rank(d) == 0)) continue;
		const wii::Mdl0Mesh* mesh = m_mdl.MeshByShapeId(d->shape);
		const wii::Mdl0Material* mat = m_mdl.MaterialById(d->material);
		if (mesh == nullptr || mat == nullptr || mesh->tris.empty()) continue;
		if (skip && mesh->name == skip) continue;
		static const char* onlyMat = std::getenv("MEGAMIX_ONLY_MAT"); // debug: draw only the material of this name
		if (onlyMat && mat->name != onlyMat) continue;
		if (mat->cullMode == 3) continue;   // GX_CULL_ALL: the shape draws nothing (Galaga enemies' hit box "Material" on node hit_model)
		if (m_vis != nullptr && d->node >= 0 && d->node < (int)m_mdl.nodes.size())   // VIS0: the node the draw is bound to may be hidden
		{
			bool visible = true;
			if (m_vis->Sample(m_mdl.nodes[d->node].name, m_visFrame, visible) && !visible) continue;
		}

		MatCache& mc = m_matCache[mat - m_mdl.materials.data()];
		if (!mc.built)
		{
			BuildMaterial(*mat, mc.md, mc.texgenSrc, mc.numTexGens);
			mc.built = true;
		}
		MaterialDraw md = mc.md;
		if (noCull) md.cull = CULL_NONE;
		static const bool noBlend = std::getenv("MEGAMIX_NOBLEND") != nullptr; // debug: opaque
		if (noBlend) md.blend = false;
		static const char* maxStages = std::getenv("MEGAMIX_TEV_STAGES"); // debug: run only the first N TEV stages
		if (maxStages) md.numStages = std::min(md.numStages, std::atoi(maxStages));
		md.fogType = m_fogType; md.fogStart = m_fogStart; md.fogEnd = m_fogEnd;
		for (int i = 0; i < 3; i++) md.fogColor[i] = m_fogColor[i];
		if (m_hasLight)
		{
			md.lit = true;
			for (int i = 0; i < 3; i++) { md.lightPos[i] = m_lightPos[i]; md.lightAim[i] = m_lightAim[i]; md.lightColor[i] = m_lightColor[i]; md.specColor[i] = m_specColor[i]; }
			md.shininess = m_shininess;
			md.numRig = m_rig.numRig;
			for (int i = 0; i < m_rig.numRig; i++)
				for (int k = 0; k < 3; k++)
				{
					md.rigDir[i][k] = m_rig.rig[i].dir[k];
					md.rigCol[i][k] = m_rig.rig[i].color[k];
					md.rigSpec[i][k] = m_rig.rig[i].hasSpec ? m_rig.rig[i].spec[k] : 0.0f;
					md.rigSpec[i][3] = m_rig.rig[i].hasSpec ? m_rig.rig[i].shininess : 0.0f;
				}
			for (int k = 0; k < 3; k++) md.sceneAmb[k] = m_rig.numRig > 0 ? m_rig.ambient[k] : 1.0f;
		}
		// the model's light set / fog index only reach materials whose own resource index is >= 0 (kingdom SetLightSetIdx 0x8001e1f8 /
		// SetFogIdx 0x8001e400 keep -1): no light set = no lights and a white ambient light (g3d LoadLightSet), no fog. Gator Panic's sky
		// was fogged and lit (dull purple instead of Dolphin's blue).
		if (mat->fogIdx < 0) md.fogType = 0;
		if (mat->lightSetIdx < 0)
		{
			md.lit = false;
			md.numRig = 0;
			for (int k = 0; k < 3; k++) md.sceneAmb[k] = 1.0f;
		}
		// CLR0: (material colour & mask) | animated colour, per 8 bit component (nw4r ClrAnmResult; targets: mat0 mat1 amb0 amb1 C0 C1 C2 K0..K3)
		if (m_clr != nullptr)
		{
			wii::ClrResult cr;
			if (m_clr->Sample(mat->name, m_clrFrame, cr))
				for (int t = 0; t < 11; t++)
				{
					if (!(cr.exist & (1u << t))) continue;
					float* dst = t < 2 ? md.chanMat[t] : t < 4 ? md.chanAmb[t - 2] : t < 7 ? md.color[t - 3] : md.konst[t - 7];
					for (int k = 0; k < 4; k++)
					{
						const int sh = 24 - 8 * k;   // r g b a
						const std::uint32_t m = (cr.mask[t] >> sh) & 0xff, a = (cr.rgba[t] >> sh) & 0xff;
						const std::uint32_t orig = (std::uint32_t)std::max(0.0f, std::min(255.0f, dst[k]));
						dst[k] = (float)((orig & m) | a);
					}
				}
		}
		static const bool noVcol = std::getenv("MEGAMIX_NOVCOL") != nullptr; // debug: ignore vertex colours
		if (noVcol) for (int c = 0; c < 2; c++) md.chanCtrlColor[c] &= ~1, md.chanCtrlAlpha[c] &= ~1;

		// textures by map; a PAT0 clip may swap what map `m` shows
		for (const wii::Mdl0TexSlot& slot : mat->tex)
		{
			if (slot.mapId < 0 || slot.mapId > 3) continue;
			std::string texName = slot.texName;
			if (m_pat)
			{
				const std::string p = m_pat->Sample(mat->name, slot.mapId, m_patFrame);
				if (!p.empty()) texName = p;
			}
			md.tex[slot.mapId] = TextureFor(texName);
			md.wrapS[slot.mapId] = (Wrap)slot.wrapS;
			md.wrapT[slot.mapId] = (Wrap)slot.wrapT;
		}

		// texture matrices (SRT0 animation wins over the material's own)
		float tm[4][6];
		bool hasTm[4] = {false, false, false, false};
		for (int t = 0; t < mc.numTexGens; t++)
		{
			wii::TexSrt anm;
			if (m_srt && m_srt->Sample(mat->name, t, wii::ClipFrame(m_srtFrame, m_srt->numFrame, m_srt->loops), anm))
			{
				wii::TexMatrix(m_srt->texMtxMode, anm, tm[t]);
				hasTm[t] = true;
			}
			else hasTm[t] = StaticTexMatrix(*mat, t, tm[t]);
		}

		const bool skinned = !m_skin.empty();
		for (int t = 0; t < 4; t++)
			if (t < mc.numTexGens && hasTm[t]) std::copy(tm[t], tm[t] + 6, md.texMtx[t]);

		// Rigid geometry (no vertex has its own weights, or all follow one node with weight 1) is uploaded once and placed by
		// world * node matrix in the vertex shader. It was transformed on the CPU and re-uploaded every frame (perf: ~85000
		// vertices a frame in the hub, the main cost of the frame). Weighted skins still go through the CPU loop below.
		if (!m_gpu) m_gpu = std::make_shared<GpuCache>();
		if (m_gpu->draws.size() != m_mdl.draws.size()) m_gpu->draws.assign(m_mdl.draws.size(), GpuDraw{});
		GpuDraw& gd = m_gpu->draws[d - m_mdl.draws.data()];
		if (!gd.tried)
		{
			gd.tried = true;
			int mode = 1, node = -2;   // 1: every vertex without weights, 2: every vertex on one node with weight 1
			for (const wii::Mdl0Vertex& v : mesh->verts)
			{
				const int m = v.skinCount == 0 ? 1 : (v.skinCount == 1 && v.skinWeight[0] == 1.0f) ? 2 : 0;
				if (m == 0 || (node != -2 && (m == 1 ? -1 : v.skinNode[0]) != node)) { mode = 0; break; }
				node = m == 1 ? -1 : v.skinNode[0];
			}
			if (mode != 0 && mesh->verts.size() <= 65535)
			{
				std::vector<Vertex3D> gv(mesh->verts.size());
				for (size_t i = 0; i < gv.size(); i++) RawVertex(mesh->verts[i], mc.texgenSrc, gv[i]);
				gd.mesh = CreateMesh(gv.data(), gv.size(), mesh->tris.data(), mesh->tris.size());
				gd.rigidNode = node < 0 ? -1 : node;
			}
		}
		if (gd.mesh != 0)
		{
			// as the CPU path: the vertices' node, else (no weights / unknown node) the draw node, when skin matrices are set
			const float* r = nullptr;
			auto valid = [&](int n) { return n >= 0 && (size_t)(n + 1) * 12 <= m_skin.size(); };
			if (skinned) r = valid(gd.rigidNode) ? &m_skin[gd.rigidNode * 12] : valid(d->node) ? &m_skin[d->node * 12] : nullptr;
			if (r != nullptr)
			{
				const float r4[16] = {r[0], r[4], r[8], 0, r[1], r[5], r[9], 0, r[2], r[6], r[10], 0, r[3], r[7], r[11], 1};
				Mul4(world, r4, md.world);
			}
			else std::copy(world, world + 16, md.world);
			DrawMaterialMesh(gd.mesh, md);
			continue;
		}

		std::copy(world, world + 16, md.world);   // the shader places the vertices
		m_buf.resize(mesh->verts.size());
		for (size_t i = 0; i < mesh->verts.size(); i++)
		{
			const wii::Mdl0Vertex& v = mesh->verts[i];
			Vertex3D& o = m_buf[i];
			float p[3] = {v.pos[0], v.pos[1], v.pos[2]};
			float n[3] = {v.nrm[0], v.nrm[1], v.nrm[2]};
			if (skinned)
			{
				float sp[3] = {0, 0, 0}, sn[3] = {0, 0, 0};
				int used = 0;
				for (int k = 0; k < v.skinCount; k++)
				{
					const int node = v.skinNode[k];
					if (node < 0 || (size_t)(node + 1) * 12 > m_skin.size()) continue;
					float tp[3], tn[3];
					Xform(&m_skin[node * 12], v.pos, tp);
					XformDir(&m_skin[node * 12], v.nrm, tn);
					for (int c = 0; c < 3; c++) { sp[c] += v.skinWeight[k] * tp[c]; sn[c] += v.skinWeight[k] * tn[c]; }
					used++;
				}
				if (used == 0 && d->node >= 0 && (size_t)(d->node + 1) * 12 <= m_skin.size()) // rigid to the draw node
				{
					Xform(&m_skin[d->node * 12], v.pos, sp);
					XformDir(&m_skin[d->node * 12], v.nrm, sn);
					used = 1;
				}
				if (used) for (int c = 0; c < 3; c++) { p[c] = sp[c]; n[c] = sn[c]; }
			}
			RawVertex(v, mc.texgenSrc, o);   // uv / colour; world and texture matrices are applied in the vertex shader
			for (int c = 0; c < 3; c++) { o.pos[c] = p[c]; o.nrm[c] = n[c]; }
		}
		DrawMaterial(m_buf.data(), m_buf.size(), mesh->tris.data(), mesh->tris.size(), md);
	}
}
}
