//*----------------------------------------------------------------------------
//*      锟斤拷荽锟斤拷锟斤拷映锟斤拷锟�
//*----------------------------------------------------------------------------
//* File Name           : DataManager.c
//* Object              : 锟斤拷录锟角采硷拷锟斤拷锟劫讹拷状态锟斤拷莸拇锟斤拷锟酵憋拷锟斤拷
//*
//* 1.0 24/02/03 PANHUI : Creation
//*----------------------------------------------------------------------------
//
#include	"DataManager.h"
#include   "atmel_dataflash.h"
#include "application.h"
#include    "lcd.h" 
#include <sys/types.h>
#include  <time.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>


extern unsigned long CurSpeed;
extern unsigned long RsSpeed;
extern int DeltaSpeed;
extern CLOCK curTime;
extern unsigned char CurStatus;
extern unsigned long LastSPE;
extern unsigned char TimeChange;	//时锟斤拷浠拷锟街�
extern unsigned long AddupSpeed;
extern unsigned short SpeedNb;
PartitionTable pTable;
StructPara Parameter;
extern unsigned long PulseTotalNumber;	/*锟斤拷锟斤拷锟斤拷驶锟斤拷锟斤拷锟斤拷锟斤拷*/
extern unsigned short STATUS;			/*16锟斤拷状态*/
extern unsigned long PulseNB_In1Sec;     //每0.2锟斤拷锟斤拷锟斤拷前锟斤拷1锟斤拷锟斤拷锟桔硷拷锟劫讹拷锟斤拷锟斤拷锟斤拷
extern unsigned char PowerOn;
extern unsigned long CurEngine;
unsigned short DriveMinuteLimit;       //疲锟酵硷拷驶锟斤拷驶时锟斤拷锟斤拷锟斤拷
unsigned short RestMinuteLimit;        //疲锟酵硷拷驶锟斤拷锟斤拷锟斤拷息时锟斤拷锟斤拷锟斤拷
extern unsigned long CurPulse;
unsigned long SpeedOf1min;
 
DoubtDataBlock ddb;			//锟斤拷前锟缴碉拷锟斤拷菘锟�
BaseDataBlock basedata;
SizeData location;
OverDriverBlock overdriverdata;
DriverRegisterBlock driverregisterdata;
PowerBlock powerdata;
ParaModifyBlock modifydata;
JournalBlock journaldata;
CLOCK startdriverclk;
CLOCK enddriverclk;

unsigned char InRecordCycle=0;		//锟角凤拷锟节硷拷录锟斤拷莨锟斤拷锟斤拷
unsigned char speeddel;
Datastatus Datastatusdata;
unsigned char InFlashWriting=0;	//锟斤拷FLASH写锟斤拷锟斤拷锟斤拷
unsigned long Tick02NB;				//锟斤拷锟斤拷停锟斤拷之锟斤拷0.2s锟侥革拷锟斤拷
OTDR_end otdrEND;			//疲锟酵硷拷驶锟斤拷录锟斤拷锟斤拷锟斤拷锟斤拷
unsigned long AddupSpeed = 0;		//1锟斤拷锟斤拷锟劫讹拷锟桔硷拷
unsigned short SpeedNb = 0;		//1锟斤拷锟斤拷锟劫讹拷值锟斤拷锟斤拷
unsigned char PowerOnTime=0;		//锟较碉拷锟斤拷锟绞憋拷锟�
unsigned char OTRecordType=0;		//锟斤拷锟斤拷疲锟酵硷拷驶锟斤拷录锟斤拷锟斤拷
unsigned long LastDistance;			//锟较达拷疲锟酵硷拷驶锟斤拷录锟桔硷拷锟斤拷锟�
unsigned char STATUS1min;			//1锟斤拷锟斤拷状态锟斤拷
unsigned char DriverStatus = 0;
unsigned char DriverRegstatus = 0;
unsigned char powerstatus = 0;
unsigned char paramodifystatus = 0;
unsigned char JournalRecordstatus = 0;
unsigned char DoubltPointstatus = 0;
DRIVER CurrentDriver;		//锟斤拷前司锟斤拷
DRIVER RecordDriver;		//锟斤拷录司锟斤拷
Record_CLOCK PowerOnDT;     //锟较碉拷锟斤拷锟斤拷时锟斤拷
RecordData_end rec_end;
unsigned char FinishFlag=0;

extern unsigned char LargeDataBuffer[];
unsigned short *DoubtData_4k = (unsigned short *)(&(LargeDataBuffer[12*1024]));//[2*1024];
unsigned char *OTDR_4k = &(LargeDataBuffer[16*1024]);//[4*1024];
unsigned short *BaseData_4k = (unsigned short *)(&(LargeDataBuffer[20*1024]));//[2*1024];
unsigned char *Location_4k = &(LargeDataBuffer[28*1024]);

//*----------------------------------------------------------------------------
//* Function Name       : Task_GetData
//* Object              : 锟皆硷拷锟斤拷锟�
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
void SelfCheck()
{
	int clock_succ = 1;//GetCurrentDateTime();
	int sl811_succ = sl811s_check();
	if(clock_succ && sl811_succ)
		DisplayOK();
	else if(!clock_succ)
		DisplayClockError();
	else
		DisplayError();
		
	int i;
	for(i=0;i<200000;i++);
	lcd_clear(lineall);
}

