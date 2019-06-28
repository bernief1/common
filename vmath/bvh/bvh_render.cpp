// =========================
// common/bvh/bvh_render.cpp
// =========================

#include "common/common.h"

#if PLATFORM_PC
#include <windows.h> // WTF why why why why??
#endif // PLATFORM_PC

#include "GraphicsTools/util/imageutil.h"
#include "GraphicsTools/util/memory.h"
#include "GraphicsTools/util/mesh.h"
#include "GraphicsTools/util/progressdisplay.h"

#include "vmath/bvh/bvh.h"
#include "vmath/bvh/bvh_render.h"

#include "vmath/vmath.h"
#include "vmath/vmath_sampling.h"
#include "vmath/vmath_soa.h"

#if PLATFORM_PC && defined(_OFFLINETOOL)
#define USE_XXX_SAVE_IMAGE (0)
#define USE_XXX_MULTITASK (0)
#elif XXX_GAME
#define USE_XXX_SAVE_IMAGE (1)
#define USE_XXX_MULTITASK (1)
#endif

#if USE_XXX_MULTITASK
#include "Utility/MultiTaskMan.h"
#endif // USE_XXX_MULTITASK

#if defined(_EMBREE)
#include "../../../embree-2.17.2/include/embree2/rtcore_ray.h"
#endif // defined(_EMBREE)

#if USE_XXX_SAVE_IMAGE
#include "engine/ScreenCap.h"
static bool SaveImage32(const char* path_, const float* image, int w, int h)
{
	char path[512];
	if (strnicmp(path_, "/app0/", strlen("/app0/")) == 0)
		strcpy(path, path_ + strlen("/app0/"));
	else
		strcpy(path, path_);
	uint32* image32 = new uint32[w*h];
	for (int i = 0; i < w*h; i++)
		image32[i] = 0xFF000000 | 0x00010101*(uint8)(0.5f + 255.0f*image[i]);
	const bool result = Engine::ScreenGrabSaver::SaveImage(path, image32, w, h, w*sizeof(uint32), false);
	delete[] image32;
	return result;
}
#else
#define SameImage32 SaveImage
#endif

using namespace geomesh;

static void ClearZBuffer(float* zbuf, unsigned w, unsigned h)
{
	for (unsigned i = 0; i < w*h; i++)
		zbuf[i] = ZBUFFER_DEFAULT;
}

template <unsigned packetW, unsigned packetH> static void UnswizzleZBuffer(float* zbuf, unsigned w, unsigned h) {}

template <> void UnswizzleZBuffer<2,2>(float* zbuf, unsigned w, unsigned h)
{
	const size_t linesize = sizeof(uint64)*w/2;
	uint64* zbuftemp = reinterpret_cast<uint64*>(alloca(linesize));
	uint64* zbufdst0 = reinterpret_cast<uint64*>(zbuf);
	uint64* zbufdst1 = zbufdst0 + w/2;
	const uint64* zbufsrc = zbufdst0;
	for (unsigned j = 0; j < h; j += 2) {
		for (unsigned i = 0; i < w/2; i++) {
			zbufdst0[i] = *zbufsrc++;
			zbuftemp[i] = *zbufsrc++;
		}
		memcpy(zbufdst1, zbuftemp, linesize);
		zbufdst0 += w;
		zbufdst1 += w;
	}
}

template <> void UnswizzleZBuffer<4,2>(float* zbuf, unsigned w, unsigned h)
{
	struct uint128 { uint64 a,b; }; // lame, windows has no 128-bit integer type
	const size_t linesize = sizeof(uint128)*w/4;
	uint128* zbuftemp = reinterpret_cast<uint128*>(alloca(linesize));
	uint128* zbufdst0 = reinterpret_cast<uint128*>(zbuf);
	uint128* zbufdst1 = zbufdst0 + w/4;
	const uint128* zbufsrc = zbufdst0;
	for (unsigned j = 0; j < h; j += 2) {
		for (unsigned i = 0; i < w/4; i++) {
			zbufdst0[i] = *zbufsrc++;
			zbuftemp[i] = *zbufsrc++;
		}
		memcpy(zbufdst1, zbuftemp, linesize);
		zbufdst0 += w/2;
		zbufdst1 += w/2;
	}
}

static void NormalizeAndSaveZBufferImage(float* zbuf, unsigned w, unsigned h, float& zscale, float& zoffset, bool& calc_z_range, const char* path, const char* suffix)
{
	if (path == NULL) // cases where i'm using the render function to get actual z values, not image values ..
		return;
	if (calc_z_range) {
		float zbufMin = +FLT_MAX;
		float zbufMax = -FLT_MAX;
		for (unsigned i = 0; i < w*h; i++) {
			const float z = zbuf[i];
			if (z != ZBUFFER_DEFAULT) {
				zbufMin = Min(z, zbufMin);
				zbufMax = Max(z, zbufMax);
			}
		}
		printf("zbuffer range %f..%f\n", zbufMin, zbufMax);
		if (zbufMin < zbufMax) {
			zscale = 1.0f/(zbufMax - zbufMin);
			zoffset = -zbufMin*zscale;
		}
		calc_z_range = false;
	}
	for (unsigned i = 0; i < w*h; i++) {
		float& z = zbuf[i];
		if (z != ZBUFFER_DEFAULT)
			z = 1.0f - (zoffset + zscale*z);
		else
			z = 0.5f; // background
	}
	if (path) {
		char path2[512];
	#if defined(_FREEIMAGE) || defined(SN_TARGET_PS4)
		const char* imageext = ".png";
	#else
		const char* imageext = ".dds";
	#endif
		strcpy(path2, path);
		strcpy(strrchr(path2, '.'), suffix);
		strcat(path2, imageext);
		SaveImage(path2, zbuf, w, h);
	}
}

