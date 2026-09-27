#pragma once
#include "Component.h"
#include "Common.h"
using namespace DirectX;

class Transform : public Component
{
public:
	Transform(GameObject* owner);
	Transform(GameObject* owner, const XMFLOAT3& m_position, const XMFLOAT4& m_rotation, const XMFLOAT3& m_scale);

	~Transform() override;

	XMFLOAT3 GetPosition()const { return m_position; }
	XMFLOAT4 GetRotation()const { return m_rotation; }
	XMFLOAT3 GetScale()const { return m_scale; }

	void Start()override;
	void Update()override;
	void Render(Camera* camera)override;
	
private:
	XMFLOAT3 m_position;
	XMFLOAT4 m_rotation;
	XMFLOAT3 m_scale;
	
};

