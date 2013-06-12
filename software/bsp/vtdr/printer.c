
//*----------------------------------------------------------------------------
//*      LCD�˵��л�
//*----------------------------------------------------------------------------
//* File Name           : printer.c
//* Object              : ʵ�ִ�ӡ����
//*
//* 1.0 10/04/03 myw : Creation
//*----------------------------------------------------------------------------
#include "printer.h"
#include "application.h"
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#define PRN_CMDE_CTR (GPIOE)
#define PRN_EN    GPIO_Pin_1
#define PRN_PWR   GPIO_Pin_7
#define PRN_STB1  GPIO_Pin_3
#define PRN_STB2  GPIO_Pin_4
#define PRN_PAPER GPIO_Pin_5

#define PRN_CMDB_CTR (GPIOB)
#define PRN_MCSTOP    GPIO_Pin_0
#define PRN_MC1       GPIO_Pin_8
#define PRN_MC2       GPIO_Pin_9
#define PRN_LAT       GPIO_Pin_1
#define PRN_MOSI      GPIO_Pin_15
#define PRN_SCK       GPIO_Pin_13
#define PRN_MISO      GPIO_Pin_14
#define PRN_CMDC_CRT (GPIOC)
#define PRN_THER      GPIO_PIN_0


extern CLOCK curTime;
extern PartitionTable pTable;
extern StructPara Parameter;
extern unsigned char LargeDataBuffer[];
extern u_short DriveMinuteLimit;
extern u_short RestMinuteLimit;

//const unsigned short     line1[]={0xb5b3,0xc6c5,0xc5ba,0xebc2,0xbaa3};//���ƺ��룺
//const unsigned short     line2[]={0xb5b3,0xc6c5,0xd6b7,0xe0c0,0xbaa3};//���Ʒ��ࣺ
//const unsigned short	 line3[]={0xddbc,0xbbca,0xb1d4,0xfab4,0xebc2,0xbaa3};//��ʻԱ���룺
//const unsigned short	 line4[]={0xddbc,0xbbca,0xa4d6,0xc5ba,0xebc2,0xbaa3};//��ʻ֤����:
//const unsigned short	 line5[]={0xf2b4,0xa1d3,0xb1ca,0xe4bc,0xbaa3};//��ӡʱ��:
//const unsigned short	 line60[]={0xa3cd,0xb5b3,0xb0c7,0x3531,0xd6b7,0xbdc6,0xf9be,0xd9cb,0xc8b6,0xafa3,0xd6b7,0xbaa3};//ͣ��ǰ15��ƽ���ٶ�/�֣�
//const unsigned short	 line8[]={0xa3c6,0xcdc0,0xddbc,0xbbca,0xc7bc,0xbcc2,0xbaa3};//ƣ�ͼ�ʻ��¼��
//const unsigned short	 line8k[]={0xaabf,0xbcca,0xb1ca,0xe4bc,0xbaa3};//��ʼʱ�䣺
//const unsigned short	 line8j[]={0xe1bd,0xf8ca,0xb1ca,0xe4bc,0xbaa3};//����ʱ�䣺
//const unsigned short     unknown[]={0xb4ce,0xaad6};//δ֪

