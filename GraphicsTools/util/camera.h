// ===============
// common/camera.h
// ===============

#ifndef _INCLUDE_COMMON_CAMERA_H_
#define _INCLUDE_COMMON_CAMERA_H_

#include "common/common.h"

#if PLATFORM_PC

#if defined(_CPU_FRAMEBUFFER)
#include "framebuffer.h"
#endif // defined(_CPU_FRAMEBUFFER)

#include "vmath/vmath_common.h"
#include "vmath/vmath_matrix.h"
#include "vmath/vmath_sphere.h"
#include "vmath/vmath_vec3.h"

class Box3V;
class Plane3V;
class Keyboard;

#define CAMERA_ZNEAR_DEFAULT 0.001f
#define CAMERA_ZFAR_DEFAULT 10.0f
#define CAMERA_FLIP_X (0) // ugh .. trying to get this right

class Camera
{
public:
	Camera();

	static Mat34V_out SetupViewToWorld(Vec3V_arg upDir, const Box3V& bounds, float relativeOffset = 2.5f, float relativeHeight = 0.0f);
	void SetupFromBounds(Vec3V_arg upDir, const Box3V& bounds, float relativeOffset = 2.5f, float relativeHeight = 0.0f, float znear = CAMERA_ZNEAR_DEFAULT, float zfar = CAMERA_ZFAR_DEFAULT);
	void Setup(Vec3V_arg upDir, Mat34V_arg viewToWorld, float znear = CAMERA_ZNEAR_DEFAULT, float zfar = CAMERA_ZFAR_DEFAULT, float worldScale = 1.0f);
	inline void SetMouseWheelSpeed(float speed) { m_mouseWheelSpeed = speed; }

	bool IsYUpDir() const { return All(m_upDir == Vec3V(V_YAXIS)); }
	bool IsZUpDir() const { return All(m_upDir == Vec3V(V_ZAXIS)); }

	void Resize(unsigned w, unsigned h);
	void ResizeUpdate();

	void MouseWheelForward(float speed, bool useDragPanSpeed = false);
	void Update(const Keyboard& keyboard, float FPS = 0.0f);

	void SetCurrentMousePos(int x, int y);
	void BeginDrag(const Keyboard& keyboard, int x, int y, int button);
	void UpdateDrag(int x, int y);
	void EndDrag(int x, int y);

	static Vec3V_out GetForwardDirFromYawPitch(float yaw, float pitch, Vec3V_arg upDir);
	inline const Vec3V_out GetForwardDirFromCurrentYawPitch() const { return GetForwardDirFromYawPitch(m_yaw, m_pitch, m_upDir); }
	static void GetYawPitchFromForwardDir(float& yaw, float& pitch, Vec3V_arg forwardDir, Vec3V_arg upDir);

	inline const Mat34V_out GetViewToWorld() const { return m_viewToWorld; }
	inline const Vec3V_out GetPosition() const { return m_viewToWorld.d(); }
	inline const Vec3V_out GetForwardDir() const { return m_viewToWorld.c(); }
	inline float GetYawInDegrees() const { return RAD_TO_DEG*m_yaw; }
	inline float GetPitchInDegrees() const { return RAD_TO_DEG*m_yaw; }
	inline float GetVFOVInDegrees() const { return m_VFOV; }
	inline float GetTanVFOV() const { return tanf(DEG_TO_RAD*0.5f*m_VFOV); }
	inline float GetAspect() const { return (float)m_viewportWidth/(float)m_viewportHeight; }
	inline float GetZNear() const { return m_znear; }
	inline float GetZFar() const { return m_zfar; }

	void SetYawPitchFromCurrentViewToWorld();
	void SetViewToWorld(Mat34V_arg viewToWorld);

#if defined(_CPU_FRAMEBUFFER)
	Pixel32& GetPixelColor_ref(int x, int y) const;
	float& GetPixelDepth_ref(int x, int y) const;
#endif // defined(_CPU_FRAMEBUFFER)
	float GetPixelDepth(int x, int y) const;
#if defined(_CPU_FRAMEBUFFER)
	int GetPixelSelectID(int x, int y) const;
	Vec3V_out GetPixelSelectUV(int x, int y) const;
#endif // defined(_CPU_FRAMEBUFFER)

