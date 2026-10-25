#pragma once

#include "common.hpp"
#include "kingdom/cKingdomAnita.hpp"

namespace kingdom
{

class cKingdomFade
{
private:
	class cFadeAnita : public cKingdomAnita
	{
	public:
		bool SetTexture(int idx, Texture* tex) override;
	};

	Camera* m_targetCamera{};

	cKingdomFade::cFadeAnita* m_Anita{};

	uint m_nType{};

	float m_fSpeed{};

	bool m_bEnd{};

	void Init();

	void Term();

	int Type2AnimNo(uint type);

public:
	cKingdomFade();

	virtual ~cKingdomFade();

	bool Fade(uint type, int msec = 500);

	bool IsTypeOfFadeIn();

	bool IsTypeOfFadeOut();

	bool IsEnd();

	void SetTexture(Texture* tex);

	void UpdateFrame();

	void Draw();
};

}
