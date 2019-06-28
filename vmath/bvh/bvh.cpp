// ==================
// common/bvh/bvh.cpp
// ==================

#include "bvh.h"
#include "GraphicsTools/util/mesh.h"
#include "GraphicsTools/util/stringutil.h"
#include "vmath/vmath_triangle.h"

#if defined(_EMBREE)
#include "../../../embree-2.17.2/include/embree2/rtcore.h"
#include "../../../embree-2.17.2/kernels/geometry/trianglev.h"
#include "../../../embree-2.17.2/common/math/constants.cpp"
#endif // defined(_EMBREE)

#if defined(_EMBREE)
#define EMBREE_TRIANGLE_SOA 8
#if defined(_EMBREE_SOURCE) // gahh .. :(
static RTCDevice g_EmbreeDevice = NULL;
static std::vector<RTCScene> g_EmbreeScenes;
RTCScene EmbreeCreateScene()
{
	using namespace embree;
	if (g_EmbreeDevice == NULL) {
		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
		g_EmbreeDevice = rtcNewDevice("tri_accel=bvh4.triangle4v");
		class EmbreeErrorHandler { public: static void func(void*,const RTCError code,const char* str = NULL) {
			if (code == RTC_NO_ERROR) 
				return;
			fprintf(stderr,"Embree: ");
			switch (code) {
			case RTC_UNKNOWN_ERROR    : fprintf(stderr,"RTC_UNKNOWN_ERROR"); break;
			case RTC_INVALID_ARGUMENT : fprintf(stderr,"RTC_INVALID_ARGUMENT"); break;
			case RTC_INVALID_OPERATION: fprintf(stderr,"RTC_INVALID_OPERATION"); break;
			case RTC_OUT_OF_MEMORY    : fprintf(stderr,"RTC_OUT_OF_MEMORY"); break;
			case RTC_UNSUPPORTED_CPU  : fprintf(stderr,"RTC_UNSUPPORTED_CPU"); break;
			case RTC_CANCELLED        : fprintf(stderr,"RTC_CANCELLED"); break;
			default                   : fprintf(stderr,"invalid error code %d",code); break;
			}
			if (str)
				fprintf(stderr," (%s)\n",str);
			exit(1);
		}};
		EmbreeErrorHandler::func(NULL,rtcDeviceGetError(g_EmbreeDevice));
		rtcDeviceSetErrorFunction2(g_EmbreeDevice,EmbreeErrorHandler::func,NULL);
	}
	RTCScene scene = rtcDeviceNewScene(g_EmbreeDevice,RTC_SCENE_STATIC,RTC_INTERSECT1|RTC_INTERSECT4|RTC_INTERSECT8);
	g_EmbreeScenes.push_back(scene);
	return scene;
}

void EmbreeAddMeshToScene(RTCScene scene,const geomesh::TriangleMesh& mesh)
{
	using namespace embree;
	EmbreeCreateScene();
	unsigned int meshID = rtcNewTriangleMesh(scene,RTC_GEOMETRY_STATIC,mesh.m_polys.size(),mesh.m_verts.size());
	StaticAssert(sizeof(Vec4V) == sizeof(Vec3fa));
	memcpy(rtcMapBuffer(scene,meshID,RTC_VERTEX_BUFFER),&mesh.m_verts.front(),mesh.m_verts.size()*sizeof(Vec4V));
	rtcUnmapBuffer(scene,meshID,RTC_VERTEX_BUFFER);
	struct Triangle { uint32 m_indices[3]; };
	Triangle* triangles = reinterpret_cast<Triangle*>(rtcMapBuffer(scene,meshID,RTC_INDEX_BUFFER));
	for (size_t triIndex = 0; triIndex < mesh.m_polys.size(); triIndex++) {
		triangles[triIndex].m_indices[0] = mesh.m_polys[triIndex].m_indices[0];
		triangles[triIndex].m_indices[1] = mesh.m_polys[triIndex].m_indices[1];
		triangles[triIndex].m_indices[2] = mesh.m_polys[triIndex].m_indices[2];
	}
	rtcUnmapBuffer(scene,meshID,RTC_INDEX_BUFFER);
}

void EmbreeFinalizeScene(RTCScene scene)
{
	using namespace embree;
	rtcCommit(scene);
}