template <unsigned packetW, unsigned packetH> static void RenderTriangles(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path)
{
	const unsigned packetSize = packetW*packetH;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType PacketType;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType::ComponentType ComponentType;
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dirPacketStepX = dirStepX*(float)packetW;
	const Vec3V dirPacketStepY = dirStepY*(float)packetH;
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dv[packetSize];
	for (unsigned j = 0; j < packetH; j++)
		for (unsigned i = 0; i < packetW; i++)
			dv[i + j*packetW] = dir00 + dirStepX*(float)i + dirStepY*(float)j;
	PacketType dirRowStart(dv);
	ComponentType* zptr = reinterpret_cast<ComponentType*>(zbuf);
	ForceAssert((reinterpret_cast<uintptr_t>(zptr) & (packetSize*sizeof(float) - 1)) == 0);
	ProgressDisplay progress("rendering %ux%u ray packets", packetW, packetH);
	for (unsigned j = 0; j < h; j += packetH) {
		PacketType dir = dirRowStart;
		for (unsigned i = 0; i < w; i += packetW) {
			for (size_t k = 0; k < triangles.size(); k++) {
				ComponentType t;
				if (triangles[k].IntersectsRay(origin, dir, t, (1 << packetSize) - 1))
					*zptr = Min(t, *zptr);
			}
			dir += dirPacketStepX;
			zptr++;
		}
		dirRowStart += dirPacketStepY;
	}
	UnswizzleZBuffer<packetW,packetH>(zbuf, w, h);
	progress.End();
	char ext[64];
	sprintf(ext, "_%ux%u", packetW, packetH);
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}

template <> void RenderTriangles<1,1>(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path)
{
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dirRowStart = dir00;
	float* zptr = zbuf;
	ProgressDisplay progress("rendering 1x1 ray packets");
	for (unsigned j = 0; j < h; j++) {
		Vec3V dir = dirRowStart;
		for (unsigned i = 0; i < w; i++) {
			for (size_t k = 0; k < triangles.size(); k++) {
				float t,u,v;
				if (triangles[k].IntersectsRay(origin, dir, t, u, v))
					*zptr = Min(t, *zptr);
			}
			dir += dirStepX;
			zptr++;
		}
		dirRowStart += dirStepY;
	}
	progress.End();
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, "_1x1");
}

#define DEF_RENDER_TRIANGLES(packetW,packetH) \
void RenderTriangles_##packetW##x##packetH(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path) \
{ \
	RenderTriangles<packetW,packetH>(triangles,camera,tanVFOV,zbuf,w,h,zclear,zscale,zoffset,calc_z_range,path); \
}
DEF_RENDER_TRIANGLES(1,1)
DEF_RENDER_TRIANGLES(2,2)
DEF_RENDER_TRIANGLES(4,1)
#if HAS_VEC8V
DEF_RENDER_TRIANGLES(8,1)
DEF_RENDER_TRIANGLES(4,2)
#endif // HAS_VEC8V
#undef DEF_RENDER_TRIANGLES

template <unsigned N> static void RenderTriangles_triN(const std::vector<typename Triangle3V_SOA_T<N>::TriangleType>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path)
{
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dirRowStart = dir00;
	float* zptr = zbuf;
	ProgressDisplay progress("rendering 1x1 ray packets with %u-triangle packets", N);
	for (unsigned j = 0; j < h; j++) {
		Vec3V dir = dirRowStart;
		for (unsigned i = 0; i < w; i++) {
			for (size_t k = 0; k < triangles.size(); k++) {
				ScalarV t;
				if (triangles[k].IntersectsRay(origin, dir, t))
					*zptr = Min(t.f(), *zptr);
			}
			dir += dirStepX;
			zptr++;
		}
		dirRowStart += dirStepY;
	}
	progress.End();
	char ext[64];
	sprintf(ext, "_TRI%u", N);
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}

#define DEF_RENDER_TRIANGLES_TRI_N(type,N) \
void RenderTriangles_tri##N(const std::vector<type>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path) \
{ \
	RenderTriangles_triN<N>(triangles,camera,tanVFOV,zbuf,w,h,zclear,zscale,zoffset,calc_z_range,path); \
}
DEF_RENDER_TRIANGLES_TRI_N(Triangle3V_SOA4,4)
#if HAS_VEC8V
DEF_RENDER_TRIANGLES_TRI_N(Triangle3V_SOA8,8)
#endif // HAS_VEC8V
#undef DEF_RENDER_TRIANGLES_TRI_N

#if BVH_THREADS
template <typename NodeType,typename PacketType,typename ComponentType> class RenderTriangles_BVH_LocalData_T
{
public:
	void Render()
	{
		const unsigned packetSize = packetW*packetH;
		const unsigned packetMask = (1 << packetSize) - 1;
		for (unsigned j = 0; j < h; j += packetH) {
			PacketType dir = dirRowStart;
			for (unsigned i = 0; i < w; i += packetW) {
				root->Trace(origin, dir, *zptr++ BVH_STATS_ONLY(, packetMask, *stats));
				dir += dirPacketStepX;
			}
			dirRowStart += dirPacketStepY;
		}
	}
#if PLATFORM_PC
	static DWORD WINAPI StaticRender(LPVOID param) { reinterpret_cast<RenderTriangles_BVH_LocalData_T*>(param)->Render(); return 0; }
#elif USE_XXX_MULTITASK
	static bool StaticRender(MultiTask task) { reinterpret_cast<RenderTriangles_BVH_LocalData_T*>(task)->Render(); return true; }
#endif
	const NodeType* root;
	unsigned w;
	unsigned h;
	unsigned packetW;
	unsigned packetH;
	ComponentType* zptr;
	Vec3V origin;
	PacketType dirRowStart;
	PacketType dirPacketStepX;
	PacketType dirPacketStepY;
	BVH_STATS_ONLY(BVHStats* stats);
};
#endif // BVH_THREADS

