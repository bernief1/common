// ===============
// common/mesh.cpp
// ===============

#include "mesh.h"

#include "crc.h"
#include "fileutil.h"
#include "progressdisplay.h"
#include "stringutil.h"

//#include "imageutil.h"

#include "vmath/vmath_intersects.h"

#if PLATFORM_PS4
namespace std
{
	class string_mappable : public string
	{
	public:
		template <typename T> string_mappable(const T str) : string(str) {}
		inline bool operator <(const string_mappable& rhs) const { return strcmp(c_str(), rhs.c_str()) < 0; }
	};
}
#else
#define string_mappable string
#endif

template <typename T> class Vec_T_mappable : public T
{
public:
	inline Vec_T_mappable(typename T::ArgType v) : T(v) {}
	inline bool operator <(const Vec_T_mappable<T>& rhs) const { return memcmp(this, &rhs, T::NumElements*sizeof(typename T::ElementType)) < 0; }
};
typedef Vec_T_mappable<Vec2V> Vec2V_mappable;
typedef Vec_T_mappable<Vec3V> Vec3V_mappable;
typedef Vec_T_mappable<Vec4V> Vec4V_mappable;

namespace geomesh {

const Triangle3V MakePoly(const IndexedTriangle& tri, const std::vector<Vec3V>& verts)
{
	return Triangle3V(
		verts[tri.m_indices[0]],
		verts[tri.m_indices[1]],
		verts[tri.m_indices[2]]);
}

const Triangle3V MakePoly(const IndexedTriangle& tri, const std::vector<Vec2V>& verts, float z)
{
	return Triangle3V(
		Vec3V(verts[tri.m_indices[0]], z),
		Vec3V(verts[tri.m_indices[1]], z),
		Vec3V(verts[tri.m_indices[2]], z));
}

const Quad3V MakePoly(const IndexedQuad& quad, const std::vector<Vec3V>& verts)
{
	return Quad3V(
		verts[quad.m_indices[0]],
		verts[quad.m_indices[1]],
		verts[quad.m_indices[2]],
		verts[quad.m_indices[3]]);
}

const Quad3V MakePoly(const IndexedQuad& quad, const std::vector<Vec2V>& verts, float z)
{
	return Quad3V(
		Vec3V(verts[quad.m_indices[0]], z),
		Vec3V(verts[quad.m_indices[1]], z),
		Vec3V(verts[quad.m_indices[2]], z),
		Vec3V(verts[quad.m_indices[3]], z));
}

const Triangle3V_SOA4 MakePoly_SOA4(const IndexedTriangle tris[4], const std::vector<Vec3V>& verts)
{
	Triangle3V_SOA4 result;
#if defined(VMATH_USE_GATHER_SCATTER)
	// we could avoid the first 3 gather's if we want to do a bunch of fixed shuffling ..
	//const __m128i* indices = reinterpret_cast<const __m128i*>(tris);
	//const __m128i temp0 = _mm_loadu_si128(indices);
	//__m128i temp1 = _mm_loadu_si128(indices + 1);
	//__m128i temp3 = _mm_loadu_si128(indices + 2);
	//const __m128i temp2 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(temp1),_mm_castsi128_ps(temp3),VMATH_PERMUTE(2,3,0,0)));
	//temp1 = _mm_alignr_epi8(temp1,temp0,12);
	//temp3 = _mm_shuffle_epi32(temp3,VMATH_PERMUTE(1,2,3,3));
	const uint32 t = sizeof(IndexedTriangle)/sizeof(uint32);
	const __m128i one = _mm_set1_epi32(1);
	const __m128i offsets0 = _mm_setr_epi32(0, t, t*2, t*3);
	const __m128i offsets1 = _mm_add_epi32(offsets0, one); // {1,4,7,10}
	const __m128i offsets2 = _mm_add_epi32(offsets1, one); // {2,5,8,11}
	const float* src = reinterpret_cast<const float*>(&verts.front());
	__m128i addr0 = _mm_slli_epi32(_mm_i32gather_epi32(reinterpret_cast<const int*>(tris), offsets0, sizeof(uint32)), 2);
	__m128i addr1 = _mm_slli_epi32(_mm_i32gather_epi32(reinterpret_cast<const int*>(tris), offsets1, sizeof(uint32)), 2);
	__m128i addr2 = _mm_slli_epi32(_mm_i32gather_epi32(reinterpret_cast<const int*>(tris), offsets2, sizeof(uint32)), 2);
	__m128* dst = reinterpret_cast<__m128*>(&result.m_positions[0]);
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr0, 4)); addr0 = _mm_add_epi32(addr0, one);
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr0, 4)); addr0 = _mm_add_epi32(addr0, one);
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr0, 4)); 
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr1, 4)); addr1 = _mm_add_epi32(addr1, one);
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr1, 4)); addr1 = _mm_add_epi32(addr1, one);
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr1, 4)); 
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr2, 4)); addr2 = _mm_add_epi32(addr2, one);
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr2, 4)); addr2 = _mm_add_epi32(addr2, one);
	_mm_store_ps(reinterpret_cast<float*>(dst++), _mm_i32gather_ps(src, addr2, 4));
#else
	const Triangle3V t0 = MakePoly(tris[0], verts);
	const Triangle3V t1 = MakePoly(tris[1], verts);
	const Triangle3V t2 = MakePoly(tris[2], verts);
	const Triangle3V t3 = MakePoly(tris[3], verts);
	result.m_positions[0] = Vec3V_SOA4(t0.m_positions[0], t1.m_positions[0], t2.m_positions[0], t3.m_positions[0]);
	result.m_positions[1] = Vec3V_SOA4(t0.m_positions[1], t1.m_positions[1], t2.m_positions[1], t3.m_positions[1]);
	result.m_positions[2] = Vec3V_SOA4(t0.m_positions[2], t1.m_positions[2], t2.m_positions[2], t3.m_positions[2]);
#endif
	return result;
}

#if HAS_VEC8V
const Triangle3V_SOA8 MakePoly_SOA8(const IndexedTriangle tris[8], const std::vector<Vec3V>& verts)
{
	Triangle3V_SOA8 result;
#if defined(VMATH_USE_GATHER_SCATTER)
	const uint32 t = sizeof(IndexedTriangle)/sizeof(uint32);
	const __m256i one = _mm256_set1_epi32(1);
	const __m256i offsets0 = _mm256_setr_epi32(0, t, t*2, t*3, t*4, t*5, t*6, t*7);
	const __m256i offsets1 = _mm256_add_epi32(offsets0, one); // {1,4,7,10,13,16,19,22}
	const __m256i offsets2 = _mm256_add_epi32(offsets1, one); // {2,5,8,11,14,17,20,23}
	const float* src = reinterpret_cast<const float*>(&verts.front());
	__m256i addr0 = _mm256_slli_epi32(_mm256_i32gather_epi32(reinterpret_cast<const int*>(tris), offsets0, sizeof(uint32)), 2);
	__m256i addr1 = _mm256_slli_epi32(_mm256_i32gather_epi32(reinterpret_cast<const int*>(tris), offsets1, sizeof(uint32)), 2);
	__m256i addr2 = _mm256_slli_epi32(_mm256_i32gather_epi32(reinterpret_cast<const int*>(tris), offsets2, sizeof(uint32)), 2);
	__m256* dst = reinterpret_cast<__m256*>(&result.m_positions[0]);
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr0, 4)); addr0 = _mm256_add_epi32(addr0, one);
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr0, 4)); addr0 = _mm256_add_epi32(addr0, one);
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr0, 4)); 
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr1, 4)); addr1 = _mm256_add_epi32(addr1, one);
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr1, 4)); addr1 = _mm256_add_epi32(addr1, one);
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr1, 4)); 
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr2, 4)); addr2 = _mm256_add_epi32(addr2, one);
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr2, 4)); addr2 = _mm256_add_epi32(addr2, one);
	_mm256_store_ps(reinterpret_cast<float*>(dst++), _mm256_i32gather_ps(src, addr2, 4));
#else
	const Triangle3V t0 = MakePoly(tris[0], verts);
	const Triangle3V t1 = MakePoly(tris[1], verts);
	const Triangle3V t2 = MakePoly(tris[2], verts);
	const Triangle3V t3 = MakePoly(tris[3], verts);
	const Triangle3V t4 = MakePoly(tris[4], verts);
	const Triangle3V t5 = MakePoly(tris[5], verts);
	const Triangle3V t6 = MakePoly(tris[6], verts);
	const Triangle3V t7 = MakePoly(tris[7], verts);
	result.m_positions[0] = Vec3V_SOA8(t0.m_positions[0], t1.m_positions[0], t2.m_positions[0], t3.m_positions[0], t4.m_positions[0], t5.m_positions[0], t6.m_positions[0], t7.m_positions[0]);
	result.m_positions[1] = Vec3V_SOA8(t0.m_positions[1], t1.m_positions[1], t2.m_positions[1], t3.m_positions[1], t4.m_positions[1], t5.m_positions[1], t6.m_positions[1], t7.m_positions[1]);
	result.m_positions[2] = Vec3V_SOA8(t0.m_positions[2], t1.m_positions[2], t2.m_positions[2], t3.m_positions[2], t4.m_positions[2], t5.m_positions[2], t6.m_positions[2], t7.m_positions[2]);
#endif
	return result;
}
#endif // HAS_VEC8V

void MeshBase::CheckVertexStreams() const
{
	if (m_normals  ) MeshAssert(m_normals  ->size() == m_verts.size());
	if (m_texcoords) MeshAssert(m_texcoords->size() == m_verts.size());
	if (m_colors   ) MeshAssert(m_colors   ->size() == m_verts.size());
}

void MeshBase::CheckVertexIndex(uint32 index) const
{
	if (index >= m_verts.size())
		printf("MeshBase::CheckVertexIndex: index (%u) is not less than m_verts.size() (%u)!\n", index, (uint32)m_verts.size());
	MeshAssert(index < m_verts.size());
}

template <typename T> static void FillVertexElements(std::vector<T>*& elements, size_t count, typename T::ArgType value)
{
	size_t start = 0;
	if (elements == NULL)
		elements = new std::vector<T>(count);
	else {
		start = elements->size();
		if (elements->size() < count)
			elements->resize(count);
	}
	for (size_t i = start; i < count; i++)
		elements->operator[](i) = value;
}

void MeshBase::FillVertexNormals(Vec3V_arg normal) // fill remaining normals with constant value
{
	FillVertexElements(m_normals, m_verts.size(), normal);
}

void MeshBase::FillVertexTexcoords(Vec2V_arg texcoord) // fill remaining texcoords with constant value
{
	FillVertexElements(m_texcoords, m_verts.size(), texcoord);
}

void MeshBase::FillVertexColors(Vec4V_arg color) // fill remaining colors with constant value
{
	FillVertexElements(m_colors, m_verts.size(), color);
}

const Box3V MeshBase::GetBounds() const
{
	Box3V bounds = Box3V::Invalid();
	for (uint32 vertIndex = 0; vertIndex < m_verts.size(); vertIndex++)
		bounds.Grow(m_verts[vertIndex]);
	return bounds;
}

#define OBJ_GROUPS (1) // can disable this for debugging
#define OBJ_GROUP_CHAR 'o' // could use 'o' or 'g'

int MeshBase::FindGroup(const char* name) const
{
#if OBJ_GROUPS
	if (name) {
		const auto f = m_groupNameMap.find(name);
		if (f != m_groupNameMap.end())
			return f->second;
	}
#endif // OBJ_GROUPS
	return -1;
}

int MeshBase::FindOrAddGroup(const char* name)
{
	int group = FindGroup(name);
#if OBJ_GROUPS
	if (group == -1 && name) {
		group = (int)m_groupNames.size();
		const std::string nameStr(name);
		m_groupNames.push_back(nameStr);
		m_groupNameMap[nameStr] = group;
	}
#endif // OBJ_GROUPS
	return group;
}

void MeshBase::AddInfo(const char* format, ...)
{
	char str[1024] = "";
	va_list args;
	va_start(args, format);
	vsnprintf(str, sizeof(str), format, args);
	va_end(args);
	m_info.push_back(str);
}

const MeshBase::Vertex MeshBase::Vertex::Transform(const Mat34V* transform, const Vec4V* texScaleOffset) const
{
	Vertex v = *this;
	if (transform) {
		v.m_pos = transform->Transform(v.m_pos);
		v.m_normal = transform->TransformDir(v.m_normal);
	}
	if (texScaleOffset)
		v.m_texcoord = v.m_texcoord*texScaleOffset->xy() + texScaleOffset->zw();
	return v;
}

uint64 MeshBase::Vertex::GetCrcHash() const
{
	uint64 hash = Crc64(m_pos);
	hash = Crc64(m_normal  , hash);
	hash = Crc64(m_texcoord, hash);
	hash = Crc64(m_color   , hash);
	return hash;
}

const MeshBase::Vertex MeshBase::GetVertex(uint32 index) const
{
	MeshAssert(index < m_verts.size());
	Vertex v;
	v.m_pos      = m_verts[index];
	v.m_normal   = m_normals   ? m_normals  ->operator[](index) : Vec3V(V_ZERO);
	v.m_texcoord = m_texcoords ? m_texcoords->operator[](index) : Vec2V(V_ZERO);
	v.m_color    = m_colors    ? m_colors   ->operator[](index) : Vec4V(V_ZERO);
	return v;
}

