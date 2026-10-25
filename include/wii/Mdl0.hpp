#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// NW4R g3d model ("MDL0", ResMdlData revision 11) reader. Format references: ~/WiiDevelopmentPackage/.../NintendoWare/Revolution
// g3d_resmdl / g3d_resshp / g3d_resmat headers, and ~/Code/proj/unpac/revolution/brres/mdl0.py + gxdl.py (the Python this follows).
//
// The model is read into a neutral form: nodes with bind matrices, one triangle mesh per shape (vertices already in model space
// for the bind pose, with their skin weights kept so a skeleton can move them later), materials with their texture slots, and the
// draw list (material, shape, node, priority) from the bytecode.
namespace wii
{
class Brres;

// ResUserData item (g3d_resuser.h): a named typed array (0 s32, 1 f32, 2 string); nodes carry them (gimmick parameters of the stage models)
struct Mdl0UserItem
{
	std::string name;
	int type{};
	std::vector<std::int32_t> ints;
	std::vector<float> floats;
	std::string str;
};

struct Mdl0Node
{
	std::string name;
	int id{};
	int mtxId{};
	int parent{-1};                  // node id of the parent, -1 for the root
	std::uint32_t flags{};
	int billboard{};                 // ResNodeData::Billboard: 0 off, 1 std, 2 persp std, 3 rot, 4 persp rot, 5 y, 6 persp y
	std::vector<Mdl0UserItem> userData;
	float scale[3]{1, 1, 1};
	float rotate[3]{};               // degrees
	float translate[3]{};
	float volMin[3]{};               // ResNodeData +0x44 / +0x50: the node's bounding volume (Galaga's cMesh hit boxes)
	float volMax[3]{};
	float model[12]{};               // bind pose model matrix, 3x4 row major
	float inv[12]{};                 // inverse of it
};

struct Mdl0Vertex
{
	float pos[3]{};
	float nrm[3]{};
	bool hasNrm{};
	float uv[8][2]{};
	bool hasUv[8]{};
	std::uint8_t clr[4]{255, 255, 255, 255}; // vertex colour 0
	bool hasClr{};
	// skin: (node id, weight) pairs; empty = rigid to the draw node
	std::array<int, 4> skinNode{{-1, -1, -1, -1}};
	std::array<float, 4> skinWeight{{0, 0, 0, 0}};
	int skinCount{};
};

struct Mdl0Mesh
{
	std::string name;                // shape name
	std::string posName;             // name of its position array (what the DOL classifies collision faces by)
	int shapeId{};
	std::vector<Mdl0Vertex> verts;
	std::vector<std::uint32_t> tris; // index triples
};

// One texture slot of a material (TexPlttInfo)
struct Mdl0TexSlot
{
	std::string texName;
	std::string plttName;
	int mapId{};                     // GX texture map the TEV stages read
	int wrapS{}, wrapT{};            // 0 clamp, 1 repeat, 2 mirror
	int minFilter{}, magFilter{};
	float lodBias{};
};

struct Mdl0TexSrt
{
	float su{1}, sv{1}, rot{}, tu{}, tv{};
};

struct Mdl0Material
{
	std::string name;
	int id{};
	std::uint32_t flag{};
	int cullMode{};                  // 0 none, 1 front, 2 back, 3 all = never drawn (GX_CULL_*)
	int nTexGens{}, nChans{}, nTevs{};
	int nIndStages{};                // ResGenModeData +3: indirect texture stages (GX_SetNumIndStages)
	int lightSetIdx{}, fogIdx{};     // ResMatMisc +1 / +2 (material +0x1d / +0x1e): < 0 = no light set / no fog, kept by SetLightSetIdx / SetFogIdx
	std::vector<Mdl0TexSlot> tex;
	std::vector<Mdl0TexSrt> srt;     // 8 entries (texture matrices)
	std::uint8_t matColor[2][4]{};   // channel 0 / 1 material colour
	std::uint8_t ambColor[2][4]{};
	std::uint32_t chanFlag[2]{};
	std::uint32_t chanCtrlColor[2]{}, chanCtrlAlpha[2]{};
	std::string tevName;             // name of the TEV program
	std::vector<std::uint8_t> tev;   // the TEV program: BP register writes (the blob minus its 0x20 byte header), see gfx/ModelDraw
	std::uint32_t texSrtFlag{};      // 4 bits per texture matrix: 1 exists, 2 scale one, 4 rotation zero, 8 translation zero
	int texMtxMode{};                // 0 Maya, 1 XSI, 2 3dsmax
	std::vector<std::uint8_t> dl;    // the 384 byte material display list (pixel / TEV colour / texgen registers)
};

struct Mdl0Draw
{
	int material{};
	int shape{};
	int node{};
	int priority{};
	std::string stream;              // bytecode stream: "DrawOpa", "DrawXlu"
};

class Mdl0
{
public:
	// `block` = offset of the MDL0 block inside the BRRES data
	bool Load(const Brres& brres, const std::string& name);

	std::string name;
	std::vector<Mdl0Node> nodes;                 // ordered by id
	std::vector<Mdl0Mesh> meshes;                // one per shape, in shape dictionary order
	std::vector<Mdl0Material> materials;
	std::vector<Mdl0Draw> draws;
	float volumeMin[3]{}, volumeMax[3]{};

	const Mdl0Node* NodeById(int id) const;
	const Mdl0Material* MaterialById(int id) const;
	const Mdl0Mesh* MeshByShapeId(int id) const;
};
}
