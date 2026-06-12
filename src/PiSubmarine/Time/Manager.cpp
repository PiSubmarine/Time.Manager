#include "PiSubmarine/Time/Manager.h"

#include <algorithm>
#include <thread>

#include "PiSubmarine/Error/Api/ErrorCondition.h"
#include "PiSubmarine/Error/Api/MakeError.h"

namespace PiSubmarine::Time
{
    Manager::Manager(
        const std::chrono::nanoseconds tickPeriod,
        Clock clock,
        SleepUntil sleepUntil)
        : m_TickPeriod(tickPeriod)
        , m_Clock(std::move(clock))
        , m_SleepUntil(std::move(sleepUntil))
    {
    }

    Error::Api::Result<void> Manager::AddTickable(ITickable& tickable)
    {
        if (m_IsRunning)
        {
            return std::unexpected(MakeError(ErrorCode::AlreadyRunning));
        }

        if (std::ranges::find(m_Tickables, &tickable) != m_Tickables.end())
        {
            return std::unexpected(MakeError(ErrorCode::TickableAlreadyAdded));
        }

        m_Tickables.push_back(&tickable);
        return {};
    }

    Error::Api::Result<void> Manager::RemoveTickable(ITickable& tickable)
    {
        if (m_IsRunning)
        {
            return std::unexpected(MakeError(ErrorCode::AlreadyRunning));
        }

        const auto iterator = std::ranges::find(m_Tickables, &tickable);
        if (iterator == m_Tickables.end())
        {
            return std::unexpected(MakeError(ErrorCode::TickableNotFound));
        }

        m_Tickables.erase(iterator);
        return {};
    }

    Error::Api::Result<void> Manager::Run()
    {
        if (m_IsRunning)
        {
            return std::unexpected(MakeError(ErrorCode::AlreadyRunning));
        }

        if (m_TickPeriod <= std::chrono::nanoseconds::zero())
        {
            return std::unexpected(MakeError(ErrorCode::InvalidTickPeriod));
        }

        m_IsRunning = true;
        m_ShouldRun = true;

        const auto startTime = m_Clock();
        auto lastTickTime = startTime;
        auto nextDeadline = startTime;

        while (m_ShouldRun)
        {
            const auto currentTime = m_Clock();
            const auto uptime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - startTime);
            const auto deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTickTime);

            for (auto* const tickable : m_Tickables)
            {
                tickable->Tick(uptime, deltaTime);
            }

            lastTickTime = currentTime;
            if (!m_ShouldRun)
            {
                break;
            }

            nextDeadline += m_TickPeriod;
            m_SleepUntil(nextDeadline);
        }

        m_IsRunning = false;
        return {};
    }

    void Manager::Stop() noexcept
    {
        m_ShouldRun = false;
    }

    bool Manager::IsRunning() const noexcept
    {
        return m_IsRunning;
    }

    Error::Api::Error Manager::MakeError(const ErrorCode code) noexcept
    {
        return Error::Api::MakeError(Error::Api::ErrorCondition::ContractError, make_error_code(code));
    }
}
