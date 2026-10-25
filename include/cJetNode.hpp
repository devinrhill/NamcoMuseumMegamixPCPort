#pragma once

#include "common.hpp"

class cJetNode
{
private:
	uint m_nNodeKind{};

	string m_szName{};

protected:
	cJetNode* m_pParent{};

	cJetNode* m_pChild{};

	cJetNode* m_pNextSibling{};

	cJetNode* m_pPrevSibling{};

public:
	cJetNode();

	virtual ~cJetNode();

	cJetNode* pParent();

	cJetNode* pChild();

	cJetNode* pPrevSibling();

	cJetNode* pNextSibling();

	bool ConnectChild(cJetNode* pNode, int nPos = 0);

	void nNodeKind(uint nKind);

	uint nNodeKind();

	void SetName(string pStr);

	string szName();

	void Release();

protected:
	virtual void QuitExecute();

	virtual void DeleteExecute();

	bool DisConnect();

public:
	cJetNode* SearchNextNode(cJetNode* pRoot = nullptr);
};
