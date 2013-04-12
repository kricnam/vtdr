#include "includes.h"
#include    "parts/r40807/lib_r40807.h"
#include    "parts/r40807/reg_r40807.h"
#include    "ibb3.h"

#define SpeedSpace 4

extern u_char USB_Communicate_Flag;
extern PartitionTable pTable;
extern u_char InRecordCycle;		//是否在记录数据过程中

UsartDesc *RS2320;
u_int CurPulse = 0;

u_int CurPN = 0;						//本次0.2脉冲数
u_int LastPN[SpeedSpace] = {0,0,0,0};   //上次0.2秒脉冲数
u_int CurTN = 0;						//本次0.1ms的个数
//u_int LastTN[SpeedSpace] = {0,0,0,0};   //上次0.1ms的个数
u_int CurSpeed = 0;						//当前速度值（1秒平均）
u_int LastSPE = 0;      				//0.2秒瞬时速度值
u_int AverageV = 0;						//显示用速度值
u_int PulseNB_In1Sec = 0;
u_int TimerNB_In1Sec = 0;

u_int IntervalTN = 0;

int DeltaSpeed = 0;		//本次速度与前一次的差值 = V1 - V0
u_int PulseTotalNumber = 0;		//本次行驶总脉冲数
u_int Distance = 0;         	//本次行驶里程

u_int CurEngine =0;     		//当前发动机转速值
u_int RPM_PN=0;					//发动机转速脉冲个数
u_int LastRPM[SpeedSpace] = {0,0,0,0};

u_short STATUS = 0;				//16种状态
u_short STATUS0 = 0;        	//前一组状态
u_char CurStatus = 0;       	//记录的8种状态
u_char LastStatus = 0;

u_char LastDoorStatus =0;
extern u_char CurDoorStatus;
u_char OpenFlag1=0;
u_char OpenFlag2=0;

u_char PowerOn = 0;
u_char LastPowerOn = 0;
u_char PowerOnT = 255;
u_char PowerOffT = 0;
u_char buff[32][4];
#if RPM_EN
//*----------------------------------------------------------------------------
//* Function Name       : ComputeRPM
//* Object              : 发动机转速信号的计算
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void ComputeRPM()
{
	u_int engine;
	engine = RPM_PN;
	RPM_PN = 0;
	CurEngine = engine;
	int i;
	for(i=0;i<SpeedSpace;i++)
	{
		engine += LastRPM[i];
		LastRPM[i]=LastRPM[i+1];
	}
	LastRPM[SpeedSpace-1] = CurEngine;
	CurEngine = engine*60;
}
#endif

#if OpenDoorDeal
//*----------------------------------------------------------------------------
//* Function Name       : DoubleLineLevel
//* Object              : 开关门状态为双线电平
//* Input Parameters    : DoorNB，门号
//* Output Parameters   : 本次开关门状态
//*                     : 0x80表示此时开关门状态为高
//*                     : 0x00表示此时开关门状态为低,或者开关门未接或未检测到开关门
//* 引用的全局变量      : CurDoorStatus
//* 修改的全局变量      : 
//*----------------------------------------------------------------------------
u_char DoubleLineLevel(u_char DoorNB)
{
	u_char doorstatus=0;
	u_char door1Ostatus=0;
	u_char door1Cstatus=0;
	u_char door2Ostatus=0;
	u_char door2Cstatus=0;
	//取出开关门的状态极性
	u_char pol=(PARAMETER_BASE->status_polarity)>>15;
	
	//Door1
	if(DoorNB==1)
	{
		//状态极性为负
		if(pol)
		{
			door1Ostatus = ((~(CurDoorStatus>>3))<<7)&0x80;
			if(door1Ostatus)
				OpenFlag1=1;
			door1Cstatus = ((~(CurDoorStatus>>2))<<7)&0x80;
			if(door1Cstatus)
				OpenFlag1=0;
		}
		//状态极性为正
		else
		{
			door1Ostatus = (CurDoorStatus<<4)&0x80;
			if(door1Ostatus)
				OpenFlag1=1;
			door1Cstatus = (CurDoorStatus<<5)&0x80;
			if(door1Cstatus)
				OpenFlag1=0;		
		}
		//2003.10.23 myw
		doorstatus = (OpenFlag1<<7);
	}
	//Door2
	if(DoorNB==2)
	{
		//状态极性为负
		if(pol)
		{
			door1Ostatus = ((~(CurDoorStatus>>1))<<7)&0x80;
			if(door1Ostatus)
				OpenFlag2=1;
			door1Cstatus = ((~(CurDoorStatus>>0))<<7)&0x80;
			if(door1Cstatus)
				OpenFlag2=0;
		}
		//状态极性为正
		else
		{
			door1Ostatus = (CurDoorStatus<<6)&0x80;
			if(door1Ostatus)
				OpenFlag2=1;
			door1Cstatus = (CurDoorStatus<<7)&0x80;
			if(door1Cstatus)
				OpenFlag2=0;		
		}
		//2003.10.23 myw
		doorstatus =(OpenFlag2<<7);
	}
	//doorstatus = (OpenFlag1<<7)|(OpenFlag2<<7);//2003.10.23  myw
	return(doorstatus);	
}

