#pragma once

#include "driver/gpio.h"

/**
 * @brief Class to control power sources via GPIO.
 * 
 * Follows standard C++ practices by encapsulating GPIO logic.
 */
class PowerController {
public:
    /**
     * @brief Construct a new Power Controller object
     * 
     * @param gpio_num The GPIO pin number to control
     */
    explicit PowerController(gpio_num_t gpio_num);

    /**
     * @brief Initialize the GPIO pin configuration
     */
    void init();

    /**
     * @brief Set the power state
     * 
     * @param on True to turn on (HIGH), false to turn off (LOW)
     */
    void setPower(bool on);

    /**
     * @brief Check if the power is currently on
     * 
     * @return True if on, false if off
     */
    bool isOn() const;

    /**
     * @brief Toggle the power state
     */
    void toggle();

private:
    gpio_num_t m_gpio_num;
    bool m_is_on;
};
