#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <vector>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Time/ErrorCode.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Time
{
    class Manager
    {
    public:
        using TimePoint = std::chrono::steady_clock::time_point;
        using Clock = std::function<TimePoint()>;
        using SleepUntil = std::function<void(const TimePoint&)>;

        explicit Manager(
            std::chrono::nanoseconds tickPeriod,
            Clock clock = [] { return std::chrono::steady_clock::now(); },
            SleepUntil sleepUntil = [](const TimePoint& deadline) { std::this_thread::sleep_until(deadline); });

        [[nodiscard]] Error::Api::Result<void> AddTickable(ITickable& tickable);
        [[nodiscard]] Error::Api::Result<void> RemoveTickable(ITickable& tickable);
        [[nodiscard]] Error::Api::Result<void> Run();
        void Stop() noexcept;
        [[nodiscard]] bool IsRunning() const noexcept;

    private:
        [[nodiscard]] static Error::Api::Error MakeError(ErrorCode code) noexcept;

        std::chrono::nanoseconds m_TickPeriod;
        Clock m_Clock;
        SleepUntil m_SleepUntil;
        std::vector<ITickable*> m_Tickables;
        std::atomic_bool m_ShouldRun = false;
        bool m_IsRunning = false;
    };
}
