#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"
// Unity-only overlay UI (TextMeshPro/Animator) of the C# port. Scrapped: the Wii game draws its menu text with Anita layouts + the font
// writer. The menu code null-checks these pointers, and nothing creates one, so every method here is a no-op kept for compilation.

class MotosUIRuleButtonSet : public MotosUIBase
{
public:
	enum EButtonType { NextPage = 0, PrevPage = 1, Close = 2 };
	enum EAnim { Highlight = 0, Push = 1, Out = 2, In = 3 };
	void PlayAnim(EButtonType, EAnim) {}
	bool IsPlayAnim(EButtonType, EAnim) { return false; }
	bool IsPlayAnim(EButtonType) { return false; }
	void ResetAnim(EButtonType, bool = false) {}
	void SetVisible(EButtonType, bool) {}
	bool IsVisible(EButtonType) { return false; }
};
