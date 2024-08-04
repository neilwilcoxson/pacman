#include "TimerService.hpp"

TimerService& TimerService::getInstance()
{
    static TimerService instance;
    return instance;
}

size_t TimerService::addTimer(uint64_t duration, bool autoRestart, std::function<void()> callback)
{
    size_t addKey = nextKey;
    ++nextKey;
    m_timers.insert({addKey, Timer(duration, autoRestart, callback)});
    return addKey;
}

void TimerService::startTimer(size_t key, uint64_t currentTicks)
{
    auto& timer = m_timers.at(key);
    timer.deadline = currentTicks + timer.duration;
    timer.isRunning = true;
}

void TimerService::pauseTimer(size_t key, uint64_t currentTicks)
{
    auto& timer = m_timers.at(key);
    timer.duration = timer.deadline - currentTicks;
    timer.isRunning = false;
}

void TimerService::stopTimer(size_t key)
{
    // erase has no effect if the key does not exist
    m_timers.erase(key);
}

void TimerService::checkTimers(uint64_t currentTicks)
{
    for(auto it = m_timers.begin(); it != m_timers.end();)
    {
        auto& [key, timer] = *it;

        if(timer.isRunning && currentTicks >= timer.deadline)
        {
            timer.callback();
            if(timer.autoRestart)
            {
                startTimer(key, currentTicks);
            }
            else
            {
                // erase invalidates the previous iterators, so get a new one
                it = m_timers.erase(it);
                continue;
            }
        }
        it++;
    }
}
