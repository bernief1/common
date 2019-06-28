// ========================
// common/bvh/bvh_builder.h
// ========================

#ifndef _INCLUDE_BVH_BUILDER_H_
#define _INCLUDE_BVH_BUILDER_H_

#include "bvh.h"
#include "GraphicsTools/util/mesh.h"
#include "GraphicsTools/util/progressdisplay.h"

template <typename NodeType, typename LeafType, typename PrimType> class BVHBuilder
{
private:
	class PrimRef : public PrimType
	{
	public:
		inline static uint32& dim() { static uint32 s_dim; return s_dim; }
		PrimRef() {}
		PrimRef(const PrimType& prim) : PrimType(prim), m_centroid(prim.GetBounds().GetCenter()) {}
		bool operator <(const PrimRef& rhs) const { return m_centroid[dim()] < rhs.m_centroid[dim()]; }
		Vec3V m_centroid;
	};

	template <typename T> static const Box3V GetRangeBounds(const std::vector<T>& prims, uint32 start, uint32 end)
	{
		ForceAssertf(start <= end,"start=%d,end=%u",start,end);
		Box3V bounds = Box3V::Invalid();
		for (uint32 i = start; i < end; i++)
			bounds.Grow(prims[i].GetBounds());
		return bounds;
	}

	template <typename T> static float GetRangeArea(const std::vector<T>& prims, uint32 start, uint32 end)
	{
		return GetArea(GetRangeBounds(prims, start, end));
	}

	static float GetArea(const Box3V& bounds)
	{
		if (bounds.IsValid()) {
			const Vec3V extent = bounds.GetExtent();
			const float dx = extent.xf();
			const float dy = extent.yf();
			const float dz = extent.zf();
			return 8.0f*(dx*dy + dy*dz + dz*dx);
		} else
			return 0.0f;
	}

	static uint32 SplitRange(std::vector<PrimType>& prims, uint32 start, uint32 end, uint32 maxPrimsPerLeaf)
	{
		ForceAssertf(start <= end,"start=%d,end=%u",start,end);
		if (end - start > maxPrimsPerLeaf) {
		#if 1 // try something simpler .. max centroid extent median split
			Box3V bounds = Box3V::Invalid();
			for (uint32 i = start; i < end; i++)
				bounds.Grow(prims[i].GetBounds().GetCenter()); // centroid bounds
			const Vec3V center = bounds.GetCenter();
			const Vec3V extent = bounds.GetExtent();
			std::vector<PrimRef> refs;
			refs.resize(end - start);
			for (uint32 i = start; i < end; i++)
				refs[i - start] = PrimRef(prims[i]);
			const uint32 dim = PrimRef::dim() = MaxElementIndex(extent);
			std::sort(refs.begin(), refs.end());
			uint32 split = end;
			for (uint32 i = start; i < end; i++) {
				prims[i] = refs[i - start]; // copy sorted prims in place
				if (split == end && prims[i].GetBounds().GetCenter()[dim] > center[dim])
					split = i;
			}
			if (split == start)
				split++;
			else if (split == end)
				split--;
			return split;
		#else
			const float traversalCost = 0.0f; // TODO -- tune these? expose these?
			const float primitiveCost = 1.0f;
			Vec3V lowestCostV(FLT_MAX);
			uint32 lowestCostSplit[3] = {0,0,0};
			std::vector<PrimRef> refs[3];
			for (uint32 dim = 0; dim < 3; dim++) {
				PrimRef::dim() = dim;
				refs[dim].resize(end - start);
				for (uint32 i = start; i < end; i++)
					refs[dim][i - start] = PrimRef(prims[i]);
				std::sort(refs[dim].begin(), refs[dim].end());
			#if 0 // O(N^3), gross.
				for (uint32 split = start + 1; split < end; split++) {
					const float leftCost = primitiveCost*GetRangeArea(refs[dim], 0, split - start)*(float)(split - start);
					const float rightCost = primitiveCost*GetRangeArea(refs[dim], split - start, end - start)*(float)(end - split);
					const float cost = traversalCost + leftCost + rightCost;
					if (lowestCostV[dim] > cost) {
						lowestCostV[dim] = cost;
						lowestCostSplit[dim] = split;
					}
				}
			#else
				std::vector<float> leftAreas;
				std::vector<float> rightAreas;
				leftAreas.resize(end - start - 1);
				rightAreas.resize(end - start - 1);
				Box3V bounds = Box3V::Invalid();
				for (uint32 split = start + 1; split < end; split++) {
					bounds.Grow(refs[dim][split - start - 1].GetBounds());
					leftAreas[split - start - 1] = GetArea(bounds);
				}
				bounds = Box3V::Invalid();
				for (uint32 split = end - 1; split > start; split--) {
					bounds.Grow(refs[dim][split - start].GetBounds());
					rightAreas[split - start - 1] = GetArea(bounds);
				}
				for (uint32 split = start + 1; split < end; split++) {
					const float leftCost = primitiveCost*leftAreas[split - start - 1]*(float)(split - start);
					const float rightCost = primitiveCost*rightAreas[split - start - 1]*(float)(end - split);
					const float cost = traversalCost + leftCost + rightCost;
					if (lowestCostV[dim] > cost) {
						lowestCostV[dim] = cost;
						lowestCostSplit[dim] = split;
					}
				}
			#endif
			}
			const uint32 dim = MinElementIndex(lowestCostV);
			const float baseCost = primitiveCost*GetRangeArea(prims, start, end)*(float)(end - start);
			const float splitCost = lowestCostV[dim];
			if (splitCost < baseCost) {
				for (uint32 i = start; i < end; i++)
					prims[i] = refs[dim][i - start]; // copy sorted prims in place
				return lowestCostSplit[dim];
			}
		#endif
		}
		return end; // no split
	}

	static void AddChild(NodeType* node, Box3V bounds[4], uint32& childCount, std::vector<PrimType>& prims, uint32 start, uint32 end, uint32 maxPrimsPerLeaf)
	{
		ForceAssertf(start <= end,"start=%d,end=%u",start,end);
		const uint32 numPrims = end - start;
		if (numPrims > 0) {
			bounds[childCount] = GetRangeBounds(prims, start, end);
			if (numPrims > maxPrimsPerLeaf) {
				const NodeType* child = BuildNode(prims, start, end, maxPrimsPerLeaf);
				node->m_children[childCount] = (uintptr_t)child;
			} else {
				LeafType* leaf = new LeafType;
				for (uint32 i = start; i < end; i++)
					leaf->SetPrimitives(prims.data() + start, end - start);
				node->m_children[childCount] = BVHCommon::BVH_LEAF_FLAG | (uintptr_t)leaf;
			}
			childCount++;
		}
	}

public:
	static NodeType* BuildNode(std::vector<PrimType>& prims, uint32 start, uint32 end, uint32 maxPrimsPerLeaf = 4)
	{
		ForceAssertf(start <= end,"start=%d,end=%u",start,end);
		NodeType* node = new NodeType;
		Box3V bounds[NodeType::N];
		uint32 childCount = 0;
		if (NodeType::N == 2) {
			const uint32 split1 = SplitRange(prims, start, end, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, start, split1, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split1, end, maxPrimsPerLeaf);
		} else if (NodeType::N == 4) {
			const uint32 split1 = SplitRange(prims, start, end, maxPrimsPerLeaf);
			const uint32 split2 = SplitRange(prims, start, split1, maxPrimsPerLeaf);
			const uint32 split3 = SplitRange(prims, split1, end, maxPrimsPerLeaf);
			//printf("start=%u,split2=%u,split1=%u,split3=%u,end=%u\n",start,split2,split1,split3,end);
			AddChild(node, bounds, childCount, prims, start, split2, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split2, split1, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split1, split3, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split3, end, maxPrimsPerLeaf);
		} else if (NodeType::N == 8) {
			const uint32 split1 = SplitRange(prims, start, end, maxPrimsPerLeaf);
			const uint32 split2 = SplitRange(prims, start, split1, maxPrimsPerLeaf);
			const uint32 split3 = SplitRange(prims, split1, end, maxPrimsPerLeaf);
			const uint32 split4 = SplitRange(prims, start, split2, maxPrimsPerLeaf);
			const uint32 split5 = SplitRange(prims, split2, split1, maxPrimsPerLeaf);
			const uint32 split6 = SplitRange(prims, split1, split3, maxPrimsPerLeaf);
			const uint32 split7 = SplitRange(prims, split3, end, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, start, split4, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split4, split2, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split2, split5, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split5, split1, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split1, split6, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split6, split3, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split3, split7, maxPrimsPerLeaf);
			AddChild(node, bounds, childCount, prims, split7, end, maxPrimsPerLeaf);
		} else
			ForceAssert(false); // not supported
		for (uint32 i = childCount; i < NodeType::N; i++) {
			bounds[i] = Box3V::Invalid();
			node->m_children[i] = 0;
		}
	#if BVH_SOA_BOUNDS
		node->m_bounds = bounds;
	#else
		memcpy(node->m_bounds, bounds, sizeof(bounds));
	#endif
		return node;
	}
};

BVH4Node* BuildBVH4(const geomesh::TriangleMesh& mesh, const Mat34V* transform = nullptr);

#endif // _INCLUDE_BVH_BUILDER_H_