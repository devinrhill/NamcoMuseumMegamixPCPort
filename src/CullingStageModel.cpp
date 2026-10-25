#include "CullingStageModel.hpp"

void CullingStageModel::SetCulling(bool on, bool changeActive)
{
	if (m_meshList.size() <= 0)
	{
		// TODO(unity): Debug.LogWarning("[PacnRoll] メッシュリストがセットされていません");
		return;
	}
	int count = (int)m_targetMeshIdxList.size();
	if (count <= 0)
	{
		// TODO(unity): Debug.LogWarning("[PacnRoll] 対象Indexリストがセットされていません");
		return;
	}
	for (int i = 0; i < count; i++)
	{
		int index = m_targetMeshIdxList[i];
		if (changeActive)
		{
			// TODO(unity): m_meshList[index].gameObject.SetActive(!on);
		}
		else
		{
			// TODO(unity): m_meshList[index].enabled = !on;
		}
	}
}
