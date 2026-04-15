#pragma once

#include <stdint.h>

#define MCP_IODIRA 0x00
#define MCP_IODIRB 
#define MCP_OLATA 0x14
#define MCP_OLATB


#define SOL0_ADDR 0x20
#define SOL1_ADDR 0x21

void solenoid_init(uint8_t addr);
void solenoid_write(uint8_t addr, uint8_t mask);

void grid_clear(uint8_t grid[3][5]);
void grid_apply(uint8_t grid[3][5]);


