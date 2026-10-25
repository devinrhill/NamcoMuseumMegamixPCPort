#pragma once

#include <cstddef>
#include <cstdint>

#include "gfx/Gfx.hpp"

// Backend independent 3D drawing. World space is the Wii's: right handed, +Y up, the camera looks down -Z, counter-clockwise
// triangles face the viewer. Matrices are COLUMN-MAJOR float[16] (OpenGL layout). Game code builds vertex arrays (skinning etc.
// happens on the CPU in gfx/ModelDraw), the backend only rasterises them.
namespace gfx
{
struct Vertex3D
{
	float pos[3];
	float nrm[3];
	float uv[4][2];              // texture coordinate sets 0..3, (0,0) = top-left of the image
	std::uint8_t rgba[4];
};

enum Cull { CULL_NONE = 0, CULL_BACK = 1, CULL_FRONT = 2 };
enum Wrap { WRAP_CLAMP = 0, WRAP_REPEAT = 1, WRAP_MIRROR = 2 };

struct DrawState
{
	TexId tex{NO_TEX};
	Cull cull{CULL_BACK};
	Blend blend{BLEND_ALPHA};
	bool blended{false};         // false = opaque (no blending)
	bool depthTest{true};
	bool depthWrite{true};
	float alphaRef{0};           // alpha test reference (0..1)
	int alphaFunc{-1};           // GX compare (0 never .. 7 always) against alphaRef; -1 = the old rule: GEQUAL when alphaRef > 0, else off
	bool linear{true};
	Wrap wrapS{WRAP_REPEAT}, wrapT{WRAP_REPEAT};
	std::uint8_t color[4]{255, 255, 255, 255}; // multiplied with texture and vertex colour
};

// One TEV stage (GX register layout, see gfx/ModelDraw.cpp for how they are read): colour combiner A B C D with bias / sub / clamp /
// scale / destination, the same for alpha, and the texture / raster channel / konstant selection of the stage.
struct TevStage
{
	std::int8_t ca, cb, cc, cd, cbias, csub, cclamp, cscale, cdest;
	std::int8_t aa, ab, ac, ad, abias, asub, aclamp, ascale, adest;
	std::int8_t texmap, texcoord, texen, chan, kc, ka;
	// indirect texturing (BP IND_CMD 0x10 + stage): ind = indirect stage + 1 (0 = this stage does not use one), format (ITF_8..3),
	// bias bits (s t u), matrix id (1..3 = IND_MTX 0..2, 5..7 / 9..11 = the S / T dynamic ones), wrap s | t << 3, add the previous stage's offset
	std::int8_t ind, ifmt, ibias, imtx, iwrap, iaddprev;
};

// A material as the GPU needs it: its TEV program and pixel state. Colours are 0..255.
struct MaterialDraw
{
	int numStages{0};
	TevStage stages[16]{};
	float color[4][4]{};         // TEV registers PREV, C0, C1, C2 at the start of the program
	float konst[4][4]{};         // K0..K3
	int alphaComp[2]{7, 7}, alphaRef[2]{0, 0}, alphaOp{0}; // GX compare (7 = always), references, 0 and 1 xor 2 xnor 3
	float chanMat[2][4]{{255, 255, 255, 255}, {255, 255, 255, 255}}; // material colour of rasterised colour channels 0 and 1
	int chanCtrlColor[2]{}, chanCtrlAlpha[2]{};                      // bit 0 vertex colour source, bit 1 lighting
	float chanAmb[2][4]{};       // material ambient colour (the ambient register unless the vertex colour is the ambient source)
	bool lit{false};             // a directional light is set
	float lightPos[3]{}, lightAim[3]{};      // the light shines from lightPos towards lightAim (world space)
	float lightColor[3]{1, 1, 1}, specColor[3]{1, 1, 1}, shininess{16};
	// the g3d light set (numRig > 0 replaces the single light above): channel 0 = mat * clamp(sceneAmb * amb + sum diffuse), a specular
	// channel (attenuation GX_AF_SPEC) = mat * clamp(sceneAmb * amb + sum spec), GX spec attenuation x^2 / (s/2 + (1 - s/2) x^2), x = N.H
	int numRig{0};
	float rigDir[4][3]{}, rigCol[4][3]{}, rigSpec[4][4]{};   // dir surface -> light (world), colour, spec rgb + shininess (0 = none)
	float sceneAmb[3]{1, 1, 1};
	int envMask{0};              // bit t: texture coordinate t comes from the view space normal (texgen source GX_TG_NRM, env mapping)
	TexId tex[4]{};              // by texture map
	Wrap wrapS[4]{}, wrapT[4]{};
	Cull cull{CULL_BACK};
	bool blend{false};
	int blendSrc{4}, blendDst{5}; // GX blend factors (4 SRCALPHA, 5 INVSRCALPHA, 1 ONE); blendSrc < 0 = GX_BM_SUBTRACT (dst - src)
	bool depthTest{true}, depthWrite{true};
	int fogType{0};              // GX fog type (0 off, 2 linear, 4 exp, 5 exp2, 6 revexp, 7 revexp2)
	float fogStart{}, fogEnd{}, fogColor[3]{};
	// applied in the vertex shader: column-major model matrix (positions; its 3x3 part turns normals) and, per texture coordinate set,
	// u' = texMtx[0..2] . (u, v, 1), v' = texMtx[3..5] . (u, v, 1)
	float world[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	float texMtx[4][6]{{1, 0, 0, 0, 1, 0}, {1, 0, 0, 0, 1, 0}, {1, 0, 0, 0, 1, 0}, {1, 0, 0, 0, 1, 0}};
	// indirect texture stages (RAS1_IREF 0x27, RAS1_SS0/1 0x25 / 0x26): texture map, texture coordinate, coordinate shift s / t;
	// matrices IND_MTX 0..2 (0x06..0x0e) as two rows each, already scaled by 2^(scale - 17): offset in texels = row . (s, t, u)
	int numInd{0};
	int indMap[4]{}, indCoord[4]{}, indShift[4][2]{};
	float indMtx[3][2][3]{};
};

// Starts a 3D pass covering the whole window (or the letterboxed 4:3 area when `fit4x3`), clearing depth.
void Begin3D(const float view[16], const float proj[16], bool fit4x3);
// the view matrix (column major) of the open 3D pass (billboards need it)
const float* CurrentView();
void DrawTriangles(const Vertex3D* verts, std::size_t vertexCount, const std::uint32_t* indices, std::size_t indexCount, const DrawState& state);
// Draws with the TEV program of `m` (vertex colour, up to 4 texture coordinate sets and texture maps).
void DrawMaterial(const Vertex3D* verts, std::size_t vertexCount, const std::uint32_t* indices, std::size_t indexCount, const MaterialDraw& m);
// Static geometry kept on the GPU: uploaded once, drawn with DrawMaterialMesh (world / texture matrices from the MaterialDraw).
// 0 = not created (no 3D support, or more than 65535 vertices: use DrawMaterial).
using MeshId = unsigned;
MeshId CreateMesh(const Vertex3D* verts, std::size_t vertexCount, const std::uint32_t* indices, std::size_t indexCount);
void DestroyMesh(MeshId mesh);
void DrawMaterialMesh(MeshId mesh, const MaterialDraw& m);
void End3D();
// The share (0..1) of a square facing the camera (centre `c`, half size `half`, world units) that the depth already in the buffer does not
// hide: an occlusion query, outside a 3D pass (the lens flare's visibility test; the Wii reads the EFB depth back). -1 when unsupported.
float VisibleShare(const float view[16], const float proj[16], const float c[3], float half);
}
