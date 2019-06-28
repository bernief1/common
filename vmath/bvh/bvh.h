// ================
// common/bvh/bvh.h
// ================

#ifndef _INCLUDE_BVH_H_
#define _INCLUDE_BVH_H_

/*
performance reference (work PC):
------------------------------------------------------------------------------------
loaded obj/dragon.obj - 50000 vertices, 100000 triangles
loaded obj/dragon.bvh4 - 9512 nodes, 23829 leaves, 100000 triangles
object is at 0.000000,4.969950,0.000000, extent = 7.046700,4.969950,3.151300
camera is at 0.000000,4.969950,-7.878250
        x = -1.00,0.00,0.00
        y = 0.00,-1.00,0.00
        z = 0.00,0.00,1.00
        det = 1.000000
resolution is 2560x2048
rendering BVH4 - 1x1 ray packets (0 threads) .. 5.3535 Mrays/sec. (979.338318 msecs)
zbuffer range 4.907804..9.871665
rendering BVH4 - 1x1 ray packets (4 threads) .. 14.5625 Mrays/sec. (360.026550 msecs)
rendering BVH4 - 1x1 ray packets (8 threads) .. 23.1064 Mrays/sec. (226.902313 msecs)
rendering BVH4 - 1x1 ray packets (12 threads) .. 24.5104 Mrays/sec. (213.905228 msecs)
rendering BVH4 - 1x1 ray packets (16 threads) .. 24.6678 Mrays/sec. (212.540741 msecs)
rendering BVH4 - 1x1 ray packets (20 threads) .. 24.1476 Mrays/sec. (217.118576 msecs)

rendering BVH4 - 4x1 ray packets (0 threads) .. 7.0848 Mrays/sec. (740.022522 msecs)
rendering BVH4 - 4x1 ray packets (4 threads) .. 20.6459 Mrays/sec. (253.942764 msecs)
rendering BVH4 - 4x1 ray packets (8 threads) .. 30.2253 Mrays/sec. (173.460281 msecs)
rendering BVH4 - 4x1 ray packets (12 threads) .. 29.2409 Mrays/sec. (179.300247 msecs)
rendering BVH4 - 4x1 ray packets (16 threads) .. 31.3605 Mrays/sec. (167.181274 msecs)
rendering BVH4 - 4x1 ray packets (20 threads) .. 32.2107 Mrays/sec. (162.768799 msecs)

rendering BVH4 - 2x2 ray packets (0 threads) .. 7.6032 Mrays/sec. (689.561218 msecs)
rendering BVH4 - 2x2 ray packets (4 threads) .. 23.9464 Mrays/sec. (218.943497 msecs)
rendering BVH4 - 2x2 ray packets (8 threads) .. 35.4897 Mrays/sec. (147.730698 msecs)
rendering BVH4 - 2x2 ray packets (12 threads) .. 35.7757 Mrays/sec. (146.549545 msecs)
rendering BVH4 - 2x2 ray packets (16 threads) .. 34.9843 Mrays/sec. (149.864944 msecs)

rendering BVH4 - 8x1 ray packets (0 threads) .. 8.7798 Mrays/sec. (597.156250 msecs)
rendering BVH4 - 8x1 ray packets (4 threads) .. 27.2895 Mrays/sec. (192.120850 msecs)
rendering BVH4 - 8x1 ray packets (8 threads) .. 40.3683 Mrays/sec. (129.876694 msecs)
rendering BVH4 - 8x1 ray packets (12 threads) .. 44.3625 Mrays/sec. (118.183357 msecs)
rendering BVH4 - 8x1 ray packets (16 threads) .. 42.7676 Mrays/sec. (122.590706 msecs)
rendering BVH4 - 8x1 ray packets (20 threads) .. 44.0113 Mrays/sec. (119.126472 msecs)

rendering BVH4 - 4x2 ray packets (0 threads) .. 10.3850 Mrays/sec. (504.854370 msecs)
rendering BVH4 - 4x2 ray packets (4 threads) .. 30.0583 Mrays/sec. (174.424774 msecs)
rendering BVH4 - 4x2 ray packets (8 threads) .. 43.2289 Mrays/sec. (121.282669 msecs)
rendering BVH4 - 4x2 ray packets (12 threads) .. 44.6139 Mrays/sec. (117.517944 msecs)
rendering BVH4 - 4x2 ray packets (16 threads) .. 42.6336 Mrays/sec. (122.976448 msecs)
rendering BVH4 - 4x2 ray packets (20 threads) .. 43.3318 Mrays/sec. (120.995010 msecs)

Press any key to continue . . .
*/

