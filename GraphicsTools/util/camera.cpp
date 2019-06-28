// =================
// common/camera.cpp
// =================

#include "common/common.h"

#if PLATFORM_PC

#include "vmath/vmath_box.h"
#include "vmath/vmath_plane.h"

#include "camera.h"
#include "crc.h"
#include "keyboard.h"
#include "progressdisplay.h" // for GetDeltaTimeInSeconds

#define WINDOW_WIDTH_DEFAULT 640
#define WINDOW_HEIGHT_DEFAULT 480
#define CAMERA_VFOV_DEFAULT 80.0f

Camera::Camera()
{
#if defined(_CPU_FRAMEBUFFER)
	m_framebufferDepthValid = false;
	m_framebufferScale = 1;
#endif // defined(_CPU_FRAMEBUFFER)
	m_viewportWidth = WINDOW_WIDTH_DEFAULT;
	m_viewportHeight = WINDOW_HEIGHT_DEFAULT;
	m_upDir = Vec3V(V_ZERO); // camera will not update until this is set
	m_worldScale = 1.0f;
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_yawVelocity = 0.0f;
	m_pitchVelocity = 0.0f;
	m_translateVelocity = Vec3V(V_ZERO);
	m_translateSpeed = 0.1f;
	m_rotateSpeed = DEG_TO_RAD*2.5f;
	m_mouseWheelSpeed = 1.0f;
	m_VFOV = CAMERA_VFOV_DEFAULT;
	m_znear = 0.1f;
	m_zfar = 10.0f;
	m_viewToWorld = Mat34V::Identity();
	m_viewToWorldDefault = Mat34V::Identity();

	m_currentMousePosX = 0;
	m_currentMousePosY = 0;
	m_dragAction = CAMERA_DRAG_ACTION_NONE;
	m_dragMousePosX = 0;
	m_dragMousePosY = 0;
	m_dragPanNormal = Vec3V(V_ZERO);
	m_dragPanStart = Vec3V(V_ZERO);
	m_dragPanScreenX = Vec3V(V_ZERO);
	m_dragPanScreenY = Vec3V(V_ZERO);
	m_dragPanScale = 0.0f;
	m_dragOrbitYaw = 0.0f;
	m_dragOrbitPitch = 0.0f;
	m_dragOrbitPitchEnabled = true;
	m_dragOrbitCenter = Vec3V(V_ZERO);
	m_dragOrbitScale = 0.0f;
	m_dragViewToWorld = Mat34V::Identity();
	m_prevDragZ = 0.0f;

	m_focus = Sphere3V::Invalid();
}

Mat34V_out Camera::SetupViewToWorld(Vec3V_arg upDir, const Box3V& bounds, float relativeOffset, float relativeHeight)
{
	Vec3V offset;
	if (All(upDir == Vec3V(V_YAXIS))) {
		offset = Vec3V(0.0f, relativeHeight, -relativeOffset);
	} else {
		ForceAssert(All(upDir == Vec3V(V_ZAXIS)));
		offset = Vec3V(-relativeOffset, 0.0f, relativeHeight);
	}
	const Vec3V camPos = bounds.GetCenter() + bounds.GetExtent()*offset;
	return Mat34V::ConstructBasisUp(camPos, bounds.GetCenter() - camPos, upDir);
}

void Camera::SetupFromBounds(Vec3V_arg upDir, const Box3V& bounds, float relativeOffset, float relativeHeight, float znear, float zfar)
{
	const Vec3V extent = bounds.GetExtent();
	const float worldScale = Dot(extent, Abs(upDir)).f();//MaxElement(extent*(Vec3V(V_ONE) - Abs(upDir))).f();
	const Mat34V viewToWorld = SetupViewToWorld(upDir, bounds, relativeOffset, relativeHeight);
	Setup(upDir, viewToWorld, znear, zfar, worldScale);
}

void Camera::Setup(Vec3V_arg upDir, Mat34V_arg viewToWorld, float znear, float zfar, float worldScale)
{
	if (ForceAssertVerify(All(m_upDir == Vec3V(V_ZERO)))) {
		m_upDir = upDir; // need to set this first, as it affects how yaw/pitch work
		m_znear = worldScale*znear;
		m_zfar = worldScale*zfar;
		m_worldScale = worldScale;
		SetViewToWorld(viewToWorld);
		m_viewToWorldDefault = viewToWorld;
		m_translateSpeed *= worldScale;
	}
}

