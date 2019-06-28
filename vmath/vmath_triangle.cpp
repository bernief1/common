// ===============================
// common/vmath/vmath_triangle.cpp
// ===============================

#include "vmath_intersects.h"
#include "vmath_sphere.h"
#include "vmath_triangle.h"

uint32 Triangle3V::IntersectsRaySign(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,ScalarV& sign,float epsilon) const
{
	const Vec3V v0v1 = m_positions[1] - m_positions[0];
	const Vec3V v0v2 = m_positions[2] - m_positions[0];
	const Vec3V pvec = Cross(dir,v0v2);
	const float det = Dot(v0v1,pvec).f();
	if (Abs(det) - epsilon < 0.0f)
		return 0;
	const float invDet = 1.0f/det;
	const Vec3V tvec = origin - m_positions[0];
	const float u = (Dot(tvec,pvec)*invDet).f();
	if (u < 0.0f) // technically we can skip the u > 1 test since it will be handled in the u + v > 1 test below
		return 0;
	const Vec3V qvec = Cross(tvec,v0v1);
	const float v = (Dot(dir,qvec)*invDet).f();
	if (Min(v,1.0f - u - v) < 0.0f)
		return 0;
	t = Dot(v0v2,qvec)*invDet;
	sign = ScalarV(det);
	return t > 0.0f ? 1 : 0;
}

// http://realtimecollisiondetection.net/blog/?p=103
uint32 Triangle3V::IntersectsSphere(const Sphere3V& sphere) const
{
	if (!Intersects(sphere,GetBounds()))
		return 0;
	const Vec3V P = sphere.GetCenter();
	const ScalarV r = sphere.GetRadius();
	const Vec3V A = m_positions[0] - P;
	const Vec3V B = m_positions[1] - P;
	const Vec3V C = m_positions[2] - P;
	const ScalarV rr = r*r;
	const Vec3V V = Cross(B - A,C - A);
	const ScalarV d = Dot(A,V);
	const ScalarV e = Dot(V,V);
	const bool test1 = (d*d <= rr*e);
	const ScalarV aa = Dot(A,A);
	const ScalarV ab = Dot(A,B);
	const ScalarV ac = Dot(A,C);
	const ScalarV bb = Dot(B,B);
	const ScalarV bc = Dot(B,C);
	const ScalarV cc = Dot(C,C);
	const bool test2 = (aa <= rr || ab <= aa || ac <= aa);
	const bool test3 = (bb <= rr || ab <= bb || bc <= bb);
	const bool test4 = (cc <= rr || ac <= cc || bc <= cc);
	const Vec3V AB = B - A;
	const Vec3V BC = C - B;
	const Vec3V CA = A - C;
	const ScalarV d1 = ab - aa;
	const ScalarV d2 = bc - bb;
	const ScalarV d3 = ac - cc;
	const ScalarV e1 = Dot(AB,AB);
	const ScalarV e2 = Dot(BC,BC);
	const ScalarV e3 = Dot(CA,CA);
	const Vec3V Q1 = A*e1 - d1*AB;
	const Vec3V Q2 = B*e2 - d2*BC;
	const Vec3V Q3 = C*e3 - d3*CA;
	const Vec3V QC = C*e1 - Q1;
	const Vec3V QA = A*e2 - Q2;
	const Vec3V QB = B*e3 - Q3;
	const bool test5 = (Dot(Q1,Q1) <= rr*e1*e1 || Dot(Q1,QC) <= 0.0f);
	const bool test6 = (Dot(Q2,Q2) <= rr*e2*e2 || Dot(Q2,QA) <= 0.0f);
	const bool test7 = (Dot(Q3,Q3) <= rr*e3*e3 || Dot(Q3,QB) <= 0.0f);
	return (test1 && test2 && test3 && test4 && test5 && test6 && test7) ? 1 : 0;
}

uint32 Triangle3V_SOA4::IntersectsSphere(const Sphere3V& sphere) const
{
	if (!Intersects(sphere,GetBounds()))
		return 0;
	const Vec3V P = sphere.GetCenter();
	const ScalarV r = sphere.GetRadius();
	const Vec3V_SOA4 A = m_positions[0] - P;
	const Vec3V_SOA4 B = m_positions[1] - P;
	const Vec3V_SOA4 C = m_positions[2] - P;
	const ScalarV rr = r*r;
	const Vec3V_SOA4 V = Cross(B - A,C - A);
	const Vec4V d = Dot(A,V);
	const Vec4V e = Dot(V,V);
	const Vec4V::BoolV test1 = (d*d <= rr*e);
	const Vec4V aa = Dot(A,A);
	const Vec4V ab = Dot(A,B);
	const Vec4V ac = Dot(A,C);
	const Vec4V bb = Dot(B,B);
	const Vec4V bc = Dot(B,C);
	const Vec4V cc = Dot(C,C);
	const Vec4V::BoolV test2 = (aa <= rr || ab <= aa || ac <= aa);
	const Vec4V::BoolV test3 = (bb <= rr || ab <= bb || bc <= bb);
	const Vec4V::BoolV test4 = (cc <= rr || ac <= cc || bc <= cc);
	const Vec3V_SOA4 AB = B - A;
	const Vec3V_SOA4 BC = C - B;
	const Vec3V_SOA4 CA = A - C;
	const Vec4V d1 = ab - aa;
	const Vec4V d2 = bc - bb;
	const Vec4V d3 = ac - cc;
	const Vec4V e1 = Dot(AB,AB);
	const Vec4V e2 = Dot(BC,BC);
	const Vec4V e3 = Dot(CA,CA);
	const Vec3V_SOA4 Q1 = A*e1 - d1*AB;
	const Vec3V_SOA4 Q2 = B*e2 - d2*BC;
	const Vec3V_SOA4 Q3 = C*e3 - d3*CA;
	const Vec3V_SOA4 QC = C*e1 - Q1;
	const Vec3V_SOA4 QA = A*e2 - Q2;
	const Vec3V_SOA4 QB = B*e3 - Q3;
	const Vec4V::BoolV test5 = (Dot(Q1,Q1) <= rr*e1*e1 || Dot(Q1,QC) <= 0.0f);
	const Vec4V::BoolV test6 = (Dot(Q2,Q2) <= rr*e2*e2 || Dot(Q2,QA) <= 0.0f);
	const Vec4V::BoolV test7 = (Dot(Q3,Q3) <= rr*e3*e3 || Dot(Q3,QB) <= 0.0f);
	return (test1 && test2 && test3 && test4 && test5 && test6 && test7).GetMask();
}

