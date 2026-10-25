#include "kingdom/game/menu/cTaskConsError.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/menu/cParamMessage.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"

namespace kingdom::game::menu
{

cParamMessage*& cTaskConsError::pParamMessage()
{
	return m_pParamMessage;
}

uint& cTaskConsError::nErrorCode()
{
	return m_nErrorCode;
}

Vector4& cTaskConsError::vTrans()
{
	return m_vTrans;
}

cTaskConsError::~cTaskConsError()
{
}

void cTaskConsError::nSetErrorCode(uint nIdx)
{
	nErrorCode() = nIdx;
}

uint cTaskConsError::nGetErrorCode()
{
	return nErrorCode();
}

cTaskConsError::cTaskConsError(int nId, string Key, cParamMessage* pMessage)
	: cTaskConsWndBase(nId)
{
	nErrorCode() = 0u;
	pParamMessage() = pMessage;
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

bool cTaskConsError::AppearProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(0u));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsError::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(2u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetState(1u, false);
		return true;
	}
	return false;
}

int cTaskConsError::nGetWndAnmNo(uint nState)
{
	int obj[3] = {0, 1, 2};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	return obj[nState];
}

bool cTaskConsError::Execute(cJetTask* pParam)
{
	return cTaskConsMatching::Execute(pParam);
}

}
