#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

#define LCD_ADDR 0x4E
void i2c_init(void) {
	//100000 = 16000000/(16+2(x)*1), assumign prescaler is 1, x = 72 
	//100kHz SCL frequency.
	// Only configure the clock
	TWBR = 72;	// TWBR = ( (cpu_freq / scl_freq) - 16 ) / 2 / 4^twps
}

uint8_t i2c_start(uint8_t address) {
	uint8_t status;		/* Declare variable */
    TWCR=(1<<TWSTA)|(1<<TWEN)|(1<<TWINT); /* Enable TWI, generate START */
    while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR&0xF8;		/* Read TWI status register */
    if(status!=0x08)		/* Check weather START transmitted or not? */
    return 0;			/* Return 0 to indicate start condition fail */
    TWDR=address;		/* Write SLA+W in TWI data register */
    TWCR=(1<<TWEN)|(1<<TWINT);	/* Enable TWI & clear interrupt flag */
    while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR&0xF8;		/* Read TWI status register */	
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
    TWCR=(1<<TWSTA)|(1<<TWEN)|(1<<TWINT);/* Enable TWI, generate start */
    while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR&0xF8;		/* Read TWI status register */
    if(status!=0x10)		/* Check for repeated start transmitted */
    return 0;			/* Return 0 for repeated start condition fail */
    TWDR=address;		/* Write SLA+R in TWI data register */
    TWCR=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR&0xF8;		/* Read TWI status register */
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
    TWDR=data;			/* Copy data in TWI data register */
    TWCR=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
    status=TWSR&0xF8;		/* Read TWI status register */
    if(status==0x28)		/* Check for data transmitted &ack received */
    return 0;			/* Return 0 to indicate ack received */
    if(status==0x30)		/* Check for data transmitted &nack received */
    return 1;			/* Return 1 to indicate nack received */
    else
    return 2;			/* Else return 2 for data transmission failure */
}

char i2c_read_ack()		/* I2C read ack function */
{
    TWCR=(1<<TWEN)|(1<<TWINT)|(1<<TWEA); /* Enable TWI, generation of ack */
    while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
    return TWDR;			/* Return received data */
}

char i2c_read_nack()		/* I2C read nack function */
{
    TWCR=(1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
    while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
    return TWDR;		/* Return received data */
}

void i2c_stop(void)			/* I2C stop function */
{
    TWCR=(1<<TWSTO)|(1<<TWINT)|(1<<TWEN);/* Enable TWI, generate stop */
    while(TWCR&(1<<TWSTO));	/* Wait until stop condition execution */
}

void send_over_twi(uint8_t data) {

	i2c_start(LCD_ADDR);
	i2c_write(data);
	i2c_stop();
}
