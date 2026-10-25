#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"
// Unity-only overlay UI (TextMeshPro/Animator) of the C# port. Scrapped: the Wii game draws its menu text with Anita layouts + the font
// writer. The menu code null-checks these pointers, and nothing creates one, so every method here is a no-op kept for compilation.

class MotosUIManager
{
public:
	enum EUIIndex { MotosHome = 0, MotosMultiHome = 1, MotosStartText = 2, MotosBossText = 3, RollHome = 4, RollStageName = 5, RollCaution = 6 };
	template<class T> static T* Get(int) { return nullptr; }
	template<class T> static T* Get(EUIIndex) { return nullptr; }
	static void SetEnable(int, bool) {}
};
