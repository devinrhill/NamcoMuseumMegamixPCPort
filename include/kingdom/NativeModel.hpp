#pragma once

#include <memory>
#include <string>

#include "kingdom/cKingdomModelBase.hpp"

#include "gfx/ModelDraw.hpp"
#include "wii/Anim.hpp"

// Internals shared by cKingdomModelBase.cpp and cKingdomModel.cpp (native model state, see cKingdomModelBase.cpp).
namespace kingdom
{
// per instance state of a model: the shared draw object, the animation slots (CHR0 bone clips blended by weight, SRT0 / PAT0 texture animation)
struct cKingdomModelBase::Native
{
	std::shared_ptr<gfx::ModelDraw> draw;
	std::string key;
	struct Chr { std::shared_ptr<wii::ChrAnim> anim; float frame{0}, rate{1}, weight{1}; } chr[BLEND_MAX];
	struct Srt { std::shared_ptr<wii::SrtAnim> anim; float frame{0}, rate{1}; } srt[BLEND_MAX];
	struct Pat { std::shared_ptr<wii::PatAnim> anim; float frame{0}, rate{1}; } pat[BLEND_MAX];
	struct Vis { std::shared_ptr<wii::VisAnim> anim; float frame{0}, rate{1}; } vis[BLEND_MAX];
	struct Clr { std::shared_ptr<wii::ClrAnim> anim; float frame{0}, rate{1}; } clr[BLEND_MAX];
};

std::shared_ptr<gfx::ModelDraw> LoadNativeModel(const std::string& archiveEntry);
void DrawNativeModel(cKingdomModelBase::Native* native, const float world[16], int lightSet, int fogIdx = -1);
bool NativeNodeMatrix(cKingdomModelBase::Native* native, const std::string& node, float m3x4[12]);
bool NativeHasModel(const cKingdomModelBase::Native* native);
}
