// ==========================
// common/bvh/bvh_builder.cpp
// ==========================

#include "bvh_builder.h"
//#include "../mesh.h"

BVH4Node* BuildBVH4(const geomesh::TriangleMesh& mesh, const Mat34V* transform)
{
	ProgressDisplay progress("building BVH4");
	std::vector<Triangle3V> triangles;
	const uint32 numTriangles = (uint32)mesh.m_polys.size();
	triangles.resize(numTriangles);
	for (uint32 i = 0; i < mesh.m_polys.size(); i++) {
		//triangles[i] = MakePoly(mesh.m_polys[i], mesh.m_verts);
		for (uint32 k = 0; k < 3; k++) {
			Vec3V p = mesh.m_verts[mesh.m_polys[i].m_indices[k]];
			if (transform)
				p = transform->Transform(p);
			triangles[i].m_positions[k] = p;
		}
	}
	BVH4Node* node = BVHBuilder<BVH4Node,BVHCommon::Leaf,Triangle3V>::BuildNode(triangles, 0, numTriangles);
	progress.End();
	return node;
}