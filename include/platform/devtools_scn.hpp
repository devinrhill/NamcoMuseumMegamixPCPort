#pragma once

#include <string>

// The dev window's SCN0 tab (src/platform/devtools_scnedit.cpp): every SCN0 the game loads registers here (wii::ScnAnim::Load); an edited copy
// replaces the resource's samples (ScnAnim::Sample*) until the end of the run. Kept out of devtools.hpp so the wii layer does not pull it in.
namespace wii { class ScnAnim; }

namespace devtools
{
void ScnLoaded(const std::string& key, const wii::ScnAnim& anim);
const wii::ScnAnim* ScnEdited(const std::string& key);   // nullptr = not edited
void RegisterScnPanel();
}
