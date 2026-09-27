#pragma once
#include<memory>
#include<string>
#include<typeindex>
#include<unordered_map>
#include<vector>
#include"Component.h"

class Camera;
class Transform;

class GameObject
{
public:
	explicit GameObject(const std::string& name = "GameObject");
	~GameObject() = default;

	void Start();
	void Update();
	void Render(Camera* camera);

	const std::string& GetName()const { return m_name; }

	template<typename T, typename ...Args>
	T* AddComponent(Args&&... args);

	template<typename T>
	T* GetComponent() const;

private:
	std::string m_name;
	std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> m_components;
		
};

template<typename T, typename ...Args>
T* GameObject::AddComponent(Args&&... args)
{
	auto comp = std::make_unique<T>(this, std::forward(args)...);
	T* raw = comp.get();
	m_components[typeid(T)].push_back(std::move(comp));

	return raw;
}

template<typename T>
T* GameObject::GetComponent() const
{
	for (const auto& pair : m_components)
	{
		for (const auto& comp : pair.second)
		{
			if (T* casted = dynamic_cast<T*>(comp.get())) return casted;
		}
	}
	return nullptr;
}