
/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>
#include <application.h>

#include<stm32f10x_gpio.h>
#include<stm32f10x_dac.h>

#ifdef RT_USING_DFS
/* dfs init */
#include <dfs_init.h>
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#endif

#include "led.h"
#include "lcd.h"
#include "i2c_drv.h"
#include "menu.h"
//#include "font_lib.h"
#include "atmel_dataflash.h"
#include<rtdef.h>
#include<rtconfig.h>
#include<string.h>
unsigned char sinx[]={

	0x80,0x83,0x86,0x89,0x8d,0x90,0x93,0x96,
	0x99,0x9c,0x9f,0xa2,0xa5,0xa8,0xab,0xae,
	0xb1,0xb4,0xb7,0xba,0xbc,0xbf,0xc2,0xc5,
	0xc7,0xca,0xcc,0xcf,0xd1,0xd4,0xd6,0xd8,
	0xda,0xdd,0xdf,0xe1,0xe3,0xe5,0xe7,0xe9,
	0xea,0xec,0xee,0xef,0xf1,0xf2,0xf4,0xf5,
	0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,
	0xfd,0xfe,0xff,0xff,0xff,0xff,0xff,0xff,

	0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xfd,
	0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,
	0xf5,0xf4,0xf2,0xf1,0xef,0xee,0xec,0xea,
	0xe9,0xe7,0xe5,0xe3,0xe1,0xdf,0xdd,0xda,
	0xd8,0xd6,0xd4,0xd1,0xcf,0xcc,0xca,0xc7,
	0xc5,0xc2,0xbf,0xbc,0xba,0xb7,0xb4,0xb1,
	0xae,0xab,0xa8,0xa5,0xa2,0x9f,0x9c,0x99,
	0x96,0x93,0x90,0x8d,0x89,0x86,0x83,0x80,

	0x80,0x7c,0x79,0x76,0x72,0x6f,0x6c,0x69,
	0x66,0x63,0x60,0x5d,0x5a,0x57,0x55,0x51,
	0x4e,0x4c,0x48,0x45,0x43,0x40,0x3d,0x3a,
	0x38,0x35,0x33,0x30,0x2e,0x2b,0x29,0x27,
	0x25,0x22,0x20,0x1e,0x1c,0x1a,0x18,0x16,
	0x15,0x13,0x11,0x10,0x0e,0x0d,0x0b,0x0a,
	0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,
	0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,
	0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
	0x0a,0x0b,0x0d,0x0e,0x10,0x11,0x13,0x15,
	0x16,0x18,0x1a,0x1c,0x1e,0x20,0x22,0x25,
	0x27,0x29,0x2b,0x2e,0x30,0x33,0x35,0x38,
	0x3a,0x3d,0x40,0x43,0x45,0x48,0x4c,0x4e,
	0x51,0x55,0x57,0x5a,0x5d,0x60,0x63,0x66,
	0x69,0x6c,0x6f,0x72,0x76,0x79,0x7c,0x80

};
/**iclude the host usb lib**/ //modify by leiyq 20120318
#include <usbh_core.h>
extern struct rt_device uart2_device;
extern CLOCK curTime;
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST             USB_Host;
extern StructPara Parameter;
 USBH_Class_cb_TypeDef USBH_MSC_cb;
 USBH_Usr_cb_TypeDef   USR_cb;
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[ 512 ];
//static rt_uint8_t usb_stack[ 512 ];//modify by leiyq 20130215
static struct rt_thread led_thread;
//static struct rt_thread usb_thread;//modify by leiyq 20130215
u8 testwritebuff[10];
u8 testreadbuff[10];
unsigned long  jonhbak;
const unsigned short abji[]=
				{
						0x0000,0x0000,0x1000,0x1000,
						0x11F0,0x11F0,0x1110,0x1110,
						0x7D10,0x7D10,0x1110,0x1110,
						0x1110,0x1110,0x3910,0x3910,
						0x3510,0x3510,0x5110,0x5110,
						0x1110,0x1110,0x1110,0x1110,
						0x1214,0x1214,0x1214,0x1214,
						0x140C,0x140C,0x0000,0x0000
				};
