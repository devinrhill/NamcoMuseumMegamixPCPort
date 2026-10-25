#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskModelEx.hpp"
#include "kingdom/game/g03/cTaskBgBox.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"

namespace kingdom::game::g03
{

class cItem
{
private:
	uint m_nKind{};

	uint m_nDFlag{};

	float m_fSpeed{};

	uint m_nCnt{};

	uint m_nRevivalCnt{};

	cTaskModelEx::cPBModel* m_pPBModel{};

	cTaskBgBox* m_pBox{};

	cTaskCharGhost* m_pGhost{};

	bool m_bDeadOld{};

	Vector4 m_vInitTrans{};

public:
	cItem();

	uint& nKind();

	uint& nDFlag();

	float& fSpeed();

	uint& nCnt();

	uint& nRevivalCnt();

	bool IsDFlag(uint flag);

	void OnDFlag(uint flag);

	void OffDFlag(uint flag);

	void SetDFlag(uint flag);

	cTaskModelEx::cPBModel*& pPBModel();

	cTaskBgBox*& pBox();

	cTaskCharGhost*& pGhost();

	Vector4& vInitTrans();

	bool& IsDeadOld();
};

}