// ================================================================================================
// TODO -- BVH4 performance
// - frustum culling to eliminate BVH nodes on a screen tile basis
//    - entry point search - i've implemented this, not sure it's working right yet
//    - path compression - this seems like a great idea
//    - hierarchical tiles
//    - frustum plane masking (instead of recalculating the in/out codes for each plane all the time)
//    - SOA frustum calculations (4 frustum planes against each child, or each plane vs all four children)
//    - support frustum culling with ray packets
//    - currently my frustum culling only rejects bounds vs frustum planes - this is not perfect culling
//    - also, not sure if even my Entry Point Search code is correct. for example maybe two child bounds are inside
//      the frustum so the parent node will be returned, but actually one of those child bounds contains *no* children
//      which are intersecting the frustum at all, so really the other child node should have been returned instead.
// - order-traversal - consider BVH children in front-to-back order along ray
//    - tried this, improved traversal in terms of calculations but actually ran slower .. TRAVERSE_FRONT_TO_BACK
//    - maybe there is a better way to do this?
//    - could try as a comparison pre-sorting the children in each node as a separate pass, and then compare rendering times
//    - also not sure how to evaluate front-to-back ordering when ranges overlap (which they usually do)
// + skip BVH children which are further along the ray than the current hit
// - ray packets which lose too many rays along BVH traversal could be "collected" into new packets
//    - need to store rays associated with the nodes
//    - not sure how useful this is, as the collected rays would be more divergent
// - consider single ray vs BVH packets (ray-vs-box8) and triangle packets (ray-vs-tri8)
// x Box3V::IntersectsRay could eliminate the Min/Max calculation of tmin,tmax if i split the traversal into octants ..?
//    - seems like this would be diminishing returns
// x handle ZBUFFER_DEFAULT better, maybe we can make 0 the default value (calculate 1-z?)
//    - seems like this would be diminishing returns
// + use a node stack instead of calling functions recursively
// + run a performance test analysis vs Embree ray tracing
// + multithreading
// 
// - render random rays from surface of a sphere onto the model (incoherent)
// - render from different directions (this will stress the order traversal more uniformly)
// + tessellate the bunny model to test high density scene
// ================================================================================================

#include "vmath/vmath_common.h"
#include "vmath/vmath_box.h"
#include "vmath/vmath_plane.h" // for BVH_TILES
#include "vmath/vmath_intersects.h"
#include "vmath/vmath_soa.h"
#include "vmath/vmath_triangle.h"
#include "vmath/vmath_vec16.h"

#include "GraphicsTools/util/mesh.h"

#if defined(_EMBREE)// && PLATFORM_PC && VISUAL_STUDIO_VERSION < 2017
//#define _EMBREE
#include "../../../embree-2.17.2/kernels/bvh/bvh.h"
#if defined(EMBREE_TARGET_AVX2) || defined(EMBREE_TARGET_AVX) || defined(EMBREE_TARGET_SSE42)
#define _EMBREE_SOURCE // can't define this for my own project because i'm not linking in the full embree source yet, only headers ..
#endif
#endif

#define BVH_THREADS (1)
#if BVH_THREADS
#define BVH_THREADS_ONLY(...) __VA_ARGS__
#define BVH_THREADS_SWITCH(_if_threads_,_if_not_threads_) _if_threads_
#else
#define BVH_THREADS_ONLY(...)
#define BVH_THREADS_SWITCH(_if_threads_,_if_not_threads_) _if_not_threads_
#endif