uint32 Triangle3V::IntersectsBox(const Box3V& box) const
{
	// ============================================================================
	// http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox3.txt
	// http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox_tam.pdf
	// 
	// AABB-triangle overlap test code
	// by Tomas Akenine-Möller
	// 
	// History:
	//   2001-03-05: released the code in its first version
	//   2001-06-18: changed the order of the tests, faster
	// 
	// Acknowledgment: Many thanks to Pierre Terdiman for
	// suggestions and discussions on how to optimize code.
	// Thanks to David Hunt for finding a ">="-bug!
	// ============================================================================

	if (!Intersects(box,GetBounds()))
		return 0;
	else if (!Intersects(GetWeightedPlane(),box)) // plane doesn't need to be normalized
		return 0;

	const Vec3V center = box.GetCenter();
	const Vec3V extent = box.GetExtent();

	const Vec3V v0 = m_positions[0] - center;
	const Vec3V v1 = m_positions[1] - center;
	const Vec3V v2 = m_positions[2] - center;

	const Vec3V e0 = v1 - v0; // TODO -- rename e01, etc.
	const Vec3V e1 = v2 - v1;
	const Vec3V e2 = v0 - v2;
	
	// TODO -- vectorize this
#define AXISTEST_X01(a,b,fa,fb) { p2 =  a*v2.y() - b*v2.z(); p0 =  a*v0.y() - b*v0.z(); r = fa*extent.y() + fb*extent.z(); if (Min(p0,p2) > r || -r > Max(p0,p2)) return 0; }
#define AXISTEST_X2( a,b,fa,fb) { p0 =  a*v0.y() - b*v0.z(); p1 =  a*v1.y() - b*v1.z(); r = fa*extent.y() + fb*extent.z(); if (Min(p0,p1) > r || -r > Max(p0,p1)) return 0; }
#define AXISTEST_Y02(a,b,fa,fb) { p2 = -a*v2.x() + b*v2.z(); p0 = -a*v0.x() + b*v0.z(); r = fa*extent.x() + fb*extent.z(); if (Min(p0,p2) > r || -r > Max(p0,p2)) return 0; }
#define AXISTEST_Y1( a,b,fa,fb) { p0 = -a*v0.x() + b*v0.z(); p1 = -a*v1.x() + b*v1.z(); r = fa*extent.x() + fb*extent.z(); if (Min(p0,p1) > r || -r > Max(p0,p1)) return 0; }
#define AXISTEST_Z12(a,b,fa,fb) { p1 =  a*v1.x() - b*v1.y(); p2 =  a*v2.x() - b*v2.y(); r = fa*extent.x() + fb*extent.y(); if (Min(p1,p2) > r || -r > Max(p1,p2)) return 0; }
#define AXISTEST_Z0( a,b,fa,fb) { p0 =  a*v0.x() - b*v0.y(); p1 =  a*v1.x() - b*v1.y(); r = fa*extent.x() + fb*extent.y(); if (Min(p0,p1) > r || -r > Max(p0,p1)) return 0; }
	ScalarV p0,p1,p2,r;
	const Vec3V a0 = Abs(e0);
	AXISTEST_X01(e0.z(),e0.y(),a0.z(),a0.y());
	AXISTEST_Y02(e0.z(),e0.x(),a0.z(),a0.x());
	AXISTEST_Z12(e0.y(),e0.x(),a0.y(),a0.x());
	const Vec3V a1 = Abs(e1);
	AXISTEST_X01(e1.z(),e1.y(),a1.z(),a1.y());
	AXISTEST_Y02(e1.z(),e1.x(),a1.z(),a1.x());
	AXISTEST_Z0( e1.y(),e1.x(),a1.y(),a1.x());
	const Vec3V a2 = Abs(e2);
	AXISTEST_X2( e2.z(),e2.y(),a2.z(),a2.y());
	AXISTEST_Y1( e2.z(),e2.x(),a2.z(),a2.x());
	AXISTEST_Z12(e2.y(),e2.x(),a2.y(),a2.x());
#undef AXISTEST_X01
#undef AXISTEST_X2
#undef AXISTEST_Y02
#undef AXISTEST_Y1
#undef AXISTEST_Z12
#undef AXISTEST_Z0

	return 1;
}

