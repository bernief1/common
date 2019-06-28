// ===========================
// common/vmath/vmath_matrix.h
// ===========================

#ifndef _INCLUDE_VMATH_MATRIX_H_
#define _INCLUDE_VMATH_MATRIX_H_

#include "vmath_common.h"
#include "vmath_floatvec.h"
#include "vmath_vec4.h"

class Box3V;
class Plane3V;
class Sphere3V;

VMATH_INLINE void Mat44VTransposeInternal(__m128& dst0,__m128& dst1,__m128& dst2,__m128& dst3,__m128 src0,__m128 src1,__m128 src2,__m128 src3)
{
	const __m128 tmp0 = _vmath_permute_AABB_ps<0,1,0,1>(src0,src1);
	const __m128 tmp2 = _vmath_permute_AABB_ps<2,3,2,3>(src0,src1);
	const __m128 tmp1 = _vmath_permute_AABB_ps<0,1,0,1>(src2,src3);
	const __m128 tmp3 = _vmath_permute_AABB_ps<2,3,2,3>(src2,src3);
	dst0 = _vmath_permute_AABB_ps<0,2,0,2>(tmp0,tmp1);
	dst1 = _vmath_permute_AABB_ps<1,3,1,3>(tmp0,tmp1);
	dst2 = _vmath_permute_AABB_ps<0,2,0,2>(tmp2,tmp3);
	dst3 = _vmath_permute_AABB_ps<1,3,1,3>(tmp2,tmp3);
}

VMATH_INLINE void Mat44VTranspose(Vec4V& dst0,Vec4V& dst1,Vec4V& dst2,Vec4V& dst3,Vec4V_arg src0,Vec4V_arg src1,Vec4V_arg src2,Vec4V_arg src3)
{
	Mat44VTransposeInternal(dst0.v_ref(),dst1.v_ref(),dst2.v_ref(),dst3.v_ref(),src0,src1,src2,src3);
}

VMATH_INLINE void Mat34VTranspose(Vec4V& dst0,Vec4V& dst1,Vec4V& dst2,Vec3V_arg src0,Vec3V_arg src1,Vec3V_arg src2,Vec3V_arg src3)
{
	__m128 dummy;
	Mat44VTransposeInternal(dst0.v_ref(),dst1.v_ref(),dst2.v_ref(),dummy,src0,src1,src2,src3);
}

VMATH_INLINE void Mat43VTranspose(Vec3V& dst0,Vec3V& dst1,Vec3V& dst2,Vec3V& dst3,Vec4V_arg src0,Vec4V_arg src1,Vec4V_arg src2)
{
	Mat44VTransposeInternal(dst0.v_ref(),dst1.v_ref(),dst2.v_ref(),dst3.v_ref(),src0,src1,src2,src2);
}

VMATH_INLINE void Mat33VTranspose(Vec3V& dst0,Vec3V& dst1,Vec3V& dst2,Vec3V_arg src0,Vec3V_arg src1,Vec3V_arg src2)
{
	__m128 dummy;
	Mat44VTransposeInternal(dst0.v_ref(),dst1.v_ref(),dst2.v_ref(),dummy,src0,src1,src2,src2);
}

class Box3V;
class Plane3V;
class Sphere3V;

class Mat33V // rotation, scale, skew (no translation)
{
public:
	typedef Mat33V T;
	typedef const T& ArgType;
	typedef const T OutType;

	VMATH_INLINE Mat33V() {}
	VMATH_INLINE Mat33V(Vec3V_arg a,Vec3V_arg b,Vec3V_arg c): m_a(a),m_b(b),m_c(c) {}
	VMATH_INLINE explicit Mat33V(VectorConstantInitializer k): m_a(k),m_b(k),m_c(k) {}

	VMATH_INLINE static OutType Identity() { return T(Vec3V(V_XAXIS),Vec3V(V_YAXIS),Vec3V(V_ZAXIS)); }
	static const T& StaticIdentity();

	VMATH_INLINE static OutType Scale(Vec3V_arg scale) { return T(Vec3V(V_XAXIS)*scale,Vec3V(V_YAXIS)*scale,Vec3V(V_ZAXIS)*scale); }
	VMATH_INLINE static OutType Scale(ScalarV_arg scale) { return Identity()*scale; }
	VMATH_INLINE static OutType Scale(float scale) { return Scale(ScalarV(scale)); }

	VMATH_INLINE static OutType RotationX(float theta)
	{
		const float c = Cos(theta);
		const float s = Sin(theta);
		return Mat33V(
			Vec3V(V_XAXIS),
			Vec3V(0.0f, +c, -s),
			Vec3V(0.0f, +s, +c));
	}