#define BVH_STATS (0 && PLATFORM_PC)
#if BVH_STATS
#define BVH_STATS_ONLY(...) __VA_ARGS__
#else
#define BVH_STATS_ONLY(...)
#endif

#define BVH_SOA_BOUNDS (1) // stores BVH bounds in SOA - allows for single ray tracing to intersect all child bounds simultaneously
#define BVH_SOA_BOUNDS_INTERSECT_SINGLE_RAYS_AGAINST_ALL_CHILDREN_SIMULTANEOUSLY (1 && BVH_SOA_BOUNDS)

#define BVH_SWITCH_TO_SINGLE_RAYS (0) // when tracing a packet and only a single ray intersects, switch to single-ray codepath (doesn't show improvement)

#define BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST (0) // experimental - this should be much faster though
#if BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
#define BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(...) __VA_ARGS__
#else
#define BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(...)
#endif

#define BVH_LEAF_NUM_TRIANGLES_SOA (1) // 1,4,8 (TODO -- this currently crashes with values other than 1)
// occlusion render however works with SOA triangles
// bunny model with 2048 samples/vertex runs in 50.7 secs with single triangles, 20.4 secs with SOA4, and 20.1 secs with SOA8 (i don't think the data is optimized for SOA8 though)

#define BVH_STACK_MAX_DEPTH 32

#define BVH_TILES (0) // entry point search, path compression, etc. (TODO -- not working yet)
#define BVH_TILES_PATHCOMPRESSION (0 && BVH_TILES)

#if defined(_EMBREE)
#if defined(_EMBREE_SOURCE)
RTCScene EmbreeCreateScene();
void EmbreeAddMeshToScene(RTCScene scene,const geomesh::TriangleMesh& mesh);
void EmbreeFinalizeScene(RTCScene scene);
void EmbreeCleanup();
#endif // defined(_EMBREE_SOURCE)
bool EmbreeSaveBVH4(const char* path,RTCScene scene,const geomesh::TriangleMesh* mesh = NULL,bool silent = false);
#endif // defined(_EMBREE)

class BVHCounts
{
public:
	VMATH_INLINE BVHCounts(uint32 nodeCount = 0,uint32 leafCount = 0,uint32 triCount = 0): m_nodeCount(nodeCount),m_leafCount(leafCount),m_triCount(triCount) {}

	VMATH_INLINE const BVHCounts operator +=(const BVHCounts& counts)
	{
		m_nodeCount += counts.m_nodeCount;
		m_leafCount += counts.m_leafCount;
		m_triCount += counts.m_triCount;
		return *this;
	}

	uint32 m_nodeCount;
	uint32 m_leafCount;
	uint32 m_triCount;
};

#if BVH_STATS
class BVHStats : public BVHCounts
{
public:
	VMATH_INLINE BVHStats(uint32 nodeCount = 0): BVHCounts(nodeCount),m_miscA(0),m_miscB(0),m_miscC(0) {}

	void EnterNode(uint32 mask,uint32 depth);
	void EnterLeaf(uint32 mask,uint32 depth);
	void TestAgainstTriangle(uint32 mask,uint32 depth);

	VMATH_INLINE void Report(unsigned packetSize) const
	{
		m_numberOfRaysEnteringNodeAtDepth.Report(packetSize,"# rays entering node at depth");
		m_numberOfRaysEnteringLeafAtDepth.Report(packetSize,"# rays entering leaf at depth");
		m_numberOfRaysTestingAgainstTriAtDepth.Report(packetSize,"# rays testing against triangle at depth");
		if (m_miscA|m_miscB|m_miscC)
			printf("miscellaneous=%u,%u,%u\n",m_miscA,m_miscB,m_miscC);
	}

	class RayPacketHistogram
	{
	public:
		VMATH_INLINE RayPacketHistogram()
		{
			memset(this,0,sizeof(*this));
		}

