#pragma once

#include "common.hpp"
#include "AABB.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "ResMdl.hpp"

namespace kingdom::game::g03
{

class cFaceList
{
private:
	std::vector<Constants::sFace*> m_vecFace[4];

	bool m_bAABB{};

	AABB m_AABB{};

	ResMdl* m_ResMdl{};

	bool m_isReuse{};

	AABB m_calcedAABB{};

public:
	bool IsAABB();

	AABB pAABB();

	AABB& ReadOnlyAABB();

	ResMdl* pResMdl();

	uint nFaceNum(int n);

	Constants::sFace* pFace(int n, int i);

	std::vector<Constants::sFace*>* pAllFace();

	bool& IsReuse();

	AABB& CalcedAABB();

	cFaceList(ResMdl* data);

	virtual ~cFaceList();

private:
	void Init();

	void Term();

	void RegistFace(ResMdl* resMdl);

public:
	void SearchHangEdge();

private:
	bool CliffCheck(const uint& noEdgeIdx, Constants::sFace* const& faceF, Constants::sFace* const& faceW);

	Constants::eFACE_KIND GetFaceKind(string matName);

	Constants::eFACE_KIND_EX GetFaceKindEx(string matName);
};

}
