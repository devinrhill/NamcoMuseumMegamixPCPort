#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsWndBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/menu/cParamMessage.hpp"

namespace kingdom::game::menu
{

class cTaskConsError : public cTaskConsWndBase
{
private:
	static constexpr uint FONT_BUF_SIZE = 1024u;

	static constexpr uint FONT_SIZE = 25u;

	static constexpr uint SET_PAT = 0u;

	static constexpr int SET_LAY = 0;

	cParamMessage* m_pParamMessage{};

	uint m_nErrorCode{};

	Vector4 m_vTrans{};

protected:
	cParamMessage*& pParamMessage();

	uint& nErrorCode();

	Vector4& vTrans();

public:
	virtual ~cTaskConsError();

	void nSetErrorCode(uint nIdx);

	uint nGetErrorCode();

	cTaskConsError(int nId, string Key, cParamMessage* pMessage);

protected:
	bool AppearProc() override;

	bool DeleteProc() override;

	int nGetWndAnmNo(uint nState) override;

public:
	bool Execute(cJetTask* pParam) override;
};

}
