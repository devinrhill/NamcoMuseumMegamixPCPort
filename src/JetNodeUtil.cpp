#include "JetNodeUtil.hpp"

#include "cJetNode.hpp"

int JetNodeUtil::NodeUtilCountUpNode(cJetNode* pTree)
{
	int num = 0;
	for (cJetNode* cJetNode2 = pTree; cJetNode2 != nullptr; cJetNode2 = cJetNode2->SearchNextNode(pTree))
	{
		num++;
	}
	return num;
}

int JetNodeUtil::NodeUtilListUpNode(cJetNode** pOutArray, cJetNode* pTree, int nMax, NODE_LISTUP_FUNC_PTR pFunc)
{
	int num = 0;
	cJetNode* cJetNode2 = pTree;
	while (cJetNode2 != nullptr && (nMax == -1 || num < nMax))
	{
		if (pFunc == nullptr || pFunc(cJetNode2))
		{
			pOutArray[num] = cJetNode2;
			num++;
		}
		cJetNode2 = cJetNode2->SearchNextNode(pTree);
	}
	return num;
}

int JetNodeUtil::NodeUtilListUpNodeRev(cJetNode** pOutArray, cJetNode* pTree, int nMax, NODE_LISTUP_FUNC_PTR pFunc)
{
	int num = NodeUtilListUpNode(pOutArray, pTree, nMax, pFunc);
	if (num >= 2)
	{
		for (int i = 0; i < num / 2; i++)
		{
			cJetNode* cJetNode2 = pOutArray[num - i - 1];
			pOutArray[num - i - 1] = pOutArray[i];
			pOutArray[i] = cJetNode2;
		}
	}
	return num;
}

int JetNodeUtil::NodeUtilListUpNodeSort(cJetNode** pOutArray, cJetNode* pTree, int nMax, NODE_COMPARE_FUNC_PTR pCFunc, NODE_LISTUP_FUNC_PTR pFunc)
{
	if (pCFunc != nullptr)
	{
		int num = NodeUtilListUpNode(pOutArray, pTree, nMax, pFunc);
		int num2 = num;
		int num3 = 1;
		while (num2 > 1 || 0 < num3)
		{
			num2 = num2 * 10 / 13;
			if (num2 < 1)
			{
				num2 = 1;
			}
			else if (num2 == 9 || num2 == 10)
			{
				num2 = 11;
			}
			num3 = 0;
			for (int i = 0; i < num - num2; i++)
			{
				if (pCFunc(pOutArray[i], pOutArray[i + num2]))
				{
					cJetNode* cJetNode2 = pOutArray[i + num2];
					pOutArray[i + num2] = pOutArray[i];
					pOutArray[i] = cJetNode2;
					num3 = 1;
				}
			}
		}
		return num;
	}
	return 0;
}

cJetNode* JetNodeUtil::NodeUtilSearchKind(int nKind, cJetNode* pNode, cJetNode* pRoot)
{
	while (pNode != nullptr && pNode->nNodeKind() != (uint)nKind)
	{
		pNode = pNode->SearchNextNode(pRoot);
	}
	return pNode;
}

void JetNodeUtil::NodeUtilViewTree(cJetNode* pRoot)
{
	int num = 0;
	cJetNode* cJetNode2 = pRoot;
	// TODO(unity): Debug.LogFormat("Node Tree---------------------------");
	while (cJetNode2 != nullptr)
	{
		if (0 < num)
		{
			for (int i = 0; i < num; i++)
			{
				// TODO(unity): Debug.LogFormat("  ");
			}
			// TODO(unity): Debug.LogFormat("+");
		}
		// TODO(unity): Debug.LogFormat(" %s", cJetNode2.szName());
		if (cJetNode2->pChild() != nullptr)
		{
			cJetNode2 = cJetNode2->pChild();
			num++;
		}
		else if (num > 0 && cJetNode2->pNextSibling() != nullptr)
		{
			cJetNode2 = cJetNode2->pNextSibling();
		}
		else if (num > 0)
		{
			do
			{
				cJetNode2 = cJetNode2->pParent();
				num--;
				if (num <= 0)
				{
					cJetNode2 = nullptr;
					break;
				}
				if (cJetNode2 != nullptr && cJetNode2->pNextSibling() != nullptr)
				{
					cJetNode2 = cJetNode2->pNextSibling();
					break;
				}
			}
			while (cJetNode2 != nullptr);
		}
		else
		{
			cJetNode2 = nullptr;
		}
	}
	// TODO(unity): Debug.LogFormat("------------------------------------");
}
