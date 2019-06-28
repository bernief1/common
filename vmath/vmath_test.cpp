// ===========================
// common/vmath/vmath_test.cpp
// ===========================

#include "vmath_test.h"

#if VMATH_TEST

#include "GraphicsTools/util/imageutil.h"
#include "GraphicsTools/util/progressdisplay.h"

#include "vmath_common.h"
#include "vmath_float16.h"
#include "vmath_intdiv.h"
#include "vmath_intersects.h"
#include "vmath_random.h"
#include "vmath_sampling.h"
#include "vmath_transcendental.h"

#if VMATH_TEST_LOAD_STORE
void TestLoadStore()
{
	const float tests[] = {0.0f,-0.0f,1.0f,-1.0f,2.0f,3.0f,4.0f,0.5f,0.25f,100.0f,1000.0f,10000.0f,10.5f,100.5f,1.0f/256.0f,1.0f/256.0f+1.0f/512.0f};
	uint32 temp[] = {0,0,0,0};

	printf("testing ScalarV load/store ..\n");
	for (int i = 0; i < icountof(tests); i++) {
		ScalarV s1 = ScalarV(tests[i]);
		s1.Store(reinterpret_cast<float*>(temp));
		ScalarV s2 = ScalarV::Load(reinterpret_cast<const float*>(temp));
		printf("%f = {0x%08x} = %f\n",tests[i],temp[0],s2.f());
		ForceAssert(s1 == s2);
	}
	printf("testing Vec2V load/store ..\n");
	for (int i = 0; i < icountof(tests) - 1; i++) {
		Vec2V v1 = Vec2V(tests[i],tests[i + 1]);
		v1.Store(reinterpret_cast<float*>(temp));
		Vec2V v2 = Vec2V::Load(reinterpret_cast<const float*>(temp));
		printf("%f,%f = {0x%08x,0x%08x} = %f,%f\n",VEC2V_ARGS(v1),temp[0],temp[1],VEC2V_ARGS(v2));
		ForceAssert(All(v1 == v2));
	}
	printf("testing Vec3V load/store ..\n");
	for (int i = 0; i < icountof(tests) - 2; i++) {
		Vec3V v1 = Vec3V(tests[i],tests[i + 1],tests[i + 2]);
		v1.Store(reinterpret_cast<float*>(temp));
		Vec3V v2 = Vec3V::Load(reinterpret_cast<const float*>(temp));
		printf("%f,%f,%f = {0x%08x,0x%08x,0x%08x} = %f,%f,%f\n",VEC3V_ARGS(v1),temp[0],temp[1],temp[2],VEC3V_ARGS(v2));
		ForceAssert(All(v1 == v2));
	}
	printf("testing Vec4V load/store ..\n");
	for (int i = 0; i < icountof(tests) - 3; i++) {
		Vec4V v1 = Vec4V(tests[i],tests[i + 1],tests[i + 2],tests[i + 3]);
		v1.Store((Vec4V*)temp);
		Vec4V v2 = Vec4V::Load((const Vec4V*)temp);
		printf("%f,%f,%f,%f = {...} = %f,%f,%f,%f\n",VEC4V_ARGS(v1),VEC4V_ARGS(v2));
		ForceAssert(All(v1 == v2));
	}
}

void TestLoadStoreFloat16()
{
	const float tests[] = {0.0f,-0.0f,1.0f,-1.0f,2.0f,3.0f,4.0f,0.5f,0.25f,100.0f,1000.0f,10000.0f,10.5f,100.5f,1.0f/256.0f,1.0f/256.0f+1.0f/512.0f,INFINITY};
	uint16 temp[] = {0,0,0,0};
	
	printf("testing ScalarV float16 load/store ..\n");
	for (int i = 0; i < icountof(tests); i++) {
		ScalarV s1 = ScalarV(tests[i]);
		s1.StoreFloat16(temp);
		ScalarV s2 = ScalarV::LoadFloat16(temp);
		printf("%f = {0x%04x,0x%04x,0x%04x,0x%04x} = %f\n",tests[i],temp[0],temp[1],temp[2],temp[3],s2.f());
		ForceAssert(s1 == s2);
	}
	printf("testing Vec2V float16 load/store ..\n");
	for (int i = 0; i < icountof(tests) - 1; i++) {
		Vec2V v1 = Vec2V(tests[i],tests[i + 1]);
		v1.StoreFloat16V(temp);
		Vec2V v2 = Vec2V::LoadFloat16V(temp);
		printf("%f,%f = {0x%04x,0x%04x} = %f,%f\n",VEC2V_ARGS(v1),temp[0],temp[1],VEC2V_ARGS(v2));
		ForceAssert(All(v1 == v2));
	}
	printf("testing Vec3V float16 load/store ..\n");
	for (int i = 0; i < icountof(tests) - 2; i++) {
		Vec3V v1 = Vec3V(tests[i],tests[i + 1],tests[i + 2]);
		v1.StoreFloat16V(temp);
		Vec3V v2 = Vec3V::LoadFloat16V(temp);
		printf("%f,%f,%f = {0x%04x,0x%04x,0x%04x} = %f,%f,%f\n",VEC3V_ARGS(v1),temp[0],temp[1],temp[2],VEC3V_ARGS(v2));
		ForceAssert(All(v1 == v2));
	}
	printf("testing Vec4V float16 load/store ..\n");
	for (int i = 0; i < icountof(tests) - 3; i++) {
		Vec4V v1 = Vec4V(tests[i],tests[i + 1],tests[i + 2],tests[i + 3]);
		v1.StoreFloat16V(temp);
		Vec4V v2 = Vec4V::LoadFloat16V(temp);
		printf("%f,%f,%f,%f = {...} = %f,%f,%f,%f\n",VEC4V_ARGS(v1),VEC4V_ARGS(v2));
		ForceAssert(All(v1 == v2));
	}

	for (int i = 4; i >= -4; i--) {
		const float base = powf(10.0f,(float)i);
		const uint16 x = Float32toFloat16(base);
		const uint16 x_test = Float32toFloat16_REFERENCE(base);
		//printf("base = %f (0x%08x), x = 0x%04x (%.9f), x_test = 0x%04x (%.9f)\n",base,*(const uint32*)&base,x,Float16toFloat32(x),x_test,Float16toFloat32(x_test));
		ForceAssert(Abs((int)x - (int)x_test) <= 1);
		const uint16 x1 = x + 1;
		const float base1 = Float16toFloat32(x1);
		const float base1_test = Float16toFloat32_REFERENCE(x1);
		//printf("base1 = %f, base1_test = %f, x1 = 0x%04x\n",base1,base1_test,x1);
		ForceAssert(base1 == base1_test);
		const float diff = base1 - base;
		printf("Float16 precision is %.9f @ %f\n",diff,base);
	}
}

