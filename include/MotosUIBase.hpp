#pragma once

#include "common.hpp"
// Unity-only overlay UI (TextMeshPro/Animator) of the C# port. Scrapped: the Wii game draws its menu text with Anita layouts + the font
// writer. The menu code null-checks these pointers, and nothing creates one, so every method here is a no-op kept for compilation.

class MotosUIBase
{
public:
	virtual ~MotosUIBase() = default;
	void SetEnable(bool) {}
	bool IsEnable() { return false; }

protected:
	friend class MotosUIManager;
	virtual void OnPause() {}
	virtual void OnResume() {}
};
