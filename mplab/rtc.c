#include "rtc.h"
#include "i2c.h"
#include <stdint.h>

static uint8_t bcd_to_dec(uint8_t bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

uint8_t rtc_init(void) {
    /*
     * Control register (0x0E):
     *   EOSC  = 0  ? oscillator always on
     *   BBSQW = 0  ? SQW disabled on battery
     *   CONV  = 0  ? no forced temp conversion
     *   RS2/1 = 11 ? 8.192 kHz (irrelevant in interrupt mode)
     *   INTCN = 1  ? INT/SQW pin acts as interrupt, not square-wave
     *   A2IE  = 0  ? alarm 2 interrupt disabled
     *   A1IE  = 0  ? alarm 1 interrupt disabled
     *
     * Encoded: 0b00011100 = 0x1C
     */
    if (!i2c_write_register(DS3231_ADDR, DS3231_REG_CONTROL, 0x1C))
        return 0;

    return 1;
}

uint8_t rtc_set_time(const rtc_time_t *t) {

    uint8_t buf[3];
    buf[0] = dec_to_bcd(t->seconds);
    buf[1] = dec_to_bcd(t->minutes);
    buf[2] = dec_to_bcd(t->hours);   // bit6=0 ? 24-hour mode


    if (!i2c_start((DS3231_ADDR << 1) | 0)) { i2c_stop(); return 0; }
    if (!i2c_send(DS3231_REG_SECONDS))       { i2c_stop(); return 0; }
    for (uint8_t i = 0; i < 3; i++) {
        if (!i2c_send(buf[i])) { i2c_stop(); return 0; }
    }
    i2c_stop();
    return 1;
}

uint8_t rtc_get_time(rtc_time_t *t) {
    uint8_t buf[7];

    if (!i2c_read_registers(DS3231_ADDR, DS3231_REG_SECONDS, buf, 3))
        return 0;

    t->seconds = bcd_to_dec(buf[0] & 0x7F);          // mask bit7 (always 0)
    t->minutes = bcd_to_dec(buf[1] & 0x7F);
    t->hours   = bcd_to_dec(buf[2] & 0x3F);          // mask 12/24 and AM/PM bits
    
    return 1;
}

uint8_t rtc_get_status(uint8_t *status_byte) {
    return i2c_read_registers(DS3231_ADDR, DS3231_REG_STATUS, status_byte, 1);
}
