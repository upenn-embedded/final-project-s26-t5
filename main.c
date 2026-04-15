#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "i2c.h"
#include "solenoid.h"

int main(void) {
    DDRB |= (1 << PB5);
    
    i2c_init();
    solenoid_init(SOL0_ADDR);
    solenoid_init(SOL1_ADDR);

    while (1) {
        // cycle through all 15 solenoids one at a time
        uint8_t grid[3][5] = {0};
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 5; c++) {
                grid_clear(grid);
                grid[r][c] = 1;
                grid_apply(grid);
                _delay_ms(200);
            }
        }
        grid_clear(grid);
        grid_apply(grid);
        _delay_ms(500);
    }
}