template <typename NodeType,unsigned packetW,unsigned packetH> class RenderTriangles_BVH { public: static void func(const NodeType* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads))
{
	const unsigned packetSize = packetW*packetH;
	const unsigned packetMask = (1 << packetSize) - 1;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType PacketType;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType::ComponentType ComponentType;
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dirPacketStepX = dirStepX*(float)packetW;
	const Vec3V dirPacketStepY = dirStepY*(float)packetH;
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dv[packetSize];
	for (unsigned j = 0; j < packetH; j++)
		for (unsigned i = 0; i < packetW; i++)
			dv[i + j*packetW] = dir00 + dirStepX*(float)i + dirStepY*(float)j;
	PacketType dirRowStart(dv);
	ComponentType* zptr = reinterpret_cast<ComponentType*>(zbuf);
	ForceAssert((reinterpret_cast<uintptr_t>(zptr) & (packetSize*sizeof(float) - 1)) == 0);
	BVH_STATS_ONLY(BVHStats stats);
#if BVH_THREADS
	ProgressDisplay progress("rendering %s - %ux%u ray packets (%u threads)", NodeType::GetClassName_(), packetW, packetH, numThreads);
#else
	ProgressDisplay progress("rendering %s - %ux%u ray packets", NodeType::GetClassName_(), packetW, packetH);
#endif
#if BVH_THREADS
	if (numThreads > 0) {
		typedef RenderTriangles_BVH_LocalData_T<NodeType,PacketType,ComponentType> LocalData;
		LocalData* data = AlignedAlloc<LocalData>(numThreads, 32);
	#if PLATFORM_PC
		HANDLE* threads = new HANDLE[numThreads];
	#elif USE_XXX_MULTITASK
		MultiTask_InitQueue();
	#endif
		unsigned stripW = w;
		unsigned stripH = (h + numThreads - 1)/numThreads; // break into numThreads strips (last strip may be too long)
		stripH = ((stripH + packetH - 1)/packetH)*packetH; // round up to packetH
		ForceAssert(stripH%packetH == 0);
		unsigned j = 0; // track the number of pixel rows for all strips
		for (unsigned i = 0; i < numThreads; i++) {
			data[i].root = root;
			data[i].w = stripW;
			data[i].h = Min(stripH, h - j); j += data[i].h;
			data[i].packetW = packetW;
			data[i].packetH = packetH;
			data[i].zptr = zptr; zptr += (w/packetW)*(stripH/packetH);
			data[i].origin = origin;
			data[i].dirRowStart = dirRowStart; dirRowStart += dirPacketStepY*(float)(stripH/packetH);
			data[i].dirPacketStepX = dirPacketStepX;
			data[i].dirPacketStepY = dirPacketStepY;
			BVH_STATS_ONLY(data[i].stats = new BVHStats);
		#if PLATFORM_PC
			threads[i] = CreateThread(NULL, 0, LocalData::StaticRender, &data[i], 0, NULL);
		#elif USE_XXX_MULTITASK
			MultiTask_AddTask((MultiTask)&data[i]);
		#endif
		}
		ForceAssert(j == h);
	#if PLATFORM_PC
		WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);
	#elif USE_XXX_MULTITASK
		MultiTask_ProcessQueue(LocalData::StaticRender);
	#endif
		for (unsigned i = 0; i < numThreads; i++) {
		#if PLATFORM_PC
			CloseHandle(threads[i]);
		#endif // PLATFORM_PC
			BVH_STATS_ONLY(stats += *data[i].stats);
			BVH_STATS_ONLY(delete data[i].stats);
		}
		AlignedFree(data);
	#if PLATFORM_PC
		delete[] threads;
	#endif // PLATFORM_PC
	} else
#endif // BVH_THREADS
	for (unsigned j = 0; j < h; j += packetH) {
		PacketType dir = dirRowStart;
		for (unsigned i = 0; i < w; i += packetW) {
			root->Trace(origin, dir, *zptr++ BVH_STATS_ONLY(, packetMask, stats));
			dir += dirPacketStepX;
		}
		dirRowStart += dirPacketStepY;
	}
	UnswizzleZBuffer<packetW,packetH>(zbuf, w, h);
	const float raysPerSecond = ((float)(w*h))/progress.GetTimeInSeconds();
#if BVH_STATS
	progress.End("%.4f Mrays/sec (nodes=%zd,leaves=%zd,tris=%zd)", raysPerSecond/1000000.0f, stats.m_nodeCount, stats.m_leafCount, stats.m_triCount);
	stats.Report(packetSize);
#else
	progress.End("%.4f Mrays/sec", raysPerSecond/1000000.0f);
#endif
	char ext[64];
#if BVH_THREADS
	sprintf(ext, "_%s_%ux%u_%02u_threads", NodeType::GetClassName_(), packetW, packetH, numThreads);
#else
	sprintf(ext, "_%s_%ux%u", NodeType::GetClassName_(), packetW, packetH);
#endif
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}};

