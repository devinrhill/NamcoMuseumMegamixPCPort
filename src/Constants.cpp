#include "Constants.hpp"
#include "JBC_Element.hpp"

namespace Constants {
	int SET_RGBA8888(int r, int g, int b, int a)
	{
		return (a << 24) | (b << 16) | (g << 8) | r;
	}

	void SET_FLAG_BIT(uint32_t& flag, uint32_t bit, uint32_t n)
	{
		flag &= ~bit;
		flag |= ((0 < n) ? bit : 0);
	}

	void SET_POW_FLAG(uint32_t& flag, uint32_t n)
	{
		flag &= 4293922815u;
		flag |= n;
	}

	float DIST(float a, float b)
	{
		return fabs(a - b);
	}

	uint32_t REFLECT_ALPHA_DECREASE(uint32_t col)
	{
		return ((static_cast<uint32_t>(static_cast<float>(col >> 24) * 0.7f) & 0xFF) << 24) | (col & 0xFFFFFF);
	}

	float PERCENT(float a, float b)
	{
		return a * b * 0.01f;
	}

	float SEC2FRAME(float a)
	{
		return 60.0f * a;
	}

	float RATIO(float a, float b)
	{
		return a / b;
	}

	float RATIO_SP(float a, float b)
	{
		return 1.0f / 6.0f * RATIO(a, b);
	}

	uint32_t PRJ_JOBTBL_GETNO(uint32_t a)
	{
		return (a >> 20) & 0xFFF;
	}
	float JET_DEG2RAD(float deg)
	{
		return PI / 180.0f * deg;
	}

	float JET_RAD2DEG(float rad)
	{
		return 57.29578f * rad;
	}

	float JET_ABS(float x)
	{
		return std::fabs(x);
	}

	float JET_MAX(float v1, float v2)
	{
		if (!(v1 > v2))
		{
			return v2;
		}
		return v1;
	}

	float JET_MIN(float v1, float v2)
	{
		if (!(v1 < v2))
		{
			return v2;
		}
		return v1;
	}

	uint16_t SWAP_ENDIAN16(uint16_t val)
	{
		return (ushort)(((val & 0xFF) << 8) | ((val & 0xFF00) >> 8));
	}

	uint32_t SWAP_ENDIAN32(uint32_t val)
	{
		return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | ((val & 0xFF0000) >> 8) | ((val & 0xFF000000u) >> 24);
	}

	uint64_t SWAP_ENDIAN64(uint64_t val)
	{
		return static_cast<uint64_t>(((val & 0xFF) << 24) | ((val & 0xFF00) << 24) | ((val & 0xFF0000) << 8) | ((val & 0xFF000000u) >> 8) | (static_cast<int64_t>(static_cast<uint64_t>(val) & 0xFF00000000uL) >> 24) | (static_cast<int64_t>(static_cast<uint64_t>(val) & 0xFF0000000000uL) >> 56));
	}