void EmbreeCleanup()
{
	using namespace embree;
	for (size_t i = 0; i < g_EmbreeScenes.size(); i++)
		rtcDeleteScene(g_EmbreeScenes[i]);
	g_EmbreeScenes.clear();
	rtcDeleteDevice(g_EmbreeDevice);
	g_EmbreeDevice = NULL;
}
#endif // defined(_EMBREE_SOURCE)

static void EmbreeSaveBVH4Internal(FILE* f,embree::BVH4::NodeRef ref,BVHCounts& counts,const geomesh::TriangleMesh* mesh)
{
	using namespace embree;
	if (ref.isAlignedNode()) {
		BVH4::AlignedNode* node = ref.alignedNode();
		counts.m_nodeCount++;
		uint8 types[4] = {0,0,0,0};
		for (int i = 0; i < 4; i++) {
			if (node->child(i) == BVH4::emptyNode)
				types[i] = BVH4Node::BVH_NODETYPE_EMPTY;
			else if (node->child(i).isAlignedNode())
				types[i] = BVH4Node::BVH_NODETYPE_NODE;
			else if (node->child(i).isLeaf())
				types[i] = BVH4Node::BVH_NODETYPE_LEAF;
			else
				DEBUG_ASSERT(false);
		}
		fwrite(types,sizeof(types),1,f);
		for (int i = 0; i < 4 && node->child(i) != BVH4::emptyNode; i++) {
			const Vec3V bmin = Vec3V(VEC3_ARGS(node->bounds(i).lower));
			const Vec3V bmax = Vec3V(VEC3_ARGS(node->bounds(i).upper));
			fwrite(&bmin,3*sizeof(float),1,f);
			fwrite(&bmax,3*sizeof(float),1,f);
		}
		for (int i = 0; i < 4 && node->child(i) != BVH4::emptyNode; i++)
			EmbreeSaveBVH4Internal(f,node->child(i),counts,mesh);
	} else {
		DEBUG_ASSERT(ref.isLeaf());
		size_t num;
		const Triangle4v* tris = (const Triangle4v*)ref.leaf(num);
		uint32 count = 0;
		for (size_t i = 0; i < num; i++)
			count += (uint32)tris[i].size();
		fwrite(&count,sizeof(count),1,f);
		counts.m_leafCount++;
		counts.m_triCount += count;
		for (size_t i = 0; i < num; i++) {
			DEBUG_ASSERT(tris[i].size() <= 4);
			for (size_t j = 0; j < tris[i].size(); j++) {
				const Vec3V v0 = Vec3V(tris[i].v0.x.f[j],tris[i].v0.y.f[j],tris[i].v0.z.f[j]);
				const Vec3V v1 = Vec3V(tris[i].v1.x.f[j],tris[i].v1.y.f[j],tris[i].v1.z.f[j]);
				const Vec3V v2 = Vec3V(tris[i].v2.x.f[j],tris[i].v2.y.f[j],tris[i].v2.z.f[j]);
				fwrite(&v0,3*sizeof(float),1,f);
				fwrite(&v1,3*sizeof(float),1,f);
				fwrite(&v2,3*sizeof(float),1,f);
			}
		}
	}
}

bool EmbreeSaveBVH4(const char* path,RTCScene scene,const geomesh::TriangleMesh* mesh,bool silent)
{
	if (!silent) {
		fprintf(stdout,"saving %s .. ",path);
		fflush(stdout);
	}
	const embree::AccelData* accel = reinterpret_cast<embree::Accel*>(scene)->intersectors.ptr;
	if (accel->type == embree::AccelData::TY_BVH4) {
		const embree::BVH4::NodeRef root = reinterpret_cast<const embree::BVH4*>(accel)->root;
		FILE* f = fopen(path,"wb");
		if (f) {
			BVHCounts counts;
			memset(&counts,0,sizeof(counts));
			fwrite(&counts,sizeof(counts),1,f); // reserve space in the file
			EmbreeSaveBVH4Internal(f,root,counts,mesh);
			rewind(f);
			fwrite(&counts,sizeof(counts),1,f);
			fclose(f);
			if (!silent) {
				fprintf(stdout,"done.\n");
				fprintf(stdout,"total node count = %u\n",counts.m_nodeCount);
				fprintf(stdout,"total leaf count = %u\n",counts.m_leafCount);
				fprintf(stdout,"total tri count = %u\n",counts.m_triCount);
			}
			return true;
		} else
			fprintf(stderr,"failed to save %s!\n",path);
	} else
		fprintf(stderr,"failed to save %s! BVH contains more than triangles! (type = %d)\n",path,accel->type);
	return false;
}
#endif // defined(_EMBREE)

