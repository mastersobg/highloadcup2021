#include <gtest/gtest.h>
#include "state.h"
#include <memory>

TEST(StateTest, TestLicenses) {
    auto state = std::make_shared<State>();
    ASSERT_FALSE(state->hasAvailableLicense());
    state->addLicence(License(1, 3, 0));
    ASSERT_TRUE(state->hasAvailableLicense());
    ASSERT_EQ(1, state->reserveAvailableLicenseId().get());
    ASSERT_EQ(1, state->reserveAvailableLicenseId().get());
    ASSERT_EQ(1, state->reserveAvailableLicenseId().get());
    ASSERT_TRUE(state->reserveAvailableLicenseId().hasError());
    ASSERT_FALSE(state->hasAvailableLicense());
}
