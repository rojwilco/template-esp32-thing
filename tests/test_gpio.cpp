#include <gtest/gtest.h>
#include "sketch_api.h"

class GpioTest : public ::testing::Test {
protected:
    void SetUp() override {
        resetMocks();
        cfg_gpio_pin           = 2;
        cfg_toggle_interval_ms = 1000;
        cfg_poll_interval_min  = 1;
        g_forceRerun           = false;
        g_mock_pin_value[cfg_gpio_pin] = LOW;
    }
};

TEST_F(GpioTest, RunTaskTogglesGpioPin) {
    int before = g_mock_pin_value[cfg_gpio_pin];
    runTask();
    EXPECT_NE(before, g_mock_pin_value[cfg_gpio_pin]);
}

TEST_F(GpioTest, RunTaskFirstCallGoesHigh) {
    g_mock_pin_value[cfg_gpio_pin] = LOW;
    runTask();
    EXPECT_EQ(HIGH, g_mock_pin_value[cfg_gpio_pin]);
}

TEST_F(GpioTest, RunTaskSecondCallGoesLow) {
    runTask(); // HIGH
    runTask(); // LOW
    EXPECT_EQ(LOW, g_mock_pin_value[cfg_gpio_pin]);
}

TEST_F(GpioTest, RunTaskRespectsConfiguredPin) {
    cfg_gpio_pin = 5;
    g_mock_pin_value[5] = LOW;
    runTask();
    EXPECT_EQ(HIGH, g_mock_pin_value[5]);
}

TEST_F(GpioTest, RunTaskDoesNotAffectOtherPins) {
    cfg_gpio_pin = 2;
    g_mock_pin_value[5] = LOW;
    runTask();
    EXPECT_EQ(LOW, g_mock_pin_value[5]);
}

TEST_F(GpioTest, RunTaskAlternatesAcrossMultipleCalls) {
    int states[4];
    for(int i=0;i<4;i++){
        runTask();
        states[i] = g_mock_pin_value[cfg_gpio_pin];
    }
    EXPECT_EQ(HIGH, states[0]);
    EXPECT_EQ(LOW,  states[1]);
    EXPECT_EQ(HIGH, states[2]);
    EXPECT_EQ(LOW,  states[3]);
}

TEST_F(GpioTest, RunTaskClearsForceRerunFlag) {
    g_forceRerun = true;
    runTask();
    EXPECT_FALSE(g_forceRerun);
}

TEST_F(GpioTest, ForceRerunClearedOnEveryCall) {
    g_forceRerun = true; runTask(); EXPECT_FALSE(g_forceRerun);
    g_forceRerun = true; runTask(); EXPECT_FALSE(g_forceRerun);
}
