#include "PowerController.hpp"
#include "Ina219Sensor.hpp"
#include "NexusBLE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"

#define CONTROL_GPIO GPIO_NUM_10
#define READ_INTERVAL_MS 1000

// Battery Calibration (3S Li-ion) - Lookup table for accurate non-linear curve
// Full usable range: 9.0V (3.0V/cell dead) to 12.6V (4.2V/cell full)
// Output is cut off at BATT_CUTOFF_PCT (10%) which corresponds to ~10.8V
static const float BATT_LUT[][2] = {
    {12.60f, 100.0f},
    {12.45f,  90.0f},
    {12.24f,  80.0f},
    {12.06f,  70.0f},
    {11.88f,  60.0f},
    {11.70f,  50.0f},
    {11.52f,  40.0f},
    {11.34f,  30.0f},
    {11.16f,  20.0f},
    {10.80f,  10.0f},
    {10.20f,   5.0f},
    { 9.00f,   0.0f},  // Absolute minimum: 3.0V per cell
};
static const int BATT_LUT_SIZE = sizeof(BATT_LUT) / sizeof(BATT_LUT[0]);

static PowerController powerCtrl(CONTROL_GPIO);
static Ina219Sensor ina219(4, 5);
static NexusBLE nexusBle;

/**
 * @brief Calculate battery percentage using a Li-ion discharge curve lookup table.
 *        Interpolates linearly between the two nearest voltage breakpoints for accuracy.
 */
uint8_t calculateBatteryPct(float voltage) {
    if (voltage >= BATT_LUT[0][0]) return 100;
    if (voltage <= BATT_LUT[BATT_LUT_SIZE - 1][0]) return 0;

    for (int i = 0; i < BATT_LUT_SIZE - 1; i++) {
        float v_high = BATT_LUT[i][0];
        float v_low  = BATT_LUT[i + 1][0];
        if (voltage <= v_high && voltage > v_low) {
            float pct_high = BATT_LUT[i][1];
            float pct_low  = BATT_LUT[i + 1][1];
            float ratio = (voltage - v_low) / (v_high - v_low);
            return (uint8_t)(pct_low + ratio * (pct_high - pct_low));
        }
    }
    return 0;
}

/**
 * @brief Callback for BLE toggle command
 */
#define BATT_CUTOFF_PCT 0  // Cut off only when battery hits 0% (9.0V) — LUT floor is the cell protection

static uint8_t systemStatus = 0; // 0: OK, 1: LOW_BATT_SHUTDOWN

void onBleToggle(bool on) {
    ESP_LOGI("MAIN", "BLE Command: Turn Power %s", on ? "ON" : "OFF");
    powerCtrl.setPower(on);
    if (on) systemStatus = 0; // Reset status if user manually overrides
}

extern "C" void app_main() {
    // Immediate log to see if we reach app_main
    printf("\n\n[SYSTEM] BOOT START...\n");

    // 1. Initialize NVS (required for BLE)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("MAIN", "Initializing Hardware...");
    powerCtrl.init();

    if (ina219.init() != ESP_OK) {
        ESP_LOGE("MAIN", "INA219 Init Failed!");
    }

    ESP_LOGI("MAIN", "Initializing BLE...");
    nexusBle.init("Nexus Power");
    nexusBle.setToggleCallback(onBleToggle);

    // --- Nexus Power Splash Screen ---
    uint8_t m[6];
    esp_read_mac(m, ESP_MAC_BT);
    uint8_t sig = (m[3] + m[4] + m[5]) ^ 0xAC;
    
    printf("\n==========================================\n");
    printf("       NEXUS POWER - SMART SYSTEM         \n");
    printf("==========================================\n");
    printf(" STATUS:   BLE Advertising Active         \n");
    printf(" DEVICE:   Nexus Power                    \n");
    printf(" SERIAL:   NX-%02X%02X%02X                 \n", m[3], m[4], m[5]);
    printf(" REG_KEY:  %02X%02X%02X%02X               \n", m[3], m[4], m[5], sig);
    printf(" DASHBOARD: https://deethunder.github.io/Smart-Power-bank/ \n");
    printf("==========================================\n\n");
    fflush(stdout);
    // Rolling voltage buffer for charging detection (5 samples)
    static const int V_BUF_SIZE = 5;
    float vBuf[V_BUF_SIZE] = {0};
    int vBufIdx = 0;
    bool vBufFull = false;

    while (true) {
        float v = ina219.getBusVoltage_V();
        float i = ina219.getCurrent_mA();
        float p = ina219.getPower_mW();
        uint8_t pct = calculateBatteryPct(v);

        // --- Charging Detection via Voltage Trend ---
        // Fill rolling buffer
        vBuf[vBufIdx] = v;
        vBufIdx = (vBufIdx + 1) % V_BUF_SIZE;
        if (vBufIdx == 0) vBufFull = true;

        // Compare oldest vs newest sample; rise > 50mV across 5s = charging
        bool isCharging = false;
        if (vBufFull) {
            float oldest = vBuf[vBufIdx];        // oldest slot (about to be overwritten)
            float newest = vBuf[(vBufIdx + V_BUF_SIZE - 1) % V_BUF_SIZE];
            isCharging = (newest - oldest) > 0.05f;
        }

        // Safety Cutoff (Percentage Based)
        if (pct <= BATT_CUTOFF_PCT && powerCtrl.isOn()) {
            ESP_LOGW("MAIN", "Battery at %d%%! Protecting cells...", pct);
            powerCtrl.setPower(false);
            systemStatus = 1; // Mark as Low Battery Shutdown
        }

        // Notify BLE Dashboard
        nexusBle.updatePowerData(v, i, p, pct, powerCtrl.isOn(), systemStatus, isCharging);

        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}