	Vec3V_out GetWorldDirAtPixel(int x, int y, float z, Mat34V_arg viewToWorld) const;
	Vec3V_out GetWorldDirAtPixel(int x, int y, float z = 1.0f) const;
#if defined(_CPU_FRAMEBUFFER)
	Vec3V_out GetWorldDirAtFramebufferPixel(int i, int j, float z = 1.0f) const;
#endif // defined(_CPU_FRAMEBUFFER)
	bool GetWorldPosAtPixel(Vec3V& pos_out, int x, int y) const;
	bool GetWorldPosUnderMouse(Vec3V& pos_out) const;
	Vec3V_out GetWorldPlaneIntersectionAtPixel(int x, int y, const Plane3V& plane) const;

	Vec2V_out GetScreenPos(Vec3V_arg worldPos) const;

	const Sphere3V& GetFocusSphere() const;
	Vec3V_out GetFocus() const;

	uint64 GetHash(uint64 hash = 0) const;

#if defined(_OPENGL)
	inline float GetLinearZFromOpenGLDepth(float depth) const
	{
		if (depth <= 0.0f) // handle znear and zfar explicitly to avoid floating-point error
			return m_znear;
		else if (depth < 1.0f)
			return m_zfar*m_znear/(m_zfar*(1.0f - depth) + m_znear*depth); // should we clamp?
		else
			return m_zfar;
	}
	const Mat34V_out GetOpenGLModelViewMatrix() const;
	const Mat44V_out GetOpenGLModelViewProjMatrix() const;
	void BeginDisplayOpenGL() const;
	void EndDisplayOpenGL() const;
	void DrawFocusSphereOpenGL() const;
#if defined(_CPU_FRAMEBUFFER)
	void CopyFramebufferOpenGL(bool forceAlpha255 = false);
#endif // defined(_CPU_FRAMEBUFFER)
#endif // defined(_OPENGL)

	inline int GetCurrentMousePosX() const { return m_currentMousePosX; }
	inline int GetCurrentMousePosY() const { return m_currentMousePosY; }

#if defined(CPU_FRAMEBUFFER)
	Framebuffer m_framebuffer;
	bool m_framebufferDepthValid;
	unsigned m_framebufferScale;
#endif // defined(CPU_FRAMEBUFFER)
	unsigned m_viewportWidth;
	unsigned m_viewportHeight;
	Vec3V m_upDir; // Y or Z axis
	float m_worldScale;

private:
	float m_yaw;
	float m_pitch;
	float m_yawVelocity;
	float m_pitchVelocity;
	Vec3V m_translateVelocity;
	float m_translateSpeed;
	float m_rotateSpeed;
	float m_mouseWheelSpeed;
	float m_VFOV; // vertical field of view, in degrees
	float m_znear;
	float m_zfar;
	Mat34V m_viewToWorld;
	Mat34V m_viewToWorldDefault;

	int m_currentMousePosX;
	int m_currentMousePosY;
	int m_dragMousePosX;
	int m_dragMousePosY;
	enum eCameraDragAction
	{
		CAMERA_DRAG_ACTION_NONE,
		CAMERA_DRAG_ACTION_PAN,
		CAMERA_DRAG_ACTION_ORBIT,
	};
	eCameraDragAction m_dragAction;
	Vec3V m_dragPanNormal;
	Vec3V m_dragPanStart;
	Vec3V m_dragPanScreenX;
	Vec3V m_dragPanScreenY;
	float m_dragPanScale;
	float m_dragOrbitYaw;
	float m_dragOrbitPitch;
	float m_dragOrbitPitchEnabled;
	Vec3V m_dragOrbitCenter;
	float m_dragOrbitScale;
	Mat34V m_dragViewToWorld;
	float m_prevDragZ;

	Sphere3V m_focus;
};

#endif // PLATFORM_PC
#endif // _INCLUDE_COMMON_CAMERA_H_