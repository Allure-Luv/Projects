#ifndef __KEY_H
#define __KEY_H

#include <stdio.h>
#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define KEY0_PIN        GET_PIN(E, 4)
#define KEY1_PIN        GET_PIN(E, 3)
#define KEY2_PIN        GET_PIN(E, 2)
#define KEY_UP          GET_PIN(A, 0)

#define KEY0            rt_pin_read(KEY0_PIN)
#define KEY1            rt_pin_read(KEY1_PIN)
#define KEY2            rt_pin_read(KEY2_PIN)
#define WK_UP           rt_pin_read(KEY_UP)

#define KEY0_PRES  1
#define KEY1_PRES  2
#define KEY2_PRES  3
#define WKUP_PRES  4


void key_init(void);
uint8_t key_scan(uint8_t mode);

#endif
