#pragma once

#include <cstdint>
#include <iterator>
#include "common.hpp"
#include "ConstantsResource.hpp"
#include "CustomKingdomUtil.hpp"
#include "JBC_Handler.hpp"

namespace Constants
{
	inline float fabs(float v) { return v < 0.0f ? -v : v; }

	const int SAVE_CLEAR_BYTEMAX = 8;

	const int SAVE_DIGDUG_RARE_BYTEMAX = 6;

	const int SAVE_DIGDUG_ITEM_MAX = 4;

	const int SAVE_PADDING = 63;

	const int FRAME_RATE = 60;

	constexpr float PI = 3.14159265358979323846f;

	const float JET_PI = PI;

	const float JET_PI2 = PI * 2.f;

	static Matrix4x4 scMtxIdentity = Matrix4x4::identity();

	const int SYS_TASK_SORTBUFF_MAX = 512;

	const int SYS_MODULE_MAX = 16;

	const uint JBC_NO_PROC = 0xffffffffUL;

	int SET_RGBA8888(int r, int g, int b, int a);

	void SET_FLAG_BIT(uint32_t& flag, uint32_t bit, uint32_t n);

	void SET_POW_FLAG(uint32_t& flag, uint32_t n);

	float DIST(float a, float b);

	uint32_t REFLECT_ALPHA_DECREASE(uint32_t col);

	float PERCENT(float a, float b);

	float SEC2FRAME(float a);

	float RATIO(float a, float b);

	float RATIO_SP(float a, float b);

	uint32_t PRJ_JOBTBL_GETNO(uint32_t a);

	float JET_DEG2RAD(float deg);

	float JET_RAD2DEG(float rad);

	float JET_ABS(float x);

	float JET_MAX(float v1, float v2);

	float JET_MIN(float v1, float v2);

	template<typename T, std::size_t N> static int ARRAY_SIZE(const T (&a)[N])
	{
		return static_cast<int>(N);
	}

	uint16_t SWAP_ENDIAN16(uint16_t val);

	uint32_t SWAP_ENDIAN32(uint32_t val);

	uint64_t SWAP_ENDIAN64(uint64_t val);

