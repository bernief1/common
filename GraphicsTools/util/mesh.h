// =============
// common/mesh.h
// =============

#ifndef _INCLUDE_COMMON_MESH_H_
#define _INCLUDE_COMMON_MESH_H_

#include "common/common.h"

#include "vmath/vmath_common.h"
#include "vmath/vmath_matrix.h"
#include "vmath/vmath_triangle.h"
#include "vmath/vmath_vec4.h"

class ProgressDisplay;

class Box3V;
class Plane3V;
class Sphere3V;

namespace geomesh {

#define MESH_FORCE_ASSERT (1)

#if MESH_FORCE_ASSERT
#define MeshAssert          ForceAssert
#define MeshAssertf         ForceAssertf
#define MeshAssertMsg       ForceAssertMsg
#define MeshAssertVerify    ForceAssertVerify
#define MeshAssertVerifyf   ForceAssertVerifyf
#define MeshAssertVerifyMsg ForceAssertVerifyMsg
#else
#define MeshAssert          DEBUG_ASSERT
#define MeshAssertf         DEBUG_ASSERT_FMT
#define MeshAssertMsg       DEBUG_ASSERT_STR
#define MeshAssertVerify    AssertVerify
#define MeshAssertVerifyf   AssertVerifyf
#define MeshAssertVerifyMsg AssertVerifyMsg
#endif

#if PLATFORM_PS4
class String_mappable : public std::string // PS4 seems to have trouble with std::map<std::string,T>
{
public:
	inline String_mappable(const std::string& str) : std::string(str) {}
	inline String_mappable(const char* str) : std::string(str) {}
	inline bool operator <(const String_mappable& rhs) const { return strcmp(c_str(), rhs.c_str()) < 0; }
};
#else
typedef std::string String_mappable;
#endif

class IndexedQuad;
class IndexedTriangle
{
public:
	enum { NumVerts = 3 };

	inline IndexedTriangle() {}
	inline IndexedTriangle(uint32 vertStart,uint32 i0,uint32 i1,uint32 i2,int group): m_group(group)
	{
		m_indices[0] = vertStart + i0;
		m_indices[1] = vertStart + i1;
		m_indices[2] = vertStart + i2;
	}

	inline void Add(std::vector<IndexedTriangle>& tris) const
	{
		tris.push_back(*this);
	}

	inline void Add(std::vector<IndexedQuad>& quads) const;

	inline static uint32 AddPolys(std::vector<IndexedTriangle>& polys,uint32 vertStart,const uint32* indices,uint32 numIndices,int group = -1)
	{
		uint32 numPolys = 0;
		for (uint32 i = 2; i < numIndices; i++) {
			polys.push_back(IndexedTriangle(vertStart,indices[0],indices[i - 1],indices[i],group));
			numPolys++;
		}
		return numPolys;
	}

	inline const IndexedTriangle CopyPoly(uint32 vertStart = 0,int group = -1) const
	{
		return IndexedTriangle(vertStart,m_indices[0],m_indices[1],m_indices[2],group);
	}

	inline bool operator <(const IndexedTriangle& rhs) const
	{
		return m_group < rhs.m_group;
	}

	uint32 m_indices[NumVerts];
	int m_group; // -1 means "none"
};

class IndexedQuad
{
public:
	enum { NumVerts = 4 };

	inline IndexedQuad() {}
	inline IndexedQuad(uint32 vertStart,uint32 i0,uint32 i1,uint32 i2,uint32 i3,int group): m_group(group)
	{
		m_indices[0] = vertStart + i0;
		m_indices[1] = vertStart + i1;
		m_indices[2] = vertStart + i2;
		m_indices[3] = vertStart + i3;
	}

	inline const IndexedTriangle GetTriangle(unsigned index) const
	{
		MeshAssert(index < 2);
		if (index == 0)
			return IndexedTriangle(0,m_indices[0],m_indices[1],m_indices[2],m_group);
		else
			return IndexedTriangle(0,m_indices[0],m_indices[2],m_indices[3],m_group);
	}

	inline void Add(std::vector<IndexedTriangle>& tris) const
	{
		tris.push_back(GetTriangle(0));
		tris.push_back(GetTriangle(1));
	}