	uint32_t JET_RGBA8888(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
	{
		return ((r & 0xFF) << 24) | ((g & 0xFF) << 16) | ((b & 0xFF) << 8) | (a & 0xFF);
	}

	float& JET_MTX_RC(Matrix4x4& m, int r, int c)
	{
		if (r == 0)
		{
			if (c == 0)
			{
				return m.m00;
			}
			if (1 == c)
			{
				return m.m01;
			}
			if (2 == c)
			{
				return m.m02;
			}
			if (3 == c)
			{
				return m.m03;
			}
		}
		else if (1 == r)
		{
			if (c == 0)
			{
				return m.m10;
			}
			if (1 == c)
			{
				return m.m11;
			}
			if (2 == c)
			{
				return m.m12;
			}
			if (3 == c)
			{
				return m.m13;
			}
		}
		else if (2 == r)
		{
			if (c == 0)
			{
				return m.m20;
			}
			if (1 == c)
			{
				return m.m21;
			}
			if (2 == c)
			{
				return m.m22;
			}
			if (3 == c)
			{
				return m.m23;
			}
		}
		else if (3 == r)
		{
			if (c == 0)
			{
				return m.m30;
			}
			if (1 == c)
			{
				return m.m31;
			}
			if (2 == c)
			{
				return m.m32;
			}
			if (3 == c)
			{
				return m.m33;
			}
		}
		return m.m00;
	}
	

	void jetGeoMtxRotXYZ(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0)
	{
		jetGeoMtxRotX(m0, m1, v0.x);
		jetGeoMtxRotY(m0, m0, v0.y);
		jetGeoMtxRotZ(m0, m0, v0.z);
	}

	void jetGeoMtxRotZYX(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0)
	{
		m0 = m1;
		if (!CustomKingdomUtil::IsEqualFloat(v0.z, 0.0f))
		{
			m0 *= rotationMatrix(Vector4(0.0f, 0.0f, 1.0f, 0.0f), v0.z);
		}
		if (!CustomKingdomUtil::IsEqualFloat(v0.y, 0.0f))
		{
			m0 *= rotationMatrix(Vector4(0.0f, 1.0f, 0.0f, 0.0f), v0.y);
		}
		if (!CustomKingdomUtil::IsEqualFloat(v0.x, 0.0f))
		{
			m0 *= rotationMatrix(Vector4(1.0f, 0.0f, 0.0f, 0.0f), v0.x);
		}
	}

	void jetGeoMtxRotAxis(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0, float r)
	{
		Matrix4x4 m2{};
		MTX44RotAxisRad(m2, v0, r);
		jetGeoMtxMultRot(m0, m1, m2);
	}

	void jetGeoMtxPers(Matrix4x4& m0, const Matrix4x4& m1, float fovy, float aspect, float r_near, float r_far)
	{
		Matrix4x4 m2{};
		MTXPerspective(m2, JET_RAD2DEG(fovy), aspect, r_near, r_far);
		jetGeoMtxMult(m0, m1, m2);
	}

	void jetGeoMtxFrustum(Matrix4x4& m0, const Matrix4x4& m1, float left, float right, float bottom, float top, float r_near, float r_far)
	{
		Matrix4x4 m2{};
		MTXFrustum(m2, top, bottom, left, right, r_near, r_far);
		jetGeoMtxMult(m0, m1, m2);
	}

	void jetGeoMtxOrtho(Matrix4x4& m0, const Matrix4x4& m1, float left, float right, float bottom, float top, float r_near, float r_far)
	{
		Matrix4x4 m2{};
		MTXOrtho(m2, top, bottom, left, right, r_near, r_far);
		jetGeoMtxMult(m0, m1, m2);
	}

	void jetGeoMtxLookAt(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& eye, const Vector4& center, const Vector4& up)
	{
		Matrix4x4 m2{};
		MTXLookAt(m2, eye, up, center);
		Matrix4x4 m3{};
		jetGeoMtx34to44(m3, m2);
		jetGeoMtxMult(m0, m1, m3);
	}

	void jetGeoVecSet(Vector4& v0, float x, float y, float z, float w)
	{
		v0.x = x;
		v0.y = y;
		v0.z = z;
		v0.w = w;
	}

	void jetGeoVecZero(Vector4& v0)
	{
		v0 = Vector4{};
		v0.x = (v0.y = (v0.z = (v0.w = 0.0f)));
	}

	void jetGeoVecUnit(Vector4& v0)
	{
		v0.x = (v0.y = (v0.z = 0.0f));
		v0.w = 1.0f;
	}

	void jetGeoVecCopy(Vector4& v0, const Vector4& v1)
	{
		v0 = v1;
	}

	void jetGeoVecAdd(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		VECAdd(v1, v2, v0);
	}

	void jetGeoVecSub(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		VECSubtract(v1, v2, v0);
	}

	void jetGeoVecMult(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		v0 = Vector4
		{
			v1.x * v2.x,
			v1.y * v2.y,
			v1.z * v2.z,
			1.0f
		};
	}

	void jetGeoVecDiv(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		v0 = Vector4{};
		v0.x = v1.x / v2.x;
		v0.y = v1.y / v2.y;
		v0.z = v1.z / v2.z;
	}

	void jetGeoVecLerp(Vector4& v0, const Vector4& v1, const Vector4& v2, float t)
	{
		v0 = Vector4{};
		v0.x = v1.x + t * (v2.x - v1.x);
		v0.y = v1.y + t * (v2.y - v1.y);
		v0.z = v1.z + t * (v2.z - v1.z);
	}

	void jetGeoVecScale(Vector4& v0, const Vector4& v1, float t)
	{
		VECScale(v1, v0, t);
	}

	float jetGeoVecInnerProduct(const Vector4& v0, const Vector4& v1)
	{
		return VECDotProduct(v0, v1);
	}

	void jetGeoVecOuterProduct(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		VECCrossProduct(v1, v2, v0);
	}

	void jetGeoVecNormalize(Vector4& v0, const Vector4& v1)
	{
		VECNormalize(v1, v0);
	}

	float jetGeoVecDistance(const Vector4& v0, const Vector4& v1)
	{
		return VECDistance(v0, v1);
	}

	float jetGeoVecLength(const Vector4& v0)
	{
		return std::sqrt(jetGeoVecInnerProduct(v0, v0));
	}

	void jetGeoVecTrans(Vector4& v0, const Matrix4x4& m0, const Vector4& v1)
	{
		v0 = transformPoint(m0, v1);
	}

	void jetGeoVecRotX(Vector4& v0, const Vector4& v1, float r)
	{
		Matrix4x4 m{};
		MTX44RotRad(m, 'X', r);
		jetGeoVecTrans(v0, m, v1);
	}

	void jetGeoVecRotY(Vector4& v0, const Vector4& v1, float r)
	{
		Matrix4x4 m{};
		MTX44RotRad(m, 'Y', r);
		jetGeoVecTrans(v0, m, v1);
	}

	void jetGeoVecRotZ(Vector4& v0, const Vector4& v1, float r)
	{
		Matrix4x4 m{};
		MTX44RotRad(m, 'Z', r);
		jetGeoVecTrans(v0, m, v1);
	}

	void jetGeoVecRotXYZ(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		jetGeoVecRotX(v0, v1, v2.x);
		jetGeoVecRotY(v0, v0, v2.y);
		jetGeoVecRotZ(v0, v0, v2.z);
	}

	void jetGeoVecRotZYX(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		jetGeoVecRotZ(v0, v1, v2.z);
		jetGeoVecRotY(v0, v0, v2.y);
		jetGeoVecRotX(v0, v0, v2.x);
	}

	void jetGeoVecRotAxis(Vector4& v0, const Vector4& v1, const Vector4& v2, float r)
	{
		Matrix4x4 m{};
		jetGeoMtxIdentity(m);
		jetGeoMtxRotAxis(m, m, v2, r);
		jetGeoVecTrans(v0, m, v1);
	}

	void jetGeoVecRotTransPers(Vector4& v0, const Matrix4x4& m0, const Vector4& v1, const Vector4& v2)
	{
		jetGeoVecTrans(v0, m0, v1);
		float num = 1.0f / v0.w;
		Vector4 v3{};
		jetGeoVecScale(v3, v0, num);
		v3.w = num;
		v3.x = v2.z / 2.0f * (v3.x + 1.0f) + v2.x;
		v3.y = v2.w / 2.0f * (1.0f - v3.y) + v2.y;
		jetGeoVecCopy(v0, v3);
	}

	void jetGeoMtx44to34(Matrix4x4& m34, Matrix4x4 m44)
	{
		m34 = m44;
	}

	void jetGeoMtx34to44(Matrix4x4& m44, Matrix4x4 m34)
	{
		m44 = Matrix4x4{};
		JET_MTX_RC(m44, 0, 0) = JET_MTX_RC(m34, 0, 0);
		JET_MTX_RC(m44, 0, 1) = JET_MTX_RC(m34, 0, 1);
		JET_MTX_RC(m44, 0, 2) = JET_MTX_RC(m34, 0, 2);
		JET_MTX_RC(m44, 0, 3) = JET_MTX_RC(m34, 0, 3);
		JET_MTX_RC(m44, 1, 0) = JET_MTX_RC(m34, 1, 0);
		JET_MTX_RC(m44, 1, 1) = JET_MTX_RC(m34, 1, 1);
		JET_MTX_RC(m44, 1, 2) = JET_MTX_RC(m34, 1, 2);
		JET_MTX_RC(m44, 1, 3) = JET_MTX_RC(m34, 1, 3);
		JET_MTX_RC(m44, 2, 0) = JET_MTX_RC(m34, 2, 0);
		JET_MTX_RC(m44, 2, 1) = JET_MTX_RC(m34, 2, 1);
		JET_MTX_RC(m44, 2, 2) = JET_MTX_RC(m34, 2, 2);
		JET_MTX_RC(m44, 2, 3) = JET_MTX_RC(m34, 2, 3);
		JET_MTX_RC(m44, 3, 0) = 0.0f;
		JET_MTX_RC(m44, 3, 1) = 0.0f;
		JET_MTX_RC(m44, 3, 2) = 0.0f;
		JET_MTX_RC(m44, 3, 3) = 1.0f;
	}

	void jetGeoMtxRotXDeg(Matrix4x4& m0, const Matrix4x4& m1, float d)
	{
		jetGeoMtxRotX(m0, m1, JET_DEG2RAD(d));
	}

	void jetGeoMtxRotYDeg(Matrix4x4& m0, const Matrix4x4& m1, float d)
	{
		jetGeoMtxRotY(m0, m1, JET_DEG2RAD(d));
	}

	void jetGeoMtxRotZDeg(Matrix4x4& m0, const Matrix4x4& m1, float d)
	{
		jetGeoMtxRotZ(m0, m1, JET_DEG2RAD(d));
	}

	void jetGeoMtxRotXYZDeg(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0)
	{
		jetGeoMtxRotX(m0, m1, JET_DEG2RAD(v0.x));
		jetGeoMtxRotY(m0, m0, JET_DEG2RAD(v0.y));
		jetGeoMtxRotZ(m0, m0, JET_DEG2RAD(v0.z));
	}

	void jetGeoMtxRotZYXDeg(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0)
	{
		jetGeoMtxRotZ(m0, m1, JET_DEG2RAD(v0.x));
		jetGeoMtxRotY(m0, m0, JET_DEG2RAD(v0.y));
		jetGeoMtxRotX(m0, m0, JET_DEG2RAD(v0.z));
	}

	void jetGeoMtxRotAxisDeg(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0, float d)
	{
		jetGeoMtxRotAxis(m0, m1, v0, JET_DEG2RAD(d));
	}

	void jetGeoVecOne(Vector4& v0)
	{
		v0 = Vector4{};
		v0.x = (v0.y = (v0.z = (v0.w = 1.0f)));
	}

	void jetGeoVecRotXDeg(Vector4& v0, const Vector4& v1, float d)
	{
		jetGeoVecRotX(v0, v1, JET_DEG2RAD(d));
	}

	void jetGeoVecRotYDeg(Vector4& v0, const Vector4& v1, float d)
	{
		jetGeoVecRotY(v0, v1, JET_DEG2RAD(d));
	}

	void jetGeoVecRotZDeg(Vector4& v0, const Vector4& v1, float d)
	{
		jetGeoVecRotZ(v0, v1, JET_DEG2RAD(d));
	}

	void jetGeoVecRotXYZDeg(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		jetGeoVecRotZDeg(v0, v1, v2.z);
		jetGeoVecRotYDeg(v0, v0, v2.y);
		jetGeoVecRotXDeg(v0, v0, v2.x);
	}

	void jetGeoVecRotZYXDeg(Vector4& v0, const Vector4& v1, const Vector4& v2)
	{
		jetGeoVecRotZDeg(v0, v1, v2.z);
		jetGeoVecRotYDeg(v0, v0, v2.y);
		jetGeoVecRotXDeg(v0, v0, v2.x);
	}

	void jetGeoVecRotAxisDeg(Vector4& v0, const Vector4& v1, const Vector4& v2, float d)
	{
		jetGeoVecRotAxisDeg(v0, v1, v2, JET_DEG2RAD(d));
	}

	void jetGeoMtxSetTrans(Matrix4x4& m0, const Vector4& v0)
	{
		m0 = Matrix4x4();
		JET_MTX_RC(m0, 0, 3) = v0.x;
		JET_MTX_RC(m0, 1, 3) = v0.y;
		JET_MTX_RC(m0, 2, 3) = v0.z;
		JET_MTX_RC(m0, 3, 3) = v0.w;
	}

	void jetGeoMtxGetTrans(Vector4& v0, Matrix4x4 m0)
	{
		jetGeoVecSet(v0, JET_MTX_RC(m0, 0, 3), JET_MTX_RC(m0, 1, 3), JET_MTX_RC(m0, 2, 3), JET_MTX_RC(m0, 3, 3));
	}

	void MTX44Identity(Matrix4x4& m0)
	{
		m0 = scMtxIdentity;
	}

	void MTX44Copy(const Matrix4x4& m1, Matrix4x4& m0)
	{
		m0 = m1;
	}

	void MTX44Concat(const Matrix4x4& m1, const Matrix4x4& m2, Matrix4x4& m0)
	{
		m0 = m1 * m2;
	}

	void MTX44Transpose(const Matrix4x4& m1, Matrix4x4& m0)
	{
		m0 = transpose(m1);
	}

	void MTX44Inverse(const Matrix4x4& m1, Matrix4x4& m0)
	{
		m0 = inverse(m1);
	}

	void MTX44Trans(Matrix4x4& m0, const float& x, const float& y, const float& z)
	{
		m0 = translateMatrix(Vector4(x, y, z, 0.0f));
	}

	void MTX44Scale(Matrix4x4& m0, const float& x, const float& y, const float& z)
	{
		m0 = scaleMatrix(Vector4(x, y, z, 0.0f));
	}

	void MTX44RotRad(Matrix4x4& m0, const char& axisType, const float& r)
	{
		Vector4 axis(0.0f, 0.0f, 0.0f, 0.0f);
		switch (axisType)
		{
		case 'X':
			axis = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			break;
		case 'Y':
			axis = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		case 'Z':
			axis = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
			break;
		}
		m0 = rotationMatrix(axis, r);
	}

	void MTX44RotAxisRad(Matrix4x4& m0, const Vector4& v0, const float& r)
	{
		m0 = rotationMatrix(v0, r);
	}

	void MTXPerspective(Matrix4x4& m0, const float& fovy, const float& aspect, const float& r_near, const float& r_far)
	{
		m0 = perspectiveMatrix(fovy, aspect, r_near, r_far);
	}

	void MTXFrustum(Matrix4x4& m0, const float& top, const float& bottom, const float& left, const float& right, const float& r_near, const float& r_far)
	{
		m0 = frustumMatrix(left, right, bottom, top, r_near, r_far);
	}

	void MTXOrtho(Matrix4x4& m0, const float& top, const float& bottom, const float& left, const float& right, const float& r_near, const float& r_far)
	{
		m0 = orthoMatrix(left, right, bottom, top, r_near, r_far);
	}

	void MTXLookAt(Matrix4x4& m0, const Vector4& eye, const Vector4& up, const Vector4& center)
	{
		m0 = lookAtMatrix(eye, center, up);
	}

	void jetGeoMtxIdentity(Matrix4x4& m0)
	{
		MTX44Identity(m0);
	}

	void jetGeoMtxCopy(Matrix4x4& m0, const Matrix4x4& m1)
	{
		MTX44Copy(m1, m0);
	}

	void jetGeoMtxMult(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2)
	{
		MTX44Concat(m1, m2, m0);
	}

	void jetGeoMtxMultTrans(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2)
	{
		MTX44Concat(m1, m2, m0);
	}

	void jetGeoMtxMultRot(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2)
	{
		MTX44Concat(m1, m2, m0);
	}

	void jetGeoMtxMultScale(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2)
	{
		MTX44Concat(m1, m2, m0);
	}

	void jetGeoMtxTranspose(Matrix4x4& m0, const Matrix4x4& m1)
	{
		MTX44Transpose(m1, m0);
	}

	void jetGeoMtxInverse(Matrix4x4& m0, const Matrix4x4& m1)
	{
		MTX44Inverse(m1, m0);
	}

	void jetGeoMtxTrans(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0)
	{
		Matrix4x4 m2{};
		MTX44Trans(m2, v0.x, v0.y, v0.z);
		jetGeoMtxMultTrans(m0, m1, m2);
	}

	void jetGeoMtxScale(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& v0)
	{
		Matrix4x4 m2{};
		MTX44Scale(m2, v0.x, v0.y, v0.z);
		jetGeoMtxMultScale(m0, m1, m2);
	}

	void jetGeoMtxRotX(Matrix4x4& m0, const Matrix4x4& m1, float r)
	{
		Matrix4x4 m2{};
		MTX44RotRad(m2, 'X', r);
		jetGeoMtxMultRot(m0, m1, m2);
	}

	void jetGeoMtxRotY(Matrix4x4& m0, const Matrix4x4& m1, float r)
	{
		Matrix4x4 m2{};
		MTX44RotRad(m2, 'Y', r);
		jetGeoMtxMultRot(m0, m1, m2);
	}

	void jetGeoMtxRotZ(Matrix4x4& m0, const Matrix4x4& m1, float r)
	{
		Matrix4x4 m2{};
		MTX44RotRad(m2, 'Z', r);
		jetGeoMtxMultRot(m0, m1, m2);
	}

	void VECAdd(const Vector4& v1, const Vector4& v2, Vector4& v0)
	{
		v0 = v1 + v2;
		v0.w = v1.w;
	}

	void VECSubtract(const Vector4& v1, const Vector4& v2, Vector4& v0)
	{
		v0 = v1 - v2;
		v0.w = v1.w;
	}

	void VECScale(const Vector4& v1, Vector4& v0, const float& t)
	{
		v0 = v1 * t;
		v0.w = v1.w;
	}

	float VECDotProduct(const Vector4& v0, const Vector4& v1)
	{
		return (v0.x * v1.x + v0.y * v1.y + v0.z * v1.z);
	}

	void VECCrossProduct(const Vector4& v1, const Vector4& v2, Vector4& v0)
	{
		v0 = Vector4(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x, 0.0f);
	}

	void VECNormalize(const Vector4& v1, Vector4& v0)
	{
		v0 = normalize3(v1);
		v0.w = v1.w;
	}

	float VECDistance(const Vector4& v0, const Vector4& v1)
	{
		return std::sqrt((v0.x-v1.x)*(v0.x-v1.x) + (v0.y-v1.y)*(v0.y-v1.y) + (v0.z-v1.z)*(v0.z-v1.z));
	}

	float bizGeoVecRadY(const Vector4& pOrg, const Vector4& pDist)
	{
		Vector4 v = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		Vector4 v2 = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		Vector4 v3{};
		jetGeoVecSub(v3, pDist, pOrg);
		float num = jetGeoVecLength(v3);
		if (num == 0.0f)
		{
			return 0.0f;
		}
		Vector4 v4{};
		jetGeoVecScale(v4, v3, 1.0f / num);
		jetGeoVecOuterProduct(v, v4, v);
		num = jetGeoVecLength(v2);
		if (num == 0.0f)
		{
			return 0.0f;
		}
		Vector4 v5{};
		jetGeoVecScale(v5, v2, 1.0f / num);
		float y = jetGeoVecInnerProduct(v5, v);
		float x = jetGeoVecInnerProduct(v5, v4);
		return std::atan2(y, x);
	}

	float bizGeoVecRadGap(float fSrc, float fDist)
	{
		while (fSrc < 0.0f)
		{
			fSrc += PI * 2.0f;
		}
		while (fSrc > PI * 2.0f)
		{
			fSrc -= PI * 2.0f;
		}
		while (fDist < 0.0f)
		{
			fDist += PI * 2.0f;
		}
		while (fDist > PI * 2.0f)
		{
			fDist -= PI * 2.0f;
		}
		float num = fDist - fSrc;
		if (num > PI)
		{
			num -= PI * 2.0f;
		}
		if (num < -PI)
		{
			num += PI * 2.0f;
		}
		return num;
	}

	float bizGeoVecDistance2(const Vector4& v0, const Vector4& v1)
	{
		Vector4 v2{};
		jetGeoVecSub(v2, v0, v1);
		return v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2];
	}

