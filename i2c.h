#pragma once

#include <stdint.h>	// uint8_t

void i2c_init(void);
uint8_t i2c_start(uint8_t address);
uint8_t i2c_repeated_start(uint8_t address);
uint8_t i2c_write(uint8_t data);
char i2c_read_ack(void);
char i2c_read_nack(void);
void i2c_Stop(void);
void send_over_twi(uint8_t data);
