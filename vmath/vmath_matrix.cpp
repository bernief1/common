// =============================
// common/vmath/vmath_matrix.cpp
// =============================

#include "vmath_matrix.h"
#include "vmath_box.h"
#include "vmath_plane.h"
#include "vmath_sphere.h"

const Mat33V& Mat33V::StaticIdentity()
{
	static Mat33V identity = Identity();
	return identity;
}

const Mat34V& Mat34V::StaticIdentity()
{
	static Mat34V identity = Identity();
	return identity;
}

const Mat44V& Mat44V::StaticIdentity()
{
	static Mat44V identity = Identity();
	return identity;
}

template <typename T> static inline bool IsNaN_T(T a) { return Any(!(a == a)); }

Mat33V_out Mat33V::ConstructBasis(Vec3V_arg forwardDir)
{
	if (!IsNaN_T(forwardDir) && !All(forwardDir == 0.0f)) { // if we don't check for NaN/zero, ConstructBasis can recurse infinitely
		const Vec3V absDir = Abs(forwardDir);
		const ScalarV minElemDir = MinElement(absDir);
		Vec3V upDir;
		if      (minElemDir == absDir.z()) upDir = Vec3V(V_ZAXIS); // would using Select be faster?
		else if (minElemDir == absDir.y()) upDir = Vec3V(V_YAXIS);
		else                               upDir = Vec3V(V_XAXIS);
		return ConstructBasisUp(forwardDir,upDir); // should we flip upDir based on sign of forwardDir element?
	} else
		return Mat33V::Identity();
}

Mat33V_out Mat33V::ConstructBasisUp(Vec3V_arg forwardDir,Vec3V_arg upDir)
{
	const Vec3V cross = Cross(upDir,forwardDir);
	if (!All(cross == 0.0f)) {
		const Vec3V basisZ = Normalize(forwardDir); // forwardDir isn't necessarily normalized
		const Vec3V basisX = Normalize(cross);
		const Vec3V basisY = Cross(basisZ,basisX); // should be normalized already
		return Mat33V(basisX,basisY,basisZ);
	} else
		return ConstructBasis(forwardDir);
}

const Box3V Mat33V::TransformBox(const Box3V& box) const
{
	const Vec3V center = Transform(box.GetCenter());
	const Vec3V extent = Mat33V(Abs(a()),Abs(b()),Abs(c())).Transform(box.GetExtent());
	return Box3V(center - extent,center + extent);
}

const Plane3V Mat33V::TransformPlane(const Plane3V& plane) const
{
	const Vec3V normal = Transform(plane.GetNormal());
	const ScalarV dist = plane.GetDistance();
	return Plane3V(normal,dist);
}

const Sphere3V Mat33V::TransformSphere(const Sphere3V& sphere) const
{
	return Sphere3V(Transform(sphere.GetCenter()),sphere.GetRadius());
}

const Box3V Mat34V::TransformBox(const Box3V& box) const
{
#if 1 // optimized
	const Vec3V center = Transform(box.GetCenter());
	const Vec3V extent = Mat33V(Abs(m_mat33.a()),Abs(m_mat33.b()),Abs(m_mat33.c())).Transform(box.GetExtent());
	return Box3V(center - extent,center + extent);
#else // reference
	Box3V box = Box3V::Invalid();
	Vec3V corners[8];
	box.GetCorners(corners);
	for (unsigned i = 0; i < countof(corners); i++)
		box.Grow(Transform(corners[i]));
	return box;
#endif
}

const Plane3V Mat34V::TransformPlane(const Plane3V& plane) const
{
#if 1 // optimized
	const Vec3V normal = TransformDir(plane.GetNormal());
	const ScalarV dist = plane.GetDistance() - Dot(m_d,normal);
	return Plane3V(normal,dist);
#else // reference
	const Vec3V normal = plane.GetNormal();
	const ScalarV dist = plane.GetDistance();
	return Plane3V::ConstructFromPointAndNormal(Transform(-normal*dist),TransformDir(normal));
#endif
}

const Sphere3V Mat34V::TransformSphere(const Sphere3V& sphere) const
{
	return Sphere3V(Transform(sphere.GetCenter()),sphere.GetRadius());
}

