#pragma once
#include"Common.h"
using namespace DirectX;
class Camera
{
public:
	enum class ProjectionType {Perspective, Orthographics};
	enum class ClearFlags {SolidColor, DepthOnly, DontColor};

	Camera() = default;
	~Camera() = default;

	void Update();

	const XMMATRIX& GetViewMatrix() const { return m_viewMatrix; }
	const XMMATRIX& GetProjectionMatrix() const { return m_projectionMatrix; }
	ClearFlags GetClearFlags() const { return m_clearFlags; }
	XMFLOAT4 GetBackgroundColor() const { return m_backgroundColor; }
	XMFLOAT4 GetViewportRect() const { return m_viewportRect; }

	void SetPosition(XMVECTOR pos) { m_position = pos; }
	void SetLookAt(XMVECTOR target) { m_lookAt = target; }
	void SetUp(XMVECTOR up) { m_up = up; }

	void SetAspectRatio(float aspect);
	void SetFOV(float fov);
	void SetNearFar(float nearZ, float farZ);
	void SetClearFlags(ClearFlags clearFlags) { m_clearFlags = clearFlags; }
	void SetBackgroundColor(const XMFLOAT4& color) { m_backgroundColor = color; }
	void SetViewRect(XMFLOAT4& rect) { m_viewportRect = rect; }


private:
	void RecalculateProjection();

	XMMATRIX m_viewMatrix = XMMatrixIdentity();
	XMMATRIX m_projectionMatrix = XMMatrixIdentity();
	bool m_projDirty = true;

	XMVECTOR m_position = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
	XMVECTOR m_lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	ProjectionType m_projectionType = ProjectionType::Perspective;
	float m_fovY = 60.0f;
	float m_aspect = 16.0f / 9.0f;
	float m_nearZ = 0.1f;
	float m_farZ = 1000.0f;

	ClearFlags m_clearFlags = ClearFlags::SolidColor;
	XMFLOAT4 m_backgroundColor = { 0.0f, 0.2f, 0.4f, 1.0f };
	XMFLOAT4 m_viewportRect = { 0.0f, 0.0f, 1.0f, 1.0f };

};

