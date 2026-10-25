#include "kingdom/game/g03/cItem.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cTaskBgBox.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cItem::cItem()
{
	m_nKind = 0u;
	m_nDFlag = 0u;
	m_fSpeed = 0.0f;
	m_nCnt = 0u;
	m_nRevivalCnt = 0u;
	m_pPBModel = nullptr;
	m_pBox = nullptr;
	m_pGhost = nullptr;
	m_bDeadOld = false;
	::Constants::jetGeoVecUnit(m_vInitTrans);
}

uint& cItem::nKind()
{
	return m_nKind;
}

uint& cItem::nDFlag()
{
	return m_nDFlag;
}

float& cItem::fSpeed()
{
	return m_fSpeed;
}

uint& cItem::nCnt()
{
	return m_nCnt;
}

uint& cItem::nRevivalCnt()
{
	return m_nRevivalCnt;
}

bool cItem::IsDFlag(uint flag)
{
	return CustomKingdomUtil::IF(m_nDFlag & flag);
}

void cItem::OnDFlag(uint flag)
{
	m_nDFlag |= flag;
}

void cItem::OffDFlag(uint flag)
{
	m_nDFlag &= ~flag;
}

void cItem::SetDFlag(uint flag)
{
	m_nDFlag = flag;
}

cTaskModelEx::cPBModel*& cItem::pPBModel()
{
	return m_pPBModel;
}

cTaskBgBox*& cItem::pBox()
{
	return m_pBox;
}

cTaskCharGhost*& cItem::pGhost()
{
	return m_pGhost;
}

Vector4& cItem::vInitTrans()
{
	return m_vInitTrans;
}

bool& cItem::IsDeadOld()
{
	return m_bDeadOld;
}

}
