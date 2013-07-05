
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include<stm32f10x_tim.h>
#include<application.h>
#include<menu.h>
#include<RS232.h>


extern CLOCK curTime;
extern LCDTCB lcd_tcb;
extern SizeData location;
unsigned char CurStatus;
#define SpeedSpace 4

extern StructPara Parameter;
extern CMD_VER Verificationstatus;
unsigned long CurPulse = 0;
///////////////////////////////////gps data and speed data////////////
unsigned long CurPN = 0;
unsigned long LastPN20ms = 0;
unsigned long LastPN1s = 0;
unsigned long LastPN1min = 0;
unsigned short CurSpeed = 0;	//当前速度（0.2秒平均速度）
unsigned long Curspeed1s = 0;//1s平均速度
unsigned long Curspeed1min = 0;//1分钟平均速度
unsigned char radionum = 0;

int DeltaSpeed = 0;
unsigned char Time6sCnt;
unsigned long Distance = 0;


////////////////ic卡data///////////////////////
///////////////////////////////////////////////
unsigned char inserticflag;
unsigned short insertcount; //防止误动作

////////////////////////////////key data////////
//////////////////////////////////////////////
Timeflag timeflag;
TimeCnt  timecnt;
typedef enum KEY_STATUS
{
	KEY_NONE,
	MENU_KEY,
	SELE_UP_KEY,
	SELE_DOWN_KEY,
	ENTER_KEY,
	PRIN_KEY,

}Key_status;
Key_status keyval = KEY_NONE,keyvalbak = KEY_NONE;
unsigned short keyvalcount;
void Getthepluse()
{
	CurPN++;
	Parameter.PulseNumber++;

}
//*----------------------------------------------------------------------------
//* Function Name       : GetStatus
//* Object              : ��ȡ״̬
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      : ״̬
//*----------------------------------------------------------------------------
void GetStatus()
{
	//PA0:点火信号；
	CurStatus |= (7<<GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_12));//制动
	CurStatus |= (6<<GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8));//turn left
	CurStatus |= (5<<GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_10));//turn right
	CurStatus |= (4<<GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_11));//high beam
	CurStatus |= (3<<(~GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_11)));//low beam
	CurStatus |= (2<<GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9));//door
	CurStatus |= (1<<GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13));//鸣笛
	CurStatus |= (0<<GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0));//点火
}


