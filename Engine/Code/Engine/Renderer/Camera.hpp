#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"


class Camera
{
public:
	enum Mode
	{
		eMode_Orthographic,
		eMode_Perspective,
		eMode_Count
	};

	Camera();

	void SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.f, float far = 1.f);
	void SetPerspectiveView(float aspect, float fov, float near, float far);

	void SetPositionAndOrientation(const Vec3& position, const EulerAngles& orientation);
	void SetPosition(const Vec3& position);
	Vec3 GetPosition() const;
	void SetOrientation(const EulerAngles& orientation);
	EulerAngles GetOrientation() const;

	Mat44 GetCameraToWorldTransform() const;
	Mat44 GetWorldToCameraTransform() const;

	void SetCameraToRenderTransform(const Mat44& m);
	Mat44 GetCameraToRenderTransorm() const;

	Mat44 GetRenderToClipTransform() const;

	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	void Translate2DFromPosition(const Vec2& translation2D, Vec2 oriPos_lb, Vec2 oriPos_tr);

	Mat44 GetOrthographicMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;

protected:
	Mode m_mode = eMode_Orthographic;

	Vec3 m_position;
	EulerAngles m_orientation;

	Vec2 m_orthographicBottomLeft;
	Vec2 m_orthograhicTopRight;
	float m_orthoraphicNear;
	float m_orthographicFar;

	float m_perspectiveAspect;
	float m_perspectiveFOV;
	float m_perspectiveNear;
	float m_perspectiveFar;

	Mat44 m_cameraToRenderTransform;
};