		VMATH_INLINE RayPacketHistogram& operator +=(const RayPacketHistogram& h)
		{
			for (unsigned i = 0; i < countof(n); i++)
				n[i] += h.n[i];
			return *this;
		}

		size_t n[16 + 1];
	};

	class RayPacketHistogramArray : public std::vector<RayPacketHistogram>
	{
	public:
		VMATH_INLINE RayPacketHistogramArray& operator +=(const RayPacketHistogramArray& ha)
		{
			while (size() < ha.size())
				push_back(RayPacketHistogram());
			for (size_t i = 0; i < ha.size(); i++)
				operator[](i) += ha[i];
			return *this;
		}

	#if 1//PLATFORM_PS4 // DO NOT SUBMIT -- put this somewhere!
		static const char* strcatf_(char* str, const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			const size_t len = strlen(str);
			vsprintf(str + len, fmt, args);
			va_end(args);
			return str;
		}
	#endif

		VMATH_INLINE void Report(unsigned packetSize,const char* str) const
		{
			ForceAssert(packetSize <= 16); // otherwise, use __popcnt(mask)
			for (uint32 depth = 0; depth < size(); depth++) {
				char temp[256] = "";
				for (unsigned i = 0; i <= packetSize; i++)
					strcatf_(temp,"%s%u",i ? "," : "",operator[](depth).n[i]);
				printf("%s %u = [%s]\n",str,depth,temp);
			}
		}
	};

	VMATH_INLINE BVHStats& operator +=(const BVHStats& stats)
	{
		m_nodeCount += stats.m_nodeCount;
		m_leafCount += stats.m_leafCount;
		m_triCount += stats.m_triCount;
		m_numberOfRaysEnteringNodeAtDepth += stats.m_numberOfRaysEnteringNodeAtDepth;
		m_numberOfRaysEnteringLeafAtDepth += stats.m_numberOfRaysEnteringLeafAtDepth;
		m_numberOfRaysTestingAgainstTriAtDepth += stats.m_numberOfRaysTestingAgainstTriAtDepth;
		m_miscA += stats.m_miscA;
		m_miscB += stats.m_miscB;
		m_miscC += stats.m_miscC;
		return *this;
	}

	RayPacketHistogramArray m_numberOfRaysEnteringNodeAtDepth;
	RayPacketHistogramArray m_numberOfRaysEnteringLeafAtDepth;
	RayPacketHistogramArray m_numberOfRaysTestingAgainstTriAtDepth;
	uint32 m_miscA; // miscellaneous debug
	uint32 m_miscB;
	uint32 m_miscC;
};

#if BVH_TILES
class EntryPointSearchStats
{
public:
	VMATH_INLINE EntryPointSearchStats() { memset(this,0,sizeof(*this)); }
	size_t m_all_outs;
	size_t m_all_ins;
	size_t m_mixed_ins_outs;
	size_t m_child_codes[3]; // BVH4Node::IntersectionCode
};
#endif // BVH_TILES
#endif // BVH_STATS

class BVHCommon
{
public:
	enum NodeType { BVH_NODETYPE_EMPTY = 0, BVH_NODETYPE_NODE = 1, BVH_NODETYPE_LEAF = 2 };
	enum IntersectionCode { BVH_OUTSIDE = 0, BVH_INSIDE = 1, BVH_INTERSECTED = 2 };
	static const uintptr_t BVH_LEAF_FLAG = 1;

#if BVH_LEAF_NUM_TRIANGLES_SOA > 1
	typedef typename Triangle3V_SOA_T<BVH_LEAF_NUM_TRIANGLES_SOA>::TriangleType TriangleType;
#else
	typedef Triangle3V TriangleType;
#endif
	class Leaf : public std::vector<TriangleType>
	{
	public:
		void SetPrimitives(const Triangle3V* triangles,uint32 count);
		static Leaf* LoadInternal(FILE* f,const Mat34V* transform BVH_STATS_ONLY(,uint32 depth));

