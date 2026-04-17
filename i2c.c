#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdint.h>

const uint8_t MAX_RETRIES = 5;

void i2c_init(void) {
	// Only configure the clock
	PRR0 &= ~(1 << PRTWI0);
	TWSR0 = 0x00;
	TWBR0 = 72;	// TWBR = ( (cpu_freq / scl_freq) - 16 ) / 2 / 4^twps
	TWCR0 |= (1 << TWEN);
}

uint8_t i2c_send(uint8_t data) {
	uint8_t retry_count = 0;

	while (retry_count < MAX_RETRIES) {

		TWDR0 = data;
		TWCR0 = (1 << TWINT) | (1 << TWEN);
		i2c_wait();

		if ((TWSR0 & 0xF8) == TW_MT_DATA_ACK){
			return 1;
		}

		retry_count++;
	}
	
	return 0;
}

uint8_t i2c_start(uint8_t address) {
	uint8_t retry_count = 0;

	while (retry_count < MAX_RETRIES) {
		TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
		i2c_wait();

		if (((TWSR0 & 0xF8) != TW_START) && ((TWSR0 & 0xF8) != TW_REP_START)){
			retry_count++;
			continue;
		}

		TWDR0 = address;
		TWCR0 = (1 << TWINT) | (1 << TWEN);
		i2c_wait();

		uint8_t status = (TWSR0 & 0xF8);
		if (status == TW_MT_SLA_ACK || status == TW_MR_SLA_ACK) {
			return 1;
		}

		retry_count++;
	}

	return 0;
}

void i2c_stop(void) {
	TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

void i2c_wait (void ) {
	while (!(TWCR0 & (1 << TWINT))) {}
}

uint8_t i2c_write_register(uint8_t addr7, uint8_t reg, uint8_t data) {
	if (!i2c_start((addr7 << 1) | 0)) {
		i2c_stop();
		return 0;
	}

	if (!i2c_send(reg)) {
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

//read one byte
uint8_t i2c_read(uint8_t* data, uint8_t send_ack) {
    TWCR0 = (1 << TWINT) | (1 << TWEN) | (send_ack ? (1 << TWEA) : 0);
    i2c_wait();
    
    uint8_t status = TWSR0 & 0xF8;
    if (send_ack && status != TW_MR_DATA_ACK) {
        return 0;
    }
    
    if (!send_ack && status != TW_MR_DATA_NACK) {
        return 0;
    }
    
    *data = TWDR0;
    return 1;
}

uint8_t i2c_read_registers(uint8_t addr7, uint8_t reg, uint8_t* buf, uint8_t len) {
    if (!i2c_start((addr7 << 1) | 0)) {
        i2c_stop();
        return 0;
    }
    
    if(!i2c_send(reg)) {
        i2c_stop();
        return 0;
    }
    
    if(!i2c_start((addr7 << 1) | 1)) {
        i2c_stop();
        return 0;
    }
    
    for (uint8_t i = 0; i < len; i++) {
        uint8_t ack = (i < len - 1) ? 1 : 0;
        if (!i2c_read(&buf[i], ack)) {
            i2c_stop();
            return 0;
        }
    }
    
    i2c_stop();
    return 1;
}