	VMATH_INLINE static OutType RotationY(float theta)
	{
		const float c = Cos(theta);
		const float s = Sin(theta);
		return Mat33V(
			Vec3V(+c, 0.0f, -s),
			Vec3V(V_YAXIS),
			Vec3V(+s, 0.0f, +c));
	}

	VMATH_INLINE static OutType RotationZ(float theta)
	{
		const float c = Cos(theta);
		const float s = Sin(theta);
		return Mat33V(
			Vec3V(+c, -s, 0.0f),
			Vec3V(+s, +c, 0.0f),
			Vec3V(V_ZAXIS));
	}

	VMATH_INLINE Vec3V_out a() const { return m_a; }
	VMATH_INLINE Vec3V_out b() const { return m_b; }
	VMATH_INLINE Vec3V_out c() const { return m_c; }

	VMATH_INLINE Vec3V& a_ref() { return m_a; }
	VMATH_INLINE Vec3V& b_ref() { return m_b; }
	VMATH_INLINE Vec3V& c_ref() { return m_c; }

	VMATH_INLINE const Vec3V& a_constref() const { return m_a; }
	VMATH_INLINE const Vec3V& b_constref() const { return m_b; }
	VMATH_INLINE const Vec3V& c_constref() const { return m_c; }

	VMATH_INLINE static OutType Load(const T* m)
	{
		return T(
			Vec3V::Load(reinterpret_cast<const float*>(m) + 0*4),
			Vec3V::Load(reinterpret_cast<const float*>(m) + 1*4),
			Vec3V::Load(reinterpret_cast<const float*>(m) + 2*4));
	}

	VMATH_INLINE static OutType LoadUnaligned(const T* m)
	{
		return Load(m); // Vec3V::Load is already using _mm_loadu_ps
	}

	VMATH_INLINE ScalarV_out Det() const { return Dot(m_a,Cross(m_b,m_c)); }

	// full matrix inversion
	VMATH_INLINE friend OutType InvertAffine(ArgType m)
	{
		const Vec3V ab = Cross(m.m_a,m.m_b);
		const Vec3V bc = Cross(m.m_b,m.m_c);
		const Vec3V ca = Cross(m.m_c,m.m_a);
		return Transpose(Mat33V(bc,ca,ab)/Dot(m.m_a,bc));
	}

	// Transpose(m) is equivalent to full matrix inversion only if the matrix is orthonormal
	VMATH_INLINE friend OutType Transpose(ArgType m)
	{
		Vec3V a,b,c;
		Mat33VTranspose(a,b,c,m.m_a,m.m_b,m.m_c);
		return Mat33V(a,b,c);
	}

	// m.Transform(v) transforms vector v through matrix m
	VMATH_INLINE Vec3V_out Transform(Vec3V_arg v) const { return MultiplyAdd(m_a,v.x(),m_b,v.y(),m_c,v.z()); }

	// m.TransformInvertAffine(v) is equivalent to InvertAffine(m).Transform(v)
	VMATH_INLINE Vec3V_out TransformInvertAffine(Vec3V_arg v) const
	{
		const Vec3V ab = Cross(m_a,m_b);
		const Vec3V bc = Cross(m_b,m_c);
		const Vec3V ca = Cross(m_c,m_a);
		return Mat33V(bc,ca,ab).TransformTranspose(v)/Dot(m_a,bc);
	}

	// m.TransformTranspose(v) is equivalent to Transpose(m).Transform(v)
	VMATH_INLINE Vec3V_out TransformTranspose(Vec3V_arg v) const { return Vec3V(Dot(m_a,v),Dot(m_b,v),Dot(m_c,v)); }

	// Multiply(m1,m2) is standard matrix multiplication
	VMATH_INLINE friend OutType Multiply(ArgType m1,ArgType m2)
	{
		return Mat33V(
			MultiplyAdd(
				m1.m_a,m2.m_a.x(),
				m1.m_b,m2.m_a.y(),
				m1.m_c,m2.m_a.z()),
			MultiplyAdd(
				m1.m_a,m2.m_b.x(),
				m1.m_b,m2.m_b.y(),
				m1.m_c,m2.m_b.z()),
			MultiplyAdd(
				m1.m_a,m2.m_c.x(),
				m1.m_b,m2.m_c.y(),
				m1.m_c,m2.m_c.z()));
	}

