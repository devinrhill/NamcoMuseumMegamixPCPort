#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// The jet particle file "UKTP162N" (.pto / PTK), big endian: 16 B header, scene record, emitter record, gen_max generator
// records, each 1024 B (jet/ptk/datPtk.hpp, cJetPtkData::SetData). Field order = the user's unpac/kingdom/jetptk/format.py
// (derived from the converted C#; not traced against the DOL, UNVERIFIED).
namespace wii
{
struct PtkScene
{
	std::string name;
	std::uint32_t flag{};
	std::int32_t genMax{};
	float loopFrame{}, drawScale{1}, airPower{}, airDirX{}, airDirY{};
};

struct PtkGen
{
	std::string name, tex;
	std::uint32_t genFlag{}, ptcFlag{}, texFlag{};
	float genMax{}, genMaxFrm{}, genBorn{}, genDead{};
	int genStyle{};
	float genSize[3]{}, genPower{}, genPowerRnd{}, genAngle{}, genAngleRnd{}, genPos[3]{}, genDirY{}, genDirX{};
	float genRot{}, genRotRnd{}, genRotSpd{}, genRotSpdRnd{}, genAccele{1};
	int scaleXCurve{}, scaleYCurve{};
	float scaleXs{}, scaleXsRnd{}, scaleXe{}, scaleXeRnd{}, scaleYs{}, scaleYsRnd{}, scaleYe{}, scaleYeRnd{};
	int rgbaCurve[4]{};                         // r g b a
	float rgbaS[4]{}, rgbaE[4]{};
	int alphaTestFunc{};
	float alphaTestRef{};
	float rotRnd[3]{}, rotSpd[3]{}, rotSpdRnd[3]{}, rot[3]{};
	int weightCurve{};
	float weightS{}, weightSRnd{}, weightE{}, age{}, ageRnd{};
	int ptcStyle{};
	float sizeX{}, sizeY{}, sizeXRnd{}, sizeYRnd{};
	int blendStyle{};
	float jump{}, alphaFrm{};
	int feelAirCurve{};
	float feelAirS{}, feelAirE{}, feelRotGenS{}, feelRotGenRnd{}, feelRotGenE{};
	int feelRotGenCurve{};
	float texPatFrm{};
	int texPatCurve{}, texFilter{}, texDivX{1}, texDivY{1}, texPatRangeS{}, texPatRangeE{};
};

struct PtkFile
{
	PtkScene scn;
	std::vector<PtkGen> gens;
	// False when the data is not a UKTP162N file.
	bool Parse(const std::uint8_t* data, std::size_t size);
};

// "./texture/ww/g10e_leaf01.tga" -> "g10e_leaf01"
std::string PtkTextureName(const std::string& path);
}
