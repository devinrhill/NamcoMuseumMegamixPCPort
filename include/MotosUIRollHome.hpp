#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"
// Unity-only overlay UI (TextMeshPro/Animator) of the C# port. Scrapped: the Wii game draws its menu text with Anita layouts + the font
// writer. The menu code null-checks these pointers, and nothing creates one, so every method here is a no-op kept for compilation.

class MotosUIRollHome : public MotosUIBase
{
public:
	enum EButtonType { Rule = 0, Back = 1 };
	enum EAnim { Highlight = 0, Push = 1 };
	enum EBaseAnim { In = 0 };
	void PlayAnim(EButtonType, EAnim) {}
	void PlayBaseAnim(EBaseAnim) {}
	bool IsPlayAnim(EButtonType, EAnim) { return false; }
	bool IsPlayBaseAnim(EBaseAnim) { return false; }
	void ResetAnim(EButtonType, bool = false) {}
	void ResetBaseAnim(bool = false) {}
};