uint32 Triangle3V_SOA4::IntersectsBox(const Box3V& box) const
{
	uint32 mask = Vec4V::BoolV::MaskAll;
	mask &= IntersectsBox3V_SOA(box,GetBounds_SOA());
	if (mask == 0)
		return 0;
	mask &= Intersects(GetWeightedPlane(),box); // plane doesn't need to be normalized
	if (mask == 0)
		return 0;

	const Vec3V center = box.GetCenter();
	const Vec3V extent = box.GetExtent();

	const Vec3V_SOA4 v0 = m_positions[0] - center;
	const Vec3V_SOA4 v1 = m_positions[1] - center;
	const Vec3V_SOA4 v2 = m_positions[2] - center;

	const Vec3V_SOA4 e0 = v1 - v0; // TODO -- rename e01, etc.
	const Vec3V_SOA4 e1 = v2 - v1;
	const Vec3V_SOA4 e2 = v0 - v2;
	
#define AXISTEST_X01(a,b,fa,fb) { p2 =  a*v2.y() - b*v2.z(); p0 =  a*v0.y() - b*v0.z(); r = fa*extent.y() + fb*extent.z(); mask &= (Min(p0,p2) <= r && -r <= Max(p0,p2)).GetMask(); }
#define AXISTEST_X2( a,b,fa,fb) { p0 =  a*v0.y() - b*v0.z(); p1 =  a*v1.y() - b*v1.z(); r = fa*extent.y() + fb*extent.z(); mask &= (Min(p0,p1) <= r && -r <= Max(p0,p1)).GetMask(); }
#define AXISTEST_Y02(a,b,fa,fb) { p2 = -a*v2.x() + b*v2.z(); p0 = -a*v0.x() + b*v0.z(); r = fa*extent.x() + fb*extent.z(); mask &= (Min(p0,p2) <= r && -r <= Max(p0,p2)).GetMask(); }
#define AXISTEST_Y1( a,b,fa,fb) { p0 = -a*v0.x() + b*v0.z(); p1 = -a*v1.x() + b*v1.z(); r = fa*extent.x() + fb*extent.z(); mask &= (Min(p0,p1) <= r && -r <= Max(p0,p1)).GetMask(); }
#define AXISTEST_Z12(a,b,fa,fb) { p1 =  a*v1.x() - b*v1.y(); p2 =  a*v2.x() - b*v2.y(); r = fa*extent.x() + fb*extent.y(); mask &= (Min(p1,p2) <= r && -r <= Max(p1,p2)).GetMask(); }
#define AXISTEST_Z0( a,b,fa,fb) { p0 =  a*v0.x() - b*v0.y(); p1 =  a*v1.x() - b*v1.y(); r = fa*extent.x() + fb*extent.y(); mask &= (Min(p0,p1) <= r && -r <= Max(p0,p1)).GetMask(); }
	Vec4V p0,p1,p2,r;
	const Vec3V_SOA4 a0 = Abs(e0);
	AXISTEST_X01(e0.z(),e0.y(),a0.z(),a0.y());
	AXISTEST_Y02(e0.z(),e0.x(),a0.z(),a0.x());
	AXISTEST_Z12(e0.y(),e0.x(),a0.y(),a0.x());
	const Vec3V_SOA4 a1 = Abs(e1);
	AXISTEST_X01(e1.z(),e1.y(),a1.z(),a1.y());
	AXISTEST_Y02(e1.z(),e1.x(),a1.z(),a1.x());
	AXISTEST_Z0( e1.y(),e1.x(),a1.y(),a1.x());
	const Vec3V_SOA4 a2 = Abs(e2);
	AXISTEST_X2( e2.z(),e2.y(),a2.z(),a2.y());
	AXISTEST_Y1( e2.z(),e2.x(),a2.z(),a2.x());
	AXISTEST_Z12(e2.y(),e2.x(),a2.y(),a2.x());
#undef AXISTEST_X01
#undef AXISTEST_X2
#undef AXISTEST_Y02
#undef AXISTEST_Y1
#undef AXISTEST_Z12
#undef AXISTEST_Z0

	return mask;
}