void Camera::Resize(unsigned w, unsigned h)
{
	m_viewportWidth = w;
	m_viewportHeight = h;
#if defined(_CPU_FRAMEBUFFER)
	m_framebuffer.Resize(w/m_framebufferScale, h/m_framebufferScale);
#endif // defined(_CPU_FRAMEBUFFER)
}

void Camera::ResizeUpdate()
{
	Resize(m_viewportWidth, m_viewportHeight);
}

void Camera::MouseWheelForward(float speed, bool useDragPanSpeed)
{
	if (useDragPanSpeed && m_dragPanScale > 0.0f)
		speed *= m_dragPanScale;
	else
		speed *= m_translateSpeed;
	m_translateVelocity = GetForwardDir()*m_mouseWheelSpeed*speed;
}

void Camera::Update(const Keyboard& keyboard, float FPS)
{
	if (FPS > 0.0f) {
		static uint64 baseTime = 0;
		const float deltaTime = ProgressDisplay::GetDeltaTimeInSeconds(baseTime);
		const DWORD sleepTime = (DWORD)Max(0.0f, 1000.0f*(1.0f/FPS - deltaTime));
		if (sleepTime > 0)
			Sleep(sleepTime);
		//else if (deltaTime > 3.0f/FPS)
		//	printf("deltaTime = %fmsecs ..\n", 1000.0f*deltaTime);
	}

#if defined(_CPU_FRAMEBUFFER)
	// note that these aren't dependent on the camera transform being set up .. should probably call "m_framebuffer.Update(keyboard)"
	const unsigned framebufferScaleMin = 1;
	const unsigned framebufferScaleMax = 16;
	if (keyboard.IsKeyPressed(VK_DOWN)) if (m_framebufferScale < framebufferScaleMax) { m_framebufferScale++; ResizeUpdate(); }
	if (keyboard.IsKeyPressed(VK_UP)) if (m_framebufferScale > framebufferScaleMin) { m_framebufferScale--; ResizeUpdate(); }
#endif // defined(_CPU_FRAMEBUFFER)

	if (All(m_upDir == Vec3V(V_ZERO)))
		return;
	if (keyboard.IsKeyDown(VK_NUMPAD8)) m_translateVelocity = +m_viewToWorld.c()*m_translateSpeed;
	if (keyboard.IsKeyDown(VK_NUMPAD5)) m_translateVelocity = -m_viewToWorld.c()*m_translateSpeed;
	if (keyboard.IsKeyDown(VK_NUMPAD3)) m_translateVelocity = +m_viewToWorld.a()*m_translateSpeed;
	if (keyboard.IsKeyDown(VK_NUMPAD1)) m_translateVelocity = -m_viewToWorld.a()*m_translateSpeed;
	if (keyboard.IsKeyDown(VK_NUMPAD4)) m_yawVelocity = -m_rotateSpeed;
	if (keyboard.IsKeyDown(VK_NUMPAD6)) m_yawVelocity = +m_rotateSpeed;
	if (keyboard.IsKeyDown(VK_NUMPAD7)) m_pitchVelocity = -m_rotateSpeed;
	if (keyboard.IsKeyDown(VK_NUMPAD9)) m_pitchVelocity = +m_rotateSpeed;

	const float decay = 0.2f;
	Vec3V& pos_ref = m_viewToWorld.d_ref();
	pos_ref += m_translateVelocity; m_translateVelocity *= 1.0f - decay;
	m_yaw   += m_yawVelocity;       m_yawVelocity       *= 1.0f - decay;
	m_pitch += m_pitchVelocity;     m_pitchVelocity     *= 1.0f - decay;

	// set to zero if moving very slowly
	const float minRelativeVelocity = 0.001f;
	if (All(Abs(m_translateVelocity) <= m_translateSpeed*minRelativeVelocity) &&
		Abs(m_yawVelocity) <= m_rotateSpeed*minRelativeVelocity &&
		Abs(m_pitchVelocity) <= m_rotateSpeed*minRelativeVelocity) {
		m_translateVelocity = Vec3V(V_ZERO);
		m_yawVelocity = 0.0f;
		m_pitchVelocity = 0.0f;
	}

	while (m_yaw > +PI) m_yaw -= 2.0f*PI;
	while (m_yaw < -PI) m_yaw += 2.0f*PI;

	if (keyboard.IsKeyPressed('F')) {
		bool disable = false;
		if (m_focus.IsValid()) {
			const Vec3V origin = m_viewToWorld.d();
			const Vec3V dir = GetWorldDirAtPixel(m_currentMousePosX, m_currentMousePosY);
			Vec3V nearHit_dummy;
			Vec3V farHit_dummy;
			if (m_focus.GetRayIntersection(origin, dir, nearHit_dummy, farHit_dummy))
				disable = true;
		}
		if (disable) {
			printf("disabling focus.\n");
			m_focus = Sphere3V::Invalid();
		} else {
			Vec3V pos(V_ZERO);
			if (GetWorldPosAtPixel(pos, m_currentMousePosX, m_currentMousePosY)) {
				m_focus = Sphere3V(pos, m_worldScale/16.0f);
				printf("enabling focus at %.4f,%.4f,%.4f (distance to camera %f)\n", VEC3V_ARGS(m_focus.GetCenter()), Mag(m_focus.GetCenter() - GetPosition()).f());
			}
		}
	}
	if (keyboard.IsKeyPressed('Z'))
		SetViewToWorld(m_viewToWorldDefault);
	const Vec3V forwardDir = GetForwardDirFromYawPitch(m_yaw, m_pitch, m_upDir);
	m_viewToWorld.GetMat33V_ref() = Mat33V::ConstructBasisUp(forwardDir, m_upDir);
}

