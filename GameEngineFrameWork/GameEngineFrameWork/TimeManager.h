#pragma once
#include<Windows.h>
#include"Singleton.h"

class TimeManager : public Singleton<TimeManager>
{
	friend class Singleton<TimeManager>;

public:
	void Initialize();
	void Update();

	float GetDeltaTime() const { return m_deltaTime; }
	float GetTotalTime() const { return m_totalTime; }
	float GetFPS()const { return (m_deltaTime > 0.0f) ? (1.0f / m_deltaTime) : 0.0f; }
private:
	TimeManager();
	~TimeManager() override;


	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_prevCount;

	float m_deltaTime;
	float m_totalTime;
};

