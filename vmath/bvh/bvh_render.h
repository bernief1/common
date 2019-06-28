// =======================
// common/bvh/bvh_render.h
// =======================

#ifndef _INCLUDE_BVH_RENDER_
#define _INCLUDE_BVH_RENDER_

#include "common/common.h"

#include "vmath/bvh/bvh.h"

#include "vmath/vmath_common.h"
#include "vmath/vmath_matrix.h"
#include "vmath/vmath_triangle.h"

void RenderTriangles_1x1(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
void RenderTriangles_4x1(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
void RenderTriangles_2x2(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
#if HAS_VEC8V
void RenderTriangles_8x1(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
void RenderTriangles_4x2(const std::vector<Triangle3V>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
#endif // HAS_VEC8V

void RenderTriangles_tri4(const std::vector<Triangle3V_SOA4>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
#if HAS_VEC8V
void RenderTriangles_tri8(const std::vector<Triangle3V_SOA8>& triangles, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
#endif // HAS_VEC8V

void RenderTriangles_BVH_1x1(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads));
void RenderTriangles_BVH_4x1(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads));
void RenderTriangles_BVH_2x2(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads));
#if HAS_VEC8V
void RenderTriangles_BVH_8x1(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads));
void RenderTriangles_BVH_4x2(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path BVH_THREADS_ONLY(, unsigned numThreads));
#endif // HAS_VEC8V

#if defined(_EMBREE_SOURCE)
void RenderTriangles_4x1_EMBREE(RTCScene scene, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
void RenderTriangles_8x1_EMBREE(RTCScene scene, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
#endif // defined(_EMBREE_SOURCE)

#if BVH_TILES
void RenderTriangles_BVH_TILES_1x1(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
void RenderTriangles_BVH_TILES_4x1(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
void RenderTriangles_BVH_TILES_2x2(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
#if HAS_VEC8V
void RenderTriangles_BVH_TILES_8x1(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
void RenderTriangles_BVH_TILES_4x2(const BVH4Node* root, Mat34V_arg camera, float tanVFOV, float* zbuf, unsigned w, unsigned h, unsigned tileW, unsigned tileH, bool zclear, float& zscale, float& zoffset, bool& calc_z_range, const char* path);
#endif // HAS_VEC8V
#endif // BVH_TILES

void RenderOcclusion(const BVH4Node* root, const std::vector<Vec3V>& verts, const std::vector<Vec3V>& normals, std::vector<float>& occlusion, unsigned numSamples BVH_THREADS_ONLY(, unsigned numThreads));

#endif // _INCLUDE_BVH_RENDER_