#if 1 // early outs
#define EARLY_OUT4(mask) if (_mm_movemask_ps(mask) == Vec4V::BoolV::MaskAll) return 0
#define EARLY_OUT4_FINAL(mask,maskbits) const uint32 maskbits = _mm_movemask_ps(mask); if (maskbits == Vec4V::BoolV::MaskAll) return 0
#define EARLY_OUT8(mask) if (_mm256_movemask_ps(mask) == Vec8V::BoolV::MaskAll) return 0
#define EARLY_OUT8_FINAL(mask,maskbits) const uint32 maskbits = _mm256_movemask_ps(mask); if (maskbits == Vec8V::BoolV::MaskAll) return 0
#define EARLY_OUT16(mask) if (_mm512_movepi32_mask(_mm512_castps_si512(mask)) == Vec16V::BoolV::MaskAll) return 0
#define EARLY_OUT16_FINAL(mask,maskbits) const __mmask16 maskbits = _mm512_movepi32_mask(_mm512_castps_si512(mask)); if (maskbits == Vec16V::BoolV::MaskAll) return 0
#else // skip early-out tests
#define EARLY_OUT4(mask)
#define EARLY_OUT4_FINAL(mask,maskbits) const uint32 maskbits = _mm_movemask_ps(mask)
#define EARLY_OUT8(mask)
#define EARLY_OUT8_FINAL(mask,maskbits) const uint32 maskbits = _mm256_movemask_ps(mask)
#define EARLY_OUT16(mask)
#define EARLY_OUT16_FINAL(mask,maskbits) const __mmask16 maskbits = _mm512_movepi32_mask(_mm512_castps_si512(mask))
#endif

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
uint32 Triangle3V::IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,float &t,float &u,float &v,bool twosided,float epsilon) const
{
	const Vec3V v0v1 = m_positions[1] - m_positions[0];
	const Vec3V v0v2 = m_positions[2] - m_positions[0];
	const Vec3V pvec = Cross(dir,v0v2);
	const float det = Dot(v0v1,pvec).f();
	float test = det;
	if (twosided)
		test = Abs(test);
	if (test - epsilon < 0.0f)
		return 0;
	const float invDet = 1.0f/det;
	const Vec3V tvec = origin - m_positions[0];
	u = (Dot(tvec,pvec)*invDet).f();
	if (u < 0.0f) // technically we can skip the u > 1 test since it will be handled in the u + v > 1 test below
		return 0;
	const Vec3V qvec = Cross(tvec,v0v1);
	v = (Dot(dir,qvec)*invDet).f();
	if (Min(v,1.0f - u - v) < 0.0f)
		return 0;
	t = (Dot(v0v2,qvec)*invDet).f();
	return t > 0.0f ? 1 : 0;
}

uint32 Triangle3V::IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,bool twosided,float epsilon) const
{
	float t_,u_,v_;
	const uint32 result = IntersectsRay(origin,dir,t_,u_,v_,twosided,epsilon);
	t = ScalarV(t_);
	return result;
}

uint32 Triangle3V::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin,Vec3V_SOA4_arg dir,Vec4V& t,uint32,bool twosided,float epsilon) const
{
	const Vec3V v0v1 = m_positions[1] - m_positions[0];
	const Vec3V v0v2 = m_positions[2] - m_positions[0];
	const Vec3V_SOA4 pvec = Cross(dir,v0v2);
	const Vec4V det = Dot(v0v1,pvec);
	Vec4V test = det;
	if (twosided)
		test = Abs(test);
	Vec4V mask = test - Vec4V(epsilon);
	EARLY_OUT4(mask);
	const Vec4V invDet = Recip(det);
	const auto tvec = origin - m_positions[0];
	const Vec4V u = Dot(tvec,pvec)*invDet;
	mask |= u;
	EARLY_OUT4(mask);
	const auto qvec = Cross(tvec,v0v1);
	const Vec4V v = Dot(dir,qvec)*invDet;
	mask |= Min(v,Vec4V(V_ONE) - u - v);
	EARLY_OUT4(mask);
	t = Dot(v0v2,qvec)*invDet;
	mask |= t;
	EARLY_OUT4_FINAL(mask,maskbits);
	t = Select(t,Vec4V(ZBUFFER_DEFAULT),Vec4V::BoolV(mask));
	return maskbits ^ Vec4V::BoolV::MaskAll;
}