template <typename NodeType> class RenderTriangles_BVH<NodeType,1,1> { public: static void func(const NodeType* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads))
{
	const unsigned packetMask = 0x0001;
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dirRowStart = dir00;
	float* zptr = zbuf;
	BVH_STATS_ONLY(BVHStats stats);
#if BVH_THREADS
	ProgressDisplay progress("rendering %s - 1x1 ray packets (%u threads)", NodeType::GetClassName_(), numThreads);
#else
	ProgressDisplay progress("rendering %s - 1x1 ray packets", NodeType::GetClassName_());
#endif
#if BVH_THREADS
	if (numThreads > 0) {
		typedef RenderTriangles_BVH_LocalData_T<NodeType,Vec3V,float> LocalData;
		LocalData* data = AlignedAlloc<LocalData>(numThreads, 32);
	#if PLATFORM_PC
		HANDLE* threads = new HANDLE[numThreads];
	#elif USE_XXX_MULTITASK
		MultiTask_InitQueue();
	#endif
		unsigned stripW = w;
		unsigned stripH = (h + numThreads - 1)/numThreads; // break into numThreads strips (last strip may be too long)
		unsigned j = 0;
		for (unsigned i = 0; i < numThreads; i++) {
			data[i].root = root;
			data[i].w = stripW;
			data[i].h = Min(stripH, h - j); j += data[i].h;
			data[i].packetW = 1;
			data[i].packetH = 1;
			data[i].zptr = zptr; zptr += w*stripH;
			data[i].origin = origin;
			data[i].dirRowStart = dirRowStart; dirRowStart += dirStepY*(float)stripH;
			data[i].dirPacketStepX = dirStepX;
			data[i].dirPacketStepY = dirStepY;
			BVH_STATS_ONLY(data[i].stats = new BVHStats);
		#if PLATFORM_PC
			threads[i] = CreateThread(NULL, 0, LocalData::StaticRender, &data[i], 0, NULL);
		#elif USE_XXX_MULTITASK
			MultiTask_AddTask((MultiTask)&data[i]);
		#endif
		}
		ForceAssert(j == h);
	#if PLATFORM_PC
		WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);
	#elif USE_XXX_MULTITASK
		MultiTask_ProcessQueue(LocalData::StaticRender);
	#endif
		for (unsigned i = 0; i < numThreads; i++) {
		#if PLATFORM_PC
			CloseHandle(threads[i]);
		#endif // PLATFORM_PC
			BVH_STATS_ONLY(stats += *data[i].stats);
			BVH_STATS_ONLY(delete data[i].stats);
		}
		AlignedFree(data);
	#if PLATFORM_PC
		delete[] threads;
	#endif // PLATFORM_PC
	} else
#endif // BVH_THREADS
	for (unsigned j = 0; j < h; j++) {
		Vec3V dir = dirRowStart;
		for (unsigned i = 0; i < w; i++) {
			root->Trace(origin, dir, *zptr++ BVH_STATS_ONLY(, packetMask, stats));
			dir += dirStepX;
		}
		dirRowStart += dirStepY;
	}
	const float raysPerSecond = ((float)(w*h))/progress.GetTimeInSeconds();
#if BVH_STATS
	progress.End("%.4f Mrays/sec (nodes=%zd,leaves=%zd,tris=%zd)", raysPerSecond/1000000.0f, stats.m_nodeCount, stats.m_leafCount, stats.m_triCount);
	stats.Report(1);
#else
	progress.End("%.4f Mrays/sec", raysPerSecond/1000000.0f);
#endif
	char ext[64];
#if BVH_THREADS
	sprintf(ext, "_%s_1x1_%02u_threads", NodeType::GetClassName_(), numThreads);
#else
	sprintf(ext, "_%s_1x1", NodeType::GetClassName_());
#endif
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}};

#define DEF_RENDER_TRIANGLES_BVH(packetW,packetH) \
void RenderTriangles_BVH_##packetW##x##packetH(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads)) \
{ \
	RenderTriangles_BVH<BVH4Node,packetW,packetH>::func(root,camera,tanVFOV,zbuf,w,h,zclear,zscale,zoffset,calc_z_range,path BVH_THREADS_ONLY(,numThreads)); \
}
DEF_RENDER_TRIANGLES_BVH(1,1)
DEF_RENDER_TRIANGLES_BVH(4,1)
DEF_RENDER_TRIANGLES_BVH(2,2)
#if HAS_VEC8V
DEF_RENDER_TRIANGLES_BVH(8,1)
DEF_RENDER_TRIANGLES_BVH(4,2)
#endif // HAS_VEC8V
#undef DEF_RENDER_TRIANGLES_BVH

#if defined(_EMBREE_SOURCE)
void RenderTriangles_4x1_EMBREE(RTCScene scene, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path)
{
	using namespace embree;
	const unsigned packetW = 4;
	const unsigned packetH = 1;
	const unsigned packetSize = packetW*packetH;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType PacketType;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType::ComponentType ComponentType;
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dirPacketStepX = dirStepX*(float)packetW;
	const Vec3V dirPacketStepY = dirStepY*(float)packetH;
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dv[packetSize];
	for (unsigned j = 0; j < packetH; j++)
		for (unsigned i = 0; i < packetW; i++)
			dv[i + j*packetW] = dir00 + dirStepX*(float)i + dirStepY*(float)j;
	PacketType dirRowStart(dv);
	ComponentType* zptr = reinterpret_cast<ComponentType*>(zbuf);
	ForceAssert((reinterpret_cast<uintptr_t>(zptr) & (packetSize*sizeof(float) - 1)) == 0);
	RTCIntersectContext context;
	context.flags = RTC_INTERSECT_COHERENT;
	__declspec(align(16)) uint32 valid[packetSize];
	memset(valid,-1,sizeof(valid));
	RTCRay4 rays;
	*(ComponentType*)rays.orgx = ComponentType(origin.x());
	*(ComponentType*)rays.orgy = ComponentType(origin.y());
	*(ComponentType*)rays.orgz = ComponentType(origin.z());
	*(ComponentType*)rays.tnear = ComponentType(V_ZERO);
	*(ComponentType*)rays.time = ComponentType(V_ZERO);
	*(ComponentType*)rays.mask = *reinterpret_cast<const ComponentType*>(valid);
	ProgressDisplay progress("rendering EMBREE BHV4 - %ux%u ray packets", packetW, packetH);
	for (unsigned j = 0; j < h; j += packetH) {
		PacketType dir = dirRowStart;
		for (unsigned i = 0; i < w; i += packetW) {
			*(ComponentType*)rays.dirx = dir.m_x;
			*(ComponentType*)rays.diry = dir.m_y;
			*(ComponentType*)rays.dirz = dir.m_z;
			*(ComponentType*)rays.tfar = ComponentType(ZBUFFER_DEFAULT);
			rtcIntersect4Ex(valid, scene, &context, rays);
			*zptr = Min(*reinterpret_cast<const ComponentType*>(rays.tfar), *zptr);
			dir += dirPacketStepX;
			zptr++;
		}
		dirRowStart += dirPacketStepY;
	}
	const float raysPerSecond = ((float)(w*h))/progress.GetTimeInSeconds();
	progress.End("%.4f Mrays/sec", raysPerSecond/1000000.0f);
	char ext[64];
	sprintf(ext, "_EMBREE_BVH4_%ux%u", packetW, packetH);
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}

