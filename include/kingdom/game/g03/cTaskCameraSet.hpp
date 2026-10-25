#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g03/sCameraSetData.hpp"
#include "kingdom/game/g03/sSaveDataCameraArea.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskCameraSet : public cTaskKingdom
{
private:
	bool m_bValid{};

	sSaveDataCameraArea m_pLoadData{};

	uint m_nDataNum{};

	std::vector<sCameraSetData> m_cameraSetData;

	int m_nSelectArea{};

	int m_nSelectIndex{};

public:
	cTaskCameraSet();

	virtual ~cTaskCameraSet();

	std::vector<sCameraSetData>& GetCameraSetData();

private:
	void Init();

	void Term();

public:
	bool LoadData(uint world, uint stage);

	bool Execute(cJetTask* pParam) override;

	void Draw(int nState, cJetTask* pParam) override;

	bool AreaCheck(sCameraSetData pData);
};

}
