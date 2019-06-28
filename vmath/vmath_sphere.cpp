// =============================
// common/vmath/vmath_sphere.cpp
// =============================

#include "vmath_intersects.h"
#include "vmath_random.h"
#include "vmath_sphere.h"

Vec2V_out Circle2V::GetRandomPointOnCirclePerimeter() const
{
	const float theta = GetRandomValueInRange(0.0f,2.0f*PI);
	return GetCenter() + GetRadius()*Vec2V::SinCos(theta);
}

Vec2V_out Circle2V::GetRandomPointInCircleInterior() const
{
	for (int i = 0; i < 10; i++) { // finite iteration just in case something weird happens ..
		const float r = GetRadius().f();
		const Vec2V p = GetCenter() + Vec2V(GetRandomValueInRange(-r,r),GetRandomValueInRange(-r,r));
		if (Intersects(p,*this))
			return p;
	}
	return GetCenter();
}

const Circle2V Circle2V::BuildCircleThrough2Points(Vec2V_arg p1,Vec2V_arg p2)
{
	return Circle2V((p2 + p1)*0.5f,Mag(p2 - p1)*0.5f);
}

// assumes p1,p2,p3 are unique and not collinear
const Circle2V Circle2V::BuildCircleThrough3Points(Vec2V_arg p1,Vec2V_arg p2,Vec2V_arg p3)
{
	const Vec2V v12 = p2 - p1;
	const Vec2V v13 = p3 - p1;
	const Vec2V q = v13*Dot(v12,p1 + p2) - v12*Dot(v13,p1 + p3);
	const Vec2V center = Vec2V(q.y(),-q.x())*0.5f/Cross(v12,v13);
	return Circle2V(center,Mag(p1 - center));
}

const Circle2V Circle2V::BuildMinimumEnclosingCircle(const Vec2V* points,int count)
{
	// uses Welzl algorithm
	class mec { public: static Circle2V func(const Vec2V* points,int count,std::vector<int>& boundaryIndices,int b = 0) {
		Circle2V result = Invalid();
		if (count == 0) {
			switch (b) {
			case 3: result = BuildCircleThrough3Points(points[boundaryIndices[0]],points[boundaryIndices[1]],points[boundaryIndices[2]]); break;
			case 2: result = BuildCircleThrough2Points(points[boundaryIndices[0]],points[boundaryIndices[1]]); break;
			case 1: result = Circle2V(points[boundaryIndices[0]],0.0f); break;
			}
		} else {
			const Vec2V p = points[--count];
			result = func(points,count,boundaryIndices,b);
			if (!Intersects(p,result)) {
				if (boundaryIndices.size() <= (size_t)b)
					boundaryIndices.resize(b + 1);
				boundaryIndices[b++] = count;
				result = func(points,count,boundaryIndices,b);
			}
		}
		return result;
	}};
	std::vector<int> boundaryIndices;
	return mec::func(points,count,boundaryIndices);
}

#if VMATH_TEST_GEOM
const Circle2V Circle2V::BuildMinimumEnclosingCircle_DEBUG(const Vec2V* points,int count,std::vector<int>& debugBoundaryIndices)
{
	// uses Welzl algorithm
	class mec { public: static Circle2V func(const Vec2V* points,int count,std::vector<int>& debugBoundaryIndices,std::vector<int>& boundaryIndices,int b = 0) {
		Circle2V result = Invalid();
		if (count == 0) {
			switch (b) {
			case 3:
				debugBoundaryIndices.clear();
				debugBoundaryIndices.push_back(boundaryIndices[0]);
				debugBoundaryIndices.push_back(boundaryIndices[1]);
				debugBoundaryIndices.push_back(boundaryIndices[2]);
				result = BuildCircleThrough3Points(points[boundaryIndices[0]],points[boundaryIndices[1]],points[boundaryIndices[2]]);
				break;
			case 2:
				debugBoundaryIndices.clear();
				debugBoundaryIndices.push_back(boundaryIndices[0]);
				debugBoundaryIndices.push_back(boundaryIndices[1]);
				result = BuildCircleThrough2Points(points[boundaryIndices[0]],points[boundaryIndices[1]]);
				break;
			case 1:
				debugBoundaryIndices.clear();
				debugBoundaryIndices.push_back(boundaryIndices[0]);
				result = Circle2V(points[boundaryIndices[0]],0.0f);
				break;
			}
		} else {
			const Vec2V p = points[--count];
			result = func(points,count,debugBoundaryIndices,boundaryIndices,b);
			if (!Intersects(p,result)) {
				if (boundaryIndices.size() <= b)
					boundaryIndices.resize(b + 1);
				boundaryIndices[b++] = count;
				result = func(points,count,debugBoundaryIndices,boundaryIndices,b);
			}
		}
		return result;
	}};
	std::vector<int> boundaryIndices;
	return mec::func(points,count,debugBoundaryIndices,boundaryIndices);
}
#endif // VMATH_TEST_GEOM

