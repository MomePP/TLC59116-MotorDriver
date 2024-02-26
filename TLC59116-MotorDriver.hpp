#pragma once

#ifndef __TLC59116_HPP__
#define __TLC59116_HPP__

#include "I2CUtils/i2c-register.hpp"

#define DEFAULT_TLC59116_ADDRESS 0x60

class TLC59116 : public I2CDevice
{
public:
    enum class driver_mode : uint8_t
    {
        OFF = 0b00,
        FULLY_ON = 0b01, // no brightness/group/pwm control
        INDIVIDUAL = 0b10,
        GROUP = 0b11
    };

    enum class group_control : uint8_t
    {
        DIMMING = 0,
        BLINKING = 1
    };

private:
    // registers
    constexpr static uint8_t _mode1 = 0x00;
    constexpr static uint8_t _mode2 = 0x01;
    constexpr static uint8_t _pwm0 = 0x02;
    constexpr static uint8_t _pwm1 = 0x03;
    constexpr static uint8_t _pwm2 = 0x04;
    constexpr static uint8_t _pwm3 = 0x05;
    constexpr static uint8_t _pwm4 = 0x06;
    constexpr static uint8_t _pwm5 = 0x07;
    constexpr static uint8_t _pwm6 = 0x08;
    constexpr static uint8_t _pwm7 = 0x09;
    constexpr static uint8_t _pwm8 = 0x0A;
    constexpr static uint8_t _pwm9 = 0x0B;
    constexpr static uint8_t _pwm10 = 0x0C;
    constexpr static uint8_t _pwm11 = 0x0D;
    constexpr static uint8_t _pwm12 = 0x0E;
    constexpr static uint8_t _pwm13 = 0x0F;
    constexpr static uint8_t _pwm14 = 0x10;
    constexpr static uint8_t _pwm15 = 0x11;
    constexpr static uint8_t _led_out0 = 0x14;
    constexpr static uint8_t _led_out1 = 0x15;
    constexpr static uint8_t _led_out2 = 0x16;
    constexpr static uint8_t _led_out3 = 0x17;
    constexpr static uint8_t _grp_pwm = 0x12;
    constexpr static uint8_t _grp_freq = 0x13;

    constexpr static uint8_t _n_ctrl_regs = 4U;

    constexpr static float _min_period = 0.041;
    constexpr static float _max_period = 10.73;

    const uint8_t _def_i2c_address = DEFAULT_TLC59116_ADDRESS;

    uint8_t _output_control_buf[_n_ctrl_regs]; // buffering led driver control status
    uint8_t _mode2_buf;
    uint8_t _mode1_buf;

    void sync_out_mode1()
    {
        write_register(_mode1, _mode1_buf);
    }

    void sync_out_mode2()
    {
        // mask because only non-reserved bits should be written
        write_register(_mode2, _mode2_buf & 0xA8);
    }

public:
    TLC59116(uint8_t i2c_address)
        : I2CDevice(i2c_address)
    {
        // init buffer for led control
        _output_control_buf[0] = 0x0;
        _output_control_buf[1] = 0x0;
        _output_control_buf[2] = 0x0;
        _output_control_buf[3] = 0x0;
    };

    TLC59116()
        : TLC59116(DEFAULT_TLC59116_ADDRESS){};

    bool begin()
    {
        bool deviceExists = false;
        Wire.begin();
        Wire.beginTransmission(DEFAULT_TLC59116_ADDRESS);
        if (Wire.endTransmission() == 0)
        {
            deviceExists = true;
            _mode2_buf = read8(_mode2);
        }
        return deviceExists;
    }

    void set_mode1(uint8_t mode)
    {
        write_register(_mode1, mode);
    }

    void set_mode2(uint8_t mode)
    {
        _mode2_buf = mode;
        sync_out_mode2(); // filter out reserved bits
    }

    void set_group_ctrl(group_control grp_ctrl)
    {
        _mode2_buf = (_mode2_buf & ~0xDF) | ((uint8_t)grp_ctrl << 5);
        sync_out_mode2();
    }

    void set_output_mode(uint8_t channel, driver_mode mode)
    {
        // get current mode from register buffer
        uint8_t *reg = _output_control_buf + ((channel & 0x0F) >> 2);
        // budget modulo to get first position of led mode in byte
        uint8_t idx = ((channel & 0x0F) & (0x03)) << 1;
        uint8_t mask = ((uint8_t)mode) << idx;
        // mask for position in byte
        uint8_t bitmask = 0x03 << idx;
        // first set zeros, then put ones where they are wanted
        *reg = (*reg & ~bitmask) | mask; // updates buffer
        write_register(_led_out0 + ((channel & 0x0F) >> 2), *reg);
    }

    void set_output_mode(uint8_t *channels, uint8_t count, driver_mode mode)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            set_output_mode(*(channels + i), mode);
        }
    }

    void turn_on_pwm(uint8_t channel, uint8_t value)
    {
        write_register(_pwm0 + (channel & 0x0F), value);
    }

    void turn_on_pwm(uint8_t *channels, uint8_t count, uint8_t value)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            turn_on_pwm(*(channels + i), value);
        }
    }

    void set_group_pwm(uint8_t value)
    {
        write_register(_grp_pwm, value);
    }

    void turn_on(uint8_t channel)
    {
        set_output_mode(channel, driver_mode::FULLY_ON);
    }

    void turn_off(uint8_t channel)
    {
        set_output_mode(channel, driver_mode::OFF);
    }

    void set_group_freq(uint8_t value)
    {
        write_register(_grp_freq, value);
    }

    void turn_all_off()
    {
        write_register(_led_out0, 0x0);
        write_register(_led_out1, 0x0);
        write_register(_led_out2, 0x0);
        write_register(_led_out3, 0x0);
    }

    void turn_all_on()
    {
        write_register(_led_out0, 0x55);
        write_register(_led_out1, 0x55);
        write_register(_led_out2, 0x55);
        write_register(_led_out3, 0x55);
    }

    static constexpr uint8_t seconds_to_freq(float seconds)
    {
        return (seconds > _max_period)   ? 255
               : (seconds < _min_period) ? 0
                                         : static_cast<uint8_t>(24 * seconds - 1);
    }
};

#endif
