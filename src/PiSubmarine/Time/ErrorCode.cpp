#include "PiSubmarine/Time/ErrorCode.h"

#include <array>
#include <string_view>

namespace PiSubmarine::Time
{
    namespace
    {
        class ErrorCategory final : public std::error_category
        {
        public:
            [[nodiscard]] const char* name() const noexcept override
            {
                return "PiSubmarine.Time.Manager";
            }

            [[nodiscard]] std::string message(const int condition) const override
            {
                constexpr std::array<std::string_view, 5> Messages{
                    "success",
                    "manager is already running",
                    "tickable already added",
                    "tickable not found",
                    "invalid tick period"};

                const auto index = static_cast<std::size_t>(condition);
                if (index >= Messages.size())
                {
                    return "unknown time manager error";
                }

                return std::string(Messages[index]);
            }
        };
    }

    [[nodiscard]] std::error_code make_error_code(const ErrorCode errorCode) noexcept
    {
        static const ErrorCategory Category;
        return {static_cast<int>(errorCode), Category};
    }
}
