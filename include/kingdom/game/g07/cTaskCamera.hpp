#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskScnRoot.hpp"

namespace kingdom::game::g07
{

class cTaskCamera : public cTaskKingdom
{
private:
	Vector4 m_vPosition{};

	Vector4 m_vTarget{};

	Vector4 m_vPositionTmp{};

	Vector4 m_vTargetTmp{};

	Vector4 m_vSpeed{};

	float m_fFovy{};

	float m_fNear{};

	float m_fFar{};

	float m_fAspect{};

	float m_fRange{};

	int m_nTimer{};

	Camera* m_mainCam{};

	GameObject* m_gazePoint{};

public:
	cTaskCamera();

	cTaskCamera(cTaskScnRoot* pTaskScnRoot, string key);

	virtual ~cTaskCamera();

	Vector4& vPosition();

	Vector4& vPositionTmp();

	Vector4& vTarget();

	Vector4& vTargetTmp();

	float& fFovy();

	float& fNear();

	float& fFar();

	float& fAspect();

	void SaveVector();

	void LoadVector();

	void SetQuake(float range, int time);

private:
	void Init();

	void Term();

public:
	void Setup(cTaskScnRoot* pTaskScnRoot, string key);

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void UpdateMatrix();

public:
	void SetPan(Vector4 vPos, float fSpeed = 0.0f);
};

}