void TestLoadStoreFixed()
{
	const float tests[] = {0.0f,0.1f,0.01f,0.001f,0.011f,0.111f,0.234f,0.678f};
	uint16 temp16[] = {0,0,0,0};
	uint8 temp8[] = {0,0,0,0};

	printf("testing Vec4V fixed16 load/store ..\n");
	for (int i = 0; i < icountof(tests) - 3; i++) {
		Vec4V v1 = Vec4V(tests[i],tests[i + 1],tests[i + 2],tests[i + 3]);
		v1.StoreFixed16V(temp16);
		Vec4V v2 = Vec4V::LoadFixed16V(temp16);
		const float maxerr = MaxElement(Abs(v1 - v2)).f();
		printf("%f,%f,%f,%f = {...} = %f,%f,%f,%f (maxerr = %.9f)\n",VEC4V_ARGS(v1),VEC4V_ARGS(v2),maxerr);
		ForceAssert(maxerr <= 1.0f/65535.0f);
	}

	printf("testing Vec4V fixed8 load/store ..\n");
	for (int i = 0; i < icountof(tests) - 3; i++) {
		Vec4V v1 = Vec4V(tests[i],tests[i + 1],tests[i + 2],tests[i + 3]);
		v1.StoreFixed8V(temp8);
		Vec4V v2 = Vec4V::LoadFixed8V(temp8);
		const float maxerr = MaxElement(Abs(v1 - v2)).f();
		printf("%f,%f,%f,%f = {...} = %f,%f,%f,%f (maxerr = %.9f)\n",VEC4V_ARGS(v1),VEC4V_ARGS(v2),maxerr);
		ForceAssert(maxerr <= 1.0f/255.0f);
	}
}
#endif // VMATH_TEST_LOAD_STORE

#if VMATH_TEST_FIXED_CONVERSIONS
void TestSignedNormConversions()
{
	for (int i = -127; i <= 127; i++) {
		const float slop = 0.0f; // nice.
		const unsigned N = 8;
		typedef int8 T;
		ForceAssert(Abs(i) <= BITMASK(T,N - 1));
		const float f = FixedSignedToFloat32<T,N>(i);
		const T i0 = Float32toFixedSigned<T,N>(f - (1.0f - slop)/(float)BITMASK(uint32,N)); // make sure +/-1/255 does not affect the compressed value
		const T i1 = Float32toFixedSigned<T,N>(f);
		const T i2 = Float32toFixedSigned<T,N>(f + (1.0f - slop)/(float)BITMASK(uint32,N));
		ForceAssert(i0 == i);
		ForceAssert(i1 == i);
		ForceAssert(i2 == i);
	}

	for (int i = -61; i <= 61; i++) {
		const float slop = 0.01f;
		const unsigned N = 7;
		typedef int8 T;
		ForceAssert(Abs(i) <= BITMASK(T,N - 1));
		const float f = FixedSignedToFloat32<T,N>(i);
		const T i0 = Float32toFixedSigned<T,N>(f - (1.0f - slop)/(float)BITMASK(uint32,N)); // make sure +/-1/255 does not affect the compressed value
		const T i1 = Float32toFixedSigned<T,N>(f);
		const T i2 = Float32toFixedSigned<T,N>(f + (1.0f - slop)/(float)BITMASK(uint32,N));
		ForceAssert(i0 == i);
		ForceAssert(i1 == i);
		ForceAssert(i2 == i);
	}

	for (int i = -5000; i <= 5000; i++) {
		const float slop = 0.01f;
		const unsigned N = 14;
		typedef int16 T;
		ForceAssert(Abs(i) <= BITMASK(T,N - 1));
		const float f = FixedSignedToFloat32<T,N>(i);
		const T i0 = Float32toFixedSigned<T,N>(f - (1.0f - slop)/(float)BITMASK(uint32,N)); // make sure +/-1/255 does not affect the compressed value
		const T i1 = Float32toFixedSigned<T,N>(f);
		const T i2 = Float32toFixedSigned<T,N>(f + (1.0f - slop)/(float)BITMASK(uint32,N));
		ForceAssert(i0 == i);
		ForceAssert(i1 == i);
		ForceAssert(i2 == i);
	}

	for (int i = -500000; i <= 500000; i++) {
		const float slop = 0.2f; // wth? is this indicative of a bug in Float32toFixedSigned?
		const unsigned N = 20;
		typedef int32 T;
		ForceAssert(Abs(i) <= BITMASK(T,N - 1));
		const float f = FixedSignedToFloat32<T,N>(i);
		const T i0 = Float32toFixedSigned<T,N>(f - (1.0f - slop)/(float)BITMASK(uint32,N));
		const T i1 = Float32toFixedSigned<T,N>(f);
		const T i2 = Float32toFixedSigned<T,N>(f + (1.0f - slop)/(float)BITMASK(uint32,N));
		ForceAssert(i0 == i);
		ForceAssert(i1 == i);
		ForceAssert(i2 == i);
	}
}
#endif // VMATH_TEST_FIXED_CONVERSIONS

