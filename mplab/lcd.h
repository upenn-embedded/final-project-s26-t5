#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// LCD I2C backpack 7-bit address
#define LCD_ADDR 0x27

// Command bytes
#define DDRAM_ADDRESS_SET   0x80
#define DISPLAY_CLEAR       0x01
#define DISPLAY_OFF         0x08
#define DISPLAY_ON          0x0F
#define ENTRY_MODE_SET      0x06
#define FUNCTION_SET_4B_2L  0x28
#define RETURN_HOME         0x02

// Initialization modes
#define BIT_MODE_4 0
#define BIT_MODE_8 1

// PCF8574 pin mapping:
// P7 P6 P5 P4 P3 P2 P1 P0
// D7 D6 D5 D4 BL  E  RW RS
#define ENABLE_BIT 2
#define RW_BIT     1
#define RS_BIT     0
#define BT_BIT     3

#define RS_DATA (1 << RS_BIT)
#define BT_DATA (1 << BT_BIT)

void lcd_init(void);
void lcd_clear(void);

void lcd_command_write(uint8_t command, uint8_t bitmode);
void lcd_data_write(uint8_t data);

void lcd_move_cursor(uint8_t line, uint8_t col);
void lcd_print(const char *msg);
void lcd_print_nl(void);

void lcd_load_pattern(uint8_t index_pattern);
void lcd_save_pattern(const char pattern[], size_t size, uint8_t addr_offset);