#include "cJetNode.hpp"

cJetNode::cJetNode()
{
	m_nNodeKind = 0u;
	m_pParent = nullptr;
	m_pChild = nullptr;
	m_pNextSibling = nullptr;
	m_pPrevSibling = nullptr;
}

cJetNode::~cJetNode()
{
}

cJetNode* cJetNode::pParent()
{
	return m_pParent;
}

cJetNode* cJetNode::pChild()
{
	return m_pChild;
}

cJetNode* cJetNode::pPrevSibling()
{
	return m_pPrevSibling;
}

cJetNode* cJetNode::pNextSibling()
{
	return m_pNextSibling;
}

bool cJetNode::ConnectChild(cJetNode* pNode, int nPos)
{
	if (pNode == nullptr)
	{
		return false;
	}
	pNode->m_pParent = this;
	if (pChild() == nullptr)
	{
		m_pChild = pNode;
		pNode->m_pPrevSibling = nullptr;
		pNode->m_pNextSibling = nullptr;
	}
	else if (nPos != 0 && nPos == 1)
	{
		cJetNode* cJetNode2 = m_pChild;
		m_pChild = pNode;
		pNode->m_pPrevSibling = nullptr;
		pNode->m_pNextSibling = cJetNode2;
		cJetNode2->m_pPrevSibling = pNode;
	}
	else
	{
		cJetNode* cJetNode2 = m_pChild;
		while (cJetNode2->m_pNextSibling != nullptr)
		{
			cJetNode2 = cJetNode2->m_pNextSibling;
		}
		pNode->m_pPrevSibling = cJetNode2;
		pNode->m_pNextSibling = nullptr;
		cJetNode2->m_pNextSibling = pNode;
	}
	return true;
}

void cJetNode::nNodeKind(uint nKind)
{
	m_nNodeKind = nKind;
}

uint cJetNode::nNodeKind()
{
	return m_nNodeKind;
}

void cJetNode::SetName(string pStr)
{
	m_szName = pStr;
}

string cJetNode::szName()
{
	return m_szName;
}

void cJetNode::Release()
{
	cJetNode* cJetNode2;
	do
	{
		cJetNode2 = m_pChild;
		if (cJetNode2 != nullptr)
		{
			while (cJetNode2->m_pNextSibling != nullptr)
			{
				cJetNode2 = cJetNode2->m_pNextSibling;
			}
			cJetNode2->Release();
		}
	}
	while (cJetNode2 != nullptr);
	QuitExecute();
	DisConnect();
	DeleteExecute();
}

void cJetNode::QuitExecute()
{
}

void cJetNode::DeleteExecute()
{
}

bool cJetNode::DisConnect()
{
	if (pChild() != nullptr)
	{
		return false;
	}
	if (m_pParent != nullptr && m_pParent->m_pChild == this)
	{
		m_pParent->m_pChild = m_pNextSibling;
	}
	if (m_pPrevSibling != nullptr)
	{
		m_pPrevSibling->m_pNextSibling = m_pNextSibling;
	}
	if (m_pNextSibling != nullptr)
	{
		m_pNextSibling->m_pPrevSibling = m_pPrevSibling;
	}
	return true;
}

cJetNode* cJetNode::SearchNextNode(cJetNode* pRoot)
{
	if (pChild() != nullptr)
	{
		return pChild();
	}
	if (pNextSibling() != nullptr)
	{
		return pNextSibling();
	}
	cJetNode* cJetNode2 = pParent();
	while (cJetNode2 != nullptr && cJetNode2 != pRoot)
	{
		if (cJetNode2->pNextSibling() != nullptr)
		{
			return cJetNode2->pNextSibling();
		}
		cJetNode2 = cJetNode2->pParent();
	}
	return nullptr;
}