	inline void Add(std::vector<IndexedQuad>& quads) const
	{
		quads.push_back(*this);
	}

	inline static uint32 AddPolys(std::vector<IndexedQuad>& polys,uint32 vertStart,const uint32* indices,uint32 numIndices,int group = -1)
	{
		uint32 numPolys = 0;
		for (uint32 i = 3; i < numIndices; i += 2) {
			polys.push_back(IndexedQuad(vertStart,indices[0],indices[i - 2],indices[i - 1],indices[i],group));
			numPolys++;
		}
		if (numIndices&1) {
			polys.push_back(IndexedQuad(0,indices[0],indices[numIndices - 2],indices[numIndices - 1],indices[numIndices - 1],group));
			numPolys++;
		}
		return numPolys;
	}

	inline const IndexedQuad CopyPoly(uint32 vertStart = 0,int group = -1) const
	{
		return IndexedQuad(vertStart,m_indices[0],m_indices[1],m_indices[2],m_indices[3],group);
	}

	inline bool operator <(const IndexedQuad& rhs) const
	{
		return m_group < rhs.m_group;
	}

	uint32 m_indices[NumVerts];
	int m_group; // -1 means "none"
};

inline void IndexedTriangle::Add(std::vector<IndexedQuad>& quads) const
{
	quads.push_back(IndexedQuad(0,m_indices[0],m_indices[1],m_indices[2],m_indices[2],m_group));
}

const Triangle3V MakePoly(const IndexedTriangle& tri, const std::vector<Vec3V>& verts);
const Triangle3V MakePoly(const IndexedTriangle& tri, const std::vector<Vec2V>& verts, float z = 0.0f);
const Quad3V MakePoly(const IndexedQuad& quad, const std::vector<Vec3V>& verts);
const Quad3V MakePoly(const IndexedQuad& quad, const std::vector<Vec2V>& verts, float z = 0.0f);
const Triangle3V_SOA4 MakePoly_SOA4(const IndexedTriangle tris[4], const std::vector<Vec3V>& verts);
#if HAS_VEC8V
const Triangle3V_SOA8 MakePoly_SOA8(const IndexedTriangle tris[8], const std::vector<Vec3V>& verts);
#endif // HAS_VEC8V

class MeshBase
{
public:
	MeshBase() : m_normals(NULL), m_texcoords(NULL), m_colors(NULL) {}
	~MeshBase()
	{
		if (m_normals) delete m_normals;
		if (m_texcoords) delete m_texcoords;
		if (m_colors) delete m_colors;
	}

	void CheckVertexStreams() const;
	void CheckVertexIndex(uint32 index) const;
	void FillVertexNormals(Vec3V_arg normal); // fill remaining normals with constant value
	void FillVertexTexcoords(Vec2V_arg texcoord); // fill remaining texcoords with constant value
	void FillVertexColors(Vec4V_arg color); // fill remaining colors with constant value

	const Box3V GetBounds() const;
	int FindGroup(const char* name) const;
	int FindOrAddGroup(const char* name);
	void AddInfo(const char* format, ...);

	class Vertex
	{
	public:
		const Vertex Transform(const Mat34V* transform, const Vec4V* texScaleOffset) const; // do nothing if transform is NULL
		uint64 GetCrcHash() const;

		Vec3V m_pos;
		Vec3V m_normal;
		Vec2V m_texcoord;
		Vec4V m_color;
	};

	const Vertex GetVertex(uint32 index) const;
	static const Vertex Interpolate(const Vertex& v0, const Vertex& v1, ScalarV_arg t, bool normalize); // v0 + (v1 - v0)*t
	uint32 AddVertex(const Vertex& v, bool useMap);
	uint32 AddInterpolatedVertex(uint32 index0, uint32 index1, ScalarV_arg t, bool normalize, bool useMap);

	void Clear()
	{
		m_groupNames.clear();
		m_groupNameMap.clear();
		m_vertexMap.clear();
		m_verts.clear();
		if (m_normals) { delete m_normals; m_normals = NULL; }
		if (m_texcoords) { delete m_texcoords; m_texcoords = NULL; }
		if (m_colors) { delete m_colors; m_colors = NULL; }
		m_info.clear();
	}

