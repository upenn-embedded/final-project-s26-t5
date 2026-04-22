#pragma once

#include <stdint.h>

/* DS3231 fixed 7-bit I2C address (0b1101000) */
#define DS3231_ADDR   0x68

/* ?? Register map ???????????????????????????????????????????????? */
#define DS3231_REG_SECONDS   0x00
#define DS3231_REG_MINUTES   0x01
#define DS3231_REG_HOURS     0x02

#define DS3231_REG_CONTROL   0x0E
#define DS3231_REG_STATUS    0x0F



#define DS3231_STATUS_OSF      (1 << 7)  // oscillator stop flag
#define DS3231_STATUS_EN32KHZ  (1 << 3)

typedef struct {
    uint8_t seconds;  // 0?59
    uint8_t minutes;  // 0?59
    uint8_t hours;    // 0?23  (driver always uses 24-hour mode)
} rtc_time_t;


uint8_t rtc_init(void);

uint8_t rtc_set_time(const rtc_time_t *t);

uint8_t rtc_get_time(rtc_time_t *t);

uint8_t rtc_get_status(uint8_t *status_byte);
