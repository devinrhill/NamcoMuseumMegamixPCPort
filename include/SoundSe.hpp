#pragma once

#include "common.hpp"
#include "SoundSeObject.hpp"

// Unity SoundSe (AudioMixer based): scrapped. Only the enums the game code passes to PlaySE are kept (the arguments are ignored).
class SoundSe
{
public:
	enum EMixerType { eNORMAL = 0, eSYSTEM = 1 };
	enum ESameKindCheck { eNONE = 0, eEARLY = 1, eLATE = 2, eLATE_IMMEDIATE = 3 };
};
