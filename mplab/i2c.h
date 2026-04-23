#pragma once

#include <stdint.h>	

void i2c_init(void);
uint8_t i2c_send(uint8_t data);
uint8_t i2c_start(uint8_t address);
void i2c_stop(void);
void i2c_wait(void);

uint8_t i2c_write_register(uint8_t addr7, uint8_t reg, uint8_t data);

uint8_t i2c_read(uint8_t *data, uint8_t send_ack);
uint8_t i2c_read_registers(uint8_t addr7, uint8_t reg, uint8_t* buf, uint8_t len);