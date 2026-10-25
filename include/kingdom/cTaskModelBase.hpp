#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskPushBack.hpp"

namespace kingdom
{

class cTaskModelBase : public cTaskKingdom
{
private:
	class sGarbage
	{
	public:
		cTaskPushBack* pb{};

		bool isGarbage{};
	};

	std::vector<cTaskModelBase::sGarbage*> m_vecGarbage;

	void Init();

public:
	virtual void Term();

protected:
	void GarbageModel(cTaskPushBack* pb);

	void ClearGarbageModel();

	void EnableGarbageModel();

	template<class T>
	bool Execute(std::vector<T*>& vec)
	{
		for (T* item : vec)
		{
			if (item != nullptr)
			{
				item->ClearGarbageModel();
			}
		}
		ClearGarbageModel();
		return true;
	}

	template<class T>
	void Draw(std::vector<T*>& vec)
	{
		for (T* item : vec)
		{
			if (item != nullptr)
			{
				item->EnableGarbageModel();
				if (item->IsVisible())
				{
					item->UpdateMatrix();
				}
			}
		}
		EnableGarbageModel();
	}

	template<class T>
	void UpdateFrame(std::vector<T*>& vec)
	{
		for (T* item : vec)
		{
			if (item != nullptr)
			{
				item->UpdateFrame();
			}
		}
	}

public:
	cTaskModelBase();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