Mat44V_out Invert(Mat44V_arg m)
{
	// "Streaming SIMD Extensions - Inverse of 4x4 Matrix"
	// https://github.com/niswegmann/small-matrix-inverse/blob/master/invert4x4_sse.h
	// http://withkei.tistory.com/attachment/jk0.pdf
	__m128 dst0,dst1,dst2,dst3;
	__m128 row0,row1,row2,row3;
	__m128 det,tmp1;
#if 0 // still deciding on syntax here ..
	tmp1 = _vmath_permute_AABB_XYZW_ps<'xyxy'>(m.a(),m.b()); // _mm_loadh_pi(_mm_loadl_pi(tmp1,(const __m64*)(src)),(const __m64*)(src+4)); // 0,1,4,5
	row1 = _vmath_permute_AABB_XYZW_ps<'xyxy'>(m.c(),m.d()); // _mm_loadh_pi(_mm_loadl_pi(row1,(const __m64*)(src+8)),(const __m64*)(src+12)); // 8,9,12,13
	row0 = _vmath_permute_AABB_XYZW_ps<'xzxz'>(tmp1,row1); // 0,4,8,12
	row1 = _vmath_permute_AABB_XYZW_ps<'ywyw'>(row1,tmp1); // 9,13,1,5
	tmp1 = _vmath_permute_AABB_XYZW_ps<'zwzw'>(m.a(),m.b()); // _mm_loadh_pi(_mm_loadl_pi(tmp1,(const __m64*)(src+2)),(const __m64*)(src+6)); // 2,3,6,7
	row3 = _vmath_permute_AABB_XYZW_ps<'zwzw'>(m.c(),m.d()); // _mm_loadh_pi(_mm_loadl_pi(row3,(const __m64*)(src+10)),(const __m64*)(src+14)); // 10,11,14,15
	row2 = _vmath_permute_AABB_XYZW_ps<'xzxz'>(tmp1,row3); // 2,6,10,14
	row3 = _vmath_permute_AABB_XYZW_ps<'ywyw'>(row3,tmp1); // 11,15,3,7
#else
	tmp1 = _vmath_permute_AABB_ps<0,1,0,1>(m.a(),m.b()); // _mm_loadh_pi(_mm_loadl_pi(tmp1,(const __m64*)(src)),(const __m64*)(src+4)); // 0,1,4,5
	row1 = _vmath_permute_AABB_ps<0,1,0,1>(m.c(),m.d()); // _mm_loadh_pi(_mm_loadl_pi(row1,(const __m64*)(src+8)),(const __m64*)(src+12)); // 8,9,12,13
	row0 = _vmath_permute_AABB_ps<0,2,0,2>(tmp1,row1); // 0,4,8,12
	row1 = _vmath_permute_AABB_ps<1,3,1,3>(row1,tmp1); // 9,13,1,5
	tmp1 = _vmath_permute_AABB_ps<2,3,2,3>(m.a(),m.b()); // _mm_loadh_pi(_mm_loadl_pi(tmp1,(const __m64*)(src+2)),(const __m64*)(src+6)); // 2,3,6,7
	row3 = _vmath_permute_AABB_ps<2,3,2,3>(m.c(),m.d()); // _mm_loadh_pi(_mm_loadl_pi(row3,(const __m64*)(src+10)),(const __m64*)(src+14)); // 10,11,14,15
	row2 = _vmath_permute_AABB_ps<0,2,0,2>(tmp1,row3); // 2,6,10,14
	row3 = _vmath_permute_AABB_ps<1,3,1,3>(row3,tmp1); // 11,15,3,7
#endif
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row2,row3);
	tmp1 = _vmath_permute_ps<1,0,3,2>(tmp1);
	dst0 = _mm_mul_ps(row1,tmp1);
	dst1 = _mm_mul_ps(row0,tmp1);
	tmp1 = _vmath_permute_ps<2,3,0,1>(tmp1);
	dst0 = _mm_fmsub_ps(row1,tmp1,dst0);
	dst1 = _mm_fmsub_ps(row0,tmp1,dst1);
	dst1 = _vmath_permute_ps<2,3,0,1>(dst1);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row1,row2);
	tmp1 = _vmath_permute_ps<1,0,3,2>(tmp1);
	dst0 = _mm_fmadd_ps(row3,tmp1,dst0);
	dst3 = _mm_mul_ps(row0,tmp1);
	tmp1 = _vmath_permute_ps<2,3,0,1>(tmp1);
	dst0 = _mm_fnmadd_ps(row3,tmp1,dst0);
	dst3 = _mm_fmsub_ps(row0,tmp1,dst3);
	dst3 = _vmath_permute_ps<2,3,0,1>(dst3);
	// -----------------------------------------------
	tmp1 = _vmath_permute_ps<2,3,0,1>(row1);
	tmp1 = _mm_mul_ps(tmp1,row3);
	tmp1 = _vmath_permute_ps<1,0,3,2>(tmp1);
	row2 = _vmath_permute_ps<2,3,0,1>(row2);
	dst0 = _mm_fmadd_ps(row2,tmp1,dst0);
	dst2 = _mm_mul_ps(row0,tmp1);
	tmp1 = _vmath_permute_ps<2,3,0,1>(tmp1);
	dst0 = _mm_fnmadd_ps(row2,tmp1,dst0);
	dst2 = _mm_fmsub_ps(row0,tmp1,dst2);
	dst2 = _vmath_permute_ps<2,3,0,1>(dst2);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0,row1);
	tmp1 = _vmath_permute_ps<1,0,3,2>(tmp1);
	dst2 = _mm_fmadd_ps(row3,tmp1,dst2);
	dst3 = _mm_fmsub_ps(row2,tmp1,dst3);
	tmp1 = _vmath_permute_ps<2,3,0,1>(tmp1);
	dst2 = _mm_fmsub_ps(row3,tmp1,dst2);
	dst3 = _mm_fnmadd_ps(row2,tmp1,dst3);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0,row3);
	tmp1 = _vmath_permute_ps<1,0,3,2>(tmp1);
	dst1 = _mm_fnmadd_ps(row2,tmp1,dst1);
	dst2 = _mm_fmadd_ps(row1,tmp1,dst2);
	tmp1 = _vmath_permute_ps<2,3,0,1>(tmp1);
	dst1 = _mm_fmadd_ps(row2,tmp1,dst1);
	dst2 = _mm_fnmadd_ps(row1,tmp1,dst2);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0,row2);
	tmp1 = _vmath_permute_ps<1,0,3,2>(tmp1);
	dst1 = _mm_fmadd_ps(row3,tmp1,dst1);
	dst3 = _mm_fnmadd_ps(row1,tmp1,dst3);
	tmp1 = _vmath_permute_ps<2,3,0,1>(tmp1);
	dst1 = _mm_fnmadd_ps(row3,tmp1,dst1);
	dst3 = _mm_fmadd_ps(row1,tmp1,dst3);
	// -----------------------------------------------
	det = _mm_mul_ps(row0,dst0);
	det = _mm_hadd_ps(det,det);
	det = _mm_hadd_ps(det,det);
	tmp1 = _mm_rcp_ss(det);
	det = _mm_fnmadd_ss(det,_mm_mul_ss(tmp1,tmp1),_mm_add_ss(tmp1,tmp1)); // Newton-Raphson refine
	det = _vmath_splatx_ps(det);
	// -----------------------------------------------
	dst0 = _mm_mul_ps(det,dst0);
	dst1 = _mm_mul_ps(det,dst1);
	dst2 = _mm_mul_ps(det,dst2);
	dst3 = _mm_mul_ps(det,dst3);
	return Mat44V(Vec4V(dst0),Vec4V(dst1),Vec4V(dst2),Vec4V(dst3));
}