#if VMATH_TEST_GEOM
void TestBuildSphereThroughPoints()
{
	const Box3V bounds(Vec3V(0.0f),Vec3V(1.0f));
	for (int i = 0; i < 1000; i++) {
		const Vec3V p1 = GetRandomPoint3D(bounds.GetMin(),bounds.GetMax());
		const Vec3V p2 = GetRandomPoint3D(bounds.GetMin(),bounds.GetMax());
		const Vec3V p3 = GetRandomPoint3D(bounds.GetMin(),bounds.GetMax());
		const Vec3V p4 = GetRandomPoint3D(bounds.GetMin(),bounds.GetMax());

		const Sphere3V sphere12 = Sphere3V::BuildSphereThrough2Points(p1,p2);
		const ScalarV d12_1 = sphere12.GetRadius() - Mag(sphere12.GetCenter() - p2); ForceAssert(Abs(d12_1) < 0.0011f);
		const ScalarV d12_2 = sphere12.GetRadius() - Mag(sphere12.GetCenter() - p2); ForceAssert(Abs(d12_2) < 0.0011f);
		
		const Sphere3V sphere123 = Sphere3V::BuildSphereThrough3Points(p1,p2,p3);
		const ScalarV d123_1 = sphere123.GetRadius() - Mag(sphere123.GetCenter() - p2); ForceAssert(Abs(d123_1) < 0.0011f);
		const ScalarV d123_2 = sphere123.GetRadius() - Mag(sphere123.GetCenter() - p2); ForceAssert(Abs(d123_2) < 0.0011f);
		const ScalarV d123_3 = sphere123.GetRadius() - Mag(sphere123.GetCenter() - p2); ForceAssert(Abs(d123_3) < 0.0011f);
		
		const Sphere3V sphere1234 = Sphere3V::BuildSphereThrough4Points(p1,p2,p3,p4);
		const ScalarV d1234_1 = sphere1234.GetRadius() - Mag(sphere1234.GetCenter() - p2); ForceAssert(Abs(d1234_1) < 0.0011f);
		const ScalarV d1234_2 = sphere1234.GetRadius() - Mag(sphere1234.GetCenter() - p2); ForceAssert(Abs(d1234_2) < 0.0011f);
		const ScalarV d1234_3 = sphere1234.GetRadius() - Mag(sphere1234.GetCenter() - p2); ForceAssert(Abs(d1234_3) < 0.0011f);
		const ScalarV d1234_4 = sphere1234.GetRadius() - Mag(sphere1234.GetCenter() - p2); ForceAssert(Abs(d1234_4) < 0.0011f);
	}
}

static const Box2V GetRandomBox2V(const Box2V& bounds,float minRelativeSize = 0.005f,float maxRelativeSize = 0.1f,float maxAspect = 10.0f)
{
	const Vec2V boundsSize = bounds.GetMax() - bounds.GetMin();
	for (int i = 100;; i--) {
		const Vec2V center = GetRandomPoint2D(bounds.GetMin(),bounds.GetMax());
		const Vec2V extent = GetRandomPoint2D(Vec2V(V_ZERO),Vec2V(maxRelativeSize)/boundsSize);
		const Box2V result(center - extent,center + extent);
		if (i > 0) {
			if (!Contains(bounds,result))
				continue;
			const Vec2V size = result.GetMax() - result.GetMin();
			const Vec2V relativeSize = size/boundsSize;
			if (Any(relativeSize < Vec2V(minRelativeSize)))
				continue;
			if (Any(relativeSize > Vec2V(maxRelativeSize)))
				continue;
			if (MaxElement(size)/MinElement(size) > maxAspect)
				continue;
		}
		return result;
	}
}

static const Circle2V GetRandomCircle2V(const Box2V& bounds,float minRelativeSize = 0.005f,float maxRelativeSize = 0.1f)
{
	const Vec2V boundsSize = bounds.GetMax() - bounds.GetMin();
	const float minRadius = minRelativeSize*MaxElement(boundsSize).f();
	const float maxRadius = maxRelativeSize*MinElement(boundsSize).f();
	ForceAssert(minRadius <= maxRadius);
	const float radius = GetRandomValueInRange(minRadius,maxRadius);
	const Vec2V center = GetRandomPoint2D(bounds.GetMin() + Vec2V(radius),bounds.GetMax() - Vec2V(radius));
	return Circle2V(center,radius);
}

static void RenderBox2V(const Box2V& bounds,Vec3V* image,int w,int h,const Box2V& box,Vec3V_arg color,float opacity)
{
	if (opacity <= 0.0f)
		return;
	const Vec2V bmin = (box.GetMin() - bounds.GetMin())/(bounds.GetMax() - bounds.GetMin()); // relative to bounds
	const Vec2V bmax = (box.GetMax() - bounds.GetMin())/(bounds.GetMax() - bounds.GetMin());
	const int i0 = Min(0,(int)floorf(0.5f + bmin.xf()*(float)w));
	const int j0 = Min(0,(int)floorf(0.5f + bmin.yf()*(float)h));
	const int i1 = Max((int)ceilf(0.5f + bmax.xf()*(float)w),w);
	const int j1 = Max((int)ceilf(0.5f + bmax.yf()*(float)h),h);
	for (int j = j0; j < j1; j++) {
		for (int i = i0; i < i1; i++) {
			Vec3V& c = image[i + j*w];
			c += (color - c)*opacity;
		}
	}
}

static void RenderCircle2V(const Box2V& bounds,Vec3V* image,int w,int h,const Circle2V& circle,Vec3V_arg color,float opacity)
{
	if (opacity <= 0.0f)
		return;
	const Box2V box(circle.GetCenter() - Vec2V(circle.GetRadius()),circle.GetCenter() + Vec2V(circle.GetRadius())); // box around circle
	const Vec2V bmin = (box.GetMin() - bounds.GetMin())/(bounds.GetMax() - bounds.GetMin()); // relative to bounds
	const Vec2V bmax = (box.GetMax() - bounds.GetMin())/(bounds.GetMax() - bounds.GetMin());
	const int i0 = Min(0,(int)floorf(0.5f + bmin.xf()*(float)w));
	const int j0 = Min(0,(int)floorf(0.5f + bmin.yf()*(float)h));
	const int i1 = Max((int)ceilf(0.5f + bmax.xf()*(float)w),w);
	const int j1 = Max((int)ceilf(0.5f + bmax.yf()*(float)h),h);
	for (int j = j0; j < j1; j++) {
		for (int i = i0; i < i1; i++) {
			const float x = ((float)i + 0.5f)/(float)w; // pixel center
			const float y = ((float)j + 0.5f)/(float)h;
			const Vec2V p = bounds.GetMin() + (bounds.GetMax() - bounds.GetMin())*Vec2V(x,y);
			if (Intersects(p,circle)) {
				Vec3V& c = image[i + j*w];
				c += (color - c)*opacity;
			}
		}
	}
}