static void RenderTriangles_8x1_EMBREE(RTCScene scene, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path)
{
	using namespace embree;
	const unsigned packetW = 8;
	const unsigned packetH = 1;
	const unsigned packetSize = packetW*packetH;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType PacketType;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType::ComponentType ComponentType;
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dirPacketStepX = dirStepX*(float)packetW;
	const Vec3V dirPacketStepY = dirStepY*(float)packetH;
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dv[packetSize];
	for (unsigned j = 0; j < packetH; j++)
		for (unsigned i = 0; i < packetW; i++)
			dv[i + j*packetW] = dir00 + dirStepX*(float)i + dirStepY*(float)j;
	PacketType dirRowStart(dv);
	ComponentType* zptr = reinterpret_cast<ComponentType*>(zbuf);
	ForceAssert((reinterpret_cast<uintptr_t>(zptr) & (packetSize*sizeof(float) - 1)) == 0);
	RTCIntersectContext context;
	context.flags = RTC_INTERSECT_COHERENT;
	__declspec(align(16)) uint32 valid[packetSize];
	memset(valid,-1,sizeof(valid));
	RTCRay8 rays;
	*(ComponentType*)rays.orgx = ComponentType(origin.x());
	*(ComponentType*)rays.orgy = ComponentType(origin.y());
	*(ComponentType*)rays.orgz = ComponentType(origin.z());
	*(ComponentType*)rays.tnear = ComponentType(V_ZERO);
	*(ComponentType*)rays.time = ComponentType(V_ZERO);
	*(ComponentType*)rays.mask = *reinterpret_cast<const ComponentType*>(valid);
	ProgressDisplay progress("rendering EMBREE BHV4 - %ux%u ray packets", packetW, packetH);
	for (unsigned j = 0; j < h; j += packetH) {
		PacketType dir = dirRowStart;
		for (unsigned i = 0; i < w; i += packetW) {
			*(ComponentType*)rays.dirx = dir.m_x;
			*(ComponentType*)rays.diry = dir.m_y;
			*(ComponentType*)rays.dirz = dir.m_z;
			*(ComponentType*)rays.tfar = ComponentType(ZBUFFER_DEFAULT);
			rtcIntersect8Ex(valid, scene, &context, rays);
			*zptr = Min(*reinterpret_cast<const ComponentType*>(rays.tfar), *zptr);
			dir += dirPacketStepX;
			zptr++;
		}
		dirRowStart += dirPacketStepY;
	}
	const float raysPerSecond = ((float)(w*h))/progress.GetTimeInSeconds();
	progress.End("%.4f Mrays/sec", raysPerSecond/1000000.0f);
	char ext[64];
	sprintf(ext, "_EMBREE_BVH4_%ux%u", packetW, packetH);
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}
#endif // defined(_EMBREE_SOURCE)