const MeshBase::Vertex MeshBase::Interpolate(const Vertex& v0, const Vertex& v1, ScalarV_arg t, bool normalize)
{
	Vertex v;
	v.m_pos      = v0.m_pos      + (v1.m_pos      - v0.m_pos     )*t;
	v.m_normal   = v0.m_normal   + (v1.m_normal   - v0.m_normal  )*t;
	v.m_texcoord = v0.m_texcoord + (v1.m_texcoord - v0.m_texcoord)*t;
	v.m_color    = v0.m_color    + (v1.m_color    - v0.m_color   )*t;
	if (normalize)
		v.m_normal = NormalizeSafe(v.m_normal);
	return v;
}

uint32 MeshBase::AddVertex(const Vertex& v, bool useMap)
{
	if (useMap && m_vertexMap.find(v.GetCrcHash()) == m_vertexMap.end())
		return AddVertex(v, false);
	const uint32 index = (uint32)m_verts.size();
	m_verts.push_back(v.m_pos);
	if (m_normals  ) m_normals  ->push_back(v.m_normal  );
	if (m_texcoords) m_texcoords->push_back(v.m_texcoord);
	if (m_colors   ) m_colors   ->push_back(v.m_color   );
	return index;
}

uint32 MeshBase::AddInterpolatedVertex(uint32 index0, uint32 index1, ScalarV_arg t, bool normalize, bool useMap)
{
	if (useMap) {
		if      (t == 0.0f) return index0;
		else if (t == 1.0f) return index1;
	}
	const Vertex v0 = GetVertex(index0);
	const Vertex v1 = GetVertex(index1);
	return AddVertex(Interpolate(v0, v1, t, normalize), useMap);
}

template <typename T> static Vec3V_out GetPolyVertexPos_T(const Mesh<T>& mesh, uint32 polyIndex, unsigned i)
{
	MeshAssert(polyIndex < mesh.m_polys.size());
	MeshAssert(i < T::NumVerts);
	const uint32 vertIndex = mesh.m_polys[polyIndex].m_indices[i];
	MeshAssert(vertIndex < mesh.m_verts.size());
	return mesh.m_verts[vertIndex];
}

Vec3V_out TriangleMesh::GetTriangleVertexPos(uint32 triIndex, unsigned i) const
{
	return GetPolyVertexPos_T(*this, triIndex, i);
}

Vec3V_out QuadMesh::GetQuadVertexPos(uint32 quadIndex, unsigned i) const
{
	return GetPolyVertexPos_T(*this, quadIndex, i);
}

template <typename MeshType> static float CalculateMeshSurfaceArea_T(const MeshType& mesh, bool textureSpace)
{
	float area = 0.0f;
	if (mesh.m_texcoords || !textureSpace) {
		for (uint32 polyIndex = 0; polyIndex < mesh.m_polys.size(); polyIndex++) {
			if (textureSpace)
				area += MakePoly(mesh.m_polys[polyIndex], *mesh.m_texcoords).GetArea().f();
			else
				area += MakePoly(mesh.m_polys[polyIndex], mesh.m_verts).GetArea().f();
		}
	}
	return area;
}

float CalculateMeshSurfaceArea(const TriangleMesh& mesh, bool textureSpace)
{
	return CalculateMeshSurfaceArea_T(mesh, textureSpace);
}

float CalculateMeshSurfaceArea(const QuadMesh& mesh, bool textureSpace)
{
	return CalculateMeshSurfaceArea_T(mesh, textureSpace);
}

static Vec2V_out LoadOBJ_ReadVec2V(char* s)
{
	s = strtok(s, " \t");
	const float x = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float y = s ? (float)atof(s) : 0.0f; MeshAssert(s);
	return Vec2V(x,y);
}

static Vec3V_out LoadOBJ_ReadVec3V(char* s)
{
	s = strtok(s, " \t");
	const float x = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float y = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float z = s ? (float)atof(s) : 0.0f; MeshAssert(s);
	return Vec3V(x,y,z);
}

static Vec3V_out LoadOBJ_ReadVec3V_RGBA(char* s, Vec4V& color, bool& hasColor)
{
	s = strtok(s, " \t");
	const float x = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float y = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float z = s ? (float)atof(s) : 0.0f; MeshAssert(s);
	if (s) { // .obj format optionally stores RGBA color after position
		s = strtok(NULL, " \t");
		if (s) {
			hasColor = true;
			color.xf_ref() = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
			color.yf_ref() = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
			color.zf_ref() = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
			color.wf_ref() = s ? (float)atof(s) : 1.0f;
		}
	}
	return Vec3V(x,y,z);
}

static Vec4V_out LoadOBJ_ReadVec4V(char* s)
{
	s = strtok(s, " \t");
	const float x = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float y = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float z = s ? (float)atof(s) : 0.0f; s = strtok(NULL, " \t");
	const float w = s ? (float)atof(s) : 0.0f; MeshAssert(s);
	return Vec4V(x,y,z,w);
}

class LoadOBJ_FaceIndex
{
public:
	LoadOBJ_FaceIndex() {}
	LoadOBJ_FaceIndex(int posIndex, int texIndex, int nrmIndex)
		: m_posIndex(posIndex)
		, m_texIndex(texIndex)
		, m_nrmIndex(nrmIndex) {}

	bool operator <(const LoadOBJ_FaceIndex& rhs) const // for std::map
	{
		return memcmp(this, &rhs, sizeof(*this)) < 0;
	}

	int m_posIndex; // positions (and per-vertex RGBA colors)
	int m_texIndex; // texcoords
	int m_nrmIndex; // normals
};

static bool LoadOBJ_ReadFace(const char* str, std::vector<LoadOBJ_FaceIndex>& face, int vertStartLocal)
{
	char buffer[512] = "";
	char* toks = buffer;
	strcpy(buffer, str);
	while (true) {
		const char* temp = strtok(toks, " \t\n\r");
		if (temp == NULL)
			break;
		toks = NULL;
		const char* ss = strstr(temp, "//");
		const char* s1 = ss ? NULL : strstr(temp, "/");
		const char* s2 = s1 ? strstr(s1 + 2, "/") : NULL;
		int posIndex = atoi(temp);
		int texIndex = 0; // if not set, 0 will get decremented to -1 which means "none"
		int nrmIndex = 0;
		if (ss) // "vertex//normal"
			nrmIndex = atoi(ss + 2);
		else if (s1) { // "vertex/texcoord"
			texIndex = atoi(s1 + 1);
			if (s2)
				nrmIndex = atoi(s2 + 1); // "vertex/texcoord/normal"
		}
		posIndex = (posIndex >= 0) ? (posIndex - 1) : (vertStartLocal - posIndex - 1);
		texIndex = (texIndex >= 0) ? (texIndex - 1) : (vertStartLocal - texIndex - 1);
		nrmIndex = (nrmIndex >= 0) ? (nrmIndex - 1) : (vertStartLocal - nrmIndex - 1);
		face.push_back(LoadOBJ_FaceIndex(posIndex, texIndex, nrmIndex));
	}
	if (face.size() == 3) {
		if (face[0].m_posIndex == face[1].m_posIndex ||
			face[1].m_posIndex == face[2].m_posIndex ||
			face[2].m_posIndex == face[0].m_posIndex) return false;
	} else if (face.size() == 4) {
		if (face[0].m_posIndex == face[2].m_posIndex ||
			face[1].m_posIndex == face[3].m_posIndex) return false;
		if ((face[0].m_posIndex == face[1].m_posIndex && face[2].m_posIndex == face[3].m_posIndex) ||
			(face[1].m_posIndex == face[2].m_posIndex && face[3].m_posIndex == face[0].m_posIndex)) return false;
	} else {
		// TODO -- handle faces with > 4 vertices, determine if the entire polygon is degenerate
	}
	return true;
}

// supports position and optional per-vertex color (but not normals or texcoords)
// also supports "local vertices" with negative indices (which don't seem to be supported well in MeshLab for normals or texcoords)
template <typename MeshType> static bool LoadOBJ_T(const char* path, MeshType& mesh, const Mat34V* transform)
{
	typedef typename MeshType::IndexedPolyType IndexedPolyType;
	char path2[512];
	strcpy(path2, PathExt(path, ".obj"));
	FILE* file = fopen(path2, "r");
	if (file) {
		const uint32 vertStart = (uint32)mesh.m_verts.size();
		uint32 vertStartLocal = 0; // this codepath supports "local" vertices, i.e. indexed with negative values
		bool firstVertInLocal = true;
		int currentGroup = -1;
		char line[1024] = "";
		unsigned lineIndex = 0;
		const char* infoStartStr = "#INFO -> ";
		const size_t infoStartStrLen = strlen(infoStartStr);
		char groupStartStr[3];
		sprintf(groupStartStr, "%c ", OBJ_GROUP_CHAR);
		const size_t groupStartStrLen = strlen(groupStartStr);
		const char* vertStartStr = "v ";
		const size_t vertStartStrLen = strlen(vertStartStr);
		const char* faceStartStr = "f ";
		const size_t faceStartStrLen = strlen(faceStartStr);
		while (fgets(line, sizeof(line), file)) {
			++lineIndex;
			char* end = strpbrk(line, "\r\n");
			if (end)
				*end = '\0';
			if (strncmp(line, infoStartStr, infoStartStrLen) == 0)
				mesh.m_info.push_back(line + infoStartStrLen);
			else if (strncmp(line, groupStartStr, groupStartStrLen) == 0)
				currentGroup = mesh.FindOrAddGroup(line + groupStartStrLen);
			else if (strncmp(line, vertStartStr, vertStartStrLen) == 0) {
				if (firstVertInLocal) {
					firstVertInLocal = false;
					vertStartLocal = (uint32)mesh.m_verts.size() - vertStart;
				}
				Vec4V color = Vec4V(V_WAXIS);
				bool hasColor = false;
				Vec3V v = LoadOBJ_ReadVec3V_RGBA(line + vertStartStrLen, color, hasColor);
				if (transform)
					v = transform->Transform(v);
				const uint32 numVerts = (uint32)mesh.m_verts.size();
				mesh.m_verts.push_back(v);
				if (hasColor && mesh.m_colors == NULL)
					mesh.m_colors = new std::vector<Vec4V>;
				if (mesh.m_colors) {
					const uint32 numVertColors = (uint32)mesh.m_colors->size();
					if (numVertColors < numVerts) {
						mesh.m_colors->resize(numVerts);
						for (uint32 i = numVertColors; i < numVerts; i++)
							mesh.m_colors->operator[](i) = Vec4V(V_WAXIS);
					}
					mesh.m_colors->push_back(color);
				}
			} else if (strncmp(line, faceStartStr, faceStartStrLen) == 0) {
				std::vector<LoadOBJ_FaceIndex> face;
				if (LoadOBJ_ReadFace(line + faceStartStrLen, face, vertStartLocal)) { // only supports position, not texcoord or normals
					const uint32 numIndices = (uint32)face.size();
					std::vector<uint32> indices;
					indices.resize(numIndices);
					for (uint32 i = 0; i < numIndices; i++) {
						indices[i] = (uint32)face[i].m_posIndex;
						MeshAssert(indices[i] < mesh.m_verts.size());
					}
					IndexedPolyType::AddPolys(mesh.m_polys, vertStart, &indices.front(), numIndices, currentGroup);
					firstVertInLocal = true;
				}
			}
		}
		fclose(file);
		return true;
	} else {
		fprintf(stderr, "failed to load OBJ %s!\n", path2);
		return false;
	}
}

