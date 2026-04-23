#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "lcd.h"
#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdint.h>
#include <stddef.h>

// Write one raw byte to the LCD backpack over I2C
static uint8_t lcd_write_raw(uint8_t data) {
    if (!i2c_start((LCD_ADDR << 1) | 0)) {
        i2c_stop();
        return 0;
    }

    if (!i2c_send(data)) {
        i2c_stop();
        return 0;
    }

    i2c_stop();
    return 1;
}

// Pulse E high then low
static void lcd_enable(uint8_t data) {
    lcd_write_raw(data | (1 << ENABLE_BIT));
    _delay_us(1);

    lcd_write_raw(data & ~(1 << ENABLE_BIT));
    _delay_us(50);
}

// Send one 4-bit transfer (already positioned in upper nibble)
static void lcd_send(uint8_t data) {
    uint8_t out = data | BT_DATA;   // keep backlight on
    lcd_write_raw(out);
    lcd_enable(out);
}

void lcd_clear(void) {
    lcd_command_write(DISPLAY_CLEAR, BIT_MODE_4);
    _delay_ms(2);
}

void lcd_command_write(uint8_t command, uint8_t bitmode) {
    if (bitmode == BIT_MODE_8) {
        lcd_send(command);
    } else if (bitmode == BIT_MODE_4) {
        uint8_t highnib = command & 0xF0;
        uint8_t lownib  = (command << 4) & 0xF0;

        lcd_send(highnib);
        lcd_send(lownib);
    }
}

void lcd_data_write(uint8_t data) {
    uint8_t highnib = data & 0xF0;
    uint8_t lownib  = (data << 4) & 0xF0;

    lcd_send(highnib | RS_DATA);
    lcd_send(lownib  | RS_DATA);
}

void lcd_init(void) {
    // HD44780 4-bit init sequence
    _delay_ms(50);

    lcd_command_write(0x03 << 4, BIT_MODE_8);
    _delay_us(4500);

    lcd_command_write(0x03 << 4, BIT_MODE_8);
    _delay_us(150);

    lcd_command_write(0x03 << 4, BIT_MODE_8);
    _delay_us(150);

    lcd_command_write(0x02 << 4, BIT_MODE_8);
    _delay_us(150);

    lcd_command_write(FUNCTION_SET_4B_2L, BIT_MODE_4);
    lcd_command_write(DISPLAY_OFF, BIT_MODE_4);
    lcd_clear();
    lcd_command_write(ENTRY_MODE_SET, BIT_MODE_4);
    lcd_command_write(DISPLAY_ON, BIT_MODE_4);
}

void lcd_move_cursor(uint8_t line, uint8_t col) {
    if (line > 1 || col > 15) {
        return;
    }

    uint8_t ddram_addr = DDRAM_ADDRESS_SET | col;
    ddram_addr += line * 0x40;

    lcd_command_write(ddram_addr, BIT_MODE_4);
}

void lcd_print(const char *msg) {
    while (*msg) {
        lcd_data_write((uint8_t)*msg++);
    }
}

void lcd_print_nl(void) {
    lcd_move_cursor(1, 0);
}

void lcd_load_pattern(uint8_t index_pattern) {
    if (index_pattern > 7) {
        return;
    }

    lcd_data_write(index_pattern);
}

void lcd_save_pattern(const char pattern[], size_t size, uint8_t addr_offset) {
    if (pattern == NULL || size != 8 || addr_offset > 7) {
        return;
    }

    // CGRAM address: 0x40 + 8 * location
    lcd_command_write(0x40 + (addr_offset * 8), BIT_MODE_4);

    for (uint8_t i = 0; i < 8; i++) {
        lcd_data_write((uint8_t)pattern[i]);
    }
}