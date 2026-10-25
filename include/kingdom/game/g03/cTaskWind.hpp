#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g03/sSaveDataWind.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g03
{

class cTaskWind : public cTaskKingdom
{
private:
	float m_fForce{};

	Vector4 m_vTrans{};

	Vector4 m_vRotate{};

	Vector4 m_vScale{};

	Vector4 m_vWindForce{};

	Vector4 m_vRotWind{};

	Vector4 m_vWindVecRev{};

	cKingdomPtkHnd* m_hPtkWind{};

	GameObject* m_hSndWind{};

public:
	Vector4& vTrans();

	float vTrans(int n);

	Vector4& vRotate();

	float vRotate(int n);

	Vector4& vScale();

	float vScale(int n);

	Vector4& vWindForce();

	float vWindForce(int n);

	cTaskWind(sSaveDataWind& wind);

	virtual ~cTaskWind();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool AreaCheck();
};

}