//*----------------------------------------------------------------------------
//* Function Name       : ComputeSpeed
//* Object              : �����ٶ�ֵ
//* Input Parameters    : ������
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      : �������ٶ�ֵ��
//*----------------------------------------------------------------------------
unsigned long ComputeSpeed(unsigned long pulse)
{
	unsigned long p,t,temp_pulse;
	int i;
	unsigned long spe;
	unsigned long T;
	unsigned char PP = 8;
	T = 7200000 / Parameter.PulseCoff;
	T = (T * 10)/ PP;

	
	spe = T*pulse/(2000);

	if((spe % 10) >= 5)
		spe = spe/10 + 1;
	else
		spe = spe/10;
		
	if(spe>255)
		spe = 255;
	return spe;

}
//#if OpenDoorDeal
//*----------------------------------------------------------------------------
//* Function Name       : GetSpeed
//* Object              : ��ȡ�ٶȺͿ�����״̬
//* Input Parameters    : times��ʾ�ڼ��βɼ�
//* Output Parameters   : �ɼ��Ƿ�ɹ�
//*0.2s执行一次
//*----------------------------------------------------------------------------
void GetSpeedandTime(void)
{
 	unsigned long pulse,timer;
	unsigned long temp;
	unsigned long p,t;
	if(timeflag.Time200msflag == 1)
	{
		if(LastPN20ms == 0)
			pulse = CurPN;
		else
			pulse = CurPN-LastPN20ms;
		timeflag.Time200msflag = 0;
		LastPN20ms = CurPN;
		CurSpeed = ComputeSpeed(pulse);
	}
	if(timeflag.Time1sflag == 1)
	{
		GetCurrentDateTime(&curTime);
		if(( lcd_tcb.ListNb == 0 ) && (lcd_tcb.mode == 0))
		{
			DisplayNormalUI();
		}
		if(LastPN1s == 0)
			pulse = CurPN;
		else
			pulse = CurPN-LastPN1s;
		LastPN1s = CurPN;
		timeflag.Time1sflag = 0;
		Curspeed1s = ComputeSpeed(pulse)/5;
	}
	if(timeflag.Time1minflag == 1)
	{
		if(LastPN1min == 0)
			pulse = CurPN;
		else
			pulse = CurPN-LastPN1min;
		LastPN1min = CurPN;
		timeflag.Time1minflag = 0;
		Curspeed1min = ComputeSpeed(pulse)/300;

	}


	//////////////////////////////////////////////////////////////////////
}
//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////////////////////////key handler////////////////
/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void Time3_irg_handler()
{
	extern void keyscanhandler(void);
	  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	  {
	      TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	      timecnt.Time1msCnt++;
	      if((timecnt.Time1msCnt )%10 ==0)//10ms扫描一次按键
	      {
	    	  keyscanhandler();
	      }
	      if( timecnt.Time1msCnt >199)
	      {
	    	  timecnt.Time1msCnt = 0;
	    	  timeflag.Time200msflag = 1;

	    	  timecnt.Time200msCnt++;
	    	  if(timecnt.Time200msCnt > 4)
	    	  {
	    		  timeflag.Time1sflag = 1;
	    		  timeflag.Ver1sflag = 1;
	    		  timecnt.Time1sCnt = 0;
	    		  timecnt.Time1sCnt ++;
	    		  if(Time6sCnt)
	    		  {
	    			  Time6sCnt--;
	    			  if((Time6sCnt == 0)&&(Verificationstatus == 1))
	    			  {
	    				  Verificationstatus = 0;
	    			  }

	    		  }
	    		  if(timecnt.Time1sCnt >59)
	    		  {
	    			  timeflag.Time1minflag = 1;
	    			  timecnt.Time1sCnt = 0;
	    		  }
	    	  }
	      }

	  }
}
//////////////////////////////////////////////
//////////////////////////////////////////////
///GPS data////////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////
unsigned char ASCII2char(unsigned char number)
{
	if((number <58)&&(number>47))
	{
		number = number-48;
	}
	return number;
}
unsigned long CountTime(CLOCK ctime)
{
	unsigned long timesec;
	timesec = ctime.hour*3600+ctime.minute*60+ctime.second;
	return timesec;
}

