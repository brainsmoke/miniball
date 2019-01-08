#ifndef LIPO_H
#define LIPO_H

#include <stdint.h>

uint16_t get_lipo_voltage(void);
int lipo_voltage_ok(void);
int lipo_voltage_ok_check(void);

void lipo_init(void);

#endif // LIPO_H
