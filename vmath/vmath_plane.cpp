// ============================
// common/vmath/vmath_plane.cpp
// ============================

#include "vmath_plane.h"

const Plane3V_SOA4 Plane3V_SOA4::ConstructFromProjectionMatrix_LTRB(Mat44V_arg proj, Plane3V* out_nearPlane, Plane3V* out_farPlane)
{
	Mat44V projTranspose = Transpose(proj);
	Plane3V_SOA4 p;
	Mat44VTranspose(
		p.m_normal.x_ref(),
		p.m_normal.y_ref(),
		p.m_normal.z_ref(),
		p.m_d,
		projTranspose.d() + projTranspose.a(), // left
		projTranspose.d() - projTranspose.b(), // top
		projTranspose.d() - projTranspose.a(), // right
		projTranspose.d() + projTranspose.b()); // bottom
	if (out_nearPlane)
		*out_nearPlane = projTranspose.d() + projTranspose.c();
	if (out_farPlane)
		*out_farPlane = projTranspose.d() - projTranspose.c();
	return p;
}