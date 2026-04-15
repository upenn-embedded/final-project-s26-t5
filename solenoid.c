#include "solenoid.h"
#include "i2c.h"
#include <stdint.h>
#define F_CPU 16000000UL

/*
 * Configure a single MCP23017:
 * - Port A outputs (IODIRA = 0x00)
 * - Port A low    (OLATA  = 0x00)
 */
void solenoid_init(uint8_t addr) {
    i2c_write_register(addr, MCP_IODIRA, 0x00);
    i2c_write_register(addr, MCP_OLATA,  0x00);
}

/*
 * Write a bitmask directly to port A
 */
void solenoid_write(uint8_t addr, uint8_t mask) {
    i2c_write_register(addr, MCP_OLATA, mask);
}

/*clear grid*/
void grid_clear(uint8_t grid[3][5]) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 5; c++) {
            grid[r][c] = 0;
        }
    }
}

/*
 * Mapping:
 * Board 0 (SOL0_ADDR) port A:
 *   bit0 = grid[0][0]
 *   bit1 = grid[0][1]
 *   bit2 = grid[0][2]
 *   bit3 = grid[0][3]
 *   bit4 = grid[0][4]
 *   bit5 = grid[1][0]
 *   bit6 = grid[1][1]
 *   bit7 = grid[1][2]
 *
 * Board 1 (SOL1_ADDR) port A:
 *   bit0 = grid[1][3]
 *   bit1 = grid[1][4]
 *   bit2 = grid[2][0]
 *   bit3 = grid[2][1]
 *   bit4 = grid[2][2]
 *   bit5 = grid[2][3]
 *   bit6 = grid[2][4]
 *   bit7 = not used
 */
void grid_apply(uint8_t grid[3][5]) {
    uint8_t b0 = 0;
    uint8_t b1 = 0;

    if (grid[0][0]) b0 |= (1 << 0);
    if (grid[0][1]) b0 |= (1 << 1);
    if (grid[0][2]) b0 |= (1 << 2);
    if (grid[0][3]) b0 |= (1 << 3);
    if (grid[0][4]) b0 |= (1 << 4);
    if (grid[1][0]) b0 |= (1 << 5);
    if (grid[1][1]) b0 |= (1 << 6);
    if (grid[1][2]) b0 |= (1 << 7);

    if (grid[1][3]) b1 |= (1 << 0);
    if (grid[1][4]) b1 |= (1 << 1);
    if (grid[2][0]) b1 |= (1 << 2);
    if (grid[2][1]) b1 |= (1 << 3);
    if (grid[2][2]) b1 |= (1 << 4);
    if (grid[2][3]) b1 |= (1 << 5);
    if (grid[2][4]) b1 |= (1 << 6);

    solenoid_write(SOL0_ADDR, b0);
    solenoid_write(SOL1_ADDR, b1);
}
