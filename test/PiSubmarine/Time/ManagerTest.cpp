#include <vector>

#include <gtest/gtest.h>

#include "PiSubmarine/Time/Manager.h"

namespace PiSubmarine::Time
{
    namespace
    {
        class RecordingTickable final : public ITickable
        {
        public:
            explicit RecordingTickable(std::function<void(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)> onTick = {})
                : m_OnTick(std::move(onTick))
            {
            }

            void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override
            {
                UptimeCalls.push_back(uptime);
                DeltaCalls.push_back(deltaTime);

                if (m_OnTick)
                {
                    m_OnTick(uptime, deltaTime);
                }
            }

            std::vector<std::chrono::nanoseconds> UptimeCalls;
            std::vector<std::chrono::nanoseconds> DeltaCalls;

        private:
            std::function<void(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)> m_OnTick;
        };
    }

    TEST(ManagerTest, RejectsDuplicateTickable)
    {
        Manager manager(std::chrono::milliseconds(10));
        RecordingTickable tickable;

        ASSERT_TRUE(manager.AddTickable(tickable).has_value());

        const auto result = manager.AddTickable(tickable);
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().Cause, make_error_code(ErrorCode::TickableAlreadyAdded));
    }

    TEST(ManagerTest, RejectsRemovingUnknownTickable)
    {
        Manager manager(std::chrono::milliseconds(10));
        RecordingTickable tickable;

        const auto result = manager.RemoveTickable(tickable);
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().Cause, make_error_code(ErrorCode::TickableNotFound));
    }

    TEST(ManagerTest, RunTicksAllTickablesWithFixedSchedule)
    {
        auto currentTime = Manager::TimePoint{};

        Manager* managerPointer = nullptr;
        RecordingTickable firstTickable([&](const auto&, const auto&)
        {
            if (firstTickable.UptimeCalls.size() == 3)
            {
                managerPointer->Stop();
            }
        });
        RecordingTickable secondTickable;

        Manager manager(
            std::chrono::milliseconds(1),
            [&] { return currentTime; },
            [&](const Manager::TimePoint& deadline) { currentTime = deadline; });
        managerPointer = &manager;

        ASSERT_TRUE(manager.AddTickable(firstTickable).has_value());
        ASSERT_TRUE(manager.AddTickable(secondTickable).has_value());

        const auto runResult = manager.Run();
        ASSERT_TRUE(runResult.has_value());

        const std::vector expectedUptime{
            std::chrono::nanoseconds{0},
            std::chrono::milliseconds{1},
            std::chrono::milliseconds{2}};
        const std::vector expectedDelta{
            std::chrono::nanoseconds{0},
            std::chrono::milliseconds{1},
            std::chrono::milliseconds{1}};

        EXPECT_EQ(firstTickable.UptimeCalls, expectedUptime);
        EXPECT_EQ(firstTickable.DeltaCalls, expectedDelta);
        EXPECT_EQ(secondTickable.UptimeCalls, expectedUptime);
        EXPECT_EQ(secondTickable.DeltaCalls, expectedDelta);
        EXPECT_FALSE(manager.IsRunning());
    }

    TEST(ManagerTest, RemoveTickablePreventsFurtherTicks)
    {
        auto currentTime = Manager::TimePoint{};

        Manager* managerPointer = nullptr;
        RecordingTickable removedTickable;
        RecordingTickable activeTickable([&](const auto&, const auto&)
        {
            if (activeTickable.UptimeCalls.size() == 2)
            {
                managerPointer->Stop();
            }
        });

        Manager manager(
            std::chrono::milliseconds(1),
            [&] { return currentTime; },
            [&](const Manager::TimePoint& deadline) { currentTime = deadline; });
        managerPointer = &manager;

        ASSERT_TRUE(manager.AddTickable(removedTickable).has_value());
        ASSERT_TRUE(manager.AddTickable(activeTickable).has_value());
        ASSERT_TRUE(manager.RemoveTickable(removedTickable).has_value());

        const auto runResult = manager.Run();
        ASSERT_TRUE(runResult.has_value());

        EXPECT_TRUE(removedTickable.UptimeCalls.empty());
        EXPECT_EQ(activeTickable.UptimeCalls.size(), 2);
    }
}
