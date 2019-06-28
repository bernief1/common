// =================================
// common/vmath/vmath_intersects.cpp
// =================================

#include "vmath_box.h"
#include "vmath_intersects.h"
#include "vmath_plane.h"
#include "vmath_random.h"
#include "vmath_triangle.h"
#include "vmath_vec4.h"

#include "GraphicsTools/util/mesh.h"

static Vec3V_out MakeRandomQuantizedPoint3D(Vec3V_arg rangeMin,Vec3V_arg rangeMax,uint32 quantization,uint32* randomState)
{
	Vec3V p = GetRandomPoint3D(Vec3V(V_ZERO),Vec3V(V_ONE),randomState);
	if (quantization > 1)
		p = Floor(p*(float)quantization)/(float)quantization;
	return rangeMin + (rangeMax - rangeMin)*p;
}

static bool TestTriangleBoxIntersectInternal(uint32 quantization,uint32 count,uint32* randomState)
{
	printf("testing triangle-box intersection (quantization %u) ..\n",quantization);
	const Vec3V rangeMin(V_ZERO);
	const Vec3V rangeMax(V_ONE);
	uint32 passed = 0;
	uint32 failed = 0;
	uint32 passed_SOA4 = 0;
	uint32 failed_SOA4 = 0;
	uint32 intersectA = 0; // number of cases where triangle intersects box using function under test
	uint32 intersectB = 0; // number of cases where triangle intersects box using reference code
	for (uint32 i = 0; i < count; i++) {
		const Vec3V b0 = MakeRandomQuantizedPoint3D(rangeMin,rangeMax,quantization,randomState);
		const Vec3V b1 = MakeRandomQuantizedPoint3D(rangeMin,rangeMax,quantization,randomState);
		const Box3V box(Min(b0,b1),Max(b0,b1));
		if (box.GetVolume() < 0.0000001f) {
			i--; // try again ..
			continue;
		}
		const Plane3V boxClipPlanes[] = {
			Plane3V(+Vec3V(V_XAXIS),-box.GetMin().x()),
			Plane3V(+Vec3V(V_YAXIS),-box.GetMin().y()),
			Plane3V(+Vec3V(V_ZAXIS),-box.GetMin().z()),
			Plane3V(-Vec3V(V_XAXIS),+box.GetMax().x()),
			Plane3V(-Vec3V(V_YAXIS),+box.GetMax().y()),
			Plane3V(-Vec3V(V_ZAXIS),+box.GetMax().z()),
		};
		Triangle3V tris[4];
		uint32 testB_SOA4 = 0;
		bool skip_SOA4 = false;
		for (uint32 j = 0; j < 4; j++) {
			const Triangle3V tri(
				MakeRandomQuantizedPoint3D(rangeMin,rangeMax,quantization,randomState),
				MakeRandomQuantizedPoint3D(rangeMin,rangeMax,quantization,randomState),
				MakeRandomQuantizedPoint3D(rangeMin,rangeMax,quantization,randomState));
			if (tri.GetArea() < 0.0001f) {
				j--; // try again ..
				continue;
			}
			tris[j] = tri;
			const size_t maxVertsPerClippedPoly = Triangle3V::NumVerts + countof(boxClipPlanes);
			Vec3V src[maxVertsPerClippedPoly];
			Vec3V dst[maxVertsPerClippedPoly];
			uint32 srcCount = Triangle3V::NumVerts;
			uint32 dstCount = 0;
			memcpy(src,tri.m_positions,Triangle3V::NumVerts*sizeof(Vec3V));
			for (uint32 i = 0; i < countof(boxClipPlanes) && srcCount >= Triangle3V::NumVerts; i++) {
				const Plane3V clipPlane = boxClipPlanes[i];
				ScalarV maxAbsDistToPlane = ScalarV(V_ZERO);
				for (uint32 j = 0; j < srcCount; j++)
					maxAbsDistToPlane = Max(Abs(clipPlane.GetDistanceToPoint(src[j])),maxAbsDistToPlane);
				if (maxAbsDistToPlane > 0.0f && geomesh::PolyClip(dst,dstCount,maxVertsPerClippedPoly,src,srcCount,clipPlane)) {
					memcpy(src,dst,dstCount*sizeof(Vec3V));
					srcCount = dstCount;
				}
			}
			//float area = 0.0f;
			//for (uint32 i = 2; i < srcCount; i++)
			//	area += Triangle3V(src[0],src[i - 1],src[i]).GetArea().f();
			//const bool testB = area > 0.0f; // reference test - clipped triangle to box face-by-face
			const bool testB = srcCount > 0;
			const bool testA = tri.IntersectsBox(box) != 0; // the intersection function we're testing
			if (testA == testB)
				passed++;
			else {
				if (failed < 10) { // output the failure case
					static uint32 index = 0;
					geomesh::TriangleMesh mesh;
					geomesh::ConstructBox(mesh,box);
					const uint32 vertStart = (uint32)mesh.m_verts.size();
					mesh.m_verts.push_back(tri.m_positions[0]);
					mesh.m_verts.push_back(tri.m_positions[1]);
					mesh.m_verts.push_back(tri.m_positions[2]);
					geomesh::IndexedTriangle(vertStart,0,1,2,-1).Add(mesh.m_polys);
					char path[512];
					sprintf(path,"failed_%06u.obj",index++);
					SaveOBJ(path,mesh);
				}
				failed++;
				skip_SOA4 = true; // if we failed a single case in the SOA4 ground, skip the SOA4 group
			}
			if (testA)
				intersectA++;
			if (testB) {
				intersectB++;
				testB_SOA4 |= BIT(j);
			}
		}
		if (!skip_SOA4) {
			const uint32 testA_SOA4 = Triangle3V_SOA4(tris).IntersectsBox(box);
			if (testA_SOA4 == testB_SOA4)
				passed_SOA4++;
			else
				failed_SOA4++;
		}
	}
	printf("triangle-box intersection test passed %u (%u SOA4), failed %u (%u SOA4), A=%u, B=%u\n",passed,passed_SOA4,failed,failed_SOA4,intersectA,intersectB);
	return failed + failed_SOA4 == 0;
}

bool TestTriangleBoxIntersect()
{
	uint32 randomState = 1234567;
	const uint32 count = 1000000;
	const bool test1 = TestTriangleBoxIntersectInternal(0,count,&randomState);
	const bool test2 = TestTriangleBoxIntersectInternal(64,count,&randomState); // testing with quantized coords exposes corner cases where the triangle and box touch edges ..
	return test1 && test2;
}