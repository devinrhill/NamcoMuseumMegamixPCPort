#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"

class MotosUIRollStageName : public MotosUIBase
{
public:
	enum EBaseAnim
	{
		In = 0,
		Out = 1
	};

private:
	Animation* m_baseAnim{};

	Text* m_text{};

	static std::vector<string> scBaseAnimName;

public:
	void Init(int worldNo);

	void PlayBaseAnim(EBaseAnim anim);

	bool IsPlayBaseAnim(EBaseAnim anim);

	bool IsPlayBaseAnim();

	void ResetBaseAnim(bool force = false);

protected:
	void OnPause() override;

	void OnResume() override;
};