//*----------------------------------------------------------------------------
//* Function Name       : SingleLineLevelOpen
//* Object              : 开关门状态为开门线电平
//* Input Parameters    : DoorNB，门号
//* Output Parameters   : 本次开关门状态
//*                     : 0x80表示此时开关门状态为高
//*                     : 0x00表示此时开关门状态为低,或者开关门未接或未检测到开关门
//* 引用的全局变量      : CurDoorStatus
//* 修改的全局变量      : 
//*----------------------------------------------------------------------------
u_char SingleLineLevelOpen(u_char DoorNB)
{
	u_char doorstatus=0;
	u_char door1status=0;
	u_char door2status=0;
	//取出开关门的状态极性
	u_char pol=(PARAMETER_BASE->status_polarity)>>15;
	
	//Door1
	if(DoorNB==1)
	{
		//状态极性为负
		if(pol)
			door1status = ((~(CurDoorStatus>>3))<<7)&0x80;
		//状态极性为正
		else
			door1status = (CurDoorStatus<<4)&0x80;
		
		//2003.10.23  myw
		doorstatus = door1status;
	}
	//Door2
	if(DoorNB==2)
	{
		//状态极性为负
		if(pol)
			door2status = ((~(CurDoorStatus>>1))<<7)&0x80;
		//状态极性为正
		else
			door2status = (CurDoorStatus<<6)&0x80;
			
		//2003.10.23  myw
		doorstatus = door2status;
	}
	//doorstatus = door1status|door2status;
	return(doorstatus);	
}

//*----------------------------------------------------------------------------
//* Function Name       : SingleLineLevelClose
//* Object              : 开关门状态为开门线电平
//* Input Parameters    : DoorNB，门号
//* Output Parameters   : 本次开关门状态
//*                     : 0x80表示此时开关门状态为高
//*                     : 0x00表示此时开关门状态为低,或者开关门未接或未检测到开关门
//* 引用的全局变量      : CurDoorStatus
//* 修改的全局变量      : 
//*----------------------------------------------------------------------------
u_char SingleLineLevelClose(u_char DoorNB)
{
	u_char doorstatus=0;
	u_char door1status=0;
	u_char door2status=0;
	//取出开关门的状态极性
	u_char pol=(PARAMETER_BASE->status_polarity)>>15;
	
	//Door1
	if(DoorNB==1)
	{
		//状态极性为负
		if(pol)
			door1status = ((~(CurDoorStatus>>2))<<7)&0x80;
		//状态极性为正
		else
			door1status = (CurDoorStatus<<5)&0x80;
		//2003.10.23 myw
		doorstatus = door1status;
	}
	//Door2
	if(DoorNB==2)
	{
		//状态极性为负
		if(pol)
			door2status = ((~(CurDoorStatus>>0))<<7)&0x80;
		//状态极性为正
		else
			door2status = (CurDoorStatus<<7)&0x80;
		//2003.10.23 myw
		doorstatus = door2status;
	}
	//doorstatus = door1status|door2status;
	return(doorstatus);	
}