static void led_thread_entry(void* parameter)
{
    unsigned int i;
    unsigned long jonh;
    unsigned char num=0;
    while (1)
    {
        rt_hw_led_on(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */
       // testreadbuff=I2C_Master_BufferWrite(I2C1,3,0,1,&testwritebuff);

#if 0
        SPI_FLASH_BulkErase(SPI1);
        for (jonh = 0;jonh<0x100000;jonh=jonh+512)
        {
			for (i = 0;i<512;i++ )
			{
				if (i< 256)
				{
					testwritebuff[i] = i;
				}
				else
				{
					testwritebuff[i] = i-256;
				}
			}
			SPI_FLASH_BufferWrite(SPI1,&testwritebuff,jonh,512);
	        SPI_FLASH_BufferRead(SPI1,&testreadbuff,jonh,512);
	        if (strcmp(testwritebuff,testreadbuff)== 0)
	        {
	               	rt_hw_led_off(0);
	               	jonhbak = jonh;

	        }
	        else
	        	break;
        }
        rt_hw_led_on(0);
        jonhbak = jonh;


        SPI_FLASH_Sector4kErase(SPI2,0);
        for(i = 0;i<10;i++)
        {
        	testwritebuff[i] = i+1;
        }
        SPI_FLASH_BufferWrite(SPI2,testwritebuff,0,10);
        SPI_FLASH_BufferRead(SPI2,testreadbuff,0,10);

        Parameter.PulseCoff = 300;
#endif
#if 0
        while(1)
      //  DisplayNormalUI();
        {
            lcd_delay(5);
            DAC_SetChannel1Data(DAC_Align_8b_R,sinx[num]);

           // DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);
            num++;

        }
#endif
        KepPressHandler();
        IckaHandler();
        rs232_handle_application(&uart2_device);

    }

}
/*usb_thread_entry */ //add by leiyq 20120516
static void usb_thread_entry(void* parameter)
{
/* init the usbhost mode *///first
	USBH_Init( &USB_OTG_Core,USB_OTG_FS_CORE_ID,
			&USB_Host,&USBH_MSC_cb,&USR_cb);
	//GPIO_ResetBits(GPIOC,USB_PWR_ON);


	while(1)
	{
		/* dectect the usb plugin *///second
		USBH_Process( &USB_OTG_Core, &USB_Host);
		rt_thread_delay(1);

	}


}
void rt_init_thread_entry(void* parameter)
{
   // rt_hw_buzz_on();
    rt_thread_delay(10);
    rt_hw_buzz_off();
/* Filesystem Initialization */
#ifdef RT_USING_DFS
	{
		/* init the device filesystem */
		dfs_init();

#ifdef RT_USING_DFS_ELMFAT
		/* init the elm chan FatFs filesystam*/
		elm_init();

		/* mount sd card fat partition 1 as root directory */
		if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
		{
			rt_kprintf("\nFile System initialized!\n");
		}
		else
			rt_kprintf("\nFile System initialzation failed!\n");
#endif
	}
#endif
		rt_hw_led_init();
		Time3_enalble();
		rt_hw_tim3_init();
		GPIO_SetBits(GPIOE,GPIO_Pin_2);
		InitialValue();
		InitializeTable();
		DisplayNormalUI();

	while(1) { rt_thread_delay(1000);};
}

int rt_application_init()
{
	rt_thread_t init_thread;
	rt_thread_t usb_thread;

	rt_err_t result;

    /* init led thread */
	result = rt_thread_init(&led_thread,
		"led",
		led_thread_entry, RT_NULL,
		(rt_uint8_t*)&led_stack[0], sizeof(led_stack), 20, 5);
	if (result == RT_EOK)
	{
        rt_thread_startup(&led_thread);
	}

	usb_thread = rt_thread_create("usb",
			usb_thread_entry, RT_NULL,
								2048, 10, 15);

	if (usb_thread != RT_NULL)
		rt_thread_startup(usb_thread);

#if (RT_THREAD_PRIORITY_MAX == 32)
	init_thread = rt_thread_create("init",
								rt_init_thread_entry, RT_NULL,
								2048, 8, 20);
#else
	init_thread = rt_thread_create("init",
								rt_init_thread_entry, RT_NULL,
								2048, 80, 20);
#endif

	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	return 0;
}

/*@}*/