	// MultiplyInvertAffine(m1,m2) is equivalent to Multiply(m1,InvertAffine(m2))
	VMATH_INLINE friend OutType MultiplyInvertAffine(ArgType m1,ArgType m2)
	{
		const Vec3V ab = Cross(m2.m_a,m2.m_b);
		const Vec3V bc = Cross(m2.m_b,m2.m_c);
		const Vec3V ca = Cross(m2.m_c,m2.m_a);
		return MultiplyTranspose(m1,Mat33V(bc,ca,ab))/Dot(m2.m_a,bc);
	}

	// MultiplyTranspose(m1,m2) is equivalent to Multiply(m1,Transpose(m2))
	VMATH_INLINE friend OutType MultiplyTranspose(ArgType m1,ArgType m2)
	{
		return Mat33V(
			MultiplyAdd(
				m1.m_a,m2.m_a.x(),
				m1.m_b,m2.m_b.x(),
				m1.m_c,m2.m_c.x()),
			MultiplyAdd(
				m1.m_a,m2.m_a.y(),
				m1.m_b,m2.m_b.y(),
				m1.m_c,m2.m_c.y()),
			MultiplyAdd(
				m1.m_a,m2.m_a.z(),
				m1.m_b,m2.m_b.z(),
				m1.m_c,m2.m_c.z()));
	}

	// multiplication operators: m1*m2*v == Multiply(m1,m2).Transform(v) == m1.Transform(m2.Transform(v))
	VMATH_INLINE friend Vec3V_out operator *(ArgType m,Vec3V_arg v) { return m.Transform(v); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return Multiply(a,b); }

	VMATH_INLINE friend OutType operator +(ArgType m1,ArgType m2) { return T(m1.m_a + m2.m_a,m1.m_b + m2.m_b,m1.m_c + m2.m_c); }
	VMATH_INLINE friend OutType operator -(ArgType m1,ArgType m2) { return T(m1.m_a - m2.m_a,m1.m_b - m2.m_b,m1.m_c - m2.m_c); }
	VMATH_INLINE friend OutType operator *(ArgType m,ScalarV_arg s) { return T(m.m_a*s,m.m_b*s,m.m_c*s); }
	VMATH_INLINE friend OutType operator /(ArgType m,ScalarV_arg s) { return m*Recip(s); }
	VMATH_INLINE friend OutType operator *(ScalarV_arg s,ArgType m) { return T(m.m_a*s,m.m_b*s,m.m_c*s); }
	VMATH_INLINE friend OutType operator *(ArgType m,float f) { return m*ScalarV(f); }
	VMATH_INLINE friend OutType operator /(ArgType m,float f) { return m/ScalarV(f); }
	VMATH_INLINE friend OutType operator *(float f,ArgType m) { return m*ScalarV(f); }

	VMATH_INLINE friend float MaxDiff(ArgType a,ArgType b) { return MaxElement(Max(Abs(a.m_a - b.m_a),Abs(a.m_b - b.m_b),Abs(a.m_c - b.m_c))).f(); }
	VMATH_INLINE float GetMaxScaleDiff() const { return MaxElement(Abs(Vec3V(MagSqr(m_a),MagSqr(m_b),MagSqr(m_c)) - Vec3V(V_ONE))).f(); }
	VMATH_INLINE float GetMaxSkew() const { return MaxElement(Abs(Vec3V(Dot(m_a,m_b),Dot(m_b,m_c),Dot(m_c,m_a)))).f(); }
	VMATH_INLINE bool IsOrtho(float scaleEpsilon=0.0001f,float skewEpsilon=0.0001f) const { return GetMaxScaleDiff() <= scaleEpsilon && GetMaxSkew() <= skewEpsilon; }

	static OutType ConstructBasis(Vec3V_arg forwardDir); // forwardDir is matrix.c
	static OutType ConstructBasisUp(Vec3V_arg forwardDir,Vec3V_arg upDir = Vec3V(V_YAXIS)); // forwardDir is matrix.c, upDir is in the plane defined by matrix.c and matrix.b

	const Box3V TransformBox(const Box3V& box) const;
	const Plane3V TransformPlane(const Plane3V& plane) const;
	const Sphere3V TransformSphere(const Sphere3V& sphere) const;

private:
	friend class Mat34V;
	friend class Mat44V;
	Vec3V m_a;
	Vec3V m_b;
	Vec3V m_c;
};

typedef Mat33V::ArgType Mat33V_arg;
typedef Mat33V::OutType Mat33V_out;

class Mat34V // affine only (no projection)
{
public:
	typedef Mat34V T;
	typedef const T& ArgType;
	typedef const T OutType;

