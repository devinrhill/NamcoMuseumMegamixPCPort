#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/cTaskBGEffect.hpp"

namespace kingdom::game::g07
{

class cTaskBG : public cTaskModel
{
public:
	static constexpr uint BG_MAX = 4u;

private:
	static constexpr int PTK_BG = 3;

	static constexpr int ePtkMax = 32;

	uint m_nWorldIdx{};

	cKingdomPtkHnd* m_hPtk[32]{};

	int m_nUseNum{};

	int m_nStagePtk{};

	Vector4 m_vNodeTrans{};

	cTaskBGEffect* m_pTaskBGEffect{};

public:
	cTaskBG();

	uint& nStageIdx();

private:
	void Init();

public:
	void Term() override;

	virtual void PtkInit();

	virtual int PtkSet(const string& name, uint nKind, uint ModelNo = 3u);

	virtual Nullable<Vector4> pModelNodeTrans(const string& name, uint ModelNo = 3u);

	virtual void PtkExecute();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
