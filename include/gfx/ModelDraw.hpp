#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gfx/Gfx3D.hpp"
#include "wii/Anim.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

// Draws a parsed MDL0 (wii/Mdl0) through gfx/Gfx3D. Textures come from the model's own BRRES first, then from the attached
// archives (gfx::FindTexture). The draw order is the model's: "DrawOpa" stream, then "DrawXlu", each by priority.
// Shading runs the material's TEV program (stages, konstants, alpha compare, blend, z mode) in a fragment shader; lighting is reduced to
// "scene ambient is white, no light enabled" which is what the title scenes use (UNVERIFIED for other scenes); indirect stages are not done.
namespace gfx
{
class ModelDraw
{
public:
	// Keeps a copy of the BRRES (textures are decoded lazily from it).
	bool Load(const wii::Brres& brres, const std::string& modelName);

	const wii::Mdl0& Model() const { return m_mdl; }

	// One CHR0 layer of a pose: the clip, the frame to sample (already looped / clamped) and its blend weight.
	struct ChrLayer { const wii::ChrAnim* anim; float frame; float weight; };

	// Poses the skeleton from blended CHR0 layers (bones a layer does not animate keep their bind SRT; bones several layers
	// animate are blended by weight: scale / translation linearly, rotation as a normalised quaternion). Empty = bind pose.
	void SetPose(const std::vector<ChrLayer>& layers);

	// Texture animation: material name -> SRT0 texture matrix 0, and material name -> PAT0 texture of map 0. nullptr = static.
	void SetTexSrt(const wii::SrtAnim* anim, float frame) { m_srts.clear(); if (anim) m_srts.push_back({anim, frame}); }
	void SetTexPat(const wii::PatAnim* anim, float frame) { m_pats.clear(); if (anim) m_pats.push_back({anim, frame}); }
	// further layers (a kingdom model's blend slots each hold their own SRT0 / PAT0 clip): per material the first layer that animates it wins
	void AddTexSrt(const wii::SrtAnim* anim, float frame) { if (anim) m_srts.push_back({anim, frame}); }
	void AddTexPat(const wii::PatAnim* anim, float frame) { if (anim) m_pats.push_back({anim, frame}); }
	// Node visibility animation (VIS0): draws bound to a node the clip hides at `frame` are skipped. nullptr = every node visible.
	void SetVisAnim(const wii::VisAnim* anim, float frame) { m_vis = anim; m_visFrame = frame; }
	// Material colour animation (CLR0): channel material / ambient colours, TEV registers C0..C2 and konstants K0..K3. nullptr = static.
	void SetClrAnim(const wii::ClrAnim* anim, float frame) { m_clr = anim; m_clrFrame = frame; }

	// The scene's directional light for this model (SCN0 light: shines from `pos` towards `aim`, world space). Without one, lit
	// materials get ambient only.
	// GX fog (type 0 = off): applied after the TEV; start / end are eye space distances
	void SetFog(int type, float start, float end, const float color[3])
	{
		m_fogType = type; m_fogStart = start; m_fogEnd = end;
		for (int i = 0; i < 3; i++) m_fogColor[i] = color[i];
	}
	void SetLight(const float pos[3], const float aim[3], const float color[3], const float spec[3], float shininess)
	{
		m_hasLight = true;
		for (int i = 0; i < 3; i++) { m_lightPos[i] = pos[i]; m_lightAim[i] = aim[i]; m_lightColor[i] = color[i]; m_specColor[i] = spec[i]; }
		m_shininess = shininess;
	}

	// The whole g3d light set (wii::ScnAnim::SampleLightSet); a LightResult with numRig 0 keeps the single light
	void SetLightRig(const wii::LightResult& l)
	{
		SetLight(l.pos, l.aim, l.color, l.specColor, l.shininess);
		m_rig = l;
	}

	// Per node skin matrices (3x4 row major, = animated model matrix * bind inverse); empty = bind pose.
	void SetSkin(const std::vector<float>& skin3x4PerNode) { m_skin = skin3x4PerNode; }
	// the skin matrices of the current pose (SetPose / SetSkin); empty = bind pose
	const std::vector<float>& Skin() const { return m_skin; }

	// Model-space matrix (3x4 row major) of a node in the current pose (bind pose until SetPose is given layers). False for an unknown node.
	bool NodeMatrix(const std::string& name, float out[12]) const;

	// Between Begin3D and End3D. `world` = column-major 4x4 placed in front of the model.
	void Draw(const float world[16]);
	// g3d draws a scene's opaque stream (DrawOpa) of every model before any translucent one (DrawXlu). The scene sets the pass:
	// 0 both streams (default, a model on its own), 1 opaque only, 2 translucent only.
	static void SetPass(int pass) { s_pass = pass; }
	static int Pass() { return s_pass; }
	bool HasXlu() const;

private:
	struct Tex { TexId id{NO_TEX}; bool loaded{false}; bool warned{false}; };
	// Per draw (index into m_mdl.draws): rigid geometry kept on the GPU (CreateMesh), placed by a matrix uniform. Shared by copies.
	struct GpuDraw { bool tried{false}; MeshId mesh{0}; int rigidNode{-1}; };   // rigidNode -1: bound to the draw node
	struct GpuCache { std::vector<GpuDraw> draws; ~GpuCache(); };
	std::shared_ptr<GpuCache> m_gpu;
	TexId TextureFor(const std::string& name);

	wii::Brres m_brres;
	wii::Mdl0 m_mdl;
	std::map<std::string, Tex> m_tex;
	std::vector<float> m_skin;
	std::vector<float> m_nodeWorld;   // 12 floats per node: animated model matrix (3x4 row major)
	static inline int s_pass = 0;
	bool m_hasLight{};
	wii::LightResult m_rig{};
	int m_fogType{0};
	float m_fogStart{}, m_fogEnd{}, m_fogColor[3]{};
	float m_lightPos[3]{}, m_lightAim[3]{}, m_lightColor[3]{1, 1, 1}, m_specColor[3]{1, 1, 1}, m_shininess{16};
	struct SrtLayer { const wii::SrtAnim* anim; float frame; };
	std::vector<SrtLayer> m_srts;
	const wii::VisAnim* m_vis{};
	float m_visFrame{};
	const wii::ClrAnim* m_clr{};
	float m_clrFrame{};
	struct PatLayer { const wii::PatAnim* anim; float frame; };
	std::vector<PatLayer> m_pats;
	std::vector<Vertex3D> m_buf;
	struct MatCache { bool built{false}; MaterialDraw md; int texgenSrc[4]{}; int numTexGens{}; };
	std::vector<MatCache> m_matCache;
};
}
