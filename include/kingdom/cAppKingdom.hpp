#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/cTaskPBPtkMan.hpp"
#include "kingdom/cTaskScnRoot.hpp"

namespace kingdom
{

class cAppKingdom : public cTaskKingdom
{
private:
	struct StopWatch
	{
	public:
		long nStart{};

		long nTotal{};

		bool bRunning{};
	};

	bool m_bPause{};

	cTaskScnRoot* m_pTaskScnRoot{};

	cTaskPBPtkMan* m_pTaskPtkMan{};

	cAppKingdom::StopWatch m_StopWatchs[8]{};

	bool m_bFreed{};

public:
	cAppKingdom();

	virtual void Free();

protected:
	// cJetNode::Release (DOL 0x8008ff14) ends with vtable +0x10, which destroys the task; the app's dtor runs its Term. The port
	// never deletes tasks: an app runs its Free() (= dtor + Term) here, once, when the job's term releases the main task.
	void DeleteExecute() override;

public:

	cTaskScnRoot*& pTaskScnRoot();

	cTaskPBPtkMan*& pTaskPtkMan();

	bool& IsPause();

private:
	void Init();

	void Term();

protected:
	void StopWatchReset(uint n);

	void StopWatchStart(uint n);

	void StopWatchStop(uint n);

	uint StopWatchCheck(uint n);

	bool StopWatchState(uint n);
};

}