		VMATH_INLINE void Release() { delete this; }

	#if BVH_LEAF_NUM_TRIANGLES_SOA == 1
		VMATH_INLINE unsigned GetTriCount() const { return (unsigned)size(); }
	#else
		VMATH_INLINE unsigned GetTriCount() const { return m_count; }
		unsigned m_count;
	#endif

		VMATH_INLINE const Triangle3V GetTriangle(unsigned i) const
		{
			DEBUG_ASSERT(i < GetTriCount());
			return operator[](i/BVH_LEAF_NUM_TRIANGLES_SOA).GetIndexed(i%BVH_LEAF_NUM_TRIANGLES_SOA);
		}

		VMATH_INLINE void IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,float& t_ BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const
		{
			ScalarV t(t_);
			IntersectsRay(origin,dir,t BVH_STATS_ONLY(,mask,stats));
			t_ = t.f();
		}

		void IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const;
		void IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin,Vec3V_SOA4_arg dir,Vec4V& t BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const;
	#if HAS_VEC8V
		void IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin,Vec3V_SOA8_arg dir,Vec8V& t BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const;
	#endif // HAS_VEC8V

		void IntersectsRaySign(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,ScalarV& sign) const;
		void IntersectsRaySignAccum(Vec3V_arg origin,Vec3V_arg dir,ScalarV& sign) const;

	#if BVH_STATS
		uint32 m_depth;
	#endif // BVH_STATS
	};

#if BVH_TILES
	static IntersectionCode GetIntersectionCode(const Box3V& bounds,const Plane3V frustum[4]);
#if BVH_TILES_PATHCOMPRESSION
	class PathCompressionNode
	{
	public:
		enum { N = 4 };

		VMATH_INLINE PathCompressionNode(uintptr_t ref = 0): m_ref(ref)
		{
			memset(m_children,0,sizeof(m_children));
		}

		void Release();

		uintptr_t m_ref; // references a BVH node or leaf
		PathCompressionNode* m_children[N];
	};
#endif // BVH_TILES_PATHCOMPRESSION
#endif // BVH_TILES
};

class BVH4Node : public BVHCommon
{
public:
	enum { N = 4 };

	VMATH_INLINE static const char* GetClassName_() { return "BVH4"; } // wanted to call this "GetClassName" but windows has defined it, apparently ..
	static BVH4Node* Load(const char* path,const Mat34V* transform = NULL);
private:
	static BVH4Node* LoadInternal(FILE* f,const Mat34V* transform BVH_STATS_ONLY(,uint32 depth = 0));
public:
	void Release();

	const BVHCounts Count() const;
	const Box3V GetBounds() const;

#if BVH_SOA_BOUNDS
	template <typename BoxType> VMATH_INLINE const BoxType GetChildBounds_BroadcastSOA(unsigned i) const { DEBUG_ASSERT(i < N); return m_bounds.GetIndexed_BroadcastSOA<BoxType>(i); }
	VMATH_INLINE const Box3V GetChildBounds(unsigned i) const { DEBUG_ASSERT(i < N); return m_bounds.GetIndexed(i); } // don't call this!
#else
	template <typename BoxType> VMATH_INLINE const BoxType GetChildBounds_BroadcastSOA(unsigned i) const { DEBUG_ASSERT(i < N); return m_bounds[i].BroadcastSOA<BoxType>(); }
	VMATH_INLINE const Box3V& GetChildBounds(unsigned i) const { DEBUG_ASSERT(i < N); return m_bounds[i]; }
#endif
	VMATH_INLINE bool IsChildNonEmpty(unsigned i) const { DEBUG_ASSERT(i < N); return m_children[i] != 0; }
	VMATH_INLINE bool IsChildLeaf(unsigned i) const { DEBUG_ASSERT(i < N); return (m_children[i] & BVH_LEAF_FLAG) != 0; }
	VMATH_INLINE BVH4Node* GetChildNode(unsigned i) const { DEBUG_ASSERT(i < N && !IsChildLeaf(i)); return reinterpret_cast<BVH4Node*>(m_children[i]); }
	VMATH_INLINE Leaf* GetChildLeaf(unsigned i) const { DEBUG_ASSERT(i < N && IsChildLeaf(i)); return reinterpret_cast<Leaf*>(m_children[i] & ~BVH_LEAF_FLAG); }