	float bizGeoVecDistance2XZ(const Vector4& v0, const Vector4& v1)
	{
		Vector4 v2{};
		jetGeoVecSub(v2, v0, v1);
		return v2[0] * v2[0] + v2[2] * v2[2];
	}

	float bizGeoVecDistanceXZ(const Vector4& v0, const Vector4& v1)
	{
		Vector4 v2{};
		jetGeoVecSub(v2, v0, v1);
		return std::sqrt(v2[0] * v2[0] + v2[2] * v2[2]);
	}

	float bizGeoVecLength2(const Vector4& vec)
	{
		return vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
	}

	bool bizMathNearValue(float& pOut, const float& fValue, const float& fTarget, const float& fAdd)
	{
		if (JET_ABS(fTarget - fValue) <= fAdd)
		{
			pOut = fTarget;
			return true;
		}
		if (fValue < fTarget)
		{
			pOut = fValue + fAdd;
		}
		else
		{
			pOut = fValue - fAdd;
		}
		return false;
	}

	bool bizMathNearTrans(Vector4& vOut, const Vector4& vNow, const Vector4& vTarget, const float& fAdd)
	{
		if (bizGeoVecDistance2(vNow, vTarget) <= fAdd * fAdd)
		{
			jetGeoVecCopy(vOut, vTarget);
			return true;
		}
		Vector4 v{};
		jetGeoVecSub(v, vTarget, vNow);
		jetGeoVecNormalize(v, v);
		jetGeoVecScale(v, v, fAdd);
		jetGeoVecAdd(vOut, vNow, v);
		return false;
	}