Vec3V_out Sphere3V::GetRandomPointOnSphereSurface() const
{
	const float r = GetRadius().f();
	const float z = GetRandomValueInRange(-r,r);
	const float theta = GetRandomValueInRange(0.0f,2.0f*PI);
	const ScalarV s = Sqrt(GetRadiusSqr() - ScalarV(z*z));
	return GetCenter() + Vec3V(Vec2V::SinCos(theta)*s,z);
}

Vec3V_out Sphere3V::GetRandomPointInSphereInterior() const
{
	for (int i = 0; i < 10; i++) { // finite iteration just in case something weird happens ..
		const float r = GetRadius().f();
		const Vec3V p = GetCenter() + Vec3V(GetRandomValueInRange(-r,r),GetRandomValueInRange(-r,r),GetRandomValueInRange(-r,r));
		if (Intersects(p,*this))
			return p;
	}
	return GetCenter();
}

const Sphere3V Sphere3V::BuildSphereThrough2Points(Vec3V_arg p1,Vec3V_arg p2)
{
	return Sphere3V((p2 + p1)*0.5f,Mag(p2 - p1)*0.5f);
}

const Sphere3V Sphere3V::BuildSphereThrough3Points(Vec3V_arg p1,Vec3V_arg p2,Vec3V_arg p3)
{
	// https://github.com/speps/TBToolkit/blob/master/TBToolkit/External/MathGeoLib/src/Mesh/Sphere.cpp
	const Vec3V v12 = p2 - p1;
	const Vec3V v13 = p3 - p1;

	const ScalarV v12v12 = Dot(v12,v12);
	const ScalarV v12v13 = Dot(v12,v13);
	const ScalarV v13v13 = Dot(v13,v13);

	const Vec2V uv = Vec2V(v13v13*(v12v12 - v12v13),v12v12*(v13v13 - v12v13))*0.5f/(v12v12*v13v13 - v12v13*v12v13); // Barycentric coords within triangle
	const Vec3V center = p1 + uv.x()*v12 + uv.y()*v13;
	return Sphere3V(center,Mag(p1 - center));
}

// assumes p1,p2,p3,p4 are unique and not collinear
const Sphere3V Sphere3V::BuildSphereThrough4Points(Vec3V_arg p1,Vec3V_arg p2,Vec3V_arg p3,Vec3V_arg p4)
{
	// https://github.com/speps/TBToolkit/blob/master/TBToolkit/External/MathGeoLib/src/Mesh/Sphere.cpp
	const Vec3V v12 = p2 - p1;
	const Vec3V v13 = p3 - p1;
	const Vec3V v14 = p4 - p1;

	const ScalarV v12v12 = Dot(v12,v12);
	const ScalarV v12v13 = Dot(v12,v13);
	const ScalarV v12v14 = Dot(v12,v14);
	const ScalarV v13v13 = Dot(v13,v13);
	const ScalarV v13v14 = Dot(v13,v14);
	const ScalarV v14v14 = Dot(v14,v14);

	const Mat33V matrix( // note that this matrix is symmetrical, we could optimize the inverse operation ..
		Vec3V(v12v12,v12v13,v12v14),
		Vec3V(v12v13,v13v13,v13v14),
		Vec3V(v12v14,v13v14,v14v14));

	const Vec3V uvw = InvertAffine(matrix).Transform(Vec3V(v12v12,v13v13,v14v14))*0.5f; // Barycentric coords within tetrahedron
	const Vec3V center = p1 + uvw.x()*v12 + uvw.y()*v13 + uvw.z()*v14;
	return Sphere3V(center,Mag(p1 - center));
}

const Sphere3V Sphere3V::BuildMinimumEnclosingSphere(const Vec3V* points,int count)
{
	// uses Welzl algorithm
	class mec { public: static Sphere3V func(const Vec3V* points,int count,std::vector<int>& boundaryIndices,int b = 0) {
		Sphere3V result = Invalid();
		if (count == 0) {
			switch (b) {
			case 4: result = BuildSphereThrough4Points(points[boundaryIndices[0]],points[boundaryIndices[1]],points[boundaryIndices[2]],points[boundaryIndices[3]]); break;
			case 3: result = BuildSphereThrough3Points(points[boundaryIndices[0]],points[boundaryIndices[1]],points[boundaryIndices[2]]); break;
			case 2: result = BuildSphereThrough2Points(points[boundaryIndices[0]],points[boundaryIndices[1]]); break;
			case 1: result = Sphere3V(points[boundaryIndices[0]],0.0f); break;
			}
		} else {
			const Vec3V p = points[--count];
			result = func(points,count,boundaryIndices,b);
			if (!Intersects(p,result)) {
				if (boundaryIndices.size() <= b)
					boundaryIndices.resize(b + 1);
				boundaryIndices[b++] = count;
				result = func(points,count,boundaryIndices,b);
			}
		}
		return result;
	}};
	std::vector<int> boundaryIndices;
	return mec::func(points,count,boundaryIndices);
}
