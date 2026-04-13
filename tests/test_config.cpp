#include <gtest/gtest.h>
#include "sketch_api.h"

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        resetMocks();
        cfg_wifi_ssid[0] = '\0';
        cfg_wifi_pass[0] = '\0';
        cfg_gpio_pin           = 2;
        cfg_toggle_interval_ms = 1000;
        cfg_poll_interval_min  = 1;
        g_pendingConnect = false;
        g_forceRerun     = false;
    }
};

// --- Defaults applied when NVS empty ---

TEST_F(ConfigTest, DefaultsAppliedWhenNvsEmpty) {
    loadConfig();
    EXPECT_STREQ("", cfg_wifi_ssid);
    EXPECT_STREQ("", cfg_wifi_pass);
    EXPECT_EQ(2,     cfg_gpio_pin);
    EXPECT_EQ(1000u, cfg_toggle_interval_ms);
    EXPECT_EQ(1,     cfg_poll_interval_min);
}

// --- Round-trip persistence ---

TEST_F(ConfigTest, RoundTripSsid) {
    strncpy(cfg_wifi_ssid, "MyNetwork", sizeof(cfg_wifi_ssid));
    saveConfig();
    cfg_wifi_ssid[0] = '\0';
    loadConfig();
    EXPECT_STREQ("MyNetwork", cfg_wifi_ssid);
}

TEST_F(ConfigTest, RoundTripPass) {
    strncpy(cfg_wifi_pass, "s3cr3t!", sizeof(cfg_wifi_pass));
    saveConfig();
    cfg_wifi_pass[0] = '\0';
    loadConfig();
    EXPECT_STREQ("s3cr3t!", cfg_wifi_pass);
}

TEST_F(ConfigTest, RoundTripGpioPin) {
    cfg_gpio_pin = 13; saveConfig(); cfg_gpio_pin = 0; loadConfig();
    EXPECT_EQ(13, cfg_gpio_pin);
}

TEST_F(ConfigTest, RoundTripToggleInterval) {
    cfg_toggle_interval_ms = 5000; saveConfig(); cfg_toggle_interval_ms = 0; loadConfig();
    EXPECT_EQ(5000u, cfg_toggle_interval_ms);
}

TEST_F(ConfigTest, RoundTripPollInterval) {
    cfg_poll_interval_min = 60; saveConfig(); cfg_poll_interval_min = 0; loadConfig();
    EXPECT_EQ(60, cfg_poll_interval_min);
}

TEST_F(ConfigTest, MultipleRoundTripsRetainLastValue) {
    cfg_gpio_pin = 4; saveConfig();
    cfg_gpio_pin = 8; saveConfig();
    cfg_gpio_pin = 0; loadConfig();
    EXPECT_EQ(8, cfg_gpio_pin);
}

// --- Clamping ---

TEST_F(ConfigTest, GpioPinClampsAbove39) {
    g_mock_prefs_store["esp32thing/gpio_pin"] = "200";
    loadConfig();
    EXPECT_EQ(2, cfg_gpio_pin);
}

TEST_F(ConfigTest, ToggleIntervalClampsBelow100) {
    g_mock_prefs_store["esp32thing/toggle_ms"] = "50";
    loadConfig();
    EXPECT_EQ(100u, cfg_toggle_interval_ms);
}

TEST_F(ConfigTest, PollIntervalClampsBelow1) {
    g_mock_prefs_store["esp32thing/poll_min"] = "0";
    loadConfig();
    EXPECT_EQ(1, cfg_poll_interval_min);
}

TEST_F(ConfigTest, PollIntervalClampsAbove1440) {
    g_mock_prefs_store["esp32thing/poll_min"] = "9999";
    loadConfig();
    EXPECT_EQ(1440, cfg_poll_interval_min);
}

// --- String truncation ---

TEST_F(ConfigTest, SsidTruncatesAtMaxLength) {
    g_mock_prefs_store["esp32thing/wifi_ssid"] = std::string(70, 'A');
    loadConfig();
    EXPECT_EQ(63u, strlen(cfg_wifi_ssid));
}

TEST_F(ConfigTest, PassTruncatesAtMaxLength) {
    g_mock_prefs_store["esp32thing/wifi_pass"] = std::string(70, 'B');
    loadConfig();
    EXPECT_EQ(63u, strlen(cfg_wifi_pass));
}