	bool bizMathNearTransXZ(Vector4& vOut, const Vector4& vNow, const Vector4& vTarget, const float& fAdd)
	{
		Vector4 v{};
		jetGeoVecCopy(v, vNow);
		v[1] = vTarget[1];
		float value = vNow[1];
		bool result = bizMathNearTrans(vOut, v, vTarget, fAdd);
		vOut[1] = value;
		return result;
	}

	int bizMathNearDir(const float& fAng1, const float& fAng2)
	{
		float num;
		for (num = fAng2 - fAng1; num > PI * 2.0f; num -= PI * 2.0f)
		{
		}
		for (; num < 0.0f; num += PI * 2.0f)
		{
		}
		if (num == 0.0f)
		{
			return 0;
		}
		if (num >= PI)
		{
			return -1;
		}
		return 1;
	}

	float bizMathGetRad(const float& fAng1, const float& fAng2)
	{
		float num;
		for (num = fAng2 - fAng1; num > PI * 2.0f; num -= PI * 2.0f)
		{
		}
		for (; num < 0.0f; num += PI * 2.0f)
		{
		}
		if (num > PI)
		{
			num -= PI * 2.0f;
		}
		return num;
	}

	bool bizMathNearRad(float& pOut, const float& fNow, const float& fTarget, const float& fAdd)
	{
		if (JET_ABS(bizMathGetRad(fNow, fTarget)) <= fAdd)
		{
			pOut = fTarget;
			return true;
		}
		pOut = fNow + fAdd * static_cast<float>(bizMathNearDir(fNow, fTarget));
		return false;
	}