template <typename MeshType> static bool LoadOBJ_PosTexNrm_T(const char* path, MeshType& mesh, const Mat34V* transform)
{
	typedef typename MeshType::IndexedPolyType IndexedPolyType;
	// TODO -- consider consolidating this code with LoadOBJ_T
	FILE* file = fopen(PathExt(path, ".obj"), "r");
	if (file) {
		const uint32 vertStart = (uint32)mesh.m_verts.size();
		int currentGroup = -1;
		char line[1024] = "";
		unsigned lineIndex = 0;
		const char* infoStartStr = "#INFO -> ";
		const size_t infoStartStrLen = strlen(infoStartStr);
		char groupStartStr[3];
		sprintf(groupStartStr, "%c ", OBJ_GROUP_CHAR);
		const size_t groupStartStrLen = strlen(groupStartStr);
		const char* vertStartStr_pos = "v ";
		const char* vertStartStr_nrm = "vn ";
		const char* vertStartStr_tex = "vt ";
		const size_t vertStartStrLen_pos = strlen(vertStartStr_pos);
		const size_t vertStartStrLen_nrm = strlen(vertStartStr_nrm);
		const size_t vertStartStrLen_tex = strlen(vertStartStr_tex);
		const char* faceStartStr = "f ";
		const size_t faceStartStrLen = strlen(faceStartStr);
		std::vector<Vec3V> verts;
		std::vector<Vec4V> vertColors;
		std::vector<Vec3V> normals;
		std::vector<Vec2V> texcoords;
		std::map<LoadOBJ_FaceIndex,uint32> vertexMap;
		while (fgets(line, sizeof(line), file)) {
			++lineIndex;
			char* end = strpbrk(line, "\r\n");
			if (end)
				*end = '\0';
			if (strncmp(line, infoStartStr, infoStartStrLen) == 0)
				mesh.m_info.push_back(line + infoStartStrLen);
			else if (strncmp(line, groupStartStr, groupStartStrLen) == 0)
				currentGroup = mesh.FindOrAddGroup(line + groupStartStrLen);
			else if (strncmp(line, vertStartStr_pos, vertStartStrLen_pos) == 0) {
				Vec4V color = Vec4V(V_WAXIS);
				bool hasColor = false;
				Vec3V v = LoadOBJ_ReadVec3V_RGBA(line + vertStartStrLen_pos, color, hasColor);
				if (transform)
					v = transform->Transform(v);
				const uint32 numVerts = (uint32)verts.size();
				verts.push_back(v);
				if (hasColor) {
					if (vertColors.size() < numVerts) {
						const uint32 numColors = (uint32)vertColors.size();
						vertColors.resize(numVerts);
						for (uint32 i = numColors; i < numVerts; i++)
							vertColors[i] = Vec4V(V_WAXIS);
					}
					vertColors.push_back(color);
				}
			} else if (strncmp(line, vertStartStr_nrm, vertStartStrLen_nrm) == 0) {
				Vec3V norm = LoadOBJ_ReadVec3V(line + vertStartStrLen_nrm);
				if (transform)
					norm = transform->TransformDir(norm);
				normals.push_back(norm);
			} else if (strncmp(line, vertStartStr_tex, vertStartStrLen_tex) == 0) {
				Vec2V texcoord = LoadOBJ_ReadVec2V(line + vertStartStrLen_tex);
				texcoords.push_back(texcoord);
			} else if (strncmp(line, faceStartStr, faceStartStrLen) == 0) {
				std::vector<LoadOBJ_FaceIndex> face;
				if (LoadOBJ_ReadFace(line + faceStartStrLen, face, 0)) {
					const uint32 numIndices = (uint32)face.size();
					std::vector<uint32> indices;
					indices.resize(numIndices);
					for (uint32 i = 0; i < numIndices; i++) {
						const auto f = vertexMap.find(face[i]);
						if (f == vertexMap.end()) {
							const uint32 numVerts = (uint32)mesh.m_verts.size();
							indices[i] = vertexMap[face[i]] = numVerts;
							MeshAssert(face[i].m_posIndex < verts.size());
							mesh.m_verts.push_back(verts[face[i].m_posIndex]);
							if (!vertColors.empty()) {
								if (mesh.m_colors == NULL)
									mesh.m_colors = new std::vector<Vec4V>;
								if (mesh.m_colors->size() < numVerts) {
									const uint32 numColors = (uint32)mesh.m_colors->size();
									mesh.m_colors->resize(numVerts);
									for (uint32 i = numColors; i < numVerts; i++)
										mesh.m_colors->operator[](i) = Vec4V(V_WAXIS);
								}
								mesh.m_colors->push_back(vertColors[face[i].m_posIndex]);
							}
							if (face[i].m_nrmIndex >= 0) {
								if (mesh.m_normals == NULL)
									mesh.m_normals = new std::vector<Vec3V>;
								if (mesh.m_normals->size() < numVerts) {
									const uint32 numNormals = (uint32)mesh.m_normals->size();
									mesh.m_normals->resize(numVerts);
									memset(&mesh.m_normals->operator[](numNormals), 0, (numVerts - numNormals)*sizeof(Vec3V));
								}
								MeshAssert(face[i].m_nrmIndex < normals.size());
								mesh.m_normals->push_back(normals[face[i].m_nrmIndex]);
							}
							if (face[i].m_texIndex >= 0) {
								if (mesh.m_texcoords == NULL)
									mesh.m_texcoords = new std::vector<Vec2V>;
								if (mesh.m_texcoords->size() < numVerts) {
									const uint32 numTexcoords = (uint32)mesh.m_texcoords->size();
									mesh.m_texcoords->resize(numVerts);
									memset(&mesh.m_texcoords->operator[](numTexcoords), 0, (numVerts - numTexcoords)*sizeof(Vec2V));
								}
								MeshAssert(face[i].m_texIndex < texcoords.size());
								mesh.m_texcoords->push_back(texcoords[face[i].m_texIndex]);
							}
						} else
							indices[i] = f->second;
					}
					IndexedPolyType::AddPolys(mesh.m_polys, vertStart, &indices.front(), numIndices, currentGroup);
				}
			}
		}
		fclose(file);
		return true;
	} else {
		fprintf(stderr, "failed to load OBJ %s!\n", path);
		return false;
	}
}

bool LoadOBJ(const char* path, TriangleMesh& mesh, bool loadTexcoordsAndNormals, const Mat34V* transform)
{
	if (loadTexcoordsAndNormals)
		return LoadOBJ_PosTexNrm_T(path, mesh, transform);
	else
		return LoadOBJ_T(path, mesh, transform);
}

bool LoadOBJ(const char* path, QuadMesh& mesh, bool loadTexcoordsAndNormals, const Mat34V* transform)
{
	if (loadTexcoordsAndNormals)
		return LoadOBJ_PosTexNrm_T(path, mesh, transform);
	else
		return LoadOBJ_T(path, mesh, transform);
}

template <typename MeshType> static void SaveOBJStream_T(FILE* file, uint32& vrtIndex, uint32& nrmIndex, uint32& texIndex, const MeshType& mesh, const std::map<std::string_mappable,std::string>* materialMap)
{
	typedef typename MeshType::IndexedPolyType IndexedPolyType;
	mesh.CheckVertexStreams();
	const char* infoStartStr = "#INFO -> ";
	for (size_t i = 0; i < mesh.m_info.size(); i++)
		fprintf(file, "%s%s\n", infoStartStr, mesh.m_info[i].c_str());
	uint32 vrtCount = 0;
	uint32 nrmCount = 0;
	uint32 texCount = 0;
	for (size_t vertIndex = 0; vertIndex < mesh.m_verts.size(); vertIndex++) {
		if (mesh.m_colors)
			fprintf(file, "v %f %f %f %f %f %f %f\n", VEC3V_ARGS(mesh.m_verts[vertIndex]), VEC4V_ARGS(mesh.m_colors->operator[](vertIndex)));
		else
			fprintf(file, "v %f %f %f\n", VEC3V_ARGS(mesh.m_verts[vertIndex]));
		vrtCount++;
	}
	if (mesh.m_normals) {
		for (size_t vertIndex = 0; vertIndex < mesh.m_normals->size(); vertIndex++) {
			fprintf(file, "vn %f %f %f\n", VEC3V_ARGS(mesh.m_normals->operator[](vertIndex)));
			nrmCount++;
		}
	}
	if (mesh.m_texcoords) {
		for (size_t vertIndex = 0; vertIndex < mesh.m_texcoords->size(); vertIndex++) {
			fprintf(file, "vt %f %f\n", VEC2V_ARGS(mesh.m_texcoords->operator[](vertIndex)));
			texCount++;
		}
	}
	int currentGroup = -1;
	std::vector<IndexedPolyType> sortedPolys;
	sortedPolys.resize(mesh.m_polys.size());
	memcpy(&sortedPolys[0], &mesh.m_polys[0], mesh.m_polys.size()*sizeof(IndexedPolyType));
	std::sort(sortedPolys.begin(), sortedPolys.end());
	for (uint32 polyIndex = 0; polyIndex < sortedPolys.size(); polyIndex++) {
		const IndexedPolyType& poly = sortedPolys[polyIndex];
		if (currentGroup != poly.m_group && !mesh.m_groupNames.empty()) {
			currentGroup = poly.m_group;
			const char* groupName = "unknown";
			if (currentGroup >= 0 && currentGroup < mesh.m_groupNames.size())
				groupName = mesh.m_groupNames[currentGroup].c_str();
			fprintf(file, "%c %s\n", OBJ_GROUP_CHAR, groupName);
			if (materialMap) {
				const char* materialName = "unknown";
				const auto f = materialMap->find(groupName);
				if (f != materialMap->end())
					materialName = f->second.c_str();
				fprintf(file, "usemtl %s\n", materialName);
			}
		}
		uint32 firstIndex = 0;
		uint32 lastIndex = IndexedPolyType::NumVerts - 1;
		if (1) {
			while (firstIndex + 1 < IndexedPolyType::NumVerts && poly.m_indices[firstIndex + 1] == poly.m_indices[firstIndex])
				firstIndex++;
			while ((int)lastIndex - 1 >= 0 && poly.m_indices[lastIndex - 1] == poly.m_indices[lastIndex])
				lastIndex--;
			if (poly.m_indices[firstIndex] == poly.m_indices[lastIndex])
				firstIndex++;
		}
		if ((lastIndex - firstIndex + 1) >= 3) {
			fprintf(file, "f");
			for (uint32 i = firstIndex; i <= lastIndex; i++) {
				const uint32 index = poly.m_indices[i];
				mesh.CheckVertexIndex(index);
				const uint32 vi = vrtIndex + index;
				const uint32 ni = nrmIndex + index;
				const uint32 ti = texIndex + index;
				fprintf(file, " %u", 1 + vi);
				if (mesh.m_texcoords && mesh.m_normals)
					fprintf(file, "/%u/%u", 1 + ti, 1 + ni);
				else if (mesh.m_texcoords)
					fprintf(file, "/%u", 1 + ti);
				else if (mesh.m_normals)
					fprintf(file, "//%u", 1 + ni);
			}
			fprintf(file, "\n");
		}
	}
	vrtIndex += vrtCount;
	nrmIndex += nrmCount;
	texIndex += texCount;
}

template <typename MeshType> static void SaveOBJStream_SaveUniqueElements_T(FILE* file, uint32& vrtIndex, uint32& nrmIndex, uint32& texIndex, const MeshType& mesh, const std::map<std::string_mappable,std::string>* materialMap)
{
	// TODO -- consider consolidating this code with SaveOBJ_T
	typedef typename MeshType::IndexedPolyType IndexedPolyType;
	mesh.CheckVertexStreams();
	const char* infoStartStr = "#INFO -> ";
	for (size_t i = 0; i < mesh.m_info.size(); i++)
		fprintf(file, "%s%s\n", infoStartStr, mesh.m_info[i].c_str());
	std::vector<uint32> vrtTable;
	std::vector<uint32> nrmTable;
	std::vector<uint32> texTable;
	uint32 vrtCount = 0;
	uint32 nrmCount = 0;
	uint32 texCount = 0;
	{ // verts
		std::map<Vec3V_mappable,uint32> vrtMap;
		vrtTable.resize(mesh.m_verts.size());
		for (uint32 vertIndex = 0; vertIndex < mesh.m_verts.size(); vertIndex++) {
			const Vec3V v = mesh.m_verts[vertIndex];
			const auto f = vrtMap.find(v);
			if (f == vrtMap.end()) {
				if (mesh.m_colors)
					fprintf(file, "v %f %f %f %f %f %f %f\n", VEC3V_ARGS(v), VEC4V_ARGS(mesh.m_colors->operator[](vertIndex)));
				else
					fprintf(file, "v %f %f %f\n", VEC3V_ARGS(v));
				vrtMap[v] = vrtTable[vertIndex] = vrtCount++;
			} else
				vrtTable[vertIndex] = f->second;
		}
	}
	if (mesh.m_normals) {
		std::map<Vec3V_mappable,uint32> nrmMap;
		nrmTable.resize(mesh.m_normals->size());
		for (uint32 normalIndex = 0; normalIndex < mesh.m_normals->size(); normalIndex++) {
			const Vec3V v = mesh.m_normals->operator[](normalIndex);
			const auto f = nrmMap.find(v);
			if (f == nrmMap.end()) {
				fprintf(file, "vn %f %f %f\n", VEC3V_ARGS(v));
				nrmMap[v] = nrmTable[normalIndex] = nrmCount++;
			} else
				nrmTable[normalIndex] = f->second;
		}
	}
	if (mesh.m_texcoords) {
		std::map<Vec2V_mappable,uint32> texMap;
		texTable.resize(mesh.m_texcoords->size());
		for (uint32 texcoordIndex = 0; texcoordIndex < mesh.m_texcoords->size(); texcoordIndex++) {
			const Vec2V v = mesh.m_texcoords->operator[](texcoordIndex);
			const auto f = texMap.find(v);
			if (f == texMap.end()) {
				fprintf(file, "vt %f %f\n", VEC2V_ARGS(v));
				texMap[v] = texTable[texcoordIndex] = texCount++;
			} else
				texTable[texcoordIndex] = f->second;
		}
	}
	int currentGroup = -1;
	std::vector<IndexedPolyType> sortedPolys;
	sortedPolys.resize(mesh.m_polys.size());
	memcpy(&sortedPolys[0], &mesh.m_polys[0], mesh.m_polys.size()*sizeof(IndexedPolyType));
	std::sort(sortedPolys.begin(), sortedPolys.end());
	for (uint32 polyIndex = 0; polyIndex < sortedPolys.size(); polyIndex++) {
		const IndexedPolyType& poly = sortedPolys[polyIndex];
		if (currentGroup != poly.m_group && !mesh.m_groupNames.empty()) {
			currentGroup = poly.m_group;
			const char* groupName = "unknown";
			if (currentGroup >= 0 && currentGroup < mesh.m_groupNames.size())
				groupName = mesh.m_groupNames[currentGroup].c_str();
			fprintf(file, "%c %s\n", OBJ_GROUP_CHAR, groupName);
			if (materialMap) {
				const char* materialName = "unknown";
				const auto f = materialMap->find(groupName);
				if (f != materialMap->end())
					materialName = f->second.c_str();
				fprintf(file, "usemtl %s\n", materialName);
			}
		}
		uint32 firstIndex = 0;
		uint32 lastIndex = IndexedPolyType::NumVerts - 1;
		if (1) { // try to strip off duplicate verts
			while (firstIndex + 1 < IndexedPolyType::NumVerts && poly.m_indices[firstIndex + 1] == poly.m_indices[firstIndex])
				firstIndex++;
			while ((int)lastIndex - 1 >= 0 && poly.m_indices[lastIndex - 1] == poly.m_indices[lastIndex])
				lastIndex--;
			if (poly.m_indices[firstIndex] == poly.m_indices[lastIndex])
				firstIndex++;
		}
		if ((lastIndex - firstIndex + 1) >= 3) {
			fprintf(file, "f");
			for (uint32 i = firstIndex; i <= lastIndex; i++) {
				const uint32 index = poly.m_indices[i];
				mesh.CheckVertexIndex(index);
				const uint32 vi = vrtIndex + vrtTable[index];
				const uint32 ni = nrmIndex + (mesh.m_normals ? nrmTable[index] : 0);
				const uint32 ti = texIndex + (mesh.m_texcoords ? texTable[index] : 0);
				fprintf(file, " %u", 1 + vi);
				if (mesh.m_texcoords && mesh.m_normals)
					fprintf(file, "/%u/%u", 1 + ti, 1 + ni);
				else if (mesh.m_texcoords)
					fprintf(file, "/%u", 1 + ti);
				else if (mesh.m_normals)
					fprintf(file, "//%u", 1 + ni);
			}
			fprintf(file, "\n");
		}
	}
	vrtIndex += vrtCount;
	nrmIndex += nrmCount;
	texIndex += texCount;
}

