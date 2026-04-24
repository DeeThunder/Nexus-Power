#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

/**
 * @brief Class to interface with the INA219 Power Monitor via I2C.
 * Works with ESP-IDF 5.x I2C master driver.
 */
class Ina219Sensor {
public:
    /**
     * @brief Construct a new Ina219 Sensor object
     * 
     * @param sda_pin GPIO for SDA (Default 8 for C3)
     * @param scl_pin GPIO for SCL (Default 9 for C3)
     * @param address I2C Address (Default 0x40)
     */
    Ina219Sensor(int sda_pin = 8, int scl_pin = 9, uint8_t address = 0x40);

    /**
     * @brief Initialize I2C bus and the INA219 sensor
     */
    esp_err_t init();

    /**
     * @brief Read bus voltage
     * @return float Voltage in Volts
     */
    float getBusVoltage_V();

    /**
     * @brief Read current
     * @return float Current in milliamperes (mA)
     */
    float getCurrent_mA();

    /**
     * @brief Read power
     * @return float Power in milliwatts (mW)
     */
    float getPower_mW();

private:
    int m_sda_pin;
    int m_scl_pin;
    uint8_t m_address;

    i2c_master_bus_handle_t m_bus_handle;
    i2c_master_dev_handle_t m_dev_handle;

    esp_err_t writeRegister(uint8_t reg, uint16_t value);
    esp_err_t readRegister(uint8_t reg, uint16_t *value);
};