//*----------------------------------------------------------------------------
//* Function Name       : DoubleLinePulse
//* Object              : 开关门状态为双线脉冲
//* Input Parameters    : DoorNB，门号
//* Output Parameters   : 本次开关门状态
//*                     : 0x80表示此时开关门状态为高
//*                     : 0x00表示此时开关门状态为低,或者开关门未接或未检测到开关门
//* 引用的全局变量      : CurDoorStatus
//* 修改的全局变量      : 
//*----------------------------------------------------------------------------
u_char DoubleLinePulse(u_char DoorNB)
{
	u_char doorstatus=0;
	u_char door1Ostatus=0;
	u_char door1Cstatus=0;
	u_char door2Ostatus=0;
	u_char door2Cstatus=0;	
	//取出开关门的状态极性
	u_char pol=(PARAMETER_BASE->status_polarity)>>15;
	
	//Door1
	if(DoorNB==1)
	{
		//状态极性为负
		if(pol)
		{
			door1Ostatus = ((~(CurDoorStatus>>3))<<7)&0x80;
			if(door1Ostatus)
				OpenFlag1=1;
			door1Cstatus = ((~(CurDoorStatus>>2))<<7)&0x80;
			if(door1Cstatus)
				OpenFlag1=0;
		}
		//状态极性为正
		else
		{
			door1Ostatus = (CurDoorStatus<<4)&0x80;
			if(door1Ostatus)
				OpenFlag1=1;
			door1Cstatus = (CurDoorStatus<<5)&0x80;
			if(door1Cstatus)
				OpenFlag1=0;		
		}
		//2003.10.23 myw
		doorstatus = (OpenFlag1<<7);
	}
	//Door2
	if(DoorNB==2)
	{
		//状态极性为负
		if(pol)
		{
			door1Ostatus = ((~(CurDoorStatus>>1))<<7)&0x80;
			if(door1Ostatus)
				OpenFlag2=1;
			door1Cstatus = ((~(CurDoorStatus>>0))<<7)&0x80;
			if(door1Cstatus)
				OpenFlag2=0;
		}
		//状态极性为正
		else
		{
			door1Ostatus = (CurDoorStatus<<6)&0x80;
			if(door1Ostatus)
				OpenFlag2=1;
			door1Cstatus = (CurDoorStatus<<7)&0x80;
			if(door1Cstatus)
				OpenFlag2=0;		
		}
		//2003.10.23 myw
		doorstatus = (OpenFlag2<<7);
	}
	//doorstatus = (OpenFlag1<<7)|(OpenFlag2<<7);//2003.10.23  myw
	return(doorstatus);	
}

//*----------------------------------------------------------------------------
//* Function Name       : SingleLinePulseOpen
//* Object              : 开关门状态为开门线单线脉冲
//* Input Parameters    : DoorNB，门号
//* Output Parameters   : 本次开关门状态
//*                     : 0x80表示此时开关门状态为高
//*                     : 0x00表示此时开关门状态为低,或者开关门未接或未检测到开关门
//* 引用的全局变量      : CurDoorStatus
//* 修改的全局变量      : 
//*----------------------------------------------------------------------------
u_char SingleLinePulseOpen(u_char DoorNB)
{

	u_char doorstatus=0;
	u_char door1status=0;
	u_char door2status=0;
	//取出开关门的状态极性
	u_char pol=(PARAMETER_BASE->status_polarity)>>15;
	
	//Door1
	if(DoorNB==1)
	{
		//状态极性为负
		if(pol)
			door1status = ((~(CurDoorStatus>>3))<<7)&0x80;
		//状态极性为正
		else
			door1status = (CurDoorStatus<<4)&0x80;
		//2003.10.23 myw
		doorstatus = door1status;
	}
	//Door2
	if(DoorNB==2)
	{
		//状态极性为负
		if(pol)
			door2status = ((~(CurDoorStatus>>1))<<7)&0x80;
		//状态极性为正
		else
			door2status = (CurDoorStatus<<6)&0x80;
		//2003.10.23 myw
		doorstatus = door2status;
	}
	//doorstatus = door1status|door2status;
	return(doorstatus);	
}