	std::vector<std::string> m_groupNames; // indexed by poly->m_group
	std::map<String_mappable,int> m_groupNameMap; // index into m_groupNames
	std::map<uint64,uint32> m_vertexMap; // maps vertex crc hash -> vertex index
	std::vector<Vec3V> m_verts;
	std::vector<Vec3V>* m_normals; // optional
	std::vector<Vec2V>* m_texcoords; // optional
	std::vector<Vec4V>* m_colors; // optional
	std::vector<std::string> m_info; // can store optional information here (e.g. camera orientation)
};

template <typename PolyType> class Mesh : public MeshBase
{
public:
	void Clear() { m_polys.clear(); MeshBase::Clear(); }
	std::vector<PolyType> m_polys; // e.g. triangles or quads
};

class TriangleMesh : public Mesh<IndexedTriangle>
{
public:
	typedef IndexedTriangle IndexedPolyType;
	Vec3V_out GetTriangleVertexPos(uint32 triIndex, unsigned i) const;
};

class QuadMesh : public Mesh<IndexedQuad>
{
public:
	typedef IndexedQuad IndexedPolyType;
	Vec3V_out GetQuadVertexPos(uint32 quadIndex, unsigned i) const;
};

float CalculateMeshSurfaceArea(const TriangleMesh& mesh, bool textureSpace);
float CalculateMeshSurfaceArea(const QuadMesh& mesh, bool textureSpace);

bool LoadOBJ(const char* path, TriangleMesh& mesh, bool loadTexcoordsAndNormals = true, const Mat34V* transform = NULL);
bool LoadOBJ(const char* path, QuadMesh& mesh, bool loadTexcoordsAndNormals = true, const Mat34V* transform = NULL);

bool SaveOBJ(const char* path, const TriangleMesh& mesh, const char* materialLib = NULL, const std::map<std::string,std::string>* materialMap = NULL, bool saveUniqueElements = true);
bool SaveOBJ(const char* path, const QuadMesh& mesh, const char* materialLib = NULL, const std::map<std::string,std::string>* materialMap = NULL, bool saveUniqueElements = true);

void SaveOBJStream(FILE* file, const TriangleMesh& mesh, uint32& vrtIndex, uint32& nrmIndex, uint32& texIndex, const std::map<std::string,std::string>* materialMap = NULL, bool saveUniqueElements = true);
void SaveOBJStream(FILE* file, const QuadMesh& mesh, uint32& vrtIndex, uint32& nrmIndex, uint32& texIndex, const std::map<std::string,std::string>* materialMap = NULL, bool saveUniqueElements = true);

Mat33V_out GetBoxFaceAxes(uint32 faceIndex);

bool AddUVMappingToOBJ(const char* objPath, const char* objWithUVsPath); // to merge exported zbrush UVs with original OBJ

enum
{
	BOX_FACE_FLAG_POS_X = BIT(0),
	BOX_FACE_FLAG_POS_Y = BIT(1),
	BOX_FACE_FLAG_POS_Z = BIT(2),
	BOX_FACE_FLAG_NEG_X = BIT(3),
	BOX_FACE_FLAG_NEG_Y = BIT(4),
	BOX_FACE_FLAG_NEG_Z = BIT(5),
	BOX_FACE_FLAGS_ALL = BOX_FACE_FLAG_POS_X | BOX_FACE_FLAG_POS_Y | BOX_FACE_FLAG_POS_Z | BOX_FACE_FLAG_NEG_X | BOX_FACE_FLAG_NEG_Y | BOX_FACE_FLAG_NEG_Z,
};

void ConstructBox(TriangleMesh& mesh, const Box3V& box, uint32 boxFaceFlags = BOX_FACE_FLAGS_ALL, const Mat34V* transform = NULL, float uvDensity = 0.0f);
void ConstructBox(QuadMesh& mesh, const Box3V& box, uint32 boxFaceFlags = BOX_FACE_FLAGS_ALL, const Mat34V* transform = NULL, float uvDensity = 0.0f);

void ConstructBoxFrame(TriangleMesh& mesh, const Box3V& box, float thickness = 1.0f/64.0f, const Mat34V* transform = NULL, bool outerOnly = false);
void ConstructBoxFrame(QuadMesh& mesh, const Box3V& box, float thickness = 1.0f/64.0f, const Mat34V* transform = NULL, bool outerOnly = false);

void ConstructBoxLine(TriangleMesh& mesh, Vec3V_arg origin, Vec3V_arg dir, float length, float radius = 0.01f);
void ConstructBoxLine(QuadMesh& mesh, Vec3V_arg origin, Vec3V_arg dir, float length, float radius = 0.01f);

void ConstructSphere(TriangleMesh& mesh, const Sphere3V& sphere, const Mat34V* transform = NULL, unsigned numSlices = 32, unsigned numStacks = 40);
void ConstructSphere(QuadMesh& mesh, const Sphere3V& sphere, const Mat34V* transform = NULL, unsigned numSlices = 32, unsigned numStacks = 40);

void ConstructRoundBox(TriangleMesh& mesh, const Box3V& box, float roundRadius, float vertDensity = 32.0f, float uvDensity = 0.0f, unsigned uvPadding = 1, uint32 faceIgnoreMask = 0, bool optimized = false, bool uniformAngularSpacing = true);
void ConstructRoundBox(QuadMesh& mesh, const Box3V& box, float roundRadius, float vertDensity = 32.0f, float uvDensity = 0.0f, unsigned uvPadding = 1, uint32 faceIgnoreMask = 0, bool optimized = false, bool uniformAngularSpacing = true);

void ConstructTessellatedQuad(TriangleMesh& mesh, const Vec3V corners[4], unsigned tess_x, unsigned tess_y, const Mat34V* transform = NULL);
void ConstructTessellatedQuad(QuadMesh& mesh, const Vec3V corners[4], unsigned tess_x, unsigned tess_y, const Mat34V* transform = NULL);

void AddTinyCubesForNormals(TriangleMesh& mesh, float offset, float size);
void AddTinyCubesForNormals(QuadMesh& mesh, float offset, float size);

bool PolyClip(Vec3V* dst, unsigned& dstCount, unsigned dstCountMax, const Vec3V* src, unsigned srcCount, const Plane3V& plane);
bool PolyClipEx(MeshBase::Vertex* dst, unsigned& dstCount, unsigned dstCountMax, const MeshBase::Vertex* src, const unsigned srcCount, const Plane3V& plane, bool normalize);

uint32 AddGeometry(TriangleMesh& mesh, const TriangleMesh& object, const Mat34V* objectTransform = NULL, const Box3V* clip = NULL, const Mat34V* clipTransform = NULL, const Vec4V* texScaleOffset = NULL, const char* groupNameSuffix = NULL);
uint32 AddGeometry(QuadMesh& mesh, const QuadMesh& object, const Mat34V* objectTransform = NULL, const Box3V* clip = NULL, const Mat34V* clipTransform = NULL, const Vec4V* texScaleOffset = NULL, const char* groupNameSuffix = NULL);

void ConstructNormals(TriangleMesh& mesh);
void ConstructNormals(QuadMesh& mesh);

void Tessellate4(TriangleMesh& mesh); // tessellates each triangle into 4 triangles
void TessellateToEdgeLength(TriangleMesh& mesh, float edgeLength, ProgressDisplay* progress = NULL, int progressPeriod = 1);

void WeldPositions(TriangleMesh& mesh, float tolerance);
void WeldPositions(QuadMesh& mesh, float tolerance);

void ConvertQuadMeshToTriangleMesh(TriangleMesh& mesh, const QuadMesh& quadMesh);

bool GenerateTestCylinderObject(const char* path, const char* mtllib, const char* material, Vec3V_arg origin, Vec3V_arg axis, float radius, float textureRepeats, unsigned numInstances, unsigned numSlices = 32, unsigned numStacks = 40, bool caps = true);

bool CreateZUPOBJ(const char* path, const char* ext = "_z_up.obj");
bool CreateTransformedOBJ(const char* path, Mat34V_arg transform, const char* ext);

} // namespace geomesh

#endif // _INCLUDE_COMMON_MESH_H_