void SaveOBJStream(FILE* file, const TriangleMesh& mesh, uint32& vrtIndex, uint32& nrmIndex, uint32& texIndex, const std::map<std::string_mappable,std::string>* materialMap, bool saveUniqueElements)
{
	if (saveUniqueElements)
		return SaveOBJStream_SaveUniqueElements_T(file, vrtIndex, nrmIndex, texIndex, mesh, materialMap);
	else
		return SaveOBJStream_T(file, vrtIndex, nrmIndex, texIndex, mesh, materialMap);
}

void SaveOBJStream(FILE* file, const QuadMesh& mesh, uint32& vrtIndex, uint32& nrmIndex, uint32& texIndex, const std::map<std::string_mappable,std::string>* materialMap, bool saveUniqueElements)
{
	if (saveUniqueElements)
		return SaveOBJStream_SaveUniqueElements_T(file, vrtIndex, nrmIndex, texIndex, mesh, materialMap);
	else
		return SaveOBJStream_T(file, vrtIndex, nrmIndex, texIndex, mesh, materialMap);
}

bool SaveOBJ(const char* path, const TriangleMesh& mesh, const char* materialLib, const std::map<std::string_mappable,std::string>* materialMap, bool saveUniqueElements)
{
	FILE* file = fopen(path, "w");
	if (file) {
		if (materialLib)
			fprintf(file, "mtllib %s\n", materialLib);
		uint32 vrtIndex = 0;
		uint32 nrmIndex = 0;
		uint32 texIndex = 0;
		SaveOBJStream(file, mesh, vrtIndex, nrmIndex, texIndex, materialMap, saveUniqueElements);
		fclose(file);
		return true;
	} else {
		fprintf(stderr, "failed to save %s!\n", path);
		return false;
	}
}

bool SaveOBJ(const char* path, const QuadMesh& mesh, const char* materialLib, const std::map<std::string_mappable,std::string>* materialMap, bool saveUniqueElements)
{
	FILE* file = fopen(path, "w");
	if (file) {
		if (materialLib)
			fprintf(file, "mtllib %s\n", materialLib);
		uint32 vrtIndex = 0;
		uint32 nrmIndex = 0;
		uint32 texIndex = 0;
		SaveOBJStream(file, mesh, vrtIndex, nrmIndex, texIndex, materialMap, saveUniqueElements);
		fclose(file);
		return true;
	} else {
		fprintf(stderr, "failed to save %s!\n", path);
		return false;
	}
}

Mat33V_out GetBoxFaceAxes(uint32 faceIndex)
{
	// https://docs.microsoft.com/en-us/windows/desktop/direct3d9/cubic-environment-mapping
	//   ---> +u
	// |            +--------+
	// |            |-z      |
	// v            | face 2 |
	// +v           |   +y +x|
	//     +--------+--------+--------+--------+
	//     |+y      |+y      |+y      |+y      |
	//     | face 1 | face 4 | face 0 | face 5 |
	//     |   -x +z|   +z +x|   +x -z|   -z -x|
	//     +--------+--------+--------+--------+
	//              |+z      |
	//              | face 3 |
	//              |   -y +x|
	//              +--------+
	switch (faceIndex) {
	case 0: return Mat33V(-Vec3V(V_ZAXIS), -Vec3V(V_YAXIS), +Vec3V(V_XAXIS)); // +x
	case 1: return Mat33V(+Vec3V(V_ZAXIS), -Vec3V(V_YAXIS), -Vec3V(V_XAXIS)); // -x
	case 2: return Mat33V(+Vec3V(V_XAXIS), +Vec3V(V_ZAXIS), +Vec3V(V_YAXIS)); // +y
	case 3: return Mat33V(+Vec3V(V_XAXIS), -Vec3V(V_ZAXIS), -Vec3V(V_YAXIS)); // -y
	case 4: return Mat33V(+Vec3V(V_XAXIS), -Vec3V(V_YAXIS), +Vec3V(V_ZAXIS)); // +z
	case 5: return Mat33V(-Vec3V(V_XAXIS), -Vec3V(V_YAXIS), -Vec3V(V_ZAXIS)); // -z
	default: ForceAssert(false);
	}
	return Mat33V(V_ZERO);
}

bool AddUVMappingToOBJ(const char* objPath, const char* objWithUVsPath)
{
	class OBJStrings
	{
	public:
		OBJStrings(const char* path)
		{
			FILE* file = fopen(path, "r");
			if (file) {
				char line[2048];
				while (rage_fgetline(line, sizeof(line), file)) {
					if      (memcmp(line, "v ",  2) == 0) m_positions.push_back(line);
					else if (memcmp(line, "vt ", 3) == 0) m_texcoords.push_back(line);
					else if (memcmp(line, "vn ", 3) == 0) m_normals  .push_back(line);
					else if (memcmp(line, "f ",  2) == 0) m_faces    .push_back(line);
				}
				fclose(file);
			}
		}

		bool Save(const char* path)
		{
			FILE* file = fopen(path, "w");
			if (file) {
				for (uint32 i = 0; i < m_positions.size(); i++) fprintf(file, "%s\n", m_positions[i].c_str());
				for (uint32 i = 0; i < m_texcoords.size(); i++) fprintf(file, "%s\n", m_texcoords[i].c_str());
				for (uint32 i = 0; i < m_normals  .size(); i++) fprintf(file, "%s\n", m_normals  [i].c_str());
				for (uint32 i = 0; i < m_faces    .size(); i++) fprintf(file, "%s\n", m_faces    [i].c_str());
				fclose(file);
				return true;
			}
			return false;
		}

		std::vector<std::string> m_positions;
		std::vector<std::string> m_texcoords;
		std::vector<std::string> m_normals;
		std::vector<std::string> m_faces;
	};
	OBJStrings obj(objPath);
	OBJStrings objWithUVs(objWithUVsPath);
	if (obj.m_positions.size() > 0 &&
		obj.m_positions.size() == objWithUVs.m_positions.size() &&
		obj.m_faces.size() > 0 &&
		obj.m_faces.size() == objWithUVs.m_faces.size() &&
		!objWithUVs.m_texcoords.empty()) {
		for (uint32 i = 0; i < obj.m_faces.size(); i++) {
			std::vector<LoadOBJ_FaceIndex> face1;
			std::vector<LoadOBJ_FaceIndex> face2;
			if (!LoadOBJ_ReadFace(obj.m_faces[i].c_str() + strlen("f "), face1, 0))
				return false;
			if (!LoadOBJ_ReadFace(objWithUVs.m_faces[i].c_str() + strlen("f "), face2, 0))
				return false;
			if (face1.size() != face2.size())
				return false;
			std::string faceStr = "f";
			for (uint32 k = 0; k < face1.size(); k++) {
				if (face1[k].m_posIndex != face2[k].m_posIndex)
					return false;
				faceStr += varString(" %d/%d", face1[k].m_posIndex + 1, face2[k].m_texIndex + 1);
				if (!obj.m_normals.empty())
					faceStr += varString("/%d", face1[k].m_nrmIndex + 1);
			}
			obj.m_faces[i] = faceStr; // replace with new face which includes texcoords
		}
		obj.m_texcoords = objWithUVs.m_texcoords;
		return obj.Save(PathExt(objPath, "_UVs.obj"));
	}
	return false;
}

template <typename MeshType> static void ConstructBox_T(MeshType& mesh, const Box3V& box, uint32 boxFaceFlags = BOX_FACE_FLAGS_ALL, const Mat34V* transform = nullptr, float uvDensity = 0.0f)
{
	mesh.CheckVertexStreams();
	if (transform == NULL)
		transform = &Mat34V::StaticIdentity();
	const uint32 vertStart = (uint32)mesh.m_verts.size();
	for (unsigned vertIndex = 0; vertIndex < 8; vertIndex++) {
		const Vec3V dir(
			(vertIndex&1) ? 1.0f : -1.0f,
			(vertIndex&2) ? 1.0f : -1.0f,
			(vertIndex&4) ? 1.0f : -1.0f);
		mesh.m_verts.push_back(transform->Transform(box.GetCenter() + box.GetExtent()*dir));
		if (mesh.m_normals)
			mesh.m_normals->push_back(transform->TransformDir(dir));
	}
	if (boxFaceFlags & BOX_FACE_FLAG_POS_X) IndexedQuad(vertStart, 1,3,7,5, -1).Add(mesh.m_polys); // +X
	if (boxFaceFlags & BOX_FACE_FLAG_POS_Y) IndexedQuad(vertStart, 2,6,7,3, -1).Add(mesh.m_polys); // +Y
	if (boxFaceFlags & BOX_FACE_FLAG_POS_Z) IndexedQuad(vertStart, 5,7,6,4, -1).Add(mesh.m_polys); // +Z
	if (boxFaceFlags & BOX_FACE_FLAG_NEG_X) IndexedQuad(vertStart, 4,6,2,0, -1).Add(mesh.m_polys); // -X
	if (boxFaceFlags & BOX_FACE_FLAG_NEG_Y) IndexedQuad(vertStart, 1,5,4,0, -1).Add(mesh.m_polys); // -Y
	if (boxFaceFlags & BOX_FACE_FLAG_NEG_Z) IndexedQuad(vertStart, 0,2,3,1, -1).Add(mesh.m_polys); // -Z
	if (mesh.m_texcoords)
		mesh.FillVertexTexcoords(Vec2V(V_ZERO));
	if (mesh.m_colors)
		mesh.FillVertexColors(Vec4V(V_ZERO));
}

void ConstructBox(TriangleMesh& mesh, const Box3V& box, uint32 boxFaceFlags, const Mat34V* transform, float uvDensity)
{
	ConstructBox_T(mesh, box, boxFaceFlags, transform, uvDensity);
}

void ConstructBox(QuadMesh& mesh, const Box3V& box, uint32 boxFaceFlags, const Mat34V* transform, float uvDensity)
{
	ConstructBox_T(mesh, box, boxFaceFlags, transform, uvDensity);
}

