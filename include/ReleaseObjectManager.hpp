#pragma once

#include "common.hpp"

// Unity GameObject garbage collector helper: scrapped (the native build owns its objects). Regist does nothing.
class ReleaseObjectManager
{
public:
	template<class T> static void Regist(T*, float = 0.0f) {}
};