#if BVH_TILES
template <unsigned packetW, unsigned packetH> void RenderTriangles_BVH_TILES(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path)
{
	const unsigned packetSize = packetW*packetH;
	const unsigned packetMask = (1 << packetSize) - 1;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType PacketType;
	typedef typename SOA_T<packetSize>::Vec3V_SOAType::ComponentType ComponentType;
	if (tileW%packetW != 0 || tileH%packetH != 0)
		return;
	const unsigned numCols = (w + tileW - 1)/tileW;
	const unsigned numRows = (h + tileH - 1)/tileH;
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dirPacketStepX = dirStepX*(float)packetW;
	const Vec3V dirPacketStepY = dirStepY*(float)packetH;
	const Vec3V dirTileStepX = dirStepX*(float)tileW;
	const Vec3V dirTileStepY = dirStepY*(float)tileH;
	const Vec3V dir00 = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	Vec3V dv[packetSize];
	for (unsigned j = 0; j < packetH; j++)
		for (unsigned i = 0; i < packetW; i++)
			dv[i + j*packetW] = dir00 + dirStepX*(float)i + dirStepY*(float)j;
	const PacketType dirStart(dv);
	Plane3V* planesX = new Plane3V[numCols + 1];
	Plane3V* planesY = new Plane3V[numRows + 1];
	for (unsigned col = 0; col <= numCols; col++) {
		const float x = -1.0f + 2.0f*(float)col/(float)numCols; // [-1..1]
		const Vec3V corner0 = camera.Transform(Vec3V(x*tanHFOV, -tanVFOV, 1.0f)); // flip tanVFOV?
		const Vec3V corner1 = camera.Transform(Vec3V(x*tanHFOV, +tanVFOV, 1.0f));
		planesX[col] = Plane3V::ConstructFrom3Points(origin, corner1, corner0);
	}
	for (unsigned row = 0; row <= numRows; row++) {
		const float y = -1.0f + 2.0f*(float)row/(float)numRows; // [-1..1]
		const Vec3V corner0 = camera.Transform(Vec3V(+tanHFOV, y*tanVFOV, 1.0f)); // flip tanVFOV?
		const Vec3V corner1 = camera.Transform(Vec3V(-tanHFOV, y*tanVFOV, 1.0f));
		planesY[row] = Plane3V::ConstructFrom3Points(origin, corner1, corner0);
	}
#if BVH_STATS
	BVHStats stats;
	EntryPointSearchStats ep_stats;
#endif // BVH_STATS
	ProgressDisplay progress("rendering BHV4 - %ux%u ray packets - %ux%u tiles", packetW, packetH, tileW, tileH);
	PacketType dirTileRowStart = dirStart;
	float* zptrTileRowStart = zbuf;
	for (unsigned row = 0; row < numRows; row++) {
		PacketType dirTile = dirTileRowStart;
		float* zptrTile = zptrTileRowStart;
		const unsigned tileH1 = tileH - ((row == numRows - 1) ? h%tileH : 0);
		for (unsigned col = 0; col < numCols; col++) {
			const Plane3V frustum[] = {planesX[col], planesY[row], -planesX[col + 1], -planesY[row + 1]};
			PacketType dirRowStart = dirTile;
			float* zptrRowStart = zptrTile;
			const uintptr_t ref = root->EntryPointSearch(frustum BVH_STATS_ONLY(, ep_stats));
			if (ref) {
				const unsigned tileW1 = tileW - ((col == numCols - 1) ? w%tileW : 0);
				for (unsigned j = 0; j < tileH1; j += packetH) {
					PacketType dir = dirRowStart;
					ComponentType* zptr = reinterpret_cast<ComponentType*>(zptrRowStart);
					ForceAssert((reinterpret_cast<uintptr_t>(zptr) & (packetSize*sizeof(float) - 1)) == 0);
					for (unsigned i = 0; i < tileW1; i += packetW) {
						BVH4Node::TraceStatic(ref, origin, dir, *zptr++ BVH_STATS_ONLY(, packetMask, stats));
						dir += dirPacketStepX;
					}
					dirRowStart += dirPacketStepY;
					zptrRowStart += w;
				}
			}
			dirTile += dirTileStepX;
			zptrTile += tileW;
		}
		dirTileRowStart += dirTileStepY;
		zptrTileRowStart += w*tileH;
	}
	UnswizzleZBuffer<packetW,packetH>(zbuf, w, h);
	const float raysPerSecond = ((float)(w*h))/progress.GetTimeInSeconds();
#if BVH_STATS
	progress.End("%.4f Mrays/sec (nodes=%zd,leaves=%zd,tris=%zd) (out=%zd,in=%zd,mixed=%zd,codes=%zd,%zd,%zd)",
		raysPerSecond/1000000.0f,
		stats.m_nodeCount,
		stats.m_leafCount,
		stats.m_triCount,
		ep_stats.m_all_outs,
		ep_stats.m_all_ins,
		ep_stats.m_mixed_ins_outs,
		ep_stats.m_child_codes[0],
		ep_stats.m_child_codes[1],
		ep_stats.m_child_codes[2]);
	stats.Report(packetSize);
#else
	progress.End("%.4f Mrays/sec", raysPerSecond/1000000.0f);
#endif
	delete[] planesX;
	delete[] planesY;
	char ext[64];
	sprintf(ext, "_BVH4_%ux%u_%ux%u_tiles", packetW, packetH, tileW, tileH);
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}