#if BVH_STATS
//#if !defined(VISUAL_STUDIO_VERSION) || VISUAL_STUDIO_VERSION == 2017 // what?
//#define __popcnt16(x) (uint16)_popcnt32((int)x)
////VMATH_INLINE uint16 __popcnt16(uint16 x)
////{
////	return (uint16)_popcnt32((int)x);
////	uint16 result = 0;
////	for (uint16 bit = 1; bit; bit <<= 1)
////		if (x&bit)
////			result++;
////	return result;
////}
//#endif // VISUAL_STUDIO_VERSION == 2017

void BVHStats::EnterNode(uint32 mask,uint32 depth)
{
	m_nodeCount++;
	while (m_numberOfRaysEnteringNodeAtDepth.size() < depth + 1)
		m_numberOfRaysEnteringNodeAtDepth.push_back(RayPacketHistogram());
	m_numberOfRaysEnteringNodeAtDepth[depth].n[__popcnt(mask)]++;
}

void BVHStats::EnterLeaf(uint32 mask,uint32 depth)
{
	m_leafCount++;
	while (m_numberOfRaysEnteringLeafAtDepth.size() < depth + 1)
		m_numberOfRaysEnteringLeafAtDepth.push_back(RayPacketHistogram());
	m_numberOfRaysEnteringLeafAtDepth[depth].n[__popcnt(mask)]++;
}

void BVHStats::TestAgainstTriangle(uint32 mask,uint32 depth)
{
	m_triCount++;
	while (m_numberOfRaysTestingAgainstTriAtDepth.size() < depth + 1)
		m_numberOfRaysTestingAgainstTriAtDepth.push_back(RayPacketHistogram());
	m_numberOfRaysTestingAgainstTriAtDepth[depth].n[__popcnt(mask)]++;
}
#endif // BVH_STATS

template <typename T,size_t Size = sizeof(T),bool ZeroMemory = false> static const T ReadFile(FILE* f)
{
	StaticAssert(Size <= sizeof(T));
	T var;
	fread(&var,Size,1,f);
	if (ZeroMemory && Size < sizeof(T))
		memset(reinterpret_cast<char*>(&var) + Size,0,sizeof(T) - Size);
	return var;
}

static Vec3V_out FlipYZ(Vec3V_arg v,bool flipYZ)
{
	if (flipYZ)
		return Permute3<0,2,1>(v);
	else
		return v;
}

void BVH4Node::Leaf::SetPrimitives(const Triangle3V* triangles,uint32 count)
{
	const uint32 countSOA = (count + BVH_LEAF_NUM_TRIANGLES_SOA - 1)/BVH_LEAF_NUM_TRIANGLES_SOA;
	const uint32 countRoundedUp = countSOA*BVH_LEAF_NUM_TRIANGLES_SOA;
	resize(countSOA);
	if (count == 0)
		return; // what??
#if BVH_LEAF_NUM_TRIANGLES_SOA == 1
	for (uint32 i = 0; i < count; i++)
		operator[](i) = triangles[i];
#else
	m_count = count;
	std::vector<Triangle3V> trianglesPadded;
	if (countRoundedUp > count) {
		trianglesPadded.resize(countRoundedUp);
		memcpy(trianglesPadded.data(),triangles,count*sizeof(Triangle3V));
		for (uint32 i = count; i < countRoundedUp; i++)
			trianglesPadded[i] = triangles[count - 1];
		triangles = trianglesPadded.data();
	}
	for (uint32 i = 0; i < countSOA; i++)
		operator[](i) = TriangleType(triangles + i*BVH_LEAF_NUM_TRIANGLES_SOA);
#endif
}

BVH4Node::Leaf* BVH4Node::Leaf::LoadInternal(FILE* f,const Mat34V* transform BVH_STATS_ONLY(,uint32 depth))
{
	if (transform == NULL)
		transform = &Mat34V::StaticIdentity();
	Leaf* leaf = new BVH4Node::Leaf();
	BVH_STATS_ONLY(leaf->m_depth = depth);
	uint32 count;
	fread(&count,sizeof(count),1,f);
	Triangle3V* triangles = new Triangle3V[count];
	for (uint32 i = 0; i < count; i++) {
		const Vec3V v0 = transform->Transform(ReadFile<Vec3V,3*sizeof(float)>(f));
		const Vec3V v1 = transform->Transform(ReadFile<Vec3V,3*sizeof(float)>(f));
		const Vec3V v2 = transform->Transform(ReadFile<Vec3V,3*sizeof(float)>(f));
		triangles[i] = Triangle3V(v0,v1,v2);
	}
	leaf->SetPrimitives(triangles,count);
	delete[] triangles;
	return leaf;
}

