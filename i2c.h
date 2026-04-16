#pragma once

#include <stdint.h>	// uint8_t
extern uint8_t LCD_ADDR;

void i2c_scan(void);
void i2c_init(void);
uint8_t i2c_start(uint8_t address);
uint8_t i2c_repeated_start(uint8_t address);
uint8_t i2c_write(uint8_t data);
char i2c_read_ack(void);
char i2c_read_nack(void);
void i2c_stop(void);
void send_over_twi(uint8_t data);