template <> void RenderTriangles_BVH_TILES<1,1>(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path)
{
	const unsigned packetMask = 0x0001;
	if (zclear)
		ClearZBuffer(zbuf, w, h);
	const Vec3V origin = camera.d();
	const float tanHFOV = tanVFOV*(float)w/(float)h;
	const Vec3V dirStepX = +camera.a()*(2.0f*tanHFOV/(float)(w - 1)); // change in dir for each pixel horizontally
	const Vec3V dirStepY = -camera.b()*(2.0f*tanVFOV/(float)(h - 1)); // change in dir for each pixel vertically
	const Vec3V dirStart = camera.TransformDir(Vec3V(-tanHFOV, tanVFOV, 1.0f));
	const Vec3V dirTileStepX = dirStepX*(float)tileW;
	const Vec3V dirTileStepY = dirStepY*(float)tileH;
	const unsigned numCols = (w + tileW - 1)/tileW;
	const unsigned numRows = (h + tileH - 1)/tileH;
	Plane3V* planesX = new Plane3V[numCols + 1];
	Plane3V* planesY = new Plane3V[numRows + 1];
	for (unsigned col = 0; col <= numCols; col++) {
		const float x = -1.0f + 2.0f*(float)col/(float)numCols; // [-1..1]
		const Vec3V corner0 = camera.Transform(Vec3V(x*tanHFOV, -tanVFOV, 1.0f)); // flip tanVFOV?
		const Vec3V corner1 = camera.Transform(Vec3V(x*tanHFOV, +tanVFOV, 1.0f));
		planesX[col] = Plane3V::ConstructFrom3Points(origin, corner1, corner0);
	}
	for (unsigned row = 0; row <= numRows; row++) {
		const float y = -1.0f + 2.0f*(float)row/(float)numRows; // [-1..1]
		const Vec3V corner0 = camera.Transform(Vec3V(+tanHFOV, y*tanVFOV, 1.0f)); // flip tanVFOV?
		const Vec3V corner1 = camera.Transform(Vec3V(-tanHFOV, y*tanVFOV, 1.0f));
		planesY[row] = Plane3V::ConstructFrom3Points(origin, corner1, corner0);
	}
#if BVH_STATS
	BVHStats stats;
	EntryPointSearchStats ep_stats;
	uint32* depths = new uint32[numCols*numRows];
	memset(depths, 0xFF, numCols*numRows*sizeof(uint32));
#endif // BVH_STATS
	ProgressDisplay progress("rendering BHV4 - 1x1 ray packets - %ux%u tiles", tileW, tileH);
	Vec3V dirTileRowStart = dirStart;
	float* zptrTileRowStart = zbuf;
	for (unsigned row = 0; row < numRows; row++) {
		Vec3V dirTile = dirTileRowStart;
		float* zptrTile = zptrTileRowStart;
		const unsigned tileH1 = tileH - ((row == numRows - 1) ? h%tileH : 0);
		for (unsigned col = 0; col < numCols; col++) {
			const Plane3V frustum[] = {planesX[col], planesY[row], -planesX[col + 1], -planesY[row + 1]};
		#if 1
			Vec3V n(V_ZERO);
			for (int i = 0; i < 4; i++)
				n += frustum[i].GetNormal();
			ForceAssert(Dot(n, camera.c()) > 0.0f); // planes face inwards
		#endif
			const uintptr_t ref = root->EntryPointSearch(frustum BVH_STATS_ONLY(, ep_stats));
			if (ref) {
				BVH_STATS_ONLY(if (depths) depths[col + row*numCols] = BVH4Node::GetDepthStatic(ref));
				Vec3V dirRowStart = dirTile;
				float* zptrRowStart = zptrTile;
				const unsigned tileW1 = tileW - ((col == numCols - 1) ? w%tileW : 0);
				for (unsigned j = 0; j < tileH1; j++) {
					Vec3V dir = dirRowStart;
					float* zptr = zptrRowStart;
					for (unsigned i = 0; i < tileW1; i++) {
						BVH4Node::TraceStatic(ref, origin, dir, *zptr++ BVH_STATS_ONLY(, packetMask, stats));
						dir += dirStepX;
					}
					dirRowStart += dirStepY;
					zptrRowStart += w;
				}
			}
			dirTile += dirTileStepX;
			zptrTile += tileW;
		}
		dirTileRowStart += dirTileStepY;
		zptrTileRowStart += w*tileH;
	}
	const float raysPerSecond = ((float)(w*h))/progress.GetTimeInSeconds();
#if BVH_STATS
	progress.End("%.4f Mrays/sec (nodes=%zd,leaves=%zd,tris=%zd) (out=%zd,in=%zd,mixed=%zd,codes=%zd,%zd,%zd)",
		raysPerSecond/1000000.0f,
		stats.m_nodeCount,
		stats.m_leafCount,
		stats.m_triCount,
		ep_stats.m_all_outs,
		ep_stats.m_all_ins,
		ep_stats.m_mixed_ins_outs,
		ep_stats.m_child_codes[0],
		ep_stats.m_child_codes[1],
		ep_stats.m_child_codes[2]);
	stats.Report(1);
	if (1) {
		printf("tile depths for entry point search:\n");
		for (unsigned row = 0; row < numRows; row++) {
			for (unsigned col = 0; col < numCols; col++) {
				const uint32 depth = depths[col + row*numCols];
				if (depth != ~0U)
					printf("%s%u", col ? "," : "", depth);
				else
					printf("%s-", col ? "," : "");
			}
			printf("\n");
		}
	}
	delete[] depths;
#else
	progress.End("%.4f Mrays/sec", raysPerSecond/1000000.0f);
#endif
	delete[] planesX;
	delete[] planesY;
	char ext[64];
	sprintf(ext, "_BVH4_1x1_%ux%u_tiles", tileW, tileH);
	NormalizeAndSaveZBufferImage(zbuf, w, h, zscale, zoffset, calc_z_range, path, ext);
}

#define DEF_RENDER_TRIANGLES_BVH_TILES(packetW,packetH) \
void RenderTriangles_BVH_TILES_##packetW##x##packetH(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path) \
{ \
	RenderTriangles_BVH_TILES<packetW,packetH>(root,camera,tanVFOV,zbuf,w,h,tileW,tileH,zclear,zscale,zoffset,calc_z_range,path); \
}
DEF_RENDER_TRIANGLES_BVH_TILES(1,1)
DEF_RENDER_TRIANGLES_BVH_TILES(4,1)
DEF_RENDER_TRIANGLES_BVH_TILES(2,2)
#if HAS_VEC8V
DEF_RENDER_TRIANGLES_BVH_TILES(8,1)
DEF_RENDER_TRIANGLES_BVH_TILES(4,2)
#endif // HAS_VEC8V
#undef DEF_RENDER_TRIANGLES_BVH_TILES
#endif // BVH_TILES

