#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"
// Unity-only overlay UI (TextMeshPro/Animator) of the C# port. Scrapped: the Wii game draws its menu text with Anita layouts + the font
// writer. The menu code null-checks these pointers, and nothing creates one, so every method here is a no-op kept for compilation.

class MotosFrontUIManager
{
public:
	enum EUIIndex { RuleButton = 0, MotosRuleImage = 1, MotosRuleText = 2, MotosMultiRuleImage = 3, MotosMultiRuleText = 4, RollRuleImage = 5, RollRuleText = 6, Pause = 7, Retry = 8 };
	template<class T> static T* Get(int) { return nullptr; }
	template<class T> static T* Get(EUIIndex) { return nullptr; }
	static void SetEnable(int, bool) {}
	static void SetForceCameraEnable(bool) {}
};