	template <typename OriginType,typename DirType> VMATH_INLINE void Trace(const OriginType& origin,const DirType& dir,typename DirType::ComponentType& t BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const
	{
		TraceStatic(reinterpret_cast<uintptr_t>(this),origin,dir,t BVH_STATS_ONLY(,mask,stats));
	}
	
	VMATH_INLINE void Trace(Vec3V_arg origin,Vec3V_arg dir,float& t_ BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const;
	VMATH_INLINE static void TraceStatic(uintptr_t ref,Vec3V_arg origin,Vec3V_arg dir,float& t_ BVH_STATS_ONLY(,uint32 mask,BVHStats& stats));

	template <typename OriginType,typename DirType> VMATH_INLINE static void TraceStatic(uintptr_t ref_,const OriginType& origin,const DirType& dir,typename DirType::ComponentType& t BVH_STATS_ONLY(,uint32 mask_,BVHStats& stats))
	{
		const DirType invdir = Recip(dir);
	#if BVH_STATS
		const unsigned numMaskBits = 8;
		const uintptr_t refMask = (uintptr_t)-1 >> numMaskBits;
		StaticAssert(DirType::ComponentType::NumElements <= numMaskBits);
	#endif // BVH_STATS
		uintptr_t stack[BVH_STACK_MAX_DEPTH] = {ref_ BVH_STATS_ONLY(| ((uintptr_t)mask_ << (64 - numMaskBits)))};
		unsigned stackIndex = 1;
		while (stackIndex > 0) {
			const uintptr_t ref = stack[--stackIndex] BVH_STATS_ONLY(& refMask);
			BVH_STATS_ONLY(mask_ = (uint32)(stack[stackIndex] >> (64 - numMaskBits)));
			if (ref & BVH_LEAF_FLAG)
				reinterpret_cast<const Leaf*>(ref & ~BVH_LEAF_FLAG)->IntersectsRay(origin,dir,t BVH_STATS_ONLY(,mask_,stats));
			else {
				const BVH4Node* node = reinterpret_cast<const BVH4Node*>(ref);
				BVH_STATS_ONLY(stats.EnterNode(mask_,node->m_depth));
				for (unsigned i = 0; i < N && node->IsChildNonEmpty(i); i++) {
					BVH_STATS_ONLY(DEBUG_ASSERT((node->m_children[i] & refMask) == 0));
					const uint32 mask = node->GetChildBounds(i).IntersectsRay(origin,invdir,t);
					//const uint32 mask = node->GetChildBounds_BroadcastSOA<Box3V_SOA_T<DirType>>(i).IntersectsRay(origin,invdir,t);
					if (mask) {
					#if BVH_SWITCH_TO_SINGLE_RAYS
						// TODO -- to split into multiple single rays, use _blsr_u32 to scan through mask bits
						if (__popcnt(mask) == 1) {
							const uint32 rayIndex = _tzcnt_u32(mask);
							TraceStatic(m_children[i],origin,dir.GetVector(rayIndex),t[rayIndex] BVH_STATS_ONLY(,0x0001,stats));
						} else
					#endif // BVH_SWITCH_TO_SINGLE_RAYS
						{
							DEBUG_ASSERT(stackIndex < BVH_STACK_MAX_DEPTH);
							stack[stackIndex++] = node->m_children[i] BVH_STATS_ONLY(| ((uintptr_t)mask << (64 - numMaskBits)));
						}
					}
				}
			}
		}
	}

	// returns sign of intersection (front/back)
	VMATH_INLINE void TraceSign(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,ScalarV& sign) const
	{
		const Vec3V invdir = Recip(dir);
		uintptr_t stack[BVH_STACK_MAX_DEPTH] = {reinterpret_cast<uintptr_t>(this)};
		unsigned stackIndex = 1;
		while (stackIndex > 0) {
			const uintptr_t ref = stack[--stackIndex];
			if (ref & BVH_LEAF_FLAG)
				reinterpret_cast<const Leaf*>(ref & ~BVH_LEAF_FLAG)->IntersectsRaySign(origin,dir,t,sign);
			else {
				const BVH4Node* node = reinterpret_cast<const BVH4Node*>(ref);
				for (unsigned i = 0; i < N && node->IsChildNonEmpty(i); i++) {
					const uint32 mask = node->GetChildBounds(i).IntersectsRay(origin,invdir,t);
					if (mask) {
						DEBUG_ASSERT(stackIndex < BVH_STACK_MAX_DEPTH);
						stack[stackIndex++] = node->m_children[i];
					}
				}
			}
		}
	}

	// accumulates +1/-1 for front/backfaces (or maybe it's the other way around)
	VMATH_INLINE void TraceSignAccum(Vec3V_arg origin,Vec3V_arg dir,ScalarV_arg t,ScalarV& sign) const
	{
		const Vec3V invdir = Recip(dir);
		uintptr_t stack[BVH_STACK_MAX_DEPTH] = {reinterpret_cast<uintptr_t>(this)};
		unsigned stackIndex = 1;
		while (stackIndex > 0) {
			const uintptr_t ref = stack[--stackIndex];
			if (ref & BVH_LEAF_FLAG)
				reinterpret_cast<const Leaf*>(ref & ~BVH_LEAF_FLAG)->IntersectsRaySignAccum(origin,dir,sign);
			else {
				const BVH4Node* node = reinterpret_cast<const BVH4Node*>(ref);
				for (unsigned i = 0; i < N && node->IsChildNonEmpty(i); i++) {
					const uint32 mask = node->GetChildBounds(i).IntersectsRay(origin,invdir,t);
					if (mask) {
						DEBUG_ASSERT(stackIndex < BVH_STACK_MAX_DEPTH);
						stack[stackIndex++] = node->m_children[i];
					}
				}
			}
		}
	}

	bool IntersectsSphere(const Sphere3V& sphere BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(,const Leaf*& leaf_,unsigned& index_)) const;
#if BVH_LEAF_NUM_TRIANGLES_SOA <= 4
	bool IntersectsBox(const Box3V& box) const;
#endif // BVH_LEAF_NUM_TRIANGLES_SOA <= 4
	float FindMinimumDistanceToSurface(Vec3V_arg pos,float startRadius,float step = 0.0f,unsigned numSteps = 24) const;
	float FindMinimumDistanceToSurfaceEx(Vec3V_arg pos,Vec3V_arg cellSize,unsigned subCellResMinMax,unsigned subCellResGrad,float startRadius,float step = 0.0f,unsigned numSteps = 24,float* out_dmin = nullptr,float* out_dmax = nullptr,Vec4V* out_dgrad = nullptr) const;

#if BVH_TILES
	uintptr_t EntryPointSearch(const Plane3V frustum[4] BVH_STATS_ONLY(,EntryPointSearchStats& stats)) const;
#if BVH_TILES_PATHCOMPRESSION
	PathCompressionNode* PathCompression(const Plane3V frustum[4]) const;
#endif // BVH_TILES_PATHCOMPRESSION
#endif // BVH_TILES

#if BVH_STATS
	void ReportTriCounts() const;
private:
	void ReportTriCountsInternal(std::vector<size_t>& histogram) const;
public:
	VMATH_INLINE static uint32 GetDepthStatic(uintptr_t ref)
	{
		if (ref & BVH_LEAF_FLAG)
			return reinterpret_cast<const Leaf*>(ref & ~BVH_LEAF_FLAG)->m_depth;
		else
			return reinterpret_cast<const BVH4Node*>(ref)->m_depth;
	}
	uint32 m_depth;
#endif // BVH_STATS

#if BVH_SOA_BOUNDS
	Box3V_SOA4 m_bounds;
#else
	Box3V m_bounds[N];
#endif
	uintptr_t m_children[N];
};

#if BVH_SOA_BOUNDS_INTERSECT_SINGLE_RAYS_AGAINST_ALL_CHILDREN_SIMULTANEOUSLY
// have to define this outside of the BVH4Node class for PS4 ...
template <> VMATH_INLINE void BVH4Node::TraceStatic<Vec3V,Vec3V>(uintptr_t ref_,const Vec3V& origin,const Vec3V& dir,ScalarV& t BVH_STATS_ONLY(,uint32 mask_,BVHStats& stats))
{
	const Vec3V invdir = Recip(dir);
	uintptr_t stack[BVH_STACK_MAX_DEPTH] = {ref_};
	unsigned stackIndex = 1;
	while (stackIndex > 0) {
		const uintptr_t ref = stack[--stackIndex];
		if (ref & BVH_LEAF_FLAG)
			reinterpret_cast<const Leaf*>(ref & ~BVH_LEAF_FLAG)->IntersectsRay(origin,dir,t BVH_STATS_ONLY(,0x0001,stats));
		else {
			const BVH4Node* node = reinterpret_cast<const BVH4Node*>(ref);
			BVH_STATS_ONLY(stats.EnterNode(0x0001,node->m_depth));
			uint32 childMask = node->m_bounds.IntersectsRay(origin,invdir,t); // intersect all child bounds at once
		#if 0 && PLATFORM_PC // use _tzcnt_u32 and _blsr_u32 (this hasn't shown improvement in my tests)
			unsigned childIndex = _tzcnt_u32(childMask);
			while (childIndex < 32 && node->IsChildNonEmpty(childIndex)) { // lame .. need to test IsChildNonEmpty because apparently my IntersectsRay code can return positive results for FLT_MAX bounding boxes
				DEBUG_ASSERT(childIndex < N)// && node->IsChildNonEmpty(childIndex));
				DEBUG_ASSERT(stackIndex < BVH_STACK_MAX_DEPTH);
				stack[stackIndex++] = node->m_children[childIndex];
				childMask = _blsr_u32(childMask);
				childIndex = _tzcnt_u32(childMask);
			}
		#else
			unsigned childIndex = 0;
			while (childMask && node->IsChildNonEmpty(childIndex)) { // lame .. need to test IsChildNonEmpty because apparently my IntersectsRay code can return positive results for FLT_MAX bounding boxes
				if (childMask & 1) {
					DEBUG_ASSERT(childIndex < N);// && node->IsChildNonEmpty(childIndex));
					DEBUG_ASSERT(stackIndex < BVH_STACK_MAX_DEPTH);
					stack[stackIndex++] = node->m_children[childIndex];
				}
				childMask >>= 1;
				childIndex++;
			}
		#endif
		}
	}
}
#endif // BVH_SOA_BOUNDS_INTERSECT_SINGLE_RAYS_AGAINST_ALL_CHILDREN_SIMULTANEOUSLY

// .. and have to define these _after_ explicit specialization of BVH4Node::TraceStatic<Vec3V,Vec3V> for PS4
VMATH_INLINE void BVH4Node::Trace(Vec3V_arg origin,Vec3V_arg dir,float& t_ BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const
{
	ScalarV t(t_);
	TraceStatic(reinterpret_cast<uintptr_t>(this),origin,dir,t BVH_STATS_ONLY(,mask,stats));
	t_ = t.f();
}

VMATH_INLINE void BVH4Node::TraceStatic(uintptr_t ref,Vec3V_arg origin,Vec3V_arg dir,float& t_ BVH_STATS_ONLY(,uint32 mask,BVHStats& stats))
{
	ScalarV t(t_);
	TraceStatic(ref,origin,dir,t BVH_STATS_ONLY(,mask,stats));
	t_ = t.f();
}

#endif // _INCLUDE_BVH_H_