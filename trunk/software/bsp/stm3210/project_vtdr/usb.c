/*
 * usb.c
 *
 *  Created on: Mar 4, 2012
 *      Author: mxx
 */
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "usb.h"

#define USB_PWR_ON			(GPIO_Pin_0)
#define USB_OC				(GPIO_Pin_1)
#define USB_VBUS			(GPIO_Pin_9)
#define USB_DM				(GPIO_Pin_11)
#define USB_DP				(GPIO_Pin_12)
#define USB_ID				(GPIO_Pin_10)

/**
  * @brief  USB_OTG_BSP_Init
  *         Initilizes BSP configurations
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
	  GPIO_InitTypeDef GPIO_InitStructure;

	  RCC_AHB1PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);

	  /* Configure SOF VBUS ID DM DP Pins */
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8  |
	    GPIO_Pin_9  |
	      GPIO_Pin_11 |
	        GPIO_Pin_12;

	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

//	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_OTG1_FS) ;
//	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_OTG1_FS) ;
//	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ;
//	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;

	  /* this for ID line debug */


//	  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
//	  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
//	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//	  GPIO_Init(GPIOA, &GPIO_InitStructure);
//	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_OTG1_FS) ;
//
//	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//	  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ;
//
//
//	  /* Intialize Timer for delay function */
//	  USB_OTG_BSP_TimeInit();
}
/**
  * @brief  USB_OTG_BSP_EnableInterrupt
  *         Enabele USB Global interrupt
  * @param  None
  * @retval None
  */
//void USB_OTG_BSP_EnableInterrupt(void)
//{
//
//}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  speed : Full, Low
  * @param  state : VBUS states
  * @retval None
  */

//void USB_OTG_BSP_DriveVBUS(uint32_t speed, uint8_t state)
//{
//
//}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  Speed : Full, Low
  * @retval None
  */

//void  USB_OTG_BSP_ConfigVBUS(uint32_t speed)
//{
//
//}

/**
  * @brief  USB_OTG_BSP_TimeInit
  *         Initialises delay unit Systick timer /Timer2
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_TimeInit ( void )
{

}

/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay (const uint32_t usec)
{

  uint32_t count = 0;
  const uint32_t utime = (120 * usec / 7);
  do
  {
    if ( ++count > utime )
    {
      return ;
    }
  }
  while (1);

}


/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
//void USB_OTG_BSP_mDelay (const uint32_t msec)
//{
//
//    USB_OTG_BSP_uDelay(msec * 1000);
//
//}


/**
  * @brief  USB_OTG_BSP_TimerIRQ
  *         Time base IRQ
  * @param  None
  * @retval None
  */

//void USB_OTG_BSP_TimerIRQ (void)
//{
//
//}


void rt_hw_usb_init()
{
    //RCC_OTGFSCLKConfig();
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

    GPIO_SetBits(GPIOB,USB_PWR_ON);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Pin   = USB_PWR_ON ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin   = USB_OC ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   =   USB_ID | USB_DM |USB_DP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = USB_VBUS;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //GPIO_ResetBits(GPIOB,USB_PWR_ON);
}