static bool SaveImageVec3V(const char* path,const Vec3V* image,int w,int h)
{
	StaticAssert(sizeof(Vec3V) == sizeof(Vec4V));
	Vec4V* temp = new Vec4V[w*h];
	memcpy(temp,image,w*h*sizeof(Vec4V));
	for (int i = 0; i < w*h; i++)
		temp[i].wf_ref() = 1.0f;
	const bool result = SaveImage(path,(const Vec4V*)temp,w,h);
	delete[] temp;
	return result;
}

static void TestIntersectionCodeWithBox(Vec3V* image,int w,int h,const Box2V& bounds,const Box2V& testBox)
{
	for (int i = 0; i < w*h; i++)
		image[i] = Vec3V(V_ZERO);

	const Box2V testObject = testBox;
	RenderBox2V(bounds,image,w,h,testObject,Vec3V(1.0f,0.0f,0.0f),0.2f);

	for (int i = 0; i < 50; i++) {
		const Circle2V circle = GetRandomCircle2V(bounds);
		Vec3V color(0.0f,0.0f,1.0f);
		if (Contains(testObject,circle))
			color = Vec3V(1.0f,0.0f,1.0f);
		else if (Intersects(circle,testObject))
			color = Vec3V(1.0f,1.0f,0.0f);
		RenderCircle2V(bounds,image,w,h,circle,color,0.2f);
	}

	for (int i = 0; i < 50; i++) {
		const Box2V box = GetRandomBox2V(bounds);
		Vec3V color(0.0f,0.0f,1.0f);
		if (Contains(testObject,box))
			color = Vec3V(1.0f,0.0f,1.0f);
		else if (Intersects(testObject,box))
			color = Vec3V(1.0f,1.0f,0.0f);
		RenderBox2V(bounds,image,w,h,box,color,0.2f);
	}

	SaveImageVec3V("test_box.png",image,w,h);
}

static void TestIntersectionCodeWithCircle(Vec3V* image,int w,int h,const Box2V& bounds,const Box2V& testBox)
{
	for (int i = 0; i < w*h; i++)
		image[i] = Vec3V(V_ZERO);

	const Circle2V testObject(testBox.GetCenter(),MinElement(testBox.GetExtent()));
	RenderCircle2V(bounds,image,w,h,testObject,Vec3V(1.0f,0.0f,0.0f),0.2f);

	for (int i = 0; i < 50; i++) {
		const Circle2V circle = GetRandomCircle2V(bounds);
		Vec3V color(0.0f,0.0f,1.0f);
		if (Contains(testObject,circle))
			color = Vec3V(1.0f,0.0f,1.0f);
		else if (Intersects(testObject,circle))
			color = Vec3V(1.0f,1.0f,0.0f);
		RenderCircle2V(bounds,image,w,h,circle,color,0.2f);
	}

	for (int i = 0; i < 50; i++) {
		const Box2V box = GetRandomBox2V(bounds);
		Vec3V color(0.0f,0.0f,1.0f);
		if (Contains(testObject,box))
			color = Vec3V(1.0f,0.0f,1.0f);
		else if (Intersects(testObject,box))
			color = Vec3V(1.0f,1.0f,0.0f);
		RenderBox2V(bounds,image,w,h,box,color,0.2f);
	}

	SaveImageVec3V("test_circle.png",image,w,h);
}

void TestIntersectionCode()
{
	const int w = 2000;
	const int h = 1500;
	Vec3V* image = new Vec3V[w*h];

	const Box2V bounds(Vec2V(V_ZERO),Vec2V((float)w,(float)h));
	const Vec2V bmin(0.1f,0.2f);
	const Vec2V bmax(0.8f,0.7f);
	const Box2V testBox(
		bounds.GetMin() + bmin*(bounds.GetMax() - bounds.GetMin()),
		bounds.GetMin() + bmax*(bounds.GetMax() - bounds.GetMin()));

	TestIntersectionCodeWithBox(image,w,h,bounds,testBox);
	TestIntersectionCodeWithCircle(image,w,h,bounds,testBox);

	delete[] image;
}
#endif // VMATH_TEST_GEOM

#if VMATH_TEST_RANDOM
void TestXorShift()
{
	const uint32 x0 = 1234567; // starting value
	const uint32 count = 54321; // iteration count
	uint32 x1,x3;
	Vec4V x2;

	// ==============================

	x1 = x0;
	x2 = Vec4V(_mm_set1_epi32(x0));
	for (uint32 i = 0; i < count; i++) {
		XorShift_LRL_32_13_17_5(x1);
		XorShift4V_LRL_32_13_17_5(x2);
	}
	x3 = XorShift_LRL_32_13_17_5_SkipAhead(x0,count);
	
	printf("XorShift_LRL<32,13,17,5>\n");
	printf("\ttest1 = 0x%08x (sequential)\n",x1);
	printf("\ttest2 = 0x%08x (sequential vector)\n",*(const uint32*)&x2);
	printf("\ttest3 = 0x%08x (using GF2Matrix exp)\n",x3);
	ForceAssert(x1 == *(const uint32*)&x2);
	ForceAssert(x1 == x3);

	// ==============================

	x1 = x0;
	x2 = Vec4V(_mm_set1_epi32(x0));
	for (uint32 i = 0; i < count; i++) {
		XorShift_RLR_31_11_13_20(x1);
		XorShift4V_RLR_31_11_13_20(x2);
	}
	x3 = XorShift_RLR_31_11_13_20_SkipAhead(x0,count);

	printf("XorShift_RLR<31,11,13,20>\n");
	printf("\ttest1 = 0x%08x (sequential)\n",x1);
	printf("\ttest2 = 0x%08x (sequential vector)\n",*(const uint32*)&x2);
	printf("\ttest3 = 0x%08x (using GF2Matrix exp)\n",x3);
	ForceAssert(x1 == *(const uint32*)&x2);
	ForceAssert(x1 == x3);

	printf("\n");
}

