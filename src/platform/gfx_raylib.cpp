// raylib / rlgl backend of gfx/Gfx.hpp
#include "gfx/Gfx.hpp"
#include "platform/platform.hpp"
#include "platform/devui.hpp"

#include <raylib.h>
#include <rlgl.h>
#include <turbojpeg.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <cstring>
#include <vector>

// the game area: the window minus the dev window's strip on the right (devtools; the whole window without --devui)
static int GameScreenWidth() { return GetScreenWidth() - devui::ReservedWidth(); }
static int GameRenderWidth() { const int sw = GetScreenWidth(); return sw > 0 ? GetRenderWidth() * GameScreenWidth() / sw : GetRenderWidth(); }

namespace gfx
{
namespace
{
struct Tex
{
	unsigned id{};
	bool linear{true};          // current MIN / MAG filter (textures are created LINEAR)
	int wrapS{-1}, wrapT{-1};   // current wrap modes as Wrap values, -1 = not set yet
};
std::unordered_map<TexId, Tex> g_tex;
Blend g_blend = BLEND_ALPHA;
bool g_blendSet = false;
float g_xs = 1.0f, g_ys = 1.0f, g_xt = 0.0f, g_yt = 0.0f;   // Set2DTransform

// game screen space -> window pixels
struct View
{
	float cx, cy, s;
};
View CurrentView()
{
	const float w = (float)GameScreenWidth(), h = (float)GetScreenHeight();
	const float s = std::min(w / SCREEN_W, h / SCREEN_H);
	return {w * 0.5f, h * 0.5f, s};
}

void SetBlend(Blend b)
{
	if (g_blendSet && g_blend == b) return;
	rlDrawRenderBatchActive(); // flush what was queued with the previous blend
	if (g_blendSet) EndBlendMode();
	if (b == BLEND_NONE)
	{
		rlSetBlendFactors(RL_ONE, RL_ZERO, RL_FUNC_ADD);   // GX_BM_NONE
		BeginBlendMode(BLEND_CUSTOM);
	}
	else
		BeginBlendMode(b == BLEND_ADD ? BLEND_ADDITIVE : BLEND_ALPHA);
	g_blend = b;
	g_blendSet = true;
}
}

TexId CreateTexture(int width, int height, const std::uint8_t* rgba)
{
	if (width <= 0 || height <= 0 || rgba == nullptr) return NO_TEX;
	unsigned id = rlLoadTexture(rgba, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
	if (id == 0) return NO_TEX;
	rlTextureParameters(id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
	rlTextureParameters(id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
	rlTextureParameters(id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
	rlTextureParameters(id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
	g_tex[(TexId)id] = Tex{id, true};
	return (TexId)id;
}

void DestroyTexture(TexId tex)
{
	auto it = g_tex.find(tex);
	if (it == g_tex.end()) return;
	rlUnloadTexture(it->second.id);
	g_tex.erase(it);
}

void UpdateTexture(TexId tex, int width, int height, const std::uint8_t* rgba)
{
	auto it = g_tex.find(tex);
	if (it == g_tex.end()) return;
	rlDrawRenderBatchActive(); // quads queued with the old image must be drawn first
	rlUpdateTexture(it->second.id, 0, 0, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, rgba);
}

bool DecodeJpeg(const std::uint8_t* data, std::size_t size, int& width, int& height, std::vector<std::uint8_t>& rgba)
{
	// libjpeg-turbo: raylib's own image loader is built without JPEG support on some distributions
	static tjhandle tj = tjInitDecompress();
	if (tj == nullptr) return false;
	int subsamp = 0, cs = 0;
	if (tjDecompressHeader3(tj, data, (unsigned long)size, &width, &height, &subsamp, &cs) != 0)
	{
		std::fprintf(stderr, "jpeg: %s\n", tjGetErrorStr2(tj));
		return false;
	}
	rgba.resize((std::size_t)width * height * 4);
	// warnings (a THP frame may end without a clean EOI) still leave a usable image: only a hard failure drops the frame
	const int r = tjDecompress2(tj, data, (unsigned long)size, rgba.data(), width, 0, height, TJPF_RGBA, TJFLAG_FASTDCT);
	if (r != 0 && tjGetErrorCode(tj) == TJERR_FATAL)
	{
		std::fprintf(stderr, "jpeg: %s\n", tjGetErrorStr2(tj));
		return false;
	}
	return true;
}

void Set2DTransform(float sx, float sy, float tx, float ty)
{
	g_xs = sx;
	g_ys = sy;
	g_xt = tx;
	g_yt = ty;
}

void Reset2DTransform()
{
	g_xs = g_ys = 1.0f;
	g_xt = g_yt = 0.0f;
}

void DrawQuad(TexId tex, const float xy[8], const float uv[8], std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a,
              Blend blend, bool linearFilter)
{
	unsigned id = 0;
	if (tex != NO_TEX)
	{
		auto it = g_tex.find(tex);
		if (it != g_tex.end())
		{
			id = it->second.id;
			if (it->second.linear != linearFilter)
			{
				rlDrawRenderBatchActive(); // texture parameters affect everything already queued with this texture
				const int f = linearFilter ? RL_TEXTURE_FILTER_LINEAR : RL_TEXTURE_FILTER_NEAREST;
				rlTextureParameters(id, RL_TEXTURE_MIN_FILTER, f);
				rlTextureParameters(id, RL_TEXTURE_MAG_FILTER, f);
				it->second.linear = linearFilter;
			}
		}
	}
	SetBlend(blend);
	rlDisableBackfaceCulling();
	rlDisableDepthTest();

	const View v = CurrentView();
	rlSetTexture(id);
	rlBegin(RL_QUADS);
	rlCheckRenderBatchLimit(4);
	rlColor4ub(r, g, b, a);
	// TL, BL, BR, TR (indices 0, 1, 3, 2) so the quad winds consistently
	static const int order[4] = {0, 1, 3, 2};
	for (int k = 0; k < 4; k++)
	{
		const int i = order[k];
		rlTexCoord2f(uv[i * 2], uv[i * 2 + 1]);
		rlVertex2f(v.cx + (xy[i * 2] * g_xs + g_xt) * v.s, v.cy - (xy[i * 2 + 1] * g_ys + g_yt) * v.s);
	}
	rlEnd();
	rlSetTexture(0);
}

// MEGAMIX_DRAWSTATS=1: average DrawMaterial calls / vertices / indices per frame, printed every 120 frames (dev measurement)
std::size_t g_statCalls = 0, g_statVerts = 0, g_statIdx = 0, g_statQuads = 0;

void DrawLetterbox()
{
	rlDrawRenderBatchActive();
	if (g_blendSet)
	{
		EndBlendMode();
		g_blendSet = false;
	}
	const View v = CurrentView();
	const float w = (float)GameScreenWidth(), h = (float)GetScreenHeight();
	const float x0 = v.cx - 0.5f * SCREEN_W * v.s, x1 = v.cx + 0.5f * SCREEN_W * v.s;
	const float y0 = v.cy - 0.5f * SCREEN_H * v.s, y1 = v.cy + 0.5f * SCREEN_H * v.s;
	if (x0 > 0.0f) DrawRectangleRec({0.0f, 0.0f, x0, h}, BLACK);
	if (x1 < w) DrawRectangleRec({x1, 0.0f, w - x1, h}, BLACK);
	if (y0 > 0.0f) DrawRectangleRec({0.0f, 0.0f, w, y0}, BLACK);
	if (y1 < h) DrawRectangleRec({0.0f, y1, w, h - y1}, BLACK);
	rlDrawRenderBatchActive();
}

void EndFrameFlush()
{
	static const bool stats = std::getenv("MEGAMIX_DRAWSTATS") != nullptr;
	static int frames = 0;
	if (stats && ++frames == 120)
	{
		std::fprintf(stderr, "drawstats: %d fps, %zu calls (%zu single quads), %zu vertices, %zu indices per frame\n", GetFPS(), g_statCalls / 120, g_statQuads / 120, g_statVerts / 120, g_statIdx / 120);
		frames = 0; g_statCalls = g_statVerts = g_statIdx = g_statQuads = 0;
	}
	rlDrawRenderBatchActive();
	if (g_blendSet)
	{
		EndBlendMode();
		g_blendSet = false;
	}
}

bool SaveScreenshot(const std::string& path)
{
	rlDrawRenderBatchActive();
	if (g_blendSet)
	{
		EndBlendMode();
		g_blendSet = false;
	}
	Image img = LoadImageFromScreen();   // reading the multisampled window resolves it: the capture shows the smoothed frame
	bool ok = ExportImage(img, path.c_str());
	UnloadImage(img);
	return ok;
}
}

// ---- 3D ---------------------------------------------------------------------------------------------------------------------
#include "gfx/Gfx3D.hpp"
namespace gfx
{
namespace
{
bool g_in3d = false;
// depth shadows: the light pass renders into g_shFbo (a depth texture only matters, the colour attachment keeps the framebuffer complete)
bool g_shadowPass = false;
unsigned g_shFbo = 0, g_shColor = 0, g_shDepth = 0;
int g_shW = 0, g_shH = 0;
bool g_shRecv = false;          // receiver materials use g_shDepth with g_shLightVP
float g_shLightVP[16];
float g_view[16], g_proj[16];
void SetWrap(unsigned id, Wrap s, Wrap t)
{
	auto conv = [](Wrap w) { return w == WRAP_REPEAT ? RL_TEXTURE_WRAP_REPEAT : (w == WRAP_MIRROR ? RL_TEXTURE_WRAP_MIRROR_REPEAT : RL_TEXTURE_WRAP_CLAMP); };
	rlTextureParameters(id, RL_TEXTURE_WRAP_S, conv(s));
	rlTextureParameters(id, RL_TEXTURE_WRAP_T, conv(t));
}
}

const float* CurrentView() { return g_view; }

static void BeginPass(const float view[16], const float proj[16], bool fit4x3, bool clearDepth);

void Begin3D(const float view[16], const float proj[16], bool fit4x3) { BeginPass(view, proj, fit4x3, true); }

// the split screen's viewports (DOL cTaskScnRoot::Draw 0x8009bc4c: Camera::SetViewport + SetScissor of the camera's rectangle in the 640 x 480 screen)
void Begin3DRect(const float view[16], const float proj[16], float gx, float gy, float gw, float gh)
{
	std::copy(view, view + 16, g_view);
	std::copy(proj, proj + 16, g_proj);
	rlDrawRenderBatchActive();
	if (g_blendSet) { EndBlendMode(); g_blendSet = false; }
	const int w = GameRenderWidth(), h = GetRenderHeight();
	const int vw4 = std::min(w, h * 4 / 3), vh4 = vw4 * 3 / 4;
	const int vx4 = (w - vw4) / 2, vy4 = (h - vh4) / 2;
	const float sx = (float)vw4 / (float)SCREEN_W, sy = (float)vh4 / (float)SCREEN_H;
	const int x = vx4 + (int)(gx * sx), width = (int)(gw * sx), height = (int)(gh * sy);
	const int y = vy4 + (int)(((float)SCREEN_H - gy - gh) * sy);   // GL: origin bottom-left
	rlViewport(x, y, width, height);
	rlEnableScissorTest();
	rlScissor(x, y, width, height);
	using GlClearFn = void (*)(unsigned);
	static GlClearFn glClearFn = (GlClearFn)rlGetProcAddress("glClear");
	if (glClearFn != nullptr) glClearFn(0x00000100u); // GL_DEPTH_BUFFER_BIT
	else rlClearScreenBuffers();
	rlEnableDepthTest();
	g_in3d = true;
}

static void BeginPass(const float view[16], const float proj[16], bool fit4x3, bool clearDepth)
{
	std::copy(view, view + 16, g_view);
	std::copy(proj, proj + 16, g_proj);
	rlDrawRenderBatchActive();
	if (g_blendSet) { EndBlendMode(); g_blendSet = false; }
	const int w = GameRenderWidth(), h = GetRenderHeight();
	int vx = 0, vy = 0, vw = w, vh = h;
	if (fit4x3)
	{
		vw = std::min(w, h * 4 / 3);
		vh = vw * 3 / 4;
		vx = (w - vw) / 2;
		vy = (h - vh) / 2;
	}
	rlViewport(vx, vy, vw, vh);
	// depth only: the menus draw a 3D model between 2D layers (the title scene runs first, over the cleared black frame)
	if (clearDepth)
	{
		using GlClearFn = void (*)(unsigned);
		static GlClearFn glClearFn = (GlClearFn)rlGetProcAddress("glClear");
		if (glClearFn != nullptr) glClearFn(0x00000100u); // GL_DEPTH_BUFFER_BIT
		else rlClearScreenBuffers();
	}
	// 3D drawing uses its own vertex buffer and mvp (DrawMaterial), so rlgl's matrix stack is left alone
	rlEnableDepthTest();
	g_in3d = true;
}

void DrawTriangles(const Vertex3D* verts, std::size_t nv, const std::uint32_t* idx, std::size_t ni, const DrawState& st)
{
	// one TEV stage: colour = texture * vertex colour, alpha likewise
	MaterialDraw md;
	md.numStages = 1;
	TevStage& t = md.stages[0];
	t.ca = 15; t.cb = 8; t.cc = 10; t.cd = 15;          // lerp(zero, texture colour, raster colour)
	t.aa = 7; t.ab = 4; t.ac = 5; t.ad = 7;             // lerp(zero, texture alpha, raster alpha)
	t.cscale = t.ascale = 0; t.cclamp = t.aclamp = 1; t.cdest = t.adest = 0;
	t.texmap = 0; t.texcoord = 0; t.texen = st.tex != NO_TEX; t.chan = 0;
	md.chanCtrlColor[0] = md.chanCtrlAlpha[0] = 1;       // vertex colour (premultiplied by st.color below)
	md.tex[0] = st.tex; md.wrapS[0] = st.wrapS; md.wrapT[0] = st.wrapT;
	md.cull = st.cull; md.blend = st.blended; md.blendSrc = st.blend == BLEND_SUBTRACT ? -1 : 4; md.blendDst = st.blend == BLEND_ADD ? 1 : 5;
	md.depthTest = st.depthTest; md.depthWrite = st.depthWrite;
	if (st.alphaFunc >= 0) { md.alphaComp[0] = st.alphaFunc; md.alphaRef[0] = (int)(st.alphaRef * 255.0f + 0.5f); }
	else if (st.alphaRef > 0) { md.alphaComp[0] = 6; md.alphaRef[0] = (int)(st.alphaRef * 255.0f); }
	std::vector<Vertex3D> tmp(verts, verts + nv);
	for (Vertex3D& v : tmp)
		for (int c = 0; c < 4; c++) v.rgba[c] = (std::uint8_t)(v.rgba[c] * st.color[c] / 255);
	DrawMaterial(tmp.data(), tmp.size(), idx, ni, md);
}

// A GL occlusion query (GL_SAMPLES_PASSED, GL 3.3) against the depth the last 3D pass left: the square drawn with the depth test, then without
float VisibleShare(const float view[16], const float proj[16], const float c[3], float half)
{
	using GenQ = void (*)(int, unsigned*);
	using BeginQ = void (*)(unsigned, unsigned);
	using EndQ = void (*)(unsigned);
	using GetQ = void (*)(unsigned, unsigned, unsigned*);
	using ColorMask = void (*)(unsigned char, unsigned char, unsigned char, unsigned char);
	static GenQ genQ = (GenQ)rlGetProcAddress("glGenQueries");
	static BeginQ beginQ = (BeginQ)rlGetProcAddress("glBeginQuery");
	static EndQ endQ = (EndQ)rlGetProcAddress("glEndQuery");
	static GetQ getQ = (GetQ)rlGetProcAddress("glGetQueryObjectuiv");
	static ColorMask colorMask = (ColorMask)rlGetProcAddress("glColorMask");
	if (genQ == nullptr || beginQ == nullptr || endQ == nullptr || getQ == nullptr || colorMask == nullptr) return -1.0f;
	static unsigned q[2] = {0, 0};
	if (q[0] == 0) genQ(2, q);
	BeginPass(view, proj, true, false);
	// the camera's right / up from the view matrix rows: a square facing the camera
	const float r[3] = {view[0], view[4], view[8]}, u[3] = {view[1], view[5], view[9]};
	Vertex3D v[4]{};
	const float sx[4] = {-1, -1, 1, 1}, sy[4] = {1, -1, 1, -1};
	for (int i = 0; i < 4; i++)
	{
		for (int k = 0; k < 3; k++) v[i].pos[k] = c[k] + (r[k] * sx[i] + u[k] * sy[i]) * half;
		v[i].rgba[0] = v[i].rgba[1] = v[i].rgba[2] = v[i].rgba[3] = 255;
	}
	const std::uint32_t idx[6] = {0, 1, 2, 2, 1, 3};
	MaterialDraw md;
	md.numStages = 1;
	TevStage& t = md.stages[0];
	t.ca = 15; t.cb = 15; t.cc = 15; t.cd = 10; t.aa = 7; t.ab = 7; t.ac = 7; t.ad = 5;
	t.cclamp = t.aclamp = 1; t.texmap = -1; t.texcoord = -1; t.texen = 0; t.chan = 0;
	md.chanCtrlColor[0] = md.chanCtrlAlpha[0] = 1;
	md.cull = CULL_NONE;
	md.depthWrite = false;
	unsigned res[2] = {0, 0};
	colorMask(0, 0, 0, 0);
	for (int pass = 0; pass < 2; pass++)
	{
		md.depthTest = pass == 0;
		beginQ(0x8914u, q[pass]);   // GL_SAMPLES_PASSED
		DrawMaterial(v, 4, idx, 6, md);
		rlDrawRenderBatchActive();
		endQ(0x8914u);
	}
	colorMask(1, 1, 1, 1);
	for (int pass = 0; pass < 2; pass++) getQ(q[pass], 0x8866u, &res[pass]);   // GL_QUERY_RESULT (waits)
	End3D();
	return res[1] != 0 ? (float)res[0] / (float)res[1] : 0.0f;
}

void End3D()
{
	rlDrawRenderBatchActive();
	rlDisableDepthTest();
	rlEnableDepthMask();
	rlDisableBackfaceCulling();
	rlSetBlendMode(RL_BLEND_ALPHA);
	rlEnableColorBlend();
	rlDisableScissorTest();
	rlViewport(0, 0, GetRenderWidth(), GetRenderHeight());
	g_in3d = false;
}

// rlgl's immediate batch with the pass's view / projection (its matrix stack is otherwise unused in 3D, see BeginPass)
void DrawLines3D(const LineVertex* v, std::size_t n, bool depthTest)
{
	if (!g_in3d || n < 2) return;
	rlDrawRenderBatchActive();
	const auto toMatrix = [](const float a[16]) {
		Matrix m;
		m.m0 = a[0]; m.m1 = a[1]; m.m2 = a[2]; m.m3 = a[3]; m.m4 = a[4]; m.m5 = a[5]; m.m6 = a[6]; m.m7 = a[7];
		m.m8 = a[8]; m.m9 = a[9]; m.m10 = a[10]; m.m11 = a[11]; m.m12 = a[12]; m.m13 = a[13]; m.m14 = a[14]; m.m15 = a[15];
		return m;
	};
	const Matrix oldProj = rlGetMatrixProjection(), oldView = rlGetMatrixModelview();
	rlSetMatrixProjection(toMatrix(g_proj));
	rlSetMatrixModelview(toMatrix(g_view));
	using DepthRangeFn = void (*)(double, double);
	static DepthRangeFn depthRange = (DepthRangeFn)rlGetProcAddress("glDepthRange");
	if (depthTest) { rlEnableDepthTest(); if (depthRange != nullptr) depthRange(0.0, 0.9999); }
	else rlDisableDepthTest();
	rlDisableDepthMask();
	rlDisableBackfaceCulling();
	rlSetBlendMode(RL_BLEND_ALPHA);
	rlEnableColorBlend();
	rlSetTexture(0);
	const std::size_t chunk = 4096;   // vertices per batch (rlgl flushes when its buffer is full only between rlBegin / rlEnd calls)
	for (std::size_t s = 0; s + 1 < n; s += chunk)
	{
		const std::size_t e = std::min(n, s + chunk) & ~(std::size_t)1;
		rlCheckRenderBatchLimit((int)(e - s));
		rlBegin(RL_LINES);
		for (std::size_t i = s; i < e; i++)
		{
			rlColor4ub(v[i].rgba[0], v[i].rgba[1], v[i].rgba[2], v[i].rgba[3]);
			rlVertex3f(v[i].pos[0], v[i].pos[1], v[i].pos[2]);
		}
		rlEnd();
	}
	rlDrawRenderBatchActive();
	if (depthRange != nullptr) depthRange(0.0, 1.0);
	rlEnableDepthMask();
	rlEnableDepthTest();
	rlSetMatrixProjection(oldProj);
	rlSetMatrixModelview(oldView);
}
}

// ---- TEV material path -----------------------------------------------------------------------------------------------------
// A GX texture environment (up to 16 stages) evaluated in a fragment shader, with its own vertex buffer so every vertex can carry
// four texture coordinate sets (rlgl's immediate batch has one). Follows unpac revolution/brres/tev.py, in 0..1 instead of 0..255.
namespace gfx
{
namespace
{
const char* kTevVs = R"(#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNrm;
layout(location = 2) in vec2 aUv0;
layout(location = 3) in vec2 aUv1;
layout(location = 4) in vec2 aUv2;
layout(location = 5) in vec2 aUv3;
layout(location = 6) in vec4 aCol;
uniform mat4 uMvp;
uniform mat4 uWorld;
uniform vec3 uTm[8];   // per coordinate set: row u, row v
uniform vec3 uViewRow[3]; // world -> view rotation rows (environment mapping)
uniform int uEnvMask;  // bit t: coordinate set t from the view space normal
out vec2 vUv0;
out vec2 vUv1;
out vec2 vUv2;
out vec2 vUv3;
out vec4 vCol;
out vec3 vNrm;
out vec3 vPos;
out float vEyeZ;
void main()
{
	vPos = (uWorld*vec4(aPos, 1.0)).xyz;
	vNrm = mat3(uWorld)*aNrm;   // rotation part only (no non-uniform scale expected), as the old CPU path
	// GX_TG_NRM through g3d's camera environment matrix: (0.5 n.x + 0.5, 0.5 - 0.5 n.y) of the view space normal, then the SRT
	vec3 nv = normalize(vec3(dot(uViewRow[0], vNrm), dot(uViewRow[1], vNrm), dot(uViewRow[2], vNrm)));
	vec2 env = vec2(0.5*nv.x + 0.5, 0.5 - 0.5*nv.y);
	vec3 s0 = vec3((uEnvMask & 1) != 0 ? env : aUv0, 1.0);
	vec3 s1 = vec3((uEnvMask & 2) != 0 ? env : aUv1, 1.0);
	vec3 s2 = vec3((uEnvMask & 4) != 0 ? env : aUv2, 1.0);
	vec3 s3 = vec3((uEnvMask & 8) != 0 ? env : aUv3, 1.0);
	vUv0 = vec2(dot(uTm[0], s0), dot(uTm[1], s0));
	vUv1 = vec2(dot(uTm[2], s1), dot(uTm[3], s1));
	vUv2 = vec2(dot(uTm[4], s2), dot(uTm[5], s2));
	vUv3 = vec2(dot(uTm[6], s3), dot(uTm[7], s3));
	vCol = aCol;
	gl_Position = uMvp*vec4(vPos, 1.0);
	vEyeZ = gl_Position.w; // eye space distance of the perspective projection
}
)";

const char* kTevFs = R"(#version 330
in vec2 vUv0;
in vec2 vUv1;
in vec2 vUv2;
in vec2 vUv3;
in vec4 vCol;
in vec3 vNrm;
in vec3 vPos;
in float vEyeZ;
uniform int uFogType;
uniform vec4 uFog; // start, end, unused, blend is additive (fog colour = black)
uniform vec3 uFogColor;
uniform vec3 uEye;
uniform int uLit;
uniform vec3 uLightDir; // towards the light
uniform vec3 uLightColor;
uniform vec3 uSpecColor;
uniform float uShininess;
uniform vec4 uChanAmb[2];
uniform int uRigN;          // > 0: the g3d light set below replaces the single light
uniform vec3 uRigDir[4];
uniform vec3 uRigCol[4];
uniform vec4 uRigSpec[4];   // rgb, shininess (0 = no specular part)
uniform vec4 uRigPos[4];    // w = 1: point / spot light at xyz (diffuse direction from the position), 0: directional (uRigDir)
uniform vec3 uRigSpotDir[4];
uniform vec3 uRigAttnA[4];
uniform vec3 uRigAttnK[4];
uniform vec3 uSceneAmb;
uniform vec3 uViewDir;      // world space direction towards the viewer (GX's (0, 0, 1) of view space)
uniform sampler2D uTex0;
uniform sampler2D uTex1;
uniform sampler2D uTex2;
uniform sampler2D uTex3;
uniform int uTexOk[4];
uniform int uShadowOn;         // depth shadow receiver: maps uShadowMaps.x (ramp16_tex) / .y (depth_tex) are made from the world position
uniform ivec2 uShadowMaps;
uniform mat4 uLightVP;         // world -> light clip space (orthographic)
uniform sampler2D uShadowTex;  // the light camera's depth map
uniform int uNum;
uniform ivec4 uS0[16]; // colour a b c d
uniform ivec4 uS1[16]; // colour bias sub clamp scale
uniform ivec4 uS2[16]; // colour dest, alpha dest, alpha bias, alpha sub
uniform ivec4 uS3[16]; // alpha clamp, alpha scale, texmap, texcoord
uniform ivec4 uS4[16]; // tex enable, chan, konst colour sel, konst alpha sel
uniform ivec4 uS5[16]; // alpha a b c d
uniform ivec4 uS6[16]; // indirect: stage + 1 (0 = none), format, bias bits, matrix id
uniform ivec4 uS7[16]; // indirect: wrap, add the previous coordinate, bump alpha select, unused
uniform int uNumInd;
uniform ivec4 uInd[4];    // indirect stage: texture map, texture coordinate, shift s, shift t
uniform vec3 uIndMtx[6];  // IND_MTX 0..2, two rows each, scaled: offset in texels = row . (s, t, u)
uniform vec3 uIndScale;   // 2^(scale - 17) of IND_MTX 0..2 (the dynamic S / T matrices)
uniform vec4 uColor[4];
uniform vec4 uKonst[4];
uniform vec4 uChanMat[2];
uniform ivec2 uChanC;
uniform ivec2 uChanA;
uniform ivec4 uAlpha; // compare 0, compare 1, ref 0, ref 1
uniform int uAlphaOp;
out vec4 finalColor;

vec2 coordUv(int coord) { return coord == 0 ? vUv0 : coord == 1 ? vUv1 : coord == 2 ? vUv2 : vUv3; }

// The engine's ramp16_tex / depth_tex lookups of a shadow receiver (DOL: texgens 1 / 2 project the position through camera 4 and the ramp maps the
// light space depth onto two 8 bit texture coordinates): the 16 bit depth of the receiver / of the nearest caster as the GR16 compare reads it
// (R = low byte, G = high byte; the swap table of the original material is folded in). Outside the depth map nothing casts (the DOL draws a
// border of maximum depth around the map).
vec4 shadowSample(bool depthMap)
{
	vec4 lp = uLightVP*vec4(vPos, 1.0);
	vec3 n = lp.xyz/lp.w;
	float d = n.z*0.5 + 0.5;
	if (depthMap)
	{
		vec2 luv = n.xy*0.5 + 0.5;
		d = (luv.x < 0.0 || luv.x > 1.0 || luv.y < 0.0 || luv.y > 1.0) ? 1.0 : texture(uShadowTex, luv).r;
	}
	int z = int(floor(clamp(d, 0.0, 1.0)*65535.0 + 0.5));
	return vec4(float(z & 255)/255.0, float(z >> 8)/255.0, 0.0, 0.0);
}

vec4 sampleMapUv(int map, vec2 uv)
{
	if (uShadowOn != 0)
	{
		if (map == uShadowMaps.x) return shadowSample(false);
		if (map == uShadowMaps.y) return shadowSample(true);
	}
	// engine generated maps (ramp16_tex, depth_tex of the depth shadow receivers) are not produced: white = "not in shadow"
	if (uTexOk[map] == 0) return vec4(1.0);
	if (map == 0) return texture(uTex0, uv);
	if (map == 1) return texture(uTex1, uv);
	if (map == 2) return texture(uTex2, uv);
	return texture(uTex3, uv);
}

vec4 sampleMap(int map, int coord) { return sampleMapUv(map, coordUv(coord)); }

vec2 mapSize(int map)
{
	ivec2 z = map == 0 ? textureSize(uTex0, 0) : map == 1 ? textureSize(uTex1, 0) : map == 2 ? textureSize(uTex2, 0) : textureSize(uTex3, 0);
	return vec2(max(z, ivec2(1)));
}

float konstFrac(int sel) { return 1.0 - float(sel)/8.0; }

vec3 konstColor(int sel)
{
	if (sel < 8) return vec3(konstFrac(sel));
	if (sel >= 12 && sel <= 15) return uKonst[sel - 12].rgb;
	if (sel >= 16)
	{
		vec4 k = uKonst[sel & 3];
		int ch = sel & 28;
		return vec3(ch == 16 ? k.r : ch == 20 ? k.g : ch == 24 ? k.b : k.a);
	}
	return vec3(0.0);
}

float konstAlpha(int sel)
{
	if (sel < 8) return konstFrac(sel);
	if (sel >= 16)
	{
		vec4 k = uKonst[sel & 3];
		int ch = sel & 28;
		return ch == 16 ? k.r : ch == 20 ? k.g : ch == 24 ? k.b : k.a;
	}
	return 0.0;
}

vec4 rasChannel(int i)
{
	vec4 m = uChanMat[i];
	int cc = i == 0 ? uChanC.x : uChanC.y;
	int ca = i == 0 ? uChanA.x : uChanA.y;
	vec3 col = (cc & 1) != 0 ? vCol.rgb : m.rgb;
	float a = (ca & 1) != 0 ? vCol.a : m.a;
	if ((cc & 2) != 0)
	{
		// GX channel lighting: material * clamp(ambient + diffuse). Channel 1 is the specular term of the light (Blinn-Phong).
		vec3 n = normalize(vNrm);
		vec3 amb = (cc & 64) != 0 ? vCol.rgb : uChanAmb[i].rgb*uSceneAmb;
		if (uLit != 0 && uRigN > 0)
		{
			// g3d LoadResMatChan + GX: a GX_AF_SPEC channel (bit 9 on, bit 10 off) sums the specular lights, others the diffuse ones
			vec3 sum = vec3(0.0);
			bool spec = (cc & 512) != 0 && (cc & 1024) == 0;
			int dfn = (cc >> 7) & 3;   // diffuse function: 0 none, 1 sign, 2 clamp
			for (int k = 0; k < 4; k++)
			{
				if (k >= uRigN) break;
				vec3 ld = uRigDir[k];
				float att = 1.0;
				if (!spec && uRigPos[k].w > 0.5)
				{
					// GX per light: L = normalize(lpos - pos); GX_AF_SPOT (bits 9 + 10) adds the cos / distance attenuation
					vec3 lv = uRigPos[k].xyz - vPos;
					float dist = length(lv);
					ld = dist > 0.0 ? lv/dist : n;
					if ((cc & 1536) == 1536)
					{
						float c = max(dot(ld, -uRigSpotDir[k]), 0.0);
						vec3 a = uRigAttnA[k], kk = uRigAttnK[k];
						att = max(a.x + a.y*c + a.z*c*c, 0.0)/max(kk.x + kk.y*dist + kk.z*dist*dist, 1e-6);
					}
				}
				float nl = dot(n, ld);
				if (spec)
				{
					if (uRigSpec[k].w <= 0.0) continue;
					vec3 h = normalize(uRigDir[k] + uViewDir);
					float x = nl >= 0.0 ? max(dot(n, h), 0.0) : 0.0;
					float sh = uRigSpec[k].w*0.5;
					att = x*x/max(sh + (1.0 - sh)*x*x, 1e-6);
					float d = dfn == 0 ? 1.0 : dfn == 1 ? nl : max(nl, 0.0);
					sum += uRigSpec[k].rgb*att*d;
				}
				else sum += uRigCol[k]*att*(dfn == 0 ? 1.0 : dfn == 1 ? nl : max(nl, 0.0));
			}
			col = col*clamp(amb + sum, 0.0, 1.0);
		}
		else if (uLit == 0) { col = i == 0 ? col*min(vec3(1.0), amb) : vec3(0.0); }
		else if (i == 0)
		{
			float d = max(dot(n, uLightDir), 0.0);
			col = col*clamp(amb + uLightColor*d, 0.0, 1.0);
		}
		else
		{
			vec3 h = normalize(uLightDir + normalize(uEye - vPos));
			float sp = dot(n, uLightDir) > 0.0 ? pow(max(dot(n, h), 0.0), uShininess) : 0.0;
			col = col*uSpecColor*sp;
		}
	}
	return vec4(col, a);
}

bool cmpOp(int op, float v, float ref)
{
	if (op == 0) return false;
	if (op == 1) return v < ref;
	if (op == 2) return v == ref;
	if (op == 3) return v <= ref;
	if (op == 4) return v > ref;
	if (op == 5) return v != ref;
	if (op == 6) return v >= ref;
	return true;
}

void main()
{
	vec4 reg[4];
	reg[0] = vec4(0.0);
	reg[1] = uColor[1]; reg[2] = uColor[2]; reg[3] = uColor[3];
	vec4 ras0 = rasChannel(0), ras1 = rasChannel(1);
	if (uNum == 0)
	{
		reg[0] = ras0;
	}
	// indirect texture stages (GX / Dolphin): the map's (a, b, g) as 8 bit values (s, t, u)
	vec3 indc[4];
	for (int i = 0; i < 4; i++)
	{
		indc[i] = vec3(0.0);
		if (i < uNumInd)
		{
			ivec4 r = uInd[i];
			vec4 c = sampleMapUv(r.x, coordUv(r.y) / vec2(float(1 << r.z), float(1 << r.w)));
			indc[i] = floor(vec3(c.a, c.b, c.g)*255.0 + 0.5);
		}
	}
	vec2 tevcoord = vec2(0.0);
	float alphabump = 0.0;   // Dolphin: the last stage with a bump alpha select sets it (0 at the start)
	for (int s = 0; s < uNum; s++)
	{
		ivec4 s0 = uS0[s], s1 = uS1[s], s2 = uS2[s], s3 = uS3[s], s4 = uS4[s], s5 = uS5[s];
		// indirect offset: (fmt-masked, biased (s, t, u)) times the matrix, in texels of this stage's map (GX: fixed point coordinates)
		ivec4 s6 = uS6[s], s7 = uS7[s];
		vec2 off = vec2(0.0);
		vec2 size = mapSize(s3.z);
		vec2 uvt = coordUv(s3.w) * size;   // the stage's coordinate in texels (Dolphin's fixpoint_uv / 128)
		if (s6.x != 0)
		{
			ivec3 raw = ivec3(indc[s6.x - 1]);
			int mask = s6.y == 0 ? 255 : s6.y == 1 ? 31 : s6.y == 2 ? 15 : 7;
			// bump alpha: the raw value of the selected component, low bits cleared by the format (Dolphin: 0xf8 / 0xe0 / 0xf0 / 0xf8)
			if (s7.z != 0)
			{
				int bmask = s6.y == 1 ? 224 : s6.y == 2 ? 240 : 248;
				alphabump = float((s7.z == 1 ? raw.x : s7.z == 2 ? raw.y : raw.z) & bmask);
			}
			ivec3 ic = raw & ivec3(mask);
			int badd = s6.y == 0 ? -128 : 1;
			if ((s6.z & 1) != 0) ic.x += badd;
			if ((s6.z & 2) != 0) ic.y += badd;
			if ((s6.z & 4) != 0) ic.z += badd;
			vec3 c = vec3(ic);
			if (s6.w >= 1 && s6.w <= 3) { int k = (s6.w - 1)*2; off = vec2(dot(uIndMtx[k], c), dot(uIndMtx[k + 1], c)); }
			// dynamic matrices (Dolphin): S = the texel coordinate times s / 256, T = times t / 256, scaled by the matrix (id & 3) - 1
			else if (s6.w >= 5 && s6.w <= 7) off = uvt * (c.x / 256.0) * uIndScale[s6.w - 5];
			else if (s6.w >= 9 && s6.w <= 11) off = uvt * (c.y / 256.0) * uIndScale[s6.w - 9];
			// indirect wrap of the regular coordinate before the offset: 1..5 = 256 / 128 / 64 / 32 / 16 texels, 6 = 0
			int ws = s7.x & 7, wt = s7.x >> 3 & 7;
			if (ws == 6) uvt.x = 0.0; else if (ws >= 1 && ws <= 5) uvt.x = mod(uvt.x, float(256 >> (ws - 1)));
			if (wt == 6) uvt.y = 0.0; else if (wt >= 1 && wt <= 5) uvt.y = mod(uvt.y, float(256 >> (wt - 1)));
		}
		vec2 uv = (uvt + off) / size;
		tevcoord = s7.y != 0 ? tevcoord + uv : uv;
		vec4 t = s4.x != 0 ? sampleMapUv(s3.z, tevcoord) : vec4(0.0);
		// raster input: colour 0 / 1, 5 = alpha bump, 6 = alpha bump normalised (ab | ab >> 5), 7 = zero
		float abn = float(int(alphabump) | (int(alphabump) >> 5));
		vec4 r = s4.y == 0 ? ras0 : s4.y == 1 ? ras1 : s4.y == 5 ? vec4(alphabump / 255.0) : s4.y == 6 ? vec4(abn / 255.0) : vec4(0.0);
		vec3 kc = konstColor(s4.z);
		float ka = konstAlpha(s4.w);
		vec4 prev = reg[0];
		vec3 ca[4];
		int ci[4] = int[4](s0.x, s0.y, s0.z, s0.w);
		for (int k = 0; k < 4; k++)
		{
			int n = ci[k];
			vec3 v = vec3(0.0);
			if (n == 0) v = prev.rgb;
			else if (n == 1) v = vec3(prev.a);
			else if (n == 2) v = reg[1].rgb;
			else if (n == 3) v = vec3(reg[1].a);
			else if (n == 4) v = reg[2].rgb;
			else if (n == 5) v = vec3(reg[2].a);
			else if (n == 6) v = reg[3].rgb;
			else if (n == 7) v = vec3(reg[3].a);
			else if (n == 8) v = t.rgb;
			else if (n == 9) v = vec3(t.a);
			else if (n == 10) v = r.rgb;
			else if (n == 11) v = vec3(r.a);
			else if (n == 12) v = vec3(1.0);
			else if (n == 13) v = vec3(0.5);
			else if (n == 14) v = kc;
			ca[k] = v;
		}
		float bias[4] = float[4](0.0, 0.5, -0.5, 0.0);
		float scale[4] = float[4](1.0, 2.0, 4.0, 0.5);
		vec3 c;
		if (s1.x == 3)
		{
			// compare mode: D + (A > B or A == B ? C : 0); scale selects what is compared (R8, GR16, BGR24, per-channel RGB8)
			ivec3 ia = ivec3(floor(ca[0]*255.0 + 0.5)), ib = ivec3(floor(ca[1]*255.0 + 0.5));
			bool eq = s1.y != 0;
			vec3 sel;
			if (s1.w == 0) { bool t = eq ? ia.r == ib.r : ia.r > ib.r; sel = vec3(t ? 1.0 : 0.0); }
			else if (s1.w == 1) { int x = ia.g*256 + ia.r, y = ib.g*256 + ib.r; bool t = eq ? x == y : x > y; sel = vec3(t ? 1.0 : 0.0); }
			else if (s1.w == 2) { int x = (ia.b*256 + ia.g)*256 + ia.r, y = (ib.b*256 + ib.g)*256 + ib.r; bool t = eq ? x == y : x > y; sel = vec3(t ? 1.0 : 0.0); }
			else sel = vec3(eq ? equal(ia, ib) : greaterThan(ia, ib));
			c = ca[3] + sel*ca[2];
		}
		else
		{
			c = ca[0] + (ca[1] - ca[0])*ca[2];
			c = (s1.y != 0 ? ca[3] - c : ca[3] + c) + bias[s1.x];
			c *= scale[s1.w];
		}
		if (s1.z != 0) c = clamp(c, 0.0, 1.0);

		float aa[4];
		int ai[4] = int[4](s5.x, s5.y, s5.z, s5.w);
		for (int k = 0; k < 4; k++)
		{
			int n = ai[k];
			float v = 0.0;
			if (n == 0) v = prev.a;
			else if (n == 1) v = reg[1].a;
			else if (n == 2) v = reg[2].a;
			else if (n == 3) v = reg[3].a;
			else if (n == 4) v = t.a;
			else if (n == 5) v = r.a;
			else if (n == 6) v = ka;
			aa[k] = v;
		}
		float a;
		if (s2.z == 3)
		{
			float x = floor(aa[0]*255.0 + 0.5), y = floor(aa[1]*255.0 + 0.5);
			bool t = s2.w != 0 ? x == y : x > y;
			a = aa[3] + (t ? aa[2] : 0.0);
		}
		else
		{
			a = aa[0] + (aa[1] - aa[0])*aa[2];
			a = (s2.w != 0 ? aa[3] - a : aa[3] + a) + bias[s2.z];
			a *= scale[s3.y];
		}
		if (s3.x != 0) a = clamp(a, 0.0, 1.0);

		reg[s2.x].rgb = c;
		reg[s2.y].a = a;
	}
	vec4 o = clamp(reg[0], 0.0, 1.0);
	float a255 = floor(o.a*255.0 + 0.5);
	bool p0 = cmpOp(uAlpha.x, a255, float(uAlpha.z)), p1 = cmpOp(uAlpha.y, a255, float(uAlpha.w));
	bool pass = uAlphaOp == 0 ? (p0 && p1) : uAlphaOp == 1 ? (p0 || p1) : uAlphaOp == 2 ? (p0 != p1) : (p0 == p1);
	if (!pass) discard;
	if (uFogType != 0)
	{
		// GX fog (Dolphin's formulas): x = (z - start) / (end - start) clamped, then by type
		float x = clamp((vEyeZ - uFog.x) / (uFog.y - uFog.x), 0.0, 1.0);
		float f = x;
		if (uFogType == 4) f = 1.0 - exp2(-8.0*x);
		else if (uFogType == 5) f = 1.0 - exp2(-8.0*x*x);
		else if (uFogType == 6) f = exp2(-8.0*(1.0 - x));
		else if (uFogType == 7) f = exp2(-8.0*(1.0 - x)*(1.0 - x));
		vec3 fc = uFog.w > 0.5 ? vec3(0.0) : uFogColor;
		o.rgb = mix(o.rgb, fc, f);
	}
	finalColor = o;
}
)";

unsigned g_tevProg = 0, g_tevVao = 0, g_tevVbo = 0, g_tevEbo = 0;
bool g_tevTried = false;
struct TevLocs
{
	int eye, lit, lightDir, lightColor, specColor, shininess, chanAmb;
	int rigN, rigDir, rigCol, rigSpec, rigPos, rigSpotDir, rigAttnA, rigAttnK, sceneAmb, viewDir, viewRot, envMask;
	int fogType, fog, fogColor;
	int shadowOn, shadowMaps, lightVP, shadowTex;
	int mvp, world, tm, tex[4], texOk, num, s[8], numInd, ind, indMtx, indScale, color, konst, chanMat, chanC, chanA, alpha, alphaOp;
} g_loc;
constexpr std::size_t kMaxVerts = 65535, kMaxIndices = 300000;

// Vertex3D attributes of the bound vertex array / buffer
void SetVertexLayout()
{
	const int stride = (int)sizeof(Vertex3D);
	rlSetVertexAttribute(0, 3, RL_FLOAT, false, stride, 0);
	rlEnableVertexAttribute(0);
	rlSetVertexAttribute(1, 3, RL_FLOAT, false, stride, 12);
	rlEnableVertexAttribute(1);
	for (int i = 0; i < 4; i++)
	{
		rlSetVertexAttribute(2 + i, 2, RL_FLOAT, false, stride, 24 + 8 * i);
		rlEnableVertexAttribute(2 + i);
	}
	rlSetVertexAttribute(6, 4, RL_UNSIGNED_BYTE, true, stride, 56);
	rlEnableVertexAttribute(6);
}

TevLocs LoadLocs(unsigned prog)
{
	TevLocs g_loc;
	auto L = [prog](const char* n) { return rlGetLocationUniform(prog, n); };
	g_loc.mvp = L("uMvp");
	g_loc.world = L("uWorld");
	g_loc.tm = L("uTm");
	g_loc.eye = L("uEye"); g_loc.lit = L("uLit"); g_loc.lightDir = L("uLightDir"); g_loc.lightColor = L("uLightColor");
	g_loc.specColor = L("uSpecColor"); g_loc.shininess = L("uShininess"); g_loc.chanAmb = L("uChanAmb");
	g_loc.rigN = L("uRigN"); g_loc.rigDir = L("uRigDir"); g_loc.rigCol = L("uRigCol"); g_loc.rigSpec = L("uRigSpec");
	g_loc.rigPos = L("uRigPos"); g_loc.rigSpotDir = L("uRigSpotDir"); g_loc.rigAttnA = L("uRigAttnA"); g_loc.rigAttnK = L("uRigAttnK");
	g_loc.sceneAmb = L("uSceneAmb"); g_loc.viewDir = L("uViewDir"); g_loc.viewRot = L("uViewRow"); g_loc.envMask = L("uEnvMask");
	const char* tn[4] = {"uTex0", "uTex1", "uTex2", "uTex3"};
	for (int i = 0; i < 4; i++) g_loc.tex[i] = L(tn[i]);
	g_loc.texOk = L("uTexOk");
	g_loc.shadowOn = L("uShadowOn"); g_loc.shadowMaps = L("uShadowMaps"); g_loc.lightVP = L("uLightVP"); g_loc.shadowTex = L("uShadowTex");
	g_loc.num = L("uNum");
	const char* sn[8] = {"uS0", "uS1", "uS2", "uS3", "uS4", "uS5", "uS6", "uS7"};
	for (int i = 0; i < 8; i++) g_loc.s[i] = L(sn[i]);
	g_loc.numInd = L("uNumInd"); g_loc.ind = L("uInd"); g_loc.indMtx = L("uIndMtx"); g_loc.indScale = L("uIndScale");
	g_loc.color = L("uColor");
	g_loc.konst = L("uKonst");
	g_loc.chanMat = L("uChanMat");
	g_loc.chanC = L("uChanC");
	g_loc.chanA = L("uChanA");
	g_loc.alpha = L("uAlpha");
	g_loc.alphaOp = L("uAlphaOp");
	g_loc.fogType = L("uFogType"); g_loc.fog = L("uFog"); g_loc.fogColor = L("uFogColor");
	return g_loc;
}

bool EnsureTev()
{
	if (g_tevTried) return g_tevProg != 0;
	g_tevTried = true;
	g_tevProg = rlLoadShaderProgram(kTevVs, kTevFs);
	if (g_tevProg == 0) return false;
	g_loc = LoadLocs(g_tevProg);

	g_tevVao = rlLoadVertexArray();
	rlEnableVertexArray(g_tevVao);
	g_tevVbo = rlLoadVertexBuffer(nullptr, (int)(kMaxVerts * sizeof(Vertex3D)), true);
	SetVertexLayout();
	g_tevEbo = rlLoadVertexBufferElement(nullptr, (int)(kMaxIndices * sizeof(unsigned short)), true);
	rlDisableVertexArray();
	return true;
}

struct GpuMesh
{
	unsigned vao, vbo, ebo;
	int count;
};
std::unordered_map<MeshId, GpuMesh> g_meshes;
MeshId g_nextMesh = 1;

// TEV programs specialised per material configuration: the stage table, channel controls and alpha compare become constants of the
// fragment shader (same code as kTevFs), so the compiler unrolls the stage loop and drops the unused selections. The generic program
// walked up to 16 stages through uniform arrays for every pixel: with 4x MSAA that made the hub GPU bound (34 fps uncapped against 74
// with one stage). MEGAMIX_TEV_GENERIC=1 keeps the generic program (comparison).
struct TevProgram
{
	unsigned prog;
	TevLocs loc;
};
std::unordered_map<std::string, TevProgram> g_programs;

void StageInts(const TevStage& t, int out[8][4])
{
	const int v[8][4] = {{t.ca, t.cb, t.cc, t.cd}, {t.cbias, t.csub, t.cclamp, t.cscale}, {t.cdest, t.adest, t.abias, t.asub},
	                     {t.aclamp, t.ascale, t.texmap, t.texcoord}, {t.texen, t.chan, t.kc, t.ka}, {t.aa, t.ab, t.ac, t.ad},
	                     {t.ind, t.ifmt, t.ibias, t.imtx}, {t.iwrap, t.iaddprev, t.ibump, 0}};
	std::memcpy(out, v, sizeof v);
}

const TevProgram& ProgramFor(const MaterialDraw& m)
{
	static const bool generic = std::getenv("MEGAMIX_TEV_GENERIC") != nullptr;
	const int n = std::clamp(m.numStages, 0, 16);
	std::string key;
	key.reserve(64 + n * 24);
	auto add = [&](int v) { key += std::to_string(v); key += ','; };
	if (!generic)
	{
		add(n);
		for (int i = 0; i < n; i++) { int s[8][4]; StageInts(m.stages[i], s); for (auto& r : s) for (int v : r) add(v); }
		add(m.chanCtrlColor[0]); add(m.chanCtrlColor[1]); add(m.chanCtrlAlpha[0]); add(m.chanCtrlAlpha[1]);
		add(m.alphaComp[0]); add(m.alphaComp[1]); add(m.alphaRef[0]); add(m.alphaRef[1]); add(m.alphaOp);
	}
	auto it = g_programs.find(key);
	if (it != g_programs.end()) return it->second;
	TevProgram p{g_tevProg, g_loc};
	if (!generic)
	{
		std::string fs = kTevFs;
		auto repl = [&](const char* from, const std::string& to) { const std::size_t at = fs.find(from); if (at != std::string::npos) fs.replace(at, std::strlen(from), to); };
		const int len = std::max(n, 1);
		const char* names[8] = {"uS0", "uS1", "uS2", "uS3", "uS4", "uS5", "uS6", "uS7"};
		for (int k = 0; k < 8; k++)
		{
			std::string decl = "const ivec4 " + std::string(names[k]) + "[" + std::to_string(len) + "] = ivec4[" + std::to_string(len) + "](";
			for (int i = 0; i < len; i++)
			{
				int s[8][4] = {};
				if (i < n) StageInts(m.stages[i], s);
				decl += (i ? ", ivec4(" : "ivec4(") + std::to_string(s[k][0]) + "," + std::to_string(s[k][1]) + "," + std::to_string(s[k][2]) + "," + std::to_string(s[k][3]) + ")";
			}
			repl((std::string("uniform ivec4 ") + names[k] + "[16];").c_str(), decl + ");");
		}
		repl("uniform int uNum;", "const int uNum = " + std::to_string(n) + ";");
		repl("uniform ivec2 uChanC;", "const ivec2 uChanC = ivec2(" + std::to_string(m.chanCtrlColor[0]) + "," + std::to_string(m.chanCtrlColor[1]) + ");");
		repl("uniform ivec2 uChanA;", "const ivec2 uChanA = ivec2(" + std::to_string(m.chanCtrlAlpha[0]) + "," + std::to_string(m.chanCtrlAlpha[1]) + ");");
		repl("uniform ivec4 uAlpha;", "const ivec4 uAlpha = ivec4(" + std::to_string(m.alphaComp[0]) + "," + std::to_string(m.alphaComp[1]) + "," +
		     std::to_string(m.alphaRef[0]) + "," + std::to_string(m.alphaRef[1]) + ");");
		repl("uniform int uAlphaOp;", "const int uAlphaOp = " + std::to_string(m.alphaOp) + ";");
		const unsigned prog = rlLoadShaderProgram(kTevVs, fs.c_str());
		if (prog != 0) p = {prog, LoadLocs(prog)};
		else std::fprintf(stderr, "gfx: specialised TEV program failed, using the generic one\n");
	}
	return g_programs.emplace(key, p).first->second;
}
}

namespace
{
// GL state, shader uniforms and texture units of a material (the TEV program stays bound until EndMaterial)
void ApplyMaterial(const MaterialDraw& mIn)
{
	// the light pass (DOL 0x80024ce4: ForceDrawMode, colour update off, GX_CULL_FRONT, z compare LEQUAL with writes): no blending, no alpha test
	MaterialDraw shadowCopy;
	if (g_shadowPass)
	{
		shadowCopy = mIn;
		shadowCopy.blend = false;
		shadowCopy.cull = CULL_FRONT;
		shadowCopy.depthTest = shadowCopy.depthWrite = true;
		shadowCopy.alphaComp[0] = shadowCopy.alphaComp[1] = 7;
		shadowCopy.shadowRampMap = shadowCopy.shadowDepthMap = -1;
	}
	const MaterialDraw& m = g_shadowPass ? shadowCopy : mIn;
	// state
	if (m.blend)
	{
		rlEnableColorBlend();
		if (m.blendSrc < 0)
		{
			rlSetBlendFactors(RL_ONE, RL_ONE, RL_FUNC_REVERSE_SUBTRACT); // GX_BM_SUBTRACT: dst - src
			rlSetBlendMode(RL_BLEND_CUSTOM);
		}
		else if (m.blendSrc == 4 && m.blendDst == 5) rlSetBlendMode(RL_BLEND_ALPHA);
		else
		{
			// GX blend factors (GX_BL_*): source 0 zero, 1 one, 2 dst colour, 3 1 - dst colour, 4 src alpha, 5 1 - src alpha, 6 dst alpha,
			// 7 1 - dst alpha; the destination's 2 / 3 are the SOURCE colour. Everything but alpha / additive fell back to alpha blending
			// (Gator Panic's water mizu is src alpha + src colour: the scene behind it is tinted by the water).
			static const int common[8] = {RL_ZERO, RL_ONE, 0, 0, RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_DST_ALPHA, RL_ONE_MINUS_DST_ALPHA};
			auto f = [](int v, bool dst) {
				if (v == 2) return dst ? RL_SRC_COLOR : RL_DST_COLOR;
				if (v == 3) return dst ? RL_ONE_MINUS_SRC_COLOR : RL_ONE_MINUS_DST_COLOR;
				return common[v & 7];
			};
			rlSetBlendFactors(f(m.blendSrc, false), f(m.blendDst, true), RL_FUNC_ADD);
			rlSetBlendMode(RL_BLEND_CUSTOM);
		}
	}
	else rlDisableColorBlend();
	if (m.cull == CULL_NONE) rlDisableBackfaceCulling();
	else { rlEnableBackfaceCulling(); rlSetCullFace(m.cull == CULL_BACK ? RL_CULL_FACE_BACK : RL_CULL_FACE_FRONT); }
	static const bool noDepth = std::getenv("MEGAMIX_NODEPTH") != nullptr;   // debug: no depth test
	if (m.depthTest && !noDepth) rlEnableDepthTest(); else rlDisableDepthTest();
	if (m.depthWrite) rlEnableDepthMask(); else rlDisableDepthMask();

	const TevProgram& prog = ProgramFor(m);
	const TevLocs& g_loc = prog.loc;   // this program's locations (constants of a specialised program have none: -1, ignored by GL)
	rlEnableShader(prog.prog);
	// mvp = proj * view (column-major)
	float mvp[16];
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
		{
			float v = 0;
			for (int k = 0; k < 4; k++) v += g_proj[k * 4 + r] * g_view[c * 4 + k];
			mvp[c * 4 + r] = v;
		}
	Matrix mat;
	float* mf = &mat.m0; // raylib's Matrix is 16 floats in column-major order (m0, m4, m8, m12 are a row in the struct but memory is m0..m15)
	mat.m0 = mvp[0]; mat.m1 = mvp[1]; mat.m2 = mvp[2]; mat.m3 = mvp[3]; mat.m4 = mvp[4]; mat.m5 = mvp[5]; mat.m6 = mvp[6]; mat.m7 = mvp[7];
	mat.m8 = mvp[8]; mat.m9 = mvp[9]; mat.m10 = mvp[10]; mat.m11 = mvp[11]; mat.m12 = mvp[12]; mat.m13 = mvp[13]; mat.m14 = mvp[14]; mat.m15 = mvp[15];
	(void)mf;
	rlSetUniformMatrix(g_loc.mvp, mat);

	// eye position from the view matrix (view = R | t, eye = -R^T t) and the light
	{
		const float* v = g_view;
		const float eye[3] = {-(v[0] * v[12] + v[1] * v[13] + v[2] * v[14]), -(v[4] * v[12] + v[5] * v[13] + v[6] * v[14]), -(v[8] * v[12] + v[9] * v[13] + v[10] * v[14])};
		rlSetUniform(g_loc.eye, eye, RL_SHADER_UNIFORM_VEC3, 1);
		float dir[3] = {m.lightPos[0] - m.lightAim[0], m.lightPos[1] - m.lightAim[1], m.lightPos[2] - m.lightAim[2]};
		const float l = std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
		if (l > 0) for (float& c : dir) c /= l;
		const int lit = m.lit;
		rlSetUniform(g_loc.lit, &lit, RL_SHADER_UNIFORM_INT, 1);
		rlSetUniform(g_loc.lightDir, dir, RL_SHADER_UNIFORM_VEC3, 1);
		rlSetUniform(g_loc.lightColor, m.lightColor, RL_SHADER_UNIFORM_VEC3, 1);
		rlSetUniform(g_loc.specColor, m.specColor, RL_SHADER_UNIFORM_VEC3, 1);
		rlSetUniform(g_loc.shininess, &m.shininess, RL_SHADER_UNIFORM_FLOAT, 1);
		float amb[2][4];
		for (int i = 0; i < 2; i++) for (int k = 0; k < 4; k++) amb[i][k] = m.chanAmb[i][k] / 255.0f;
		rlSetUniform(g_loc.chanAmb, amb, RL_SHADER_UNIFORM_VEC4, 2);
		const int rigN = m.lit ? m.numRig : 0;
		rlSetUniform(g_loc.rigN, &rigN, RL_SHADER_UNIFORM_INT, 1);
		if (rigN > 0)
		{
			rlSetUniform(g_loc.rigDir, m.rigDir, RL_SHADER_UNIFORM_VEC3, 4);
			rlSetUniform(g_loc.rigCol, m.rigCol, RL_SHADER_UNIFORM_VEC3, 4);
			rlSetUniform(g_loc.rigSpec, m.rigSpec, RL_SHADER_UNIFORM_VEC4, 4);
			rlSetUniform(g_loc.rigPos, m.rigPos, RL_SHADER_UNIFORM_VEC4, 4);
			rlSetUniform(g_loc.rigSpotDir, m.rigSpotDir, RL_SHADER_UNIFORM_VEC3, 4);
			rlSetUniform(g_loc.rigAttnA, m.rigAttnA, RL_SHADER_UNIFORM_VEC3, 4);
			rlSetUniform(g_loc.rigAttnK, m.rigAttnK, RL_SHADER_UNIFORM_VEC3, 4);
		}
		rlSetUniform(g_loc.sceneAmb, m.sceneAmb, RL_SHADER_UNIFORM_VEC3, 1);
		// view matrix rows (column-major g_view: row r = v[r], v[4 + r], v[8 + r]); the viewer direction = view row 2
		const float viewDir[3] = {v[2], v[6], v[10]};
		rlSetUniform(g_loc.viewDir, viewDir, RL_SHADER_UNIFORM_VEC3, 1);
		const float rows[9] = {v[0], v[4], v[8], v[1], v[5], v[9], v[2], v[6], v[10]};
		rlSetUniform(g_loc.viewRot, rows, RL_SHADER_UNIFORM_VEC3, 3);
		rlSetUniform(g_loc.envMask, &m.envMask, RL_SHADER_UNIFORM_INT, 1);
	}

	// rlTextureParameters binds the texture to the ACTIVE unit and unbinds it afterwards, so all parameters are set before any unit is
	// filled (setting them in the same loop wiped the previous unit: every map but the last one used sampled black).
	int texOk[4];
	unsigned texId[4];
	for (int i = 0; i < 4; i++)
	{
		auto it = m.tex[i] != NO_TEX ? g_tex.find(m.tex[i]) : g_tex.end();
		texOk[i] = it != g_tex.end();
		if (texOk[i])
		{
			// only when they change: every parameter call rebinds the texture and makes the driver revalidate it (perf: ~1500 calls / frame in the hub)
			Tex& tx = it->second;
			texId[i] = tx.id;
			if (!tx.linear)
			{
				rlTextureParameters(texId[i], RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
				rlTextureParameters(texId[i], RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
				tx.linear = true;
			}
			if (tx.wrapS != (int)m.wrapS[i] || tx.wrapT != (int)m.wrapT[i])
			{
				SetWrap(texId[i], m.wrapS[i], m.wrapT[i]);
				tx.wrapS = (int)m.wrapS[i]; tx.wrapT = (int)m.wrapT[i];
			}
		}
		else texId[i] = rlGetTextureIdDefault();
	}
	for (int i = 0; i < 4; i++)
	{
		rlActiveTextureSlot(i);
		rlEnableTexture(texId[i]);
		rlSetUniform(g_loc.tex[i], &i, RL_SHADER_UNIFORM_INT, 1);
	}
	rlSetUniform(g_loc.texOk, texOk, RL_SHADER_UNIFORM_INT, 4);
	{
		const int on = g_shRecv && m.shadowDepthMap >= 0 && m.shadowRampMap >= 0 ? 1 : 0;
		rlSetUniform(g_loc.shadowOn, &on, RL_SHADER_UNIFORM_INT, 1);
		if (on)
		{
			const int maps[2] = {m.shadowRampMap, m.shadowDepthMap};
			rlSetUniform(g_loc.shadowMaps, maps, RL_SHADER_UNIFORM_IVEC2, 1);
			Matrix lv;
			const float* c = g_shLightVP;
			lv.m0 = c[0]; lv.m1 = c[1]; lv.m2 = c[2]; lv.m3 = c[3]; lv.m4 = c[4]; lv.m5 = c[5]; lv.m6 = c[6]; lv.m7 = c[7];
			lv.m8 = c[8]; lv.m9 = c[9]; lv.m10 = c[10]; lv.m11 = c[11]; lv.m12 = c[12]; lv.m13 = c[13]; lv.m14 = c[14]; lv.m15 = c[15];
			rlSetUniformMatrix(g_loc.lightVP, lv);
			const int unit = 4;
			rlActiveTextureSlot(unit);
			rlEnableTexture(g_shDepth);
			rlSetUniform(g_loc.shadowTex, &unit, RL_SHADER_UNIFORM_INT, 1);
			rlActiveTextureSlot(0);
		}
	}

	int s[8][16][4] = {};
	for (int i = 0; i < m.numStages && i < 16 && g_loc.s[0] >= 0; i++)
	{
		int v[8][4];
		StageInts(m.stages[i], v);
		for (int k = 0; k < 8; k++) for (int j = 0; j < 4; j++) s[k][i][j] = v[k][j];
	}
	const int num = std::min(m.numStages, 16);
	rlSetUniform(g_loc.num, &num, RL_SHADER_UNIFORM_INT, 1);
	for (int k = 0; k < 8; k++) rlSetUniform(g_loc.s[k], s[k], RL_SHADER_UNIFORM_IVEC4, 16);
	{
		int ind[4][4] = {};
		float mtx[6][3];
		for (int i = 0; i < 4; i++) { ind[i][0] = m.indMap[i]; ind[i][1] = m.indCoord[i]; ind[i][2] = m.indShift[i][0]; ind[i][3] = m.indShift[i][1]; }
		for (int k = 0; k < 3; k++) for (int r = 0; r < 2; r++) for (int c = 0; c < 3; c++) mtx[k * 2 + r][c] = m.indMtx[k][r][c];
		rlSetUniform(g_loc.numInd, &m.numInd, RL_SHADER_UNIFORM_INT, 1);
		rlSetUniform(g_loc.ind, ind, RL_SHADER_UNIFORM_IVEC4, 4);
		rlSetUniform(g_loc.indMtx, mtx, RL_SHADER_UNIFORM_VEC3, 6);
		rlSetUniform(g_loc.indScale, m.indScale, RL_SHADER_UNIFORM_VEC3, 1);
	}
	float color[4][4], konst[4][4], chan[2][4];
	for (int i = 0; i < 4; i++)
		for (int k = 0; k < 4; k++) { color[i][k] = m.color[i][k] / 255.0f; konst[i][k] = m.konst[i][k] / 255.0f; }
	for (int i = 0; i < 2; i++)
		for (int k = 0; k < 4; k++) chan[i][k] = m.chanMat[i][k] / 255.0f;
	rlSetUniform(g_loc.color, color, RL_SHADER_UNIFORM_VEC4, 4);
	rlSetUniform(g_loc.konst, konst, RL_SHADER_UNIFORM_VEC4, 4);
	rlSetUniform(g_loc.chanMat, chan, RL_SHADER_UNIFORM_VEC4, 2);
	rlSetUniform(g_loc.chanC, m.chanCtrlColor, RL_SHADER_UNIFORM_IVEC2, 1);
	rlSetUniform(g_loc.chanA, m.chanCtrlAlpha, RL_SHADER_UNIFORM_IVEC2, 1);
	const int alpha[4] = {m.alphaComp[0], m.alphaComp[1], m.alphaRef[0], m.alphaRef[1]};
	rlSetUniform(g_loc.alpha, alpha, RL_SHADER_UNIFORM_IVEC4, 1);
	rlSetUniform(g_loc.alphaOp, &m.alphaOp, RL_SHADER_UNIFORM_INT, 1);
	rlSetUniform(g_loc.fogType, &m.fogType, RL_SHADER_UNIFORM_INT, 1);
	{
		const float fog[4] = {m.fogStart, m.fogEnd, 0.0f, (m.blend && m.blendDst == 1) || m.blendSrc < 0 ? 1.0f : 0.0f}; // additive / subtract blends fade to black
		rlSetUniform(g_loc.fog, fog, RL_SHADER_UNIFORM_VEC4, 1);
		rlSetUniform(g_loc.fogColor, m.fogColor, RL_SHADER_UNIFORM_VEC3, 1);
	}

	{
		const float* c = m.world;   // column-major; raylib's Matrix fields are named by that index (m0..m15), declared in another order
		Matrix w;
		w.m0 = c[0]; w.m1 = c[1]; w.m2 = c[2]; w.m3 = c[3]; w.m4 = c[4]; w.m5 = c[5]; w.m6 = c[6]; w.m7 = c[7];
		w.m8 = c[8]; w.m9 = c[9]; w.m10 = c[10]; w.m11 = c[11]; w.m12 = c[12]; w.m13 = c[13]; w.m14 = c[14]; w.m15 = c[15];
		rlSetUniformMatrix(g_loc.world, w);
		float tm[8][3];
		for (int i = 0; i < 4; i++) for (int k = 0; k < 3; k++) { tm[i * 2][k] = m.texMtx[i][k]; tm[i * 2 + 1][k] = m.texMtx[i][3 + k]; }
		rlSetUniform(g_loc.tm, tm, RL_SHADER_UNIFORM_VEC3, 8);
	}
}

void EndMaterial()
{
	for (int i = 4; i >= 0; i--)
	{
		rlActiveTextureSlot(i);
		rlDisableTexture();
	}
	rlActiveTextureSlot(0);
	rlDisableShader();
}
}

void DrawMaterial(const Vertex3D* verts, std::size_t nv, const std::uint32_t* idx, std::size_t ni, const MaterialDraw& m)
{
	if (!g_in3d || ni < 3 || nv == 0) return;
	if (nv > kMaxVerts || ni > kMaxIndices)
	{
		std::fprintf(stderr, "gfx: mesh too large for the TEV path (%zu vertices, %zu indices)\n", nv, ni);
		return;
	}
	if (!EnsureTev()) return;
	rlDrawRenderBatchActive(); // whatever the immediate path queued goes first
	g_statCalls++; g_statVerts += nv; g_statIdx += ni; if (nv == 4) g_statQuads++;

	static std::vector<unsigned short> idx16;
	idx16.resize(ni - ni % 3);
	for (std::size_t i = 0; i < idx16.size(); i++) idx16[i] = (unsigned short)(idx[i] < nv ? idx[i] : 0);

	ApplyMaterial(m);
	rlEnableVertexArray(g_tevVao);
	rlUpdateVertexBuffer(g_tevVbo, verts, (int)(nv * sizeof(Vertex3D)), 0);
	rlUpdateVertexBufferElements(g_tevEbo, idx16.data(), (int)(idx16.size() * sizeof(unsigned short)), 0);
	rlDrawVertexArrayElements(0, (int)idx16.size(), nullptr);
	rlDisableVertexArray();
	EndMaterial();
}

MeshId CreateMesh(const Vertex3D* verts, std::size_t nv, const std::uint32_t* idx, std::size_t ni)
{
	if (nv == 0 || nv > 65535 || ni < 3 || !EnsureTev()) return 0;
	std::vector<unsigned short> idx16(ni - ni % 3);
	for (std::size_t i = 0; i < idx16.size(); i++) idx16[i] = (unsigned short)(idx[i] < nv ? idx[i] : 0);
	rlDrawRenderBatchActive();
	GpuMesh g{};
	g.vao = rlLoadVertexArray();
	rlEnableVertexArray(g.vao);
	g.vbo = rlLoadVertexBuffer(verts, (int)(nv * sizeof(Vertex3D)), false);
	SetVertexLayout();
	g.ebo = rlLoadVertexBufferElement(idx16.data(), (int)(idx16.size() * sizeof(unsigned short)), false);
	rlDisableVertexArray();
	g.count = (int)idx16.size();
	const MeshId id = g_nextMesh++;
	g_meshes[id] = g;
	return id;
}

void DestroyMesh(MeshId mesh)
{
	auto it = g_meshes.find(mesh);
	if (it == g_meshes.end()) return;
	if (IsWindowReady())
	{
		rlUnloadVertexArray(it->second.vao);
		rlUnloadVertexBuffer(it->second.vbo);
		rlUnloadVertexBuffer(it->second.ebo);
	}
	g_meshes.erase(it);
}

void DrawMaterialMesh(MeshId mesh, const MaterialDraw& m)
{
	if (!g_in3d) return;
	auto it = g_meshes.find(mesh);
	if (it == g_meshes.end() || !EnsureTev()) return;
	rlDrawRenderBatchActive();
	g_statCalls++; g_statIdx += (std::size_t)it->second.count;
	ApplyMaterial(m);
	rlEnableVertexArray(it->second.vao);
	rlDrawVertexArrayElements(0, it->second.count, nullptr);
	rlDisableVertexArray();
	EndMaterial();
}

// Depth shadows (DOL cKingdomShadow update 0x80024ce4): the light pass writes only depth (GXSetColorUpdate(0)) into a depth texture
bool ShadowBeginPass(const float view[16], const float proj[16], int w, int h)
{
	if (g_in3d || w <= 0 || h <= 0 || !EnsureTev()) return false;
	if (g_shFbo == 0 || g_shW != w || g_shH != h)
	{
		if (g_shFbo != 0) { rlUnloadFramebuffer(g_shFbo); rlUnloadTexture(g_shColor); rlUnloadTexture(g_shDepth); }
		g_shFbo = rlLoadFramebuffer();
		g_shColor = rlLoadTexture(nullptr, w, h, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
		g_shDepth = rlLoadTextureDepth(w, h, false);
		rlFramebufferAttach(g_shFbo, g_shColor, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
		rlFramebufferAttach(g_shFbo, g_shDepth, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
		if (!rlFramebufferComplete(g_shFbo))
		{
			std::fprintf(stderr, "gfx: the shadow map framebuffer is incomplete\n");
			rlUnloadFramebuffer(g_shFbo); rlUnloadTexture(g_shColor); rlUnloadTexture(g_shDepth);
			g_shFbo = g_shColor = g_shDepth = 0;
			return false;
		}
		g_shW = w; g_shH = h;
	}
	std::copy(view, view + 16, g_view);
	std::copy(proj, proj + 16, g_proj);
	rlDrawRenderBatchActive();
	if (g_blendSet) { EndBlendMode(); g_blendSet = false; }
	rlEnableFramebuffer(g_shFbo);
	rlViewport(0, 0, w, h);
	rlDisableScissorTest();
	rlEnableDepthMask();
	rlEnableDepthTest();
	rlClearScreenBuffers();   // colour + depth of the shadow framebuffer
	rlColorMask(false, false, false, false);
	g_in3d = true;
	g_shadowPass = true;
	return true;
}

void ShadowEndPass()
{
	if (!g_shadowPass) return;
	rlDrawRenderBatchActive();
	rlColorMask(true, true, true, true);
	rlDisableFramebuffer();
	g_shadowPass = false;
	End3D();   // restores the window viewport and the 2D state
}

void SetShadowReceiver(const float view[16], const float proj[16])
{
	if (view == nullptr || proj == nullptr || g_shDepth == 0) { g_shRecv = false; return; }
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
		{
			float v = 0;
			for (int k = 0; k < 4; k++) v += proj[k * 4 + r] * view[c * 4 + k];
			g_shLightVP[c * 4 + r] = v;
		}
	g_shRecv = true;
}

}
