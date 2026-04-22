#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>


#include "i2c.h"
#include "solenoid.h"
#include "rtc.h"
#include "uart.h"
#include "lcd.h"

volatile bool time_mode_active = true;
volatile uint8_t mode_banner = 0;
volatile uint8_t draw_button_event = 0;
int hall = 0;

#define HALL PC1
#define DRAW PD2

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

bool read_hall_sensor() {
    return (PINC & (1 << HALL));
}

void delay_ms_var(uint16_t ms) {
    while (ms--) {
        _delay_ms(1);

    }
}

void update_lcd_status(int hall_value) {
    char line[17];
    char tmp[17];

    lcd_move_cursor(0, 0);
    if (time_mode_active) {
        snprintf(line, sizeof line, "%-16s", "Time");
    } else {
        snprintf(line, sizeof line, "%-16s", "Draw");
    }
    lcd_print(line);

    lcd_move_cursor(1, 0);
    snprintf(tmp, sizeof tmp, "str:%d", hall_value);
    snprintf(line, sizeof line, "%-16s", tmp);
    lcd_print(line);
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

void delay_ms_checked(uint16_t ms) {
    while (ms--) {
        _delay_ms(1);
        if(!time_mode_active) return;

    }
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

ISR(INT0_vect) {
    draw_button_event = 1;
}