//*----------------------------------------------------------------------------
//* Function Name       : TimeToASCII()  //myw
//* Object              : ��ǰʱ��ת����ASCII
//* Input Parameters    : curTime only to print time;
//* Output Parameters   : <ASCII[]>��ŵ�ǰʱ��ת����ASCII
//*���õ�ȫ�ֱ���       : curTime
//*�޸ĵ�ȫ�ֱ���       : None
//*----------------------------------------------------------------------------
void TimeToASCII(unsigned char *ASCII)
{

	ASCII[0]=0x32;
	ASCII[1]=0x30;//20
	
	ASCII[2]=((curTime.year&0xf0)>>4)+0x30;//shiwei
	ASCII[3]=(curTime.year&0x0f)+0x30;//gewei
	ASCII[4]=0x2f;// /

	ASCII[5]=((curTime.month&0xf0)>>4)+0x30;//shiwei
	ASCII[6]=(curTime.month&0x0f)+0x30;//gewei
	ASCII[7]=0x2f;// /

	ASCII[8]=((curTime.day&0xf0)>>4)+0x30;//shiwei
	ASCII[9]=(curTime.day&0x0f)+0x30;//gewei
	ASCII[10]=0x20;// /
	
	ASCII[11]=((curTime.hour&0xf0)>>4)+0x30;//shiwei
	ASCII[12]=(curTime.hour&0x0f)+0x30;//gewei
	ASCII[13]=0x3a;//:
	
	ASCII[14]=((curTime.minute&0xf0)>>4)+0x30;//shiwei
	ASCII[15]=(curTime.minute&0x0f)+0x30;//gewei
	ASCII[16]=0x3a;//:
	
	ASCII[17]=((curTime.second&0xf0)>>4)+0x30;//shiwei
	ASCII[18]=(curTime.second&0x0f)+0x30;//gewei

}
//*----------------------------------------------------------------------------
//* Function Name       : TiredToASCII()  //myw
//* Object              :buf[]���ת����ASCII
//* Input Parameters    : ƣ��ʱ�� only to print time;
//* Output Parameters   : <ASCII[]>���buf[]ת����ASCII
//*���õ�ȫ�ֱ���       : none
//*�޸ĵ�ȫ�ֱ���       : None
//*----------------------------------------------------------------------------
void TiredToASCII(unsigned char *ASCII,unsigned char *buf)
{
	ASCII[0]=0x32;
	ASCII[1]=0x30;//20
	//  / ��
	ASCII[2]=((buf[0]&0xf0)>>4)+0x30;//shiwei
	ASCII[3]=(buf[0]&0x0f)+0x30;//gewei
	ASCII[4]=0x2f;
//  /��	
	ASCII[5]=((buf[1]&0xf0)>>4)+0x30;//shiwei
	ASCII[6]=(buf[1]&0x0f)+0x30;//gewei
	ASCII[7]=0x2f;
//��	
	ASCII[8]=((buf[2]&0xf0)>>4)+0x30;//shiwei
	ASCII[9]=(buf[2]&0x0f)+0x30;//gewei
	ASCII[10]=0x1b;  //��2��
	ASCII[11]=0x66;//shiwei
	ASCII[12]=0x0;
	ASCII[13]=0x2;//gewei
//  :	
	ASCII[14]=((buf[3]&0xf0)>>4)+0x30;//shiwei
	ASCII[15]=(buf[3]&0x0f)+0x30;//gewei
	ASCII[16]=0x3a;
	
	ASCII[17]=((buf[4]&0xf0)>>4)+0x30;//shiwei
	ASCII[18]=(buf[4]&0x0f)+0x30;//gewei
}
//*----------------------------------------------------------------------------
//* Function Name       : IntToASCII() //myw
//* Object              :��ʻԱ����ת����ASCII
//* Input Parameters    : int DriverCode
//* Output Parameters   : <buf[]>��ż�ʻԱ����ת����ASCII���������鳤��
//*���õ�ȫ�ֱ���       : none
//*�޸ĵ�ȫ�ֱ���       : None
//*----------------------------------------------------------------------------
unsigned char IntToASCII(unsigned char *buf,u_int drivercode)
{
	//unsigned char gew;unsigned char shiw;unsigned char baiw;
	// u_int 16--10����,
	unsigned int y;
	y=drivercode;
	unsigned int x=y/10;
	unsigned char i=0;
	do{
	    buf[i] =(y-x*10)+0x30;
		y=y/10;
		x=y/10;
		i=i+1;
	}while(y!=0);
     return i;
}
//*----------------------------------------------------------------------------
//* Function Name       : CharToASCII() //myw
//* Object              :һ���ֽڵ����ת���ɹ�꺺����
//* Input Parameters    : charһ���ֽڵ����
//* Output Parameters   :< ASCII[]>���һ���ֽڵ����ת���ɹ�꺺���룬ͬʱ���ظ����鳤��
//*���õ�ȫ�ֱ���       : none
//*�޸ĵ�ȫ�ֱ���       : None
//*----------------------------------------------------------------------------
unsigned char CharToASCII(unsigned char Char,unsigned char *ASCII)
{
	
	//unsigned char gew;unsigned char shiw;unsigned char baiw;
	// unsigned char16--10����,
	unsigned char geshu;
	if((Char/100)==0)
	{
	   if((Char/10)==0)
	   {
	      ASCII[0]=Char%10+0x30;//gew
	      geshu=1;
	   }
	   else
	   {
			 ASCII[0]=Char/10+0x30;//shiw
			 ASCII[1]=Char%10+0x30;//gew
			 geshu=2;
	    }
	}
	else
	{    	 
	 ASCII[0]=Char/100+0x30;//baiw
	 ASCII[1]=(Char/10)%10+0x30;//shiw
	 ASCII[2]=Char%10+0x30;//gew
	 geshu=3;
	 
	 } 
     return geshu;
}

//*----------------------------------------------------------------------------
//* Function Name       : PrintReady
//* Object              : �Ƿ���Դ�ӡ
//* Input Parameters    : none
//* Output Parameters   : none
//*----------------------------------------------------------------------------
unsigned char PrintReady()
{ 
	return 1;
}
unsigned char printOneOTDRRecord(unsigned char number,OTDR *record)
{
	return 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : printline8() //myw
//* Object              :��ӡƣ�ͼ�ʻ��¼
//* Input Parameters    : none
//* Output Parameters   : �ɹ�����1��ʧ�ܷ���0
//*���õ�ȫ�ֱ���       : none
//*�޸ĵ�ȫ�ֱ���       : None
//*----------------------------------------------------------------------------
unsigned char printline8()
{
return 1;
}


//*----------------------------------------------------------------------------
//* Function Name       : Printer
//* Object              : ��ӡ�������
//* Input Parameters    : none
//* Output Parameters   : none
//*----------------------------------------------------------------------------
unsigned char Printer()
{
	return 1;
}

void rt_hw_printer_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin   = PRN_PWR|PRN_EN|PRN_STB1|PRN_STB2;
	//PRN_PWR，PRN_EN，SRTOB12。
	GPIO_Init(PRN_CMDE_CTR, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = PRN_MCSTOP |PRN_LAT;
	GPIO_Init(PRN_CMDB_CTR, &GPIO_InitStructure);

	GPIO_Init(GPIOE, &GPIO_InitStructure);

	printer_pwr_ctrl(1);

}

void printer_pwr_ctrl(unsigned char ctldr)
{
	if(ctldr)
	{
		GPIO_SetBits(PRN_CMDE_CTR,PRN_EN);
		lcd_delay(5);
		GPIO_SetBits(PRN_CMDE_CTR,PRN_PWR);
	}
	else
	{
		GPIO_ResetBits(PRN_CMDE_CTR,PRN_EN);
		lcd_delay(5);
		GPIO_ResetBits(PRN_CMDE_CTR,PRN_PWR);
	}
}