	VMATH_INLINE Mat34V() {}
	VMATH_INLINE Mat34V(Mat33V::ArgType m,Vec3V_arg d): m_mat33(m),m_d(d) {}
	VMATH_INLINE Mat34V(Vec3V_arg a,Vec3V_arg b,Vec3V_arg c,Vec3V_arg d): m_mat33(a,b,c),m_d(d) {}
	VMATH_INLINE explicit Mat34V(Mat33V_arg m): m_mat33(m),m_d(V_ZERO) {}
	VMATH_INLINE explicit Mat34V(VectorConstantInitializer k): m_mat33(k),m_d(k) {}

	VMATH_INLINE static OutType Identity() { return T(Mat33V::Identity(),Vec3V(V_ZERO)); }
	static const T& StaticIdentity();

	VMATH_INLINE static OutType Scale(Vec3V_arg scale) { return T(Mat33V::Scale(scale), Vec3V(V_ZERO)); }
	VMATH_INLINE static OutType Scale(ScalarV_arg scale) { return T(Mat33V::Scale(scale), Vec3V(V_ZERO)); }
	VMATH_INLINE static OutType Scale(float scale) { return Scale(ScalarV(scale)); }
	VMATH_INLINE static OutType Translation(Vec3V_arg translation) { return T(Mat33V::Identity(), translation); }
	
	VMATH_INLINE const Vec3V_out a() const { return m_mat33.m_a; }
	VMATH_INLINE const Vec3V_out b() const { return m_mat33.m_b; }
	VMATH_INLINE const Vec3V_out c() const { return m_mat33.m_c; }
	VMATH_INLINE const Vec3V_out d() const { return m_d; }

	VMATH_INLINE Vec3V& a_ref() { return m_mat33.a_ref(); }
	VMATH_INLINE Vec3V& b_ref() { return m_mat33.b_ref(); }
	VMATH_INLINE Vec3V& c_ref() { return m_mat33.c_ref(); }
	VMATH_INLINE Vec3V& d_ref() { return m_d; }

	VMATH_INLINE const Vec3V& a_constref() const { return m_mat33.a_constref(); }
	VMATH_INLINE const Vec3V& b_constref() const { return m_mat33.b_constref(); }
	VMATH_INLINE const Vec3V& c_constref() const { return m_mat33.c_constref(); }
	VMATH_INLINE const Vec3V& d_constref() const { return m_d; }

	VMATH_INLINE Mat33V_out GetMat33V() const { return m_mat33; }
	VMATH_INLINE Mat33V& GetMat33V_ref() { return m_mat33; }
	VMATH_INLINE const Mat33V& GetMat33V_constref() const { return m_mat33; }

	VMATH_INLINE static OutType Load(const T* m)
	{
		return T(
			Vec3V::Load(reinterpret_cast<const float*>(m) + 0*4),
			Vec3V::Load(reinterpret_cast<const float*>(m) + 1*4),
			Vec3V::Load(reinterpret_cast<const float*>(m) + 2*4),
			Vec3V::Load(reinterpret_cast<const float*>(m) + 3*4));
	}

	VMATH_INLINE static OutType LoadUnaligned(const T* m)
	{
		return Load(m); // Vec3V::Load is already using _mm_loadu_ps
	}

	VMATH_INLINE friend OutType InvertAffine(ArgType m) { const Mat33V inv = InvertAffine(m.m_mat33); return Mat34V(inv,-inv.Transform(m.d())); }
	VMATH_INLINE friend OutType InvertOrtho(ArgType m) { const Mat33V inv = Transpose(m.m_mat33); return Mat34V(inv,-inv.Transform(m.d())); }
	VMATH_INLINE friend OutType InvertOrtho_ALT(ArgType m) { return Mat34V(Transpose(m.m_mat33),-m.m_mat33.TransformTranspose(m.d())); } // is this faster?

	// m.Transform(v) transforms vector v through matrix m
	VMATH_INLINE Vec3V_out Transform(Vec3V_arg v) const { return MultiplyAdd(
		m_mat33.m_a,v.x(),
		m_mat33.m_b,v.y(),
		m_mat33.m_c,v.z(),d()); }

	// m.TransformDir(v) treats v as a direction rather than a location - this is equivalent to m.GetMat33V().Transform(v)
	VMATH_INLINE Vec3V_out TransformDir(Vec3V_arg v) const { return m_mat33.Transform(v); }

	// m.TransformInvertOrtho(v) is equivalent to InvertOrtho(m).Transform(v)
	VMATH_INLINE Vec3V_out TransformInvertOrtho(Vec3V_arg v) const { return m_mat33.TransformTranspose(v - d()); }

