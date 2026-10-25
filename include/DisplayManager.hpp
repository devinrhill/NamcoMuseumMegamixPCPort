#pragma once

#include <string>

#include "ELocalizeDefine.hpp"

// Native stand-in for the Unity DisplayManager (localised UI messages). The English text of the Pac-Motos tutorial pages comes from a
// message CSV of the DOL: not located yet (UNVERIFIED), so the key is returned as a visible placeholder.
class DisplayManager
{
public:
	static std::string GetMessage(ELocalizeDefine key) { return "[pacmotos " + std::to_string((int)key - 1) + "]"; }
};
