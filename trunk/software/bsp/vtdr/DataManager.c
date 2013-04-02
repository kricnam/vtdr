//*----------------------------------------------------------------------------
//*      数据处理子程序
//*----------------------------------------------------------------------------
//* File Name           : DataManager.c
//* Object              : 记录仪采集的速度状态数据的处理和保存
//*
//* 1.0 24/02/03 PANHUI : Creation
//*----------------------------------------------------------------------------
//
#include	"DataManager.h"
#include	"ibb3.h"
#include	"flash_sst39.h"
#include    "lcd.h" 

extern u_int CurSpeed;
extern int DeltaSpeed;
extern CLOCK curTime;
extern u_char CurStatus;
extern u_int LastSPE;
extern u_char TimeChange;	//时间变化标志
extern u_int AddupSpeed;
extern u_short SpeedNb;
extern PartitionTable pTable;
extern StructPara Parameter;
extern u_int PulseTotalNumber;	/*本次行驶总脉冲数*/
extern u_short STATUS;			/*16种状态*/
extern u_int PulseNB_In1Sec;     //每0.2秒周期前推1秒内累计速度脉冲数
extern u_char PowerOn;
extern u_int CurEngine;
extern u_short DriveMinuteLimit;       //疲劳驾驶行驶时间门限
extern u_short RestMinuteLimit;        //疲劳驾驶最少休息时间门限
extern u_int CurPulse; 
 
DoubtDataBlock ddb;			//当前疑点数据块
u_char InRecordCycle=0;		//是否在记录数据过程中
u_char InFlashWriting=0;	//在FLASH写周期中
u_int Tick02NB;				//两次停车之间0.2s的个数
OTDR_end otdrEND;			//疲劳驾驶记录结束点数据
u_int AddupSpeed = 0;		//1分钟速度累计
u_short SpeedNb = 0;		//1分钟速度值个数
u_char PowerOnTime=0;		//上电持续时间
u_char OTRecordType=0;		//定义疲劳驾驶记录类型
u_int LastDistance;			//上次疲劳驾驶记录累计里程
u_char STATUS1min;			//1秒钟状态或
DRIVER CurrentDriver;		//当前司机
DRIVER RecordDriver;		//记录司机
Record_CLOCK PowerOnDT;     //上电日期时间
RecordData_end rec_end;
u_char FinishFlag=0;

extern u_char LargeDataBuffer[];
u_short *DoubtData_4k = (u_short *)(&(LargeDataBuffer[12*1024]));//[2*1024];
u_char *OTDR_4k = &(LargeDataBuffer[16*1024]);//[4*1024];
u_short *BaseData_4k = (u_short *)(&(LargeDataBuffer[20*1024]));//[2*1024];

//*----------------------------------------------------------------------------
//* Function Name       : Task_GetData
//* Object              : 自检程序
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
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
	lcm_clear_ram(ALL);	
}