uint32 Triangle3V::IntersectsRay_intrinsic(__m128 origin,__m128 dir_x,__m128 dir_y,__m128 dir_z,__m128& t) const
{
	const __m128 one = _mm_set1_ps(1.0f);
	const __m128 two = _mm_set1_ps(2.0f);
	const __m128 eps = _mm_set1_ps(TRIANGLE_RAY_EPSILON);
	const __m128 v0 = _mm_load_ps(reinterpret_cast<const float*>(&m_positions[0]));
	const __m128 v1 = _mm_load_ps(reinterpret_cast<const float*>(&m_positions[1]));
	const __m128 v2 = _mm_load_ps(reinterpret_cast<const float*>(&m_positions[2]));
	const __m128 v0v1 = _mm_sub_ps(v1,v0);
	const __m128 v0v2 = _mm_sub_ps(v2,v0);
	const __m128 v0v1_x = _vmath_splatx_ps(v0v1);
	const __m128 v0v1_y = _vmath_splaty_ps(v0v1);
	const __m128 v0v1_z = _vmath_splatz_ps(v0v1);
	const __m128 v0v2_x = _vmath_splatx_ps(v0v2);
	const __m128 v0v2_y = _vmath_splaty_ps(v0v2);
	const __m128 v0v2_z = _vmath_splatz_ps(v0v2);
	const __m128 pvec_x = _mm_fmsub_ps(dir_y,v0v2_z,_mm_mul_ps(dir_z,v0v2_y)); // pvec = Cross(dir,v0v2)
	const __m128 pvec_y = _mm_fmsub_ps(dir_z,v0v2_x,_mm_mul_ps(dir_x,v0v2_z));
	const __m128 pvec_z = _mm_fmsub_ps(dir_x,v0v2_y,_mm_mul_ps(dir_y,v0v2_x));
	const __m128 det = _mm_fmadd_ps(v0v1_x,pvec_x,_mm_fmadd_ps(v0v1_y,pvec_y,_mm_mul_ps(v0v1_z,pvec_z))); // det = Dot(v0v1,pvec)
	__m128 mask = _mm_sub_ps(det,eps);
	EARLY_OUT4(mask);
	__m128 invDet = _mm_rcp_ps(det);
	invDet = _mm_mul_ps(invDet,_mm_fnmadd_ps(det,invDet,two)); // est <- est*(2 - a*est)
	const __m128 tvec = _mm_sub_ps(origin,v0);
	const __m128 tvec_x = _vmath_splatx_ps(tvec);
	const __m128 tvec_y = _vmath_splaty_ps(tvec);
	const __m128 tvec_z = _vmath_splatz_ps(tvec);
	const __m128 u = _mm_mul_ps(_mm_fmadd_ps(tvec_x,pvec_x,_mm_fmadd_ps(tvec_y,pvec_y,_mm_mul_ps(tvec_z,pvec_z))),invDet); // u = Dot(tvec,pvec)*invDet
	mask = _mm_or_ps(mask,u);
	EARLY_OUT4(mask);
	const __m128 tvec_yzxw = _vmath_permute_ps<1,2,0,3>(tvec); // tvec.yzxw
	const __m128 v0v1_yzxw = _vmath_permute_ps<1,2,0,3>(v0v1); // v0v1.yzxw
	const __m128 temp = _mm_fnmadd_ps(v0v1,tvec_yzxw,_mm_mul_ps(tvec,v0v1_yzxw)); // tvec.xyzw*v0v1.yzxw - v0v1.xyzw*tvec.yzxw
	const __m128 qvec = _vmath_permute_ps<1,2,0,3>(temp); // qvec = tvec.yzxw*v0v1.zxyw - v0v1.yzxw*tvec.zxyw = Cross(tvec,v0v1)
	const __m128 qvec_x = _vmath_splatx_ps(qvec);
	const __m128 qvec_y = _vmath_splaty_ps(qvec);
	const __m128 qvec_z = _vmath_splatz_ps(qvec);
	const __m128 v = _mm_mul_ps(_mm_fmadd_ps(dir_x,qvec_x,_mm_fmadd_ps(dir_y,qvec_y,_mm_mul_ps(dir_z,qvec_z))),invDet); // v = Dot(dir,qvec)*invDet
	mask = _mm_or_ps(mask,_mm_min_ps(v,_mm_sub_ps(_mm_sub_ps(one,u),v))); // mask |= min(v,1 - u - v)
	EARLY_OUT4(mask);
	t = _mm_mul_ps(_mm_dp_ps(v0v2,qvec,0x7F),invDet); // t = Dot(v0v2,qvec)*invDet
	mask = _mm_or_ps(mask,t);
	EARLY_OUT4_FINAL(mask,maskbits);
	t = _mm_blendv_ps(t,_mm_set1_ps(ZBUFFER_DEFAULT),mask);
	return maskbits ^ Vec4V::BoolV::MaskAll;
}

#if HAS_VEC8V
uint32 Triangle3V::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin,Vec3V_SOA8_arg dir,Vec8V& t,uint32,bool twosided,float epsilon) const
{
	const Vec3V v0v1 = m_positions[1] - m_positions[0];
	const Vec3V v0v2 = m_positions[2] - m_positions[0];
	const Vec3V_SOA8 pvec = Cross(dir,v0v2);
	const Vec8V det = Dot(v0v1,pvec);
	Vec8V test = det;
	if (twosided)
		test = Abs(test);
	Vec8V mask = test - Vec8V(epsilon);
	EARLY_OUT8(mask);
	const Vec8V invDet = Recip(det);
	const auto tvec = origin - m_positions[0];
	const Vec8V u = Dot(tvec,pvec)*invDet;
	mask |= u;
	EARLY_OUT8(mask);
	const auto qvec = Cross(tvec,v0v1);
	const Vec8V v = Dot(dir,qvec)*invDet;
	mask |= Min(v,Vec8V(V_ONE) - u - v);
	EARLY_OUT8(mask);
	t = Dot(v0v2,qvec)*invDet;
	mask |= t;
	EARLY_OUT8_FINAL(mask,maskbits);
	t = Select(t,Vec8V(ZBUFFER_DEFAULT),Vec8V::BoolV(mask));
	return maskbits ^ Vec8V::BoolV::MaskAll;
}