#if !defined(_OPENGL)
#define GLUT_LEFT_BUTTON   0x0000
#define GLUT_MIDDLE_BUTTON 0x0001
#define GLUT_RIGHT_BUTTON  0x0002
#endif // !defined(_OPENGL)

void Camera::SetCurrentMousePos(int x, int y)
{
	m_currentMousePosX = x;
	m_currentMousePosY = y;
}

void Camera::BeginDrag(const Keyboard& keyboard, int x, int y, int button)
{
	m_currentMousePosX = x;
	m_currentMousePosY = y;
	if (All(m_upDir == Vec3V(V_ZERO)))
		return;
	const float tanVFOV = GetTanVFOV();
	if (button == GLUT_LEFT_BUTTON) {
		float z = GetPixelDepth(x, y);
		if (z <= m_znear || z >= m_zfar) {
			if (m_prevDragZ > 0.0f)
				z = m_prevDragZ;
			else
				z = m_worldScale;
		}
		m_prevDragZ = z;
		const bool dragForward = keyboard.IsControlDown();
		const bool dragForwardAnchorToWorld = dragForward && keyboard.IsShiftDown();
		if (dragForwardAnchorToWorld && GetWorldPosAtPixel(m_dragPanStart, x, y)) {
			m_dragPanNormal = -m_viewToWorld.b(); // <- set to +m_viewToWorld.c() to do viewspace XY panning with this codepath
			m_dragPanScreenX = m_dragPanScreenY = Vec3V(V_ZERO);
			m_dragPanScale = 0.0f;
			//printf("beginning %s drag (anchored) ..\n", dragForward ? "forward" : "XY");
		} else {
			m_dragPanNormal = m_dragPanStart = Vec3V(V_ZERO);
			m_dragPanScreenX = m_viewToWorld.a();
			m_dragPanScreenY = dragForward ? Normalize(m_viewToWorld.c()*(Vec3V(V_ONE) - m_upDir)) : m_viewToWorld.b();
			m_dragPanScale = z*2.0f*tanVFOV/(float)m_viewportHeight;
			//printf("beginning %s drag (z=%f) ..\n", dragForward ? "forward" : "XY", z);
		}
		m_dragAction = CAMERA_DRAG_ACTION_PAN;
	} else if (button == GLUT_RIGHT_BUTTON) {
		m_dragOrbitYaw = m_yaw;
		m_dragOrbitPitch = m_pitch;
		m_dragOrbitPitchEnabled = keyboard.IsControlDown();
		m_dragOrbitCenter = GetFocus();
		m_dragOrbitScale = atanf(2.0f*tanVFOV/(float)m_viewportHeight); // strange equation .. ?
		m_dragAction = CAMERA_DRAG_ACTION_ORBIT;
	}
	m_dragMousePosX = x;
	m_dragMousePosY = y;
	m_dragViewToWorld = m_viewToWorld;
}

