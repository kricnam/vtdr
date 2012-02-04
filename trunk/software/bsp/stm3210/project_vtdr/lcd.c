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

u_char lcd_bit_reverse(u_char byte)
{
    u_char newvalue=0;
    u_char temp = byte;
    newvalue = ((temp << 7)&0x80);
    newvalue |= ((temp >> 7)&0x01); 
    newvalue |= ((byte >> 5)&0x02);
    newvalue |= ((byte >> 3)&0x04);
    newvalue |= ((byte >> 1)&0x08);
    newvalue |= ((byte << 1)&0x10);
    newvalue |= ((byte << 3)&0x20);
    newvalue |= ((byte << 5)&0x40);
               
   	return newvalue;
}

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
    GPIO_Init(lcd_gpio_ctrl, &GPIO_InitStructure);
    
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
	volatile int m;
  for(int i =0;i<n;i++)
  {
    for(int j=0;j<10;j++)
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
	lcd_set_date_port_read();
	GPIO_SetBits(lcd_gpio_ctrl,lcd_RW);
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_A0 );

	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
        lcd_delay(10);  
        GPIO_Write(lcd_gpio_data,(GPIO_ReadOutputData(lcd_gpio_data) & 0xFF00)|(0x00FF));
	value = GPIO_ReadInputData(lcd_gpio_data);
	value = lcd_bit_reverse(value & 0x00FF);
	lcd_delay(2);
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
	lcd_set_date_port_write();
	return (unsigned char)(value & 0x00FF);
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
	uint16_t pinEnable;

	pinEnable  = (bank == 0)?  lcd_E1 : lcd_E2 ;
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(10);
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_A0 | lcd_RW);
	lcd_delay(10);
	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
	u_char value = lcd_bit_reverse(cCmd | oprand);
	GPIO_Write(lcd_gpio_data,((GPIO_ReadOutputData(lcd_gpio_data) & 0xFF00)|(value & 0x00FF)));
	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
	lcd_delay(10);
	GPIO_ResetBits(lcd_gpio_ctrl,pinEnable);
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
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_RW);
	GPIO_SetBits(lcd_gpio_ctrl,lcd_A0 );
	data = lcd_bit_reverse(data);
	GPIO_SetBits(lcd_gpio_ctrl,pinEnable);
        lcd_delay(10);
	GPIO_Write(lcd_gpio_data,((GPIO_ReadOutputData(lcd_gpio_data) & 0xFF00)|(data & 0x00FF)));
	lcd_delay(10);
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
	GPIO_ResetBits(lcd_gpio_reset,lcd_RST);
	lcd_delay(1);
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

void lcd_set_column(char nCol)
{
	lcd_detect_write_cmd(0,Column_Set,nCol & 0x7F);
}

void lcd_write_matrix(u_char row,u_char column,FONT_MATRIX *pt)
{
	    u_char i;
	    u_char  tempCol;
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
FINSH_FUNCTION_EXPORT(lcd, set led[0 - 1] on[1] or off[0].)
#endif