void TestRandom()
{
	Vec4V state = XorShift4V_LRL_32_13_17_5_Init();
	const int w = 1024;
	const int h = 800;
	Vec3V* image = new Vec3V[w*h];
	for (int i = 0; i < w*h; i++)
		image[i] = XorShift4V_LRL_32_13_17_5(state).xyz();
	SaveImageVec3V("rng.png",image,w,h);
	delete[] image;
}
#endif // VMATH_TEST_RANDOM

#if VMATH_TEST_TRANSCENDENTAL
static void PrintFloatFullPrecision(char* str,float x)
{
	if (x >= 0.1f)
		sprintf(str,"%f",x);
	else {
		bool neg = false;
		if (*reinterpret_cast<const uint32*>(&x) >> 31) {
			neg = true;
			x = -x;
		}
		int zeros = 0;
		while (x < 0.1f) {
			x *= 10.0f;
			zeros++;
		}
		char temp[64];
		sprintf(temp,"%f",x);
		sprintf(str,"%s0.",neg ? "-" : "");
		for (int i = 0; i < zeros; i++)
			strcat(str,"0");
		strcat(str,temp + 2);
	}
}

template <unsigned order> static float _vmath_exp2_ps_test(unsigned count,int w = 4096,int h = 512)
{
	ProgressDisplay progress("testing _vmath_exp2_ps_<%u>",order);
	__m128 x = _mm_set1_ps(1.2345f);
	__m128 y = _mm_set1_ps(9.8765f);
	for (unsigned i = 0; i < count; i++) {
		x = _vmath_exp2_ps_<order>(x);
		x = _mm_min_ps(x,y);
	}
	progress.End();

	if (1) { // graph relative error
		double* g = new double[w];
		double gmin = FLT_MAX;
		double gmax = -FLT_MAX;
		for (int i = 0; i < w; i++) {
			const double x = 20.0*(double)i/(double)(w - 1);
			const double y1 = _vmath_extract_ps(_vmath_exp2_ps_<order>(_mm_set1_ps((float)x)),0);
			const double y2 = exp2(x);
			if (Abs(y2) > 0.00000001)
				g[i] = (y1 - y2)/y2; // relative error
			else
				g[i] = y1;
			gmin = Min(g[i],gmin);
			gmax = Max(g[i],gmax);
		}
		for (int i = 0; i < w; i++)
			g[i] = (g[i] - gmin)/(gmax - gmin);
		float* image = new float[w*h];
		ImageGraph(image,g,w,h);
		char path[512];
		char minstr[64]; PrintFloatFullPrecision(minstr,(float)gmin);
		char maxstr[64]; PrintFloatFullPrecision(maxstr,(float)gmax);
		sprintf(path,"_vmath_exp2_ps_order%u_min%s_max%s.png",order,minstr,maxstr);
		SaveImage(path,image,w,h);
		delete[] image;
		delete[] g;
	}
	return _vmath_extract_ps(x,0); // returned so compiler won't optimize this out
}

VMATH_INLINE static __m128 _vmath_exp2_ps_reference(__m128 x)
{
	return _mm_setr_ps(
		exp2f(_vmath_extract_ps(x,0)),
		exp2f(_vmath_extract_ps(x,1)),
		exp2f(_vmath_extract_ps(x,2)),
		exp2f(_vmath_extract_ps(x,3)));
}

#if HAS_VEC8V
template <unsigned order> static float _vmath256_exp2_ps_test(unsigned count)
{
	ProgressDisplay progress("testing _vmath256_exp2_ps_<%u>",order);
	__m256 x = _mm256_set1_ps(1.2345f);
	__m256 y = _mm256_set1_ps(9.8765f);
	for (unsigned i = 0; i < count; i++) {
		x = _vmath256_exp2_ps_<order>(x);
		x = _mm256_min_ps(x,y);
	}
	progress.End();
	return _vmath256_extract_ps(x,0); // returned so compiler won't optimize this out
}

VMATH_INLINE static __m256 _vmath256_exp2_ps_reference(__m256 x)
{
	return _mm256_setr_ps(
		exp2f(_vmath256_extract_ps(x,0)),
		exp2f(_vmath256_extract_ps(x,1)),
		exp2f(_vmath256_extract_ps(x,2)),
		exp2f(_vmath256_extract_ps(x,3)),
		exp2f(_vmath256_extract_ps(x,4)),
		exp2f(_vmath256_extract_ps(x,5)),
		exp2f(_vmath256_extract_ps(x,6)),
		exp2f(_vmath256_extract_ps(x,7)));
}
#endif // HAS_VEC8V

void TestExp2Performance()
{
	//testing _vmath_exp2_ps_<5> .. done. (868.781860 msecs)
	//testing _vmath_exp2_ps_<4> .. done. (879.821472 msecs)
	//testing _vmath_exp2_ps_<3> .. done. (651.185364 msecs)
	//testing _vmath_exp2_ps_<2> .. done. (560.202209 msecs)
	//testing _vmath_exp2_ps_reference .. done. (10.474182 secs)
	//testing _vmath256_exp2_ps_<5> .. done. (980.123474 msecs)
	//testing _vmath256_exp2_ps_<4> .. done. (967.450195 msecs)
	//testing _vmath256_exp2_ps_<3> .. done. (747.778748 msecs)
	//testing _vmath256_exp2_ps_<2> .. done. (686.952820 msecs)
	//testing _vmath256_exp2_ps_reference .. done. (21.306499 secs)
	//Press any key to continue . . .
	const unsigned count = 64*1000*1000;
	float result = 0.0f;
	result += _vmath_exp2_ps_test<5>(count);
	result += _vmath_exp2_ps_test<4>(count);
	result += _vmath_exp2_ps_test<3>(count);
	result += _vmath_exp2_ps_test<2>(count);
	{
		ProgressDisplay progress("testing _vmath_exp2_ps_reference");
		__m128 x = _mm_set1_ps(1.2345f);
		__m128 y = _mm_set1_ps(9.8765f);
		for (unsigned i = 0; i < count; i++) {
			x = _vmath_exp2_ps_reference(x);
			x = _mm_min_ps(x,y);
		}
		progress.End();
		result += _vmath_extract_ps(x,0);
	}
#if HAS_VEC8V
	result += _vmath256_exp2_ps_test<5>(count);
	result += _vmath256_exp2_ps_test<4>(count);
	result += _vmath256_exp2_ps_test<3>(count);
	result += _vmath256_exp2_ps_test<2>(count);
	{
		ProgressDisplay progress("testing _vmath256_exp2_ps_reference");
		__m256 x = _mm256_set1_ps(1.2345f);
		__m256 y = _mm256_set1_ps(9.8765f);
		for (unsigned i = 0; i < count; i++) {
			x = _vmath256_exp2_ps_reference(x);
			x = _mm256_min_ps(x,y);
		}
		progress.End();
		result += _vmath256_extract_ps(x,0);
	}
#endif // HAS_VEC8V
	if (result == 0.0f)
		printf("boom.\n");
}