//*----------------------------------------------------------------------------
//* Function Name       : SingleLinePulseClose
//* Object              : 开关门状态为双线脉冲
//* Input Parameters    : DoorNB，门号
//* Output Parameters   : 本次开关门状态
//*                     : 0x80表示此时开关门状态为高
//*                     : 0x00表示此时开关门状态为低,或者开关门未接或未检测到开关门
//* 引用的全局变量      : CurDoorStatus
//* 修改的全局变量      : 
//*----------------------------------------------------------------------------
u_char SingleLinePulseClose(u_char DoorNB)
{
	u_char doorstatus=0;
	u_char door1status=0;
	u_char door2status=0;
	//取出开关门的状态极性
	u_char pol=(PARAMETER_BASE->status_polarity)>>15;
	
	//Door1
	if(DoorNB==1)
	{
		//状态极性为负
		if(pol)
			door1status = ((~(CurDoorStatus>>2))<<7)&0x80;
		//状态极性为正
		else
			door1status = (CurDoorStatus<<5)&0x80;
		//2003.10.23 myw
		doorstatus = door1status;
	}
	//Door2
	if(DoorNB==2)
	{
		//状态极性为负
		if(pol)
			door2status = ((~(CurDoorStatus>>0))<<7)&0x80;
		//状态极性为正
		else
			door2status = (CurDoorStatus<<7)&0x80;
		//2003.10.23 myw
		doorstatus = door2status;
	}
	//doorstatus = door1status|door2status;
	return(doorstatus);	

}

//*----------------------------------------------------------------------------
//* Function Name       : JudgeDoorStatus
//* Object              : 判断本周期的开关门状态
//* Input Parameters    : none
//* Output Parameters   : 本次开关门状态
//*                     : 0x80表示此时开关门状态为高
//*                     : 0x00表示此时开关门状态为低,或者开关门未接或未检测到开关门
//* 引用的全局变量      : CurDoorStatus,Door1Type,Door2Type
//* 修改的全局变量      : 
//*----------------------------------------------------------------------------
u_char JudgeDoorStatus()
{
	u_char door1type;
	u_char door2type;
	u_char curdoorstatus = 0x0;
	u_char door1status=0;
	u_char door2status=0;
	
	door1type = PARAMETER_BASE->Door1Type;
	door2type = PARAMETER_BASE->Door2Type;
	
	//检测到Door1
	if(door1type!=0xff)
	{
		switch(door1type)
		{
			case 0x0b : door1status=DoubleLineLevel(1);       break;
			case 0x0a : door1status=SingleLineLevelOpen(1);   break;
			case 0x09 : door1status=SingleLineLevelClose(1);  break;
			case 0x07 : door1status=DoubleLinePulse(1);       break;
			case 0x06 : door1status=SingleLinePulseOpen(1);   break;
			case 0x05 : door1status=SingleLinePulseClose(1);  break;
			default   : break;
		}
	}
	
	//检测到Door2
	if(door2type!=0xff)
	{
		switch(door2type)
		{
			case 0x0b : door2status=DoubleLineLevel(2);       break;
			case 0x0a : door2status=SingleLineLevelOpen(2);   break;
			case 0x09 : door2status=SingleLineLevelClose(2);  break;
			case 0x07 : door2status=DoubleLinePulse(2);       break;
			case 0x06 : door2status=SingleLinePulseOpen(2);   break;
			case 0x05 : door2status=SingleLinePulseClose(2);  break;
			default   : break;
		}
	}
	curdoorstatus = door1status|door2status;
	return(curdoorstatus);

}
#endif
//*----------------------------------------------------------------------------
//* Function Name       : GetStatus
//* Object              : 获取状态
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      : 状态
//*----------------------------------------------------------------------------
void GetStatus()
{
	u_short bit_val;
	u_int loop_count;
	u_char doorstatus=0;

	STATUS0 = STATUS;
	STATUS = 0;
	u_char i;

	//确定上电信号
	LastPowerOn = PowerOn;
#if POWERON_LINE_EN
	if((PIO_PDSR & POWERON_LINE) != POWERON_LINE)
    	PowerOn = 0;
	else
    	PowerOn = 1;    	
#else    
    if((STATUS & (1<<POWERON)) != 0)
    	PowerOn = 1;
    else
    	PowerOn = 0;
#endif

	//状态数据与状态极性合并
	u_short cs;
	u_short pol=(PARAMETER_BASE->status_polarity)>>7;
	if(PowerOn)
	{
		for(i=1;i<9;i++)
		{
			cs = 1<<i;
			if((pol&cs)==cs)
			{
				if((STATUS&cs)==cs)
					STATUS&=~cs;//置0
				else
					STATUS|=cs;//置1
			}
		}

	}
    #if SectionAlarm_EN
    if(pTable.InOSAlarmCycle&&(pTable.OSAlarmAddupDistance>200))
   		STATUS |= (1<<STATION);
    else
    	STATUS &= ~(1<<STATION);
    #endif
    
	//忽略“上电”状态	
    CurStatus = (u_char)(STATUS>>1); 

    //开关门
    #if OpenDoorDeal
   	doorstatus = JudgeDoorStatus();
    CurStatus=(CurStatus&0x7f)|doorstatus;
//    CurStatus = (CurStatus&0xf0)|CurDoorStatus;
    #endif

}

