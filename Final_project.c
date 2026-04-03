#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

bool time_mode_active = false;
bool draw_mode_active = false;

//PORTC, PC4 
//100000 = 16000000/(16+2(x)*1), assumign prescaler is 1, x = 72 
//100kHz SCL frequency. 
void init_I2C() {
    TWBR = 72;          
    TWCR0 = (1 << TWEN); 
}

void get_time_from_RTC() {
    
}

void update_LCD_time(bool is_time_mode) {

}

void update_LCD_draw(bool is_draw_mode) {

}

void init_hall_sensor() {
    //PC0 is the input for the hall sensor. 
    DDRC &= ~(1 << DDC0); 
}

bool read_hall_sensor() {
    //return whether or not the hall sensor reads a magnetic field or not.  
    return (PINC & (1 << PINC0));
}

void init_button_interrupts() {
    //PD2(INT0) and PD3(INT1) are input buttons 
    DDRD &= ~((1 << DDD2) | (1 << DDD3)); 
    //Enable Pull up resistors 
    PORTD |= (1 << PORTD2) | (1 << PORTD3);

    //Interrupt Sense Control (interrupts activate on any logical change)
    EICRA |= (1<<ISC10) | (1<<ISC00); 
    //External Interrupt Mask Register (turns these external pins on)
    EIMSK |= (1<<INT1) | (1<<INT0);


}

ISR(INT0_vect) {
    time_mode_active = true;
    draw_mode_active = false;
}

ISR(INT1_vect) {
    draw_mode_active = true;  
    time_mode_active = false;
}

int main(void) {
    init_I2C(); 
    init_hall_sensor(); 
    init_button_interrupts(); 

    sei(); 

    while(1){
        bool field_detected = read_hall_sensor();
        if (time_mode_active) {
            get_time_from_RTC();
            update_LCD_time(true);
        } else {
            update_LCD_draw(true);
        }

    }
    
    return 0;
}