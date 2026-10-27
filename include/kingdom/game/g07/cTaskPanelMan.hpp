#pragma once

#include "common.hpp"
#include "DObjParam.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/MAP_HIT_RESULT.hpp"
#include "kingdom/game/g07/MO_METEO_INFO.hpp"
#include "kingdom/game/g07/MO_SLOPE_INFO.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosMapoo.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharMeteo.hpp"
#include "kingdom/game/g07/cTaskPanel.hpp"

namespace kingdom::game::g07
{

class cTaskPanelMan : public cTaskKingdom
{
private:
	enum EPanel
	{
		PANEL_WIDTH = 16,
		PANEL_DEPTH = PANEL_WIDTH
	};

	using dir_func = std::function<int(int x, int z, int max, int w, int d)>;

	class EDGE_INFO
	{
	public:
		int nFlag{};

		Vector4 vNormal{};

		Vector4 vInV0{};

		Vector4 vInV1{};

		Vector4 vOutV0{};

		Vector4 vOutV1{};

		Vector4 vGOutV0{};

		Vector4 vGOutV1{};
	};

public:
	static constexpr int METEOR_EDGE_SHOOT = 25;

	static constexpr int PANEL_REBIRTH_TIME = 60;

private:
	cTaskPanel* m_pTaskPanels[256]{};

	cAppG07MotosGame* m_pMotosGame{};

	uint m_nWidth{};

	uint m_nDepth{};

	float m_fPanelSize{};

	uint m_nPanelPattern{};

	Constants::sMOTOS_S32_RECT m_sStageRect{};

	MAP_HIT_RESULT* m_HitResult[4]{};

	MO_METEO_INFO* m_MeteoInfo{};

	MO_SLOPE_INFO* m_SlopeInfo{};

	cTaskCharMeteo* m_pMeteoTask{};

	static constexpr int MAX_PANEL_ANIM_FRAME = 240;

	static cTaskPanelMan::EDGE_INFO* EdgeInfo[4];   // (not const: the code writes into the edges)

	static const int s_nDirs[8];

	static int level[9];

	static float dist[4];

public:
	cMotosMapoo* Mapoo{};

	cTaskPanelMan();

	cTaskPanelMan(cAppG07MotosGame* pGame);

	uint nWidth();

	uint nDepth();

	float fPanelSize();

	float nStageWidth();

	float nStageDepth();

	Constants::PANEL_INFO* SearchPanel(Vector4 vec);

	Constants::sMOTOS_S32_RECT& pStageRect();

	uint HitFlag(int x, int z);

	uint HitFlag(Vector4 pMTrans);

	MO_SLOPE_INFO*& pSlopeInfo();

	MAP_HIT_RESULT* HitResult(int n);

	int StrikePanel(int id, int level, float range, Vector4 vec, bool spawnEffect = true);

	int StrikePanel(int id, int level, int PanelID);

private:
	void Init();

public:
	void Term();

	void SlopeMan();

private:
	void SetupPanel();

	bool SetupPanelCB(DObjParam::sObj* obj, cJetTask* args);

	bool SetupPanel(Constants::ePANEL type, int kind, int x, int z, float r);

	uint SearchObjPanelPattern();

	static bool PanelPatternCB(DObjParam::sObj* obj, Ref<uint>* args);

public:
	Constants::PANEL_INFO* pPanel(uint no);

	bool Execute(cJetTask* pParam) override;

private:
	cTaskPanel* pTaskPanel(int x, int z);

	cTaskPanel* pTaskPanel(int id);

	static int FrameSet_random(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_sync(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_none(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_up(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_down(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_left(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_right(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_leftup(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_leftdown(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_rightup(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_rightdown(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_cross_v(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_cross_h(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_cross_ru(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_cross_rd(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_cross_x(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_converge(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

	static int FrameSet_diverge(int x = 0, int z = 0, int max = 0, int w = 0, int d = 0);

public:
	float PSP_PanelFrameSet(int x, int z, uint nPatern);

	void MeteoMan();

private:
	bool IsBreakPanel(Constants::PANEL_INFO* pPanel);

	void MeteoShoot();

public:
	int nMeteoShootPoint(const Vector4& target);

	int nMeteoShootPoint();

	void Map2World(Vector4 pWorld, const Vector4& pMap);

	void World2Map(Vector4 pMap, const Vector4& pWorld);

	bool IsFall(float fx, float fy, float fz);

	bool IsFall(Vector4 pPos);

	bool IsMove(Vector4 pPos);

	Constants::PANEL_INFO* SearchPanel(float mx, float mz);

	Constants::PANEL_INFO* SearchPanelNo(int x, int z);

	int HitCheck(Vector4 pPos, Vector4 pVec, float fRange);

	bool GetResHitCheck(Constants::PANEL_INFO* pPanel, MAP_HIT_RESULT* pOut, Vector4 pPos, Vector4 pVec, float fRange);

	void SetPlayer();

	void SetEnemy();

	void SetItem();

	int AvoidPingBall(cTaskChar* pChar0, cTaskChar* pChar1, int axis);

	int HitCheckSubRect(Vector4 pP0, Vector4 pP1, float fRange, Vector4 pPoint);

	bool HitCheckSubCrsLine(Vector4& pOut, const Vector4& pP0, const Vector4& pP1, const Vector4& pV0, const Vector4& pV1);

	int HitCheckSub(Constants::PANEL_INFO* pPanel, Vector4 pos, Vector4 vec, float fRange, Vector4& normal, Vector4& pCrs);

	int StrikePanel(int id, int level, float range, float mx, float mz, bool spawnEffect = true);

	int StrikePanel(int id, int level, Constants::PANEL_INFO* pPanel, bool spawnEffect = true);

	Constants::PANEL_INFO* SearchPanelID(int PanelID);

	bool MeteoSpawn(Vector4 pTarget, float speed, Vector4 pShootPos, float yuragi, int seid);

	int AdjustPanel(Vector4& pOut, Vector4 pPos, float fRange, int _base = 0);

	int ForceAdjustPanel(Vector4& pOut, Vector4 pPos, float fRange, int input_dir = 0);

protected:
	void ReleaseTerm() override { cTaskPanelMan::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
