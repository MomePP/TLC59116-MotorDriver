#pragma once

#ifndef __TLC59116_HPP__
#define __TLC59116_HPP__

#include <Wire.h>

class TLC59116 {
private:
    constexpr static uint8_t _mode1 =       0x00;
    constexpr static uint8_t _mode2 =       0x01;
    constexpr static uint8_t _pwm0 =        0x02;
    constexpr static uint8_t _pwm1 =        0x03;
    constexpr static uint8_t _pwm2 =        0x04;
    constexpr static uint8_t _pwm3 =        0x05;
    constexpr static uint8_t _pwm4 =        0x06;
    constexpr static uint8_t _pwm5 =        0x07;
    constexpr static uint8_t _pwm6 =        0x08;
    constexpr static uint8_t _pwm7 =        0x09;
    constexpr static uint8_t _pwm8 =        0x0A;
    constexpr static uint8_t _pwm9 =        0x0B;
    constexpr static uint8_t _pwm10 =       0x0C;
    constexpr static uint8_t _pwm11 =       0x0D;
    constexpr static uint8_t _pwm12 =       0x0E;
    constexpr static uint8_t _pwm13 =       0x0F;
    constexpr static uint8_t _pwm14 =       0x10;
    constexpr static uint8_t _pwm15 =       0x11;
    constexpr static uint8_t _led_out0 =    0x14;
    constexpr static uint8_t _led_out1 =    0x15;
    constexpr static uint8_t _led_out2 =    0x16;
    constexpr static uint8_t _led_out3 =    0x17;
    constexpr static uint8_t _grp_pwm =     0x12;
    constexpr static uint8_t _grp_freq =    0x13;

    constexpr static uint8_t _def_base_address = 0x60;
    constexpr static uint8_t _n_ctrl_regs = 4U;

    const uint8_t _i2c_address = 0x60;

    uint8_t _led_control_buf[_n_ctrl_regs];

    void writeRegister(uint8_t addr, uint8_t val){
        Wire.beginTransmission(_i2c_address);
        Wire.write(addr);
        Wire.write(val);
        Wire.endTransmission();
    }

public:

    TLC59116(uint8_t i2c_address) : _i2c_address(i2c_address){
        //init buffer for led control
        _led_control_buf[0] = 0x0;
        _led_control_buf[1] = 0x0;
        _led_control_buf[2] = 0x0;
        _led_control_buf[3] = 0x0;
    };

    TLC59116() : TLC59116(_def_base_address){};

    void begin(){
        Wire.begin();
    }

    void set_mode1(uint8_t mode){
        writeRegister(_mode1, mode);
    }

    void set_mode2(uint8_t mode){
        //filter out reserved bits
        writeRegister(_mode2, mode & 0xA8);
    }

    void set_led_mode(uint8_t led, uint8_t mode){
        //get current mode from register buffer
        uint8_t* reg = _led_control_buf + ((led & 0x0F) >> 2);
        //budget modulo to get first position of led mode in byte
        uint8_t idx = ((led & 0x0F) & (0x03)) << 1;
        //only allow valid modes from 0x00 to 0x03
        uint8_t mask = (mode & 0x03) << idx;
        //mask for position in byte
        uint8_t bitmask = 0x03 << idx;
        //first set zeros, then put ones where they are wanted
        *reg = (*reg & ~bitmask) | mask;
        writeRegister(_led_out0 + ((led & 0x0F) >> 2), *reg);
    }

    void set_led_mode_multi(uint8_t* leds, uint8_t n, uint8_t mode){
        for(uint8_t i = 0; i < n; i++){
            set_led_mode(*(leds + i), mode);
        }
    }

    void set_individual_brightness_multi(uint8_t* leds, uint8_t n, uint8_t val){
        for(uint8_t i = 0; i < n; i++){
            set_individual_brightness(*(leds + i), val);
        }
    }

    void set_individual_brightness(uint8_t led, uint8_t val){
        writeRegister(_pwm0 + (led & 0x0F), val);
    }
    

    void set_group_brightness(uint8_t val){
        writeRegister(_grp_pwm, val);
    }

    void turn_on_led(uint8_t led){
        set_led_mode(led, 0x01);
    }

    void turn_off_led(uint8_t led){
        set_led_mode(led, 0x00);
    }

    void set_group_freq(uint8_t val){
        writeRegister(_grp_freq, val);
    }

    void turn_all_off(){
        writeRegister(_led_out0, 0x0);
        writeRegister(_led_out1, 0x0);
        writeRegister(_led_out2, 0x0);
        writeRegister(_led_out3, 0x0);
    }

    void turn_all_on(){
        writeRegister(_led_out0, 0x55);
        writeRegister(_led_out1, 0x55);
        writeRegister(_led_out2, 0x55);
        writeRegister(_led_out3, 0x55);
    }

};


#endif