template <typename MeshType> static void ConstructBoxFrame_T(MeshType& mesh, const Box3V& box, float thickness, const Mat34V* transform, bool outerOnly)
{
	mesh.CheckVertexStreams();
	ForceAssert(mesh.m_normals   == NULL); // TODO -- support normals? do i have to?
	ForceAssert(mesh.m_texcoords == NULL);
	ForceAssert(mesh.m_colors    == NULL);
	if (transform == NULL)
		transform = &Mat34V::StaticIdentity();
	const uint32 vertStart = (uint32)mesh.m_verts.size();
	for (unsigned i = 0; i < 8; i++) {
		const Vec3V dir = Vec3V(
			(i&1) ? +1.0f : -1.0f,
			(i&2) ? +1.0f : -1.0f,
			(i&4) ? +1.0f : -1.0f);
		const Vec3V v = transform->Transform(box.GetCenter() + box.GetExtent()*dir);
		mesh.m_verts.push_back(v);
		mesh.m_verts.push_back(v - transform->TransformDir(Vec3V(1.0f,1.0f,0.0f)*dir*thickness)); // xy0
		mesh.m_verts.push_back(v - transform->TransformDir(Vec3V(0.0f,1.0f,1.0f)*dir*thickness)); // 0yz
		mesh.m_verts.push_back(v - transform->TransformDir(Vec3V(1.0f,0.0f,1.0f)*dir*thickness)); // x0z
	}
	const uint32 indices[][4] = {
		{0,4,5,1},{4,12,13,5},{12,8,9,13},{8,0,1,9},
		{4,20,22,6},{20,28,30,22},{28,12,14,30},{12,4,6,14},
		{20,16,17,21},{16,24,25,17},{24,28,29,25},{28,20,21,29},
		{16,0,2,18},{0,8,10,2},{8,24,26,10},{24,16,18,26},
		{8,12,15,11},{12,28,31,15},{28,24,27,31},{24,8,11,27},
		{4,0,3,7},{0,16,19,3},{16,20,23,19},{20,4,7,23},
	};
	for (unsigned i = 0; i < countof(indices); i++)
		IndexedQuad(vertStart, indices[i][3], indices[i][2], indices[i][1], indices[i][0], -1).Add(mesh.m_polys);
	if (!outerOnly) {
		for (unsigned i = 0; i < 8; i++) {
			const Vec3V dir = Vec3V(
				(i&1) ? +1.0f : -1.0f,
				(i&2) ? +1.0f : -1.0f,
				(i&4) ? +1.0f : -1.0f);
			mesh.m_verts.push_back(mesh.m_verts[vertStart + i*4] - transform->TransformDir(dir*thickness)); // xyz
		}
		const uint32 innerIndices[][4] = {
			{32,1,5,33},{33,6,22,37},{37,21,17,36},{36,18,2,32},
			{32,3,19,36},{36,17,25,38},{38,27,11,34},{34,9,1,32},
			{33,7,3,32},{32,2,10,34},{34,11,15,35},{35,14,6,33},
			{35,13,9,34},{39,30,14,35},{38,25,29,39},{34,10,26,38},
			{33,5,13,35},{35,15,31,39},{39,29,21,37},{37,23,7,33},
			{36,19,23,37},{37,22,30,39},{39,31,27,38},{38,26,18,36},
		};
		for (unsigned i = 0; i < countof(innerIndices); i++)
			IndexedQuad(vertStart, innerIndices[i][3], innerIndices[i][2], innerIndices[i][1], innerIndices[i][0], -1).Add(mesh.m_polys);
	}
}

void ConstructBoxFrame(TriangleMesh& mesh, const Box3V& box, float thickness, const Mat34V* transform, bool outerOnly)
{
	ConstructBoxFrame_T(mesh, box, thickness, transform, outerOnly);
}

void ConstructBoxFrame(QuadMesh& mesh, const Box3V& box, float thickness, const Mat34V* transform, bool outerOnly)
{
	ConstructBoxFrame_T(mesh, box, thickness, transform, outerOnly);
}

template <typename MeshType> static void ConstructBoxLine_T(MeshType& mesh, Vec3V_arg origin, Vec3V_arg dir, float length, float radius)
{
	const Mat34V basis = Mat34V::ConstructBasis(origin, dir);
	ConstructBox_T(mesh, Box3V(Vec3V(-radius,-radius,0.0f),Vec3V(radius,radius,length)), BOX_FACE_FLAGS_ALL, &basis);
}

void ConstructBoxLine(TriangleMesh& mesh, Vec3V_arg origin, Vec3V_arg dir, float length, float radius)
{
	ConstructBoxLine_T(mesh, origin, dir, length, radius);
}

void ConstructBoxLine(QuadMesh& mesh, Vec3V_arg origin, Vec3V_arg dir, float length, float radius)
{
	ConstructBoxLine_T(mesh, origin, dir, length, radius);
}

template <typename MeshType> static void ConstructSphere_T(MeshType& mesh, const Sphere3V& sphere, const Mat34V* transform, unsigned numSlices, unsigned numStacks)
{
	mesh.CheckVertexStreams();
	MeshAssert(numSlices >= 3 && numStacks >= 2);
	if (transform == NULL)
		transform = &Mat34V::StaticIdentity();
	const uint32 vertStart = (uint32)mesh.m_verts.size();
	mesh.m_verts.push_back(transform->Transform(sphere.GetCenter() - sphere.GetRadius()*Vec3V(V_ZAXIS))); // vertIndex = 0
	if (mesh.m_normals)
		mesh.m_normals->push_back(transform->TransformDir(-Vec3V(V_ZAXIS)));
	for (unsigned stack = 1; stack < numStacks; stack++) {
		float phi = PI*((float)stack/(float)numStacks - 0.5f); // (-PI/2..PI/2)
		//phi = 0.5f*PI*sinf(phi); // hack to make quads kinda uniform aspect, i.e. not become super thin near the poles. this math isn't really correct though ..
		//phi = 0.5f*PI*sinf(phi);
		const float cosPhi = cosf(phi);
		const float sinPhi = sinf(phi);
		for (unsigned slice = 0; slice < numSlices; slice++) {
			const float theta = 2.0f*PI*(float)slice/(float)numSlices; // [0..2PI)
			const float cosTheta = cosf(theta);
			const float sinTheta = sinf(theta);
			const Vec3V dir(cosTheta*cosPhi, sinTheta*cosPhi, sinPhi);
			mesh.m_verts.push_back(transform->Transform(sphere.GetCenter() + sphere.GetRadius()*dir));
			if (mesh.m_normals)
				mesh.m_normals->push_back(transform->TransformDir(dir));
		}
	}
	mesh.m_verts.push_back(transform->Transform(sphere.GetCenter() + sphere.GetRadius()*Vec3V(V_ZAXIS))); // vertIndex = 1 + (numStacks - 1)*numSlices
	if (mesh.m_normals)
		mesh.m_normals->push_back(transform->TransformDir(Vec3V(V_ZAXIS)));
	for (unsigned slice = 0; slice < numSlices; slice++)
		IndexedTriangle(vertStart,
			0,
			1 + (slice + 1)%numSlices,
			1 + slice,
		-1).Add(mesh.m_polys);
	for (unsigned stack = 1; stack < numStacks - 1; stack++) {
		for (unsigned slice = 0; slice < numSlices; slice++) {
			IndexedQuad(vertStart,
				1 + (stack - 1)*numSlices + slice,
				1 + (stack - 1)*numSlices + (slice + 1)%numSlices,
				1 + (stack - 0)*numSlices + (slice + 1)%numSlices,
				1 + (stack - 0)*numSlices + slice,
			-1).Add(mesh.m_polys);
		}
	}
	for (unsigned slice = 0; slice < numSlices; slice++)
		IndexedTriangle(vertStart,
			1 + (numStacks - 1)*numSlices,
			1 + (numStacks - 1)*numSlices - numSlices + slice,
			1 + (numStacks - 1)*numSlices - numSlices + (slice + 1)%numSlices,
		-1).Add(mesh.m_polys);
	if (mesh.m_texcoords)
		mesh.FillVertexTexcoords(Vec2V(V_ZERO));
	if (mesh.m_colors)
		mesh.FillVertexColors(Vec4V(V_ZERO));
}

void ConstructSphere(TriangleMesh& mesh, const Sphere3V& sphere, const Mat34V* transform, unsigned numSlices, unsigned numStacks)
{
	ConstructSphere_T(mesh, sphere, transform, numSlices, numStacks);
}

void ConstructSphere(QuadMesh& mesh, const Sphere3V& sphere, const Mat34V* transform, unsigned numSlices, unsigned numStacks)
{
	ConstructSphere_T(mesh, sphere, transform, numSlices, numStacks);
}

static float RoundBoxFunction(uint32 i, uint32 w, uint32 roundRes, uint32 interiorRes, float roundRadiusRelative, bool uniformAngularSpacing)
{
	float x = 0.0f;
	if (roundRes == 0)
		x = (float)i/(float)w;
	else if (i <= roundRes) {
		x = (float)i/(float)roundRes; // [0..1]
		if (uniformAngularSpacing)
			x = 1.0f - tanf(0.25f*PI*(1.0f - x));
		x *= roundRadiusRelative;
	} else if (i - roundRes <= interiorRes)
		x = roundRadiusRelative + (1.0f - 2.0f*roundRadiusRelative)*(float)(i - roundRes)/(float)interiorRes;
	else {
		x = (float)(roundRes*2 + interiorRes - i)/(float)roundRes; // [0..1]
		if (uniformAngularSpacing)
			x = 1.0f - tanf(0.25f*PI*(1.0f - x));
		x = 1.0f - x*roundRadiusRelative;
	}
	return x;
}

static void BoxUVSmartPack(std::vector<Box2V>& uvSmartPackQuads, uint32 orderedExtentDims[3], Vec3V_arg extent, float uvDensity, unsigned uvPadding, bool uvSmartPack3x2 = true)
{
	// =========================================================
	// A = min extent, B = mid extent, C = max extent
	// width = 2B + 2A
	// height = C + A
	// +------++------++---++---+
	// |BxC   ||BxC   ||AxC||AxC|
	// |      ||      ||   ||   |
	// |     0||     1||  2||  3|
	// +------++------++---++---+
	// +------++------+
	// |BxA  4||BxA  5|
	// +------++------+
	// 
	// if C - A is small, we can do slightly better:
	// instead of (2B + 2A) x (C + A)
	// we have (2B + A) x (2C)
	// in terms of area, this is better if A*(A + B) > B*C
	// +------++------++-----+
	// |BxC   ||BxC   ||AxC  |
	// |      ||      ||     |
	// |     0||     1||    2|
	// +------++------++-----+
	// +------++------++-----+
	// |BxA   ||BxA   ||AxC  |
	// |     4||     5||     |
	// +------++------+|    3|
	//                 +-----+
	//
	// TODO -- we could share edges if we rearranged slightly ..
	// +------+---+------+---+
	// |BxC   |AxC|BxC   |AxC|
	// |      |   |      |   |
	// |     0|  1|     2|  3|
	// +------+---+------+---+
	// |BxA  4|+------+
	// +------+|BxA  5|
	//         +------+
	// =========================================================

	orderedExtentDims[0] = MinElementIndex(extent);
	orderedExtentDims[2] = MaxElementIndex(extent);
	if (orderedExtentDims[0] == orderedExtentDims[2]) {
		orderedExtentDims[0] = 0;
		orderedExtentDims[1] = 1;
		orderedExtentDims[2] = 2;
	} else {
		Vec3V v(V_ZERO);
		v[orderedExtentDims[0]] = 1.0f;
		v[orderedExtentDims[2]] = 1.0f;
		orderedExtentDims[1] = MinElementIndex(v);
	}
	const float A = Ceiling(extent[orderedExtentDims[0]]*2.0f*uvDensity);
	const float B = Ceiling(extent[orderedExtentDims[1]]*2.0f*uvDensity);
	const float C = Ceiling(extent[orderedExtentDims[2]]*2.0f*uvDensity);
	const bool pack3x2 = uvSmartPack3x2 && (A*(A + B) > B*C);
	float x0 = 0.5f + (float)uvPadding;
	float y0 = 0.5f + (float)uvPadding;
	float x1;
	float y1 = y0 + C;
	x1 = x0 + B;
	uvSmartPackQuads.push_back(Box2V(x0, y0, x1, y1)); // BxC
	x0 = x1 + (float)uvPadding + 1.0f;
	x1 = x0 + B;
	uvSmartPackQuads.push_back(Box2V(x0, y0, x1, y1)); // BxC
	x0 = x1 + (float)uvPadding + 1.0f;
	x1 = x0 + A;
	uvSmartPackQuads.push_back(Box2V(x0, y0, x1, y1)); // AxC
	if (pack3x2) {
		y0 = y1 + (float)uvPadding + 1.0f;
		y1 = y0 + C;
		uvSmartPackQuads.push_back(Box2V(x0, y0, x1, y1)); // AxC (bottom left)
	} else {
		x0 = x1 + (float)uvPadding + 1.0f;
		x1 = x0 + A;
		uvSmartPackQuads.push_back(Box2V(x0, y0, x1, y1)); // AxC
	}
	x0 = 0.5f + (float)uvPadding;
	y0 = 0.5f + (float)uvPadding + C + (float)uvPadding + 1.0f;
	y1 = y0 + A;
	x1 = x0 + B;
	uvSmartPackQuads.push_back(Box2V(x0, y0, x1, y1)); // BxA
	x0 = x1 + (float)uvPadding + 1.0f;
	x1 = x0 + B;
	uvSmartPackQuads.push_back(Box2V(x0, y0, x1, y1)); // BxA
}

