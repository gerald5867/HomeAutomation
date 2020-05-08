#pragma once
#include <chrono>
namespace utils {
	template<typename TimeType = std::chrono::seconds>
	class TimeOutClock {
	public:
		using ClockT = std::chrono::system_clock;
		explicit TimeOutClock(const TimeType& timeOut) {
			m_start = ClockT::now();
			SetTimeOut(timeOut);
		}
		bool IsExpired() const noexcept {
			return std::chrono::duration_cast<TimeType>(ClockT::now() - m_start) >= m_timeOut;
		}
		void Reset() {
			m_start = ClockT::now();
		}
		void SetTimeOut(const TimeType& timeOut) {
			m_timeOut = timeOut;
		}
	private:
		TimeType m_timeOut;
		ClockT::time_point m_start;
	};
	using TimeOutClockSeconds = TimeOutClock<std::chrono::seconds>;
	using TimeOutClockMinutes = TimeOutClock<std::chrono::minutes>;
}