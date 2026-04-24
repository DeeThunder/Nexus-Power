#include "PowerController.hpp"
#include "Ina219Sensor.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define CONTROL_GPIO GPIO_NUM_2
#define LOW_BATTERY_THRESHOLD 11.5f
#define READ_INTERVAL_MS 2000

extern "C" void app_main() {
    // 1. Initialize Power Controller (GPIO 2)
    PowerController powerCtrl(CONTROL_GPIO);
    powerCtrl.init();

    // 2. Initialize INA219 (SDA=8, SCL=9)
    Ina219Sensor ina219(8, 9);
    if (ina219.init() != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize INA219!");
    }

    ESP_LOGI("MAIN", "Smart Power Bank Monitoring Started (INA219)");

    while (true) {
        float voltage = ina219.getBusVoltage_V();
        float current = ina219.getCurrent_mA();
        float power   = ina219.getPower_mW();

        ESP_LOGI("MAIN", "V: %.2f V | I: %.1f mA | P: %.1f mW", voltage, current, power);

        if (voltage < LOW_BATTERY_THRESHOLD && voltage > 1.0f) { // voltage > 1V to avoid false triggers if disconnected
            ESP_LOGW("MAIN", "LOW BATTERY! Voltage: %.2f V. Turning OFF power.", voltage);
            powerCtrl.setPower(false);
        } else if (voltage >= LOW_BATTERY_THRESHOLD) {
            if (!powerCtrl.isOn()) {
                ESP_LOGI("MAIN", "Battery OK. Restoring power.");
                powerCtrl.setPower(true);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}
