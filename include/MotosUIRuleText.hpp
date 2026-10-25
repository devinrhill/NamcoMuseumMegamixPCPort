#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"
// Unity-only overlay UI (TextMeshPro/Animator) of the C# port. Scrapped: the Wii game draws its menu text with Anita layouts + the font
// writer. The menu code null-checks these pointers, and nothing creates one, so every method here is a no-op kept for compilation.

class MotosUIRuleText : public MotosUIBase
{
public:
	enum EPageType { Page1 = 0, Page2 = 1, Page3 = 2, Page4 = 3, Page5 = 4, Max = 5 };
	enum EAnim { In = 0, Out = 1 };
	void Init(const string*) {}
	void PlayAnim(EPageType, EAnim) {}
	void PlayAnim(uint, EAnim) {}
	bool IsPlayAnim(EPageType, EAnim) { return false; }
	bool IsPlayAnim(uint, EAnim) { return false; }
	void ResetAnim(EPageType, bool = false) {}
	void ResetAnim(uint, bool = false) {}
	void SetVisible(EPageType, bool) {}
	void SetVisible(uint, bool) {}
	void ResetVisible() {}
};
