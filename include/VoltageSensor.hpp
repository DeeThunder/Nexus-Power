#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

/**
 * @brief Class to read voltage from a DC Voltage Sensor (Voltage Divider).
 */
class VoltageSensor {
public:
    /**
     * @brief Construct a new Voltage Sensor object
     * 
     * @param unit ADC unit (usually ADC_UNIT_1)
     * @param channel ADC channel
     * @param divider_ratio The ratio of the voltage divider (default 5.0 for standard 0-25V sensor)
     */
    VoltageSensor(adc_unit_t unit, adc_channel_t channel, float divider_ratio = 5.0f);

    /**
     * @brief Initialize the ADC and calibration
     */
    void init();

    /**
     * @brief Read the current voltage
     * 
     * @return float The measured voltage in Volts
     */
    float readVoltage();

private:
    adc_unit_t m_unit;
    adc_channel_t m_channel;
    float m_divider_ratio;
    
    adc_oneshot_unit_handle_t m_adc_handle;
    adc_cali_handle_t m_cali_handle;
    bool m_do_cali;
};