template <typename MeshType> static void ConstructRoundBox_T(MeshType& mesh, const Box3V& box, float roundRadius, float vertDensity, float uvDensity, unsigned uvPadding, uint32 faceIgnoreMask, bool optimized, bool uniformAngularSpacing)
{
	const bool uvSmartPack = true; // use layout which minimizes uv area 
	const bool uvSmartPack3x2 = true; // minor optimization
	// to transform a box mesh into a round box, we can apply this simple transformation:
	//    p = vertex of tessellated box mesh (not rounded)
	//    r = rounding radius
	//    q = clamp(p, box.min + r, box.max - r)
	//    p' = q + normalize(p - q)*r = corresponding vertex of round box mesh
	const bool uniqueVerts = true;
	mesh.CheckVertexStreams();
	const uint32 vertStart = (uint32)mesh.m_verts.size();
	const float roundResScale = 0.25f*PI; // not sure if this is actually correct ..
	const uint32 roundRes = (uint32)Ceiling(roundRadius*vertDensity*roundResScale); 
	const Vec3V interiorResV = Max(ScalarV(V_ONE), Ceiling((box.GetSize() - ScalarV(roundRadius*2.0f))*vertDensity*(optimized ? 0.0f : 1.0f)));
	const Vec3V resV = ScalarV((float)(roundRes*2)) + interiorResV;
	const Vec3V extent = box.GetExtent();
	std::map<Vec3V_mappable,uint32> pm;
	std::vector<Box2V> uvSmartPackQuads;
	uint32 orderedExtentDims[3] = {0,0,0}; // min, mid, max
	if (vertDensity <= 0.0f) {
		vertDensity = 0.0f;
		roundRadius = 0.0f;
	}
	if (mesh.m_texcoords == nullptr)
		uvDensity = 0.0f;
	if (uvDensity > 0.0f && uvSmartPack)
		BoxUVSmartPack(uvSmartPackQuads, orderedExtentDims, extent, uvDensity, uvPadding, uvSmartPack3x2);
	float faceUVx1 = 0.0f;
	for (uint32 faceIndex = 0; faceIndex < 6; faceIndex++) {
		if (faceIgnoreMask & BIT(faceIndex))
			continue;
		const Mat33V axes = GetBoxFaceAxes(faceIndex);
		const Vec3V corner = box.GetCenter() + extent*(axes.c() - axes.a() - axes.b());
		const Vec3V a = axes.a()*extent*2.0f;
		const Vec3V b = axes.b()*extent*2.0f;
		const uint32 interiorResX = (uint32)interiorResV[MaxElementIndex(Abs(axes.a()))];
		const uint32 interiorResY = (uint32)interiorResV[MaxElementIndex(Abs(axes.b()))];
		const uint32 w = roundRes*2 + interiorResX;
		const uint32 h = roundRes*2 + interiorResY;
		const float roundRadiusRelativeX = roundRadius/Mag(a).f();
		const float roundRadiusRelativeY = roundRadius/Mag(b).f();
		Box2V faceUVs;
		bool swapUV = false;
		if (uvDensity > 0.0f) {
			if (uvSmartPack) {
				const uint32 ai = MaxElementIndex(Abs(axes.a()));
				const uint32 bi = MaxElementIndex(Abs(axes.b()));
				if      (ai == orderedExtentDims[1] && bi == orderedExtentDims[2]) { faceUVs = uvSmartPackQuads[0 + (faceIndex&1)]; swapUV = 0; } // mid,max
				else if (ai == orderedExtentDims[2] && bi == orderedExtentDims[1]) { faceUVs = uvSmartPackQuads[0 + (faceIndex&1)]; swapUV = 1; } // max,mid
				else if (ai == orderedExtentDims[0] && bi == orderedExtentDims[2]) { faceUVs = uvSmartPackQuads[2 + (faceIndex&1)]; swapUV = 0; } // min,max
				else if (ai == orderedExtentDims[2] && bi == orderedExtentDims[0]) { faceUVs = uvSmartPackQuads[2 + (faceIndex&1)]; swapUV = 1; } // max,min
				else if (ai == orderedExtentDims[1] && bi == orderedExtentDims[0]) { faceUVs = uvSmartPackQuads[4 + (faceIndex&1)]; swapUV = 0; } // mid,min
				else if (ai == orderedExtentDims[0] && bi == orderedExtentDims[1]) { faceUVs = uvSmartPackQuads[4 + (faceIndex&1)]; swapUV = 1; } // min,mid
				else ForceAssert(false);
			} else { // dumb horizontal pack ..
				const Vec2V faceUVMin(faceIndex == 0 ? (0.5f + (float)uvPadding) : (faceUVx1 + (float)uvPadding + 1.0f), 0.5f + (float)uvPadding); 
				const Vec2V faceUVMax = faceUVMin + Ceiling(Vec2V(Mag(a), Mag(b))*uvDensity);
				faceUVs = Box2V(faceUVMin, faceUVMax);
				faceUVx1 = faceUVMax.xf();
			}
		}
		uint32* indices = new uint32[(w + 1)*(h + 1)];
		for (uint32 j = 0; j <= h; j++) {
			for (uint32 i = 0; i <= w; i++) {
				const float x = RoundBoxFunction(i, w, roundRes, interiorResX, roundRadiusRelativeX, uniformAngularSpacing); // [0..1]
				const float y = RoundBoxFunction(j, h, roundRes, interiorResY, roundRadiusRelativeY, uniformAngularSpacing); // [0..1]
				const Vec3V p = corner + a*x + b*y; // vertex of tessellated box (not rounded)
				Vec3V n = axes.c();
				Vec3V v = p;
				Vec2V texcoord(V_ZERO);
				if (roundRadius > 0.0f) {
					const Vec3V q = Clamp(p, box.GetMin() + ScalarV(roundRadius), box.GetMax() - ScalarV(roundRadius));
					n = Normalize(p - q);
					v = q + n*roundRadius;
				}
				Vec3V p_quant;
				int index = -1;
				bool inside = false;
				if (uvDensity > 0.0f) {
					float tx = RoundBoxFunction(i, w, roundRes, interiorResX, roundRadiusRelativeX, false); // [0..1]
					float ty = RoundBoxFunction(j, h, roundRes, interiorResY, roundRadiusRelativeY, false); // [0..1]
					if (swapUV)
						std::swap(tx, ty);
					const float expand = 1.0f/256.0f; // in texels - expand UVs slightly so that we make sure to render texel centers
					const Vec2V tmin = faceUVs.GetMin() - ScalarV(expand);
					const Vec2V tmax = faceUVs.GetMax() + ScalarV(expand);
					texcoord = tmin + (tmax - tmin)*Vec2V(tx, ty);
				} else if (uniqueVerts && (i == 0 || j == 0 || i == w || j == h)) {
					const float x_uniform = (float)i/(float)w;
					const float y_uniform = (float)j/(float)h;
					const Vec3V p_uniform = corner + a*x_uniform + b*y_uniform;
				#if 1 // test quantization robustness
					const float test = 0.4999f*(-1.0f + 2.0f*(float)rand()/(float)RAND_MAX);
				#else
					const float test = 0.0f;
				#endif
					p_quant = Floor(ScalarV(0.5f + test) + (p_uniform - box.GetMin())/box.GetSize()*resV);
					const auto it = pm.find(p_quant);
					if (it != pm.end())
						index = it->second;
					inside = true;
					if (roundRadius <= 0.0f && mesh.m_normals)
						n = Normalize(p - box.GetCenter()); // edge normals
				}
				if (index == -1) {
					index = (int)mesh.m_verts.size();
					mesh.m_verts.push_back(v);
					if (mesh.m_normals)
						mesh.m_normals->push_back(n);
					if (mesh.m_texcoords)
						mesh.m_texcoords->push_back(texcoord);
					if (inside)
						pm[p_quant] = (uint32)index;
				}
				indices[i + j*(w + 1)] = (uint32)index;
			}
		}
		for (uint32 j = 0; j < h; j++) {
			for (uint32 i = 0; i < w; i++) {
				IndexedQuad(vertStart,
					indices[(i + 0) + (j + 1)*(w + 1)],
					indices[(i + 1) + (j + 1)*(w + 1)],
					indices[(i + 1) + (j + 0)*(w + 1)],
					indices[(i + 0) + (j + 0)*(w + 1)], -1).Add(mesh.m_polys);
			}
		}
		delete[] indices;
	}
	if (mesh.m_texcoords)
		mesh.FillVertexTexcoords(Vec2V(V_ZERO));
	if (mesh.m_colors)
		mesh.FillVertexColors(Vec4V(V_ZERO));
}

void ConstructRoundBox(TriangleMesh& mesh, const Box3V& box, float roundRadius, float vertDensity, float uvDensity, unsigned uvPadding, uint32 faceIgnoreMask, bool optimized, bool uniformAngularSpacing)
{
	ConstructRoundBox_T(mesh, box, roundRadius, vertDensity, uvDensity, uvPadding, faceIgnoreMask, optimized, uniformAngularSpacing);
}

void ConstructRoundBox(QuadMesh& mesh, const Box3V& box, float roundRadius, float vertDensity, float uvDensity, unsigned uvPadding, uint32 faceIgnoreMask, bool optimized, bool uniformAngularSpacing)
{
	ConstructRoundBox_T(mesh, box, roundRadius, vertDensity, uvDensity, uvPadding, faceIgnoreMask, optimized, uniformAngularSpacing);
}

template <typename MeshType> static void ConstructTessellatedQuad_T(MeshType& mesh, const Vec3V corners[4], unsigned tess_x, unsigned tess_y, const Mat34V* transform)
{
	// 3 ---- 2
	// |      |
	// |      |
	// 0 ---- 1
	mesh.CheckVertexStreams();
	MeshAssert(tess_x > 0 && tess_y > 0);
	if (transform == NULL)
		transform = &Mat34V::StaticIdentity();
	const uint32 vertStart = (uint32)mesh.m_verts.size();
	for (unsigned j = 0; j <= tess_y; j++) {
		const float y = (float)j/(float)tess_y; // [0..1]
		const Vec3V v03 = corners[0] + (corners[3] - corners[0])*y;
		const Vec3V v12 = corners[1] + (corners[2] - corners[1])*y;
		for (unsigned i = 0; i <= tess_x; i++) {
			const float x = (float)i/(float)tess_x; // [0..1]
			mesh.m_verts.push_back(transform->Transform(v03 + (v12 - v03)*x));
		}
	}
	for (unsigned j = 0; j < tess_y; j++) {
		for (unsigned i = 0; i < tess_x; i++) {
			IndexedQuad(vertStart,
				(i + 0) + (j + 0)*(tess_x + 1),
				(i + 1) + (j + 0)*(tess_x + 1),
				(i + 1) + (j + 1)*(tess_x + 1),
				(i + 0) + (j + 1)*(tess_x + 1), -1).Add(mesh.m_polys);
		}
	}
	if (mesh.m_normals)
		mesh.FillVertexNormals(NormalizeSafe(transform->TransformDir(Cross(corners[2] - corners[0], corners[3] - corners[1]))));
	if (mesh.m_texcoords)
		mesh.FillVertexTexcoords(Vec2V(V_ZERO)); // TODO -- we could support this pretty easily
	if (mesh.m_colors)
		mesh.FillVertexColors(Vec4V(V_ZERO));
}

void ConstructTessellatedQuad(TriangleMesh& mesh, const Vec3V corners[4], unsigned tess_x, unsigned tess_y, const Mat34V* transform)
{
	ConstructTessellatedQuad_T(mesh, corners, tess_x, tess_y, transform);
}

void ConstructTessellatedQuad(QuadMesh& mesh, const Vec3V corners[4], unsigned tess_x, unsigned tess_y, const Mat34V* transform)
{
	ConstructTessellatedQuad_T(mesh, corners, tess_x, tess_y, transform);
}

template <typename MeshType> static void AddTinyCubesForNormals_T(MeshType& mesh, float offset, float size)
{
	if (mesh.m_normals) {
		MeshAssert(mesh.m_normals->size() == mesh.m_verts.size());
		const uint32 numVerts = (uint32)mesh.m_verts.size();
		for (uint32 vertIndex = 0; vertIndex < numVerts; vertIndex++) {
			const Vec3V pos = mesh.m_verts[vertIndex];
			const Vec3V nrm = mesh.m_normals->operator[](vertIndex);
			const Vec3V center = pos + nrm*offset;
			const Vec3V extent = Vec3V(size*0.5f);
			ConstructBox_T(mesh, Box3V(center - extent, center + extent));
		}
	}
}

void AddTinyCubesForNormals(TriangleMesh& mesh, float offset, float size)
{
	AddTinyCubesForNormals_T(mesh, offset, size);
}

void AddTinyCubesForNormals(QuadMesh& mesh, float offset, float size)
{
	AddTinyCubesForNormals_T(mesh, offset, size);
}

bool PolyClip(Vec3V* dst, unsigned& dstCount, unsigned dstCountMax, const Vec3V* src, unsigned srcCount, const Plane3V& plane)
{
	bool clipped = false;
	dstCount = 0;
	Vec3V p0 = src[srcCount - 1];
	ScalarV d0 = plane.GetDistanceToPoint(p0);
	bool g0 = (d0 >= 0.0f);
	for (unsigned i = 0; i < srcCount; i++) {
		const Vec3V p1 = src[i];
		const ScalarV d1 = plane.GetDistanceToPoint(p1);
		const bool g1 = (d1 >= 0.0f);
		if (g1 != g0) {
			if (MeshAssertVerify(dstCount < dstCountMax))
				dst[dstCount++] = (p0*d1 - p1*d0)/(d1 - d0);
			clipped = true;
		}
		if (g1 && MeshAssertVerify(dstCount < dstCountMax))
			dst[dstCount++] = p1;
		else
			clipped = true;
		p0 = p1;
		d0 = d1;
		g0 = g1;
	}
	return clipped;
}

