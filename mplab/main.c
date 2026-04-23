#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "i2c.h"
#include "solenoid.h"
#include "rtc.h"
#include "uart.h"
#include "lcd.h"

volatile bool time_mode_active = true;
volatile uint8_t mode_banner = 0;
volatile uint32_t g_ms_ticks = 0;
int hall = 0;

#define HALL PC1
#define DRAW PD2

//pin mappings
//PC5 -> SCL
//PC4 -> SDA

//to setup: lab bench, put 5V through both solenoid boards, set current limit to 2A

const uint8_t digits[11][3][5] = {
    //0
    {
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1}
    },
    //1
    {
        {0,0,0,0,0},
        {1,1,1,1,1},
        {0,0,0,0,0}
    },
    //2
    {
        {1,1,1,0,1},
        {1,0,1,0,1},
        {1,0,1,1,1}
    },
    //3
    {
        {1,0,1,0,1},
        {1,0,1,0,1},
        {1,1,1,1,1}
    },
    //4
    {
        {0,0,1,1,1},
        {0,0,1,0,0},
        {1,1,1,1,1}
    },
    
    //5
    {
        {1,0,1,1,1},
        {1,0,1,0,1},
        {1,1,1,0,1}
    },
    
    //6
    {
        {1,1,1,1,1},
        {1,0,1,0,1},
        {1,1,1,0,1}
    },
    
    //7
    {
        {0,0,0,0,1},
        {0,0,0,0,1},
        {1,1,1,1,1}
    },
    
    //8
    {
        {1,1,1,1,1},
        {1,0,1,0,1},
        {1,1,1,1,1}
    },
    
    //9
    {
        {0,0,1,1,1},
        {0,0,1,0,1},
        {1,1,1,1,1}
    },
    
    //:
    {
        {0,0,0,0,0},
        {0,1,0,1,0},
        {0,0,0,0,0}
    } 
};

void timer_init(void) {
    TCCR1A = 0;
    TCCR1B = 0;

    // CTC mode
    TCCR1B |= (1 << WGM12);

    // 16 MHz / 64 = 250 kHz
    // 250 counts = 1 ms -> OCR1A = 249
    OCR1A = 249;

    // Enable compare match interrupt
    TIMSK1 |= (1 << OCIE1A);

    // Start timer, prescaler 64
    TCCR1B |= (1 << CS11) | (1 << CS10);
}

bool read_hall_sensor() {
    return (PINC & (1 << HALL));
}

void delay_ms_var(uint16_t ms) {
    uint32_t start = g_ms_ticks;
    while ((uint32_t)(g_ms_ticks - start) < ms) {

    }
}

void delay_ms_checked(uint16_t ms) {
    uint32_t start = g_ms_ticks;
    while ((uint32_t)(g_ms_ticks - start) < ms) {
        if (!time_mode_active) return;
    }
}

void lcd_print_line(uint8_t line_num, const char *text) {
    char buf[17];

    // Left-justify and pad to exactly 16 chars
    snprintf(buf, sizeof(buf), "%-16s", text);

    lcd_move_cursor(line_num, 0);
    lcd_print(buf);
}

void update_lcd_status(uint16_t hall_value) {
    char buf[17];

    if (time_mode_active) {
        lcd_print_line(0, "Mode: Time");
    } else {
        lcd_print_line(0, "Mode: Draw");
    }

    snprintf(buf, sizeof(buf), "ADC: %-10u", hall_value);
    lcd_print_line(1, buf);
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
	
    //	ADMUX=ADMUX|(channel & 0x0f);
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);	
    ADCSRA |= (1<<ADIF);   
	ADCSRA |= (1<<ADSC);		
	while((ADCSRA&(1<<ADIF))==0);	
	
	_delay_us(10);
	AinLow = (int)ADCL;		
	Ain = (int)ADCH*256;		
		
	Ain = Ain + AinLow;				
	return(Ain);			
}

void init_buttons(void) {
    DDRD &= ~(1 << DRAW);   // input
    PORTD |= (1 << DRAW);   // pull-up enabled
}

void load_digit(uint8_t grid[3][5], uint8_t symbol) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 5; c++) {
            grid[r][c] = digits[symbol][r][c];
        }
    }
}

void show_digit(uint8_t symbol, uint16_t ms) {
    uint8_t grid[3][5];
    grid_clear(grid);
    load_digit(grid, symbol);
    grid_apply(grid);
    delay_ms_checked(ms);
}

int main(void) {
//    DDRB |= (1 << PB5);

    uart_init();
    i2c_init();
    solenoid_init(SOL0_ADDR);
    solenoid_init(SOL1_ADDR);
    rtc_init();
    init_buttons();
    ADC_Init();
    lcd_init();
    timer_init();
    
    sei();
    while (1) {
        hall = ADC_Read(1);
        printf("%d\n", hall);

        time_mode_active = (PIND & (1 << DRAW)) ? true : false;
        update_lcd_status(hall);

        rtc_time_t now;
        rtc_get_time(&now);

        if (time_mode_active) {
            uint8_t hours = now.hours % 12;
            if (hours == 0) hours = 12;

            uint8_t h_tens = hours / 10;
            uint8_t h_ones = hours % 10;
            uint8_t m_tens = now.minutes / 10;
            uint8_t m_ones = now.minutes % 10;

            show_digit(h_tens, 750);
            time_mode_active = (PIND & (1 << DRAW)) ? true : false;
            if (!time_mode_active) continue;

            show_digit(h_ones, 750);
            time_mode_active = (PIND & (1 << DRAW)) ? true : false;
            if (!time_mode_active) continue;

            show_digit(10, 600);
            time_mode_active = (PIND & (1 << DRAW)) ? true : false;
            if (!time_mode_active) continue;

            show_digit(m_tens, 750);
            time_mode_active = (PIND & (1 << DRAW)) ? true : false;
            if (!time_mode_active) continue;

            show_digit(m_ones, 750);
        } else {
            uint8_t grid[3][5] = {0};

            for (int c = 0; c < 5; c++) {
                grid_clear(grid);
                grid[0][c] = 1;
                grid[1][c] = 1;
                grid[2][c] = 1;
                grid_apply(grid);
                delay_ms_var(200);

                time_mode_active = (PIND & (1 << DRAW)) ? true : false;
                if (time_mode_active) break;
            }

            grid_clear(grid);
            grid_apply(grid);
        }
    }
}

ISR(TIMER1_COMPA_vect) {
    g_ms_ticks++;
}