// ================================================================================================

template <unsigned order> static float _vmath_log2_ps_test(unsigned count,int w = 4096,int h = 512)
{
	ProgressDisplay progress("testing _vmath_log2_ps_<%u>",order);
	__m128 x = _mm_set1_ps(1.2345f);
	__m128 y = _mm_set1_ps(9.8765f);
	for (unsigned i = 0; i < count; i++) {
		x = _vmath_log2_ps_<order>(x);
		x = _mm_min_ps(x,y);
	}
	progress.End();

	if (1) { // graph relative error
		double* g = new double[w];
		double gmin = FLT_MAX;
		double gmax = -FLT_MAX;
		for (int i = 0; i < w; i++) {
			const double x = 20.0*(double)i/(double)(w - 1);
			const double y1 = _vmath_extract_ps(_vmath_log2_ps_<order>(_mm_set1_ps((float)x)),0);
			const double y2 = log2(x);
			if (Abs(y2) > 0.00000001)
				g[i] = (y1 - y2)/y2; // relative error
			else
				g[i] = y1;
			gmin = Min(g[i],gmin);
			gmax = Max(g[i],gmax);
		}
		for (int i = 0; i < w; i++)
			g[i] = (g[i] - gmin)/(gmax - gmin);
		float* image = new float[w*h];
		ImageGraph(image,g,w,h);
		char path[512];
		char minstr[64]; PrintFloatFullPrecision(minstr,(float)gmin);
		char maxstr[64]; PrintFloatFullPrecision(maxstr,(float)gmax);
		sprintf(path,"_vmath_log2_ps_order%u_min%s_max%s.png",order,minstr,maxstr);
		SaveImage(path,image,w,h);
		delete[] image;
		delete[] g;
	}
	return _vmath_extract_ps(x,0); // returned so compiler won't optimize this out
}

VMATH_INLINE static __m128 _vmath_log2_ps_reference(__m128 x)
{
	return _mm_setr_ps(
		log2f(_vmath_extract_ps(x,0)),
		log2f(_vmath_extract_ps(x,1)),
		log2f(_vmath_extract_ps(x,2)),
		log2f(_vmath_extract_ps(x,3)));
}

#if HAS_VEC8V
template <unsigned order> static float _vmath256_log2_ps_test(unsigned count)
{
	ProgressDisplay progress("testing _vmath256_log2_ps_<%u>",order);
	__m256 x = _mm256_set1_ps(1.2345f);
	__m256 y = _mm256_set1_ps(9.8765f);
	for (unsigned i = 0; i < count; i++) {
		x = _vmath256_log2_ps_<order>(x);
		x = _mm256_min_ps(x,y);
	}
	progress.End();
	return _vmath256_extract_ps(x,0); // returned so compiler won't optimize this out
}

VMATH_INLINE static __m256 _vmath256_log2_ps_reference(__m256 x)
{
	return _mm256_setr_ps(
		log2f(_vmath256_extract_ps(x,0)),
		log2f(_vmath256_extract_ps(x,1)),
		log2f(_vmath256_extract_ps(x,2)),
		log2f(_vmath256_extract_ps(x,3)),
		log2f(_vmath256_extract_ps(x,4)),
		log2f(_vmath256_extract_ps(x,5)),
		log2f(_vmath256_extract_ps(x,6)),
		log2f(_vmath256_extract_ps(x,7)));
}
#endif // HAS_VEC8V

void TestLog2Performance()
{
	//Press any key to continue . . .
	const unsigned count = 64*1000*1000;
	float result = 0.0f;
	result += _vmath_log2_ps_test<5>(count);
	result += _vmath_log2_ps_test<4>(count);
	result += _vmath_log2_ps_test<3>(count);
	result += _vmath_log2_ps_test<2>(count);
	{
		ProgressDisplay progress("testing _vmath_log2_ps_reference");
		__m128 x = _mm_set1_ps(1.2345f);
		__m128 y = _mm_set1_ps(9.8765f);
		for (unsigned i = 0; i < count; i++) {
			x = _vmath_log2_ps_reference(x);
			x = _mm_max_ps(x,y);
		}
		progress.End();
		result += _vmath_extract_ps(x,0);
	}
#if HAS_VEC8V
	result += _vmath256_log2_ps_test<5>(count);
	result += _vmath256_log2_ps_test<4>(count);
	result += _vmath256_log2_ps_test<3>(count);
	result += _vmath256_log2_ps_test<2>(count);
	{
		ProgressDisplay progress("testing _vmath256_log2_ps_reference");
		__m256 x = _mm256_set1_ps(1.2345f);
		__m256 y = _mm256_set1_ps(9.8765f);
		for (unsigned i = 0; i < count; i++) {
			x = _vmath256_log2_ps_reference(x);
			x = _mm256_max_ps(x,y);
		}
		progress.End();
		result += _vmath256_extract_ps(x,0);
	}
#endif // HAS_VEC8V
	if (result == 0.0f)
		printf("boom.\n");
}

// ================================================================================================