bool PolyClipEx(MeshBase::Vertex* dst, unsigned& dstCount, unsigned dstCountMax, const MeshBase::Vertex* src, const unsigned srcCount, const Plane3V& plane, bool normalize)
{
	bool clipped = false;
	dstCount = 0;
	MeshBase::Vertex v0 = src[srcCount - 1];
	ScalarV d0 = plane.GetDistanceToPoint(v0.m_pos);
	bool g0 = (d0 >= 0.0f);
	for (unsigned i = 0; i < srcCount; i++) {
		const MeshBase::Vertex v1 = src[i];
		const ScalarV d1 = plane.GetDistanceToPoint(v1.m_pos);
		const bool g1 = (d1 >= 0.0f);
		if (g1 != g0) {
			if (MeshAssertVerify(dstCount < dstCountMax))
				dst[dstCount++] = MeshBase::Interpolate(v0, v1, d0/(d0 - d1), false); // (v0*d1 - v1*d0)/(d1 - d0)
			clipped = true;
		}
		if (g1 && MeshAssertVerify(dstCount < dstCountMax))
			dst[dstCount++] = v1;
		else
			clipped = true;
		v0 = v1;
		d0 = d1;
		g0 = g1;
	}
	if (clipped && normalize) {
		for (unsigned i = 0; i < dstCount; i++)
			dst[i].m_normal = NormalizeSafe(dst[i].m_normal);
	}
	return clipped;
}

template <typename MeshType> static uint32 AddGeometry_T(MeshType& mesh, const MeshType& object, const Mat34V* objectTransform_, const Box3V* clip, const Mat34V* clipTransform_, const Vec4V* texScaleOffset, const char* groupNameSuffix)
{
	typedef typename MeshType::IndexedPolyType IndexedPolyType;
	mesh.CheckVertexStreams();
	object.CheckVertexStreams();
	if (mesh.m_normals   == NULL && object.m_normals  ) mesh.FillVertexNormals  (Vec3V(V_ZERO));
	if (mesh.m_texcoords == NULL && object.m_texcoords) mesh.FillVertexTexcoords(Vec2V(V_ZERO));
	if (mesh.m_colors    == NULL && object.m_colors   ) mesh.FillVertexColors   (Vec4V(V_ONE ));
	std::vector<int> groupTable;
	groupTable.resize(object.m_groupNames.size());
	for (uint32 groupIndex = 0; groupIndex < object.m_groupNames.size(); groupIndex++) {
		char buf[512];
		const char* groupName = object.m_groupNames[groupIndex].c_str();
		if (groupNameSuffix) {
			strcpy(buf, groupName);
			strcat(buf, groupNameSuffix);
			groupName = buf;
		}
		groupTable[groupIndex] = mesh.FindOrAddGroup(groupName);
	}
	uint32 numPolysAdded = 0;
	if (clip == NULL) {
		const uint32 vertStart = (uint32)mesh.m_verts.size();
		for (uint32 vertIndex = 0; vertIndex < object.m_verts.size(); vertIndex++)
			mesh.AddVertex(object.GetVertex(vertIndex).Transform(objectTransform_, texScaleOffset), false);
		for (uint32 polyIndex = 0; polyIndex < object.m_polys.size(); polyIndex++) {
			const IndexedPolyType& poly = object.m_polys[polyIndex];
			int group = poly.m_group;
			if (group != -1)
				group = groupTable[group];
			mesh.m_polys.push_back(poly.CopyPoly(vertStart, group));
			numPolysAdded++;
		}
	} else {
		const Mat34V objectTransform = objectTransform_ ? *objectTransform_ : Mat34V::Identity();
		const Mat34V clipTransform = clipTransform_ ? *clipTransform_ : Mat34V::Identity();
		if (IntersectsTransformed(object.GetBounds(), objectTransform, *clip, clipTransform)) {
			Plane3V clipPlanes[] = {
				Plane3V(+Vec3V(V_XAXIS),-clip->GetMin().x()),
				Plane3V(+Vec3V(V_YAXIS),-clip->GetMin().y()),
				Plane3V(+Vec3V(V_ZAXIS),-clip->GetMin().z()),
				Plane3V(-Vec3V(V_XAXIS),+clip->GetMax().x()),
				Plane3V(-Vec3V(V_YAXIS),+clip->GetMax().y()),
				Plane3V(-Vec3V(V_ZAXIS),+clip->GetMax().z()),
			};
			if (clipTransform_) {
				for (unsigned i = 0; i < countof(clipPlanes); i++)
					clipPlanes[i] = clipTransform.TransformPlane(clipPlanes[i]);
			}
			for (uint32 polyIndex = 0; polyIndex < object.m_polys.size(); polyIndex++) {
				const IndexedPolyType& poly = object.m_polys[polyIndex];
				const unsigned maxVertsPerClippedPoly = IndexedPolyType::NumVerts + 6 + 1; // T::N for unclipped polygon + 6 for cube planes + 1 to catch errors
				MeshBase::Vertex tempA[maxVertsPerClippedPoly];
				MeshBase::Vertex tempB[maxVertsPerClippedPoly];
				MeshBase::Vertex* dst = tempA;
				MeshBase::Vertex* src = tempB;
				unsigned dstCount = 0;
				unsigned srcCount = 0;
				for (unsigned i = 0; i < IndexedPolyType::NumVerts; i++) {
					if (i > 0) {
						if (poly.m_indices[i] == poly.m_indices[i - 1]) // consider case of triangle represented as a quad with a degenerate vertex ..
							continue;
						else if (All(object.m_verts[poly.m_indices[i]] == object.m_verts[poly.m_indices[i - 1]]))
							continue;
					}
					src[srcCount++] = object.GetVertex(poly.m_indices[i]).Transform(&objectTransform, texScaleOffset);
				}
				for (unsigned i = 0; i < countof(clipPlanes); i++) {
					if (PolyClipEx(dst, dstCount, maxVertsPerClippedPoly, src, srcCount, clipPlanes[i], false)) {
						MeshAssert(dstCount < maxVertsPerClippedPoly); // max verts was incremented by 1 - make sure we didn't hit this
						std::swap(src, dst);
						std::swap(srcCount, dstCount);
						if (srcCount < 3)
							break;
					}
				}
				if (srcCount >= 3) {
					uint32 indices[maxVertsPerClippedPoly];
					for (unsigned i = 0; i < srcCount; i++) {
						if (object.m_normals)
							src[i].m_normal = NormalizeSafe(src[i].m_normal);
						indices[i] = mesh.AddVertex(src[i], true);
					}
					int group = -1;
					if (poly.m_group != -1)
						group = groupTable[poly.m_group];
					numPolysAdded += IndexedPolyType::AddPolys(mesh.m_polys, 0, indices, srcCount, group);
				}
			}
			mesh.m_vertexMap.clear(); // currently we only maintain this map for a single instance
		}
	}
	return numPolysAdded;
}

uint32 AddGeometry(TriangleMesh& mesh, const TriangleMesh& object, const Mat34V* objectTransform, const Box3V* clip, const Mat34V* clipTransform, const Vec4V* texScaleOffset, const char* groupNameSuffix)
{
	return AddGeometry_T(mesh, object, objectTransform, clip, clipTransform, texScaleOffset, groupNameSuffix);
}

uint32 AddGeometry(QuadMesh& mesh, const QuadMesh& object, const Mat34V* objectTransform, const Box3V* clip, const Mat34V* clipTransform, const Vec4V* texScaleOffset, const char* groupNameSuffix)
{
	return AddGeometry_T(mesh, object, objectTransform, clip, clipTransform, texScaleOffset, groupNameSuffix);
}

template <typename MeshType,typename PolyType> static void ConstructNormals_T(MeshType& mesh)
{
	mesh.CheckVertexStreams();
	if (mesh.m_normals == NULL) {
		mesh.m_normals = new std::vector<Vec3V>(mesh.m_verts.size());
		memset(&mesh.m_normals->front(), 0, mesh.m_normals->size()*sizeof(Vec3V));
		for (uint32 polyIndex = 0; polyIndex < mesh.m_polys.size(); polyIndex++) {
			const Vec3V weightedNormal = MakePoly(mesh.m_polys[polyIndex], mesh.m_verts).GetWeightedNormal();
			for (unsigned i = 0; i < PolyType::NumVerts; i++)
				mesh.m_normals->operator[](mesh.m_polys[polyIndex].m_indices[i]) += weightedNormal;
		}
		for (uint32 vertIndex = 0; vertIndex < mesh.m_verts.size(); vertIndex++) {
			Vec3V& normal = mesh.m_normals->operator[](vertIndex);
			normal = NormalizeSafe(normal);
		}
	} else
		printf("already has normals!\n");
}

void ConstructNormals(TriangleMesh& mesh)
{
	ConstructNormals_T<TriangleMesh,Triangle3V>(mesh);
}

void ConstructNormals(QuadMesh& mesh)
{
	ConstructNormals_T<QuadMesh,Quad3V>(mesh);
}

void Tessellate4(TriangleMesh& mesh)
{
	mesh.CheckVertexStreams();
	std::map<uint32,uint32> midpointIndexMap;
	for (uint32 triIndex0 = 0; triIndex0 < mesh.m_polys.size(); triIndex0++) {
		for (unsigned side0 = 0; side0 < 3; side0++) {
			const uint32 indexA0 = mesh.m_polys[triIndex0].m_indices[side0];
			const uint32 indexB0 = mesh.m_polys[triIndex0].m_indices[(side0 + 1)%3];
			bool found = false; // is this side is shared, we can create a single shared midpoint vertex
			for (uint32 triIndex1 = triIndex0 + 1; triIndex1 < mesh.m_polys.size() && !found; triIndex1++) {
				for (unsigned side1 = 0; side1 < 3; side1++) {
					const uint32 indexA1 = mesh.m_polys[triIndex1].m_indices[side1];
					const uint32 indexB1 = mesh.m_polys[triIndex1].m_indices[(side1 + 1)%3];
					if (indexA0 == indexB1 && indexB0 == indexA1) {
						const uint32 index = mesh.AddInterpolatedVertex(indexA0, indexB0, ScalarV(0.5f), true, true);
						midpointIndexMap[(triIndex0 << 2)|side0] = index;
						midpointIndexMap[(triIndex1 << 2)|side1] = index;
						found = true;
						break;
					}
				}
			}
			if (!found) {
				if (midpointIndexMap.find((triIndex0 << 2)|side0) == midpointIndexMap.end()) {
					const uint32 index = mesh.AddInterpolatedVertex(indexA0, indexB0, ScalarV(0.5f), true, true);
					midpointIndexMap[(triIndex0 << 2)|side0] = index;
				}
			}
		}
	}
	const uint32 numTris = (uint32)mesh.m_polys.size();
	for (uint32 i = 0; i < numTris; i++) {
		const uint32 mid01 = midpointIndexMap[(i << 2)|0];
		const uint32 mid12 = midpointIndexMap[(i << 2)|1];
		const uint32 mid20 = midpointIndexMap[(i << 2)|2];
		mesh.m_polys.push_back(IndexedTriangle(0, mesh.m_polys[i].m_indices[0], mid01, mid20, mesh.m_polys[i].m_group));
		mesh.m_polys.push_back(IndexedTriangle(0, mesh.m_polys[i].m_indices[1], mid12, mid01, mesh.m_polys[i].m_group));
		mesh.m_polys.push_back(IndexedTriangle(0, mesh.m_polys[i].m_indices[2], mid20, mid12, mesh.m_polys[i].m_group));
		mesh.m_polys[i].m_indices[0] = mid01;
		mesh.m_polys[i].m_indices[1] = mid12;
		mesh.m_polys[i].m_indices[2] = mid20;
	}
}

