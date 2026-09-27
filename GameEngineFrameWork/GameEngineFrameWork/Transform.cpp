#include "Transform.h"

Transform::Transform(GameObject* owner)
	:Component(owner),
	m_position(0.0f, 0.0f, 0.0f),
	m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
	m_scale(0.0f, 0.0f, 0.0f)
{
}
Transform::Transform(GameObject* owner, const XMFLOAT3& m_position, const XMFLOAT4& m_rotation, const XMFLOAT3& m_scale)
	:Component(owner), m_position(m_position), m_rotation(m_rotation), m_scale(m_scale)
{
}

Transform::~Transform()
{
}

void Transform::Start()
{
}

void Transform::Update()
{
}

void Transform::Render(Camera* camera)
{
	(void)camera;
}