	// m.TransformTransposeDir(v) is equivalent to Transpose(m.GetMat33V()).Transform(v) or m.GetMat33V().TransformTranspose(v)
	VMATH_INLINE Vec3V_out TransformTransposeDir(Vec3V_arg v) const { return m_mat33.TransformTranspose(v); }

	const Box3V TransformBox(const Box3V& box) const;
	const Plane3V TransformPlane(const Plane3V& plane) const;
	const Sphere3V TransformSphere(const Sphere3V& sphere) const;

	// Multiply(m1,m2) is standard matrix multiplication
	VMATH_INLINE friend OutType Multiply(ArgType m1,ArgType m2)
	{
		return Mat34V(
			MultiplyAdd(
				m1.a(),m2.a().x(),
				m1.b(),m2.a().y(),
				m1.c(),m2.a().z()),
			MultiplyAdd(
				m1.a(),m2.b().x(),
				m1.b(),m2.b().y(),
				m1.c(),m2.b().z()),
			MultiplyAdd(
				m1.a(),m2.c().x(),
				m1.b(),m2.c().y(),
				m1.c(),m2.c().z()),
			MultiplyAdd(
				m1.a(),m2.d().x(),
				m1.b(),m2.d().y(),
				m1.c(),m2.d().z(),m1.d()));
	}

	// MultiplyInvertOrtho(m1,m2) is equivalent to Multiply(m1,InvertOrtho(m2))
	VMATH_INLINE friend OutType MultiplyInvertOrtho(ArgType m1,ArgType m2)
	{
		const Vec3V neg_d = -m2.d();
		return Mat34V(
			MultiplyAdd(
				m1.a(),m2.a().x(),
				m1.b(),m2.b().x(),
				m1.c(),m2.c().x()),
			MultiplyAdd(
				m1.a(),m2.a().y(),
				m1.b(),m2.b().y(),
				m1.c(),m2.c().y()),
			MultiplyAdd(
				m1.a(),m2.a().z(),
				m1.b(),m2.b().z(),
				m1.c(),m2.c().z()),
			MultiplyAdd(
				m1.a(),Dot(m2.a(),neg_d),
				m1.b(),Dot(m2.b(),neg_d),
				m1.c(),Dot(m2.c(),neg_d),m1.d()));
	}

	// multiplication operators: m1*m2*v == Multiply(m1,m2).Transform(v) == m1.Transform(m2.Transform(v))
	VMATH_INLINE friend Vec3V_out operator *(ArgType m,Vec3V_arg v) { return m.Transform(v); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return Multiply(a,b); }
	VMATH_INLINE friend OutType operator *(Mat33V_arg a,ArgType b) { return Multiply(Mat34V(a),b); } // optimize!
	VMATH_INLINE friend OutType operator *(ArgType a,Mat33V_arg b) { return Multiply(a,Mat34V(b)); } // optimize!

	VMATH_INLINE friend OutType operator +(ArgType m1,ArgType m2) { return T(m1.a() + m2.a(),m1.b() + m2.b(),m1.c() + m2.c(),m1.d() + m2.d()); }
	VMATH_INLINE friend OutType operator -(ArgType m1,ArgType m2) { return T(m1.a() - m2.a(),m1.b() - m2.b(),m1.c() - m2.c(),m1.d() - m2.d()); }
	VMATH_INLINE friend OutType operator *(ArgType m,ScalarV_arg s) { return T(m.a()*s,m.b()*s,m.c()*s,m.d()*s); }
	VMATH_INLINE friend OutType operator /(ArgType m,ScalarV_arg s) { return m*Recip(s); }
	VMATH_INLINE friend OutType operator *(ScalarV_arg s,ArgType m) { return T(m.a()*s,m.b()*s,m.c()*s,m.d()*s); }
	VMATH_INLINE friend OutType operator *(ArgType m,float f) { return m*ScalarV(f); }
	VMATH_INLINE friend OutType operator /(ArgType m,float f) { return m/ScalarV(f); }
	VMATH_INLINE friend OutType operator *(float f,ArgType m) { return m*ScalarV(f); }

	VMATH_INLINE friend float MaxDiff(ArgType a,ArgType b) { return Max(MaxDiff(a.m_mat33,b.m_mat33),MaxElement(Abs(a.d() - b.d())).f()); }
	VMATH_INLINE float GetMaxScaleDiff() const { return m_mat33.GetMaxScaleDiff(); }
	VMATH_INLINE float GetMaxSkew() const { return m_mat33.GetMaxSkew(); }
	VMATH_INLINE bool IsOrtho(float scaleEpsilon=0.0001f,float skewEpsilon=0.0001f) const { return m_mat33.IsOrtho(scaleEpsilon,skewEpsilon); }

