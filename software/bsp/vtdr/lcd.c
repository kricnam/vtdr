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


#define lcd_data 					RCC_APB2Periph_GPIOB
#define lcd_gpio_data				GPIOB
#define lcd_data_pin                ((uint16_t)0xFF00)

#define lcd_ctrl					RCC_APB2Periph_GPIOC
#define lcd_gpio_ctrl              GPIOC
#define lcd_A0						(GPIO_Pin_8)
#define lcd_E1						(GPIO_Pin_7)
#define lcd_E2						(GPIO_Pin_6)
#define lcd_RW						(GPIO_Pin_9)

#define lcd_bk_ctrl				    GPIOB
#define lcd_BK						(GPIO_Pin_5)

#define lcd_reset					RCC_APB2Periph_GPIOA
#define lcd_gpio_reset				GPIOA
#define lcd_RST						(GPIO_Pin_8)


enum LCD_CMD
{
	Display_Off = 0xAE,
	Display_On = 0xAF,
	Start_Line = 0xC0,
	Static_Drive = 0xA4,
	Column_Set = 0x00,
	Page_Set = 0xB8,
	Duty_Set = 0xA8,
	ADC_Select = 0xA0,
	Read_Modi = 0xE0,
	Read_ModiEnd = 0xEE,
	Reset = 0xE2
};


void rt_hw_lcd_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = lcd_data_pin;
    GPIO_Init(lcd_gpio_data, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin   = lcd_A0 | lcd_E1 | lcd_E2 | lcd_RW ;
    GPIO_Init(lcd_gpio_ctrl, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = lcd_BK;
    GPIO_Init(lcd_bk_ctrl, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin   = lcd_RST;
    GPIO_Init(lcd_gpio_reset, &GPIO_InitStructure);

    lcd_Reset();

}

void lcd_set_date_port_read(void)
{
    return;
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = lcd_data_pin;
    GPIO_Init(lcd_gpio_data, &GPIO_InitStructure);
}

void lcd_set_date_port_write(void)
{
    return;
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = lcd_data_pin;
    GPIO_Init(lcd_gpio_data, &GPIO_InitStructure);
}

// delay ms
void lcd_delay(int n)
{
volatile int m,i,j;
  for( i =0;i<n;i++)
  {
    for( j=0;j<10;j++)
    {
    	m++;
    }
  }	
}

unsigned char lcd_read_status(int bank)
{
	uint16_t pinEnable;
	uint16_t value = 0xFFFF;
	pinEnable  = (bank == 0)?  lcd_E1 : lcd_E2 ;
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(5);
	GPIO_SetBits(lcd_gpio_ctrl,lcd_RW);
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_A0 );
	GPIO_Write(lcd_gpio_data,(GPIO_ReadOutputData(lcd_gpio_data) & 0x00FF)|(0xFF00));
	lcd_delay(5);
	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
    lcd_delay(5);
    value = GPIO_ReadInputData(lcd_gpio_data);
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);

	return (unsigned char)((value>>8) & 0x00FF);
}

void lcd_wait(int bank)
{
	unsigned char status = 0xFF;
	do
	{
		lcd_delay(3);
		status = lcd_read_status(bank);
	}while(status & 0x80);
	return;
}

void lcd_write_cmd(int bank, unsigned char cCmd, int oprand)
{
	rt_uint16_t pinEnable;

	pinEnable  = (bank == 0)?  lcd_E1 : lcd_E2 ;
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(5);
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_A0 | lcd_RW);
	lcd_delay(5);
	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(5);
	rt_uint8_t value = cCmd | oprand;
	GPIO_Write(lcd_gpio_data,((GPIO_ReadOutputData(lcd_gpio_data) & 0x00FF)|((value<<8) & 0xFF00)));
	lcd_delay(5);
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(5);
}