void RenderOcclusion(const BVH4Node* root, const std::vector<Vec3V>& verts, const std::vector<Vec3V>& normals, std::vector<float>& occlusion, unsigned numSamples BVH_THREADS_ONLY(, unsigned numThreads))
{
	BVH_STATS_ONLY(BVHStats stats(1));
	occlusion.resize(verts.size());
	const float hemisphereAngle = 170.0f; // degrees
	const float relativeBias = 0.002f;
	std::vector<Vec3V> samples;
	samples.resize(numSamples);
	for (uint32 sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
		samples[sampleIndex] = HammersleyHemisphere(sampleIndex, numSamples, false, hemisphereAngle);
	Box3V bounds = Box3V::Invalid();
	for (size_t vertIndex = 0; vertIndex < verts.size(); vertIndex++)
		bounds.Grow(verts[vertIndex]);
	const ScalarV bias = MaxElement(bounds.GetExtent())*relativeBias;
	const uint32 numVerts = (uint32)verts.size();
#if BVH_THREADS
	ProgressDisplay progress("rendering occlusion (%u verts, %u samples, %u threads)", numVerts, numSamples, numThreads);
#else
	ProgressDisplay progress("rendering occlusion (%u verts, %u samples)", numVerts, numSamples);
#endif
#if BVH_THREADS
	if (numThreads > 0) {
		class LocalData
		{
		public:
			void Render() const
			{
				for (vertIndex = 0; vertIndex < numVerts; vertIndex++) {
					const Vec3V normal = normals[vertIndex];
					const Vec3V origin = verts[vertIndex] + normal*bias;
					const Mat33V basis = Mat33V::ConstructBasis(normal);
					uint32 numOccluded = 0;
					for (uint32 sampleIndex = 0; sampleIndex < numSamples; sampleIndex++) {
						ScalarV t(ZBUFFER_DEFAULT);
						const Vec3V dir = basis.Transform(samples[sampleIndex]);
						root->Trace(origin, dir, t BVH_STATS_ONLY(, 0x0001, *stats));
						DEBUG_ASSERT(t >= 0.0f);
						if (t < ZBUFFER_DEFAULT)
							numOccluded++;
					}
					occlusion[vertIndex] = 1.0f - (float)numOccluded/(float)numSamples;
				}
			}
		#if PLATFORM_PC
			static DWORD WINAPI StaticRender(LPVOID param) { reinterpret_cast<LocalData*>(param)->Render(); return 0; }
		#elif USE_XXX_MULTITASK
			static bool StaticRender(MultiTask task) { reinterpret_cast<LocalData*>(task)->Render(); return true; }
		#endif
			const BVH4Node* root;
			uint32 numSamples;
			const Vec3V* samples;
			uint32 numVerts;
			const Vec3V* verts;
			const Vec3V* normals;
			ScalarV bias;
			float* occlusion;
			BVH_STATS_ONLY(BVHStats* stats);
			mutable uint32 vertIndex;
		};
		LocalData* data = AlignedAlloc<LocalData>(numThreads, 32);
	#if PLATFORM_PC
		HANDLE* threads = new HANDLE[numThreads];
	#elif USE_XXX_MULTITASK
		MultiTask_InitQueue();
	#endif
		uint32 vertIndex = 0;
		for (unsigned i = 0; i < numThreads; i++) {
			data[i].root = root;
			data[i].numSamples = numSamples;
			data[i].samples = &samples.front();
			data[i].numVerts = Min((numVerts + numThreads - 1)/numThreads, numVerts - vertIndex);
			data[i].verts = &verts[vertIndex];
			data[i].normals = &normals[vertIndex];
			data[i].bias = bias;
			data[i].occlusion = &occlusion[vertIndex];
			BVH_STATS_ONLY(data[i].stats = new BVHStats);
			vertIndex += data[i].numVerts;
		#if PLATFORM_PC
			threads[i] = CreateThread(NULL, 0, LocalData::StaticRender, &data[i], 0, NULL);
		#elif USE_XXX_MULTITASK
			MultiTask_AddTask((MultiTask)&data[i]);
		#endif
		}
	#if PLATFORM_PC
		if (1) { // show minimum progress of threads
			while (WaitForMultipleObjects(numThreads, threads, TRUE, 100) >= numThreads) {
				float prog = FLT_MAX;
				for (unsigned i = 0; i < numThreads; i++)
					prog = Min((float)data[i].vertIndex/(float)data[i].numVerts, prog);
				progress.Update(prog);
			}
		} else
			WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);
	#elif USE_XXX_MULTITASK
		MultiTask_ProcessQueue(LocalData::StaticRender);
	#endif
		for (unsigned i = 0; i < numThreads; i++) {
		#if PLATFORM_PC
			CloseHandle(threads[i]);
		#endif // PLATFORM_PC
			BVH_STATS_ONLY(stats += *data[i].stats);
			BVH_STATS_ONLY(delete data[i].stats);
		}
	#if PLATFORM_PC
		delete[] threads;
	#endif // PLATFORM_PC
		AlignedFree(data);
	} else
#endif // BVH_THREADS
	for (uint32 vertIndex = 0; vertIndex < numVerts; vertIndex++) {
		if (vertIndex%64 == 0)
			progress.Update(vertIndex, numVerts);
		const Vec3V normal = normals[vertIndex];
		const Vec3V origin = verts[vertIndex] + normal*bias;
		const Mat33V basis = Mat33V::ConstructBasis(normal);
		uint32 numOccluded = 0;
		for (uint32 sampleIndex = 0; sampleIndex < numSamples; sampleIndex++) {
			ScalarV t(ZBUFFER_DEFAULT);
			const Vec3V dir = basis.Transform(samples[sampleIndex]);
			root->Trace(origin, dir, t BVH_STATS_ONLY(, 0x0001, stats));
			DEBUG_ASSERT(t >= 0.0f);
			if (t < ZBUFFER_DEFAULT)
				numOccluded++;
		}
		occlusion[vertIndex] = 1.0f - (float)numOccluded/(float)numSamples;
	}
	const float raysPerSecond = ((float)(numSamples*numVerts))/progress.GetTimeInSeconds();
#if BVH_STATS
	progress.End("%.4f Mrays/sec (nodes=%zd,leaves=%zd,tris=%zd)", raysPerSecond/1000000.0f, stats.m_nodeCount, stats.m_leafCount, stats.m_triCount);
	stats.Report(1);
#else
	progress.End("%.4f Mrays/sec", raysPerSecond/1000000.0f);
#endif
}