	VMATH_INLINE static OutType ConstructBasis(Vec3V_arg origin,Vec3V_arg forwardDir) { return T(Mat33V::ConstructBasis(forwardDir),origin); }
	VMATH_INLINE static OutType ConstructBasisUp(Vec3V_arg origin,Vec3V_arg forwardDir,Vec3V_arg upDir = Vec3V(V_YAXIS)) { return T(Mat33V::ConstructBasisUp(forwardDir,upDir),origin); }

private:
	friend class Mat44V;
	Mat33V m_mat33;
	Vec3V m_d; // translation
};

typedef Mat34V::ArgType Mat34V_arg;
typedef Mat34V::OutType Mat34V_out;

class Mat44V
{
public:
	typedef Mat44V T;
	typedef const T& ArgType;
	typedef const T OutType;

	VMATH_INLINE Mat44V() {}
	VMATH_INLINE Mat44V(Vec4V_arg a,Vec4V_arg b,Vec4V_arg c,Vec4V_arg d): m_a(a),m_b(b),m_c(c),m_d(d) {}
	VMATH_INLINE explicit Mat44V(Mat33V_arg m): m_a(Vec4V(m.m_a,0.0f)),m_b(Vec4V(m.m_b,0.0f)),m_c(Vec4V(m.m_c,0.0f)),m_d(Vec4V(V_WAXIS)) {}
	VMATH_INLINE explicit Mat44V(Mat34V_arg m): m_a(Vec4V(m.a(),0.0f)),m_b(Vec4V(m.b(),0.0f)),m_c(Vec4V(m.c(),0.0f)),m_d(Vec4V(m.d(),1.0f)) {}
	VMATH_INLINE explicit Mat44V(VectorConstantInitializer k): m_a(k),m_b(k),m_c(k),m_d(k) {}

	VMATH_INLINE static OutType Identity() { return T(Vec4V(V_XAXIS),Vec4V(V_YAXIS),Vec4V(V_ZAXIS),Vec4V(V_WAXIS)); }
	static const T& StaticIdentity();

	VMATH_INLINE Vec4V_out a() const { return m_a; }
	VMATH_INLINE Vec4V_out b() const { return m_b; }
	VMATH_INLINE Vec4V_out c() const { return m_c; }
	VMATH_INLINE Vec4V_out d() const { return m_d; }

	VMATH_INLINE Vec4V& a_ref() { return m_a; }
	VMATH_INLINE Vec4V& b_ref() { return m_b; }
	VMATH_INLINE Vec4V& c_ref() { return m_c; }
	VMATH_INLINE Vec4V& d_ref() { return m_d; }

	VMATH_INLINE const Vec4V& a_constref() const { return m_a; }
	VMATH_INLINE const Vec4V& b_constref() const { return m_b; }
	VMATH_INLINE const Vec4V& c_constref() const { return m_c; }
	VMATH_INLINE const Vec4V& d_constref() const { return m_d; }

	VMATH_INLINE Mat33V_out GetMat33V() const { return Mat33V(m_a.xyz(),m_b.xyz(),m_c.xyz()); }
	VMATH_INLINE Mat34V_out GetMat34V() const { return Mat34V(m_a.xyz(),m_b.xyz(),m_c.xyz(),m_d.xyz()); }

	VMATH_INLINE static OutType Load(const T* m)
	{
		return T(
			Vec4V::Load(reinterpret_cast<const Vec4V*>(m) + 0),
			Vec4V::Load(reinterpret_cast<const Vec4V*>(m) + 1),
			Vec4V::Load(reinterpret_cast<const Vec4V*>(m) + 2),
			Vec4V::Load(reinterpret_cast<const Vec4V*>(m) + 3));
	}

	VMATH_INLINE static OutType LoadUnaligned(const T* m)
	{
		return T(
			Vec4V::LoadUnaligned(reinterpret_cast<const Vec4V*>(m) + 0),
			Vec4V::LoadUnaligned(reinterpret_cast<const Vec4V*>(m) + 1),
			Vec4V::LoadUnaligned(reinterpret_cast<const Vec4V*>(m) + 2),
			Vec4V::LoadUnaligned(reinterpret_cast<const Vec4V*>(m) + 3));
	}

