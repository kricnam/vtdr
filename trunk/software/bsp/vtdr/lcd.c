/*
 * File      : lcd.c
 */
#include <rtthread.h>
#include <lcd.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>


#define lcd_data 					RCC_APB2Periph_GPIOB
#define lcd_gpio_data				GPIOB
//#define lcd_data_pin                ((uint16_t)0xFF00)
#define lcd_data_pin                (GPIO_Pin_15)
#define lcd_clk_pin                (GPIO_Pin_13)

#if 0
#define lcd_ctrl					RCC_APB2Periph_GPIOC
#define lcd_gpio_ctrl              GPIOC
#define lcd_A0						(GPIO_Pin_8)
#define lcd_E1						(GPIO_Pin_7)
#define lcd_E2						(GPIO_Pin_6)
#define lcd_RW						(GPIO_Pin_9)
#endif

#define lcd_bk_ctrl				    GPIOB
#define lcd_BK						(GPIO_Pin_8)

#define lcd_ctrl					RCC_APB2Periph_GPIOD
#define lcd_gpio_ctrl				GPIOD
#define lcd_CSB                 (GPIO_Pin_0)
#define lcd_RST						(GPIO_Pin_1)
#define lcd_A0                      (GPIO_Pin_3)
#define Delay_us           1

enum LCD_CMD
{
	Display_Off = 0xAE,
	Display_On = 0xAF,
	Start_Line = 0x40,
	Static_Drive = 0xA4,
	Column_Set = 0x10,
	Page_Set = 0xB8,
	Duty_Set = 0xA8,
	ADC_Select = 0xA0,
	Read_Modi = 0xE0,
	Read_ModiEnd = 0xEE,
	Reset = 0xE2,
	Power_Set = 0x2f
};

typedef enum DATA_CMD
{
	CMD,
	DATA
}COM_STATUS;

// delay us
void lcd_delay(int n)
{
volatile int m,i,j;
  for( i =0;i<n;i++)
  {
    for( j=0;j<3;j++)
    {
    	m++;
    }
  }
}

void rt_hw_lcd_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD ,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = lcd_data_pin | lcd_clk_pin;
    GPIO_Init(lcd_gpio_data, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    //GPIO_InitStructure.GPIO_Pin   = lcd_A0 | lcd_E1 | lcd_E2 | lcd_RW; ;
    GPIO_InitStructure.GPIO_Pin   = lcd_A0 | lcd_CSB | lcd_RST;
    GPIO_Init(lcd_gpio_ctrl, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = lcd_BK;
    GPIO_Init(lcd_bk_ctrl, &GPIO_InitStructure);
    lcd_Reset();
    lcd_delay(30);
    rt_hw_lcd_on();

}

void lcd_senddata(u8 value)
{
	unsigned char s,temp;
	int i;
	GPIO_ResetBits(lcd_gpio_data,lcd_clk_pin);
	lcd_delay(Delay_us);
	s=value;
	for(i=8;i>0;i--)
	{
		GPIO_ResetBits(lcd_gpio_data,lcd_clk_pin);
		lcd_delay(Delay_us);
		temp=s & 0x80;
		if(temp){
			GPIO_SetBits(lcd_gpio_data, lcd_data_pin);
		}else{
			GPIO_ResetBits(lcd_gpio_data, lcd_data_pin);
		}
		GPIO_SetBits(lcd_gpio_data, lcd_clk_pin);
		lcd_delay(Delay_us);
		s=s<<1;
	}

}

void lcd_write_cmdordata(COM_STATUS bank, unsigned char cCmd, int oprand)
{
	rt_uint8_t value = cCmd | oprand;
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_CSB);
	if(bank)
	{
		GPIO_SetBits(lcd_gpio_ctrl,lcd_A0);
	}
	else
	{
		GPIO_ResetBits(lcd_gpio_ctrl,lcd_A0);
	}
	lcd_senddata(value);
	GPIO_SetBits(lcd_gpio_ctrl ,lcd_CSB);
}

void lcd_set_column(char nCol)
{
    nCol = nCol&0x83;
	lcd_write_cmdordata(CMD,Column_Set,(nCol&0xf0)>>4);
	lcd_write_cmdordata(CMD,0,(nCol&0x0f));

}

void lcd_Reset(void)
{
	GPIO_SetBits(lcd_bk_ctrl,lcd_BK);
	lcd_delay(30);
	GPIO_ResetBits(lcd_gpio_ctrl,lcd_RST);
	lcd_delay(60);
	GPIO_SetBits(lcd_gpio_ctrl,lcd_RST);
	lcd_delay(30);
}

void rt_hw_lcd_clear(unsigned char patten)
{
	rt_uint8_t x,y;
	for (x = 0; x < 8; x++)
	{
		lcd_set_column(0);
		lcd_write_cmdordata(CMD, Page_Set, x);
		for (y = 0; y < 132; y += 1)
		{
			lcd_write_cmdordata(DATA,patten,0x00);
		}
	}

}

void rt_hw_lcd_on(void)
{
	//lcd_Reset();

	    lcd_write_cmdordata(CMD,Reset,0);

	    lcd_write_cmdordata(CMD, Display_On, 0);

	    lcd_write_cmdordata(CMD, Start_Line, 0);

	    lcd_write_cmdordata(CMD, Static_Drive, 0);

	    lcd_write_cmdordata(CMD,0xa2,0);

	    //lcd_write_cmdordata(CMD, Duty_Set, 1);

	    lcd_write_cmdordata(CMD, ADC_Select, 0);

	    lcd_write_cmdordata(CMD,Power_Set,0);
	    lcd_delay(100);
		rt_hw_lcd_clear(0);




}

void rt_hw_lcd_off(void)
{
    lcd_write_cmdordata(CMD, Display_Off, 0);
}

void lcd_write_matrix(rt_uint8_t row,rt_uint8_t column,FONT_MATRIX *pt)
{
	rt_uint8_t i,x,y;
	rt_uint8_t  temp;
	lcd_write_cmdordata(CMD, Page_Set, row);
	lcd_set_column(0);

	for (i = 0; i < 132; i++)
	{
		lcd_write_cmdordata(DATA, (*(unsigned char*) pt++),0);
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static rt_uint8_t lcd_inited = 0;
void lcd(rt_uint32_t value)
{
	if (value==0)
	{
		lcd_write_cmdordata(CMD, Display_Off, 0);

	}
	else
	{
		lcd_write_cmdordata(CMD, Display_On, 0);
	}
}

void lcd_bk(rt_uint32_t value)
{
	if (value==0)
	{
		GPIO_ResetBits(lcd_bk_ctrl,lcd_BK);
	}
	else
	{
		GPIO_SetBits(lcd_bk_ctrl,lcd_BK);
	}
}
void lcd_clear(rt_uint8_t value)
{
	rt_hw_lcd_clear(value);
}

FINSH_FUNCTION_EXPORT(lcd, set lcd on[1] or off[0].)
FINSH_FUNCTION_EXPORT(lcd_bk, set lcd backlight on[1] or off[0].)
FINSH_FUNCTION_EXPORT(lcd_clear, set patten x.)

#endif

