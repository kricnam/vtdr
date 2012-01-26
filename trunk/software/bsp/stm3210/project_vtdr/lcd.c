/*
 * File      : led.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rtthread.h>
#include <lcd.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#ifdef STM32_SIMULATOR
#define led1_rcc                    RCC_APB2Periph_GPIOA
#define led1_gpio                   GPIOA
#define led1_pin                    (GPIO_Pin_5)

#define led2_rcc                    RCC_APB2Periph_GPIOA
#define led2_gpio                   GPIOA
#define led2_pin                    (GPIO_Pin_6)
#else
#define lcd_data 					RCC_APB2Periph_GPIOC
#define lcd_gpio_data				GPIOC
#define lcd_data_pin                ((uint16_t)0x00FF)

#define lcd_ctrl					RCC_APB2Periph_GPIOB
#define lcd_gpio_ctrl               GPIOB
#define lcd_A0						(GPIO_Pin_8)
#define lcd_E1						(GPIO_Pin_9)
#define lcd_E2						(GPIO_Pin_10)
#define lcd_RW						(GPIO_Pin_11)
#define lcd_BK						(GPIO_Pin_5)

#define lcd_reset					RCC_APB2Periph_GPIOA
#define lcd_gpio_reset				GPIOA
#define lcd_RST						(GPIO_Pin_8)
#endif

enum LCD_CMD
{
	Display_Off = 0xAF,
	Display_On = 0xAE,
	Start_Line = 0xC0,
	Static_Drive = 0xA4,
	Column_Set = 0x00,
	Page_Set = 0xB8,
	Duty_Set = 0xA8,
	ADC_Select = 0xA0,
	Read_Modi = 0xE0,
	Read_ModiEnd = 0xEE
};

void rt_hw_lcd_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = lcd_data_pin;
    GPIO_Init(lcd_gpio_data, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = lcd_A0 | lcd_E1 | lcd_E2 | lcd_RW | lcd_BK;
    GPIO_Init(lcd_gpio_ctrl, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = lcd_RST;
    GPIO_Init(lcd_gpio_reset, &GPIO_InitStructure);

    lcd_Reset();

}

void lcd_delay(int n)
{
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<100;j++)
		{

		}
	}
}

void lcd_write_cmd(int bank, unsigned char cCmd, int oprand)
{
	uint16_t pinEnable;

	pinEnable  = (bank == 0)?  lcd_E1 : lcd_E2 ;

	GPIO_ResetBits(lcd_gpio_ctrl,lcd_A0 | lcd_RW);
	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
	GPIO_Write(lcd_gpio_data,((GPIO_ReadOutputData(lcd_gpio_data) & 0xFF00)|((cCmd|oprand) & 0x00FF)));
	lcd_delay(10);
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
}

void lcd_Reset(void)
{
	GPIO_SetBits(lcd_gpio_reset,lcd_RST);
	GPIO_ResetBits(lcd_gpio_reset,lcd_RST);
	lcd_delay(10);
	GPIO_SetBits(lcd_gpio_reset,lcd_RST);
}

void rt_hw_lcd_on(void)
{
	lcd_Reset();
	lcd_write_cmd(0,Display_Off,0);
	lcd_write_cmd(0,Start_Line,0);
	lcd_write_cmd(0,Static_Drive,0);
	lcd_write_cmd(0,Column_Set,0);
	lcd_write_cmd(0,Page_Set,0);
	lcd_write_cmd(0,Duty_Set,1);
	lcd_write_cmd(0,ADC_Select,0);
	lcd_write_cmd(0,Read_ModiEnd,0);

	GPIO_SetBits(lcd_gpio_ctrl,lcd_BK);

}

void rt_hw_lcd_off(void)
{

}

#ifdef RT_USING_FINSH
#include <finsh.h>
static rt_uint8_t lcd_inited = 0;
void lcd(rt_uint32_t lcd, rt_uint32_t value)
{

}
FINSH_FUNCTION_EXPORT(lcd, set led[0 - 1] on[1] or off[0].)
#endif

