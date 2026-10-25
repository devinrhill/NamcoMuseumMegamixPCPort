// Matrix4x4 / Quaternion conventions the 2D layer code depends on (a regression test: Matrix4x4 * Matrix4x4 used to multiply
// element by element, which made every composed layer transform wrong).
#include <cmath>
#include <cstdio>
#include "common.hpp"

#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)
static bool near(float a, float b) { return std::fabs(a - b) < 1e-4f; }

int main()
{
	// (T * R * S) applies S first, then R, then T (column vectors)
	Matrix4x4 m = Matrix4x4::Translate(Vector3(10, 5, 0)) * Matrix4x4::Rotate(Quaternion::Euler(0, 0, 90)) * Matrix4x4::Scale(Vector3(2, 3, 1));
	Vector4 p = m * Vector4(1, 0, 0, 1);       // (1,0) -> scale (2,0) -> rotate 90 deg ccw (0,2) -> translate (10,7)
	CHECK(near(p.x, 10) && near(p.y, 7));
	p = m * Vector4(0, 1, 0, 1);               // (0,1) -> (0,3) -> (-3,0) -> (7,5)
	CHECK(near(p.x, 7) && near(p.y, 5));

	// identity and associativity
	Matrix4x4 i = Matrix4x4::identity();
	Matrix4x4 a = Matrix4x4::Translate(Vector3(1, 2, 3)), b = Matrix4x4::Scale(Vector3(2, 2, 2)), c = Matrix4x4::Rotate(Quaternion::Euler(0, 0, 30));
	Matrix4x4 l = (a * b) * c, r = a * (b * c), ia = i * a;
	for (int k = 0; k < 16; k++) { CHECK(near(l.m[k], r.m[k])); CHECK(near(ia.m[k], a.m[k])); }

	// *= is the product too
	Matrix4x4 acc = a; acc *= b;
	Matrix4x4 ab = a * b;
	for (int k = 0; k < 16; k++) CHECK(near(acc.m[k], ab.m[k]));

	// SetTRS == T * R * S
	Matrix4x4 t; t.SetTRS(Vector3(4, 5, 6), Quaternion::Euler(0, 0, 45), Vector3(2, 1, 1));
	Matrix4x4 e = Matrix4x4::Translate(Vector3(4, 5, 6)) * Matrix4x4::Rotate(Quaternion::Euler(0, 0, 45)) * Matrix4x4::Scale(Vector3(2, 1, 1));
	for (int k = 0; k < 16; k++) CHECK(near(t.m[k], e.m[k]));

	// an Anita layer as cJetAnita::LayerAffine builds it: the logo's end cap (position (253,118), turned -90 degrees)
	// 160 x 96 quad: its corners must land at x 205..301, y 38..198 (checked against the Python reference layer_matrix)
	Matrix4x4 lm = Matrix4x4::identity();
	Matrix4x4 inner; inner.SetTRS(Vector3(253, 118, 0), Quaternion::Euler(0, 0, 90), Vector3(1, 1, 1)); // -(-90)
	lm *= inner;
	Vector4 tl = lm * Vector4(-80, 48, 0, 1), br = lm * Vector4(80, -48, 0, 1);
	CHECK(near(tl.x, 205) && near(tl.y, 38) && near(br.x, 301) && near(br.y, 198));

	std::printf("gfx math tests passed\n");
	return 0;
}
