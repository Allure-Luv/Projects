/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-28     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include "time.h"

#define LED0_PIN      GET_PIN(F, 9)

static void led_entry(void *parameter)
{
    while(1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}

int main(void)
{
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    rt_thread_t thread = rt_thread_create("led_t", led_entry, RT_NULL, 512, 25, 5);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("thread led_t startup error");
    }

    time_t now;

    while (1)
    {
        now = time(RT_NULL);
        LOG_I("time: %s\n", ctime(&now));
        rt_thread_mdelay(50000);
    }

    return RT_EOK;
}
