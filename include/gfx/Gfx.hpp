#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Backend independent 2D drawing for the game's UI layers (Anita) and anything else flat. Game code includes this header; only
// platform/gfx_raylib.cpp knows raylib, so other backends (or a headless one for tests) can replace it.
//
// Screen space is the game's: 640 x 480 units, x to the right, y UP, origin in the middle of the screen. The backend fits that
// rectangle into the window (letterboxed) every frame.
namespace gfx
{
constexpr int SCREEN_W = 640;
constexpr int SCREEN_H = 480;

using TexId = int;          // 0 = no texture (flat colour)
constexpr TexId NO_TEX = 0;

enum Blend
{
	BLEND_ALPHA = 0,        // normal alpha blending (Anita "multiply" layers)
	BLEND_ADD = 1,          // additive (Anita "add" layers)
	BLEND_SUBTRACT = 2,     // GX_BM_SUBTRACT: destination - source (3D draws only)
	BLEND_NONE = 3          // GX_BM_NONE: the source replaces the destination (Anita "none" layers; 2D quads only)
};

// Uploads RGBA8 pixels (row 0 = top). Returns NO_TEX on failure.
TexId CreateTexture(int width, int height, const std::uint8_t* rgba);
void DestroyTexture(TexId tex);
// Replaces the whole image of a texture made by CreateTexture (same size), e.g. the next movie frame.
void UpdateTexture(TexId tex, int width, int height, const std::uint8_t* rgba);

// Decodes a JPEG (the THP movie frames) to RGBA8, row 0 = top.
bool DecodeJpeg(const std::uint8_t* data, std::size_t size, int& width, int& height, std::vector<std::uint8_t>& rgba);

// One textured quad. Corners are given top-left, bottom-left, top-right, bottom-right (the order of the Anita layer mesh);
// `xy` = 4 x (x, y) in screen space, `uv` = 4 x (u, v) with (0,0) = top-left of the image, v growing downwards.
// Colour is straight RGBA multiplied with the texture.
void DrawQuad(TexId tex, const float xy[8], const float uv[8], std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a,
              Blend blend, bool linearFilter);

// A transform applied to the corners of every following DrawQuad (x' = x * sx + tx, y' = y * sy + ty, game space): the push-back layers of
// the split screen are drawn scaled into one camera's viewport. Reset2DTransform() = identity.
void Set2DTransform(float sx, float sy, float tx, float ty);
void Reset2DTransform();
// Flushes queued quads and closes any blend state; the platform calls this once per frame before presenting.
void EndFrameFlush();
// Paints the window outside the 640 x 480 game rectangle black (after the game drew: layouts / effects wider than the screen spill there)
void DrawLetterbox();

// Writes the current frame buffer to a PNG. Call after drawing and before platform::EndFrame().
bool SaveScreenshot(const std::string& path);
}