//*----------------------------------------------------------------------------
//* Function Name       : StrCmp
//* Object              : 比较两个字符串是否相同
//* Input Parameters    : 待比较的两个字符串str1,str2,长度length
//* Output Parameters   : 1——字符串相同；0——字符串不同
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
char StrCmp(u_char *str1, u_char *str2, short length)
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
//* Function Name       : WriteParameterTable
//* Object              : 写参数表
//* Input Parameters    : none
//* Output Parameters   : 是否成功
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WriteParameterTable(StructPara *para)
{
	flash_word *data;
	flash_word *p;
	int i,size;

	//擦除4k
	p = (flash_word *)PARAMETER_BASE;
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,p))
		return(FALSE);
	
	
	//将参数表写入FLASH
	data = (flash_word *)(para);
	size=sizeof(StructPara);

	for(i=0;i<size;i+=2)
	{
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,p,*data))
			return ( FALSE ) ;
		p++;
		data++;
	}
	
	return ( TRUE ) ;
}
//*----------------------------------------------------------------------------
//* Function Name       : WritePartitionTable
//* Object              : 写分区表
//* Input Parameters    : none
//* Output Parameters   : 是否成功
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WritePartitionTable(PartitionTable *ptt)
{
	flash_word *data;
	flash_word *p;
	int i,size;

	//擦除4k
	p = (flash_word *)PartitionTable_BASE;
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,p))
		return(FALSE);
	
	//将分区表写入FLASH
	if(ptt->TotalDistance==0xffffffff)
		ptt->TotalDistance = 0;
	data = (flash_word *)(ptt);
	size=sizeof(PartitionTable);
	
	for(i=0;i<size;i+=2)
	{
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,p,*data))
			return ( FALSE ) ;
		p++;
		data++;
	}
		
	return ( TRUE ) ;
}
//*----------------------------------------------------------------------------
//* Function Name       : IsPartitionTableCorrect
//* Object              : 初始化分区表
//* Input Parameters    : none
//* Output Parameters   : 写分区表是否成功
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int IsPartitionTableCorrect()
{
	flash_word *data;
	flash_word *p;
	int i,size;
	
	pTable = *PartitionTable_BASE;
	/////////*******2003.10.06 panhui*********////////
	u_short temp = PartitionTableFlag;
	if( pTable.Available != temp)
		return(0);
	/////////*******2003.10.06 panhui*********////////
	if(	pTable.DoubtPointData.BaseAddr != DPD_BASE)
		return(0);
	if(	pTable.DoubtPointData.EndAddr != DPD_END)
		return(0);
	if(	pTable.RunRecord360h.BaseAddr != RR360H_BASE)
		return(0);
	if(	pTable.RunRecord360h.EndAddr != RR360H_END)
		return(0);
	if(	pTable.BaseData.BaseAddr != BASEDATA_BASE)
		return(0);
	if(	pTable.BaseData.EndAddr != BASEDATA_END)
		return(0);

	/////////////modified by panhui 2003.10.20////////////
	if((pTable.DoubtPointData.CurPoint < DPD_BASE)||(pTable.DoubtPointData.CurPoint > DPD_END-110))
		return(-1);
	if((pTable.RunRecord360h.CurPoint < RR360H_BASE)||(pTable.RunRecord360h.CurPoint > RR360H_END))
		return(-2);
	if((pTable.BaseData.CurPoint < BASEDATA_BASE)||(pTable.BaseData.CurPoint > BASEDATA_END))
		return(-3);
	/////////////防止指针跑非/////////////////////////////

	//将参数表写入FLASH
	return (1);
	

}
//*----------------------------------------------------------------------------
//* Function Name       : InitializeTable
//* Object              : 初始化分区表
//* Input Parameters    : u_char parti——是否重新恢复参数表
//                        u_char para——是否重新恢复分区表
//                        u_char change_set——是否重新设置车牌号，
//                                             是则累计总里程清零
//* Output Parameters   : 初始化分区表和参数表是否成功
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int InitializeTable(u_char parti,u_char para,u_char change_set)
{
	int i;

	//读取参数表	
	Parameter = *PARAMETER_BASE;
	if(para)
	{
		Parameter.mark=0x30aa;//*特征字——2
		Parameter.IBBType=0x3000;//记录仪代码——2
		if( !WriteParameterTable(&Parameter) )
			return (0);
	}
	
	pTable = *PartitionTable_BASE;
	if(parti == 1){
		pTable.Available = PartitionTableFlag;
		pTable.DoubtPointData.BaseAddr = DPD_BASE;
		pTable.DoubtPointData.EndAddr = DPD_END;
		pTable.DoubtPointData.CurPoint = DPD_BASE;
		pTable.RunRecord360h.BaseAddr = RR360H_BASE;
		pTable.RunRecord360h.EndAddr = RR360H_END;
		pTable.RunRecord360h.CurPoint = RR360H_BASE;
		pTable.BaseData.BaseAddr = BASEDATA_BASE;
		pTable.BaseData.EndAddr = BASEDATA_END;
		pTable.BaseData.CurPoint = BASEDATA_BASE;
		if(change_set)
		{
			pTable.TotalDistance = 0;
			pTable.DriverCode = 0;//驾驶员代码
			for( i = 0; i < 20; i++)
				pTable.DriverLisenseCode[i] = 0;//驾驶证号码
			pTable.InOSAlarmCycle = 0;//“在分路段报警周期中”标志
			pTable.OSAlarmAddupDistance = 0;//分路段报警路程累计
		}
		if( !WritePartitionTable(&pTable) )
			return(0);
	}
	else if(parti == 2)
	{
		/////////////modified by panhui 2003.10.20////////////
		if((pTable.DoubtPointData.CurPoint < DPD_BASE)||(pTable.DoubtPointData.CurPoint > DPD_END-110))
			pTable.DoubtPointData.CurPoint = DPD_BASE;
		if((pTable.RunRecord360h.CurPoint < RR360H_BASE)||(pTable.RunRecord360h.CurPoint > RR360H_END))
			pTable.RunRecord360h.CurPoint = RR360H_BASE;
		if((pTable.BaseData.CurPoint < BASEDATA_BASE)||(pTable.BaseData.CurPoint > BASEDATA_END))
			pTable.BaseData.CurPoint = BASEDATA_BASE;
		/////////////防止指针跑非/////////////////////////////
		if(change_set)
		{
			pTable.TotalDistance = 0;
			pTable.DriverCode = 0;//驾驶员代码
			for( i = 0; i < 20; i++)
				pTable.DriverLisenseCode[i] = 0;//驾驶证号码
			pTable.InOSAlarmCycle = 0;//“在分路段报警周期中”标志
			pTable.OSAlarmAddupDistance = 0;//分路段报警路程累计
		}
		if( !WritePartitionTable(&pTable) )
			return(0);
	
	}
	
	return (1);
}
//*----------------------------------------------------------------------------
//* Function Name       : UpdateParameterPartition
//* Object              : 更新参数表区域
//* Input Parameters    : none
//* Output Parameters   : 新参数表区域是否成功
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int UpdateParameterPartition()
{
	u_int sector_addr;
	sector_addr = 0;
	flash_word *data;
	flash_word *p;
	int i,size;
	
	//擦除FLASH
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr))
		return(0);
	//刷新参数表
	p = (flash_word *)PARAMETER_BASE;
	data = (flash_word *)(&Parameter);
	size=sizeof(StructPara);
	
	for(i=0;i<size;i+=2)
	{
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,p,*data))
			return ( 0 ) ;
		p++;
		data++;
	}
	
	//刷新分区表
	p = (flash_word *)PartitionTable_BASE;
	data = (flash_word *)(&pTable);
	size=sizeof(PartitionTable);
	
	for(i=0;i<size;i+=2)
	{
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,p,*data))
			return ( 0 ) ;
		p++;
		data++;
	}
	return(1);
}
//*----------------------------------------------------------------------------
//* Function Name       : Update4k
//* Object              : FLASH中4K读入内存，擦除当前4k，并根据类型确定是否
//                        将指针之前的数据写回FLASH
//* Input Parameters    : p——当前数据指针
//*                       Buffer——内存中待更新的4k数据区首地址
//*                       type——类型
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int Update4k(u_short *p,u_short *Buffer,u_char type)
{
	u_short i;
	flash_word *sector_addr;
	sector_addr = (flash_word *)((u_int)p & 0xfffff000);
	//4k读入内存
	for(i=0;i<2*1024;i++)
		Buffer[i] = sector_addr[i];

	if(type == UpdateFlashOnce)
		return(1); 
		
	//擦除FLASH
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,sector_addr))
		return(0);
	//指针之前的数据重新写回FLASH
	if(type == UpdateFlashTimes)
	{
		i=0;
		while(sector_addr<p)
		{
			if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,sector_addr,Buffer[i]))
				return ( FALSE ) ;
			sector_addr++;
			i++;
		}
	}
	return(1);
}		
//*----------------------------------------------------------------------------
//* Function Name       : WriteDoubtDataToFlash
//* Object              : 写一个疑点数据到FLASH中
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : curTime
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WriteDoubtDataToFlash()
{
	u_short i; 
	u_short *p4k;
	p4k = (u_short *)(pTable.DoubtPointData.CurPoint);
	if((InFlashWriting&(1<<DOUBTPOINTDATA))==0)
	{//如果没有开始FLASH写，首先读入当前的疑点4K数据常驻内存
		InFlashWriting |= 1<<DOUBTPOINTDATA;//标记疑点数据为“在写FLASH”
		if(!Update4k(p4k,DoubtData_4k,UpdateFlashTimes))
			return (0);
	}
	
	//写当前的停车疑点到FLASH中
	p4k = (u_short *)((u_int)p4k&0xff000);
	if(RecordDriver.DriverCode != 0) 
		ddb.DriverCode = RecordDriver.DriverCode;
	else
		ddb.DriverCode = 0xffffffff;
	ddb.StopTime = curTime;
	u_short *p,*data;
	p = (u_short *)(pTable.DoubtPointData.CurPoint);
	/////////////2003.10.20 modified by panhui/////////
	if( (u_int)p > (pTable.DoubtPointData.EndAddr-110))
	{
		pTable.DoubtPointData.CurPoint = pTable.DoubtPointData.BaseAddr;
		p=(u_short *)(pTable.DoubtPointData.BaseAddr);
	}
	///////////////////////////////////////////////////
	data = (u_short *)(&ddb);
	for(i=0;i<10;i+=2)
	{
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,p,*data))
			return ( FALSE ) ;
		p++;
		data++;
		
		//查看指针是否要转换到下一个4k数据区
		if(p4k != (u_short *)((u_int)p&0xff000))
		{
			if(!Update4k(p,DoubtData_4k,UpdateFlashTimes))
				return (0);
			p4k = (u_short *)((u_int)p&0xff000);
		}
	}
	u_short vs;
	u_char j;
	for(i=0;i<100;i++)
	{
		j = ddb.pt + i;
		if(j>=100)
			j=j-100;
		vs =((ddb.data[j].status)<<8) + ddb.data[j].speed;
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,p,vs))
			return ( FALSE ) ;
		p++;
		
		//查看指针是否要转换到下一个4k数据区
		if(p4k != (u_short *)((u_int)p&0xff000))
		{
			if(!Update4k(p,DoubtData_4k, UpdateFlashTimes))
				return (0);
			p4k = (u_short *)((u_int)p&0xff000);
		}
	}
	
	//更新疑点数据指针
	if((u_int)p > pTable.DoubtPointData.EndAddr-110)
	{
		pTable.DoubtPointData.CurPoint = pTable.DoubtPointData.BaseAddr;
		p=(u_short *)(pTable.DoubtPointData.BaseAddr);
		if(!Update4k(p, DoubtData_4k, UpdateFlashTimes))
			return (0);
	}
	else
		pTable.DoubtPointData.CurPoint = (u_int)p;
		
	return(1);
}
//*----------------------------------------------------------------------------
//* Function Name       : BCD2Char
//* Object              : BCD码转换为十进制数
//* Input Parameters    : bcd——待写转换的BCD码
//* Output Parameters   : 转换后的十进制数
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
u_char BCD2Char(u_char bcd)
{
	u_char ch;
	u_char d1,d0;
	d1=((bcd & 0xf0)>>4);
	if(d1>9)
		return (0xff);
	d0=bcd & 0x0f;
	if(d0>9)
		return (0xff);
	ch = d1*10 + d0;
	return(ch);
}
//*----------------------------------------------------------------------------
//* Function Name       : IfOneAfterAotherDay
//* Object              : 判断是否是连续的两天
//* Input Parameters    : time——相对新的时间指针
//*                       end——上一个疲劳记录结束点数据指针
//* Output Parameters   : 0——不是；1——是
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int IfOneAfterAotherDay(CLOCK *time,OTDR_end *end)
{
	u_char day,month,year;
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
//* Object              : 判断时间间隔
//* Input Parameters    : time——相对新的时间指针
//*                       end——上一个疲劳记录结束点数据指针
//* Output Parameters   : 时间间隔值（分钟），当大于20分钟时，给FF
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
u_char JudgeTimeSpace(CLOCK *time,OTDR_end *end)
{
	int space,conti;
	u_char ret;
	if((time->year==end->dt.year)&&(time->month==end->dt.month)&&(time->day==end->dt.day))
	{
		space = BCD2Char(time->hour)*60 + BCD2Char(time->minute) - BCD2Char(end->dt.hour)*60 - BCD2Char(end->dt.minute);
		if((space<0)||(space>20))
			ret=0xff;
		else
			ret=(u_char)space;
			
	}
	else
	{
		conti = IfOneAfterAotherDay(time,end);
		if(conti){
			space = 24*60 + BCD2Char(time->hour)*60 + BCD2Char(time->minute) - BCD2Char(end->dt.hour)*60 - BCD2Char(end->dt.minute);
			if((space<0)||(space>20))
				ret=0xff;
			else
				ret=(u_char)space;
		}
		else
			ret = 0xff;
	}
	return ret;
}
//*----------------------------------------------------------------------------
//* Function Name       : AddPointer
//* Object              : 计算指针增加偏移量
//* Input Parameters    : pt——数据区域结构
//*                       inc——累加值
//* Output Parameters   : 计算后的结果
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
u_int AddPointer(StructPT *pt, int inc)
{
	u_int result;
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
//* Object              : 获取疲劳驾驶结束点数据并且更新数据缓存区
//* Input Parameters    : none
//* Output Parameters   : none
//* Global Parameters   : otdrEND,InFlashWriting
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int Get_otdrEND(OTDR_start *start,OTDR_end *end)
{
	if((InFlashWriting&(1<<RUNRECORD360h))==0)
	{//如果没有开始写FLASH，首先读入最近一组数据的结束点数据
		return(GetOTDR( pTable.RunRecord360h.CurPoint, start, end ));
	}
	else{
		return(2);
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : Write4kToFlashOTDR
//* Object              : 先擦除再写4k内存数据到FLASH中
//* Input Parameters    : p——当前数据指针
//*                       Buffer——内存中待更新的4k数据区首地址
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int Write4kToFlashOTDR(u_short *p,u_short *Buffer)
{
	//擦除FLASH
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,(flash_word *)p))
		return (0);

	return(Write4kToFlash(p,Buffer));
}
//*----------------------------------------------------------------------------
//* Function Name       : Write4kToFlash
//* Object              : 写4k内存数据到FLASH中
//* Input Parameters    : p——当前数据指针
//*                       Buffer——内存中待更新的4k数据区首地址
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int Write4kToFlash(u_short *p,u_short *Buffer)
{
	u_short *inside_p,*flash_p;
	u_short data;
	inside_p = (u_short *)((u_int)p & 0x00fff);//4k内地址指针
	flash_p = p;
	while((u_int)inside_p<0x01000)
	{
		data = *((u_short *)((u_int)Buffer + (u_int)inside_p));
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,flash_p,data))
			return ( 0 ) ;
		
		inside_p++;
		flash_p++;
	}
	return ( 1 );
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteOTDRData
//* Object              : 写OTDR数据到OTDR内存区，如果需要，则换下一4k
//* Input Parameters    : buf——待写入的数据首地址
//*                       len——数据长度
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WriteOTDRData(u_char *buf,u_char len)
{
	u_char *p;
	u_int pos,pt,last_pos;
	u_char i;
	
	pos = 0x00fff & pTable.RunRecord360h.CurPoint;
	p = (u_char *)((u_int)(OTDR_4k) + pos);
	
	for(i=0;i<len;i++)
	{
		*p=buf[i];
		p++;
		pos++;
		if(pos==0x01000)
		{//切换到下一个4k
			//更新地址指针
			p=(u_char *)(OTDR_4k);
			last_pos = pTable.RunRecord360h.CurPoint&0xfffff000;
			pos = (pTable.RunRecord360h.CurPoint&0xff000) + 0x01000;
			if(pos>(pTable.RunRecord360h.EndAddr&0xff000))
				pos = pTable.RunRecord360h.BaseAddr;
			else
				pos += (u_int)DATAFLASH_BASE; 
			pTable.RunRecord360h.CurPoint = pos;
		
			//写当前4k缓冲区到FLASH中
			Write4kToFlashOTDR((u_short *)last_pos,(u_short *)OTDR_4k);
			//更新缓冲区
			Update4k((u_short *)pos,(u_short *)OTDR_4k,UpdateFlashOnce);
			pos=0;
		}
	}
	pTable.RunRecord360h.CurPoint &= 0xfffff000;
	pTable.RunRecord360h.CurPoint += pos;
	return(1);
}

//*----------------------------------------------------------------------------
//* Function Name       : WriteZeroToOTDREndData
//* Object              : 补零
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WriteZeroToOTDREndData(u_char zeroNB)
{
	u_int pt;
	u_char i,buf[20];
	int inc;
	
	if((InFlashWriting&(1<<RUNRECORD360h))==0)
	{//如果没有开始写FLASH，首先读入最近一组数据的结束点数据
		InFlashWriting |= 1<<RUNRECORD360h;//标记数据为“在写FLASH”
		inc = 0 - sizeof(OTDR_end);
		pt = AddPointer(&(pTable.RunRecord360h), inc);
		if(!Update4k((u_short *)pt,(u_short *)OTDR_4k,UpdateFlashOnce))
			return(0);
		pTable.RunRecord360h.CurPoint=pt;
	}
	
	for(i=0;i<20;i++)
		buf[i] = 0;
	WriteOTDRData(buf,zeroNB);
	
	return(1);
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteAverageSpeed
//* Object              : 写一个平均速度数据到FLASH中
//* Input Parameters    : V——待写入的平均速度值
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void WriteAverageSpeed(u_char v)
{
	u_char *p;
	u_int pos,pt,last_pos;
	pos = 0x00fff & pTable.RunRecord360h.CurPoint;
	p = (u_char *)((u_int)(OTDR_4k) + pos);
	*p = v;
	pos++;	
	if(pos==0x01000)
	{//切换到下一个4k
		//更新地址指针
		p=(u_char *)(OTDR_4k);
		last_pos = pTable.RunRecord360h.CurPoint&0xfffff000;
		pos = (pTable.RunRecord360h.CurPoint&0xff000) + 0x01000;
		if(pos>(pTable.RunRecord360h.EndAddr&0xff000))
			pos = pTable.RunRecord360h.BaseAddr;
		else
			pos += (u_int)DATAFLASH_BASE; 
		pTable.RunRecord360h.CurPoint = pos;
	
		//写当前4k缓冲区到FLASH中
		Write4kToFlashOTDR((u_short *)last_pos,(u_short *)OTDR_4k);
		//更新缓冲区
		Update4k((u_short *)pos,(u_short *)OTDR_4k,UpdateFlashOnce);
	}
	else
		pTable.RunRecord360h.CurPoint++;	 

}
//*----------------------------------------------------------------------------
//* Function Name       : WriteOTDRStartData
//* Object              : 360小时疲劳驾驶数据起始点数据
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : curTime
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WriteOTDRStartData()
{
	u_char *p;
	u_int pos;
	u_char i,buf[8];
	int inc;
	
	if((InFlashWriting&(1<<RUNRECORD360h))==0)
	{//如果没有开始写FLASH，首先读入最近一组数据的结束点数据
		InFlashWriting |= 1<<RUNRECORD360h;//标记数据为“在写FLASH”
		pos = pTable.RunRecord360h.CurPoint;
		if(!Update4k((u_short *)pos,(u_short *)OTDR_4k,UpdateFlashOnce))
			return(0);
	}
	else
		WriteOTDREndData(&otdrEND);	
	
	buf[0]=0xaf;
	buf[1]=0xaf;
	p=(u_char *)(&curTime);
	for(i=0;i<6;i++)
		buf[2+i]=p[i];
	
	WriteOTDRData(buf,sizeof(OTDR_start));
	return(1);
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteOTDREndData
//* Object              : 360小时疲劳驾驶数据起始点数据
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : curTime
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WriteOTDREndData(OTDR_end *end)
{
	u_int pos;

	if(end->MinuteNb==0)
	{
		pos = AddPointer(&(pTable.RunRecord360h), 0 - sizeof(OTDR_start));
		if((pos&0xff000)!=(pTable.RunRecord360h.CurPoint&0xff000))
		{
			if(!Update4k((u_short *)pos,(u_short *)OTDR_4k,UpdateFlashOnce))
				return(0);
		}
		pTable.RunRecord360h.CurPoint = pos;
	}
	else
		WriteOTDRData((u_char *)end,sizeof(OTDR_end));
}

//*----------------------------------------------------------------------------
//* Function Name       : IsDoubtPointData
//* Object              : 判断是否是一个疑点数据
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : Tick02NB,ddb
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int IsDoubtPointData()
{
    int i;
	if(Tick02NB>=25)
		return TRUE;
	else
	{
		i=ddb.pt-1;
		if(i<0)
			i=99;
		do{
			if(ddb.data[i].speed>4)
				return TRUE;
			i--;
			if(i<0)
				i=99;
		}while(ddb.data[i].speed!=0);
		return FALSE;
	}

}
//*----------------------------------------------------------------------------
//* Function Name       : RunRecord360Handler
//* Object              :360小时平均速度行驶记录和统计疲劳驾驶
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : CurSpeed,curTime
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void RunRecord360Handler()
{
	u_char AverageV; 
	u_char flag=InRecordCycle&(1<<RUNRECORD360h);
	u_char space=0xff;
	int succ;
	
	
	//记录当前疑点数据
	ddb.data[ddb.pt].speed = (u_char)CurSpeed;
	ddb.data[ddb.pt].status = CurStatus;
	ddb.pt++;
	if(ddb.pt>99)
		ddb.pt = 0;
		
	if(flag==0)//没有记录数据中（speed==0）
	{
//		if((CurSpeed > 0)&&(LastSPE > 0))
		if((CurSpeed - DeltaSpeed==0)&&(CurSpeed>0))
		{//记录起始点
			InRecordCycle |= 1<<RUNRECORD360h;//修改记录数据标志
			
			Tick02NB = 0;

			//判断是否连接上一条记录，即间隔时间是否不超过20分钟
			OTDR_start CurOTDR_start;
			OTDR_end CurOTDR_end;
			succ=Get_otdrEND(&CurOTDR_start,&CurOTDR_end);
			if(succ==1)//从FLASH中读入上一条记录
				otdrEND = CurOTDR_end;
				
			//连续两条记录的司机代号相等，且不是未知司机	
			if(succ && (otdrEND.driver.DriverCode != 0) && 
				(otdrEND.driver.DriverCode==RecordDriver.DriverCode))	
				space = JudgeTimeSpace(&curTime,&otdrEND);
				
			//一条记录内（一次上电）连续两次速度曲线的司机代号为未知司机
			if((succ == 2) && (otdrEND.driver.DriverCode == 0)&&
				(RecordDriver.DriverCode ==0 ))
				space = JudgeTimeSpace(&curTime,&otdrEND);
				
			//一条记录内（一次上电）连续两次速度曲线,前次为未知司机，后次为已知
			if((succ == 2) && (otdrEND.driver.DriverCode == 0)&&
			(RecordDriver.DriverCode !=0 )&&(PowerOnDT.type==0xefef))
				space = JudgeTimeSpace(&curTime,&otdrEND);
				
			//时间间隔在限定时间内，合并上一条记录
			if((space<=RestMinuteLimit)&&(PowerOnDT.type!=0))
			{
				OTRecordType = MergeLastData;
				LastDistance = otdrEND.TotalDistance;
				//补space个零到end指针位置
				WriteZeroToOTDREndData(space);
				otdrEND.MinuteNb += space; 
			}
			else//新记录
			{
				if((succ == 2) && (otdrEND.driver.DriverCode == 0)&&
					(RecordDriver.DriverCode !=0 )&&(PowerOnDT.type==0xefef))
				{
					//修改前一条记录的司机
					//ModifyDriverToLastOTDR();
					otdrEND.driver = RecordDriver;
					 
				}
				//开始写一条新记录
				WriteOTDRStartData();
				OTRecordType = NewOTData;
				otdrEND.MinuteNb = 0;
			}
			//给otdrEND数据结构赋值
			otdrEND.TotalDistance = PulseTotalNumber;
			
		}
	}
	else//正在记录数据（speed>0）
	{
		//*记录1分钟平均速度
		AddupSpeed +=CurSpeed;
		SpeedNb ++;
		if(TimeChange & (0x01<<MINUTE_CHANGE))//到一分钟
		{
			AverageV=AddupSpeed/SpeedNb;
			AddupSpeed = 0;
			SpeedNb = 0;
			otdrEND.MinuteNb ++;
			WriteAverageSpeed(AverageV);
		}
		
		Tick02NB ++;
		
		//*记录停车结束点
		if(((CurSpeed == 0)&&(PulseNB_In1Sec == 0))||((CurSpeed == 0)&&(!PowerOn)))
		{
			if(IsDoubtPointData())
				WriteDoubtDataToFlash();//写一个停车疑点数据到数据flash中
			InRecordCycle &= ~(1<<RUNRECORD360h);//修改记录数据标志
			//记录结束数据点
			otdrEND.dt.type = 0xeaea;
			otdrEND.dt.year = curTime.year;
			otdrEND.dt.month = curTime.month;
			otdrEND.dt.day = curTime.day;
			otdrEND.dt.hour = curTime.hour;
			otdrEND.dt.minute = curTime.minute;
			otdrEND.dt.second = curTime.second;
			otdrEND.TotalDistance = PulseTotalNumber - otdrEND.TotalDistance;
			otdrEND.driver = RecordDriver;
			if(OTRecordType == MergeLastData)
				otdrEND.TotalDistance += LastDistance;
		}
	}
}		
//*----------------------------------------------------------------------------
//* Function Name       : FinishOTDRToFlash
//* Object              :将当前记录360小时平均速度行驶记录和统计疲劳驾驶内存
//                       保存到FLASH中
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void FinishOTDRToFlash()
{
	u_int p;
	otdrEND.driver = RecordDriver;
	if((InRecordCycle&(1<<RUNRECORD360h))!=0)
	{
		//记录结束数据点
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
	if(pTable.RunRecord360h.CurPoint!=PartitionTable_BASE->RunRecord360h.CurPoint)
	{
		p = pTable.RunRecord360h.CurPoint;
		p &= 0xfffff000;
		Write4kToFlashOTDR((u_short *)p,(u_short *)OTDR_4k);
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : ModifyUnknownDriver
//* Object              : 每次行驶记录结束时将记录中记录成未知司机，但后来插卡后
//						  变成已知司机的记录修改其司机代号
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void ModifyUnknownDriver()
{
	u_int dpp;
	u_short data;
	Record_CLOCK stoptime,PowerOffDT;
	int cmp1,cmp2;
	Record_CLOCK *temp;
	u_char i=0;
	
	PowerOffDT.year = curTime.year;
	PowerOffDT.month = curTime.month;
	PowerOffDT.day = curTime.day;
	PowerOffDT.hour = curTime.hour;
	PowerOffDT.minute = curTime.minute;
	PowerOffDT.second = curTime.second;
	
	//疑点记录
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
			data = (u_short)(RecordDriver.DriverCode);
			flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,(u_short *)dpp,data);
			data = (u_short)(RecordDriver.DriverCode>>16);
			flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,(u_short *)(dpp+2),data);
		}
		i++;//2003.11.11,panhui(死机问题)
	}while((cmp1>0)&&(cmp2>=0)&&(i<97));
	
	//360小时平均速度记录
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteBaseDataToFlash
//* Object              : 将行驶记录写入数据FLASH
//* Input Parameters    : buf——数据缓冲区指针
//                        len——u_short型数据长度
//						  type——数据类型
//* Output Parameters   : if data write TRUE or FALSE
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int WriteBaseDataToFlash(u_short *buf,u_char len,u_char type)
{

	u_char i;
	u_int pos,UpdatePos;
	
	if((InFlashWriting&(1<<BASEDATA))==0)
	{//如果没有开始写FLASH
		if(type!=START)
			return(0);
		InFlashWriting |= 1<<BASEDATA;//标记数据为“在写FLASH”
		pos = pTable.BaseData.CurPoint;
		if(!Update4k((u_short *)pos,BaseData_4k,UpdateFlashTimes))
			return(0);

		/////////*******2003.10.06 panhui*********////////
		UpdatePos = pos + RecordFlagByte*2;
		if(UpdatePos > pTable.BaseData.EndAddr)
			UpdatePos = pTable.BaseData.BaseAddr + (UpdatePos-pTable.BaseData.EndAddr)-1;
		if((pos&0xff000)!=(UpdatePos&0xff000))
		{//跨页
			if(!Update4k((u_short *)UpdatePos,BaseData_4k,UpdateFlashAll))
				return(0);
		}
		/////////*******2003.10.06 panhui*********////////
		
	}
	else
		pos = pTable.BaseData.CurPoint;
	
	for(i=0;i<len;i++)
	{
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,(flash_word *)pos,buf[i]))
			return(0);
		pos+=2;
		////////////add by panhui 2003.10.28////////
		if(pos>pTable.BaseData.EndAddr)
			pos = pTable.BaseData.BaseAddr;
		////////////////////////////////////////////
		UpdatePos = pos + RecordFlagByte*2;
		if(UpdatePos > pTable.BaseData.EndAddr)
			UpdatePos = pTable.BaseData.BaseAddr + (UpdatePos-pTable.BaseData.EndAddr)-1;
/*		if((pos&0x00fff)==0)
		{//需要更换下一4k
			if(pos>pTable.BaseData.EndAddr)
				pos = pTable.BaseData.BaseAddr;
			if(!Update4k((u_short *)pos,BaseData_4k,UpdateFlashTimes))
				return(0);
			
		}*/
		if(((pos&0xff000)!=(UpdatePos&0xff000))&&((UpdatePos&0x00fff)==0))
		{//需要更换下一4k
			if(!Update4k((u_short *)UpdatePos,BaseData_4k,UpdateFlashAll))
				return(0);
		}
		
	}
	pTable.BaseData.CurPoint = pos;
	
	if(type==END)
	{
		Write4kToFlash((u_short *)UpdatePos,BaseData_4k);
		if(pTable.DoubtPointData.CurPoint!=PartitionTable_BASE->DoubtPointData.CurPoint)
		{
			Write4kToFlash((u_short *)pTable.DoubtPointData.CurPoint,DoubtData_4k);
			FinishOTDRToFlash();
		}
		else if(pTable.RunRecord360h.CurPoint!=PartitionTable_BASE->RunRecord360h.CurPoint)
			FinishOTDRToFlash();

		InFlashWriting = 0;
		WritePartitionTable(&pTable);
		
		//修改可能的“未知司机”疑点和360平均速度记录
		if(PowerOnDT.type==0xefef)
			ModifyUnknownDriver();
		
		PowerOnDT.type = 0;

	}
	return(1);

}
//*----------------------------------------------------------------------------
//* Function Name       : BaseDataHandler
//* Object              : 基本详细数据处理及其记录
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : PowerOnTime,STATUS1min,TimeChange,InRecordCycle,curTime
//* 修改的全局变量      : PowerOnTime,STATUS1min,InRecordCycle   
//*----------------------------------------------------------------------------
void BaseDataHandler()
{
	int i,pt;
	u_short data;
	RecordData_start rec_start;

	if((InRecordCycle&(1<<BASEDATA))==0)
	{//如果没有开始记录基本行驶数据
		
//		if((PowerOn&&(PowerOnTime>=5))||((CurSpeed - DeltaSpeed==0)&&(CurSpeed>0)))
		if(CurSpeed>0)
		{//上电时刻，记录一个起点
			PowerOnTime = 0;
			PulseTotalNumber = 0;//里程	清零
			
			InRecordCycle |= 1<<BASEDATA;//标记“开始记录数据”
			STATUS1min = 0;//2003.10.23,panhui
			
			//准备起始数据
			rec_start.dt.type = 0xFEFE;
			rec_start.dt.year = curTime.year;
			rec_start.dt.month = curTime.month;
			rec_start.dt.day = curTime.day;
			rec_start.dt.hour = curTime.hour;
			rec_start.dt.minute = curTime.minute;
			rec_start.dt.second = curTime.second;
			
			PowerOnDT = rec_start.dt;//初始化上电时间和标志
			if(CurrentDriver.DriverCode == 0)
				PowerOnDT.type = 0xefef;
			
			//判断当前指针是否为偶数，若为奇数，加一个字节
			if((pTable.BaseData.CurPoint&1)!=0)
			{
				pTable.BaseData.CurPoint++;
				if(pTable.BaseData.CurPoint>pTable.BaseData.EndAddr)
					pTable.BaseData.CurPoint=pTable.BaseData.BaseAddr;
			}
			
			//行驶记录起点写入dataflash
			WriteBaseDataToFlash((u_short *)(&rec_start),(sizeof(RecordData_start))/2,START);
			
		}
		else if(PowerOn&&(PowerOnTime<5))
//		{
//			if(PowerOnTime==0)
//				PulseTotalNumber = 0;
			PowerOnTime ++;
//		}
		else
			PowerOnTime = 0;
	}
	else
	{//正在记录行驶数据
		if((CurSpeed == 0)&&(!PowerOn))
		{//速度为零时断电，记录一个结束点
			InRecordCycle &= ~(1<<BASEDATA);//标记“结束记录数据”
			//准备结束数据
			rec_end.dt.type = 0xaeae;
			rec_end.dt.year = curTime.year;
			rec_end.dt.month = curTime.month;
			rec_end.dt.day = curTime.day;
			rec_end.dt.hour = curTime.hour;
			rec_end.dt.minute = curTime.minute;
			rec_end.dt.second = curTime.second;
			rec_end.DistancePulse = PulseTotalNumber;//DistancePulse
			rec_end.DriverCode = RecordDriver.DriverCode;

			pTable.TotalDistance += PulseTotalNumber;
			
			FinishFlag = 1;

		}
		else if((CurSpeed - DeltaSpeed==0)&&(CurSpeed>0)&&
		(CurrentDriver.DriverCode!=RecordDriver.DriverCode)&&
		(CurrentDriver.DriverCode!=0))
		{//更换司机，记录一个结束点
			//准备结束数据
			rec_end.dt.type = 0xaeae;
			rec_end.dt.year = curTime.year;
			rec_end.dt.month = curTime.month;
			rec_end.dt.day = curTime.day;
			rec_end.dt.hour = curTime.hour;
			rec_end.dt.minute = curTime.minute;
			rec_end.dt.second = curTime.second;
			rec_end.DistancePulse = PulseTotalNumber;//DistancePulse
			rec_end.DriverCode = RecordDriver.DriverCode;
			pTable.TotalDistance += PulseTotalNumber;
			
			//行驶记录结束点写入dataflash
			WriteBaseDataToFlash((u_short *)(&rec_end),(sizeof(RecordData_end))/2,END);
			
			//准备起始数据
			rec_start.dt.type = 0xFEFE;
			rec_start.dt.year = curTime.year;
			rec_start.dt.month = curTime.month;
			rec_start.dt.day = curTime.day;
			rec_start.dt.hour = curTime.hour;
			rec_start.dt.minute = curTime.minute;
			rec_start.dt.second = curTime.second;
			PowerOnDT = rec_start.dt;//初始化上电时间和标志
			
			//行驶记录起点写入dataflash
			WriteBaseDataToFlash((u_short *)(&rec_start),(sizeof(RecordData_start))/2,START);
			RecordDriver = CurrentDriver;
			PulseTotalNumber = 0;

		}
		else
		{//1秒钟记录一组速度状态数据
			STATUS1min |= CurStatus;
			if(TimeChange & (0x01<<SECOND_CHANGE))//到一秒钟
			{
				data = (u_char)CurSpeed;
				data = (STATUS1min<<8) + data;
				STATUS1min = 0;
				WriteBaseDataToFlash((u_short *)(&data),1,DATA_1min);
				#if RPM_EN
				data = (u_short)CurEngine;
				WriteBaseDataToFlash((u_short *)(&data),1,DATA_1min);
				#endif
			}
	
		}
	}

}
//*----------------------------------------------------------------------------
//* Function Name       : FinishAllRecord
//* Object              : 结束所有记录
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : PowerOnTime,STATUS1min,TimeChange,InRecordCycle,curTime
//* 修改的全局变量      : PowerOnTime,STATUS1min,InRecordCycle   
//*----------------------------------------------------------------------------
void FinishAllRecord()
{
	if(FinishFlag)
	{
		//行驶记录结束点写入dataflash
		WriteBaseDataToFlash((u_short *)(&rec_end),(sizeof(RecordData_end))/2,END);
		RecordDriver = CurrentDriver;//恢复记录司机
		FinishFlag = 0;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : GetOTDRDataFromFlash
//* Object              : 从给定的地址获取inc长度的疲劳驾驶数据
//*                       并存放到BUF中
//* Input Parameters    : p——给定的地址
//*                       inc——数据长度，根据符号确定读数据的方向
//*                       注意：inc可以为奇数或偶数
//*                       buf——缓冲区
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void GetOTDRDataFromFlash(u_short *p, int inc,u_char *buf)
{
	int  i;
	u_short data;

	if(inc==0)
		return;
	if(inc<0){
		StructPT temp;
		temp = pTable.RunRecord360h;
		temp.CurPoint = (u_int)p;
		p = (u_short *)AddPointer(&temp, inc);
		inc=0-inc;
	}

	//如果指针为偶数
	if(((u_int)p&1)==0)
	{
		for(i=0;i<inc/2;i++){
			buf[2*i] = (u_char)(*p);
			buf[2*i+1] = (u_char)((*p)>>8);
			p++;
			if((u_int)p > pTable.RunRecord360h.EndAddr)
				p = (u_short *)pTable.RunRecord360h.BaseAddr;
		}
		//如果inc为奇数
		if((inc&1)==1)
		{
//			p=(u_short *)((u_int)p-1);
			data = *p;
			buf[inc-1] = (u_char)data;
			if((u_int)p > pTable.RunRecord360h.EndAddr)
				p = (u_short *)pTable.RunRecord360h.BaseAddr;
		}
	}
	//如果指针为奇数
	else{
		//如果inc为偶数
		if((inc&1)==0)
		{
			p=(u_short *)((u_int)p-1);
			data = *p;
			buf[0] = (u_char)(data>>8);
			p++;
			for(i=0;i<inc/2;i++)
			{
				data = *p;
				buf[i*2+1]=data;
				if(i!=(inc/2-1))
					buf[i*2+2]=(u_char)(data>>8);
				p++;
				if((u_int)p > pTable.RunRecord360h.EndAddr)
					p = (u_short *)pTable.RunRecord360h.BaseAddr;
			}
		}
		//如果inc为奇数
		if((inc&1)==1)
		{
			p=(u_short *)((u_int)p-1);
			data = *p;
			buf[0] = (u_char)(data>>8);
			p++;
			for(i=0;i<inc/2;i++)
			{
				data = *p;
				buf[i*2+1]=data;
				buf[i*2+2]=(u_char)(data>>8);
				p++;
				if((u_int)p > pTable.RunRecord360h.EndAddr)
					p = (u_short *)pTable.RunRecord360h.BaseAddr;
			}		
		}
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : IsCorrectCLOCK
//* Object              : 判断时间数据是否正确
//* Input Parameters    : 时钟指针
//* Output Parameters   : 是否是一个正确的时间数据
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int IsCorrectCLOCK(CLOCK *dt)
{
	u_char data;
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
//* Function Name       : IsCorrectClock
//* Object              : 判断时间数据是否正确
//* Input Parameters    : 时钟指针
//* Output Parameters   : 是否是一个正确的时间数据
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int IsCorrectClock(Record_CLOCK *dt)
{
	u_char data;
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
//* Object              : 获取当前的疲劳驾驶记录
//* Input Parameters    : p——当前指针;
//*                       s——疲劳驾驶记录起始数据;
//*                       e——疲劳驾驶记录结束数据;
//* Output Parameters   : 是否是一个疲劳驾驶记录
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int GetOTDR( u_int p, OTDR_start *s, OTDR_end *e )
{
	int offset;
	offset = 0 - sizeof(OTDR_end);
	GetOTDRDataFromFlash((u_short *)p, offset,(u_char *)e);
	if(e->dt.type!=0xeaea)
		return (0);
	if(!IsCorrectClock(&(e->dt)))
		return (0);
	
	StructPT temp;
	temp = pTable.RunRecord360h;
	temp.CurPoint = (u_int)p;
	offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+e->MinuteNb);
	p = AddPointer(&temp, offset);
	offset = sizeof(OTDR_start);
	GetOTDRDataFromFlash((u_short *)p, offset,(u_char *)s);
	if(s->dt.type!=0xafaf)
		return (0);
	if(!IsCorrectClock(&(s->dt)))
		return (0);

	return(1);
	
}
//*----------------------------------------------------------------------------
//* Function Name       : GetOverTimeRecordIn2Days
//* Object              : 获取当前司机的两个日历天内的疲劳驾驶记录
//* Input Parameters    : 记录疲劳驾驶记录的数组指针（数组大小＝15个）
//* Output Parameters   : 疲劳驾驶记录的个数
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
u_char GetOverTimeRecordIn2Days(OTDR *record)
{
	OTDR_start CurOTDR_start;
	OTDR_end CurOTDR_end;
	StructPT spt;
	u_char next=0;//=0——重新开始一个新记录
				  //=1——等待下一条记录
	
	u_int p;
	int offset,i;
	u_char number=0;
	CLOCK time;
	u_char timeflag=0;
	DateTime LastDT,CurDT;
	int space;
	u_int addup,bytes=0;

	spt = pTable.RunRecord360h;
	p = pTable.RunRecord360h.CurPoint;

	for(i=0;i<15;i++)
	{
		record[i].end.MinuteNb = 0;
		record[i].end.driver.DriverCode = 0;
	}

	do
	{
		//获取当前的疲劳驾驶记录
		if(!GetOTDR( p, &CurOTDR_start, &CurOTDR_end))
		{
			offset = -1;
			p = AddPointer(&spt, offset);
			spt.CurPoint = p;
			bytes++;
			if((bytes>=4096)||(p == pTable.RunRecord360h.CurPoint))
			{//再也没有记录了
				if(record[number].end.MinuteNb>DriveMinuteLimit)
					number++;
				break;
			}
			continue;
		}
		//计算新的指针值
		offset = 0 - (sizeof(CurOTDR_end)+sizeof(CurOTDR_start)+CurOTDR_end.MinuteNb);
		p = AddPointer(&spt, offset);
		spt.CurPoint = p;
				
		//比较司机代号是否相等
		if(PartitionTable_BASE->DriverCode!=CurOTDR_end.driver.DriverCode)
			continue;
			
		if(timeflag == 0)
		{//记录计算2个日历天的起点
			time.year = CurOTDR_end.dt.year;
			time.month = CurOTDR_end.dt.month;
			time.day = CurOTDR_end.dt.day;
			time.hour = CurOTDR_end.dt.hour;
			time.minute = CurOTDR_end.dt.minute;
			time.second = CurOTDR_end.dt.second;
			timeflag = 1;
		}

		
		//比较时间是否满足要求以及是否在两个日历天内
		if((time.year!=CurOTDR_end.dt.year)||(time.month!=CurOTDR_end.dt.month)||(time.day!=CurOTDR_end.dt.day))
		{//如果不是同一日历天
			//判断是否是前一个日历天？
			if(!IfOneAfterAotherDay(&time, &CurOTDR_end))
			{
				if(record[number].end.MinuteNb>DriveMinuteLimit)
					number++;
				break;
			}
		}
		
		//统计疲劳驾驶
		if(next == 0)
		{
			record[number].start = CurOTDR_start;
			record[number].end = CurOTDR_end;
			next = 1;
		}
		else if(next == 1)
		{
			PrepareTime((u_char *)(&(CurOTDR_end.dt.year)),&LastDT);
			PrepareTime((u_char *)(&(record[number].start.dt.year)),&CurDT);
			space=HaveTime(CurDT,LastDT);
			if(space<0)
			{
				if(record[number].end.MinuteNb>DriveMinuteLimit)
					number++;
				break;
			}
			else if(space<RestMinuteLimit)
			{//合并记录
				addup = record[number].end.MinuteNb+space+CurOTDR_end.MinuteNb;
				record[number].end.MinuteNb = addup;
				addup = record[number].end.TotalDistance+CurOTDR_end.TotalDistance;
				record[number].end.TotalDistance = addup;
				record[number].start = CurOTDR_start;
			}
			else
			{//确定是否上一条为疲劳驾驶记录，并记录本次记录
				if(record[number].end.MinuteNb>DriveMinuteLimit)
					number++;
				record[number].start = CurOTDR_start;
				record[number].end = CurOTDR_end;
			}
		}
		
	
	}while((bytes<4096)&&(p != pTable.RunRecord360h.CurPoint));//当未读完疲劳驾驶记录

	return (number);
}
//*----------------------------------------------------------------------------
//* Function Name       : ComputeDistance100m
//* Object              : 根据脉冲数和车轮系数计算里程数，单位为百米
//* Input Parameters    : pulseNb——里程脉冲数
//* Output Parameters   : 百米数（0-5804009）
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
u_int ComputeDistance100m(u_int pulseNb)
{
	u_int result;
	u_char PP = PARAMETER_BASE->PulseNumber;
	if((PP==0)||(PP>24))
		PP = 8;
	result = (pulseNb/(PP*PARAMETER_BASE->CHCO/10));
	return result;
}
//*----------------------------------------------------------------------------
//* Function Name       : CompareDateTime
//* Object              : 比较两个时间的大小
//* Input Parameters    : dt1——时间1；dt2——时间2
//* Output Parameters   : 1 —— dt1 > dt2;
//                       -1 —— dt1 < dt2;
//                        0 —— dt1 = dt2;
//* 引用的全局变量      : none
//* 修改的全局变量      : none
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
//* Object              : 数据指针定位
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void DataPointerSeek()
{
	u_int DataPoint;
	u_short data;
	u_char *p;
	u_char temp,f1,f2;
	u_char update = 0;
	int i;
	DataPoint = pTable.BaseData.CurPoint;
	p = (u_char *)DataPoint;
	//查找当前指针位置是否存在标志
	for(i=0;i<RecordFlagByte;i++)
	{
		if(*p!=0xff)
			break;
		if((u_int)p==pTable.BaseData.EndAddr)
			p=(u_char *)pTable.BaseData.BaseAddr;
		else
			p++;
	}
	if(i==RecordFlagByte)//标志存在，结束指针定位。
		return;
		
	//重新定位基本数据的指针
	do 
	{
		if((*p == 0xff)&&(((u_int)p&1)==0))
		{//可能找到标志
			i=0;
			do
			{
				if((u_int)p == pTable.BaseData.EndAddr)
					p = (u_char *)pTable.BaseData.BaseAddr;
				else
					p++;
				i++;
			}while((*p==0xff)&&(i<RecordFlagByte));
			if(i == RecordFlagByte)//找到最新指针位置
			{
				//查看标志之前是否有结束标志AEAE
				//1)判断是否存在已经写过aeae标志的非正常断电记录
				DataPoint = (u_int)p;
				DataPoint -= (RecordFlagByte+2);
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				f1 = *((u_char *)DataPoint);
				f2 = *((u_char *)(DataPoint+1));
				if((f1==0xae)&&(f2==0xae))
				{//非正常断电记录，但指针调整
					pTable.BaseData.CurPoint = (u_int)p;
					pTable.BaseData.CurPoint = AddPointer(&(pTable.BaseData),-2);
					update = 1;
					break;
				}
				
				//2)判断是否存在正常断电记录
				DataPoint = (u_int)p;
				DataPoint -= RecordFlagByte*2;
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				f1 = *((u_char *)DataPoint);
				f2 = *((u_char *)(DataPoint+1));
					
				//计算标志起始位置
				/////////*******2003.10.06 panhui*********////////
				DataPoint = (u_int)p;
				DataPoint -= RecordFlagByte;
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				if((f1==0xae)&&(f2==0xae))//正常记录的数据，但指针调整
					pTable.BaseData.CurPoint = DataPoint;
				else
				{//非正常断电,未来得及写结束标志AEAE
					//写数据结束标志
					data = 0xaeae;
					flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,(flash_word *)DataPoint,data);
					//更新分区表
					pTable.BaseData.CurPoint = (u_int)p;
				}
				/////////*******2003.10.06 panhui*********////////
				update = 1;
				break;
			}
		}
		else
		{//未找到标志，向下移动指针
			if((u_int)p == pTable.BaseData.EndAddr)
				p = (u_char *)pTable.BaseData.BaseAddr;
			else
				p++;
		}
	}while((u_int)p != DataPoint);
	
	if(update)
	{
		//重新定位疑点数据的指针
		Record_CLOCK LastDT,CurDT;
		u_char *dp;
		DataPoint = pTable.DoubtPointData.CurPoint;
		int result;
		
		if(DataPoint == pTable.DoubtPointData.BaseAddr)
			dp = (u_char *)(DataPoint+96*210+4);
		else
			dp = (u_char *)(DataPoint-210+4);
		LastDT.year = *dp;dp++;
		LastDT.month = *dp;dp++;
		LastDT.day = *dp;dp++;
		LastDT.hour = *dp;dp++;
		LastDT.minute = *dp;dp++;
		LastDT.second = *dp;
		do{
		
			dp = (u_char *)(DataPoint+4);
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
			//////////秦皇岛3台车//////////////////////////////
			LastDT = CurDT;
				
		}while(DataPoint != pTable.DoubtPointData.CurPoint);	
		
		pTable.DoubtPointData.CurPoint = DataPoint;
		
		WritePartitionTable(&pTable);
	}
}