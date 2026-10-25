#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"

class MotosUIRetry : public MotosUIBase
{
public:
	enum EButtonType
	{
		Yes = 0,
		No = 1
	};

	enum EAnim
	{
		Highlight = 0,
		Push = 1
	};

private:
	std::vector<Animation*> m_buttonAnim{};

	std::vector<GameObject*> m_buttonHighlightObj{};

	static std::vector<string> scAnimName;

public:
	void PlayAnim(EButtonType type, EAnim anim);

	bool IsPlayAnim(EButtonType type, EAnim anim);

	bool IsPlayAnim(EButtonType type);

	void ResetAnim(EButtonType type, bool force = false);

	void ResetAllAnim(bool force = false);

	void SetVisible(EButtonType type, bool visible);

protected:
	void OnPause() override;

	void OnResume() override;
};