	float bizMathLerpFloat(const float& fVal0, const float& fVal1, const float& fRate)
	{
		return fVal0 * fRate + fVal1 * (1.0f - fRate);
	}



	void Jbc_SetConfig(JBC_Handler& hnd, uint32_t msk, uint32_t sft, uint32_t ntbl, JBC_Element* tbl)
	{
		hnd.mask = msk;
		hnd.shift = sft;
		hnd.n_table = ntbl;
		hnd.table = tbl;
	}

	void Jbc_SetProcNo(JBC_Handler& hnd, uint32_t no)
	{
		hnd.new_no = no;
	}

	void Jbc_SetForceProcNo(JBC_Handler& hnd, uint32_t no)
	{
		hnd.new_no = no;
		hnd.force = true;
	}

	uint32_t Jbc_GetProcNo(JBC_Handler& hnd)
	{
		return hnd.cur_no;
	}

	uint32_t Jbc_GetProcNoOld(JBC_Handler& hnd)
	{
		return hnd.old_no;
	}

	uint32_t GetTableNum(JBC_Handler& hnd, uint32_t num)
	{
		return (num >> (int)hnd.shift) & hnd.mask;
	}

	uint32_t GetAboveNum(JBC_Handler& hnd, uint32_t num)
	{
		return (num >> (int)hnd.shift) & ~hnd.mask;
	}

