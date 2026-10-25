#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// NW4R g3d animation resources read out of a BRRES (format notes: ~/Code/proj/unpac/revolution/brres/{chr0,srt0,pat0,scn0,anm}.py,
// NintendoWare/Revolution g3d_resanm*.h). Every `Load` takes the first block of the matching folder of the BRRES; the game's animation
// files (TITLE_PACMAN_01_CHA, TITLE_CLOUD_TSA, TITLE_CAMERA_GATE01_SCA, ...) hold exactly one.
namespace wii
{
class Brres;

// Frame of a looping / one-shot clip: loops wrap into [0, numFrame), one-shot clamps to [0, numFrame].
float ClipFrame(float frame, int numFrame, bool loops);

// ---- CHR0: bone animation -------------------------------------------------------------------------------------------------
struct BoneKey
{
	bool hasScale{}, hasRotate{}, hasTrans{};
	float scale[3]{1, 1, 1};
	float rotate[3]{};   // degrees
	float trans[3]{};
};

class ChrAnim
{
public:
	bool Load(const Brres& brres);
	int numFrame{};
	bool loops{};
	std::string name;
	// false when the clip does not animate that bone
	bool Sample(const std::string& bone, float frame, BoneKey& out) const;
	std::vector<std::string> BoneNames() const { std::vector<std::string> v; for (const auto& n : m_nodes) v.push_back(n.first); return v; }

private:
	struct Track
	{
		int enc{};              // 0 constant, 1 FVS32, 2 FVS48, 3 FVS96, 4 CV8, 5 CV16, 6 CV32
		float constant{};
		struct Key { float frame, value, slope; };
		std::vector<Key> keys;
		std::vector<float> samples;
		float Eval(float frame) const;
	};
	struct Node
	{
		std::uint32_t flags{};
		Track track[9];
		bool has[9]{};
	};
	std::map<std::string, Node> m_nodes;
};

// ---- SRT0: texture matrix animation -----------------------------------------------------------------------------------------
struct TexSrt
{
	float su{1}, sv{1}, rot{}, tu{}, tv{};
};

class SrtAnim
{
public:
	bool Load(const Brres& brres);
	int numFrame{};
	bool loops{};
	int texMtxMode{};       // 0 Maya, 1 XSI, 2 3dsmax
	// material name -> texture matrix index (0..7) -> animated SRT
	bool Sample(const std::string& material, int mtx, float frame, TexSrt& out) const;

private:
	struct Chan { bool constant{true}; float value{}; std::vector<std::array<float, 3>> keys; float Eval(float f) const; };
	struct Tex { std::uint32_t flags{}; Chan su, sv, rot, tu, tv; };
	std::map<std::string, std::map<int, Tex>> m_mats;
};

// (u', v') = (m[0] u + m[1] v + m[2], m[3] u + m[4] v + m[5]) for the SRT of a material's texture matrix (g3d_maya/xsi/3dsmax).
void TexMatrix(int mode, const TexSrt& srt, float m[6]);

// ---- PAT0: texture pattern (which texture a material map shows) ------------------------------------------------------------
class PatAnim
{
public:
	bool Load(const Brres& brres);
	int numFrame{};
	bool loops{};
	// Name of the texture material `material` shows in map `map` at `frame`; empty when the clip does not drive it.
	std::string Sample(const std::string& material, int map, float frame) const;

private:
	struct Frame { float frame; int tex; };
	struct Map { bool constant{}; int constTex{}; bool hasTex{}; std::vector<Frame> frames; };
	std::vector<std::string> m_texNames;
	std::map<std::string, std::map<int, Map>> m_mats;
};

// ---- VIS0: node visibility (g3d_resanmvis.h) ---------------------------------------------------------------------------------
class VisAnim
{
public:
	bool Load(const Brres& brres);
	int numFrame{};
	bool loops{};
	// Visibility of the node `bone` at `frame` (clamped to [0, numFrame], whole frames); false when the clip does not drive that node.
	bool Sample(const std::string& bone, float frame, bool& visible) const;

private:
	struct Node { std::uint32_t flags{}; std::vector<std::uint32_t> words; };
	std::map<std::string, Node> m_nodes;
};

// ---- CLR0: material colour animation (nw4r g3d_resanmclr.h, revision 4) ---------------------------------------------------
// Per material up to 11 targets: 0/1 material colour of channel 0/1, 2/3 ambient colour of channel 0/1, 4..6 TEV registers C0..C2,
// 7..10 TEV konstants K0..K3. Result colour = (material colour & mask) | animated colour (RGBA8, r in the top byte).
struct ClrResult
{
	std::uint32_t exist{};            // bit i: target i is animated
	std::uint32_t mask[11]{};
	std::uint32_t rgba[11]{};
};

class ClrAnim
{
public:
	bool Load(const Brres& brres);
	int numFrame{};
	bool loops{};
	// false when the clip does not animate `material`
	bool Sample(const std::string& material, float frame, ClrResult& out) const;
	std::vector<std::string> MaterialNames() const { std::vector<std::string> v; for (const auto& m : m_mats) v.push_back(m.first); return v; }

private:
	struct Target { std::uint32_t mask{}; bool constant{}; std::uint32_t value{}; std::vector<std::uint32_t> frames; };
	struct Mat { std::uint32_t exist{}; Target t[11]; };
	std::map<std::string, Mat> m_mats;
};

// ---- SCN0: scene animation (cameras only; lights / fog are not read yet) ---------------------------------------------------
struct CameraResult
{
	bool aim{};             // false: rotate camera
	int projType{};         // 0 perspective, 1 orthographic
	float pos[3]{};
	float aspect{1}, nearZ{1}, farZ{1000};
	float aimPos[3]{};      // aim cameras: look-at point; rotate cameras: Euler rotation in degrees (same slot)
	float twist{};          // degrees
	float fovy{45};         // perspective; orthographic: height
};

struct LightResult
{
	bool directional{};
	float pos[3]{}, aim[3]{};
	float color[3]{1, 1, 1};    // 0..1
	float specColor[3]{1, 1, 1};
	float shininess{16};
	// the scene's first fog (GX fog type: 2 linear, 4 exp, 5 exp2, 6 revexp, 7 revexp2; 0 = none), eye space distances
	int fogType{0};
	float fogStart{}, fogEnd{}, fogColor[3]{};
	// the whole light set (ScnAnim::SampleLightSet; numRig 0 = only the single light above): g3d LoadResMatChan / LightObj
	struct RigLight { float dir[3]; float color[3]; float spec[3]; float shininess; bool hasSpec; };   // dir: surface -> light, world space
	int numRig{0};
	RigLight rig[4]{};
	float ambient[3]{1, 1, 1};   // the light set's ambient light (multiplies the materials' ambient colours)
};

class ScnAnim
{
public:
	// first light of the scene at `frame` (false if it has none)
	bool SampleLight(float frame, LightResult& out) const;
	// fills the fog fields of `out` from the first fog (false if there is none)
	bool SampleFog(LightResult& out) const;
	// the first light set at `frame`: its ambient light and up to 4 enabled lights (also fills the single light fields with the first)
	bool SampleLightSet(float frame, LightResult& out) const;
	bool Load(const Brres& brres);
	int numFrame{};
	bool loops{};
	// first camera when `name` is empty
	bool SampleCamera(const std::string& name, float frame, CameraResult& out) const;

private:
	struct Slot { bool constant{true}; float value{}; std::vector<std::array<float, 3>> keys; float Eval(float f) const; };
	struct Cam
	{
		std::uint32_t flags{};
		int projType{};
		Slot slot[15];       // pos xyz, aspect, near, far, rot xyz, aim xyz, twist, fovy, orthoHeight
	};
	std::vector<std::pair<std::string, Cam>> m_cams;
	struct Light
	{
		std::string name;
		std::uint32_t flags{}; Slot pos[3], aim[3], shininess; std::uint32_t color{}, specColor{};
		std::vector<std::uint32_t> colorFrames, specFrames;   // per-frame colours (empty = constant)
	};
	std::vector<Light> m_lights;
	struct AmbLight { std::string name; std::uint32_t color{0xffffffff}; std::vector<std::uint32_t> frames; };
	std::vector<AmbLight> m_ambs;
	struct LightSet { std::string ambient; std::vector<std::string> lights; };
	std::vector<LightSet> m_sets;
	struct Fog { int type{}; float start{}, end{}; std::uint32_t color{}; };
	std::vector<Fog> m_fogs;
};
}
