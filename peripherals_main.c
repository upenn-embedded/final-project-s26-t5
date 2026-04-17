#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdint.h>
#include <util/delay.h> 
#include <util/twi.h>    

#include "i2c.h"
#include "lcd_peripherals.h"


volatile bool time_mode_active = true;
volatile bool draw_mode_active = false;
volatile uint8_t mode_banner = 0;
int hall = 0;

#define HALL PC1
#define DRAW PD2
#define TIME PD3
#define RTC_ADDR 0xD0

//SOURCES
//https://github.com/hooperwf1/rtclock/blob/main/schematic.png
//https://github.com/hooperwf1/rtclock/blob/main/main.c
//https://github.com/hwfranck/lcd-avr-i2c/blob/main/src/i2c.c

void write_RTC(int addr, int data) {
    i2c_start(RTC_ADDR);

    TWDR0 = addr;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
    if ((TWSR0 & 0xF8) != TW_MT_DATA_ACK) {
        i2c_stop();
        return;
    }

    TWDR0 = data;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
    if ((TWSR0 & 0xF8) != TW_MT_DATA_ACK) {
        i2c_stop();
        return;
    }

    i2c_stop();
}

char read_RTC(int addr) {
    i2c_start(RTC_ADDR);

    TWDR0 = addr;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
    if ((TWSR0 & 0xF8) != TW_MT_DATA_ACK) {
        i2c_stop();
        return 0;
    }

    TWCR0 = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
    if ((TWSR0 & 0xF8) != TW_REP_START) {
        i2c_stop();
        return 0;
    }

    TWDR0 = RTC_ADDR | 1;

    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
    if ((TWSR0 & 0xF8) != TW_MR_SLA_ACK) {
        i2c_stop();
        return 0;
    }

    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)));
    if ((TWSR0 & 0xF8) != TW_MR_DATA_NACK) {
        i2c_stop();
        return 0;
    }
    char temp = TWDR0;

    i2c_stop();
    return temp;
}

void update_LCD_time(void) {
    char temp;
    temp = read_RTC(1);
    int m = (temp & 0b1111) + ((temp & 0b1110000) >> 4) * 10;
    temp = read_RTC(2);
    int h = (temp & 0b1111) + ((temp & 0b1110000) >> 4) * 10;

    char line[17];
    char tmp[12];

    lcd_move_cursor(0, 0);
    snprintf(tmp, sizeof tmp, "%02d:%02d", h, m);
    snprintf(line, sizeof line, "%-16s", tmp);
    lcd_print(line);

    lcd_move_cursor(1, 0);
    snprintf(tmp, sizeof tmp, "str:%d", hall);
    snprintf(line, sizeof line, "%-16s", tmp);
    lcd_print(line);
}

void update_LCD_draw(void) {
    char line[17];
    char tmp[12];

    lcd_move_cursor(0, 0);
    snprintf(line, sizeof line, "%-16s", "Draw");
    lcd_print(line);

    lcd_move_cursor(1, 0);
    snprintf(tmp, sizeof tmp, "str:%d", hall);
    snprintf(line, sizeof line, "%-16s", tmp);
    lcd_print(line);
}

void init_hall_sensor() {

    DDRC &= ~(1 << HALL); 
}

bool read_hall_sensor() {

    return (PINC & (1 << HALL));
}

void init_button_interrupts() {

    DDRD &= ~((1 << DRAW) | (1 << TIME)); 

    PORTD |= (1 << DRAW) | (1 << TIME);

    EICRA |= (1 << ISC01) | (1 << ISC11);
    EICRA &= (uint8_t)~((1 << ISC00) | (1 << ISC10));

    EIMSK |= (1<<INT1) | (1<<INT0);

}

void ADC_Init()
{
    DDRC &= ~(1<<HALL);

    ADCSRA = 0x87;			/* Enable ADC, fr/128  */
	ADMUX = 0x40;			/* Vref: Avcc, ADC channel: 0 */
	
}

int ADC_Read(char channel)
{
	int Ain,AinLow;
	
	ADMUX=ADMUX|(channel & 0x0f);	
    ADCSRA |= (1<<ADIF);   
	ADCSRA |= (1<<ADSC);		
	while((ADCSRA&(1<<ADIF))==0);	
	
	_delay_us(10);
	AinLow = (int)ADCL;		
	Ain = (int)ADCH*256;		
		
	Ain = Ain + AinLow;				
	return(Ain);			
}

int main(void) {
    i2c_init();
    i2c_scan();   // find the LCD address first
    lcd_init(); 
    init_hall_sensor(); 
    ADC_Init();
    init_button_interrupts();

    sei(); 

    char mins = read_RTC(0);
    mins &= ~(1 << 7);
    write_RTC(0, mins);

    char hours = read_RTC(2);
    hours &= ~(1 << 6);
    write_RTC(2, hours);

    while (1) {
        hall = ADC_Read(1);

        if (mode_banner != 0) {
            uint8_t b = mode_banner;
            mode_banner = 0;
            lcd_move_cursor(0, 0);
            if (b == 1) {
                lcd_print("Time            ");
            } else {
                lcd_print("Draw            ");
            }
            lcd_move_cursor(1, 0);
            lcd_print("                ");
            _delay_ms(600);
        }

        if (time_mode_active) {
            update_LCD_time();
        } else {
            update_LCD_draw();
        }
    }
    return 0;
}


ISR(INT0_vect) {
    draw_mode_active = true;
    time_mode_active = false;
    mode_banner = 2;
}

ISR(INT1_vect) {
    time_mode_active = true;
    draw_mode_active = false;
    mode_banner = 1;
}