	bool IsPrgChg(JBC_Handler& hnd, uint32_t num0, uint32_t num1)
	{
		return num0 >> (int)hnd.shift != num1 >> (int)hnd.shift;
	}

	bool IsAboveChg(JBC_Handler& hnd, uint32_t num0, uint32_t num1)
	{
		return GetAboveNum(hnd, num0) != GetAboveNum(hnd, num1);
	}

	void Jbc_Init(JBC_Handler& hnd)
	{
		hnd.force = false;
		hnd.new_no = UINT32_MAX;
		hnd.cur_no = UINT32_MAX;
		hnd.old_no = UINT32_MAX;
		hnd.mask = UINT32_MAX;
		hnd.shift = 0u;
		hnd.n_table = 0u;
		hnd.table = nullptr;
	}

	bool Jbc_ExecProc(JBC_Handler& hnd)
	{
		if (hnd.table != nullptr && (hnd.force || IsPrgChg(hnd, hnd.new_no, hnd.cur_no)))
		{
			hnd.force = false;
			if (hnd.cur_no != UINT32_MAX)
			{
				JBC_Element jBC_Element = hnd.table[GetTableNum(hnd, hnd.cur_no)];
				if (jBC_Element.term != nullptr)
				{
					jBC_Element.term();
				}
			}
			if (IsAboveChg(hnd, hnd.new_no, hnd.cur_no) && hnd.cur_no != UINT32_MAX)
			{
				hnd.new_no = UINT32_MAX;
			}
			hnd.old_no = hnd.cur_no;
			hnd.cur_no = hnd.new_no;
			if (hnd.new_no != UINT32_MAX)
			{
				JBC_Element jBC_Element = hnd.table[GetTableNum(hnd, hnd.new_no)];
				if (jBC_Element.init != nullptr)
				{
					jBC_Element.init();
				}
			}
		}
		return hnd.cur_no != UINT32_MAX;
	}
}