static float _vmath_sinPI_ps_test(unsigned count,int w = 4096,int h = 512)
{
	ProgressDisplay progress("testing _vmath_sinPI_ps");
	__m128 x = _mm_set1_ps(0.2345f);
	for (unsigned i = 0; i < count; i++)
		x = _vmath_sinPI_ps(x);
	progress.End();

	if (1) { // graph relative error
		double* g = new double[w];
		double gmin = FLT_MAX;
		double gmax = -FLT_MAX;
		for (int i = 0; i < w; i++) {
			const double x = -1.0 + 2.0*(double)i/(double)(w - 1);
			const double y1 = _vmath_extract_ps(_vmath_sinPI_ps(_mm_set1_ps((float)x)),0);
		//	const double y1 = _vmath_sinPI((float)x);
			const double y2 = sin(x*3.14159265358979323846264338327950288419716939937510582); // high precision PI
			if (Abs(y2) > 0.00000001)
				g[i] = (y1 - y2)/y2; // relative error
			else
				g[i] = y1;
			gmin = Min(g[i],gmin);
			gmax = Max(g[i],gmax);
		}
		for (int i = 0; i < w; i++)
			g[i] = (g[i] - gmin)/(gmax - gmin);
		float* image = new float[w*h];
		ImageGraph(image,g,w,h);
		char path[512];
		char minstr[64]; PrintFloatFullPrecision(minstr,(float)gmin);
		char maxstr[64]; PrintFloatFullPrecision(maxstr,(float)gmax);
		sprintf(path,"_vmath_sinPI_ps_min%s_max%s.png",minstr,maxstr);
		SaveImage(path,image,w,h);
		delete[] image;
		delete[] g;
	}
	return _vmath_extract_ps(x,0); // returned so compiler won't optimize this out
}

VMATH_INLINE static __m128 _vmath_sinPI_ps_reference(__m128 x)
{
	x = _mm_mul_ps(x,_mm_set1_ps(PI));
	return _mm_setr_ps(
		sinf(_vmath_extract_ps(x,0)),
		sinf(_vmath_extract_ps(x,1)),
		sinf(_vmath_extract_ps(x,2)),
		sinf(_vmath_extract_ps(x,3)));
}

#if HAS_VEC8V
static float _vmath256_sinPI_ps_test(unsigned count)
{
	ProgressDisplay progress("testing _vmath256_sinPI_ps");
	__m256 x = _mm256_set1_ps(0.2345f);
	for (unsigned i = 0; i < count; i++)
		x = _vmath256_sinPI_ps(x);
	progress.End();
	return _vmath256_extract_ps(x,0); // returned so compiler won't optimize this out
}

VMATH_INLINE static __m256 _vmath256_sinPI_ps_reference(__m256 x)
{
	x = _mm256_mul_ps(x,_mm256_set1_ps(PI));
	return _mm256_setr_ps(
		sinf(_vmath256_extract_ps(x,0)),
		sinf(_vmath256_extract_ps(x,1)),
		sinf(_vmath256_extract_ps(x,2)),
		sinf(_vmath256_extract_ps(x,3)),
		sinf(_vmath256_extract_ps(x,4)),
		sinf(_vmath256_extract_ps(x,5)),
		sinf(_vmath256_extract_ps(x,6)),
		sinf(_vmath256_extract_ps(x,7)));
}
#endif // HAS_VEC8V

void TestSinPIPerformance()
{
	const unsigned count = 64*1000*1000;
	float result = 0.0f;
	result += _vmath_sinPI_ps_test(count);
	{
		ProgressDisplay progress("testing _vmath_sinPI_ps_reference");
		__m128 x = _mm_set1_ps(0.2345f);
		for (unsigned i = 0; i < count; i++)
			x = _vmath_sinPI_ps_reference(x);
		progress.End();
		result += _vmath_extract_ps(x,0);
	}
	{
		ProgressDisplay progress("testing _vmath_sinPI (float)");
		float x = 0.2345f;
		for (unsigned i = 0; i < count; i++)
			x = _vmath_sinPI(x);
		progress.End();
		result += x;
	}
	{
		ProgressDisplay progress("testing sinf(x*PI) (float)");
		float x = 0.2345f;
		for (unsigned i = 0; i < count; i++)
			x = sinf(x*PI);
		progress.End();
		result += x;
	}

#if HAS_VEC8V
	result += _vmath256_sinPI_ps_test(count);
	{
		ProgressDisplay progress("testing _vmath256_sinPI_ps_reference");
		__m256 x = _mm256_set1_ps(1.2345f);
		for (unsigned i = 0; i < count; i++)
			x = _vmath256_sinPI_ps_reference(x);
		progress.End();
		result += _vmath256_extract_ps(x,0);
	}
#endif // HAS_VEC8V
	if (result == 0.0f)
		printf("boom.\n");
}

void TestSinCos()
{
	const int n = 10000;
	float maxerr = 0.0f;
	float maxrel = 0.0f;
	for (int i = 0; i < n; i++) {
		const float x = -0.5f*PI + PI*(float)i/(float)(n - 1);
		const float y1 = FastSinPI(Vec3V(1.0f,2.0f,x)).zf();
		const float y2 = sinf(x);
		const float err = Abs(y1 - y2);
		maxerr = Max(err,maxerr);
		maxrel = Max(err/Abs(y2),maxrel);
	}
	printf("FastSin maxerr = %f (relative = %f)\n",maxerr,maxrel);
}
#endif // VMATH_TEST_TRANSCENDENTAL

