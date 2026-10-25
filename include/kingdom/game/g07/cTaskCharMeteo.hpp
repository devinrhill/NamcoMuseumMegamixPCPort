#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g07
{

class cTaskCharMeteo : public cTaskChar
{
public:
	static constexpr int MATRIX_USE = 1;

private:
	Vector4 m_vTarget{};

	float m_fSpeed{};

	Matrix4x4 m_Ptkmtx{};

	cKingdomPtkHnd* m_phEffTail{};

public:
	cTaskCharMeteo();

	Vector4& vTarget();

	float& fSpeed();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no);

	void UpdateMatrix() override;

	bool Execute(cJetTask* pParam) override;

	bool Move() override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool ActionLocal() override;

	bool ActionOther() override;
};

}