	VMATH_INLINE friend OutType Transpose(ArgType m)
	{
		Vec4V a,b,c,d;
		Mat44VTranspose(a,b,c,d,m.m_a,m.m_b,m.m_c,m.d());
		return Mat44V(a,b,c,d);
	}

	// m.Transform(v) transforms vector v through matrix m
	VMATH_INLINE Vec4V_out Transform(Vec4V_arg v) const { return MultiplyAdd(m_a,v.x(),m_b,v.y(),m_c,v.z(),m_d,v.w()); }
	VMATH_INLINE Vec4V_out Transform(Vec3V_arg v) const { return MultiplyAdd(m_a,v.x(),m_b,v.y(),m_c,v.z(),m_d); } // treat v as {x,y,z,1}

	VMATH_INLINE Vec4V_out TransformSelect000(Vec3V_arg a,Vec3V_arg b) const { return Transform(a); }
	VMATH_INLINE Vec4V_out TransformSelect100(Vec3V_arg a,Vec3V_arg b) const { return MultiplyAdd(m_a,b.x(),m_b,a.y(),m_c,a.z(),m_d); } // same as Transform(Select100(a,b))
	VMATH_INLINE Vec4V_out TransformSelect010(Vec3V_arg a,Vec3V_arg b) const { return MultiplyAdd(m_a,a.x(),m_b,b.y(),m_c,a.z(),m_d); } // same as Transform(Select010(a,b))
	VMATH_INLINE Vec4V_out TransformSelect110(Vec3V_arg a,Vec3V_arg b) const { return MultiplyAdd(m_a,b.x(),m_b,b.y(),m_c,a.z(),m_d); } // same as Transform(Select110(a,b))
	VMATH_INLINE Vec4V_out TransformSelect001(Vec3V_arg a,Vec3V_arg b) const { return MultiplyAdd(m_a,a.x(),m_b,a.y(),m_c,b.z(),m_d); } // same as Transform(Select001(a,b))
	VMATH_INLINE Vec4V_out TransformSelect101(Vec3V_arg a,Vec3V_arg b) const { return MultiplyAdd(m_a,b.x(),m_b,a.y(),m_c,b.z(),m_d); } // same as Transform(Select101(a,b))
	VMATH_INLINE Vec4V_out TransformSelect011(Vec3V_arg a,Vec3V_arg b) const { return MultiplyAdd(m_a,a.x(),m_b,b.y(),m_c,b.z(),m_d); } // same as Transform(Select011(a,b))
	VMATH_INLINE Vec4V_out TransformSelect111(Vec3V_arg a,Vec3V_arg b) const { return Transform(b); }

	// m.TransformProject(v) transforms 3D vector v (as if w=1) and then projects the result by dividing by w
	VMATH_INLINE Vec3V_out TransformProject(Vec3V_arg v) const { const Vec4V q = Transform(Vec4V(v,1.0f)); return q.xyz()/q.w(); }

	// Multiply(m1,m2) is standard matrix multiplication
	VMATH_INLINE friend OutType Multiply(ArgType m1,ArgType m2)
	{
		return Mat44V(
			MultiplyAdd(
				m1.m_a,m2.m_a.x(),
				m1.m_b,m2.m_a.y(),
				m1.m_c,m2.m_a.z(),
				m1.d(),m2.m_a.w()),
			MultiplyAdd(
				m1.m_a,m2.m_b.x(),
				m1.m_b,m2.m_b.y(),
				m1.m_c,m2.m_b.z(),
				m1.d(),m2.m_b.w()),
			MultiplyAdd(
				m1.m_a,m2.m_c.x(),
				m1.m_b,m2.m_c.y(),
				m1.m_c,m2.m_c.z(),
				m1.d(),m2.m_c.w()),
			MultiplyAdd(
				m1.m_a,m2.d().x(),
				m1.m_b,m2.d().y(),
				m1.m_c,m2.d().z(),
				m1.d(),m2.d().w()));
	}

	// multiplication operators: m1*m2*v == Multiply(m1,m2).Transform(v) == m1.Transform(m2.Transform(v))
	VMATH_INLINE friend Vec4V_out operator *(ArgType m,Vec4V_arg v) { return m.Transform(v); }
	VMATH_INLINE friend Vec3V_out operator *(ArgType m,Vec3V_arg v) { return m.TransformProject(v); }
	VMATH_INLINE friend OutType operator *(ArgType a,ArgType b) { return Multiply(a,b); }