void Camera::UpdateDrag(int x, int y)
{
	m_currentMousePosX = x;
	m_currentMousePosY = y;
	if (All(m_upDir == Vec3V(V_ZERO)))
		return;
	const float dx = (float)(x - m_dragMousePosX)*(CAMERA_FLIP_X ? -1.0f : 1.0f);
	const float dy = (float)(y - m_dragMousePosY);
	if (m_dragAction == CAMERA_DRAG_ACTION_PAN) {
		if (m_dragPanScale != 0.0f)
			m_viewToWorld.d_ref() = m_dragViewToWorld.d() - (m_dragPanScreenX*dx - m_dragPanScreenY*dy)*m_dragPanScale;
		else {
			// TODO -- clamp pixel coord x,y to horizon line (from m_dragMousePosX,Y)
			const Vec3V v = GetWorldDirAtPixel(x, y, 1.0f, m_dragViewToWorld); // world dir at new pixel based on start of drag
			const ScalarV t = Dot(m_dragPanStart - m_dragViewToWorld.d(), m_dragPanNormal)/Dot(v, m_dragPanNormal);
			if (t > 0.0f)
				m_viewToWorld.d_ref() = m_dragPanStart - t*v;
		}
		const Vec3V pan = m_viewToWorld.d() - m_dragViewToWorld.d();
		const float panDist = Mag(pan).f();
		const float panDistMax = m_worldScale*100.0f; // TODO -- config
		if (panDist > panDistMax)
			m_viewToWorld.d_ref() = m_dragViewToWorld.d() + Normalize(pan)*panDistMax;
	} else if (m_dragAction == CAMERA_DRAG_ACTION_ORBIT) {
		m_yaw = m_dragOrbitYaw - m_dragOrbitScale*dx;
		m_pitch = Clamp(m_dragOrbitPitch + m_dragOrbitScale*dy*(m_dragOrbitPitchEnabled ? 1.0f : 0.0f), -PI/2.1f, PI/2.1f); // clamp pitch so it can't go all the way to 90 degrees
		const Vec3V forwardDir = GetForwardDirFromYawPitch(m_yaw, m_pitch, m_upDir);
		const Mat33V basis = Mat33V::ConstructBasisUp(forwardDir, m_upDir);
		const Vec3V v0 = m_dragViewToWorld.TransformInvertOrtho(m_dragOrbitCenter); // could be precalculated
		const Vec3V v1 = basis.TransformTranspose(m_dragOrbitCenter);
		const Vec3V position = basis.Transform(v1 - v0); // adjust position so that the orbit center stays the same in viewspace
		m_viewToWorld = Mat34V(basis, position);
	}
}

void Camera::EndDrag(int x, int y)
{
	m_currentMousePosX = x;
	m_currentMousePosY = y;
	if (All(m_upDir == Vec3V(V_ZERO)))
		return;
	m_dragAction = CAMERA_DRAG_ACTION_NONE;
}

Vec3V_out Camera::GetForwardDirFromYawPitch(float yaw, float pitch, Vec3V_arg upDir)
{
	const float sinYaw = sinf(yaw);
	const float cosYaw = cosf(yaw);
	const float sinPitch = sinf(pitch);
	const float cosPitch = cosf(pitch);
	if (All(upDir == Vec3V(V_YAXIS)))
		return Vec3V(cosPitch*sinYaw, sinPitch, cosPitch*cosYaw);
	else {
		ForceAssert(All(upDir == Vec3V(V_ZAXIS)));
		return Vec3V(cosPitch*cosYaw, cosPitch*sinYaw, sinPitch);
	}
}

void Camera::GetYawPitchFromForwardDir(float& yaw, float& pitch, Vec3V_arg forwardDir, Vec3V_arg upDir)
{
	if (All(upDir == Vec3V(V_YAXIS))) {
		yaw = atan2f(forwardDir.xf(), forwardDir.zf());
		pitch = asinf(forwardDir.yf());
	} else {
		ForceAssert(All(upDir == Vec3V(V_ZAXIS)));
		yaw = atan2f(forwardDir.yf(), forwardDir.xf());
		pitch = asinf(forwardDir.zf());
	}
}

void Camera::SetYawPitchFromCurrentViewToWorld()
{
	GetYawPitchFromForwardDir(m_yaw, m_pitch, GetForwardDir(), m_upDir);
}

