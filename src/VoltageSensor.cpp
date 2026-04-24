#include "VoltageSensor.hpp"
#include "esp_log.h"

static const char* TAG = "VoltageSensor";

VoltageSensor::VoltageSensor(adc_unit_t unit, adc_channel_t channel, float divider_ratio)
    : m_unit(unit), m_channel(channel), m_divider_ratio(divider_ratio), 
      m_adc_handle(nullptr), m_cali_handle(nullptr), m_do_cali(false) {}

void VoltageSensor::init() {
    // 1. ADC Init
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = m_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &m_adc_handle));

    // 2. ADC Channel Config
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11, // Up to ~3.1V
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(m_adc_handle, m_channel, &config));

    // 3. Calibration Init (Optional but recommended)
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = m_unit,
        .chan = m_channel,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &m_cali_handle);
    if (ret == ESP_OK) {
        m_do_cali = true;
        ESP_LOGI(TAG, "ADC Calibration initialized");
    } else {
        ESP_LOGW(TAG, "ADC Calibration failed or not supported, using raw readings");
    }
}

float VoltageSensor::readVoltage() {
    int adc_raw;
    int voltage_mv;
    
    ESP_ERROR_CHECK(adc_oneshot_read(m_adc_handle, m_channel, &adc_raw));
    
    if (m_do_cali) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(m_cali_handle, adc_raw, &voltage_mv));
    } else {
        // Fallback: 12-bit ADC (0-4095) for 3.1V
        voltage_mv = (adc_raw * 3100) / 4095;
    }
    
    // Convert mV to V and apply divider ratio
    return (voltage_mv / 1000.0f) * m_divider_ratio;
}
