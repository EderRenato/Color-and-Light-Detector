#ifndef GY33_H
#define GY33_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"


void gy33_init(i2c_inst_t *i2c_port, uint8_t sda_pin, uint8_t scl_pin);
void gy33_read_normalized_color(uint8_t *r, uint8_t *g, uint8_t *b, uint16_t *clear_val);

#endif // GY33_H