void Camera::SetViewToWorld(Mat34V_arg viewToWorld)
{
	m_viewToWorld = viewToWorld;
	SetYawPitchFromCurrentViewToWorld();
}

#if defined(_CPU_FRAMEBUFFER)
Pixel32& Camera::GetPixelColor_ref(int x, int y) const
{
	x = (x + m_framebufferScale/2)/m_framebufferScale; // round to nearest-ish
	y = (y + m_framebufferScale/2)/m_framebufferScale;
	return m_framebuffer.GetPixelColor_ref(x, y);
}

float& Camera::GetPixelDepth_ref(int x, int y) const
{
	x = (x + m_framebufferScale/2)/m_framebufferScale; // round to nearest-ish
	y = (y + m_framebufferScale/2)/m_framebufferScale;
	DEBUG_ASSERT(m_framebufferDepthValid);
	return m_framebuffer.GetPixelDepth_ref(x, y);
}
#endif // defined(_CPU_FRAMEBUFFER)

float Camera::GetPixelDepth(int x, int y) const
{
#if defined(_CPU_FRAMEBUFFER)
	if (m_framebufferDepthValid) {
		x = (x + m_framebufferScale/2)/m_framebufferScale; // round to nearest-ish
		y = (y + m_framebufferScale/2)/m_framebufferScale;
		return m_framebuffer.GetPixelDepth_ref(x, y);
	} else
#endif // defined(_CPU_FRAMEBUFFER)
	{
	#if defined(_OPENGL)
		float depth = 0.0f;
		glReadPixels(x, m_viewportHeight - 1 - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		return GetLinearZFromOpenGLDepth(depth);
	#else
		return 0.0f;
	#endif
	}
}

#if defined(_CPU_FRAMEBUFFER)
int Camera::GetPixelSelectID(int x, int y) const
{
	x = (x + m_framebufferScale/2)/m_framebufferScale; // round to nearest-ish
	y = (y + m_framebufferScale/2)/m_framebufferScale;
	return m_framebuffer.GetPixelSelectID(x, y);
}

Vec3V_out Camera::GetPixelSelectUV(int x, int y) const
{
	x = (x + m_framebufferScale/2)/m_framebufferScale; // round to nearest-ish
	y = (y + m_framebufferScale/2)/m_framebufferScale;
	return m_framebuffer.GetPixelSelectUV(x, y);
}
#endif // defined(_CPU_FRAMEBUFFER)

Vec3V_out Camera::GetWorldDirAtPixel(int x, int y, float z, Mat34V_arg viewToWorld) const
{
	const Vec2V screenUV(-1.0f + 2.0f*((float)x + 0.5f)/(float)m_viewportWidth, 1.0f - 2.0f*((float)y + 0.5f)/(float)m_viewportHeight); // [-1..1]
	const float tanVFOV = GetTanVFOV();
	const float tanHFOV = tanVFOV*GetAspect();
	return viewToWorld.TransformDir(Vec3V(screenUV*Vec2V(tanHFOV, tanVFOV), 1.0f)*z);
}

Vec3V_out Camera::GetWorldDirAtPixel(int x, int y, float z) const
{
	return GetWorldDirAtPixel(x, y, z, GetViewToWorld());
}

#if defined(_CPU_FRAMEBUFFER)
Vec3V_out Camera::GetWorldDirAtFramebufferPixel(int i, int j, float z) const
{
	i = i*m_framebufferScale + m_framebufferScale/2; // round to nearest-ish
	j = j*m_framebufferScale + m_framebufferScale/2;
	return GetWorldDirAtPixel(i, j, z);
}
#endif // defined(_CPU_FRAMEBUFFER)

bool Camera::GetWorldPosAtPixel(Vec3V& pos_out, int x, int y) const
{
	const float z = GetPixelDepth(x, y);
	if (z > 0.0f) {
		pos_out = GetPosition() + GetWorldDirAtPixel(x, y, z);
		return true;
	} else
		return false;
}

bool Camera::GetWorldPosUnderMouse(Vec3V& pos_out) const
{
	return GetWorldPosAtPixel(pos_out, m_currentMousePosX, m_currentMousePosY);
}

Vec3V_out Camera::GetWorldPlaneIntersectionAtPixel(int x, int y, const Plane3V& plane) const
{
	return plane.GetRayIntersection(GetPosition(), Normalize(GetWorldDirAtPixel(x, y)));
}

Vec2V_out Camera::GetScreenPos(Vec3V_arg worldPos) const
{
	const Vec3V viewPos = GetViewToWorld().TransformInvertOrtho(worldPos);
	const float tanVFOV = GetTanVFOV();
	const float tanHFOV = tanVFOV*GetAspect();
	return viewPos.xy()/(Vec2V(tanHFOV, -tanVFOV)*viewPos.z());
}

const Sphere3V& Camera::GetFocusSphere() const
{
	return m_focus;
}

Vec3V_out Camera::GetFocus() const
{
	if (m_focus.IsValid())
		return m_focus.GetCenter();
	else
		return GetPosition();
}

uint64 Camera::GetHash(uint64 hash) const
{
	const Mat34V viewToWorld = GetViewToWorld();
	hash = Crc64((const Vec3f&)viewToWorld.a_constref(), hash);
	hash = Crc64((const Vec3f&)viewToWorld.b_constref(), hash);
	hash = Crc64((const Vec3f&)viewToWorld.c_constref(), hash);
	hash = Crc64((const Vec3f&)viewToWorld.d_constref(), hash);
	hash = Crc64(m_VFOV, hash);
	hash = Crc64(m_znear, hash);
	hash = Crc64(m_zfar, hash);
	hash = Crc64(m_viewportWidth, hash);
	hash = Crc64(m_viewportHeight, hash);
	return hash;
}

#if defined(_OPENGL)
const Mat34V_out Camera::GetOpenGLModelViewMatrix() const
{
	Mat34V viewToWorld_OpenGL = m_viewToWorld;
	viewToWorld_OpenGL.c_ref() = -viewToWorld_OpenGL.c(); // OpenGL is weird ..
#if CAMERA_FLIP_X
	viewToWorld_OpenGL.a_ref() = -viewToWorld_OpenGL.a(); // .. and we don't want negative determinant
#endif // CAMERA_FLIP_X
	return InvertOrtho(viewToWorld_OpenGL);
}

const Mat44V_out Camera::GetOpenGLModelViewProjMatrix() const
{
	Mat44V proj;
	glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*)&proj);
	return proj*Mat44V(GetOpenGLModelViewMatrix());
}