void GetTheGPSTime(unsigned char *tbuf)
{
	CLOCK GPStime;
	unsigned long nwtime,gtime;
	unsigned char i,dtime;
	unsigned char *pbuf;
	unsigned char *buf;
	pbuf = &GPStime.day;
	buf = &curTime.day;
	for(i= 0;i<3;i++)
	{
		*(pbuf+i+1) = 10*ASCII2char(*tbuf)+ ASCII2char(*tbuf);
	}
	nwtime =CountTime(curTime);
	gtime = CountTime(GPStime);
	if(gtime>nwtime)
	{
		dtime = gtime-nwtime;
	}
	else
	{
		dtime = nwtime-gtime;
	}
	if((dtime >10) &&(dtime<200))
	{
		for(i= 0;i<3;i++)
		{
			*(buf+i-1) = (*(pbuf+i-1));
		}
		//set the time
		SetCurrentDateTime(curTime);
	}

}
void GetGPSLocation1(unsigned char *tbuf)
{
	unsigned char i,j= 1;
	unsigned long *pbuf = (unsigned long *)&location.longtitude;
	*pbuf = 0;
	for(i= 0;i<9;i++)
	{
		if(i!=5)
		{
			*pbuf =ASCII2char(*tbuf)+j*(*pbuf);
			j= j*10;
		}
	}

}
void GetGPSLocation2(unsigned char *tbuf)
{
	unsigned char i,j= 1;
	unsigned long *pbuf = (unsigned long *)&location.latitude;
	*pbuf = 0;
	for(i= 0;i<9;i++)
	{
		if(i!=5)
		{
			*pbuf =ASCII2char(*tbuf)+j*(*pbuf);
			j= j*10;
		}
	}

}
void GetGPSLocation3(unsigned char *tbuf)
{
	unsigned char i,j= 1;
	unsigned short *pbuf = (unsigned short *)&location.latitude;
	*pbuf = 0;
	for(i= 0;i<6;i++)
	{
		if(i!=5)
		{
			*pbuf =ASCII2char(*tbuf)+j*(*pbuf);
			j= j*10;
		}
	}

}
unsigned char GettheSinaldata(unsigned char *tbuf)
{
		unsigned char tempdata;
		tempdata = 10*ASCII2char(tbuf[0])+ASCII2char(tbuf[1]);
		return tempdata;

}
//////////////////////////////////////////////////////
////////////////////////key press handle//////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
void KepPressHandler()
{
	switch(keyval)
	{
	case MENU_KEY:
		MenutKeyHandler();
		keyval = KEY_NONE;
		break;
	case SELE_UP_KEY:
		SelectKeyHandler(1);
		keyval = KEY_NONE;
		break;
	case SELE_DOWN_KEY:
		SelectKeyHandler(0);
		keyval = KEY_NONE;
		break;
	case ENTER_KEY:
		OKKeyHandler();
		keyval = KEY_NONE;
		break;
	case PRIN_KEY:
		if((lcd_tcb.ListNb == 0 )&&(lcd_tcb.mode ==0))
		{
			PrintAllData();
			printer();
			lcd_clear(lineall);
			DisplayNormalUI();
			keyval = KEY_NONE;
		}
	    break;
	default:
		break;
	}

}
void keyscanhandler(void)
{
	unsigned char tempkey = KEY_NONE;
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14) == 0)
	{
		tempkey = MENU_KEY;
	}
	else if(GPIO_ReadInputDataBit( GPIOD, GPIO_Pin_15) == 0 )
	{
		tempkey = SELE_UP_KEY;
	}
	else if(GPIO_ReadInputDataBit( GPIOC, GPIO_Pin_6) == 0 )
	{
		tempkey = SELE_DOWN_KEY;
	}
	else if(GPIO_ReadInputDataBit( GPIOC, GPIO_Pin_7) == 0 )
	{
		tempkey = ENTER_KEY;
	}
	if(tempkey)
	{
		if(tempkey == keyvalbak)
		{

			if(keyvalcount !=0xffff)
				keyvalcount++;
			if((keyvalcount ==300)&& (keyvalbak == ENTER_KEY))
			{
				keyval = PRIN_KEY;
			}
		}
		else
		{
			keyvalcount = 1;
			keyvalbak = tempkey;
		}
	}
	else
	{
		if(keyvalcount >15)
		{
			if((keyvalcount >300)&& (keyvalbak == ENTER_KEY))
			{
				if(keyval == KEY_NONE)
				{
					keyval = KEY_NONE;
				}
			}
			else
			{
				keyval = keyvalbak;
			}
			keyvalcount =0;
		}
		keyvalcount = 0;
		keyvalbak = 0;
	}

}
//////////////////////////////////////////////////////
///////////////////////////////////////////////////////
/////////IC 卡数据处理//////////////////////////////////
void GetTheDriverNumber()
{

	I2C_Master_BufferRead(I2C1,
								  0xa0,
	                              32,
	                              18,
	                              Parameter.DriverLisenseCode);
}
void IckaHandler()
{
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_4) == 1)
	{
		insertcount++;
		if(insertcount == 1000)
		{
			insertcount = 0;
			inserticflag = 1;
			GetTheDriverNumber();
		}
	}
	else
	{
		insertcount = 0;
		inserticflag = 0;

	}

}



 
