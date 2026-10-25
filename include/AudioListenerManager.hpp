#pragma once

#include "common.hpp"

// Unity AudioListener manager: scrapped (the Wii mixer has one fixed listener, see WiiSnd.hpp). All calls are no-ops / "audible".
class AudioListenerManager
{
public:
	static void SetEnable(bool) {}
	static void SetEnableRangeCheck(bool) {}
	static bool IsListenRange(GameObject*, float) { return true; }
	static void SetPosStalkTarget(GameObject*) {}
	static void SetRotStalkTarget(GameObject*) {}
	static void SetStalkReset() {}
};