//*----------------------------------------------------------------------------
//* Function Name       : StrCmp
//* Object              : 锟饺斤拷锟斤拷锟斤拷锟街凤拷锟角凤拷锟斤拷同
//* Input Parameters    : 锟斤拷冉系锟斤拷锟斤拷锟斤拷址锟絪tr1,str2,锟斤拷锟斤拷length
//* Output Parameters   : 1锟斤拷锟斤拷锟街凤拷锟斤拷同锟斤拷0锟斤拷锟斤拷锟街凤拷同
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
char StrCmp(unsigned char *str1, unsigned char *str2, short length)
{
	short i;
	char ret;
	ret=1;
	for(i=0;i<length;i++)
	{
		if(str1[i]!=str2[i]){
			ret = 0;
			break;
		}
	}
	return(ret);
}
//*----------------------------------------------------------------------------
//* Function Name       : BCD2Char
//* Object              : BCD锟斤拷转锟斤拷为十锟斤拷锟斤拷锟斤拷
//* Input Parameters    : bcd锟斤拷锟斤拷锟斤拷写转锟斤拷锟斤拷BCD锟斤拷
//* Output Parameters   : 转锟斤拷锟斤拷锟绞拷锟斤拷锟斤拷锟�
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
unsigned char BCD2Char(unsigned char bcd)
{
	unsigned char ch;
	unsigned char d1,d0;
	d1=((bcd & 0xf0)>>4);
	if(d1>9)
		return (0xff);
	d0=bcd & 0x0f;
	if(d0>9)
		return (0xff);
	ch = d1*10 + d0;
	return(ch);
}
time_t timechange(CLOCK time)
{
	time_t now;
	struct tm ti;
	ti.tm_year =   BCD2Char(time.year) +100;
	ti.tm_mon 	= BCD2Char(time.month )- 1; /* ti->tm_mon 	= month; 0~11 */
	ti.tm_mday = BCD2Char(time.day);
	ti.tm_hour = BCD2Char(time.hour);
	ti.tm_min 	= BCD2Char(time.minute);
	ti.tm_sec 	= 0;
    now = mktime(&ti);
    return now;
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteParameterTable
//* Object              : 写锟斤拷锟斤拷锟�
//* Input Parameters    : none
//* Output Parameters   : 锟角凤拷晒锟�
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int WriteParameterTable(StructPara *para)
{
	unsigned long *data;
	unsigned long *p;
	int i,size;

	//锟斤拷锟斤拷4k
	p = (unsigned long *)PARAMETER_BASE;
	SPI_FLASH_Sector4kErase(SPI1,*p);
	
	
	//锟斤拷锟斤拷锟斤拷锟叫达拷锟紽LASH
	data = (unsigned long *)(para);
	size=sizeof(StructPara);

	SPI_FLASH_BufferWrite( SPI1 ,(u8 *)data, *p, size);

	
	return ( TRUE ) ;
}
//*----------------------------------------------------------------------------
//* Function Name       : WritePartitionTable
//* Object              : 写锟斤拷锟斤拷锟�
//* Input Parameters    : none
//* Output Parameters   : 锟角凤拷晒锟�
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int WritePartitionTable(PartitionTable *ptt)
{
	unsigned long *data;
	unsigned long *p;
	int i,size;

	//锟斤拷锟斤拷4k
	p = (unsigned long *)PartitionTable_BASE;
	SPI_FLASH_Sector4kErase(SPI1,*p);
	
	//锟斤拷锟斤拷锟斤拷锟叫达拷锟紽LASH
	if(ptt->TotalDistance==0xffffffff)
		ptt->TotalDistance = 0;
	data = (unsigned long *)(ptt);
	size=sizeof(PartitionTable);
	
	SPI_FLASH_BufferWrite( SPI1 ,(u8 *)data, *p, size);
		
	return ( TRUE ) ;
}
//*----------------------------------------------------------------------------
//* Function Name       : IsPartitionTableCorrect
//* Object              : 锟斤拷始锟斤拷锟斤拷锟斤拷锟�
//* Input Parameters    : none
//* Output Parameters   : 写锟斤拷锟斤拷锟斤拷欠锟缴癸拷
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int IsPartitionTableCorrect()
{
	unsigned long *data;
	unsigned long *p;
	int i,size;
	
	/////////*******2003.10.06 panhui*********////////
	unsigned short temp = PartitionTableFlag;
	if( pTable.Available != temp)
		return(0);
	/////////*******2003.10.06 panhui*********////////
	if(	pTable.DoubtPointData.BaseAddr != DPD_BASE)
		return(0);
	if(	pTable.DoubtPointData.EndAddr != DPD_END)
		return(0);

	if(	pTable.BaseData.BaseAddr != BASEDATA_BASE)
		return(0);
	if(	pTable.BaseData.EndAddr != BASEDATA_END)
		return(0);

	/////////////modified by panhui 2003.10.20////////////
	if((pTable.DoubtPointData.CurPoint < DPD_BASE)||(pTable.DoubtPointData.CurPoint > DPD_END-110))
		return(-1);

	if((pTable.BaseData.CurPoint < BASEDATA_BASE)||(pTable.BaseData.CurPoint > BASEDATA_END))
		return(-3);
	/////////////锟斤拷止指锟斤拷锟杰凤拷/////////////////////////////

	//锟斤拷锟斤拷锟斤拷锟叫达拷锟紽LASH
	return (1);
	

}
//*----------------------------------------------------------------------------
//* Function Name       : InitializeTable
//* Object              : 锟斤拷始锟斤拷锟斤拷锟斤拷锟�
//* Input Parameters    : unsigned char parti锟斤拷锟斤拷锟角凤拷锟斤拷锟铰恢革拷锟斤拷锟斤拷锟�
//                        unsigned char para锟斤拷锟斤拷锟角凤拷锟斤拷锟铰恢革拷锟斤拷锟斤拷锟�
//                        unsigned char change_set锟斤拷锟斤拷锟角凤拷锟斤拷锟斤拷锟斤拷锟矫筹拷锟狡号ｏ拷
//                                             锟斤拷锟斤拷锟桔硷拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
//* Output Parameters   : 锟斤拷始锟斤拷锟斤拷锟斤拷锟酵诧拷锟斤拷锟斤拷欠锟缴癸拷
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int InitializeTable()
{
	int i;
	SPI_FLASH_BufferRead(SPI1,(unsigned char *)&pTable,PartitionTable_BASE,sizeof(pTable));
	SPI_FLASH_BufferRead(SPI1,(unsigned char *)&Parameter,PartitionTable_BASE,sizeof(pTable));
	if(Parameter.mark != 0xaeae)
	{
		//
		Parameter.mark = 0xaeae;
		if( !WriteParameterTable(&Parameter) )
			return (0);
	}
	
	if(pTable.Available !=PartitionTableFlag)
	{
		//初始化pTable
		pTable.Available = PartitionTableFlag;
		pTable.DoubtPointData.BaseAddr = DPD_BASE;
		pTable.DoubtPointData.EndAddr = DPD_END;
		pTable.DoubtPointData.CurPoint = DPD_BASE;

		pTable.BaseData.BaseAddr = BASEDATA_BASE;
		pTable.BaseData.EndAddr = BASEDATA_END;
		pTable.BaseData.CurPoint = BASEDATA_BASE;

		pTable.DriverReRecord.BaseAddr = DRVRG_BASE;
		pTable.DriverReRecord.EndAddr = DRVRG_END;
		pTable.DriverReRecord.CurPoint = DRVRG_BASE;

		pTable.LocationData.BaseAddr = LOCATION_BASE;
		pTable.LocationData.EndAddr = LOCATION_END;
		pTable.LocationData.CurPoint = LOCATION_BASE;

		pTable.OverSpeedRecord.BaseAddr = OVERDRV_BASE;
		pTable.OverSpeedRecord.EndAddr = OVERDRV_END;
		pTable.OverSpeedRecord.CurPoint = OVERDRV_BASE;

		pTable.ModifyRecord.BaseAddr = PARA_BASE;
		pTable.ModifyRecord.EndAddr = PARA_END;
		pTable.ModifyRecord.CurPoint = PARA_BASE;

		pTable.PowerOffRunRecord.BaseAddr = POWER_BASE;
		pTable.PowerOffRunRecord.EndAddr = POWER_END;
		pTable.PowerOffRunRecord.CurPoint = POWER_BASE;

		pTable.journalRecord.BaseAddr = JN_BASE;
		pTable.journalRecord.EndAddr = JN_END;
		pTable.journalRecord.CurPoint = JN_BASE;

		if( !WritePartitionTable(&pTable) )
			return(0);
	}
	
	return (1);
}
//*----------------------------------------------------------------------------
//* Function Name       : UpdateParameterPartition
//* Object              : 锟斤拷锟铰诧拷锟斤拷锟斤拷锟斤拷锟�
//* Input Parameters    : none
//* Output Parameters   : 锟铰诧拷锟斤拷锟斤拷锟斤拷锟斤拷欠锟缴癸拷
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int UpdateParameterPartition()
{
	unsigned long sector_addr;
	sector_addr = 0;
	unsigned long *data;
	unsigned long *p;
	int i,size;
	
	//锟斤拷锟斤拷FLASH
	SPI_FLASH_Sector4kErase(SPI1,*p);
	//刷锟铰诧拷锟斤拷锟�
	p = (unsigned long *)PARAMETER_BASE;
	data = (unsigned long *)(&Parameter);
	size=sizeof(StructPara);
	
	SPI_FLASH_BufferWrite( SPI1 ,(u8 *)data, *p, size);
	
	//刷锟铰凤拷锟斤拷锟�
	p = (unsigned long *)PartitionTable_BASE;
	data = (unsigned long *)(&pTable);
	size=sizeof(PartitionTable);
	
	SPI_FLASH_BufferWrite( SPI1 ,(u8 *)data, *p, size);
	return(1);
}
//*----------------------------------------------------------------------------
//* Function Name       : Update4k
//* Object              : FLASH锟斤拷4K锟斤拷锟斤拷锟节存，锟斤拷锟斤拷前4k锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟饺凤拷锟斤拷欠锟�
//                        锟斤拷指锟斤拷之前锟斤拷锟斤拷锟叫达拷锟紽LASH
//* Input Parameters    : p锟斤拷锟斤拷锟斤拷前锟斤拷锟街革拷锟�
//*                       Buffer锟斤拷锟斤拷锟节达拷锟叫达拷锟斤拷碌锟�k锟斤拷锟斤拷锟斤拷椎锟街�
//*                       type锟斤拷锟斤拷锟斤拷锟斤拷
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int Update4k(unsigned long p,unsigned char *Buffer,unsigned char type)
{
	unsigned short i;
	unsigned long sector_addr;
	sector_addr = (p & 0xfffff000);
	SPI_FLASH_BufferRead(SPI1,Buffer,sector_addr,4096);
	if(type == UpdateFlashOnce)
		return(1); 
		
	//锟斤拷锟斤拷FLASH
	SPI_FLASH_Sector4kErase(SPI1,sector_addr);
	//指锟斤拷之前锟斤拷锟斤拷锟斤拷锟斤拷锟叫达拷锟紽LASH
	if(type == UpdateFlashTimes)
	{
		i=0;
		while(sector_addr<p)
		{
			SPI_FLASH_BufferWrite( SPI1 ,(u8 *)(&Buffer[i]), sector_addr, 2);
			sector_addr++;
			i++;
		}
	}
	return(1);
}
int erase4kflash(unsigned long p ,unsigned long num)
{
	if (num == 0)
		return 0;
	if((p &0x00000fff)== 0)
	{			//Update4k( p,&Location_4k,UpdateFlashTimes);
		SPI_FLASH_Sector4kErase(SPI1,p);

	}
	else if(((p+num)&0x00000fff)<num)
	{
		p = (p+num)&0xfffff000;
		SPI_FLASH_Sector4kErase(SPI1,p+6);
	}
	return 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteDoubtDataToFlash
//* Object              : 写一锟斤拷锟缴碉拷锟斤拷莸锟紽LASH锟斤拷
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : curTime
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int WriteDoubtDataToFlash()
{
	
}

#if 0
unsigned Char2BCD(unsigned char ch)
{
	unsigned char bcd;
	if(ch >99)
		return (0xff);
	bcd = (((ch/10)<< 4)&0xf0)+(ch%10);
	return (bcd);

}
#endif
//*----------------------------------------------------------------------------
//* Function Name       : IfOneAfterAotherDay
//* Object              : 锟叫讹拷锟角凤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
//* Input Parameters    : time锟斤拷锟斤拷锟斤拷锟斤拷碌锟绞憋拷锟街革拷锟�
//*                       end锟斤拷锟斤拷锟斤拷一锟斤拷疲锟酵硷拷录锟斤拷锟斤拷锟斤拷锟斤拷指锟斤拷
//* Output Parameters   : 0锟斤拷锟斤拷锟斤拷锟角ｏ拷1锟斤拷锟斤拷锟斤拷
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int IfOneAfterAotherDay(CLOCK *time,OTDR_end *end)
{
	unsigned char day,month,year;
	day = BCD2Char(time->day);
	if(day>1){
		day --;
		if((day==BCD2Char(end->dt.day))&&(time->month==end->dt.month)&&(time->year==end->dt.year))
			return(1);
		else
			return(0);
	}
	else{
		year = BCD2Char(time->year);
		month = BCD2Char(time->month);
		if(month>1)
			month--;
		else{
			month = 12;
			if(year>1)
				year--;
			else
				year=99;
		}
			
		switch(month){
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				day = 31;break;
			case 4:
			case 6:
			case 9:
			case 11:
				day = 30;break;
			case 2:
				if((year&0x03)==0)
					day = 29;
				else
					day = 28;
				break;
			default:
				return(0);	
		}
		
		if((day==BCD2Char(end->dt.day))&&(month==BCD2Char(end->dt.month))&&(year==BCD2Char(end->dt.year)))
			return(1);
		else
			return(0);

		
	}
	
}
//*----------------------------------------------------------------------------
//* Function Name       : JudgeTimeSpace
//* Object              : 锟叫讹拷时锟斤拷锟斤拷
//* Input Parameters    : time锟斤拷锟斤拷锟斤拷锟斤拷碌锟绞憋拷锟街革拷锟�
//*                       end锟斤拷锟斤拷锟斤拷一锟斤拷疲锟酵硷拷录锟斤拷锟斤拷锟斤拷锟斤拷指锟斤拷
//* Output Parameters   : 时锟斤拷锟斤拷值锟斤拷锟斤拷锟接ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷20锟斤拷锟斤拷时锟斤拷锟斤拷FF
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
unsigned char JudgeTimeSpace(CLOCK *time,OTDR_end *end)
{
	int space,conti;
	unsigned char ret;
	if((time->year==end->dt.year)&&(time->month==end->dt.month)&&(time->day==end->dt.day))
	{
		space = BCD2Char(time->hour)*60 + BCD2Char(time->minute) - BCD2Char(end->dt.hour)*60 - BCD2Char(end->dt.minute);
		if((space<0)||(space>20))
			ret=0xff;
		else
			ret=(unsigned char)space;
			
	}
	else
	{
		conti = IfOneAfterAotherDay(time,end);
		if(conti){
			space = 24*60 + BCD2Char(time->hour)*60 + BCD2Char(time->minute) - BCD2Char(end->dt.hour)*60 - BCD2Char(end->dt.minute);
			if((space<0)||(space>20))
				ret=0xff;
			else
				ret=(unsigned char)space;
		}
		else
			ret = 0xff;
	}
	return ret;
}
//*----------------------------------------------------------------------------
//* Function Name       : AddPointer
//* Object              : 锟斤拷锟斤拷指锟斤拷锟斤拷锟斤拷偏锟斤拷锟斤拷
//* Input Parameters    : pt锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟结构
//*                       inc锟斤拷锟斤拷锟桔硷拷值
//* Output Parameters   : 锟斤拷锟斤拷锟侥斤拷锟�
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
unsigned long AddPointer(StructPT *pt, int inc)
{
	unsigned long result;
	if(inc>=0)
	{
		result = pt->CurPoint + inc;
		if(result > pt->EndAddr)
			result = pt->BaseAddr + ( result - pt->EndAddr ) - 1 ;
	}
	else
	{
		result = pt->CurPoint + inc;
		if(result < pt->BaseAddr)
			result = pt->EndAddr - (pt->BaseAddr - result) + 1 ;
	}
	return(result);
}
//*----------------------------------------------------------------------------
//* Function Name       : Get_otdrEND
//* Object              : 锟斤拷取疲锟酵硷拷驶锟斤拷锟斤拷锟斤拷锟捷诧拷锟揭革拷锟斤拷锟斤拷莼锟斤拷锟斤拷锟�
//* Input Parameters    : none
//* Output Parameters   : none
//* Global Parameters   : otdrEND,InFlashWriting
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int Get_otdrEND(OTDR_start *start,OTDR_end *end)
{

		return(2);

}
//*----------------------------------------------------------------------------
//* Function Name       : Write4kToFlashOTDR
//* Object              : 锟饺诧拷锟斤拷锟斤拷写4k锟节达拷锟斤拷莸锟紽LASH锟斤拷
//* Input Parameters    : p锟斤拷锟斤拷锟斤拷前锟斤拷锟街革拷锟�
//*                       Buffer锟斤拷锟斤拷锟节达拷锟叫达拷锟斤拷碌锟�k锟斤拷锟斤拷锟斤拷椎锟街�
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int Write4kToFlashOTDR(unsigned short *p,unsigned short *Buffer)
{
	//锟斤拷锟斤拷FLASH
	if(!flash_sst39_erase_sector((unsigned long *)DATAFLASH_BASE,(unsigned long *)p))
		return (0);

	return(Write4kToFlash(p,Buffer));
}
//*----------------------------------------------------------------------------
//* Function Name       : Write4kToFlash
//* Object              : 写4k锟节达拷锟斤拷莸锟紽LASH锟斤拷
//* Input Parameters    : p锟斤拷锟斤拷锟斤拷前锟斤拷锟街革拷锟�
//*                       Buffer锟斤拷锟斤拷锟节达拷锟叫达拷锟斤拷碌锟�k锟斤拷锟斤拷锟斤拷椎锟街�
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int Write4kToFlash(unsigned short *p,unsigned short *Buffer)
{
	unsigned short *inside_p,*flash_p;
	unsigned short data;
	inside_p = (unsigned short *)((unsigned long)p & 0x00fff);//4k锟节碉拷址指锟斤拷
	flash_p = p;
	while((unsigned long)inside_p<0x01000)
	{
		data = *((unsigned short *)((unsigned long)Buffer + (unsigned long)inside_p));
		SPI_FLASH_BufferWrite( SPI1 ,(u8 *)&data, *flash_p, 2);
		inside_p++;
		flash_p++;
	}
	return ( 1 );
}

//*----------------------------------------------------------------------------
//* Function Name       : WriteAverageSpeed
//* Object              : 写一锟斤拷平锟斤拷锟劫讹拷锟斤拷莸锟紽LASH锟斤拷
//* Input Parameters    : V锟斤拷锟斤拷锟斤拷写锟斤拷锟狡斤拷锟斤拷俣锟街�
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
void WriteAverageSpeed(unsigned char v)
{
	unsigned char *p;
	unsigned long pos,pt,last_pos;
#if 0
	pos = 0x00fff & pTable.RunRecord360h.CurPoint;
	p = (unsigned char *)((unsigned long)(OTDR_4k) + pos);
	*p = v;
	pos++;	

	if(pos==0x01000)
	{//锟叫伙拷锟斤拷锟斤拷一锟斤拷4k
		//锟斤拷锟铰碉拷址指锟斤拷
		p=(unsigned char *)(OTDR_4k);
		last_pos = pTable.RunRecord360h.CurPoint&0xfffff000;
		pos = (pTable.RunRecord360h.CurPoint&0xff000) + 0x01000;
		if(pos>(pTable.RunRecord360h.EndAddr&0xff000))
			pos = pTable.RunRecord360h.BaseAddr;
		else
			pos += (unsigned long)DATAFLASH_BASE;
		pTable.RunRecord360h.CurPoint = pos;
	
		//写锟斤拷前4k锟斤拷锟斤拷锟斤拷FLASH锟斤拷
		Write4kToFlashOTDR((unsigned short *)last_pos,(unsigned short *)OTDR_4k);
		//锟斤拷锟铰伙拷锟斤拷锟斤拷
		//Update4k((unsigned short *)pos,(unsigned short *)OTDR_4k,UpdateFlashOnce);
	}
	else
		pTable.RunRecord360h.CurPoint++;	 
#endif

}
//*----------------------------------------------------------------------------
//* Function Name       : FinishOTDRToFlash
//* Object              :锟斤拷锟斤拷前锟斤拷录360小时平锟斤拷锟劫讹拷锟斤拷驶锟斤拷录锟斤拷统锟斤拷疲锟酵硷拷驶锟节达拷
//                       锟斤拷锟芥到FLASH锟斤拷
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
void FinishOTDRToFlash()
{
	unsigned long p;
	otdrEND.driver = RecordDriver;
	if((InRecordCycle&(1<<DOUBLTPOINT))!=0)
	{
		//锟斤拷录锟斤拷锟斤拷锟斤拷莸锟�
		otdrEND.dt.type = 0xeaea;
		otdrEND.dt.year = curTime.year;
		otdrEND.dt.month = curTime.month;
		otdrEND.dt.day = curTime.day;
		otdrEND.dt.hour = curTime.hour;
		otdrEND.dt.minute = curTime.minute;
		otdrEND.dt.second = curTime.second;
		otdrEND.TotalDistance = PulseTotalNumber - otdrEND.TotalDistance;
		if(OTRecordType == MergeLastData)
			otdrEND.TotalDistance += LastDistance;
	}
	
	WriteOTDREndData(&otdrEND);
#if 0
	if(pTable.RunRecord360h.CurPoint!=pTable.RunRecord360h.CurPoint)
	{
		p = pTable.RunRecord360h.CurPoint;
		p &= 0xfffff000;
		Write4kToFlashOTDR((unsigned short *)p,(unsigned short *)OTDR_4k);
	}
#endif
}
//*----------------------------------------------------------------------------
//* Function Name       : ModifyUnknownDriver
//* Object              : 每锟斤拷锟斤拷驶锟斤拷录锟斤拷锟斤拷时锟斤拷锟斤拷录锟叫硷拷录锟斤拷未知司锟斤拷锟斤拷锟斤拷锟藉卡锟斤拷
//						  锟斤拷锟斤拷锟街撅拷锟侥硷拷录锟睫革拷锟斤拷司锟斤拷锟斤拷
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : none
//* 锟睫改碉拷全锟街憋拷锟斤拷      : none
//*----------------------------------------------------------------------------
void ModifyUnknownDriver()
{
	unsigned long dpp;
	unsigned short data;
	Record_CLOCK stoptime,PowerOffDT;
	int cmp1,cmp2;
	Record_CLOCK *temp;
	unsigned char i=0;
	
	PowerOffDT.year = curTime.year;
	PowerOffDT.month = curTime.month;
	PowerOffDT.day = curTime.day;
	PowerOffDT.hour = curTime.hour;
	PowerOffDT.minute = curTime.minute;
	PowerOffDT.second = curTime.second;
	
	//锟缴碉拷锟铰�
	dpp=pTable.DoubtPointData.CurPoint;
	do
	{
		if(dpp==pTable.DoubtPointData.BaseAddr)
			dpp = pTable.DoubtPointData.BaseAddr+96*210+2;
		else
			dpp = dpp-210+2;
		temp = (Record_CLOCK *)dpp;
		stoptime.type = temp->type;
		stoptime.year = temp->year;
		stoptime.month = temp->month;
		stoptime.day = temp->day;
		stoptime.hour = temp->hour;
		stoptime.minute = temp->minute;
		stoptime.second = temp->second;
//		stoptime=*temp;
		dpp = dpp - 2;
		cmp1=CompareDateTime(stoptime,PowerOnDT);
		cmp2=CompareDateTime(PowerOffDT,stoptime); 
		if((cmp1==1)&&(cmp2==1)&&(stoptime.type==0xffff))
		{
			data = (unsigned short)(RecordDriver.DriverCode);
			SPI_FLASH_BufferWrite( SPI1 ,(u8 *)&data, dpp, 2);
			data = (unsigned short)(RecordDriver.DriverCode>>16);
			SPI_FLASH_BufferWrite( SPI1 ,(u8 *)&data, dpp+2, 2);
		}
		i++;//2003.11.11,panhui(锟斤拷锟斤拷锟斤拷锟斤拷)
	}while((cmp1>0)&&(cmp2>=0)&&(i<97));
	
	//360小时平锟斤拷锟劫度硷拷录
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteBaseDataToFlash
//* Object              : 锟斤拷锟斤拷驶锟斤拷录写锟斤拷锟斤拷锟紽LASH
//* Input Parameters    : buf锟斤拷锟斤拷锟斤拷莼锟斤拷锟斤拷锟街革拷锟�
//                        len锟斤拷锟斤拷unsigned short锟斤拷锟斤拷莩锟斤拷锟�
//						  type锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
//* Output Parameters   : if data write TRUE or FALSE
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int WriteBaseDataToFlash(unsigned short *buf,unsigned char len,unsigned char type)
{

	unsigned char i;
	unsigned long pos,UpdatePos;
	
	pos = pTable.BaseData.CurPoint;
	
	for(i=0;i<len;i++)
	{
		SPI_FLASH_BufferWrite( SPI1 ,(u8 *)(&buf[i]), pos, 2);
		pos+=2;
		////////////add by panhui 2003.10.28////////
		if(pos>pTable.BaseData.EndAddr)
			pos = pTable.BaseData.BaseAddr;
		////////////////////////////////////////////
		UpdatePos = pos + RecordFlagByte*2;
		if(UpdatePos > pTable.BaseData.EndAddr)
			UpdatePos = pTable.BaseData.BaseAddr + (UpdatePos-pTable.BaseData.EndAddr)-1;

		if(((pos&0xff000)!=(UpdatePos&0xff000))&&((UpdatePos&0x00fff)==0))
		{
			if(!Update4k(UpdatePos,(unsigned char *)BaseData_4k,UpdateFlashAll))
				return(0);
		}
		
	}
	pTable.BaseData.CurPoint = pos;
	return(1);

}

void WritelocationData2Flash(CMD_LOCATION type)
{
	unsigned long p,i;
	p = pTable.LocationData.BakPoint;
	if(type == TIME)
	{
		erase4kflash(p,6);
		for(i= 0;i++;i<6)
		{
			Location_4k[i] = (unsigned char )*((unsigned char * )(&curTime+i));
		}
		SPI_FLASH_BufferWrite(SPI1,Location_4k , p, 6);
		pTable.LocationData.BakPoint =6 ;

	}
	else if(type == DATA)
	{
		erase4kflash(p,11);
		for(i = 0;i++;i<10)
		{
			Location_4k[i] = (unsigned char )*((unsigned char * )(&location+i));
		}
		Location_4k[i] = SpeedOf1min;
		SPI_FLASH_BufferWrite(SPI1,Location_4k , p, 6);
		pTable.LocationData.BakPoint = p+11;
	}

}

#if 0
void WriteOverDriverData2Flash()
{
	unsigned long p,i;
	p = pTable.OverSpeedRecord.CurPoint;

		erase4kflash(p,50);
		SPI_FLASH_BufferWrite(SPI1,(unsigned char *)overdriverdata , p, 50);

}

void WriteDriverRegData2Flash()
{
	unsigned long p,i;
	p = pTable.DriverReRecord.CurPoint;
	DataManager.c:822:4:
	erase4kflash(p,50);
	SPI_FLASH_BufferWrite(SPI1,(unsigned char *)driverregisterdata , p, 50);

}
#endif
void WriteRecordData2Flash(unsigned long p ,unsigned char *buff,unsigned long num)
{
	erase4kflash(p,num);
	SPI_FLASH_BufferWrite(SPI1,(unsigned char *)buff , p, 50);
}

//*----------------------------------------------------------------------------
//* Function Name       : BaseDataHandler
//* Object              : 锟斤拷锟斤拷细锟斤拷荽锟斤拷?锟斤拷锟铰�
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : PowerOnTime,STATUS1min,TimeChange,InRecordCycle,curTime
//* 锟睫改碉拷全锟街憋拷锟斤拷      : PowerOnTime,STATUS1min,InRecordCycle
//*----------------------------------------------------------------------------
void ValueStatusHandler()
{
	//驾驶状态变化处理
	if((CurSpeed >0)&&(Datastatusdata.keepstart10s == 1))
	{
		DriverStatus |= DRIVING_STAR;
		DriverStatus &= ~DRIVING_STAR;
		if(Datastatusdata.Over4hour)//并且同一个驾驶人
		{
			DriverStatus |= DRIVING_OVERTIME;
		}
		//Ifshijianchaoguo

	}
	else if ((CurSpeed == 0)&&(Datastatusdata.keepstop10s == 1))
	{
		DriverStatus |= DRIVING_STOP;
		DriverStatus &= ~DRIVING_STAR;
		if( Datastatusdata.Over20min == 1)//并且同一个驾驶人
		{
			DriverStatus |= DRIVING_STOP_DRIVER;
		}

	}
	//疑点数据判断处理状态变化
	if((Datastatusdata.locationchagestatus == 1)&&(DriverStatus & DRIVING_STAR ))//或者记录仪断电
	{
		DoubltPointstatus = DBRECORD_END;
	}
	else if((DriverStatus & DRIVING_STAR ))
	{
		DoubltPointstatus = DBRECORD_START;
	}
	else
	{
		DoubltPointstatus= NONE_DB;
	}
	//司机登记状态变化
	if((GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_4))&& (Datastatusdata.keeprestatus == 0))
	{
		Datastatusdata.keeprestatus = 1;
		DriverRegstatus = DRIVER_REG_IN;
	}
	else if((GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_4)== 0)&&Datastatusdata.keeprestatus == 0)
	{
		DriverRegstatus = DRIVER_REG_OUT;
		Datastatusdata.keeprestatus = 1;
	}

	//日志状态转换
	if(CurSpeed >40 )
	{
		if(Datastatusdata.keepthespeed == 0)
		{
			Datastatusdata.keepthespeed =1;
			JournalRecordstatus = RECORD_STARTTIME;
		}
		else if((Datastatusdata.keepthespeed == 1)&&(Datastatusdata.keepspeed5min))
		{
			if(speeddel <11)
				JournalRecordstatus = NORMAL;
			else
				JournalRecordstatus = ABORT;
		}
	}


}
void BaseDataHandler()
{
	int i,pt;
	if(DriverStatus & DRIVING_STAR )
	{

		STATUS1min = 0;//2003.10.23,panhui
		if((pTable.BaseData.CurPoint&1)!=0)
		{
			pTable.BaseData.CurPoint++;
			if(pTable.BaseData.CurPoint>pTable.BaseData.EndAddr)
				pTable.BaseData.CurPoint=pTable.BaseData.BaseAddr;
		}
		STATUS1min |= CurStatus;
		if(TimeChange & (0x01<<SECOND_CHANGE))//锟斤拷一锟斤拷锟斤拷
		{
			if (curTime.second == 0)
			{
				basedata.basedataclk.year = curTime.year;
				basedata.basedataclk.month = curTime.month;
				basedata.basedataclk.day = curTime.day;
				basedata.basedataclk.hour = curTime.hour;
				basedata.basedataclk.minute = curTime.minute;
				basedata.basedataclk.second = curTime.second;
				InRecordCycle |= 1<<BASEDATA;
				InRecordCycle &= ~(1<<BASEDATA);
			}
			else if(curTime.second == 0x59)
			{
				basedata.speed[curTime.second] = (unsigned char)CurSpeed;
				basedata.status[curTime.second] = (unsigned char)CurStatus;
				WriteBaseDataToFlash((unsigned short *)(&basedata),sizeof(BaseDataBlock),DATA_1min);
				#if RPM_EN
				data = (unsigned short)CurEngine;
				WriteBaseDataToFlash((unsigned short *)(&basedata),1,DATA_1min);
				#endif
			}
			basedata.speed[curTime.second] = (unsigned char)CurSpeed;
			basedata.status[curTime.second] = (unsigned char)CurStatus;
			STATUS1min = 0;
		}
	}
	else
	{
		if(( InRecordCycle&(1<<BASEDATA))==BASEDATA)
		{
			if(TimeChange & (0x01<<SECOND_CHANGE))
			{
				if (curTime.second != 0)
				{
					basedata.speed[curTime.second] = 0xff;
					basedata.status[curTime.second] = 0xff;
				}
				else
				{
					WriteBaseDataToFlash((unsigned short *)(&basedata),sizeof(BaseDataBlock),DATA_1min);
					InRecordCycle &= ~(1<<BASEDATA);
				}

			}

		}

	}
	
}

void LocationHandler()
{
	int i,pt;
	if((DriverStatus & DRIVING_STAR ))
	{

		if((pTable.BaseData.CurPoint&1)!=0)
		{
			pTable.BaseData.CurPoint++;
			if(pTable.BaseData.CurPoint>pTable.BaseData.EndAddr)
				pTable.BaseData.CurPoint=pTable.BaseData.BaseAddr;
		}

		if(TimeChange & (0x01<<MINUTE_CHANGE))//锟斤拷一锟斤拷锟斤拷
		{
			if (curTime.minute == 0)
			{
				WritelocationData2Flash(TIME);//write the time
				InRecordCycle |= 1<<LOCATIONDATA;
			}
			else if(curTime.minute == 0x59)
			{
				//update the location pt
				InRecordCycle &= ~(1<<LOCATIONDATA);
			}
			//jisuan sudu
			WritelocationData2Flash(DATA);//write the data

		}
	}
	else
	{
		if(( InRecordCycle&(1<<LOCATIONDATA))==LOCATIONDATA)
		{
			if(TimeChange & (0x01<<SECOND_CHANGE))
			{
				if (curTime.minute == 0)
				{
					WritelocationData2Flash(DATA);//write the data
					InRecordCycle &= ~(1<<LOCATIONDATA);
				}



			}

		}

	}

}
void OverDriverHandler()
{
	int i,pt;
	if((DriverStatus & DRIVING_OVERTIME ))
	{
		overdriverdata.startdrivertime.year =startdriverclk.year;
		overdriverdata.startdrivertime.month =startdriverclk.month;
		overdriverdata.startdrivertime.day =startdriverclk.day;
		overdriverdata.startdrivertime.hour =startdriverclk.hour;
		overdriverdata.startdrivertime.minute =startdriverclk.minute;
		overdriverdata.startdrivertime.second =startdriverclk.second;

	    if (DriverStatus & DRIVING_STOP_DRIVER )
		{
			overdriverdata.enddrivertime.year =enddriverclk.year;
			overdriverdata.enddrivertime.month =enddriverclk.month;
			overdriverdata.enddrivertime.day =enddriverclk.day;
			overdriverdata.enddrivertime.hour =enddriverclk.hour;
			overdriverdata.enddrivertime.minute =enddriverclk.minute;
			overdriverdata.enddrivertime.second =enddriverclk.second;

			if((pTable.BaseData.CurPoint&1)!=0)
			{
				pTable.BaseData.CurPoint++;
				if(pTable.BaseData.CurPoint>pTable.BaseData.EndAddr)
					pTable.BaseData.CurPoint=pTable.BaseData.BaseAddr;
			}
			WriteRecordData2Flash(pTable.OverSpeedRecord.CurPoint,(unsigned char *)&overdriverdata,50);//write the data
		}

	}
}

//*----------------------------------------------------------------------------
//* Function Name       : RunRecord360Handler
//* Object              :360小时平锟斤拷锟劫讹拷锟斤拷驶锟斤拷录锟斤拷统锟斤拷疲锟酵硷拷驶
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : CurSpeed,curTime
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
//0.2秒跑一遍
void DoubltPointHandler()
{
	unsigned char i,cnt;
	static unsigned DBcnt = 0;
	unsigned char *ptr ;
	switch(DoubltPointstatus)
	{
		case DBRECORD_START:
			ddb.data[DBcnt].speed = CurSpeed;
			ddb.data[DBcnt].status = CurStatus;
			DBcnt++;
			if(DBcnt == 100)
				DBcnt = 0;
			break;
		case DBRECORD_END:
			for(i= 0;i++;i<100)
			{
				cnt = DBcnt+i;
				if(cnt>99)
				{
					cnt = cnt-100;
				}
				ddb.data[i].speed= ddb.data[cnt].speed;
				ddb.data[i].status= ddb.data[cnt].status;
			}
			ptr = (unsigned char *)&ddb.StopTime;
			for(i = 0;i++;i<6)
			{
				ptr[i] = (unsigned char )*((unsigned char * )(&curTime+i));
			}
			ptr = (unsigned char *)&ddb.vaildlocation;
			for(i = 0;i++;i<10)
			{
				ptr[i]= (unsigned char )*((unsigned char * )&location+i);
			}
			WriteRecordData2Flash(pTable.DoubtPointData.CurPoint,(unsigned char *)&ddb,234);
		break;
		default:
			break;



}
void DrvierRegisterHandler()
{
	unsigned char i;
	unsigned char *ptr;
		if((DriverRegstatus & DRIVER_REG_IN)||(DriverRegstatus & DRIVER_REG_OUT))
		{
			ptr = (unsigned char *)&driverregisterdata.happentime;
			for(i=0;i++;i<18)
			{
				driverregisterdata.DriverLisenseCode[i] = CurrentDriver.DriverLisenseCode[i];
			}
			if(DriverRegstatus &DRIVER_REG_IN)
			{

				for(i=0;i++;i<6)
				{
					ptr[i] = (unsigned char )*((unsigned char * )(&startdriverclk+i));
				}
				driverregisterdata.registerstatus = 0x01;
			}
			else
			{
				for(i=0;i++;i<6)
				{
					ptr[i] = (unsigned char )*((unsigned char * )&enddriverclk+i);
				}
				driverregisterdata.registerstatus = 0x02;

			}
			WriteRecordData2Flash(pTable.DriverReRecord.CurPoint,(unsigned char *)&driverregisterdata,25);
		}
}

void PowerHandle()
{
	unsigned char i;
	unsigned char *ptr = (unsigned char *)&powerdata.powertime;

	if((powerstatus & POWER_ON)||(DriverRegstatus & POWER_OFF))
	{
		for(i=0;i++;i<6)
		{
			ptr[i] = (unsigned char )*((unsigned char * )(&curTime+i));
		}
		powerdata.powerstatus = paramodifystatus;
		WriteRecordData2Flash(pTable.PowerOffRunRecord.CurPoint,(unsigned char *)&modifydata,7);
	}

}

void ModifyHandle()
{
	unsigned char i;
	unsigned char *ptr = (unsigned char *)&modifydata.modifytime;
	if(paramodifystatus)
	{
		for(i=0;i++;i<6)
		{
			ptr[i] = (unsigned char )*((unsigned char * )(&curTime+i));
		}
		modifydata.modifystatus = powerstatus;
		WriteRecordData2Flash(pTable.ModifyRecord.CurPoint,(unsigned char *)&modifydata,7);
	}

}

//此函数一秒钟执行一次
void JournalHandle()
{
	unsigned char i;
	static unsigned JNcnt;
	unsigned char *ptr;
	switch(JournalRecordstatus)
	{
		case NORMAL://记录正常数据
		case ABORT://记录异常数据
			journaldata.speedstatus = JournalRecordstatus;
			JournalRecordstatus = NONE_OPR;
			ptr = (unsigned char *)&journaldata.jouranlendtime;
			for(i = 0;i++;i<6)
			{
				ptr[i]= (unsigned char )*((unsigned char * )(&curTime+i));
			}
			WriteRecordData2Flash(pTable.journalRecord.CurPoint,(unsigned char *)&journaldata,133);
		break;
		case RECORD_STARTTIME://满足速度>40,并且异常与正常交替
			ptr = (unsigned char *)&journaldata.journalstartime;
			for(i = 0;i++;i<6)
			{
				ptr[i] = (unsigned char )*((unsigned char * )(&curTime+i));
			}
			journaldata.rspeed[0] = RsSpeed;
			journaldata.speed[0]  = CurSpeed;
			JNcnt = 1;
			JournalRecordstatus = RECORD_DATA;
			break;
		case RECORD_DATA://满足速度>40,并且误差率限数值保持在一个水平
			journaldata.rspeed[JNcnt] = RsSpeed;
			journaldata.speed[JNcnt]  = CurSpeed;
			if(JNcnt== 59)
			{
				JNcnt = 0;
				JournalRecordstatus = NONE_OPR;
			}
			break;
		default:
			break;
	}

}
//*----------------------------------------------------------------------------
//* Function Name       : FinishAllRecord
//* Object              : 锟斤拷锟斤拷锟斤拷锟叫硷拷录
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : PowerOnTime,STATUS1min,TimeChange,InRecordCycle,curTime
//* 锟睫改碉拷全锟街憋拷锟斤拷      : PowerOnTime,STATUS1min,InRecordCycle
//*----------------------------------------------------------------------------
void FinishAllRecord()
{
	if(FinishFlag)
	{
		//锟斤拷驶锟斤拷录锟斤拷锟斤拷锟叫达拷锟絛ataflash
		WriteBaseDataToFlash((unsigned short *)(&rec_end),(sizeof(RecordData_end))/2,END);
		RecordDriver = CurrentDriver;//锟街革拷锟斤拷录司锟斤拷
		FinishFlag = 0;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : GetOTDRDataFromFlash
//* Object              : 锟接革拷牡锟街凤拷锟饺nc锟斤拷锟饺碉拷疲锟酵硷拷驶锟斤拷锟�
//*                       锟斤拷锟斤拷诺锟紹UF锟斤拷
//* Input Parameters    : p锟斤拷锟斤拷锟斤拷牡锟街�
//*                       inc锟斤拷锟斤拷锟斤拷莩锟斤拷龋锟斤拷锟捷凤拷锟饺凤拷锟斤拷锟斤拷锟捷的凤拷锟斤拷
//*                       注锟解：inc锟斤拷锟斤拷为锟斤拷锟斤拷锟脚硷拷锟�
//*                       buf锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
void GetOTDRDataFromFlash(unsigned short *p, int inc,unsigned char *buf)
{
	int  i;
	unsigned short data;

	if(inc==0)
		return;
#if 0
	if(inc<0){
		StructPT temp;
		temp = pTable.RunRecord360h;
		temp.CurPoint = (unsigned long)p;
		p = (unsigned short *)AddPointer(&temp, inc);
		inc=0-inc;
	}

	//锟斤拷锟街革拷锟轿硷拷锟�
	if(((unsigned long)p&1)==0)
	{
		for(i=0;i<inc/2;i++){
			buf[2*i] = (unsigned char)(*p);
			buf[2*i+1] = (unsigned char)((*p)>>8);
			p++;
			if((unsigned long)p > pTable.RunRecord360h.EndAddr)
				p = (unsigned short *)pTable.RunRecord360h.BaseAddr;
		}
		//锟斤拷锟絠nc为锟斤拷锟斤拷
		if((inc&1)==1)
		{
//			p=(unsigned short *)((unsigned long)p-1);
			data = *p;
			buf[inc-1] = (unsigned char)data;
			if((unsigned long)p > pTable.RunRecord360h.EndAddr)
				p = (unsigned short *)pTable.RunRecord360h.BaseAddr;
		}
	}
	//锟斤拷锟街革拷锟轿拷锟斤拷锟�
	else{
		//锟斤拷锟絠nc为偶锟斤拷
		if((inc&1)==0)
		{
			p=(unsigned short *)((unsigned long)p-1);
			data = *p;
			buf[0] = (unsigned char)(data>>8);
			p++;
			for(i=0;i<inc/2;i++)
			{
				data = *p;
				buf[i*2+1]=data;
				if(i!=(inc/2-1))
					buf[i*2+2]=(unsigned char)(data>>8);
				p++;
				if((unsigned long)p > pTable.RunRecord360h.EndAddr)
					p = (unsigned short *)pTable.RunRecord360h.BaseAddr;
			}
		}
		//锟斤拷锟絠nc为锟斤拷锟斤拷
		if((inc&1)==1)
		{
			p=(unsigned short *)((unsigned long)p-1);
			data = *p;
			buf[0] = (unsigned char)(data>>8);
			p++;
			for(i=0;i<inc/2;i++)
			{
				data = *p;
				buf[i*2+1]=data;
				buf[i*2+2]=(unsigned char)(data>>8);
				p++;
				if((unsigned long)p > pTable.RunRecord360h.EndAddr)
					p = (unsigned short *)pTable.RunRecord360h.BaseAddr;
			}		
		}
	}
#endif
}
//*----------------------------------------------------------------------------
//* Function Name       : IsCorrectCLOCK
//* Object              : 锟叫讹拷时锟斤拷锟斤拷锟斤拷欠锟斤拷锟饺�
//* Input Parameters    : 时锟斤拷指锟斤拷
//* Output Parameters   : 锟角凤拷锟斤拷一锟斤拷锟斤拷确锟斤拷时锟斤拷锟斤拷锟�
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int IsCorrectCLOCK(CLOCK *dt)
{
	unsigned char data;
	data = BCD2Char(dt->year);
	if(data>99)
		return(0);
	data = BCD2Char(dt->month);
	if((data>12)||(data<=0))
		return(0);
	data = BCD2Char(dt->day);
	if((data>31)||(data<=0))
		return(0);
	data = BCD2Char(dt->hour);
	if(data>23)
		return(0);
	data = BCD2Char(dt->minute);
	if(data>59)
		return(0);
	data = BCD2Char(dt->second);
	if(data>59)
		return(0);
		
	return (1);
}
//*----------------------------------------------------------------------------
//* Function Name       : IsCorrectClock
//* Object              : 锟叫讹拷时锟斤拷锟斤拷锟斤拷欠锟斤拷锟饺�
//* Input Parameters    : 时锟斤拷指锟斤拷
//* Output Parameters   : 锟角凤拷锟斤拷一锟斤拷锟斤拷确锟斤拷时锟斤拷锟斤拷锟�
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int IsCorrectClock(Record_CLOCK *dt)
{
	unsigned char data;
	data = BCD2Char(dt->year);
	if(data>99)
		return(0);
	data = BCD2Char(dt->month);
	if((data>12)||(data<=0))
		return(0);
	data = BCD2Char(dt->day);
	if((data>31)||(data<=0))
		return(0);
	data = BCD2Char(dt->hour);
	if(data>23)
		return(0);
	data = BCD2Char(dt->minute);
	if(data>59)
		return(0);
//	data = BCD2Char(dt->second);
//	if(data>59)
//		return(0);
		
	return (1);
}
//*----------------------------------------------------------------------------
//* Function Name       : GetOTDR
//* Object              : 锟斤拷取锟斤拷前锟斤拷疲锟酵硷拷驶锟斤拷录
//* Input Parameters    : p锟斤拷锟斤拷锟斤拷前指锟斤拷;
//*                       s锟斤拷锟斤拷疲锟酵硷拷驶锟斤拷录锟斤拷始锟斤拷锟�
//*                       e锟斤拷锟斤拷疲锟酵硷拷驶锟斤拷录锟斤拷锟斤拷锟斤拷锟�
//* Output Parameters   : 锟角凤拷锟斤拷一锟斤拷疲锟酵硷拷驶锟斤拷录
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      :
//* 锟睫改碉拷全锟街憋拷锟斤拷      :
//*----------------------------------------------------------------------------
int GetOTDR( unsigned long p, OTDR_start *s, OTDR_end *e )
{
	int offset;
	offset = 0 - sizeof(OTDR_end);
	GetOTDRDataFromFlash((unsigned short *)p, offset,(unsigned char *)e);
	if(e->dt.type!=0xeaea)
		return (0);
	if(!IsCorrectClock(&(e->dt)))
		return (0);
	
	StructPT temp;
	temp.CurPoint = (unsigned long)p;
	offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+e->MinuteNb);
	p = AddPointer(&temp, offset);
	offset = sizeof(OTDR_start);
	GetOTDRDataFromFlash((unsigned short *)p, offset,(unsigned char *)s);
	if(s->dt.type!=0xafaf)
		return (0);
	if(!IsCorrectClock(&(s->dt)))
		return (0);

	return(1);
	
}
//*----------------------------------------------------------------------------
//* Function Name       : ComputeDistance100m
//* Object              : 锟斤拷锟斤拷锟斤拷锟斤拷锟酵筹拷锟斤拷系锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷位为锟斤拷锟斤拷
//* Input Parameters    : pulseNb锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
//* Output Parameters   : 锟斤拷锟斤拷锟斤拷0-5804009锟斤拷
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : none
//* 锟睫改碉拷全锟街憋拷锟斤拷      : none
//*----------------------------------------------------------------------------
unsigned long ComputeDistance100m(unsigned long pulseNb)
{
	unsigned long result;
	unsigned char PP =8;
	result = (pulseNb/(PP*Parameter.PulseCoff/10));
	return result;
}
//*----------------------------------------------------------------------------
//* Function Name       : CompareDateTime
//* Object              : 锟饺斤拷锟斤拷锟斤拷时锟斤拷拇锟叫�
//* Input Parameters    : dt1锟斤拷锟斤拷时锟斤拷1锟斤拷dt2锟斤拷锟斤拷时锟斤拷2
//* Output Parameters   : 1 锟斤拷锟斤拷 dt1 > dt2;
//                       -1 锟斤拷锟斤拷 dt1 < dt2;
//                        0 锟斤拷锟斤拷 dt1 = dt2;
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : none
//* 锟睫改碉拷全锟街憋拷锟斤拷      : none
//*----------------------------------------------------------------------------
int CompareDateTime(Record_CLOCK dt1,Record_CLOCK dt2)
{
	int res1,res2;
	res1 = IsCorrectClock(&dt1);
	res2 = IsCorrectClock(&dt2);
	if((res1==0)&&(res2==0))
		return 0;
	else if((res1==1)&&(res2==0))
		return 1;
	else if((res1==0)&&(res2==1))
		return -1;

	if(dt1.year > dt2.year)
		return 1;
	if(dt1.year < dt2.year)
		return -1;
		
	//dt1.yea == dt2.year	
	if(dt1.month >  dt2.month)
		return 1;
	if(dt1.month < dt2.month)
		return -1;
	
	//dt1.month == dt2.month
	if(dt1.day > dt2.day)
		return 1;
	if(dt1.day < dt2.day)
		return -1;
	
	//dt1.day == dt2.day
	if(dt1.hour > dt2.hour)
		return 1;
	if(dt1.hour < dt2.hour)
		return -1;

	//dt1.hour == dt2.hour
	if(dt1.minute > dt2.minute)
		return 1;
	if(dt1.minute < dt2.minute)
		return -1;

	//dt1.minute == dt2.minute
	if(dt1.second > dt2.second)
		return 1;
	if(dt1.second < dt2.second)
		return -1;
		
	//dt1.second == dt2.second
	return 0;
}
//*----------------------------------------------------------------------------
//* Function Name       : DataPointerSeek
//* Object              : 锟斤拷锟街革拷攵ㄎ�
//* Input Parameters    : none
//* Output Parameters   : none
//* 锟斤拷锟矫碉拷全锟街憋拷锟斤拷      : none
//* 锟睫改碉拷全锟街憋拷锟斤拷      : none
//*----------------------------------------------------------------------------
void DataPointerSeek()
{
	unsigned long DataPoint;
	unsigned short data;
	unsigned char *p;
	unsigned char temp,f1,f2;
	unsigned char update = 0;
	int i;
	DataPoint = pTable.BaseData.CurPoint;
	p = (unsigned char *)DataPoint;
	//锟斤拷锟揭碉拷前指锟斤拷位锟斤拷锟角凤拷锟斤拷诒锟街�
	for(i=0;i<RecordFlagByte;i++)
	{
		if(*p!=0xff)
			break;
		if((unsigned long)p==pTable.BaseData.EndAddr)
			p=(unsigned char *)pTable.BaseData.BaseAddr;
		else
			p++;
	}
	if(i==RecordFlagByte)//锟斤拷志锟斤拷锟节ｏ拷锟斤拷锟斤拷指锟诫定位锟斤拷
		return;
		
	//锟斤拷锟铰讹拷位锟斤拷锟斤拷莸锟街革拷锟�
	do 
	{
		if((*p == 0xff)&&(((unsigned long)p&1)==0))
		{//锟斤拷锟斤拷锟揭碉拷锟斤拷志
			i=0;
			do
			{
				if((unsigned long)p == pTable.BaseData.EndAddr)
					p = (unsigned char *)pTable.BaseData.BaseAddr;
				else
					p++;
				i++;
			}while((*p==0xff)&&(i<RecordFlagByte));
			if(i == RecordFlagByte)//锟揭碉拷锟斤拷锟斤拷指锟斤拷位锟斤拷
			{
				//锟介看锟斤拷志之前锟角凤拷锟叫斤拷锟斤拷锟街続EAE
				//1)锟叫讹拷锟角凤拷锟斤拷锟斤拷丫锟叫达拷锟絘eae锟斤拷志锟侥凤拷锟斤拷系锟斤拷录
				DataPoint = (unsigned long)p;
				DataPoint -= (RecordFlagByte+2);
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				f1 = *((unsigned char *)DataPoint);
				f2 = *((unsigned char *)(DataPoint+1));
				if((f1==0xae)&&(f2==0xae))
				{//锟斤拷锟斤拷系锟斤拷录锟斤拷锟斤拷指锟斤拷锟斤拷锟�
					pTable.BaseData.CurPoint = (unsigned long)p;
					pTable.BaseData.CurPoint = AddPointer(&(pTable.BaseData),-2);
					update = 1;
					break;
				}
				
				//2)锟叫讹拷锟角凤拷锟斤拷锟斤拷锟较碉拷锟铰�
				DataPoint = (unsigned long)p;
				DataPoint -= RecordFlagByte*2;
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				f1 = *((unsigned char *)DataPoint);
				f2 = *((unsigned char *)(DataPoint+1));
					
				//锟斤拷锟斤拷锟街撅拷锟绞嘉伙拷锟�
				/////////*******2003.10.06 panhui*********////////
				DataPoint = (unsigned long)p;
				DataPoint -= RecordFlagByte;
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				if((f1==0xae)&&(f2==0xae))//锟斤拷锟铰硷拷锟斤拷锟捷ｏ拷锟斤拷指锟斤拷锟斤拷锟�
					pTable.BaseData.CurPoint = DataPoint;
				else
				{//锟斤拷锟斤拷系锟�未锟斤拷锟矫硷拷写锟斤拷锟斤拷锟街続EAE
					//写锟斤拷萁锟斤拷锟斤拷志
					data = 0xaeae;
					SPI_FLASH_BufferWrite(SPI1,(u8 *)(&data),(unsigned long )DataPoint,2);
					//锟斤拷锟铰凤拷锟斤拷锟�
					pTable.BaseData.CurPoint = (unsigned long)p;
				}
				/////////*******2003.10.06 panhui*********////////
				update = 1;
				break;
			}
		}
		else
		{//未锟揭碉拷锟斤拷志锟斤拷锟斤拷锟斤拷锟狡讹拷指锟斤拷
			if((unsigned long)p == pTable.BaseData.EndAddr)
				p = (unsigned char *)pTable.BaseData.BaseAddr;
			else
				p++;
		}
	}while((unsigned long)p != DataPoint);
	
	if(update)
	{
		//锟斤拷锟铰讹拷位锟缴碉拷锟斤拷莸锟街革拷锟�
		Record_CLOCK LastDT,CurDT;
		unsigned char *dp;
		DataPoint = pTable.DoubtPointData.CurPoint;
		int result;
		
		if(DataPoint == pTable.DoubtPointData.BaseAddr)
			dp = (unsigned char *)(DataPoint+96*210+4);
		else
			dp = (unsigned char *)(DataPoint-210+4);
		LastDT.year = *dp;dp++;
		LastDT.month = *dp;dp++;
		LastDT.day = *dp;dp++;
		LastDT.hour = *dp;dp++;
		LastDT.minute = *dp;dp++;
		LastDT.second = *dp;
		do{
		
			dp = (unsigned char *)(DataPoint+4);
			CurDT.year = *dp;dp++;
			CurDT.month = *dp;dp++;
			CurDT.day = *dp;dp++;
			CurDT.hour = *dp;dp++;
			CurDT.minute = *dp;dp++;
			CurDT.second = *dp;dp++;
	
			result = CompareDateTime( CurDT, LastDT);
			if(result!=1)
				break;
			DataPoint += 210;
			//////////modified by panhui 2003.10.20////////////
			if(DataPoint > (pTable.DoubtPointData.EndAddr-110))
				DataPoint = pTable.DoubtPointData.BaseAddr;
			//////////锟截皇碉拷3台锟斤拷//////////////////////////////
			LastDT = CurDT;
				
		}while(DataPoint != pTable.DoubtPointData.CurPoint);	
		
		pTable.DoubtPointData.CurPoint = DataPoint;
		
		WritePartitionTable(&pTable);
	}
}
}




