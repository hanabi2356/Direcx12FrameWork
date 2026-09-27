#pragma once

class GameObject;
class Camera;

class Component
{
public:
	explicit Component(GameObject* owner) : m_owner(owner){}
	virtual ~Component() = default;

	virtual void Start() {}
	virtual void Update() {}
	virtual void Render(Camera* camera){}

	GameObject* GetOwner()const { return m_owner; }

protected:
	GameObject* m_owner = nullptr;
};

