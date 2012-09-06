/*
 * gpio_drv.c
 *
 *  Created on: Sep 5, 2012
 *      Author: mxx
 */
#include <rtthread.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "gpio_drv.h"


#define speed_plus_pin  (GPIO_Pin_11)
#define speed_plus_port (GPIOC)

void rt_hw_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

    GPIO_InitStructure.GPIO_Pin   = speed_plus_pin;
    GPIO_Init(speed_plus_port, &GPIO_InitStructure);
}
