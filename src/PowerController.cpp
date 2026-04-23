#include "PowerController.hpp"
#include "esp_log.h"

static const char* TAG = "PowerController";

PowerController::PowerController(gpio_num_t gpio_num) 
    : m_gpio_num(gpio_num), m_is_on(false) {}

void PowerController::init() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << m_gpio_num);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "GPIO %d initialized successfully", m_gpio_num);
    } else {
        ESP_LOGE(TAG, "Failed to initialize GPIO %d", m_gpio_num);
    }
    
    // Ensure it starts OFF
    setPower(false);
}

void PowerController::setPower(bool on) {
    m_is_on = on;
    gpio_set_level(m_gpio_num, on ? 1 : 0);
    ESP_LOGI(TAG, "Power set to %s on GPIO %d", on ? "ON" : "OFF", m_gpio_num);
}

bool PowerController::isOn() const {
    return m_is_on;
}

void PowerController::toggle() {
    setPower(!isOn());
}
