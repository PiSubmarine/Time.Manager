#pragma once

#include <system_error>

namespace PiSubmarine::Time
{
    enum class ErrorCode
    {
        AlreadyRunning = 1,
        TickableAlreadyAdded,
        TickableNotFound,
        InvalidTickPeriod
    };

    [[nodiscard]] std::error_code make_error_code(ErrorCode errorCode) noexcept;
}

namespace std
{
    template<>
    struct is_error_code_enum<PiSubmarine::Time::ErrorCode> : true_type
    {
    };
}
