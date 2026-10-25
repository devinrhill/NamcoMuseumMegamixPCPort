#pragma once

#include "common.hpp"
#include "NODE_COMPARE_FUNC_PTR.hpp"
#include "NODE_LISTUP_FUNC_PTR.hpp"
#include "cJetNode.hpp"

class JetNodeUtil
{
public:
	static int NodeUtilCountUpNode(cJetNode* pTree);

	static int NodeUtilListUpNode(cJetNode** pOutArray, cJetNode* pTree, int nMax = -1, NODE_LISTUP_FUNC_PTR pFunc = nullptr);

	static int NodeUtilListUpNodeRev(cJetNode** pOutArray, cJetNode* pTree, int nMax = -1, NODE_LISTUP_FUNC_PTR pFunc = nullptr);

	static int NodeUtilListUpNodeSort(cJetNode** pOutArray, cJetNode* pTree, int nMax = -1, NODE_COMPARE_FUNC_PTR pCFunc = nullptr, NODE_LISTUP_FUNC_PTR pFunc = nullptr);

	static cJetNode* NodeUtilSearchKind(int nKind, cJetNode* pNode, cJetNode* pRoot = nullptr);

	static void NodeUtilViewTree(cJetNode* pRoot);
};
