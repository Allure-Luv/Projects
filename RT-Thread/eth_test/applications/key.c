#include <stdio.h>
#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "./key.h"

void key_init(void)
{
    rt_pin_mode(KEY0_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY1_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY2_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_UP, PIN_MODE_INPUT_PULLDOWN);
}

uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;     /* 按键松开标志, 静态局部变量，仅初始化一次，后续在原有基础上变化 */
    uint8_t keyval = 0;
    if (mode) key_up = 1;          /* 支持连续按下 */

    if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || WK_UP == 1))
    {
        rt_thread_mdelay(10);
        key_up = 0;
        if (KEY0 == 0) keyval = KEY0_PRES;
        if (KEY1 == 0) keyval = KEY1_PRES;
        if (KEY2 == 0) keyval = KEY2_PRES;
        if (WK_UP == 1) keyval = WKUP_PRES;
    }
    else if (KEY0 == 1 && KEY2 == 1 && KEY1 == 1 && WK_UP == 0)
    {
        key_up = 1;
    }
    return keyval;
}
