#pragma once

#include <string>

#include "common.hpp"

// Native stand-in for the Unity assets the C# code fetched with ResourceDatabase.Get<Object>(key): a handle that only remembers the archive entry
// name ("BIN_PAC_NORMAL_MDL", "BIN_PAC_N_MOTION_01_CHA", ...). It is an AnimationClip so dynamic_cast<AnimationClip*>(handle) keeps working.
namespace kingdom
{
class ResHandle : public AnimationClip
{
public:
	// the archive entry (an attached archive's name, with ".brres" appended when only that exists); empty for a null handle
	std::string entry;
};

// nullptr when no attached archive has `key` (or `key.brres`)
ResHandle* MakeResHandle(const std::string& key);
}
