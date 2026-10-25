#pragma once

#include <string>

#include "common.hpp"

// Textures by name, the way the game finds them: "<name>.brres" in the attached archives (most recent first), decoded from the
// TEX0 inside and uploaded once. Names are matched without directory or extension, case-insensitively.
namespace gfx
{
// nullptr when no attached archive has the texture. The returned object lives as long as the cache.
Texture* FindTexture(const std::string& name);

// Forget everything (e.g. after archives were detached). GPU textures are released.
void ClearTextureCache();
}