Mat44V_out Invert_REFERENCE(Mat44V_arg m)
{
	const float a00 = m.a().xf(), a10 = m.a().yf(), a20 = m.a().zf(), a30 = m.a().wf();
	const float a01 = m.b().xf(), a11 = m.b().yf(), a21 = m.b().zf(), a31 = m.b().wf();
	const float a02 = m.c().xf(), a12 = m.c().yf(), a22 = m.c().zf(), a32 = m.c().wf();
	const float a03 = m.d().xf(), a13 = m.d().yf(), a23 = m.d().zf(), a33 = m.d().wf();

	// http://www.euclideanspace.com/maths/algebra/matrix/code
	const float invdet = 1.0f/(
		a03*a12*a21*a30 - a02*a13*a21*a30 - a03*a11*a22*a30 + a01*a13*a22*a30 +
		a02*a11*a23*a30 - a01*a12*a23*a30 - a03*a12*a20*a31 + a02*a13*a20*a31 +
		a03*a10*a22*a31 - a00*a13*a22*a31 - a02*a10*a23*a31 + a00*a12*a23*a31 +
		a03*a11*a20*a32 - a01*a13*a20*a32 - a03*a10*a21*a32 + a00*a13*a21*a32 +
		a01*a10*a23*a32 - a00*a11*a23*a32 - a02*a11*a20*a33 + a01*a12*a20*a33 +
		a02*a10*a21*a33 - a00*a12*a21*a33 - a01*a10*a22*a33 + a00*a11*a22*a33);

	const float m00 = a12*a23*a31 - a13*a22*a31 + a13*a21*a32 - a11*a23*a32 - a12*a21*a33 + a11*a22*a33;
	const float m01 = a03*a22*a31 - a02*a23*a31 - a03*a21*a32 + a01*a23*a32 + a02*a21*a33 - a01*a22*a33;
	const float m02 = a02*a13*a31 - a03*a12*a31 + a03*a11*a32 - a01*a13*a32 - a02*a11*a33 + a01*a12*a33;
	const float m03 = a03*a12*a21 - a02*a13*a21 - a03*a11*a22 + a01*a13*a22 + a02*a11*a23 - a01*a12*a23;
	const float m10 = a13*a22*a30 - a12*a23*a30 - a13*a20*a32 + a10*a23*a32 + a12*a20*a33 - a10*a22*a33;
	const float m11 = a02*a23*a30 - a03*a22*a30 + a03*a20*a32 - a00*a23*a32 - a02*a20*a33 + a00*a22*a33;
	const float m12 = a03*a12*a30 - a02*a13*a30 - a03*a10*a32 + a00*a13*a32 + a02*a10*a33 - a00*a12*a33;
	const float m13 = a02*a13*a20 - a03*a12*a20 + a03*a10*a22 - a00*a13*a22 - a02*a10*a23 + a00*a12*a23;
	const float m20 = a11*a23*a30 - a13*a21*a30 + a13*a20*a31 - a10*a23*a31 - a11*a20*a33 + a10*a21*a33;
	const float m21 = a03*a21*a30 - a01*a23*a30 - a03*a20*a31 + a00*a23*a31 + a01*a20*a33 - a00*a21*a33;
	const float m22 = a01*a13*a30 - a03*a11*a30 + a03*a10*a31 - a00*a13*a31 - a01*a10*a33 + a00*a11*a33;
	const float m23 = a03*a11*a20 - a01*a13*a20 - a03*a10*a21 + a00*a13*a21 + a01*a10*a23 - a00*a11*a23;
	const float m30 = a12*a21*a30 - a11*a22*a30 - a12*a20*a31 + a10*a22*a31 + a11*a20*a32 - a10*a21*a32;
	const float m31 = a01*a22*a30 - a02*a21*a30 + a02*a20*a31 - a00*a22*a31 - a01*a20*a32 + a00*a21*a32;
	const float m32 = a02*a11*a30 - a01*a12*a30 - a02*a10*a31 + a00*a12*a31 + a01*a10*a32 - a00*a11*a32;
	const float m33 = a01*a12*a20 - a02*a11*a20 + a02*a10*a21 - a00*a12*a21 - a01*a10*a22 + a00*a11*a22;

	return Mat44V(
		Vec4V(m00,m10,m20,m30)*invdet,
		Vec4V(m01,m11,m21,m31)*invdet,
		Vec4V(m02,m12,m22,m32)*invdet,
		Vec4V(m03,m13,m23,m33)*invdet);
}