#if OpenDoorDeal
//*----------------------------------------------------------------------------
//* Function Name       : JudgeChecksum
//* Object              : 判断校验位
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      : 脉冲数、速度值、
//*----------------------------------------------------------------------------
int JudgeChecksum(u_int data)
{
	u_char checksum = 0;
	u_int temp;
	u_char k;
	//判断校验位
	temp = (data>>3);
	for(k=0;k<28;k++)
	{
		if(((temp>>k)&1)==1)
			checksum++;
	}
	if((checksum&1) != ((data>>1)&1))
		return 0;
	return 1;
}
#else
//*----------------------------------------------------------------------------
//* Function Name       : JudgeChecksum
//* Object              : 判断校验位
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      : 脉冲数、速度值、
//*----------------------------------------------------------------------------
int JudgeChecksum(u_short data)
{
	u_char checksum = 0;
	u_char temp;
	u_char k;
	//判断校验位
	temp = (u_char)(data>>2);
	for(k=0;k<8;k++)
	{
		if(((temp>>k)&1)==1)
			checksum++;
	}
	if((checksum&1) != ((data>>1)&1))
		return 0;
	return 1;
}
#endif
//*----------------------------------------------------------------------------
//* Function Name       : ComputeSpeed
//* Object              : 计算速度值
//* Input Parameters    : 脉冲数
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      : 脉冲数、速度值、
//*----------------------------------------------------------------------------
void ComputeSpeed(u_int pulse)
{
	u_int p,t,temp_pulse;
	int i;
	u_int spe;
	u_int T;
	u_char NotZeroNB,ContinueNZ,temp;

//	CurPulse = pulse;	
/*	LastPowerOn = PowerOn;
////////////////////////////////////////////////////////
//	at91_pio_open ( &PIO_DESC,POWERON_LINE, PIO_INPUT );
////////////////////////////////////////////////////////
#if POWERON_LINE_EN
	if((PIO_PDSR & POWERON_LINE) != POWERON_LINE)
    	PowerOn = 0;
	else
    	PowerOn = 1;
#endif

	if((LastPowerOn==0)&&(PowerOn == 1))
		PowerOnT = 0;
	else if(PowerOn&&(PowerOnT<10))
		PowerOnT ++;
	
	if((LastPowerOn==1)&&(PowerOn == 0))
		PowerOffT = 0;
	else if((PowerOn==0)&&(PowerOffT<15))
		PowerOffT++;	
*/	
	// 计算当前速度值 
	u_char PP = PARAMETER_BASE->PulseNumber;
	if((PP==0)||(PP>24))
		PP = 8;
	T = 7200000 / PARAMETER_BASE->CHCO;
	T = (T * 10)/ PP;
	p=0;//t=0;
	NotZeroNB = 0;
	ContinueNZ = 0;
	temp = 0;
	for(i=0;i<SpeedSpace;i++)
	{
		p += LastPN[i];
		if(LastPN[i]>0)
		{
			NotZeroNB++;
			temp++;
		}
		else//==0
		{
			if(temp>ContinueNZ)
				ContinueNZ = temp;
			temp = 0;
		}	
	}
	if(pulse>0)
	{
		NotZeroNB++;
		temp++;
	}
	if(temp>ContinueNZ)
		ContinueNZ = temp;
	

	PulseNB_In1Sec = pulse+p;
	
	spe = T*PulseNB_In1Sec/(2000);
	LastSPE = T * pulse / 400;
	//四舍五入
	if((LastSPE % 10) >= 5)
		LastSPE = LastSPE/10 + 1;
	else
		LastSPE = LastSPE/10;
	
	
	//四舍五入
	if((spe % 10) >= 5)
		spe = spe/10 + 1;
	else
		spe = spe/10;
		
/////////////////2003.5.9//////////////////	
	if((PulseNB_In1Sec>0)&&(spe==0))
		spe = 1;
//////////////////////////////////////////

	if((CurSpeed==0)&&((NotZeroNB<4)||(ContinueNZ<3)))
		spe = 0;
		
	if(spe>255)
		spe = 255;
/*	
	if((CurSpeed==0)&&PowerOn&&(PowerOnT<10))
	{//上电后2秒
		spe = 0;
		pulse = 0;
	}
			
	if((CurSpeed<=1)&&(PowerOn==0)&&(PowerOffT<15))
	{//断电后3秒
		spe = 0;
		pulse = 0;
	}
*/
	DeltaSpeed = spe - CurSpeed;
	
	if(DeltaSpeed>4)//如果上升速度大于4,认为不合理
	{
		DeltaSpeed = 4; 
		spe = CurSpeed + 4;
		t = spe*20000/T;
		if(t>p)
			temp_pulse = t - p;
//			pulse =  t - p;
		else
			temp_pulse = 1;
//			pulse = 1;
	}
	AverageV = spe;	
	CurSpeed = spe;
	CurPulse = pulse;
	
	for(i=0;i<(SpeedSpace-1);i++)
	{
		LastPN[i] = LastPN[i+1];
	}
	LastPN[SpeedSpace-1] = pulse;

    PulseTotalNumber += pulse;//本次行驶总脉冲数
    pTable.OSAlarmAddupDistance += pulse;
	Distance = ComputeDistance100m(PulseTotalNumber);

}
//#if OpenDoorDeal
//*----------------------------------------------------------------------------
//* Function Name       : GetSpeed
//* Object              : 获取速度和开关门状态
//* Input Parameters    : times表示第几次采集
//* Output Parameters   : 采集是否成功
//* 引用的全局变量      :
//* 修改的全局变量      : 脉冲数、速度值、
//*----------------------------------------------------------------------------
 
