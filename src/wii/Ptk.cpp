#include "wii/Ptk.hpp"

#include <cstring>

namespace wii
{
namespace
{
// Sequential big-endian reader over one record, in the field order of format.py.
struct Cur
{
	const std::uint8_t* p;
	std::uint32_t U()
	{
		std::uint32_t v = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
		p += 4;
		return v;
	}
	int I() { return (int)U(); }
	float F()
	{
		std::uint32_t u = U();
		float f;
		std::memcpy(&f, &u, 4);
		return f;
	}
	void Skip(int words) { p += 4 * words; }
	std::string S()
	{
		std::string s((const char*)p, strnlen((const char*)p, 256));
		p += 256;
		return s;
	}
};

void ReadGen(const std::uint8_t* rec, PtkGen& g)
{
	Cur c{rec};
	g.name = c.S();
	g.tex = c.S();
	g.genFlag = c.U(); g.ptcFlag = c.U(); g.texFlag = c.U(); c.Skip(1);
	g.genMax = c.F(); g.genMaxFrm = c.F(); g.genBorn = c.F(); g.genDead = c.F();
	g.genStyle = c.I(); c.Skip(3);
	for (int i = 0; i < 3; i++) g.genSize[i] = c.F();
	c.F(); // gen_size_w
	g.genPower = c.F(); g.genPowerRnd = c.F(); g.genAngle = c.F(); g.genAngleRnd = c.F();
	for (int i = 0; i < 3; i++) g.genPos[i] = c.F();
	c.F(); // gen_pos_w
	g.genDirY = c.F(); g.genDirX = c.F();
	c.F(); c.F(); // gen_dir_z / w
	g.genRot = c.F(); g.genRotRnd = c.F(); g.genRotSpd = c.F(); g.genRotSpdRnd = c.F();
	g.genAccele = c.F(); c.Skip(3);
	g.scaleXCurve = c.I(); g.scaleXs = c.F(); g.scaleXsRnd = c.F(); g.scaleXe = c.F();
	g.scaleYCurve = c.I(); g.scaleYs = c.F(); g.scaleYsRnd = c.F(); g.scaleYe = c.F();
	for (int i = 0; i < 4; i++) g.rgbaCurve[i] = c.I();
	for (int i = 0; i < 4; i++) g.rgbaS[i] = c.F();
	for (int i = 0; i < 4; i++) g.rgbaE[i] = c.F();
	g.alphaTestFunc = c.I(); g.alphaTestRef = c.F(); c.Skip(2);
	for (int i = 0; i < 3; i++) { g.rotRnd[i] = c.F(); g.rotSpd[i] = c.F(); g.rotSpdRnd[i] = c.F(); g.rot[i] = c.F(); }
	g.weightCurve = c.I(); g.weightS = c.F(); g.weightSRnd = c.F(); g.weightE = c.F(); g.age = c.F(); g.ageRnd = c.F(); c.Skip(2);
	g.ptcStyle = c.I(); c.Skip(3);
	g.sizeX = c.F(); g.sizeY = c.F(); g.sizeXRnd = c.F(); g.sizeYRnd = c.F();
	g.blendStyle = c.I(); c.Skip(3);
	g.jump = c.F(); c.Skip(3);
	g.alphaFrm = c.F(); c.Skip(3);
	g.feelAirCurve = c.I(); g.feelAirS = c.F(); g.feelAirE = c.F(); c.Skip(1);
	g.feelRotGenS = c.F(); g.feelRotGenRnd = c.F(); g.feelRotGenE = c.F(); g.feelRotGenCurve = c.I();
	g.texPatFrm = c.F(); g.texPatCurve = c.I(); c.Skip(2);
	g.texFilter = c.I(); g.texDivX = c.I(); g.texDivY = c.I(); c.Skip(1);
	g.texPatRangeS = c.I(); g.texPatRangeE = c.I(); c.Skip(1);
	g.scaleXeRnd = c.F(); g.scaleYeRnd = c.F();
}
}

bool PtkFile::Parse(const std::uint8_t* d, std::size_t size)
{
	constexpr std::size_t kRec = 1024, kHeader = 16;
	if (size < kHeader + 2 * kRec || std::memcmp(d, "UKTP162N", 8) != 0) return false;
	{
		Cur c{d + kHeader};
		scn.name = c.S();
		c.S();                      // path_relative
		scn.flag = c.U(); c.Skip(3);
		c.I(); c.I();               // scn_max, emi_max
		scn.genMax = c.I(); c.Skip(1);
		c.I(); c.Skip(3);           // path_type
		c.I(); c.Skip(3);           // bg_tex_filter
		c.S();                      // bg_tex_name
		for (int i = 0; i < 5; i++) c.F(); // bg rgba, gakaku
		c.Skip(3);
		c.Skip(12);                 // ui_si_pos / itr / rot
		scn.loopFrame = c.F(); scn.drawScale = c.F(); c.Skip(2);
		c.F(); c.Skip(3);           // fps_draw
		scn.airPower = c.F(); c.Skip(3);
		scn.airDirX = c.F(); scn.airDirY = c.F();
	}
	const std::size_t n = scn.genMax < 0 ? 0 : (std::size_t)scn.genMax;
	if (size < kHeader + (2 + n) * kRec) return false;
	gens.resize(n);
	for (std::size_t i = 0; i < n; i++) ReadGen(d + kHeader + (2 + i) * kRec, gens[i]);
	return true;
}

std::string PtkTextureName(const std::string& path)
{
	std::size_t s = path.find_last_of("/\\");
	std::string n = s == std::string::npos ? path : path.substr(s + 1);
	std::size_t dot = n.find_last_of('.');
	return dot == std::string::npos ? n : n.substr(0, dot);
}
}