	VMATH_INLINE friend OutType operator +(ArgType m1,ArgType m2) { return T(m1.m_a + m2.m_a,m1.m_b + m2.m_b,m1.m_c + m2.m_c,m1.d() + m2.d()); }
	VMATH_INLINE friend OutType operator -(ArgType m1,ArgType m2) { return T(m1.m_a - m2.m_a,m1.m_b - m2.m_b,m1.m_c - m2.m_c,m1.d() - m2.d()); }
	VMATH_INLINE friend OutType operator *(ArgType m,ScalarV_arg s) { return T(m.m_a*s,m.m_b*s,m.m_c*s,m.d()*s); }
	VMATH_INLINE friend OutType operator /(ArgType m,ScalarV_arg s) { return m*Recip(s); }
	VMATH_INLINE friend OutType operator *(ScalarV_arg s,ArgType m) { return T(m.m_a*s,m.m_b*s,m.m_c*s,m.d()*s); }
	VMATH_INLINE friend OutType operator *(ArgType m,float f) { return m*ScalarV(f); }
	VMATH_INLINE friend OutType operator /(ArgType m,float f) { return m/ScalarV(f); }
	VMATH_INLINE friend OutType operator *(float f,ArgType m) { return m*ScalarV(f); }

	VMATH_INLINE friend float MaxDiff(ArgType a,ArgType b) { return MaxElement(Max(Abs(a.m_a - b.m_a),Abs(a.m_b - b.m_b),Abs(a.m_c - b.m_c),Abs(a.d() - b.d()))).f(); }

private:
	Vec4V m_a;
	Vec4V m_b;
	Vec4V m_c;
	Vec4V m_d;
};

typedef Mat44V::ArgType Mat44V_arg;
typedef Mat44V::OutType Mat44V_out;

Mat44V_out Invert(Mat44V_arg m);
Mat44V_out Invert_REFERENCE(Mat44V_arg m);

VMATH_INLINE void PrintV(const char* name,Mat33V_arg m) { printf("%s={%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f}\n",name,VEC3V_ARGS(m.a()),VEC3V_ARGS(m.b()),VEC3V_ARGS(m.c())); }
VMATH_INLINE void PrintV(const char* name,Mat34V_arg m) { printf("%s={%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f}\n",name,VEC3V_ARGS(m.a()),VEC3V_ARGS(m.b()),VEC3V_ARGS(m.c()),VEC3V_ARGS(m.d())); }
VMATH_INLINE void PrintV(const char* name,Mat44V_arg m) { printf("%s={%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f}\n",name,VEC4V_ARGS(m.a()),VEC4V_ARGS(m.b()),VEC4V_ARGS(m.c()),VEC4V_ARGS(m.d())); }

class Mat33f
{
public:
	typedef Mat33f T;
	typedef const T& ArgType;
	typedef const T OutType;

	VMATH_INLINE Mat33f() {}
	VMATH_INLINE Mat33f(Vec3f_arg a,Vec3f_arg b,Vec3f_arg c): m_a(a),m_b(b),m_c(c) {}
	VMATH_INLINE Mat33f(Mat33V_arg m) : m_a(m.a()),m_b(m.b()),m_c(m.c()) {}
	VMATH_INLINE operator Mat33V_out() const { return Mat33V(m_a,m_b,m_c); }
	VMATH_INLINE static OutType Identity() { return T(Vec3f(1.0f,0.0f,0.0f),Vec3f(0.0f,1.0f,0.0f),Vec3f(0.0f,0.0f,1.0f)); }

	Vec3f m_a;
	Vec3f m_b;
	Vec3f m_c;
};

class Mat34f
{
public:
	typedef Mat34f T;
	typedef const T& ArgType;
	typedef const T OutType;

	VMATH_INLINE Mat34f() {}
	VMATH_INLINE Mat34f(typename Mat33f::ArgType m,Vec3f_arg d): m_mat33(m),m_d(d) {}
	VMATH_INLINE Mat34f(Vec3f_arg a,Vec3f_arg b,Vec3f_arg c,Vec3f_arg d): m_mat33(a,b,c),m_d(d) {}
	VMATH_INLINE Mat34f(Mat34V_arg m) : m_mat33(m.GetMat33V()),m_d(m.d()) {}
	VMATH_INLINE operator Mat34V_out() const { return Mat34V(m_mat33,m_d); }
	VMATH_INLINE static OutType Identity() { return T(Vec3f(1.0f,0.0f,0.0f),Vec3f(0.0f,1.0f,0.0f),Vec3f(0.0f,0.0f,1.0f),Vec3f(0.0f)); }

	Mat33f m_mat33;
	Vec3f m_d;
};

#endif // _INCLUDE_VMATH_MATRIX_H_