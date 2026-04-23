#include "PowerController.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define CONTROL_GPIO GPIO_NUM_2  // Use GPIO 2
#define CYCLE_DELAY_MS 10000    // 5 secs in milliseconds

extern "C" void app_main() {
    // Initialize the PowerController on GPIO 2
    PowerController controller(CONTROL_GPIO);
    controller.init();

    ESP_LOGI("MAIN", "Starting Power Control Loop (10-minute intervals)");

    while (true) {
        // Stay OFF for 10 minutes
        controller.setPower(false);
        vTaskDelay(pdMS_TO_TICKS(CYCLE_DELAY_MS));

        // Stay ON for 10 minutes
        controller.setPower(true);
        vTaskDelay(pdMS_TO_TICKS(CYCLE_DELAY_MS));
    }
}
