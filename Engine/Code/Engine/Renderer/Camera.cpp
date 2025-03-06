
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec3.hpp"


Camera::Camera()
{
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_orthographicBottomLeft;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_orthograhicTopRight;
}

void Camera::Translate2DFromPosition(const Vec2& translation2D, Vec2 oriPos_bl, Vec2 oriPos_tr)
{
	m_orthographicBottomLeft = oriPos_bl+translation2D;
	m_orthograhicTopRight = oriPos_tr+translation2D;
}

void Camera::SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near, float far)
{
	m_mode = Mode::eMode_Orthographic;
	m_orthographicBottomLeft = bottomLeft;
	m_orthograhicTopRight = topRight;
	m_orthoraphicNear = near;
	m_orthographicFar = far;
}

void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
	m_mode = Mode::eMode_Perspective;
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = near;
	m_perspectiveFar = far;
}

void Camera::SetPositionAndOrientation(const Vec3& position, const EulerAngles& orientation)
{
	m_position = position;
	m_orientation = orientation;
}

void Camera::SetPosition(const Vec3& position)
{
	m_position = position;
}

Vec3 Camera::GetPosition() const
{
	return m_position;
}

void Camera::SetOrientation(const EulerAngles& orientation)
{
	m_orientation = orientation;
}

EulerAngles Camera::GetOrientation() const
{
	return m_orientation;
}

Mat44 Camera::GetCameraToWorldTransform() const
{
	//Mat44 camToWorld = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	//camToWorld = camToWorld.GetOrthonormalInverse();
	//camToWorld.Append(Mat44::MakeTranslation3D(m_position));
	Mat44 camToWorld = Mat44::MakeTranslation3D(m_position);
	camToWorld.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());

	
	return camToWorld;
}

Mat44 Camera::GetWorldToCameraTransform() const
{
	return GetCameraToWorldTransform().GetOrthonormalInverse();
}

void Camera::SetCameraToRenderTransform(const Mat44& m)
{
	m_cameraToRenderTransform = m;
}

Mat44 Camera::GetCameraToRenderTransorm() const
{
	//return Mat44(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f));
	return m_cameraToRenderTransform;
}

Mat44 Camera::GetRenderToClipTransform() const
{
	if (m_mode == Mode::eMode_Perspective)
	{
		return GetProjectionMatrix();
	}
	else  
	{
		return GetOrthographicMatrix();
	}
}


Mat44 Camera::GetOrthographicMatrix() const
{
	return Mat44::MakeOrthoProjection(m_orthographicBottomLeft.x, m_orthograhicTopRight.x,
		m_orthographicBottomLeft.y, m_orthograhicTopRight.y,
		m_orthoraphicNear, m_orthographicFar);
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::MakePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
}

Mat44 Camera::GetProjectionMatrix() const
{
	if (m_mode == Mode::eMode_Orthographic)
	{
		return Mat44::MakeOrthoProjection(m_orthographicBottomLeft.x, m_orthograhicTopRight.x,
			m_orthographicBottomLeft.y, m_orthograhicTopRight.y,
			m_orthoraphicNear, m_orthographicFar);
	}
	else 
	{
		return Mat44::MakePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
	}
}

