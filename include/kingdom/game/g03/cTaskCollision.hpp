#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "RAY3.hpp"
#include "SPHERE.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"

namespace kingdom::game::g03
{

class cTaskCollision : public cTaskKingdom
{
private:
	class sCollisionParam
	{
	public:
		cTaskChar* taskChar{};

		cFaceList* hitFace{};

		Vector4 trans{};

		Vector4 transOld{};

		Vector4 move{};

		Constants::sFace* face{};
	};

	std::vector<cTaskChar*> m_vecChar;

	std::vector<cTaskBgHit*> m_vecObj;

	std::vector<cHitChar*> pl_atk;

	std::vector<cHitChar*> pl_def;

	std::vector<cHitChar*> ene_atk;

	std::vector<cHitChar*> ene_def;

	std::vector<cHitChar*> obj_push;

	static Vector4 vTmp[3];

	static Vector4 vrt[3];

public:
	std::vector<cTaskChar*>& pVecChar();

	std::vector<cTaskBgHit*>& pVecObj();

	cTaskCollision();

	virtual ~cTaskCollision();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void CollisionChars2Chars();

	void CollisionPacman2BG();

public:
	bool HitProcessWall();

	bool HitProcessFloor();

	static bool CollisionFace(Constants::sHitParam*& hitParam, cTaskChar* chr, cTaskBgHit* obj, Constants::sFace* face, const Vector4& vTransL, const Vector4& vMoveL, const Vector4& vMoveLN, Nullable<Vector4> vOffsetL = nullptr);

	void ClearEntry();

	void PushBackChar(cTaskChar* chr);

	void PushBackCollisionObj(cTaskBgHit* obj);

	Constants::sFace* CheckDashFloor(cTaskChar* chr, cTaskBgHit*& outDashObj);

	static bool CollisionSphereAABB(cTaskChar* taskChar, cTaskBgHit* obj);

	static bool CollisionSphereAABB(const Vector3& sphereMin, const Vector3& sphereMax, cTaskBgHit* obj);

	static bool CollisionSphereFaceAABB(const Vector3& sphereMin, const Vector3& sphereMax, cTaskBgHit* obj, Constants::sFace* face);

	static SPHERE GetCharSphere(cTaskChar* const& taskChar);

	static bool CollisionRayAABB(const Vector4& pos, const Vector4& move, cTaskBgHit* obj, float maxDist = 30.0f);

	static bool CollisionRayFaceAABB(const RAY3& ray, cTaskBgHit* obj, Constants::sFace* face, float maxDist = 30.0f);

	static RAY3 GetRay(const Vector4& pos, const Vector4& move);

	static bool CollisionLinePlane(const Vector4& vPos, const Vector4& vLine, const Vector4& vrt, const Vector4& vNorm, Vector4& vOutCross);

	static bool CollisionRayPlane(const Vector4& vPos, const Vector4& vRay, const Vector4& vrt, const Vector4& vNorm, Vector4& vOutCross);

	static bool CollisionSegmentPlane(const Vector4& vPos0, const Vector4& vPos1, const Vector4& vrt, const Vector4& vNorm, Vector4& vOutCross);

	static bool CollisionInnerTriangle(const Vector4& vPos, const Vector4& vrt0, const Vector4& vrt1, const Vector4& vrt2, const Vector4& vNorm);

	static int CollisionTriangleEdge(const Vector4& vPos, const Vector4& vrt0, const Vector4& vrt1, const Vector4& vrt2, const Vector4& vNorm, const float& length, Vector4& vOutEdgePos, Vector4& vOutEdgeVec);

	static bool CollisionCircleSegment(const Vector4& segSt, const Vector4& segEn, const Vector4& cirPos, const float& cirR);

	bool CheckBottom(const Vector4& vPos, float& outDist, Vector4& outCross, Constants::sFace*& outFace, cTaskBgHit*& outObj);

	bool CheckVector(uint nKind, const Vector4& vPos, const Vector4& vVec, float& outDist, Vector4& outCross, Constants::sFace*& outFace, cTaskBgHit*& outObj);

	bool CheckVectorSimple(const uint& nKind, const Vector4& vPos, const Vector4& vVec, Vector4& outCross, Constants::sFace*& outFace, cTaskBgHit*& outObj);

	void HitListup(std::vector<cHitChar*>& pVec, const int& nSide, const uint& nFlag = 0u);

	bool HitPreCheck(cHitChar* pHit);
};

}