uint32 Triangle3V::IntersectsRay_intrinsic(__m128 origin,__m256 dir_x,__m256 dir_y,__m256 dir_z,__m256& t) const
{
	const __m256 one = _mm256_set1_ps(1.0f);
	const __m256 two = _mm256_set1_ps(2.0f);
	const __m256 eps = _mm256_set1_ps(TRIANGLE_RAY_EPSILON);
	const __m128 v0 = _mm_load_ps(reinterpret_cast<const float*>(&m_positions[0]));
	const __m128 v1 = _mm_load_ps(reinterpret_cast<const float*>(&m_positions[1]));
	const __m128 v2 = _mm_load_ps(reinterpret_cast<const float*>(&m_positions[2]));
	const __m128 v0v1 = _mm_sub_ps(v1,v0);
	const __m128 v0v2 = _mm_sub_ps(v2,v0);
	const __m256 v0v1_x = _vmath256_splatx_128_ps(v0v1);
	const __m256 v0v1_y = _vmath256_splaty_128_ps(v0v1);
	const __m256 v0v1_z = _vmath256_splatz_128_ps(v0v1);
	const __m256 v0v2_x = _vmath256_splatx_128_ps(v0v2);
	const __m256 v0v2_y = _vmath256_splaty_128_ps(v0v2);
	const __m256 v0v2_z = _vmath256_splatz_128_ps(v0v2);
	const __m256 pvec_x = _mm256_fmsub_ps(dir_y,v0v2_z,_mm256_mul_ps(dir_z,v0v2_y)); // pvec = Cross(dir,v0v2)
	const __m256 pvec_y = _mm256_fmsub_ps(dir_z,v0v2_x,_mm256_mul_ps(dir_x,v0v2_z));
	const __m256 pvec_z = _mm256_fmsub_ps(dir_x,v0v2_y,_mm256_mul_ps(dir_y,v0v2_x));
	const __m256 det = _mm256_fmadd_ps(v0v1_x,pvec_x,_mm256_fmadd_ps(v0v1_y,pvec_y,_mm256_mul_ps(v0v1_z,pvec_z))); // det = Dot(v0v1,pvec)
	__m256 mask = _mm256_sub_ps(det,eps);
	EARLY_OUT8(mask);
	__m256 invDet = _mm256_rcp_ps(det);
	invDet = _mm256_mul_ps(invDet,_mm256_fnmadd_ps(det,invDet,two)); // est <- est*(2 - a*est)
	const __m128 tvec = _mm_sub_ps(origin,v0);
	const __m256 tvec_x = _vmath256_splatx_128_ps(tvec);
	const __m256 tvec_y = _vmath256_splaty_128_ps(tvec);
	const __m256 tvec_z = _vmath256_splatz_128_ps(tvec);
	const __m256 u = _mm256_mul_ps(_mm256_fmadd_ps(tvec_x,pvec_x,_mm256_fmadd_ps(tvec_y,pvec_y,_mm256_mul_ps(tvec_z,pvec_z))),invDet); // u = Dot(tvec,pvec)*invDet
	mask = _mm256_or_ps(mask,u);
	EARLY_OUT8(mask);
	const __m128 tvec_yzxw = _vmath_permute_ps<1,2,0,3>(tvec); // tvec.yzxw
	const __m128 v0v1_yzxw = _vmath_permute_ps<1,2,0,3>(v0v1); // v0v1.yzxw
	const __m128 temp = _mm_fnmadd_ps(v0v1,tvec_yzxw,_mm_mul_ps(tvec,v0v1_yzxw)); // tvec.xyzw*v0v1.yzxw - v0v1.xyzw*tvec.yzxw
	const __m128 qvec = _vmath_permute_ps<1,2,0,3>(temp); // qvec = tvec.yzxw*v0v1.zxyw - v0v1.yzxw*tvec.zxyw = Cross(tvec,v0v1)
	const __m256 qvec_x = _vmath256_splatx_128_ps(qvec);
	const __m256 qvec_y = _vmath256_splaty_128_ps(qvec);
	const __m256 qvec_z = _vmath256_splatz_128_ps(qvec);
	const __m256 v = _mm256_mul_ps(_mm256_fmadd_ps(dir_x,qvec_x,_mm256_fmadd_ps(dir_y,qvec_y,_mm256_mul_ps(dir_z,qvec_z))),invDet); // v = Dot(dir,qvec)*invDet
	mask = _mm256_or_ps(mask,_mm256_min_ps(v,_mm256_sub_ps(_mm256_sub_ps(one,u),v))); // mask |= min(v,1 - u - v)
	EARLY_OUT8(mask);
	t = _mm256_mul_ps(_vmath256_splatx_128_ps(_mm_dp_ps(v0v2,qvec,0x7F)),invDet); // t = Dot(v0v2,qvec)*invDet
	mask = _mm256_or_ps(mask,t);
	EARLY_OUT8_FINAL(mask,maskbits);
	t = _mm256_blendv_ps(t,_mm256_set1_ps(ZBUFFER_DEFAULT),mask);
	return maskbits ^ Vec8V::BoolV::MaskAll;
}

#if HAS_VEC16V
uint32 Triangle3V::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA16_arg origin,Vec3V_SOA16_arg dir,Vec16V& t,uint32,bool twosided,float epsilon) const
{
	const Vec3V v0v1 = m_positions[1] - m_positions[0];
	const Vec3V v0v2 = m_positions[2] - m_positions[0];
	const Vec3V_SOA16 pvec = Cross(dir,v0v2);
	const Vec16V det = Dot(v0v1,pvec);
	Vec16V test = det;
	if (twosided)
		test = Abs(test);
	Vec16V mask = test - Vec16V(epsilon);
	EARLY_OUT16(mask);
	const Vec16V invDet = Recip(det);
	const auto tvec = origin - m_positions[0];
	const Vec16V u = Dot(tvec,pvec)*invDet;
	mask |= u;
	EARLY_OUT16(mask);
	const auto qvec = Cross(tvec,v0v1);
	const Vec16V v = Dot(dir,qvec)*invDet;
	mask |= Min(v,Vec16V::One() - u - v);
	EARLY_OUT16(mask);
	t = Dot(v0v2,qvec)*invDet;
	mask |= t;
	EARLY_OUT16_FINAL(mask,maskbits);
	t = Vec16V(_mm512_mask_blend_ps(maskbits,_mm512_set1_ps(ZBUFFER_DEFAULT),t));
	return maskbits ^ 0xFFFF;
}
#endif // HAS_VEC16V
#endif // HAS_VEC8V

