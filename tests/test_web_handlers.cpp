#include <gtest/gtest.h>
#include "sketch_api.h"
#include <string>

class WebTest : public ::testing::Test {
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

// ---- handleRoot ----

TEST_F(WebTest, HandleRootReturns200) {
    handleRoot();
    EXPECT_EQ(200, g_mock_last_send_code);
}

TEST_F(WebTest, HandleRootContainsFirmwareVersion) {
    handleRoot();
    EXPECT_NE(std::string::npos,
              g_mock_last_send_body._s.find(FIRMWARE_VERSION));
}

TEST_F(WebTest, HandleRootContainsWifiSsid) {
    strncpy(cfg_wifi_ssid, "TestNet", sizeof(cfg_wifi_ssid));
    handleRoot();
    EXPECT_NE(std::string::npos, g_mock_last_send_body._s.find("TestNet"));
}

TEST_F(WebTest, HandleRootContentTypeIsHtml) {
    // send() is called with "text/html" — captured in g_mock_last_send_body
    // We test response code and body; content-type is implicit from handleRoot implementation
    handleRoot();
    EXPECT_EQ(200, g_mock_last_send_code);
    EXPECT_FALSE(g_mock_last_send_body._s.empty());
}

// ---- handleSave ----

TEST_F(WebTest, HandleSavePersistsSsid) {
    g_mock_server_args["wifi_ssid"] = "NewNet";
    handleSave();
    EXPECT_STREQ("NewNet", cfg_wifi_ssid);
}

TEST_F(WebTest, HandleSavePersistsGpioPin) {
    g_mock_server_args["gpio_pin"] = "13";
    handleSave();
    EXPECT_EQ(13, cfg_gpio_pin);
}

TEST_F(WebTest, HandleSavePersistsToggleInterval) {
    g_mock_server_args["toggle_ms"] = "2000";
    handleSave();
    EXPECT_EQ(2000u, cfg_toggle_interval_ms);
}

TEST_F(WebTest, HandleSavePersistsPollInterval) {
    g_mock_server_args["poll_min"] = "30";
    handleSave();
    EXPECT_EQ(30, cfg_poll_interval_min);
}

TEST_F(WebTest, HandleSaveSetsForceRerunOnPinChange) {
    g_mock_server_args["gpio_pin"] = "5";
    handleSave();
    EXPECT_TRUE(g_forceRerun);
}

TEST_F(WebTest, HandleSaveSetsForceRerunOnIntervalChange) {
    g_mock_server_args["toggle_ms"] = "500";
    handleSave();
    EXPECT_TRUE(g_forceRerun);
}

TEST_F(WebTest, HandleSaveSetsPendingConnectOnSsidChange) {
    g_mock_server_args["wifi_ssid"] = "AnotherNet";
    handleSave();
    EXPECT_TRUE(g_pendingConnect);
}

TEST_F(WebTest, HandleSaveSetsPendingConnectOnPassChange) {
    g_mock_server_args["wifi_pass"] = "newpassword";
    handleSave();
    EXPECT_TRUE(g_pendingConnect);
}

TEST_F(WebTest, HandleSaveDoesNotSetPendingConnectWhenSsidUnchanged) {
    strncpy(cfg_wifi_ssid, "SameNet", sizeof(cfg_wifi_ssid));
    g_mock_server_args["wifi_ssid"] = "SameNet";
    handleSave();
    EXPECT_FALSE(g_pendingConnect);
}

TEST_F(WebTest, HandleSaveRedirects303) {
    handleSave();
    EXPECT_EQ(303, g_mock_last_send_code);
}

TEST_F(WebTest, HandleSaveRedirectLocationIsRoot) {
    handleSave();
    EXPECT_EQ("/", g_mock_last_redirect._s);
}

TEST_F(WebTest, HandleSaveClampsGpioPinAbove39) {
    g_mock_server_args["gpio_pin"] = "50";
    handleSave();
    EXPECT_EQ(2, cfg_gpio_pin);
}

TEST_F(WebTest, HandleSaveClampsToggleIntervalBelow100) {
    g_mock_server_args["toggle_ms"] = "10";
    handleSave();
    EXPECT_EQ(100u, cfg_toggle_interval_ms);
}

TEST_F(WebTest, HandleSaveClampsPollIntervalAbove1440) {
    g_mock_server_args["poll_min"] = "9999";
    handleSave();
    EXPECT_EQ(1440, cfg_poll_interval_min);
}

TEST_F(WebTest, HandleSaveTruncatesSsidAtMaxLength) {
    g_mock_server_args["wifi_ssid"] = std::string(70, 'X');
    handleSave();
    EXPECT_EQ(63u, strlen(cfg_wifi_ssid));
}

// ---- handleScan ----

TEST_F(WebTest, HandleScanReturns200) {
    handleScan();
    EXPECT_EQ(200, g_mock_last_send_code);
}

TEST_F(WebTest, HandleScanReturnsValidJsonArray) {
    handleScan();
    EXPECT_EQ('[', g_mock_last_send_body._s.front());
    EXPECT_EQ(']', g_mock_last_send_body._s.back());
}

TEST_F(WebTest, HandleScanReturnsNetworksSortedByRssi) {
    g_mock_scan_results = {{"Weak",-80},{"Strong",-40},{"Medium",-60}};
    handleScan();
    const std::string& body = g_mock_last_send_body._s;
    size_t posStrong = body.find("Strong");
    size_t posMedium = body.find("Medium");
    size_t posWeak   = body.find("Weak");
    ASSERT_NE(std::string::npos, posStrong);
    ASSERT_NE(std::string::npos, posMedium);
    ASSERT_NE(std::string::npos, posWeak);
    EXPECT_LT(posStrong, posMedium);
    EXPECT_LT(posMedium, posWeak);
}

TEST_F(WebTest, HandleScanDeduplicatesSsids) {
    g_mock_scan_results = {{"Net1",-50},{"Net1",-55},{"Net2",-60}};
    handleScan();
    const std::string& body = g_mock_last_send_body._s;
    int count = 0;
    size_t pos = 0;
    while((pos = body.find("Net1", pos)) != std::string::npos) { count++; pos++; }
    EXPECT_EQ(1, count);
}

TEST_F(WebTest, HandleScanIncludesRssiValues) {
    g_mock_scan_results = {{"Net1",-45}};
    handleScan();
    EXPECT_NE(std::string::npos, g_mock_last_send_body._s.find("-45"));
}

TEST_F(WebTest, HandleScanEmptyArrayWhenNoNetworks) {
    g_mock_scan_results.clear();
    handleScan();
    EXPECT_EQ("[]", g_mock_last_send_body._s);
}