void BVH4Node::Leaf::IntersectsRay(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const
{
	BVH_STATS_ONLY(stats.EnterLeaf(mask,m_depth));
	for (size_t i = 0; i < size(); i++) {
		BVH_STATS_ONLY(stats.TestAgainstTriangle(mask,m_depth));
		ScalarV t_;
		if (operator[](i).IntersectsRay(origin,dir,t_))
			t = Min(t_,t);
	}
}

void BVH4Node::Leaf::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA4_arg origin,Vec3V_SOA4_arg dir,Vec4V& t BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const
{
	BVH_STATS_ONLY(stats.EnterLeaf(mask,m_depth));
	for (size_t i = 0; i < size(); i++) {
		BVH_STATS_ONLY(stats.TestAgainstTriangle(mask,m_depth));
		Vec4V t_;
		if (operator[](i).IntersectsRay(origin,dir,t_,-1)) // TODO -- only process active rays
			t = Min(t_,t);
	}
}

#if HAS_VEC8V
void BVH4Node::Leaf::IntersectsRay(RAY_PACKET_ORIGIN_TYPE_SOA8_arg origin,Vec3V_SOA8_arg dir,Vec8V& t BVH_STATS_ONLY(,uint32 mask,BVHStats& stats)) const
{
	BVH_STATS_ONLY(stats.EnterLeaf(mask,m_depth));
	for (size_t i = 0; i < size(); i++) {
		BVH_STATS_ONLY(stats.TestAgainstTriangle(mask,m_depth));
		Vec8V t_;
		if (operator[](i).IntersectsRay(origin,dir,t_,-1)) // TODO -- only process active rays
			t = Min(t_,t);
	}
}
#endif // HAS_VEC8V

void BVH4Node::Leaf::IntersectsRaySign(Vec3V_arg origin,Vec3V_arg dir,ScalarV& t,ScalarV& sign) const
{
	for (unsigned i = 0; i < GetTriCount(); i++) {
		ScalarV t_;
		ScalarV sign_;
		if (GetTriangle(i).IntersectsRaySign(origin,dir,t_,sign_)) {
			if (t > t_) {
				t = t_;
				sign = sign_;
			}
		}
	}
}

void BVH4Node::Leaf::IntersectsRaySignAccum(Vec3V_arg origin,Vec3V_arg dir,ScalarV& sign) const
{
	for (unsigned i = 0; i < GetTriCount(); i++) {
		ScalarV t_;
		ScalarV sign_;
		if (GetTriangle(i).IntersectsRaySign(origin,dir,t_,sign_)) {
			if (sign_ > 0.0f)
				sign += ScalarV(V_ONE);
			else
				sign -= ScalarV(V_ONE);
		}
	}
}

BVH4Node* BVH4Node::Load(const char* path,const Mat34V* transform)
{
	BVH4Node* root = NULL;
	char path2[512];
	strcpy(path2,PathExt(path,".bvh4"));
	FILE* f = fopen(path2,"rb");
	if (f) {
		BVHCounts counts;
		fread(&counts,sizeof(counts),1,f);
		root = LoadInternal(f,transform);
		fclose(f);
	#if 1 // check!
		BVHCounts counts2 = root->Count();
		ForceAssert(counts2.m_nodeCount == counts.m_nodeCount);
		ForceAssert(counts2.m_leafCount == counts.m_leafCount);
		ForceAssertf(counts2.m_triCount == counts.m_triCount,"tri count doesn't match! expected %u, got %u",counts.m_triCount,counts2.m_triCount);
	#endif
	} else
		fprintf(stderr,"failed to load BVH %s!\n",path2);
	return root;
}

BVH4Node* BVH4Node::LoadInternal(FILE* f,const Mat34V* transform BVH_STATS_ONLY(,uint32 depth))
{
	if (transform == NULL)
		transform = &Mat34V::StaticIdentity();
	BVH4Node* node = new BVH4Node();
	BVH_STATS_ONLY(node->m_depth = depth);
	uint8 types[N];
	fread(types,sizeof(types),1,f);
	Box3V bounds[N];
	for (unsigned i = 0; i < N; i++) {
		if (types[i] != BVH_NODETYPE_EMPTY) {
			const Vec3V bmin = ReadFile<Vec3V,3*sizeof(float)>(f);
			const Vec3V bmax = ReadFile<Vec3V,3*sizeof(float)>(f);
			bounds[i] = transform->TransformBox(Box3V(bmin,bmax)); // not necessarily spacially efficient ..
		} else
			bounds[i] = Box3V(Vec3V(FLT_MAX),Vec3V(FLT_MAX)); // Box3V::Invalid() won't return empty intersection ..
	}
#if BVH_SOA_BOUNDS
	node->m_bounds = Box3V_SOA4(bounds);
#else
	for (unsigned i = 0; i < N; i++)
		node->m_bounds[i] = bounds[i];
#endif
	for (unsigned i = 0; i < N; i++) {
		if (types[i] == BVH_NODETYPE_EMPTY)
			node->m_children[i] = 0;
		else if (types[i] == BVH_NODETYPE_NODE)
			node->m_children[i] = reinterpret_cast<uintptr_t>(LoadInternal(f,transform BVH_STATS_ONLY(,depth + 1)));
		else if (types[i] == BVH_NODETYPE_LEAF)
			node->m_children[i] = reinterpret_cast<uintptr_t>(Leaf::LoadInternal(f,transform BVH_STATS_ONLY(,depth + 1))) | BVH_LEAF_FLAG;
		else
			DEBUG_ASSERT(false);
	}
	return node;
}

void BVH4Node::Release()
{
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
		if (IsChildLeaf(i))
			GetChildLeaf(i)->Release();
		else
			GetChildNode(i)->Release();
	}
	delete this;
}