void TessellateToEdgeLength(TriangleMesh& mesh, float edgeLength, ProgressDisplay* progress, int progressPeriod)
{
	mesh.CheckVertexStreams();
	const float edgeLengthSqr = edgeLength*edgeLength;
	if (progressPeriod <= 1)
		progressPeriod = 1;
	for (uint32 triIndex = 0; triIndex < mesh.m_polys.size(); triIndex++) {
		if (progress && triIndex%progressPeriod == 0)
			progress->Update(triIndex, mesh.m_polys.size());
		while (true) {
			const Vec3V pos[] = {
				mesh.GetTriangleVertexPos(triIndex, 0),
				mesh.GetTriangleVertexPos(triIndex, 1),
				mesh.GetTriangleVertexPos(triIndex, 2),
			};
			const ScalarV edgeLengthsSqr[] = {
				MagSqr(pos[1] - pos[0]),
				MagSqr(pos[2] - pos[1]),
				MagSqr(pos[0] - pos[2]),
			};
			const ScalarV edgeLengthSqrMax = Max(edgeLengthsSqr[0], edgeLengthsSqr[1], edgeLengthsSqr[2]);
			if (edgeLengthSqrMax > edgeLengthSqr) {
				unsigned splitSideIndex;
				if      (edgeLengthSqrMax == edgeLengthsSqr[0]) splitSideIndex = 0; // split between verts 0 and 1
				else if (edgeLengthSqrMax == edgeLengthsSqr[1]) splitSideIndex = 1; // split between verts 1 and 2
				else                                            splitSideIndex = 2; // split between verts 2 and 0
				const uint32 index0 = mesh.m_polys[triIndex].m_indices[splitSideIndex];
				const uint32 index1 = mesh.m_polys[triIndex].m_indices[(splitSideIndex + 1)%3];
				const uint32 index = mesh.AddInterpolatedVertex(index0, index1, ScalarV(0.5f), true, true);
				IndexedTriangle triCopy = mesh.m_polys[triIndex]; // copy
				mesh.m_polys[triIndex].m_indices[(splitSideIndex + 1)%3] = index;
				triCopy.m_indices[splitSideIndex] = index;
				mesh.m_polys.push_back(triCopy);
			} else
				break;
		}
	}
}

template <typename MeshType> static void WeldPositions_T(MeshType& mesh, float tolerance)
{
	class Bucket
	{
	public:
		void Set(uint32 index)
		{
			memset(m_axes, 0, sizeof(m_axes));
			m_index = index;
		}
		bool operator <(const Bucket& rhs) const
		{
			for (unsigned axis = 0; axis < countof(m_axes); axis++) {
				if (m_axes[axis] < rhs.m_axes[axis])
					return true;
				else if (m_axes[axis] > rhs.m_axes[axis])
					return false;
			}
			return false;
		}
		uint32 m_axes[3];
		uint32 m_index;
	};
	std::vector<Bucket> buckets(mesh.m_verts.size());
	for (uint32 vertIndex = 0; vertIndex < mesh.m_verts.size(); vertIndex++)
		buckets[vertIndex].Set(vertIndex);
	for (unsigned axis = 0; axis < 3; axis++) {
		class SortablePositionReference
		{
		public:
			void Set(const std::vector<Vec3V>& verts, uint32 index, unsigned axis)
			{
				m_value = verts[index][axis];
				m_index = index;
			}
			bool operator <(const SortablePositionReference& rhs) const
			{
				return m_value < rhs.m_value;
			}
			float m_value; // x,y, or z
			uint32 m_index; // index into mesh.m_verts
		};
		std::vector<SortablePositionReference> refs(mesh.m_verts.size());
		for (uint32 vertIndex = 0; vertIndex < mesh.m_verts.size(); vertIndex++)
			refs[vertIndex].Set(mesh.m_verts, vertIndex, axis);
		std::sort(refs.begin(), refs.end());
		float value = refs[0].m_value;
		uint32 bucket = 0;
		for (uint32 i = 0; i < mesh.m_verts.size(); i++) {
			if (refs[i].m_value - value > tolerance) {
				value = refs[i].m_value;
				bucket++;
			}
			buckets[refs[i].m_index].m_axes[axis] = bucket;
		}
	}
	std::sort(buckets.begin(), buckets.end());
	uint32 bucketStart = 0;
	for (uint32 i = 1; i < mesh.m_verts.size(); i++) {
		if (buckets[i - 1] < buckets[i]) {
			if (bucketStart < i - 1) {
				Box3V bounds = Box3V::Invalid();
				for (uint32 j = bucketStart; j < i; j++)
					bounds.Grow(mesh.m_verts[buckets[j].m_index]);
				const Vec3V extent = bounds.GetExtent();
				if (Any(extent != Vec3V(V_ZERO))) {
					//printf("welding %u vert positions! extent = %f,%f,%f\n", i - bucketStart, VEC3V_ARGS(extent));
					const Vec3V center = bounds.GetCenter();
					for (uint32 j = bucketStart; j < i; j++)
						mesh.m_verts[buckets[j].m_index] = center;
				}
			}
			bucketStart = i;
		}
	}
}

void WeldPositions(TriangleMesh& mesh, float tolerance)
{
	WeldPositions_T(mesh, tolerance);
}

void WeldPositions(QuadMesh& mesh, float tolerance)
{
	WeldPositions_T(mesh, tolerance);
}

void ConvertQuadMeshToTriangleMesh(TriangleMesh& mesh, const QuadMesh& quadMesh)
{
	mesh.Clear();
	mesh.m_verts.resize(quadMesh.m_verts.size());
	memcpy(mesh.m_verts.data(), quadMesh.m_verts.data(), quadMesh.m_verts.size()*sizeof(Vec3V));
	if (quadMesh.m_normals  ) mesh.m_normals   = new std::vector<Vec3V>(*quadMesh.m_normals  );
	if (quadMesh.m_texcoords) mesh.m_texcoords = new std::vector<Vec2V>(*quadMesh.m_texcoords);
	if (quadMesh.m_colors   ) mesh.m_colors    = new std::vector<Vec4V>(*quadMesh.m_colors   );
	for (uint32 polyIndex = 0; polyIndex < quadMesh.m_polys.size(); polyIndex++)
		quadMesh.m_polys[polyIndex].Add(mesh.m_polys);
}

bool GenerateTestCylinderObject(const char* path, const char* mtllib, const char* material, Vec3V_arg origin, Vec3V_arg axis, float radius, float textureRepeats, unsigned numInstances, unsigned numSlices, unsigned numStacks, bool caps)
{
	FILE* file = fopen(path, "w");
	if (file) {
		const Mat34V basis = Mat34V::ConstructBasis(origin, axis);
		const float length = Mag(axis).f();
		if (mtllib)
			fprintf(file, "mtllib %s\n", mtllib);

		// A = numSlices
		// B = numStacks
		// there will be A*(B + 1) vertex positions, (A + 1)*(B + 1) texcoords, and A normals
		// additionally 2 vertex positions, A + 1 texcoords, and 2 normals if we generate caps
		// we can use this to test the case where the position/texcoord/normal indices differ in the OBJ file

		Vec3V offset(V_ZERO); 
		for (unsigned instance = 0; instance < numInstances; instance++) {
			const float rad = radius/(float)(1 + instance);
			const float len = length/(float)(1 + instance);

			// positions
			for (unsigned stack = 0; stack <= numStacks; stack++) {
				const float z = len*(float)stack/(float)numStacks; // [0..length]
				for (unsigned slice = 0; slice < numSlices; slice++) {
					const float theta = 2.0f*PI*(float)slice/(float)numSlices; // [0..2PI)
					const Vec3V pos = offset + basis.Transform(Vec3V(cosf(theta)*rad, sinf(theta)*rad, z));
					fprintf(file, "v %f %f %f\n", VEC3V_ARGS(pos));
				}
			}
			if (caps) {
				const Vec3V p1 = offset + basis.d();
				const Vec3V p2 = offset + basis.d() + basis.c()*len;
				fprintf(file, "v %f %f %f\n", VEC3V_ARGS(p1)); // numSlices*(numStacks + 1) + 0
				fprintf(file, "v %f %f %f\n", VEC3V_ARGS(p2)); // numSlices*(numStacks + 1) + 1
			}

			// texcoords
			const Vec2V texScale = Vec2V(1.0f, len/(2.0f*PI*rad))*textureRepeats;
			for (unsigned stack = 0; stack <= numStacks; stack++) {
				const float y = (float)stack/(float)numStacks; // [0..1]
				for (unsigned slice = 0; slice <= numSlices; slice++) {
					const float x = (float)slice/(float)numSlices; // [0..1]
					const Vec2V tex = Vec2V(x, y)*texScale;
					fprintf(file, "vt %f %f\n", VEC2V_ARGS(tex));
				}
			}
			if (caps) {
				for (unsigned slice = 0; slice < numSlices; slice++) {
					const float theta = 2.0f*PI*(float)slice/(float)numSlices; // [0..2PI)
					fprintf(file, "vt %f %f\n", 0.5f*(1.0f + cosf(theta)), 0.5f*(1.0f + sinf(theta))); // (numSlices + 1)*(numStacks + 1) + [0..numSlices)
				}
				fprintf(file, "vt %f %f\n", 0.5f, 0.5f); // (numSlices + 1)*(numStacks + 1) + numSlices
			}

			// normals
			for (unsigned slice = 0; slice < numSlices; slice++) {
				const float theta = 2.0f*PI*(float)slice/(float)numSlices; // [0..2PI)
				const Vec3V normal = basis.TransformDir(Vec3V(cosf(theta), sinf(theta), 0.0f));
				fprintf(file, "vn %f %f %f\n", VEC3V_ARGS(normal));
			}
			if (caps) {
				const Vec3V nz = -basis.c();
				const Vec3V pz = +basis.c();
				fprintf(file, "vn %f %f %f\n", VEC3V_ARGS(nz)); // numSlices + 0
				fprintf(file, "vn %f %f %f\n", VEC3V_ARGS(pz)); // numSlices + 1
			}

			const int k = numInstances > 1 ? -1 : 1; // multiple instances use local vertices
			if (material)
				fprintf(file, "usemtl %s\n", material); // something textured and tileable
			for (unsigned stack = 0; stack < numStacks; stack++) {
				for (unsigned slice = 0; slice < numSlices; slice++) {
					const int pos00 = (slice + 0)%numSlices + (stack + 0)*numSlices;
					const int pos10 = (slice + 1)%numSlices + (stack + 0)*numSlices;
					const int pos11 = (slice + 1)%numSlices + (stack + 1)*numSlices;
					const int pos01 = (slice + 0)%numSlices + (stack + 1)*numSlices;
					const int tex00 = (slice + 0) + (stack + 0)*(numSlices + 1);
					const int tex10 = (slice + 1) + (stack + 0)*(numSlices + 1);
					const int tex11 = (slice + 1) + (stack + 1)*(numSlices + 1);
					const int tex01 = (slice + 0) + (stack + 1)*(numSlices + 1);
					const int nrm00 = 1 + (slice + 0)%numSlices;
					const int nrm10 = 1 + (slice + 1)%numSlices;
					const int nrm11 = 1 + (slice + 1)%numSlices;
					const int nrm01 = 1 + (slice + 0)%numSlices;
					fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
						k*(1 + pos00), k*(1 + tex00), k*(1 + nrm00),
						k*(1 + pos10), k*(1 + tex10), k*(1 + nrm10),
						k*(1 + pos11), k*(1 + tex11), k*(1 + nrm11),
						k*(1 + pos01), k*(1 + tex01), k*(1 + nrm01));
				}
			}
			if (caps) {
				for (unsigned slice = 0; slice < numSlices; slice++) {
					const int pos0 = (slice + 1)%numSlices;
					const int pos1 = (slice + 0)%numSlices;
					const int posC = numSlices*(numStacks + 1);
					const int tex0 = (numSlices + 1)*(numStacks + 1) + (slice + 1)%numSlices;
					const int tex1 = (numSlices + 1)*(numStacks + 1) + (slice + 0)%numSlices;
					const int texC = (numSlices + 1)*(numStacks + 1) + numSlices;
					fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
						k*(1 + posC), k*(1 + texC), k*(1 + numSlices),
						k*(1 + pos0), k*(1 + tex0), k*(1 + numSlices),
						k*(1 + pos1), k*(1 + tex1), k*(1 + numSlices));
				}
				for (unsigned slice = 0; slice < numSlices; slice++) {
					const int pos0 = numSlices*numStacks + (slice + 0)%numSlices;
					const int pos1 = numSlices*numStacks + (slice + 1)%numSlices;
					const int posC = numSlices*(numStacks + 1) + 1;
					const int tex0 = (numSlices + 1)*(numStacks + 1) + (slice + 0)%numSlices;
					const int tex1 = (numSlices + 1)*(numStacks + 1) + (slice + 1)%numSlices;
					const int texC = (numSlices + 1)*(numStacks + 1) + numSlices;
					fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
						k*(1 + posC), k*(1 + texC), k*(1 + numSlices + 1),
						k*(1 + pos0), k*(1 + tex0), k*(1 + numSlices + 1),
						k*(1 + pos1), k*(1 + tex1), k*(1 + numSlices + 1));
				}
			}

			offset += basis.a()*rad*2.0f;
		}
		fclose(file);
		return true;
	} else
		return false;
}

bool CreateZUPOBJ(const char* path, const char* ext)
{
	const Mat34V transform = Mat34V(Vec3V(V_YAXIS), Vec3V(V_ZAXIS), Vec3V(V_XAXIS), Vec3V(V_ZERO));
	return CreateTransformedOBJ(path, transform, ext);
}

bool CreateTransformedOBJ(const char* path, Mat34V_arg transform, const char* ext)
{
	TriangleMesh mesh;
	if (LoadOBJ(path, mesh, false)) {
		for (uint32 vertIndex = 0; vertIndex < mesh.m_verts.size(); vertIndex++) {
			Vec3V& v = mesh.m_verts[vertIndex];
			v = transform.Transform(v);
		}
		return SaveOBJ(PathExt(path, ext), mesh);
	}
	return false;
}

} // namespace geomesh