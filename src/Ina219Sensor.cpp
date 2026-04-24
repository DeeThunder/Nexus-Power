#include "Ina219Sensor.hpp"
#include "esp_log.h"
#include <endian.h>

static const char* TAG = "INA219";

// Registers
#define REG_CONFIG      0x00
#define REG_SHUNT_VOLT  0x01
#define REG_BUS_VOLT    0x02
#define REG_POWER       0x03
#define REG_CURRENT     0x04
#define REG_CALIB       0x05

Ina219Sensor::Ina219Sensor(int sda_pin, int scl_pin, uint8_t address)
    : m_sda_pin(sda_pin), m_scl_pin(scl_pin), m_address(address), 
      m_bus_handle(nullptr), m_dev_handle(nullptr) {}

esp_err_t Ina219Sensor::init() {
    // 1. Initialize I2C Bus
    i2c_master_bus_config_t bus_config = {
        .i2c_port = -1, // Auto-select
        .sda_io_num = (gpio_num_t)m_sda_pin,
        .scl_io_num = (gpio_num_t)m_scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = { .enable_internal_pullup = 1 }
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &m_bus_handle));

    // 2. Add Device to Bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = m_address,
        .scl_speed_hz = 100000, // 100kHz
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(m_bus_handle, &dev_config, &m_dev_handle));

    // 3. Calibrate for 32V, 2A (Typical configuration)
    // Calibration Value = 4096 (for 100mOhm shunt, 32V range, 2A max)
    writeRegister(REG_CALIB, 4096);
    
    // Set Config: Range=32V, Gain=/8, ADC=12bit, Mode=Shunt&Bus Continuous
    // 0x399F is a common default for 32V/2A
    writeRegister(REG_CONFIG, 0x399F);

    ESP_LOGI(TAG, "Initialized successfully at address 0x%02X", m_address);
    return ESP_OK;
}

float Ina219Sensor::getBusVoltage_V() {
    uint16_t value;
    if (readRegister(REG_BUS_VOLT, &value) != ESP_OK) return 0.0f;
    // Shift right by 3, LSB is 4mV
    return (float)((value >> 3) * 4) / 1000.0f;
}

float Ina219Sensor::getCurrent_mA() {
    uint16_t value;
    if (readRegister(REG_CURRENT, &value) != ESP_OK) return 0.0f;
    // Current LSB is 0.1mA for 32V/2A calibration
    return (float)((int16_t)value) / 10.0f;
}

float Ina219Sensor::getPower_mW() {
    uint16_t value;
    if (readRegister(REG_POWER, &value) != ESP_OK) return 0.0f;
    // Power LSB is 2mW for 32V/2A calibration
    return (float)value * 2.0f;
}

esp_err_t Ina219Sensor::writeRegister(uint8_t reg, uint16_t value) {
    uint8_t data[3];
    data[0] = reg;
    data[1] = (value >> 8) & 0xFF;
    data[2] = value & 0xFF;
    return i2c_master_transmit(m_dev_handle, data, 3, -1);
}

esp_err_t Ina219Sensor::readRegister(uint8_t reg, uint16_t *value) {
    uint8_t data[2];
    esp_err_t err = i2c_master_transmit_receive(m_dev_handle, &reg, 1, data, 2, -1);
    if (err == ESP_OK) {
        *value = (data[0] << 8) | data[1];
    }
    return err;
}