const BVHCounts BVH4Node::Count() const
{
	BVHCounts counts(1,0,0);
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
		if (IsChildLeaf(i)) {
			counts.m_leafCount++;
			counts.m_triCount += GetChildLeaf(i)->GetTriCount();
		} else
			counts += GetChildNode(i)->Count();
	}
	return counts;
}

const Box3V BVH4Node::GetBounds() const
{
	Box3V bounds = Box3V::Invalid();
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++)
		bounds.Grow(GetChildBounds(i));
	return bounds;
}

bool BVH4Node::IntersectsSphere(const Sphere3V& sphere BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(,const Leaf*& leaf_,unsigned& index_)) const
{
#if BVH_SOA_BOUNDS_INTERSECT_SINGLE_RAYS_AGAINST_ALL_CHILDREN_SIMULTANEOUSLY
	uint32 childMask = IntersectsBox3V_SOA(sphere,m_bounds); // intersect all child bounds at once
	unsigned childIndex = 0;
	while (childMask && IsChildNonEmpty(childIndex)) { // lame .. do i need to test IsChildNonEmpty or can i rely on the intersection mask being 0?
		if (childMask & 1) {
			DEBUG_ASSERT(childIndex < N);// && IsChildNonEmpty(childIndex));
			if (IsChildLeaf(childIndex)) {
				const Leaf* leaf = GetChildLeaf(childIndex);
				for (unsigned j = 0; j < leaf->size(); j++) {
					if (leaf->operator[](j).IntersectsSphere(sphere)) { // if *any* triangles intersect sphere
					#if BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
						leaf_ = leaf;
						index_ = j;
					#endif // BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
						return true;
					}
				}
			} else if (GetChildNode(childIndex)->IntersectsSphere(sphere BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(,leaf_,index_)))
				return true;
		}
		childMask >>= 1;
		childIndex++;
	}
#else
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
		if (Intersects(sphere,GetChildBounds(i))) {
			if (IsChildLeaf(i)) {
				const Leaf* leaf = GetChildLeaf(i);
				for (unsigned j = 0; j < leaf->size(); j++) {
					if (leaf->operator[](j).IntersectsSphere(sphere)) { // if *any* triangles intersect sphere
					#if BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
						leaf_ = leaf;
						index_ = j;
					#endif // BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
						return true;
					}
				}
			} else if (GetChildNode(i)->IntersectsSphere(sphere BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(,leaf_,index_)))
				return true;
		}
	}
#endif
	return false;
}

