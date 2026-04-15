<<<<<<< HEAD
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include <stdio.h>      
#include <util/delay.h> 
#include <util/twi.h>    

#include "i2c.h"
#include "lcd.h"


volatile bool time_mode_active = false;
volatile bool draw_mode_active = false;
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

    // Write first address
    TWDR = addr;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return;

    // Write first data
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != TW_MT_DATA_ACK)
        return;

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

char read_RTC(int addr) {
    i2c_start(RTC_ADDR);

    // Write first address
    TWDR = addr;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != TW_MT_DATA_ACK){
        return 0;
    }

    // Repeated start
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // Wait for start
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != TW_REP_START){
        return 0;
    }

    // Send slave Address
    TWDR = RTC_ADDR | 1;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != TW_MR_SLA_ACK){
        return 0;
    }

    TWCR = (1 << TWINT) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != TW_MR_DATA_NACK){
        return 0;
    }
    char temp = TWDR;

    i2c_stop();
    return temp;
}

void update_LCD_time() {
        // Minutes
        lcd_move_cursor(0, 0);
        char temp;
        temp = read_RTC(1);
        int m = (temp & 0b1111) + ((temp & 0b1110000) >> 4) * 10;

        // Hours
        temp = read_RTC(2);
        int h = (temp & 0b1111) + ((temp & 0b1110000) >> 4) * 10;

        char buf[32];   
        sprintf(buf, "%02d:%02d magnetic field:%d", h, m, hall);
    
        lcd_print(buf);
    }

void update_LCD_draw() {
        // Minutes
        lcd_move_cursor(0, 0);
        char temp;
        temp = read_RTC(1);
        int m = (temp & 0b1111) + ((temp & 0b1110000) >> 4) * 10;

        // Hours
        temp = read_RTC(2);
        int h = (temp & 0b1111) + ((temp & 0b1110000) >> 4) * 10;

        char buf[32];   
        sprintf(buf, "%02d:%02d magnetic field:%d", h, m, hall);
    
        lcd_print(buf);
}

void init_hall_sensor() {
    //PC1 is the input for the hall sensor. 
    DDRC &= ~(1 << HALL); 
}

bool read_hall_sensor() {
    //return whether or not the hall sensor reads a magnetic field or not.  
    return (PINC & (1 << HALL));
}

void init_button_interrupts() {
    //PD2(INT0) and PD3(INT1) are input buttons 
    DDRD &= ~((1 << DRAW) | (1 << TIME)); 
    //Enable Pull up resistors 
    PORTD |= (1 << DRAW) | (1 << TIME);

    //Interrupt Sense Control (interrupts activate on any logical change)
    EICRA |= (1<<ISC10) | (1<<ISC00); 
    //External Interrupt Mask Register (turns these external pins on)
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
	
	ADMUX=ADMUX|(channel & 0x0f);	/* Set input channel to read */
    ADCSRA |= (1<<ADIF);   // add this line — clears flag by writing 1
	ADCSRA |= (1<<ADSC);		/* Start conversion */
	while((ADCSRA&(1<<ADIF))==0);	/* Monitor end of conversion interrupt */
	
	_delay_us(10);
	AinLow = (int)ADCL;		/* Read lower byte*/
	Ain = (int)ADCH*256;		/* Read higher 2 bits and 
					Multiply with weight */
	Ain = Ain + AinLow;				
	return(Ain);			/* Return digital value*/
}

int main(void) {
    i2c_init();
    i2c_scan();   // find the LCD address first
    lcd_init(); 
    init_hall_sensor(); 
    ADC_Init();
    init_button_interrupts(); 

    sei(); 

    // We need to alter CH and 24 hour bits without chaning time
    char mins = read_RTC(0);
    mins &= ~(1 << 7);
    write_RTC(0, mins);

    char hours = read_RTC(2);
    hours &= ~(1 << 6);
    write_RTC(2, hours);

    while(1){
        hall = ADC_Read(1);
        if (time_mode_active) {
            update_LCD_time();
        } else {
            update_LCD_draw();
        }
    }
    return 0;
}


ISR(INT0_vect) {
    time_mode_active = true;
    draw_mode_active = false;
}

ISR(INT1_vect) {
    draw_mode_active = true;  
    time_mode_active = false;
}

=======
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
>>>>>>> 8983ff0 (solenoid driver working)
