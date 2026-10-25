#pragma once

#include "common.hpp"

// Unity SE object (AudioSource wrapper): scrapped. Only the enums the game code passes to PlaySE are kept (the arguments are ignored).
class SoundSeObject
{
public:
	enum class ESeType { eNORMAL = 0, eSYSTEM = 1 };
	enum class ESeGroup { eNORMAL = 0, eRESULT = 1, eENVIRONMENT = 2, eENEMY = 3, eMINIGAME = 4, eINPECTION = 5 };
};