#if BVH_LEAF_NUM_TRIANGLES_SOA <= 4
bool BVH4Node::IntersectsBox(const Box3V& box) const
{
#if BVH_SOA_BOUNDS_INTERSECT_SINGLE_RAYS_AGAINST_ALL_CHILDREN_SIMULTANEOUSLY
	uint32 childMask = IntersectsBox3V_SOA(box,m_bounds); // intersect all child bounds at once
	unsigned childIndex = 0;
	while (childMask && IsChildNonEmpty(childIndex)) { // lame .. do i need to test IsChildNonEmpty or can i rely on the intersection mask being 0?
		if (childMask & 1) {
			DEBUG_ASSERT(childIndex < N);// && IsChildNonEmpty(childIndex));
			if (IsChildLeaf(childIndex)) {
				const Leaf* leaf = GetChildLeaf(childIndex);
				for (unsigned j = 0; j < leaf->size(); j++) {
					if (leaf->operator[](j).IntersectsBox(box)) { // if *any* triangles intersect box
					#if BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
						leaf_ = leaf;
						index_ = j;
					#endif // BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
						return true;
					}
				}
			} else if (GetChildNode(childIndex)->IntersectsBox(box))
				return true;
		}
		childMask >>= 1;
		childIndex++;
	}
#else
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
		if (Intersects(box,GetChildBounds(i))) {
			if (IsChildLeaf(i)) {
				const Leaf* leaf = GetChildLeaf(i);
				for (unsigned j = 0; j < leaf->size(); j++) {
					if (leaf->operator[](j).IntersectsBox(box)) // if *any* triangles intersect box
						return true;
				}
			} else if (GetChildNode(i)->IntersectsBox(box))
				return true;
		}
	}
#endif
	return false;
}
#endif // BVH_LEAF_NUM_TRIANGLES_SOA <= 4

// TODO -- this method is quite fast since it doesn't involve shooting out hundreds of rays
// however, it's difficult to determine inside/outside with respect to the mesh ..
float BVH4Node::FindMinimumDistanceToSurface(Vec3V_arg pos,float startRadius,float step,unsigned numSteps) const
{
	float radius = startRadius;
	if (step == 0.0f)
		step = startRadius;
#if BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
	const Leaf* leaf = NULL; // track closest leaf and index into leaf's triangles
	unsigned leafIndex = 0;
	float leafRadius = 0.0f;
#endif // BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
	for (unsigned i = 0; i < numSteps; i++) {
		step *= 0.5f;
		if (IntersectsSphere(Sphere3V(pos,radius) BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(,leaf,leafIndex))) {
			BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST_ONLY(leafRadius = radius);
			radius -= step;
		} else
			radius += step;
	}
#if BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
	uint32 mask = leaf->operator[](leafIndex).IntersectsSphere(Sphere3V(pos,leafRadius));
	unsigned index = 0;
	while (mask) {
		if (mask & 1) {
			const Triangle3V triangle = leaf->operator[](leafIndex).GetIndexed(index); // this triangle intersects the minimum sphere
			if (Dot(triangle.GetNormal(),pos - triangle.m_positions[0]) < 0.0f)
				radius = -radius;
			break; // there is probably only one triangle intersecting the minimum sphere .. so ignore the rest
		}
		mask >>= 1;
		index++;
	}
#endif // BVH_INSIDE_OUTSIDE_TEST_VIA_SPHERE_TEST
	return radius;
}