void lcd_detect_write_cmd(int bank, unsigned char cCmd, int oprand)
{
	lcd_wait(bank);
	lcd_write_cmd(bank, cCmd,oprand);
}

void lcd_write_data(int bank, unsigned char data)
{
	uint16_t pinEnable;
	pinEnable  = (bank == 0)?  lcd_E1 : lcd_E2 ;
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(5);
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_RW|lcd_A0);
	lcd_delay(5);
	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(5);
	GPIO_Write(lcd_gpio_data,((GPIO_ReadOutputData(lcd_gpio_data) & 0x00FF)|((data<<8) & 0xFF00)));
    lcd_delay(5);
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
}

void lcd_detect_write_data(int bank, unsigned char data)
{
	lcd_wait(bank);
	lcd_write_data(bank, data);
}

void lcd_Reset(void)
{
	GPIO_SetBits(lcd_gpio_reset,lcd_RST);
	lcd_delay(10);
	GPIO_ResetBits(lcd_gpio_reset,lcd_RST);
	lcd_delay(60);
	GPIO_SetBits(lcd_gpio_reset,lcd_RST);
	lcd_delay(30);
}
#define RESET1520       0xE2
#define ON1520		0xAF
#define OFF1520         0xAE
#define SLEEP		0xA5
#define WAKEUP		0xA4
#define SETDUTY32	0xA9
#define SETDUTY16	0xA8
#define ADC1_60		0xA0
#define ADC60_1		0xA1
#define SET_PAGE	0xB8
#define OVERWRITE	0xE0
#define QUIT_REWRITE    0xEE
#define START_LINE	0xC0   //0xC0~0xFF
#define Y_ADDRESS	0x40   //select segment(0~63)-CS1 (0~63)-CS2

//detect_write(RESET1520,0);
//detect_write(WAKEUP,0);
//detect_write(SETDUTY32,0);
//detect_write(ADC1_60,0);
//detect_write(START_LINE|0,0);
//detect_write(ON1520,0);
//    clear1520(0x00);
void rt_hw_lcd_on(void)
{
	lcd_Reset();

		lcd_detect_write_cmd(0,Reset,0);
		lcd_detect_write_cmd(1,Reset,0);

		lcd_detect_write_cmd(0, Static_Drive, 0);
		lcd_detect_write_cmd(1, Static_Drive, 0);

		lcd_detect_write_cmd(0, Duty_Set, 1);
		lcd_detect_write_cmd(1, Duty_Set, 1);

		lcd_detect_write_cmd(0, ADC_Select, 0);
		lcd_detect_write_cmd(1, ADC_Select, 0);

		lcd_detect_write_cmd(0, Start_Line, 0);
		lcd_detect_write_cmd(1, Start_Line, 0);

		lcd_detect_write_cmd(0, Display_On, 0);
		lcd_detect_write_cmd(1, Display_On, 0);


//		lcd_detect_write_cmd(bank, Column_Set, 0);
//		lcd_detect_write_cmd(bank, Page_Set, 0);
//
//		lcd_detect_write_cmd(bank, Read_ModiEnd, 0);

	GPIO_SetBits(lcd_bk_ctrl,lcd_BK);

}

void rt_hw_lcd_off(void)
{

}

void lcd_set_column(char nCol)
{
	lcd_detect_write_cmd(0,Column_Set,nCol & 0x7F);
}

void lcd_write_matrix(rt_uint8_t row,rt_uint8_t column,FONT_MATRIX *pt)
{
	rt_uint8_t i;
	rt_uint8_t  tempCol;
	    lcd_set_column(column);

		for (i=0;i<24;i++)
		{
			lcd_detect_write_data(0, (*(unsigned char*)pt++));
		}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static rt_uint8_t lcd_inited = 0;
void lcd(rt_uint32_t lcd, rt_uint32_t value)
{

}
FINSH_FUNCTION_EXPORT(lcd, set lcd[0 - 1] on[1] or off[0].)
#endif

