#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"

class MotosUIPause : public MotosUIBase
{
public:
	enum EButtonType
	{
		Continue = 0,
		ReturnMenu = 1
	};

	enum EAnim
	{
		Highlight = 0,
		Push = 1,
		Out = 2
	};

private:
	std::vector<Animation*> m_buttonAnim{};

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