u_char GetSpeed(u_char times)
{
 	u_int pulse,timer;
	u_int temp;
	u_int p,t;
	int i=0;
	int j=0;
	u_int PIO_Pin_Status;
	u_char RunFlagOfTC2 = 0;
	u_char Received_Bit[4];
	u_int Received_PulseNB = 0;
	CurDoorStatus = LastDoorStatus;            //开关门状态的初值应为上一次的状态
	pulse = LastPN[SpeedSpace-1];
	
	u_char  SmallLimit=200,LargeLimit=250;        //200为10ms,250为12.5ms,同步头为12ms
////////////////////////////////////////////////////////
	at91_pio_open ( &PIO_DESC,SPEED, PIO_INPUT );
////////////////////////////////////////////////////////

	CurTN = 0;     //0.05ms定时器
	//读取同步头
	do
	{
		PIO_Pin_Status = PIO_PDSR & SPEED;
		
		if((PIO_Pin_Status == 0)&& (RunFlagOfTC2 ==0))
		{
			//打开计数器，开始计时
			at91_tc_trig_cmd(&TC2_DESC, TC_TRIG_CHANNEL);
			temp=TC2_SR;
			TC2_IER = TC_CPCS;
			RunFlagOfTC2 = 1;
			j=0;
		}
		else if(PIO_Pin_Status != 0)
		{
			//计时结束，读定时器
			if((CurTN > SmallLimit)&&(CurTN < LargeLimit))
			{
				CurTN = 0;
				break;
			}
			else
			{
				TC2_IDR = TC_CPCS;
				RunFlagOfTC2 = 0;
				CurTN = 0;
			}
			j++;
			i=0;
		}
		else
		{
			i++;
			j=0;
		}		
	}while((i<5000)&&(j<5000));
	
	if((i>=5000)||(j>=5000))
	{
		if(times)
			ComputeSpeed(pulse);
		return 0;
	}
	
	//读取速度脉冲数
	u_char BitNB=32;
	for(i=0;i<BitNB;i++)
	{
		t=0;
		while((CurTN==0)&&(t<20))
			t++;
		if((PIO_PDSR & SPEED)==SPEED)
			Received_Bit[j] = 1;
		else
			Received_Bit[j] = 0;
			
		t=0;
		while((CurTN==1)&&(t<20))
			t++;
		if((PIO_PDSR & SPEED)==SPEED)
			Received_Bit[1] = 1;
		else
			Received_Bit[1] = 0;
			
		t=0;
		while((CurTN==2)&&(t<20))
			t++;
		if((PIO_PDSR & SPEED)==SPEED)
			Received_Bit[2] = 1;
		else
			Received_Bit[2] = 0;
		
		t=0;
		while((CurTN==3)&&(t<20))
			t++;
		if((PIO_PDSR & SPEED)==SPEED)
			Received_Bit[3] = 1;
		else
			Received_Bit[3] = 0;
			
		Received_Bit[0] =Received_Bit[0]+Received_Bit[1]+Received_Bit[2]+Received_Bit[3];
		Received_PulseNB = Received_PulseNB << 1;
		if(Received_Bit[0]>2)
			Received_PulseNB = Received_PulseNB+1;
				
		t=0;
		while((CurTN<=4)&&(t<20));
			t++;
		CurTN = 0;
	}
	TC2_IDR = TC_CPCS;
	//判断起始位和停止位
	if(((Received_PulseNB&2)!=0)||((Received_PulseNB&0x80000000)!=0x80000000))
	{
		if(times)
			ComputeSpeed(pulse);
		return 0;
	} 
 	 
 	//判断速度校验,panhui,2005-04-29
 	u_char spd_chk;
 	spd_chk = (u_char)(Received_PulseNB>>15);
	pulse = (u_char)(Received_PulseNB>>23);
	if((pulse+spd_chk)!=0xff)
	{
		if(times)
			ComputeSpeed(pulse);
		return 0;
	} 
	/////////////////////////////////////////
	if(!JudgeChecksum(Received_PulseNB))
	{
		if(times)
			ComputeSpeed(pulse);
		return 0;
	}
	//判断USB通讯标志和校验位
	if((!USB_Communicate_Flag))
	{
		STATUS0 = STATUS;
		STATUS = (u_char)(Received_PulseNB>>3);
//		CurDoorStatus = (u_char)((Received_PulseNB>>2)&0x0f);
//		LastDoorStatus = CurDoorStatus;
	}
	else
	{
//		pulse = 0;
		pulse = LastPN[SpeedSpace-1];
//		CurDoorStatus = LastDoorStatus;
	}
	USB_Communicate_Flag = 0;  
	ComputeSpeed(pulse);
	
	for(i=0;i<BitNB;i++)
	{
		for(j=0;j<4;j++)
		{
			buff[i][j]=0;
		}
	}
	
	////////////////////////add by panhui 2005-03-17//////////////////////
	////////////////////////for 状态采集//////////////////////////////////
/*	u_char sta;
	sta = (CurDoorStatus<<2)&0x30;
	sta = sta + ((CurDoorStatus<<1)&0x06);
	STATUS0 = STATUS;
	STATUS = sta;
*/	
	//状态数据与状态极性合并
	u_short cs;
	u_short pol=(PARAMETER_BASE->status_polarity)>>7;
		for(i=1;i<9;i++)
		{
			cs = 1<<i;
			if((pol&cs)==cs)
			{
				if((STATUS&cs)==cs)
					STATUS&=~cs;//置0
				else
					STATUS|=cs;//置1
			}
		}

	//忽略“上电”状态	
    CurStatus = (u_char)(STATUS>>1); 

	//////////////////////////////////////////////////////////////////////
}
 
 
 
