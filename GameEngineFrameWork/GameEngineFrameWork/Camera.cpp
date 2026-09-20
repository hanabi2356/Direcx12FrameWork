#include "Camera.h"



void Camera::Update()
{
	m_viewMatrix = XMMatrixLookAtLH(m_position, m_lookAt, m_up);

	if (m_projDirty) //Dirty Flag Àû¿ë
	{
		RecalculateProjection();
		m_projDirty = false;
	}
}

void Camera::SetAspectRatio(float aspect)
{
	m_aspect = aspect;
	m_projDirty = true;
}

void Camera::SetFOV(float fov)
{
	m_fovY = fov;
	m_projDirty = true;
}

void Camera::SetNearFar(float nearZ, float farZ)
{
	m_nearZ = nearZ;
	m_farZ = farZ;
	m_projDirty;
}

void Camera::RecalculateProjection()
{
	if (m_projectionType == ProjectionType::Perspective)
	{
		float fov = m_fovY * (XM_PI / 180.0f);
		m_projectionMatrix = XMMatrixPerspectiveFovLH(fov, m_aspect, m_nearZ, m_farZ);
	}
	else
	{
		float h = 5.0f;
		float w = h * m_aspect;
		m_projectionMatrix = XMMatrixOrthographicLH(w, h, m_nearZ, m_farZ);
	}
}