Triangle3V_SOA4::Triangle3V_SOA4(const Triangle3V tris[NumTriangles])
{
	m_positions[0] = T(tris[0].m_positions[0],tris[1].m_positions[0],tris[2].m_positions[0],tris[3].m_positions[0]);
	m_positions[1] = T(tris[0].m_positions[1],tris[1].m_positions[1],tris[2].m_positions[1],tris[3].m_positions[1]);
	m_positions[2] = T(tris[0].m_positions[2],tris[1].m_positions[2],tris[2].m_positions[2],tris[3].m_positions[2]);
}

uint32 Triangle3V_SOA4::IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t_,bool twosided,float epsilon) const
{
	const Vec3V_SOA4 v0v1 = m_positions[1] - m_positions[0];
	const Vec3V_SOA4 v0v2 = m_positions[2] - m_positions[0];
	const Vec3V_SOA4 pvec = Cross(dir,v0v2);
	const Vec4V det = Dot(v0v1,pvec);
	Vec4V test = det;
	if (twosided)
		test = Abs(test);
	Vec4V mask = test - Vec4V(epsilon);
	EARLY_OUT4(mask);
	const Vec4V invDet = Recip(det);
	const Vec3V_SOA4 tvec = origin - m_positions[0];
	const Vec4V u = Dot(tvec,pvec)*invDet;
	mask |= u;
	EARLY_OUT4(mask);
	const Vec3V_SOA4 qvec = Cross(tvec,v0v1);
	const Vec4V v = Dot(dir,qvec)*invDet;
	mask |= Min(v,Vec4V(V_ONE) - u - v);
	EARLY_OUT4(mask);
	const Vec4V t = Dot(v0v2,qvec)*invDet;
	mask |= t;
	EARLY_OUT4(mask);
	t_ = MinElement(Select(t,Vec4V(ZBUFFER_DEFAULT),Vec4V::BoolV(mask)));
	return 1;
}

uint32 Triangle3V_SOA4::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin_,Vec3V_SOA4_arg dir_,Vec4V& t_,uint32 mask,bool twosided,float epsilon) const
{
	// TODO -- we don't actually need to call GetVectors (which does a transpose), we should be able to extract the Nth triangle broadcast as a Vec3V_SOA4 directly
	// actually i'm not sure how we should be doing this - looping over the rays or looping over the triangles
	uint32 bit = 1;
	RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(Vec3V origins[4]; origin_.GetVectors(origins));
	Vec3V dirs[4]; dir_.GetVectors(dirs);
	for (unsigned i = 0; mask; i++, bit <<= 1) {
		DEBUG_ASSERT(i < countof(dirs));
		if (mask & bit) {
			ScalarV t;
			if (IntersectsRay(RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(origins[i],origin_),dirs[i],t,twosided,epsilon))
				t_[i] = t.f();
			else {
				t_[i] = ZBUFFER_DEFAULT;
				mask ^= bit;
			}
		}
	}
	return mask;
}

#if HAS_VEC8V
uint32 Triangle3V_SOA4::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin_,Vec3V_SOA8_arg dir_,Vec8V& t_,uint32 mask,bool twosided,float epsilon) const
{
	uint32 bit = 1;
	RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(Vec3V origins[8]; origin_.GetVectors(origins));
	Vec3V dirs[8]; dir_.GetVectors(dirs);
	for (unsigned i = 0; mask; i++, bit <<= 1) {
		DEBUG_ASSERT(i < countof(dirs));
		if (mask & bit) {
			ScalarV t;
			if (IntersectsRay(RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(origins[i],origin_),dirs[i],t,twosided,epsilon))
				t_[i] = t.f();
			else {
				t_[i] = ZBUFFER_DEFAULT;
				mask ^= bit;
			}
		}
	}
	return mask;
}

#if HAS_VEC16V
uint32 Triangle3V_SOA4::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA16_arg origin_,Vec3V_SOA16_arg dir_,Vec16V& t_,uint32 mask,bool twosided,float epsilon) const
{
	uint32 bit = 1;
	RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(Vec3V origins[16]; origin_.GetVectors(origins));
	Vec3V dirs[16]; dir_.GetVectors(dirs);
	for (unsigned i = 0; mask; i++, bit <<= 1) {
		DEBUG_ASSERT(i < countof(dirs));
		if (mask & bit) {
			ScalarV t;
			if (IntersectsRay(RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(origins[i],origin_),dirs[i],t,twosided,epsilon))
				t_[i] = t.f();
			else {
				t_[i] = ZBUFFER_DEFAULT;
				mask ^= bit;
			}
		}
	}
	return mask;
}
#endif // HAS_VEC16V
#endif // HAS_VEC8V