#if GetSpeedStatusBy232
//*************************************************************
//*************************************************************
//start    用串口与适配器通讯同时获取速度和全部状态   start
//*************************************************************
//*************************************************************
//*----------------------------------------------------------------------------
//* Function Name            : rs232_status_ready
//* Object                   : 判断232接口是否准备好
//* Input Parameters         : status——存放232接口状态寄存器(US_CSR)的内存单元；
//*                          : mask——屏蔽位(0x01：RXRDY)
//* Output Parameters        : 是否准备好,"1"表示receiver就绪，
//*                          : "0"表示receiver未就绪或等待超过界限,本函数的界限是100次
//* Global  Variable Quoted  : none
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
int rs2320_status_ready(u_int *status, u_int mask)
{
	int j;
	j=0;
	do
	{
		*status = at91_usart_get_status(RS2320);
		j++;
	}while((((*status) & mask) != mask)&&(j<5000));
	
	if((j>=5000)&&(((*status)&mask)==0))
	{
		return FALSE;
	}
	return TRUE;
}


//*----------------------------------------------------------------------------
//* Function Name       : GetStatusAndSpeed
//* Object              : 获取状态
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      : 状态
//*----------------------------------------------------------------------------
void GetStatusAndSpeed()
{
	u_char i,j=0;
	u_int  status232;
	u_int  rhr;
	u_char CheckSum = 0;
	u_int  pulse;
	u_char receiveNB = 6;
	u_char command1 = 0xb5;
	u_char command2 = 0xa5;
	u_char doorstatus=0;
	u_char TempStatus;
	u_char rxBuf[6];
	
	//确定上电信号
////////////////////////////////////////////////////////
//	at91_pio_open ( &PIO_DESC,POWERON_LINE, PIO_INPUT );
////////////////////////////////////////////////////////
#if POWERON_LINE_EN
	if((PIO_PDSR & POWERON_LINE) != POWERON_LINE)
    	PowerOn = 0;
	else
    	PowerOn = 1;
#else    
    if((STATUS & (1<<POWERON)) != 0)
    	PowerOn = 1;
    else
    	PowerOn = 0;
#endif
	
	//发送命令字
	while((at91_usart_get_status(RS2320) & 0x02) != 0x02);
	at91_usart_write(RS2320,command1);
	while((at91_usart_get_status(RS2320) & 0x02) != 0x02);
	at91_usart_write(RS2320,command2);
		
	
	//如果不超时，从2051收取速度和状态
	for(i = 0;i < receiveNB;i++)
	{
		if(!rs2320_status_ready(&status232, 0x01))
		{
			CurStatus = LastStatus;
			pulse = LastPN[SpeedSpace-1];
			ComputeSpeed(pulse);
			return;
		}
		
		at91_usart_read(RS2320,&rhr);
		rxBuf[i] = (u_char)rhr;
	}
	//收取命令字正确
	if((rxBuf[0]==command1)&&(rxBuf[1]==command2))
	{
		//计算校验和
		for(i=2;i<(receiveNB-1);i++)
			CheckSum = CheckSum^rxBuf[i];
		//校验和正确
		if(rxBuf[receiveNB-1]==CheckSum)
		{
			CurDoorStatus = rxBuf[4];
			TempStatus = rxBuf[3];

			//状态数据与状态极性合并
			u_short cs;
			u_short pol=(PARAMETER_BASE->status_polarity)>>7;
			if(PowerOn)
			{
				for(i=1;i<9;i++)
				{
					cs = 1<<i;
					if((pol&cs)==cs)
					{
						if((TempStatus&cs)==cs)
							TempStatus&=~cs;//置0
						else
							TempStatus|=cs;//置1
					}
				}
		
			}
			
		    #if SectionAlarm_EN
		    if(pTable.InOSAlarmCycle&&(pTable.OSAlarmAddupDistance>200))
		   		TempStatus |= (1<<STATION);
		    else
		    	TempStatus &= ~(1<<STATION);
		    #endif
		    
			//忽略“上电”状态	
		    CurStatus = (u_char)(TempStatus>>1);
		    //开关门
			doorstatus = JudgeDoorStatus();
		    CurStatus=(CurStatus&0x7f)|doorstatus;
		    
		    LastStatus = CurStatus; 
			pulse = rxBuf[2];
			ComputeSpeed(pulse);
			return;
		}
		//校验和错误
		else
		{
			CurStatus = LastStatus;
			pulse = LastPN[SpeedSpace-1];
			ComputeSpeed(pulse);
			return;
		}
	}
	//收取命令字错误
	else
	{
		CurStatus = LastStatus;
		pulse = LastPN[SpeedSpace-1];
		ComputeSpeed(pulse);
		return;    
	}
}
//*************************************************************
//*************************************************************
//end    用串口与适配器通讯同时获取速度和全部状态   end
//*************************************************************
//*************************************************************
#endif