void Camera::BeginDisplayOpenGL() const
{
	glViewport(0, 0, m_viewportWidth, m_viewportHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(GetVFOVInDegrees(), GetAspect(), GetZNear(), GetZFar());
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	const Mat44V worldToView44 = Mat44V(GetOpenGLModelViewMatrix());
	glLoadMatrixf((const float*)&worldToView44);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void Camera::EndDisplayOpenGL() const
{
	DrawFocusSphereOpenGL();
	glPopMatrix();
	glDepthMask(GL_TRUE); // needs to be true before glutSwapBuffers?
	glutSwapBuffers();
}

void Camera::DrawFocusSphereOpenGL() const
{
	if (GetFocusSphere().IsValid()) {
		const Vec3V v = GetFocus();
		glPushMatrix();
		glTranslatef(VEC3V_ARGS(v));
		glDepthMask(GL_FALSE);
		glColor3f(1.0f,1.0f,1.0f);
		glutWireSphere(GetFocusSphere().GetRadius().f(), 12, 16);
		glDepthMask(GL_TRUE);
		glPopMatrix();
	}
}

#if defined(_CPU_FRAMEBUFFER)
void Camera::CopyFramebufferOpenGL(bool forceAlpha255) 
{
	const uint32 w = m_framebuffer.m_w;
	const uint32 h = m_framebuffer.m_h;
	glFlush(); // needed before glReadPixels?
	glReadPixels(0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_framebuffer.m_color);
	glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, m_framebuffer.m_depth);
	FlipVertical(m_framebuffer.m_color, w, h);
	FlipVertical(m_framebuffer.m_depth, w, h);
	for (uint32 i = 0; i < m_framebuffer.m_w*m_framebuffer.m_h; i++) {
		float& z = m_framebuffer.m_depth[i];
		z = GetLinearZFromOpenGLDepth(z);
		if (forceAlpha255)
			m_framebuffer.m_color[i].a = 255;
	}
	m_framebufferDepthValid = true;
}
#endif // defined(_CPU_FRAMEBUFFER)
#endif // defined(_OPENGL)
#endif // PLATFORM_PC