#pragma once

#include <functional>
#include <unordered_map>
#include <SDL.h>

class TimerService
{
public:
    static TimerService& getInstance();
    size_t addTimer(uint64_t duration, bool autoRestart, std::function<void()> callback);
    void startTimer(size_t key, uint64_t currentTicks = SDL_GetTicks64());
    void pauseTimer(size_t key, uint64_t currentTicks = SDL_GetTicks64());
    void stopTimer(size_t key);
    void checkTimers(uint64_t currentTicks = SDL_GetTicks64());

private:
    struct Timer
    {
        uint64_t deadline = 0;
        uint64_t duration;
        std::function<void()> callback;
        bool autoRestart;
        bool isRunning = false;

        Timer(uint64_t duration, bool autoRestart, std::function<void()>& callback)
        : duration(duration), autoRestart(autoRestart), callback(callback)
        {
        }
    };
    std::unordered_map<size_t, Timer> m_timers;
    size_t nextKey = 0;

    TimerService() = default;
    TimerService(TimerService&) = delete;
    TimerService(TimerService&&) = delete;
    TimerService& operator=(TimerService&) = delete;
    TimerService& operator=(TimerService&&) = delete;
};