	uint32_t JET_RGBA8888(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

	float& JET_MTX_RC(Matrix4x4& m, int r, int c);

	void jetGeoMtxRotXYZ(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0);

	void jetGeoMtxRotZYX(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0);

	void jetGeoMtxRotAxis(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0, float r);

	void jetGeoMtxPers(Matrix4x4& m0, const Matrix4x4& m1, float fovy, float aspect, float r_near, float r_far);

	void jetGeoMtxFrustum(Matrix4x4& m0, const Matrix4x4& m1, float left, float right, float bottom, float top, float r_near, float r_far);

	void jetGeoMtxOrtho(Matrix4x4& m0, const Matrix4x4& m1, float left, float right, float bottom, float top, float r_near, float r_far);

	void jetGeoMtxLookAt(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& eye, const Vector4& center, const Vector4& up);

	void jetGeoVecSet(Vector4& v0, float x, float y, float z, float w);
	
	void jetGeoVecZero(Vector4& v0);

	void jetGeoVecUnit(Vector4& v0);

	void jetGeoVecCopy(Vector4& v0, const Vector4& v1);

	void jetGeoVecAdd(Vector4& v0, const Vector4& v1, const Vector4& v2);
	
	void jetGeoVecSub(Vector4& v0, const Vector4& v1, const Vector4& v2);
	
	void jetGeoVecMult(Vector4& v0, const Vector4& v1, const Vector4& v2);
	
	void jetGeoVecDiv(Vector4& v0, const Vector4& v1, const Vector4& v2);
	
	void jetGeoVecLerp(Vector4& v0, const Vector4& v1, const Vector4& v2, float t);

	void jetGeoVecScale(Vector4& v0, const Vector4& v1, float t);

	// dot product
	float jetGeoVecInnerProduct(const Vector4& v0, const Vector4& v1);

	// cross product
	void jetGeoVecOuterProduct(Vector4& v0, const Vector4& v1, const Vector4& v2);

	void jetGeoVecNormalize(Vector4& v0, const Vector4& v1);

	float jetGeoVecDistance(const Vector4& v0, const Vector4& v1);

	float jetGeoVecLength(const Vector4& v0);

	void jetGeoVecTrans(Vector4& v0, const Matrix4x4& m0, const Vector4& v1);

	void jetGeoVecRotX(Vector4& v0, const Vector4& v1, float r);

	void jetGeoVecRotY(Vector4& v0, const Vector4& v1, float r);

	void jetGeoVecRotZ(Vector4& v0, const Vector4& v1, float r);

	void jetGeoVecRotXYZ(Vector4& v0, const Vector4& v1, const Vector4& v2);

	void jetGeoVecRotZYX(Vector4& v0, const Vector4& v1, const Vector4& v2);

	void jetGeoVecRotAxis(Vector4& v0, const Vector4& v1, const Vector4& v2, float r);

	void jetGeoVecRotTransPers(Vector4& v0, const Matrix4x4& m0, const Vector4& v1, const Vector4& v2);

	void jetGeoMtx44to34(Matrix4x4& m34, Matrix4x4 m44);

	void jetGeoMtx34to44(Matrix4x4& m44, Matrix4x4 m34);

	void jetGeoMtxRotXDeg(Matrix4x4& m0, const Matrix4x4& m1, float d);

	void jetGeoMtxRotYDeg(Matrix4x4& m0, const Matrix4x4& m1, float d);

	void jetGeoMtxRotZDeg(Matrix4x4& m0, const Matrix4x4& m1, float d);

	void jetGeoMtxRotXYZDeg(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0);

	void jetGeoMtxRotZYXDeg(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0);

	void jetGeoMtxRotAxisDeg(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0, float d);

	void jetGeoVecOne(Vector4& v0);

	void jetGeoVecRotXDeg(Vector4& v0, const Vector4& v1, float d);

	void jetGeoVecRotYDeg(Vector4& v0, const Vector4& v1, float d);

	void jetGeoVecRotZDeg(Vector4& v0, const Vector4& v1, float d);

	void jetGeoVecRotXYZDeg(Vector4& v0, const Vector4& v1, const Vector4& v2);

	void jetGeoVecRotZYXDeg(Vector4& v0, const Vector4& v1, const Vector4& v2);

	void jetGeoVecRotAxisDeg(Vector4& v0, const Vector4& v1, const Vector4& v2, float d);
	
	void jetGeoMtxSetTrans(Matrix4x4& m0, const Vector4& v0);

	void jetGeoMtxGetTrans(Vector4& v0, Matrix4x4 m0);
	
	void MTX44Identity(Matrix4x4& m0);

	void MTX44Copy(const Matrix4x4& m1, Matrix4x4& m0);

	void MTX44Concat(const Matrix4x4& m1, const Matrix4x4& m2, Matrix4x4& m0);
	
	void MTX44Transpose(const Matrix4x4& m1, Matrix4x4& m0);

	void MTX44Inverse(const Matrix4x4& m1, Matrix4x4& m0);
	
	void MTX44Trans(Matrix4x4& m0, const float& x, const float& y, const float& z);

	void MTX44Scale(Matrix4x4& m0, const float& x, const float& y, const float& z);

	void MTX44RotRad(Matrix4x4& m0, const char& axisType, const float& r);

	void MTX44RotAxisRad(Matrix4x4& m0, const Vector4& v0, const float& r);

	void MTXPerspective(Matrix4x4& m0, const float& fovy, const float& aspect, const float& r_near, const float& r_far);

	void MTXFrustum(Matrix4x4& m0, const float& top, const float& bottom, const float& left, const float& right, const float& r_near, const float& r_far);

	void MTXOrtho(Matrix4x4& m0, const float& top, const float& bottom, const float& left, const float& right, const float& r_near, const float& r_far);

	void MTXLookAt(Matrix4x4& m0, const Vector4& eye, const Vector4& up, const Vector4& center);

	void jetGeoMtxIdentity(Matrix4x4& m0);

	void jetGeoMtxCopy(Matrix4x4& m0, const Matrix4x4& m1);

	void jetGeoMtxMult(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2);

	void jetGeoMtxMultTrans(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2);

	void jetGeoMtxMultRot(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2);

	void jetGeoMtxMultScale(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2);

	void jetGeoMtxTranspose(Matrix4x4& m0, const Matrix4x4& m1);

	void jetGeoMtxInverse(Matrix4x4& m0, const Matrix4x4& m1);

	void jetGeoMtxTrans(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0);

	void jetGeoMtxScale(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0);

	void jetGeoMtxRotX(Matrix4x4& m0, const Matrix4x4& m1, float r);

	void jetGeoMtxRotY(Matrix4x4& m0, const Matrix4x4& m1, float r);

	void jetGeoMtxRotZ(Matrix4x4& m0, const Matrix4x4& m1, float r);

	void VECAdd(const Vector4& v1, const Vector4& v2, Vector4& v0);

	void VECSubtract(const Vector4& v1, const Vector4& v2, Vector4& v0);

	void VECScale(const Vector4& v1, Vector4& v0, const float& t);

	float VECDotProduct(const Vector4& v0, const Vector4& v1);

	void VECCrossProduct(const Vector4& v1, const Vector4& v2, Vector4& v0);

	void VECNormalize(const Vector4& v1, Vector4& v0);

	float VECDistance(const Vector4& v0, const Vector4& v1);

	float bizGeoVecRadY(const Vector4& pOrg, const Vector4& pDist);

	float bizGeoVecRadGap(float fSrc, float fDist);

	float bizGeoVecDistance2(const Vector4& v0, const Vector4& v1);

	float bizGeoVecDistance2XZ(const Vector4& v0, const Vector4& v1);

	float bizGeoVecDistanceXZ(const Vector4& v0, const Vector4& v1);

	float bizGeoVecLength2(const Vector4& vec);

	bool bizMathNearValue(float& pOut, const float& fValue, const float& fTarget, const float& fAdd);

	bool bizMathNearTrans(Vector4& vOut, const Vector4& vNow, const Vector4& vTarget, const float& fAdd);

	bool bizMathNearTransXZ(Vector4& vOut, const Vector4& vNow, const Vector4& vTarget, const float& fAdd);

	int bizMathNearDir(const float& fAng1, const float& fAng2);

	float bizMathGetRad(const float& fAng1, const float& fAng2);

	bool bizMathNearRad(float& pOut, const float& fNow, const float& fTarget, const float& fAdd);

	float bizMathLerpFloat(const float& fVal0, const float& fVal1, const float& fRate);

	void Jbc_SetConfig(JBC_Handler& hnd, uint32_t msk, uint32_t sft, uint32_t ntbl, JBC_Element* tbl);

	void Jbc_SetProcNo(JBC_Handler& hnd, uint32_t no);

	void Jbc_SetForceProcNo(JBC_Handler& hnd, uint32_t no);

	uint32_t Jbc_GetProcNo(JBC_Handler& hnd);

	uint32_t Jbc_GetProcNoOld(JBC_Handler& hnd);

	uint32_t GetTableNum(JBC_Handler& hnd, uint32_t num);

	uint32_t GetAboveNum(JBC_Handler& hnd, uint32_t num);

	bool IsPrgChg(JBC_Handler& hnd, uint32_t num0, uint32_t num1);

	bool IsAboveChg(JBC_Handler& hnd, uint32_t num0, uint32_t num1);

	void Jbc_Init(JBC_Handler& hnd);

	bool Jbc_ExecProc(JBC_Handler& hnd);
}
