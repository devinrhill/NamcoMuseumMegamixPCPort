#include "gfx/TextureCache.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

#include "gfx/Gfx.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Dol.hpp"

namespace gfx
{
namespace
{
std::unordered_map<std::string, std::unique_ptr<Texture>> g_cache;

std::string BaseName(const std::string& path)
{
	std::string s = path;
	size_t slash = s.find_last_of("/\\");
	if (slash != std::string::npos) s = s.substr(slash + 1);
	size_t dot = s.rfind('.');
	if (dot != std::string::npos && (s.substr(dot) == ".brres" || s.substr(dot) == ".tga" || s.substr(dot) == ".png")) s = s.substr(0, dot);
	return s;
}

std::string Lower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	return s;
}

// The engine textures "depth_tex" / "ramp16_tex" are not on the disc: cKingdomSys init (DOL 0x8000bcd8) registers the in-memory files
// "depth_tex.dat" (blob 0x80396c00, size word at 0x8047b000) and "ramp16_tex.dat" (blob 0x80416d00, size at 0x8047b004), BRRES files
// embedded in main.dol (textures "depth_dummy" and "ramp16"; cTaskScnRoot ctor 0x8009ac1c names them depth_tex / ramp16_tex).
const std::uint8_t* EngineBlob(const std::string& key, std::size_t& size)
{
	if (std::getenv("MEGAMIX_NOENGTEX")) return nullptr;
	std::uint32_t addr = 0, sizeAddr = 0;
	if (key == "depth_tex") { addr = 0x80396c00; sizeAddr = 0x8047b000; }
	else if (key == "ramp16_tex") { addr = 0x80416d00; sizeAddr = 0x8047b004; }
	else return nullptr;
	const wii::Dol* dol = wii::GameDol();
	if (dol == nullptr) return nullptr;
	const std::uint8_t* p = dol->At(sizeAddr, 4);
	if (p == nullptr) return nullptr;
	size = ((std::size_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
	return dol->At(addr, size);
}
}

Texture* FindTexture(const std::string& name)
{
	const std::string base = BaseName(name);
	if (base.empty()) return nullptr;
	const std::string key = Lower(base);
	auto it = g_cache.find(key);
	if (it != g_cache.end()) return it->second.get();

	std::size_t size = 0;
	if (key == "depth_tex" && !std::getenv("MEGAMIX_NOENGTEX"))
	{
		// depth_tex is the engine's shadow depth map: main.dol only embeds a zeroed placeholder ("depth_dummy", 512x512), the engine fills it
		// every frame with a depth render from the light (not ported). The materials' TEV marks a pixel as shadowed when the ramp16 value is
		// greater than the map, so the zeroed placeholder shadowed EVERYTHING; "cleared to far" (all ones) = nothing is in shadow.
		const std::vector<std::uint8_t> far(4 * 4 * 4, 0xff);
		auto out = std::make_unique<Texture>();
		out->name = base;
		out->width = out->height = 4;
		out->gfxId = CreateTexture(4, 4, far.data());
		Texture* raw = out.get();
		g_cache[key] = std::move(out);
		return raw;
	}
	const std::uint8_t* data = wii::ArchiveStore::Find(base + ".brres", size);
	if (data == nullptr) data = wii::ArchiveStore::Find(key + ".brres", size);
	if (data == nullptr) data = EngineBlob(key, size);
	if (data == nullptr) return nullptr; // not cached: an archive attached later may have it

	wii::Brres brres;
	if (!brres.Load(data, size) || brres.TextureNames().empty())
	{
		std::fprintf(stderr, "texture %s: not a usable BRRES\n", base.c_str());
		return nullptr;
	}
	std::string tex = brres.TextureNames().front();
	for (const std::string& t : brres.TextureNames())
	{
		if (Lower(t) == key) { tex = t; break; }
	}
	wii::Image img;
	if (!brres.DecodeTexture(tex, img))
	{
		std::fprintf(stderr, "texture %s: cannot decode\n", base.c_str());
		return nullptr;
	}
	auto out = std::make_unique<Texture>();
	out->name = base;
	out->width = img.width;
	out->height = img.height;
	out->gfxId = CreateTexture(img.width, img.height, img.rgba.data());
	Texture* raw = out.get();
	g_cache[key] = std::move(out);
	return raw;
}

void ClearTextureCache()
{
	for (auto& kv : g_cache)
	{
		if (kv.second->gfxId != NO_TEX) DestroyTexture(kv.second->gfxId);
	}
	g_cache.clear();
}
}
