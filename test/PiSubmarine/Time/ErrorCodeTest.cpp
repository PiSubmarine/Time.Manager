#include <gtest/gtest.h>

#include "PiSubmarine/Time/ErrorCode.h"

namespace PiSubmarine::Time
{
    TEST(ErrorCodeTest, ConvertsToErrorCode)
    {
        const auto errorCode = make_error_code(ErrorCode::InvalidTickPeriod);

        EXPECT_EQ(errorCode.value(), static_cast<int>(ErrorCode::InvalidTickPeriod));
        EXPECT_STREQ(errorCode.category().name(), "PiSubmarine.Time.Manager");
    }
}
