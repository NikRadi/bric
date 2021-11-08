#ifndef BRIC_TIMER_HPP
#define BRIC_TIMER_HPP
#include <chrono>


class Timer {
public:
    void Start() {
        m_start_time = std::chrono::high_resolution_clock::now();
    }

    void Stop() {
        m_total_milliseconds += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start_time).count();
    }

    long long ElapsedMilliseconds() {
        return m_total_milliseconds;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
    long long m_total_milliseconds = 0;

};

#endif // BRIC_TIMER_HPP
