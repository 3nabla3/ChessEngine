#include <utility>

#pragma once

typedef std::chrono::microseconds time_unit;

class Timer {
public:
	explicit Timer(std::string name):
		m_Name(std::move(name)),
		m_Start(std::chrono::steady_clock::now())
	{}

	~Timer() {
		Stop();
	}

	void Pause() {
		auto end = std::chrono::steady_clock::now();
		auto us = std::chrono::duration_cast<time_unit>(end - m_Start);
		s_Duration[m_Name].first += us.count();
	}

	void Stop() {
		auto end = std::chrono::steady_clock::now();
		auto us = std::chrono::duration_cast<time_unit>(end - m_Start);
		s_Duration[m_Name].first += us.count();
		s_Duration[m_Name].second++;
	}

	static void PrintDurations() {
		for (auto& [name, data]: s_Duration) {
			std::cout << name << " " << data.second << "[ct] " <<
			(int)(data.first / 1000) << "[ms]" << std::endl;
		}
	}
private:
	std::string m_Name;
	std::chrono::time_point<std::chrono::steady_clock> m_Start;

	// name to duration and call count
	static std::unordered_map<std::string, std::pair<long, int>> s_Duration;
};

# if PROFILE == 1
	#define PROFILE_SCOPE_NAME(name) Timer timer##__LINE__(name)
	#define PROFILE_SCOPE Timer timer##__LINE__(__FUNCTION__)
#else
	#define PROFILE_SCOPE_NAME(name)
	#define PROFILE_SCOPE
#endif