#if HAS_VEC8V
Triangle3V_SOA8::Triangle3V_SOA8(const Triangle3V tris[NumTriangles])
{
	m_positions[0] = T(tris[0].m_positions[0],tris[1].m_positions[0],tris[2].m_positions[0],tris[3].m_positions[0],tris[4].m_positions[0],tris[5].m_positions[0],tris[6].m_positions[0],tris[7].m_positions[0]);
	m_positions[1] = T(tris[0].m_positions[1],tris[1].m_positions[1],tris[2].m_positions[1],tris[3].m_positions[1],tris[4].m_positions[1],tris[5].m_positions[1],tris[6].m_positions[1],tris[7].m_positions[1]);
	m_positions[2] = T(tris[0].m_positions[2],tris[1].m_positions[2],tris[2].m_positions[2],tris[3].m_positions[2],tris[4].m_positions[2],tris[5].m_positions[2],tris[6].m_positions[2],tris[7].m_positions[2]);
}

uint32 Triangle3V_SOA8::IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t_,bool twosided,float epsilon) const
{
	const Vec3V_SOA8 v0v1 = m_positions[1] - m_positions[0];
	const Vec3V_SOA8 v0v2 = m_positions[2] - m_positions[0];
	const Vec3V_SOA8 pvec = Cross(dir,v0v2);
	const Vec8V det = Dot(v0v1,pvec);
	Vec8V test = det;
	if (twosided)
		test = Abs(test);
	Vec8V mask = test - Vec8V(epsilon);
	EARLY_OUT8(mask);
	const Vec8V invDet = Recip(det);
	const Vec3V_SOA8 tvec = origin - m_positions[0];
	const Vec8V u = Dot(tvec,pvec)*invDet;
	mask |= u;
	EARLY_OUT8(mask);
	const Vec3V_SOA8 qvec = Cross(tvec,v0v1);
	const Vec8V v = Dot(dir,qvec)*invDet;
	mask |= Min(v,Vec8V(V_ONE) - u - v);
	EARLY_OUT8(mask);
	const Vec8V t = Dot(v0v2,qvec)*invDet;
	mask |= t;
	EARLY_OUT8_FINAL(mask,maskbits);
	t_ = MinElement(Select(t,Vec8V(ZBUFFER_DEFAULT),Vec8V::BoolV(mask)));
	return 1;
}

uint32 Triangle3V_SOA8::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin_,Vec3V_SOA4_arg dir_,Vec4V& t_,uint32 mask,bool twosided,float epsilon) const
{
	uint32 bit = 1;
	RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(Vec3V origins[4]; origin_.GetVectors(origins));
	Vec3V dirs[4]; dir_.GetVectors(dirs);
	for (unsigned i = 0; mask; i++, bit <<= 1) {
		DEBUG_ASSERT(i < countof(dirs));
		if (mask & bit) {
			ScalarV t;
			if (IntersectsRay(RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(origins[i],origin_),dirs[i],t,twosided,epsilon))
				t_[i] = t.f();
			else {
				t_[i] = ZBUFFER_DEFAULT;
				mask ^= bit;
			}
		}
	}
	return mask;
}

uint32 Triangle3V_SOA8::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin_,Vec3V_SOA8_arg dir_,Vec8V& t_,uint32 mask,bool twosided,float epsilon) const
{
	uint32 bit = 1;
	RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(Vec3V origins[8]; origin_.GetVectors(origins));
	Vec3V dirs[8]; dir_.GetVectors(dirs);
	for (unsigned i = 0; mask; i++, bit <<= 1) {
		DEBUG_ASSERT(i < countof(dirs));
		if (mask & bit) {
			ScalarV t;
			if (IntersectsRay(RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(origins[i],origin_),dirs[i],t,twosided,epsilon))
				t_[i] = t.f();
			else {
				t_[i] = ZBUFFER_DEFAULT;
				mask ^= bit;
			}
		}
	}
	return mask;
}

#if HAS_VEC16V
uint32 Triangle3V_SOA8::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA16_arg origin_,Vec3V_SOA16_arg dir_,Vec16V& t_,uint32 mask,bool twosided,float epsilon) const
{
	uint32 bit = 1;
	RAY_PACKET_INDEPENDENT_ORIGINS_ONLY(Vec3V origins[16]; origin_.GetVectors(origins));
	Vec3V dirs[16]; dir_.GetVectors(dirs);
	for (unsigned i = 0; mask; i++, bit <<= 1) {
		DEBUG_ASSERT(i < countof(dirs));
		if (mask & bit) {
			ScalarV t;
			if (IntersectsRay(RAY_PACKET_INDEPENDENT_ORIGINS_SWITCH(origins[i],origin_),dirs[i],t,twosided,epsilon))
				t_[i] = t.f();
			else {
				t_[i] = ZBUFFER_DEFAULT;
				mask ^= bit;
			}
		}
	}
	return mask;
}
#endif // HAS_VEC16V
#endif // HAS_VEC8V

#undef EARLY_OUT4
#undef EARLY_OUT4_FINAL
#undef EARLY_OUT8
#undef EARLY_OUT8_FINAL
#undef EARLY_OUT16
#undef EARLY_OUT16_FINAL
