#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "i2c.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include <stdio.h>      
#include <util/delay.h> 
#include <util/twi.h>    

uint8_t LCD_ADDR = 0; 

void i2c_scan(void) {
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        if (addr == 0x68) {
            continue;
        }
        uint8_t result = i2c_start(addr << 1);
        i2c_stop();
        if (result == 1) {
            LCD_ADDR = addr << 1;
            break;
        }
    }
    if (LCD_ADDR == 0) {
        LCD_ADDR = (uint8_t)(0x27 << 1);
    }
}

void i2c_init(void) {
	DDRC &= ~((1 << PC4) | (1 << PC5));
	PORTC |= (1 << PC4) | (1 << PC5);

	TWSR0 = 0x00;
	TWBR0 = 72;
}

uint8_t i2c_start(uint8_t address) {
	uint8_t status;		/* Declare variable */
    TWCR0=(1<<TWSTA)|(1<<TWEN)|(1<<TWINT); /* Enable TWI, generate START */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */
    if(status!=0x08)		/* Check weather START transmitted or not? */
    return 0;			/* Return 0 to indicate start condition fail */
    TWDR0=address;		/* Write SLA+W in TWI data register */
    TWCR0=(1<<TWEN)|(1<<TWINT);	/* Enable TWI & clear interrupt flag */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */	
    if(status==0x18)		/* Check for SLA+W transmitted &ack received */
    return 1;			/* Return 1 to indicate ack received */
    if(status==0x20)		/* Check for SLA+W transmitted &nack received */
    return 2;			/* Return 2 to indicate nack received */
    else
    return 3;			/* Else return 3 to indicate SLA+W failed */
}

uint8_t i2c_repeated_start(uint8_t address) /* I2C repeated start function (SLA+R)*/ 
{
    uint8_t status;		/* Declare variable */
    TWCR0=(1<<TWSTA)|(1<<TWEN)|(1<<TWINT);/* Enable TWI, generate start */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */
    if(status!=0x10)		/* Check for repeated start transmitted */
    return 0;			/* Return 0 for repeated start condition fail */
    TWDR0=address;		/* Write SLA+R in TWI data register */
    TWCR0=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */
    if(status==0x40)		/* Check for SLA+R transmitted &ack received */
    return 1;			/* Return 1 to indicate ack received */
    if(status==0x48)		/* Check for SLA+R transmitted &nack received */
    return 2;			/* Return 2 to indicate nack received */
    else
    return 3;			/* Else return 3 to indicate SLA+W failed */
}

uint8_t i2c_write(uint8_t data)	/* I2C write function */
{
    uint8_t status;		/* Declare variable */
    TWDR0=data;			/* Copy data in TWI data register */
    TWCR0=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR0&0xF8;		/* Read TWI status register */
    if(status==0x28)		/* Check for data transmitted &ack received */
    return 0;			/* Return 0 to indicate ack received */
    if(status==0x30)		/* Check for data transmitted &nack received */
    return 1;			/* Return 1 to indicate nack received */
    else
    return 2;			/* Else return 2 for data transmission failure */
}

char i2c_read_ack()		/* I2C read ack function */
{
    TWCR0=(1<<TWEN)|(1<<TWINT)|(1<<TWEA); /* Enable TWI, generation of ack */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    return TWDR0;			/* Return received data */
}

char i2c_read_nack()		/* I2C read nack function */
{
    TWCR0=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR0&(1<<TWINT)));	/* Wait until TWI finish its current job */
    return TWDR0;		/* Return received data */
} 

void i2c_stop(void)			/* I2C stop function */
{
    TWCR0=(1<<TWSTO)|(1<<TWINT)|(1<<TWEN);/* Enable TWI, generate stop */
    while(TWCR0&(1<<TWSTO));	/* Wait until stop condition execution */
}

void send_over_twi(uint8_t data) {

	i2c_start(LCD_ADDR);
	i2c_write(data);
	i2c_stop();
}
