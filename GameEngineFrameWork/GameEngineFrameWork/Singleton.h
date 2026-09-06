#pragma once
#include<Windows.h>
#include<typeinfo>

template<typename T>
class Singleton
{
protected:
	Singleton() = default;
	virtual ~Singleton() = default;

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

private:
	static T* instance;
public:
	static T* GetInstance()
	{
		if (!instance)
		{
			instance = new T();
			static struct Deleter {
				~Deleter() {
					Singleton<T>::DestroyManager();
				}
			} deleter;
		}
		return instance;
	}

	static void DestroyManager()
	{
		if (instance)
		{
			auto& id = typeid(*instance);

			delete instance;
			instance = nullptr;
		}
	}
};

template <typename T>
T* Singleton<T>::instance = nullptr;