float BVH4Node::FindMinimumDistanceToSurfaceEx(Vec3V_arg pos,Vec3V_arg cellSize,unsigned subCellResMinMax,unsigned subCellResGrad,float startRadius,float step,unsigned numSteps,float* out_dmin,float* out_dmax,Vec4V* out_dgrad) const
{
	Vec3V dgrad0(V_ZERO);
	Vec3V dgrad1(V_ZERO);
	bool dgradValid = false;
	float dcenter = 0.0f;
	bool dcenterValid = false;
	if (out_dmin || out_dmax) { // need to evaluate distance grid (and we can construct gradient with the same data)
		const unsigned sx = subCellResMinMax;
		const unsigned sy = subCellResMinMax;
		const unsigned sz = subCellResMinMax;
		float* g = new float[sx*sy*sz];
		float dmin = +FLT_MAX;
		float dmax = -FLT_MAX;
		for (unsigned k = 0; k < sz; k++) {
			for (unsigned j = 0; j < sy; j++) {
				for (unsigned i = 0; i < sx; i++) {
					const float x = -0.5f + (float)i/(float)(sx - 1); // [-0.5..0.5]
					const float y = -0.5f + (float)j/(float)(sy - 1); // [-0.5..0.5]
					const float z = -0.5f + (float)k/(float)(sz - 1); // [-0.5..0.5]
					const float d = FindMinimumDistanceToSurface(pos + cellSize*Vec3V(x,y,z),startRadius,step,numSteps);
					g[i + (j + k*sy)*sx] = d;
					dmin = Min(d,dmin);
					dmax = Max(d,dmax);
				}
			}
		}
		if (out_dmin) {
			if (IntersectsBox(Box3V(pos - cellSize*0.5f, pos + cellSize*0.5f)))
				dmin = 0.0f;
			*out_dmin = dmin;
		}
		if (out_dmax)
			*out_dmax = dmax;
		if (out_dgrad && subCellResGrad == subCellResMinMax) {
			ForceAssert((sx & sy & sz) == 1 && Min(sx,sy,sz) >= 3); // must all be odd and >= 3
			dgrad0 = Vec3V(
				g[(sx/2 - 1) + ((sy/2 + 0) + (sz/2 + 0)*sy)*sx],
				g[(sx/2 + 0) + ((sy/2 - 1) + (sz/2 + 0)*sy)*sx],
				g[(sx/2 + 0) + ((sy/2 + 0) + (sz/2 - 1)*sy)*sx]);
			dgrad1 = Vec3V(
				g[(sx/2 + 1) + ((sy/2 + 0) + (sz/2 + 0)*sy)*sx],
				g[(sx/2 + 0) + ((sy/2 + 1) + (sz/2 + 0)*sy)*sx],
				g[(sx/2 + 0) + ((sy/2 + 0) + (sz/2 + 1)*sy)*sx]);
			dgradValid = true;
		}
		dcenter = g[(sx/2) + ((sy/2) + (sz/2)*sy)*sx];
		dcenterValid = true;
		delete[] g;
	}
	if (!dcenterValid) {
		dcenter = FindMinimumDistanceToSurface(pos,startRadius,step,numSteps);
		dcenterValid = true;
	}
	if (out_dgrad) {
		const Vec3V subCellSize = cellSize/(float)(subCellResGrad - 1);
		if (!dgradValid) { // can compute gradient using 6 samples
			dgrad0.xf_ref() = FindMinimumDistanceToSurface(pos - subCellSize*Vec3V(V_XAXIS),startRadius,step,numSteps);
			dgrad0.yf_ref() = FindMinimumDistanceToSurface(pos - subCellSize*Vec3V(V_YAXIS),startRadius,step,numSteps);
			dgrad0.zf_ref() = FindMinimumDistanceToSurface(pos - subCellSize*Vec3V(V_ZAXIS),startRadius,step,numSteps);
			dgrad1.xf_ref() = FindMinimumDistanceToSurface(pos + subCellSize*Vec3V(V_XAXIS),startRadius,step,numSteps);
			dgrad1.yf_ref() = FindMinimumDistanceToSurface(pos + subCellSize*Vec3V(V_YAXIS),startRadius,step,numSteps);
			dgrad1.zf_ref() = FindMinimumDistanceToSurface(pos + subCellSize*Vec3V(V_ZAXIS),startRadius,step,numSteps);
			dgradValid = true;
		}
		*out_dgrad = Vec4V((dgrad1 - dgrad0)/(subCellSize*2.0f), dcenter); // store distance in gradient alpha channel too
	}
	return dcenter;
}

#if BVH_TILES
// TODO -- vectorize this loop, maybe storing frustum in SOA form?
BVHCommon::IntersectionCode BVHCommon::GetIntersectionCode(const Box3V& bounds,const Plane3V frustum[4])
{
	const Vec3V bmin = bounds.GetMin();
	const Vec3V bmax = bounds.GetMax();
	IntersectionCode result = BVH_INSIDE;
	for (unsigned j = 0; j < 4; j++) {
		const Vec3V::BoolV n(frustum[j].GetNormal()); // select bounds min/max components via sign of normal components
		if (frustum[j].GetDistanceToPoint(Select(bmax,bmin,n)) < 0.0f) // select n < 0 ? bmin : bmax, on a per-component basis
			return BVH_OUTSIDE; // bounds is completely outside of this plane
		else if (frustum[j].GetDistanceToPoint(Select(bmin,bmax,n)) < 0.0f) // select n < 0 ? bmax : bmin, on a per-component basis
			result = BVH_INTERSECTED; // bounds is partially outside of this plane, therefore not completely inside frustum
	}
	return result;
}

