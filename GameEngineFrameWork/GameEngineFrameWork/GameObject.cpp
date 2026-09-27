#include "GameObject.h"
GameObject::GameObject(const std::string& name)
	:m_name(name)
{

}
void GameObject::Start()
{
	for (auto& pair : m_components)
	{
		for (auto& comp : pair.second)
		{
			comp->Start();
		}
	}
}
void GameObject::Update()
{
	for (auto& pair : m_components)
	{
		for (auto& comp : pair.second)
		{
			comp->Update();
		}
	}
}
void GameObject::Render(Camera* camera)
{
	for (auto& pair : m_components)
	{
		for (auto& comp : pair.second)
		{
			comp->Render(camera);
		}
	}
}