#if VMATH_TEST_INTEGER_DIV
bool TestIntegerDiv(uint32 q_min,uint32 q_max,uint32 q,uint32 p_bits,uint32 fixedShift,bool silent)
{
	// ===========================================================================================================================
	// notes:
	// the purpose of this code is to find constant integer values M,S such that we can compute p/q without division as (p*M) >> S.
	// assuming q is fixed, but p can vary between [0 .. 2^p_bits), we find M,S as a function of q and p_bits.
	// for example, for the first few small q we can compute:
	//   p/3 = (p*0x55555556) >> 32 for p < 2^32
	//   p/5 = (p*0x33333334) >> 32 for p < 2^31
	//   p/7 = (p*0x24924925) >> 32 for p < 2^31
	//   p/11= (p*0x1745D175) >> 32 for p < 2^30
	// ideally, we would like the constant M to be as small as possible (see 'optimize' in IntegerDiv_GetMultiplier)
	// and in some cases we may require the shift amount M to be a specific value (e.g. 32)
	// it turns out that the constant M tends to be as many bits as p_bits, but not exactly.
	// the code here does not necessarily produce completely optimal values for M,S. and it actually fails in some
	// cases when shift override is specified.
	// also, the optimize function isn't quite optimal - in many cases the value M could be modified slightly without
	// affecting the results (this might imply that M could be shifted).
	// set fixedShift = 32 to test integer division appropriate for SIMD
	// ===========================================================================================================================
	const bool skipPow2 = false;
	uint32 failed = 0;
	if (q == 0) {
		if (fixedShift != 0)
			printf("WARNING: overriding shift amount to %u - not all q,p_bits combinations work!\n",fixedShift);
		for (q = q_min; q <= q_max; q++) {
			if ((q & (q - 1)) == 0 && skipPow2)
				continue; // boring.
			if (!silent)
				printf("testing q=%u:\n",q);
			bool last = false;
			for (p_bits = 3; p_bits <= 32 && !last; p_bits++) {
				if ((q & (q - 1)) == 0) {
					if (p_bits > 16)
						break; // ok, *really* boring.
				} else if (fixedShift != 0) {
					p_bits = fixedShift;
					last = true; // we only care about p_bits = fixedShift
				}
				const bool passed = TestIntegerDiv(0,0,q,p_bits,fixedShift,silent);
				if (!passed)
					break;
			}
		}
		if (!silent)
			system("pause");
	} else {
		const uint32 original_p_bits = p_bits;
		uint32 s = IntegerDiv_GetShift(q,p_bits);
		uint32 m;
		if (fixedShift != 0) { // force the shift amount to a specific value (e.g. 32)
			if (s < fixedShift) {
				m = IntegerDiv_GetMultiplier(q,s,false);
				if (!silent && 0)
					printf("shift was %u, overriding to %u .. m was 0x%08x, now 0x%08x\n",s,fixedShift,m,m << (fixedShift - s));
				m <<= (fixedShift - s);
			} else {
				int d_p_bits = 0;//int(s - fixedShift);
				d_p_bits = Max(0,d_p_bits - 1);
				p_bits -= d_p_bits; // will only be valid for this many bits ..
				m = IntegerDiv_GetMultiplier(q,fixedShift,false);
				if (s > fixedShift && !silent && 0)
					printf("shift was %u, overriding to %u .. m is 0x%08x, p_bits decreased by %d\n",s,fixedShift,m,d_p_bits);
			}
			s = fixedShift;
		} else
			m = IntegerDiv_GetMultiplier(q,s,true);
		char temp[256] = "";
		if (!silent) {
			char temp2[32] = "";
			if (original_p_bits != p_bits)
				sprintf(temp2,"(%u)",original_p_bits);
			sprintf(temp,"testing q=%u, multiplier=%u(%08x), shift=%u, p_bits=%u%s .. ",q,m,m,s,p_bits,temp2);
			fprintf(stdout,"%s",temp);
			fflush(stdout);
		}
		uint64 p_max = (1ULL << (p_bits - 1)) - 1;
		for (uint64 p = 0; p <= p_max; p++) {
		//for (uint64 p = p_max; p <= p_max; p--) {
			const uint32 r = (uint32)((p*m) >> s); // that's it!
			if (r != p/q) {
				if (!silent) {
					// hmm ..
					// well, what is the smallest value p such that p*m >> s is not equal to p/q?
					fprintf(stdout,"[0x%08x : %u] %u/%u -> %u, should be %u!\n",(uint32)p,(uint32)(p_max - p),(uint32)p,q,r,(uint32)p/q);
					const uint32 maxFailed = 1;
					if (++failed < maxFailed) {
						fprintf(stdout,"%s",temp);
						fflush(stdout);
						continue;
					}
				}
				return false;
			}
		}
	}
	fprintf(stdout,"done.\n");
	return failed == 0;
}
#endif // VMATH_TEST_INTEGER_DIV

#if VMATH_TEST_HALTON_SEQ
void TestHaltonSequenceGenerator()
{
	const uint32 primes[] = {2,3,5,7,11,13};
	const uint32 start = 0x12345678;
	const uint32 count = 0x100000;
	float* h1 = new float[count];
	float* h2 = new float[count];
	for (uint32 i = 0; i < countof(primes); i++) {
		const uint32 b = primes[i];
		bool failed = false;

		// reference
		fprintf(stdout,"generating halton sequence for b=%u (reference) .. ",b);
		fflush(stdout);
		uint64 time0 = ProgressDisplay::GetCurrentPerformanceTime();
		GenerateHaltonSequence(h1,start,count,b,false);
		const float dtime0 = ProgressDisplay::GetDeltaTimeInSeconds(time0);
		fprintf(stdout,"%f msecs per 1000 samples\n",1000.0f*1000.0f*dtime0/(float)count);

		// vectorized
		fprintf(stdout,"generating halton sequence for b=%u (vectorized) .. ",b);
		fflush(stdout);
		uint64 time1 = ProgressDisplay::GetCurrentPerformanceTime();
		GenerateHaltonSequence(h2,start,count,b,true);
		const float dtime1 = ProgressDisplay::GetDeltaTimeInSeconds(time1);
		char msg[256] = "";
		if (dtime1 <= dtime0)
			sprintf(msg," (speedup factor = x%.4f)",dtime0/dtime1);
		else
			sprintf(msg," (SLOWER!)"); // boo.
		fprintf(stdout,"%f msecs per 1000 samples%s\n",1000.0f*1000.0f*dtime1/(float)count,msg);

		if (memcmp(h1,h2,count*sizeof(float)) != 0) {
			uint32 index = 0;
			while (h1[index] == h2[index] && index < count)
				index++;
			printf("halton sequence failed for b=%u! index=%u:",b,index);
			printf("\n\tref: ");
			for (uint32 k = index; k < Min(index + 10,count); k++)
				printf("%s%.4f",k > index ? ", " : "",h1[k]);
			printf("\n\tvec: ");
			for (uint32 k = index; k < Min(index + 10,count); k++)
				printf("%s%.4f",k > index ? ", " : "",h2[k]);
			printf("\n");
			failed = true;
		}
		if (!failed)
			printf("halton sequence ok for b=%u.\n",b);
	}
	delete[] h1;
	delete[] h2;
	system("pause");
}
#endif // VMATH_TEST_HALTON_SEQ

#endif // VMATH_TEST
