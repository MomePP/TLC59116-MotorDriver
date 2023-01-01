#include <Wire.h>

#include "TLC59116.hpp"

TLC59116 tlc59116(0x60);

uint8_t reds[] = {2, 5, 9, 11, 12};
uint8_t blues[] = {0, 3, 6, 10, 14};
uint8_t greens[] = {1, 4, 7, 8, 13};

void setup(){

    tlc59116.begin();
    tlc59116.set_mode1(0x01);

    tlc59116.turn_all_off();
    tlc59116.set_led_mode_multi(greens, 5, TLC59116::driver_mode::GROUP);
    tlc59116.set_led_mode_multi(reds, 5, TLC59116::driver_mode::INDIVIDUAL);
    tlc59116.set_led_mode_multi(blues, 5, TLC59116::driver_mode::GROUP);
    tlc59116.set_individual_brightness_multi(greens, 5, 0x10);
    tlc59116.set_individual_brightness_multi(blues, 5, 0x10);
    tlc59116.set_individual_brightness_multi(reds, 5, 0x10);

    tlc59116.set_group_ctrl(TLC59116::group_control::BLINKING);
    tlc59116.set_group_pwm(0x10);

    tlc59116.set_group_freq(TLC59116::seconds_to_freq(3.2));

    Serial.begin(115200);
    Serial.println("Setup complete");
}

int i = 0;

void loop(){
    tlc59116.turn_on_led(reds[i%5]);
    delay(200);
    tlc59116.turn_off_led(reds[i++%5]);
}