#if BVH_TILES_PATHCOMPRESSION
void BVHCommon::PathCompressionNode::Release()
{
	delete this;
}
#endif // BVH_TILES_PATHCOMPRESSION

// see An Improved Multi-Level Raytracing Algorithm - Joshua Barczak
// TODO -- implement path compression, not just Entry Point Search
uintptr_t BVH4Node::EntryPointSearch(const Plane3V frustum[4] BVH_STATS_ONLY(,EntryPointSearchStats& stats)) const
{
	IntersectionCode codes[N];
	bool all_out = true;
	bool all_in = true;
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
		codes[i] = GetIntersectionCode(GetChildBounds(i),frustum);
		switch (codes[i]) {
		case BVH_OUTSIDE: all_in = false; break;
		case BVH_INSIDE: all_out = false; break;
		default: all_in = all_out = false;
		}
		BVH_STATS_ONLY(stats.m_child_codes[codes[i]]++);
	}
#if BVH_STATS
	ForceAssert(!(all_out && all_in));
	if (all_out)
		stats.m_all_outs++;
	else if (all_in)
		stats.m_all_ins++;
	else
		stats.m_mixed_ins_outs++;
#endif // BVH_STATS
	if (all_out)
		return 0;
	else if (all_in)
		return reinterpret_cast<uintptr_t>(this);
	else {
		uintptr_t only = 0;
		for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
			if (codes[i] != BVH_OUTSIDE) {
				uintptr_t child = m_children[i]; // node or leaf
				if (codes[i] == BVH_INTERSECTED && !IsChildLeaf(i))
					child = reinterpret_cast<const BVH4Node*>(child)->EntryPointSearch(frustum BVH_STATS_ONLY(,stats));
				if (child) {
					if (only == 0)
						only = child;
					else
						return reinterpret_cast<uintptr_t>(this); // multiple children intersect, so return this node
				}
			}
		}
		return only;
	}
}

#if BVH_TILES_PATHCOMPRESSION
BVHCommon::PathCompressionNode* BVH4Node::PathCompression(const Plane3V frustum[4]) const
{
	ForceAssert(false); // TODO -- this code is not written yet! see http://www.joshbarczak.com/pathcompression.pdf. i have not assigned PathCompressionNode children, etc ..
	IntersectionCode codes[N];
	bool all_out = true;
	bool all_in = true;
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
		codes[i] = GetIntersectionCode(GetChildBounds(i),frustum);
		switch (codes[i]) {
		case BVH_OUTSIDE: all_in = false; break;
		case BVH_INSIDE: all_out = false; break;
		default: all_in = all_out = false;
		}
	}
	if (all_out)
		return 0;
	else if (all_in)
		return new PathCompressionNode(reinterpret_cast<uintptr_t>(this));
	else {
		PathCompressionNode* only = NULL;
		for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
			if (codes[i] != BVH_OUTSIDE) {
				PathCompressionNode* child; // node or leaf
				if (codes[i] == BVH_INTERSECTED && !IsChildLeaf(i))
					child = reinterpret_cast<const BVH4Node*>(child)->PathCompression(frustum);
				else
					child = new PathCompressionNode(m_children[i]);
				if (child) {
					if (only == NULL)
						only = child;
					else {
						only->Release();
						child->Release();
						return new PathCompressionNode(reinterpret_cast<uintptr_t>(this)); // multiple children intersect, so return this node
					}
				}
			}
		}
		return only;
	}
}
#endif // BVH_TILES_PATHCOMPRESSION
#endif // BVH_TILES

#if BVH_STATS
void BVH4Node::ReportTriCounts() const
{
	std::vector<size_t> histogram;
	ReportTriCountsInternal(histogram);
	for (size_t i = 0; i < histogram.size(); i++)
		printf("# leaves with %zd triangles = %zd\n",i,histogram[i]);
}

void BVH4Node::ReportTriCountsInternal(std::vector<size_t>& histogram) const
{
	for (unsigned i = 0; i < N && IsChildNonEmpty(i); i++) {
		if (IsChildLeaf(i)) {
			const Leaf* leaf = GetChildLeaf(i);
			while (histogram.size() <= leaf->GetTriCount())
				histogram.push_back(0);
			histogram[leaf->GetTriCount()]++;
		} else
			GetChildNode(i)->ReportTriCountsInternal(histogram);
	}
}
#endif // BVH_STATS