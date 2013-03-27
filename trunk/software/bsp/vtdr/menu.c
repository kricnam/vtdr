//*----------------------------------------------------------------------------
//*      LCD菜单切换
//*----------------------------------------------------------------------------
//* File Name           : menu.c
//* Object              : 实现液晶显示器菜单功能
//*
//* 1.0 24/02/03 PANHUI : Creation
//*----------------------------------------------------------------------------
#include    "parts/r40807/reg_r40807.h"
#include    "parts/r40807/lib_r40807.h"
#include 	"menu.h"
#include 	"lcd.h"
#include 	"ibb3.h"
#include 	"lcd_word_model.h"
#include    "sl811mheader.h"
#include    "includes.h"

extern CLOCK curTime;
extern u_int PulseNum;
extern u_int TimerNum;
extern u_int CurEngine;     
extern u_short STATUS;			/*16种状态*/
extern PartitionTable pTable;
extern StructPara Parameter;
extern u_int Distance;         /*本次行驶里程*/
extern u_int LastPN[];
extern u_char PowerOn;
extern u_char LastPowerOn;

extern u_char LargeDataBuffer[];
extern char CardIn;
extern u_int CurSpeed;
extern char ClockVL;
extern u_char ClockType; 
//#if OpenDoorDeal
extern u_char TimeControlREG0;
extern u_char CurDoorStatus;
extern 	void JudgeDoorType(void);
extern void DoorType();
//#endif
extern u_char AlarmFlag;

extern u_char RoadNb;

extern UsartDesc *RS2320;

//全局变量
LCDTCB lcd_tcb;			//lcd显示当前控制块
LCDTCB last_lcd_tcb;	//lcd显示上一状态控制块
ACTION_TCB act_tcb;		//菜单动作控制块
OTDR *OTDR_Array = (OTDR *)(&(LargeDataBuffer[24*1024]));//[15];

//每项菜单显示的文字
const LCD_ZM * content00[] = {
	display_xian,
	display_shi,
	can,
	data_shu,
	NULL
};//显示参数

const LCD_ZM * content01[] = {
	display_xian,
	display_shi,
	record_ji,
	record_lu,
	data_shu,
	data_ju,
	NULL	
};//显示记录数据

const LCD_ZM * content02[] = {
	print_da,
	print_yin,
	NULL
};//打印

const LCD_ZM * content04[] = {
	data_shu,
	data_ju,
	save_bao,
	save_cun,
	to_dao,
	udisk_u,
	udisk_pan,
	NULL
};//数据保存到优盘

const LCD_ZM * content03[] = {
	other_qi,
	other_ta,
	operate_cao,
	operate_zuo,
	NULL
};//其他操作

const LCD_ZM * content30[] = {
	door_jian,
	door_ce,
	door_kai,
	door_guan,
	door_men,
	NULL
};//检测开关门
const LCD_ZM * content31[] = {

	door_kai,
	door_guan,
	door_men,
	can,
	data_shu,
	NULL
};//开关门参数

const LCD_ZM * back[] = {
	back_fan,
	back_hui,
	NULL
};//返回

const LCD_ZM * content10[] = {
	auto_che,
	auto_pai,
	number_hao,
	code_ma,
	NULL
};//车牌号码

const LCD_ZM * content11[] = {
	driver_jia,
	driver_shi,
	driver_yuan,
	code_dai,
	code_ma,
	NULL
};//驾驶员代码

const LCD_ZM * content12[] = {
	//driver_jia,//myw 2003.7.14驾驶员
	//driver_shi,
	//driver_yuan,
	driver_jia,
	driver_shi,
	zheng,
	number_hao,
	code_ma,
	NULL
};//驾驶证号码

const LCD_ZM * content13[] = {
	auto_che,
	auto_liang,
	character_te,
	character_zheng,
	xi,
	data_shu,
	NULL
};//车辆特征系数

const LCD_ZM * content14[] = {
	status_zhuang,
	status_tai,
	ji,
	xing,
	NULL
};//状态极性

const LCD_ZM * content15[] = {
	product_chan,
	product_pin,
	version_ban,
	version_ben,
	number_hao,
	NULL
};//产品版本号

const LCD_ZM * content20[] = {
	every_mei,
	minute_fen,
	minute_zhong,
	//average_ping,//myw 2003.7.14平均
	//average_jun,
	auto_che,
	speed_su,
	NULL
};//每分钟车速

const LCD_ZM * content21[] = {
	lian,
	xu,
	driver_jia,
	driver_shi,
	record_ji,
	record_lu,
	NULL
};//连续驾驶记录

const LCD_ZM * content22[] = {
	addup_lei,
	addup_ji,
	all_zong,
	distance_li,
	distance_cheng,
	NULL
};//累计总里程

const LCD_ZM * none[] = {
	none_wu,
	record_ji,
	record_lu,
	NULL
};//无记录

const LCD_ZM * being_stat[] = {
	being_zheng,
	being_zai,
	stat_tong,
	stat_ji,
	NULL
};//正在统计

const LCD_ZM * working_ok[] = {
	work_gong,
	work_zuo,
	being_zheng,
	normal_chang,
	NULL
};//工作正常

//菜单结点列表
const MENU_NODE list0[6]={
	{
		(LCD_ZM **)content00,
		1,
		-1,
		-1,
		NULL
	},
	{
		(LCD_ZM **)content01,
		2,
		-1,
		-1,
		NULL
	},
	{
		(LCD_ZM **)content02,
		-1,
		-1,
		-1,
		PrintAllData
	},
	{
		(LCD_ZM **)content04,
		-1,
		-1,
		-1,
		SaveDatatoUdisk
	},
	{
		(LCD_ZM **)content03,
		3,
		-1,
		-1,
		NULL
	},
	{
		(LCD_ZM **)back,
		-1,
		-1,
		-1,
		NULL
	}
};
const MENU_NODE list1[7]={
	{
		(LCD_ZM **)content10,
		-1,
		0,
		0,
		DisplayAutoCode
	},
	{
		(LCD_ZM **)content11,
		-1,
		0,
		0,
		DisplayDriverNumber
	},
	{
		(LCD_ZM **)content12,
		-1,
		0,
		0,
		DisplayDriverCode
	},
	{
		(LCD_ZM **)content13,
		-1,
		0,
		0,
		Displaywheel
	},
	{
		(LCD_ZM **)content14,
		-1,
		0,
		0,
		DisplayStatusPolarity
	},
	{
		(LCD_ZM **)content15,
		-1,
		0,
		0,
		DisplayProductVersion
	},
	{
		(LCD_ZM **)back,
		-1,
		0,
		0,
		NULL
	}

};
const MENU_NODE list2[4]={
	{
		(LCD_ZM **)content20,
		-1,
		0,
		1,
		Display15MinAverageSpeed
	},
	{
		(LCD_ZM **)content21,
		-1,
		0,
		1,
		Display2DayOTDR
	},
	{
		(LCD_ZM **)content22,
		-1,
		0,
		1,
		DisplayTotalDistance
	},
	{
		(LCD_ZM **)back,
		-1,
		0,
		1,
		NULL
	}
};
#if OpenDoorDeal
const MENU_NODE list3[3]={
	{
		(LCD_ZM **)content30,
		-1,
		0,
		3,
		JudgeDoorType
	},
	{
		(LCD_ZM **)content31,
		-1,
		0,
		3,
		DoorType
	},
	{
		(LCD_ZM **)back,
		-1,
		0,
		3,
		NULL
	}
};
#else
const MENU_NODE list3[3]={
	{
		(LCD_ZM **)content30,
		-1,
		0,
		3,
		NULL
	},
	{
		(LCD_ZM **)content31,
		-1,
		0,
		3,
		NULL
	},
	{
		(LCD_ZM **)back,
		-1,
		0,
		3,
		NULL
	}
};
#endif
NODE_LIST NodeListTable[4] = {
	{
		(MENU_NODE *)list0,
		6
	},
	{
		(MENU_NODE *)list1,
		7
	},
	{
		(MENU_NODE *)list2,
		4
	},
	{
		(MENU_NODE *)list3,
		3
	}
};
//*----------------------------------------------------------------------------
//* Function Name       : SaveDatatoUdisk
//* Object              : 保存数据到优盘，动作完成后返回到通常界面
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void SaveDatatoUdisk()
{
   usb_host_init();
   Udisk_handler();

}
//*----------------------------------------------------------------------------
//* Function Name       : PrintAllData
//* Object              : 打印所有数据，动作完成后返回到通常界面
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void PrintAllData()
{
	lcm_clear_ram(LINE2);
	lcm_write_hz1(1,1,(LCD_ZM *)being_zheng);
	lcm_write_hz1(1,2,(LCD_ZM *)being_zai);
	lcm_write_hz1(1,3,(LCD_ZM *)print_da);
	lcm_write_hz1(1,4,(LCD_ZM *)print_yin);

	#if GetSpeedStatusBy232
	PIO_SODR = SPEED;//使能打印机的通讯线
	#endif
	
	Printer();//打印
	
	#if GetSpeedStatusBy232
	PIO_CODR = SPEED;//禁止打印机的通讯线
	at91_usart_open(RS2320,US_ASYNC_MODE,BAUDS4800,0);
	#endif
	
	DisplayNormalUI();
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteDataToUDiskMenu
//* Object              : 保存数据到U盘，动作完成后返回到通常界面
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void WriteDataToUDiskMenu()
{
	//at91_pio_write ( &PIO_DESC, (1<<8), PIO_CLEAR_OUT );
	//delayms(10);
	//at91_pio_write ( &PIO_DESC, (1<<8), PIO_SET_OUT );//复位SL811
	//delayms(10);

//	sl811write(0x0f,0x80);
	int i;
	//等待
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)scan_sao);
	lcm_write_hz1(1,2,(LCD_ZM *)scan_miao);
	lcm_write_hz1(1,3,(LCD_ZM *)udisk_u);
	lcm_write_hz1(1,4,(LCD_ZM *)udisk_pan);

	if(Scan_UDisk())		
	{	
		lcm_clear_ram(LINE2);
		lcm_write_hz1(1,1,(LCD_ZM *)being_zheng);
		lcm_write_hz1(1,2,(LCD_ZM *)being_zai);
		lcm_write_hz1(1,3,(LCD_ZM *)save_bao);
		lcm_write_hz1(1,4,(LCD_ZM *)save_cun);
		/////////////
		//关中断
//		OS_ENTER_CRITICAL();
	
		if(!WriteDataToUDisk())
		{
			//失败
			lcm_clear_ram(LINE2);
			lcm_write_hz1(1,1,(LCD_ZM *)operate_cao);
			lcm_write_hz1(1,2,(LCD_ZM *)operate_zuo);
			lcm_write_hz1(1,3,(LCD_ZM *)fail_shi);
			lcm_write_hz1(1,4,(LCD_ZM *)fail_bai);
			for(i=0;i<100000;i++);
		}
				
		//开中断
//		OS_EXIT_CRITICAL();	  
	}
	
	Modify_LastUploadTime();
	
	/////////////
//	sl811write(0x0f,0x00);
	//at91_init();
	
	DisplayNormalUI();
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayAutoCode
//* Object              : 显示车牌号码
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : act_tcb
//*----------------------------------------------------------------------------
void DisplayAutoCode()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);

	StructPara *para = PARAMETER_BASE;
	unsigned char j=0,col=0,type=0;//type＝0汉字；＝1数字＝2字母
	unsigned char buf=para->AutoCode[0];
	unsigned short hz;
	while((buf!='\0')&&(col<20)&&(j<12))
	{
		if(buf>127)
		{//汉字处理
			hz = buf;
			hz = hz<<8;
			j++;
			buf=para->AutoCode[j];
			hz = hz+buf;
			if((col&1)==1)//如果是奇数
				col++;
			lcm_write_hz1(1,col/2,AutoCodeHZ2LCM(hz));
			col+=2;
			type = 0;
		}
		else
		{//字母或数字
			if(buf<60){
				lcm_write_ez(1,col,ASCII2LCM(buf));
				type = 1;
			}
			else{
				if(type==2)
					col--;
				lcm_write_zimu(1,col,ASCII2LCM(buf));
				col++;
				type = 2;
			}
			col++;
		}
		j++;
		buf = para->AutoCode[j];
	}

	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayDriverNumber
//* Object              : 显示司机代号
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : act_tcb
//*----------------------------------------------------------------------------
void DisplayDriverNumber()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);

//	StructPara *para = PARAMETER_BASE;
	unsigned int y=pTable.DriverCode;
	unsigned int x=y/10;
	unsigned char i=0;
	unsigned char buf[10];
	do{
		buf[i] = y-x*10;
		y=y/10;
		x=y/10;
		i++;
	}while(y!=0);
	
	i--;
	int k,m=0;
	for(k=i;k>=0;k--){
		lcm_write_ez(1,m,BCD2LCM(buf[k],0));
		m++;
	}


	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayDriverCode
//* Object              : 显示司机驾驶证号码
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : act_tcb
//*----------------------------------------------------------------------------
void DisplayDriverCode()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);

//	StructPara *para = PARAMETER_BASE;
	int i;
	unsigned char buf;
	for(i=0;i<18;i++)
	{
		buf=pTable.DriverLisenseCode[i];
		if(buf==0)
			break;
		lcm_write_ez(1,i,ASCII2LCM(buf));
	}

	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : Displaywheel
//* Object              : 显示车轮系数
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : act_tcb
//*----------------------------------------------------------------------------
void Displaywheel()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);
	
	StructPara *para ;
	para = PARAMETER_BASE;
	unsigned int y=para->CHCO;
	unsigned int x=y/10;
	unsigned char i=0;
	unsigned char buf[10];
	do{
		buf[i] = y-x*10;
		y=y/10;
		x=y/10;
		i++;
	}while(y!=0);
	
	i--;
	int k,m=0;
	for(k=i;k>=0;k--){
		lcm_write_ez(1,i-k,BCD2LCM(buf[k],0));
		m++;
	}
	
	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayStatusPolarity
//* Object              : 显示状态极性
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : act_tcb
//*----------------------------------------------------------------------------
void DisplayStatusPolarity()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);
	
	StructPara *para = PARAMETER_BASE;
	unsigned short s=para->status_polarity;
	unsigned short z;
	int j;
	for(j=15;j>=0;j--)
	{
		z=1<<j;
		if((s & z)==0)
			lcm_write_ez(1,19-j,(LCD_ZM *)digital_0);
		else
			lcm_write_ez(1,19-j,(LCD_ZM *)digital_1);
	}
	
	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : DisplayProductVersion
//* Object              : 显示产品版本号
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      : none
//* 修改的全局变量      : act_tcb
//*----------------------------------------------------------------------------
void DisplayProductVersion()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);
	
	u_char col=0;
	lcm_write_ez(1,col,(LCD_ZM *)digital_0);
	lcm_write_ez(1,col+1,(LCD_ZM *)digital_6);
	lcm_write_ez(1,col+2,(LCD_ZM *)charater_point);
	lcm_write_ez(1,col+3,(LCD_ZM *)digital_0);
	lcm_write_ez(1,col+4,(LCD_ZM *)digital_8);
	lcm_write_ez(1,col+5,(LCD_ZM *)charater_point);
	lcm_write_ez(1,col+6,(LCD_ZM *)digital_2);
	lcm_write_ez(1,col+7,(LCD_ZM *)digital_8);
	lcm_write_ez(1,col+8,(LCD_ZM *)charater_point);
	lcm_write_ez(1,col+9,(LCD_ZM *)digital_1);
#if	guizhou
	lcm_write_ez(1,col+10,(LCD_ZM *)charater_xing);
#else
	lcm_write_ez(1,col+10,(LCD_ZM *)charater_point);
#endif
	
#if OpenDoorDeal//包含软件开关门处理
	lcm_write_ez(1,col+11,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+11,(LCD_ZM *)digital_0);
#endif
#if RTC8025//时钟芯片选择开关
	lcm_write_ez(1,col+12,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+12,(LCD_ZM *)digital_0);
#endif
#if GetSpeedStatusBy232     //通过串口和适配器通讯获取速度和全部状态开关
	lcm_write_ez(1,col+13,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+13,(LCD_ZM *)digital_0);
#endif

#if WATCH_DOG_EN          // 看门狗开关
	lcm_write_ez(1,col+14,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+14,(LCD_ZM *)digital_0);
#endif
#if RPM_EN			     //发动机转速开关
	lcm_write_ez(1,col+15,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+15,(LCD_ZM *)digital_0);
#endif
#if SectionAlarm_EN        //分路段报警开关
	lcm_write_ez(1,col+16,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+16,(LCD_ZM *)digital_0);
#endif
#if OpenDoorAlarm	     //开门行驶报警开关
	lcm_write_ez(1,col+17,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+17,(LCD_ZM *)digital_0);
#endif
#if Test        	  //测试工装
	lcm_write_ez(1,col+18,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+18,(LCD_ZM *)digital_0);
#endif
#if Status14    //超过8种状态
	lcm_write_ez(1,col+19,(LCD_ZM *)digital_1);
#else
	lcm_write_ez(1,col+19,(LCD_ZM *)digital_0);
#endif
	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : Get15MinuteSpeed
//* Object              : 15分钟内的时间和速度
//* Input Parameters    : 
//* Output Parameters   : 停车时间 时,分,速度(3*15)
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
int Get15MinAverageSpeed(PrintSpeed *speed)
{
 	OTDR record;
	OTDR_start last_start;
	OTDR_end last_end,last_end_temp;
	CLOCK *cl;
	char temp;
	u_int i,Nb,p;
	int TimeInterval;
	u_int TimeIntervalSum;
	u_int TimeLimit;
	u_int curPointer;
	u_char buf[2];
	StructPT spt;
	DateTime BigTime,SmallTime,ti;	
	u_char StartTimeBuf[6];
	u_char StopTimeBuf[6];
	int offset,addup_offset=0;
	int j;
	
	//置初值
	TimeIntervalSum = 0;
	spt = pTable.RunRecord360h;
	curPointer = pTable.RunRecord360h.CurPoint;
	TimeLimit = 15;
	j = 15;	
	for(i = 0;i < 15;i++)
	{
		speed[i].hour = 0;
		speed[i].minute = 0;
		speed[i].speed = 0;
		speed[i].DriverCode = 0;
	}
		
	last_start.dt.type = 0;
	do
	{
		//取出当前记录是不正确的
		if(!GetOTDR(curPointer,&(record.start), &(record.end)))
		{
			addup_offset++;
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			if(curPointer == pTable.RunRecord360h.CurPoint)
				break;
			continue;
		}
		last_end = record.end;		
	
		if(last_start.dt.type==0xafaf)
			PrepareTime((u_char *)(&(last_start.dt.year)),&BigTime);
		else
		{
			PrepareTime((u_char *)(&(last_end.dt.year)),&BigTime);
			ti = BigTime;
			for(i=0;i<15;i++)
			{
				speed[i].hour = Char2BCD((ti.time)/(60));
				speed[i].minute = Char2BCD(ti.time%60);
				IncreaseTime(&ti, -1);
			}
		}
			
		//计算当前记录与上一条记录之间的时间差	
		PrepareTime((u_char *)(&(record.end.dt.year)),&SmallTime);
		TimeInterval = HaveTime(BigTime,SmallTime);
		if(TimeInterval < 0)
			break;
		TimeIntervalSum  += TimeInterval;
		if(TimeIntervalSum >=TimeLimit)
			break;
			
		j=j-TimeInterval;//panhui,2003,7,12	
		Nb = record.end.MinuteNb;
		TimeIntervalSum += Nb;
		i = 0;
		p = AddPointer(&spt, -sizeof(OTDR_end));
		while((j>=0)&&(Nb>0))
		{
			offset = -2;
			GetOTDRDataFromFlash((u_short *)p, offset,buf);
			speed[15-j].speed = buf[1];
			speed[15-j].DriverCode = record.end.driver.DriverCode;
			j--;
			Nb--;
			if(Nb<=0)
				break;
			if(j>=0){
				speed[15-j].speed = buf[0];
				speed[15-j].DriverCode = record.end.driver.DriverCode;
				j--;
				Nb--;
				if(Nb<=0)
					break;	
			}
			i += 2;
			p = AddPointer(&spt, -sizeof(OTDR_end)-i);
		}
			
		if(TimeIntervalSum >=TimeLimit)
			break;
		//修改指针
		offset = (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
		addup_offset += offset;
		offset = 0 - offset;
		curPointer = AddPointer(&spt, offset);
		spt.CurPoint = curPointer;
//		j -= TimeInterval;	
		last_start = record.start;		

	}while((j>=2)&&(pTable.RunRecord360h.CurPoint!=curPointer)&&(addup_offset<200));
	return(15-j);
			
}
//*----------------------------------------------------------------------------
//* Function Name       : Display15MinAverageSpeed
//* Object              : 显示停车时刻起前推15分钟平均速度
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void Display15MinAverageSpeed()
{//记录循环显示
	u_char i;
	LCD_ZM ** p;
	PrintSpeed *pt;
	pt=(PrintSpeed *)OTDR_Array;
	act_tcb.type = SHOW;
//	act_tcb.LineNumber = 15;
//myw 2003.7.14
		lcm_write_zimu(0,9,(LCD_ZM *)Letter_k);
		lcm_write_zimu(0,10,(LCD_ZM *)Letter_m);
		lcm_write_ez(0,13,(LCD_ZM *)charater_slash);
		lcm_write_zimu(0,12,(LCD_ZM *)Letter_h);
		
//
	if(act_tcb.CurLine == 0)//取记录
	{
		//显示“正在统计”
		lcm_clear_ram(LINE2);
		int i=0;
		p = (LCD_ZM **)being_stat;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
		act_tcb.LineNumber = Get15MinAverageSpeed(pt);
	}
	
	if(act_tcb.LineNumber == 0)
	{//显示“无记录”
		lcm_clear_ram(LINE2);
		int i=0;
		p = (LCD_ZM **)none;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
	}
	else{	 

		act_tcb.CurLine ++;
		if(act_tcb.CurLine>15)
			act_tcb.CurLine = 1;
			
		lcm_clear_ram(LINE2);
		u_char col=0;
		u_char row=1;
		//时间：时
		lcm_write_ez(row,col,BCD2LCM(pt[act_tcb.CurLine-1].hour,1));
		lcm_write_ez(row,col+1,BCD2LCM(pt[act_tcb.CurLine-1].hour,0));
		lcm_write_ez(row,col+2,(LCD_ZM *)digital_);
		//分
		lcm_write_ez(row,col+3,BCD2LCM(pt[act_tcb.CurLine-1].minute,1));
		lcm_write_ez(row,col+4,BCD2LCM(pt[act_tcb.CurLine-1].minute,0));
		//速度
		DisplayInteger(pt[act_tcb.CurLine-1].speed,row,col+8,3);
		
		/*lcm_write_zimu(row,8,(LCD_ZM *)Letter_k);//myw 2003.7.14
		lcm_write_zimu(row,9,(LCD_ZM *)Letter_m);
		lcm_write_ez(row,12,(LCD_ZM *)charater_slash);
		lcm_write_zimu(row,11,(LCD_ZM *)Letter_h);
		*/
//司机代号 
		DisplayInteger(pt[act_tcb.CurLine-1].DriverCode,row,19,0);
	}
	

}
//*----------------------------------------------------------------------------
//* Function Name       : Display2DayOTDR
//* Object              : 显示2个日历天超过3小时连续行驶记录
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void Display2DayOTDR()
{//记录循环显示
	LCD_ZM ** p;
	act_tcb.type = SHOW;
	if(act_tcb.CurLine == 0)//取记录
	{
		//显示“正在统计”
		lcm_clear_ram(LINE2);
		int i=0;
		p = (LCD_ZM **)being_stat;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
		act_tcb.LineNumber = GetOverTimeRecordIn2Days(OTDR_Array);
	}

	if(act_tcb.LineNumber == 0)
	{//显示“无记录”
		lcm_clear_ram(LINE2);
		int i=0;
		p = (LCD_ZM **)none;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
	}
	else{	 
		act_tcb.CurLine ++;
		if(act_tcb.CurLine>act_tcb.LineNumber)
			act_tcb.CurLine = 1;
		
		lcm_clear_ram(ALL);
		lcm_write_ez(0,0,BCD2LCM(Char2BCD(act_tcb.CurLine),1));
		lcm_write_ez(0,1,BCD2LCM(Char2BCD(act_tcb.CurLine),0));
		lcm_write_ez(0,2,(LCD_ZM *)digital_);
		u_char col=4;
		u_char row=0;
		//起始时间：年
		lcm_write_ez(row,col,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.year,1));
		lcm_write_ez(row,col+1,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.year,0));
		//月
		lcm_write_ez(row,col+3,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.month,1));
		lcm_write_ez(row,col+4,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.month,0));
		//日
		lcm_write_ez(row,col+6,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.day,1));
		lcm_write_ez(row,col+7,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.day,0));
		//时
		lcm_write_ez(row,col+9,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.hour,1));
		lcm_write_ez(row,col+10,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.hour,0));
		lcm_write_ez(row,col+11,(LCD_ZM *)digital_);
		//分
		lcm_write_ez(row,col+12,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.minute,1));
		lcm_write_ez(row,col+13,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.minute,0));

		row = 1;
		//结束时间：年
		lcm_write_ez(row,col,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.year,1));
		lcm_write_ez(row,col+1,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.year,0));
		//月
		lcm_write_ez(row,col+3,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.month,1));
		lcm_write_ez(row,col+4,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.month,0));
		//日
		lcm_write_ez(row,col+6,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.day,1));
		lcm_write_ez(row,col+7,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.day,0));
		//时
		lcm_write_ez(row,col+9,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.hour,1));
		lcm_write_ez(row,col+10,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.hour,0));
		lcm_write_ez(row,col+11,(LCD_ZM *)digital_);
		//分
		lcm_write_ez(row,col+12,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.minute,1));
		lcm_write_ez(row,col+13,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.minute,0));
			
		
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayTotalDistance
//* Object              : 显示总里程
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void DisplayTotalDistance()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);
	
	//累计总里程
	u_int Dis=ComputeDistance100m(pTable.TotalDistance);

	u_int x=Dis/10;
	u_int y=Dis;
	u_char i=0;
	u_char buf[10];

	do{
		buf[i] = y-x*10;
		y=y/10;
		x=y/10;
		i++;
	}while(y!=0);
	
	i--;
	u_char m=15;
	lcm_write_ez(1,m,BCD2LCM(buf[0],0));
	m--;
	lcm_write_ez(1,m,(LCD_ZM *)charater_point);
	m--;
	u_char k;
	for(k=1;k<=i;k++){
		lcm_write_ez(1,m,BCD2LCM(buf[k],0));
		m--;
	}
	
	lcm_write_hz1(1,8,(LCD_ZM *)km_gong);	
	lcm_write_hz1(1,9,(LCD_ZM *)distance_li);	

	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : SelectKeyHandler
//* Object              : 显示当前菜单结点
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void DisplayCurrentNode()
{
	LCD_ZM ** p1;
	LCD_ZM ** p2;

	lcm_clear_ram(ALL);

	p1=NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].content;
	if(lcd_tcb.NodeNb<NodeListTable[lcd_tcb.ListNb].NodeNumber-1)
		p2=NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb+1].content;
	else if(lcd_tcb.NodeNb == NodeListTable[lcd_tcb.ListNb].NodeNumber-1)
		p2=NodeListTable[lcd_tcb.ListNb].ListPt[0].content;
	else
		p2=NULL;
	int i=0;
	do{
		lcm_write_hz1(0,i,p1[i]);
		i++;
	}while(p1[i]!=NULL);
	
	//显示指示标志
	lcm_write_ez(0,18,(LCD_ZM *)charater_arrow1);
	lcm_write_ez(0,19,(LCD_ZM *)charater_arrow2);
	
	i=0;
	if(p2!=NULL)
	{
		do{
			lcm_write_hz1(1,i,p2[i]);
			i++;
		}while(p2[i]!=NULL);

	}
}
//*----------------------------------------------------------------------------
//* Function Name       : SelectKeyHandler
//* Object              : “选择”键处理程序
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void SelectKeyHandler()
{
	lcd_tcb.KeepTime = 0;

	switch(lcd_tcb.mode)
	{
		case Normal://
			last_lcd_tcb.mode = lcd_tcb.mode;
			last_lcd_tcb.ListNb = lcd_tcb.ListNb;
			last_lcd_tcb.NodeNb = lcd_tcb.NodeNb;
			last_lcd_tcb.KeepTime = lcd_tcb.KeepTime;

			lcd_tcb.mode = Node;
			lcd_tcb.ListNb = 0;
			lcd_tcb.NodeNb = 0;
			break;
		case Node://当前结点修改为同层下一个结点
		case BackLeaf:
		case ActLeaf:
			last_lcd_tcb.mode = lcd_tcb.mode;
			last_lcd_tcb.ListNb = lcd_tcb.ListNb;
			last_lcd_tcb.NodeNb = lcd_tcb.NodeNb;
			last_lcd_tcb.KeepTime = lcd_tcb.KeepTime;

			if(lcd_tcb.NodeNb<NodeListTable[lcd_tcb.ListNb].NodeNumber-1)
				lcd_tcb.NodeNb++;
			else if(lcd_tcb.NodeNb==NodeListTable[lcd_tcb.ListNb].NodeNumber-1)
				lcd_tcb.NodeNb = 0;
			lcd_tcb.mode = Node;
			if(NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].ChildrenList == -1)
				lcd_tcb.mode = ActLeaf;
			if((lcd_tcb.mode == ActLeaf)
				&&(NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].handler == NULL))
				lcd_tcb.mode = BackLeaf;
			break;
		case Action:
			last_lcd_tcb.mode = lcd_tcb.mode;
			last_lcd_tcb.ListNb = lcd_tcb.ListNb;
			last_lcd_tcb.NodeNb = lcd_tcb.NodeNb;
			last_lcd_tcb.KeepTime = lcd_tcb.KeepTime;
			lcd_tcb.mode = ActLeaf;
			break;
		default:
			last_lcd_tcb.mode = lcd_tcb.mode;
			last_lcd_tcb.ListNb = lcd_tcb.ListNb;
			last_lcd_tcb.NodeNb = lcd_tcb.NodeNb;
			last_lcd_tcb.KeepTime = lcd_tcb.KeepTime;
			lcd_tcb.mode = Normal;
			break;
	}
	DisplayCurrentNode();
}
//*----------------------------------------------------------------------------
//* Function Name       : OKKeyHandler
//* Object              : “确认”键处理程序
//* Input Parameters    : none
//* Output Parameters   : none
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void OKKeyHandler()
{
	short TemplistNb;

	lcd_tcb.KeepTime = 0;
	switch(lcd_tcb.mode)
	{
		case Node://切换到第一个孩子结点
			last_lcd_tcb = lcd_tcb;
			lcd_tcb.ListNb = NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].ChildrenList;	
			lcd_tcb.NodeNb = 0;
			lcd_tcb.mode = Node;
			if(NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].ChildrenList == -1)
				lcd_tcb.mode = ActLeaf;
			if((lcd_tcb.mode == ActLeaf)
				&&(NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].handler == NULL))
				lcd_tcb.mode = BackLeaf;
			DisplayCurrentNode();
			break;
		case BackLeaf://切换到父结点或通常模式
			last_lcd_tcb = lcd_tcb;

			if(NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].FatherList==-1)
			{
				DisplayNormalUI();
			}
			else{
				lcd_tcb.mode = Node;
				TemplistNb = lcd_tcb.ListNb;
				lcd_tcb.ListNb = NodeListTable[TemplistNb].ListPt[lcd_tcb.NodeNb].FatherList;
				lcd_tcb.NodeNb = NodeListTable[TemplistNb].ListPt[lcd_tcb.NodeNb].FatherNB;
				DisplayCurrentNode();
			}
			break;
		case ActLeaf://切换到动作模式
			last_lcd_tcb = lcd_tcb;
			
			lcd_tcb.mode = Action;
			act_tcb.IfActionEnd = 0;
			act_tcb.CurLine = 0;
			NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].handler();
			break;
		case Action:
			if(act_tcb.type == SHOW)
				NodeListTable[lcd_tcb.ListNb].ListPt[lcd_tcb.NodeNb].handler();
			else if(act_tcb.IfActionEnd)
			{
				last_lcd_tcb = lcd_tcb;
				lcd_tcb.mode = Normal;
				DisplayNormalUI();
			}
			break;
		default  : 
			last_lcd_tcb = lcd_tcb;
			lcd_tcb.mode = Normal;
			DisplayNormalUI();
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : ASCII2LCM
//* Object              : ASCII码转换为字模指针
//* Input Parameters    : data——待转换的字母
//* Output Parameters   : 字模指针
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
LCD_ZM *ASCII2LCM(unsigned char data)
{
	LCD_ZM *ret;
	
	switch(data)
	{
		case 48:
			ret=(LCD_ZM *)digital_0;break;
		case 49:
			ret=(LCD_ZM *)digital_1;break;
		case 50:
			ret=(LCD_ZM *)digital_2;break;
		case 51:
			ret=(LCD_ZM *)digital_3;break;
		case 52:
			ret=(LCD_ZM *)digital_4;break;
		case 53:
			ret=(LCD_ZM *)digital_5;break;
		case 54:
			ret=(LCD_ZM *)digital_6;break;
		case 55:
			ret=(LCD_ZM *)digital_7;break;
		case 56:
			ret=(LCD_ZM *)digital_8;break;
		case 57:
			ret=(LCD_ZM *)digital_9;break;
		
		case 65:
			ret=(LCD_ZM *)Letter_A;break;
		case 66:
			ret=(LCD_ZM *)Letter_B;break;
		case 67:
			ret=(LCD_ZM *)Letter_C;break;
		case 68:
			ret=(LCD_ZM *)Letter_D;break;
		case 69:
			ret=(LCD_ZM *)Letter_E;break;
		case 70:
			ret=(LCD_ZM *)Letter_F;break;
		case 71:
			ret=(LCD_ZM *)Letter_G;break;
		case 72:
			ret=(LCD_ZM *)Letter_H;break;
		case 73:
			ret=(LCD_ZM *)Letter_I;break;
		case 74:
			ret=(LCD_ZM *)Letter_J;break;
		case 75:
			ret=(LCD_ZM *)Letter_K;break;
		case 76:
			ret=(LCD_ZM *)Letter_L;break;
		case 77:
			ret=(LCD_ZM *)Letter_M;break;
		case 78:
			ret=(LCD_ZM *)Letter_N;break;
		case 79:
			ret=(LCD_ZM *)Letter_O;break;
		case 80:
			ret=(LCD_ZM *)Letter_P;break;
		case 81:
			ret=(LCD_ZM *)Letter_Q;break;
		case 82:
			ret=(LCD_ZM *)Letter_R;break;
		case 83:
			ret=(LCD_ZM *)Letter_S;break;
		case 84:
			ret=(LCD_ZM *)Letter_T;break;
		case 85:
			ret=(LCD_ZM *)Letter_U;break;
		case 86:
			ret=(LCD_ZM *)Letter_V;break;
		case 87:
			ret=(LCD_ZM *)Letter_W;break;
		case 88:
			ret=(LCD_ZM *)Letter_X;break;
		case 89:
			ret=(LCD_ZM *)Letter_Y;break;
		case 90:
			ret=(LCD_ZM *)Letter_Z;break;
			
		case 97:
			ret=(LCD_ZM *)Letter_a;break;
		case 98:
			ret=(LCD_ZM *)Letter_b;break;
		case 99:
			ret=(LCD_ZM *)Letter_c;break;
		case 100:
			ret=(LCD_ZM *)Letter_d;break;
		case 101:
			ret=(LCD_ZM *)Letter_e;break;
		case 102:
			ret=(LCD_ZM *)Letter_f;break;
		case 103:
			ret=(LCD_ZM *)Letter_g;break;
		case 104:
			ret=(LCD_ZM *)Letter_h;break;
		case 105:
			ret=(LCD_ZM *)Letter_i;break;
		case 106:
			ret=(LCD_ZM *)Letter_j;break;
		case 107:
			ret=(LCD_ZM *)Letter_k;break;
		case 108:
			ret=(LCD_ZM *)Letter_l;break;
		case 109:
			ret=(LCD_ZM *)Letter_m;break;
		case 110:
			ret=(LCD_ZM *)Letter_n;break;
		case 111:
			ret=(LCD_ZM *)Letter_o;break;
		case 112:
			ret=(LCD_ZM *)Letter_p;break;
		case 113:
			ret=(LCD_ZM *)Letter_q;break;
		case 114:
			ret=(LCD_ZM *)Letter_r;break;
		case 115:
			ret=(LCD_ZM *)Letter_s;break;
		case 116:
			ret=(LCD_ZM *)Letter_t;break;
		case 117:
			ret=(LCD_ZM *)Letter_u;break;
		case 118:
			ret=(LCD_ZM *)Letter_v;break;
		case 119:
			ret=(LCD_ZM *)Letter_w;break;
		case 120:
			ret=(LCD_ZM *)Letter_x;break;
		case 121:
			ret=(LCD_ZM *)Letter_y;break;
		case 122:
			ret=(LCD_ZM *)Letter_z;break;
		default:
			ret=(LCD_ZM *)space;
	}
	return ret;
}
//*----------------------------------------------------------------------------
//* Function Name       : AutoCodeHZ2LCM
//* Object              : 车牌号中的汉字转换为字模指针
//* Input Parameters    : data——待转换的汉字
//* Output Parameters   : 字模指针
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
LCD_ZM *AutoCodeHZ2LCM(unsigned short data)
{
	LCD_ZM *ret;


	switch(data)
	{
		case 0xbea9:
			ret = (LCD_ZM *)ch_jing1;//{"京"}
			break; 
		case 0xbba6:
			ret = (LCD_ZM *)ch_hu;//{"沪"} 
			break; 
		case 0xbdf2:
			ret = (LCD_ZM *)ch_jin1;//{"津"} 
			break; 
		case 0xcbd5:
			ret = (LCD_ZM *)ch_su;//{"苏"} 
			break; 
		case 0xcdee:
			ret = (LCD_ZM *)ch_wan;//{"皖"} 
			break; 
		case 0xb8d3:
			ret = (LCD_ZM *)ch_gan4;//{"赣"} 
			break; 
		case 0xc3f6:
			ret = (LCD_ZM *)ch_min;//{"闽"} 
			break; 
		case 0xc2b3:
			ret = (LCD_ZM *)ch_lu;//{"鲁"} 
			break; 
		case 0xd5e3:
			ret = (LCD_ZM *)ch_zhe;//{"浙"}
			break; 
		case 0xbdfa:
			ret = (LCD_ZM *)ch_jin4;//{"晋"} 
			break; 
		case 0xbcbd:
			ret = (LCD_ZM *)ch_ji4;//{"冀"} 
			break; 
		case 0xd4a5:
			ret = (LCD_ZM *)ch_yu4;//{"豫"} 
			break; 
		case 0xc3c9:
			ret = (LCD_ZM *)ch_meng;//{"蒙"} 
			break; 
		case 0xd0c2:
			ret = (LCD_ZM *)ch_xin;//{"新"} 
			break; 
		case 0xc4fe:
			ret = (LCD_ZM *)ch_ning;//{"宁"} 
			break; 
		case 0xc1c9:
			ret = (LCD_ZM *)ch_liao;//{"辽"} 
			break; 
		case 0xbada:
			ret = (LCD_ZM *)ch_hei;//{"黑"} 
			break; 
		case 0xbcaa:
			ret = (LCD_ZM *)ch_ji2;//{"吉"}
			break; 
		case 0xcfe6:
			ret = (LCD_ZM *)ch_xiang;//{"湘"} 
			break; 
		case 0xb6f5:
			ret = (LCD_ZM *)ch_e;//{"鄂"} 
			break; 
		case 0xb9f0:
			ret = (LCD_ZM *)ch_gui;//{"桂"} 
			break; 
		case 0xd4c1:
			ret = (LCD_ZM *)ch_yue;//{"粤"} 
			break; 
		case 0xc7ed:
			ret = (LCD_ZM *)ch_qiong;//{"琼"} 
			break; 
		case 0xb2d8:
			ret = (LCD_ZM *)ch_zang;//{"藏"} 
			break; 
		case 0xc9c2:
			ret = (LCD_ZM *)ch_shan;//{"陕"} 
			break; 
		case 0xb8ca:
			ret = (LCD_ZM *)ch_gan1;//{"甘"} 
			break; 
		case 0xc7e0:
			ret = (LCD_ZM *)ch_qing;//{"青"}
			break; 
		case 0xb4a8:
			ret = (LCD_ZM *)ch_chuan;//{"川"} 
			break; 
		case 0xc7ad:
			ret = (LCD_ZM *)ch_qian;//{"黔"} 
			break; 
		case 0xd4c6:
			ret = (LCD_ZM *)ch_yun;//{"云"} 
			break; 
		case 0xbaa3:
			ret = (LCD_ZM *)ch_hai3;//{"海"} 
			break; 
		case 0xcca8:
			ret = (LCD_ZM *)ch_tai;//{"台"} 
			break; 
		case 0xd3e5:
			ret = (LCD_ZM *)ch_yu2;//{"渝"} 
			break; 
		case 0xb8db:
			ret = (LCD_ZM *)ch_gang;//{"港"} 
			break; 
		case 0xb0c4:
			ret = (LCD_ZM *)ch_ao;//{"澳"} 
			break; 
		case 0xcab9:
			ret = (LCD_ZM *)ch_shi;//{"使"}
			break; 
		case 0xbcd7:
			ret = (LCD_ZM *)ch_jia;//{"甲"} 
			break; 
		case 0xd2d2:
			ret = (LCD_ZM *)ch_yi;//{"乙"} 
			break; 
		case 0xb1fb:
			ret = (LCD_ZM *)ch_bing;//{"丙"}
			break; 
		case 0xb6a1:
			ret = (LCD_ZM *)ch_ding;//{"丁"} 
			break; 
		case 0xceec:
			ret = (LCD_ZM *)ch_wu4;//{"戊"} 
			break; 
		case 0xbcba:
			ret = (LCD_ZM *)ch_ji3;//{"己"} 
			break; 
		case 0xb8fd:
			ret = (LCD_ZM *)ch_geng;//{"庚"} 
			break; 
		case 0xd0c1:
			ret = (LCD_ZM *)ch_xin1;//{"辛"} 
			break; 
		case 0xd7d3:
			ret = (LCD_ZM *)ch_zi;//{"子"}
			break; 
		case 0xb3f3:
			ret = (LCD_ZM *)ch_chou;//{"丑"} 
			break; 
		case 0xd2fa:
			ret = (LCD_ZM *)ch_yin;//{"寅"} 
			break; 
		case 0xc3ae:
			ret = (LCD_ZM *)ch_mou;//{"卯"} 
			break; 
		case 0xb3bd:
			ret = (LCD_ZM *)ch_chen;//{"辰"} 
			break; 
		case 0xcee7:
			ret = (LCD_ZM *)ch_wu3;//{"午"} 
			break; 
		case 0xceb4:
			ret = (LCD_ZM *)ch_wei;//{"未"} 
			break; 
		case 0xc9ea:
			ret = (LCD_ZM *)ch_shen;//{"申"} 
			break; 
		case 0xd3cf:
			ret = (LCD_ZM *)ch_you;//{"酉"} 
			break; 
		case 0xbaa5:
			ret = (LCD_ZM *)ch_hai4;//{"亥"}
			break; 
		case 0xc8c9:
			ret = (LCD_ZM *)ch_ren;//{"壬"} 
			break; 
		case 0xbeaf:
			ret = (LCD_ZM *)ch_jing3;//{"警"} 
			break; 
		case 0xb9f3:
			ret = (LCD_ZM *)ch_gui4;//{"贵"}
			break;
		case 0xc1ec:
			ret = (LCD_ZM *)ch_ling;//{"领"}
			break;
		case 0xd1a7:
			ret = (LCD_ZM *)ch_xue;//{"学"}
			break;
		case 0xcad4:
			ret = (LCD_ZM *)ch_shi_try;//{"试"}
			break;
		case 0xbeb3:
			ret = (LCD_ZM *)ch_jing;//{"境"}
			break;
		default:
			ret = (LCD_ZM *)space;
	}
	return ret;
}
//*----------------------------------------------------------------------------
//* Function Name       : BCD2LCM
//* Object              : BCD格式的数据转换为字模指针
//* Input Parameters    : data——待转换的BCD格式数据
//*                       type——0：低四位；1：高四位
//* Output Parameters   : 字模指针
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
LCD_ZM *BCD2LCM(u_char data, u_char type)
{
	u_char temp;
	LCD_ZM *ret;
	if(type)
	{//高四位
		temp = (data & 0xf0) >> 4;
	}
	else
		temp = data & 0x0f;
	
	switch(temp)
	{
		case 0:
			ret = (LCD_ZM *)digital_0;break;
		case 1:
			ret = (LCD_ZM *)digital_1;break;
		case 2:
			ret = (LCD_ZM *)digital_2;break;
		case 3:
			ret = (LCD_ZM *)digital_3;break;
		case 4:
			ret = (LCD_ZM *)digital_4;break;
		case 5:
			ret = (LCD_ZM *)digital_5;break;
		case 6:
			ret = (LCD_ZM *)digital_6;break;
		case 7:
			ret = (LCD_ZM *)digital_7;break;
		case 8:
			ret = (LCD_ZM *)digital_8;break;
		case 9:
			ret = (LCD_ZM *)digital_9;break;
		default:
			ret = 0;break;
	}
	return ret;
} 
//*----------------------------------------------------------------------------
//* Function Name       : DisplayInteger
//* Object              : 显示整数
//* Input Parameters    : row——行数
//*                       end_column——最后一位的列数
//*                       Integer——待显示的数字
//*                       len——显示数据的最长长度
//* Output Parameters   : none
//* Functions called    : 
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void DisplayInteger(u_int integer,u_char row,u_char end_column,u_char len)
{
	u_int x=integer/10;
	u_int y=integer;
	u_char i=0;
	u_char buf;
	u_char m=end_column;

	do{
		buf = y-x*10;
		lcm_write_ez(row,m,BCD2LCM(buf,0));
		y=y/10;
		x=y/10;
		i++;
		m--;
	}while(y!=0);
	
	int k;
	for(k=i;k<len;k++){
		lcm_write_ez(row,m,(LCD_ZM *)space);
		m--;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayFloat
//* Object              : 显示一位小数的浮点数
//* Input Parameters    : row——行数
//*                       end_column——最后一位的列数
//*                       Float——待显示的数字
//*                       len——显示数据的最长长度
//* Output Parameters   : none
//* Functions called    : 
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void DisplayFloat(u_int Float,u_char row,u_char end_column,u_char len)
{
	u_int x=Float/10;
	u_int y=Float;
	u_char i=0,m,k;
	u_char buf[10];

	do{
		buf[i] = y-x*10;
		y=y/10;
		x=y/10;
		i++;
	}while(y!=0);
	
	i--;
	m=end_column;
	lcm_write_ez(row,m,BCD2LCM(buf[0],0));
	m--;
	lcm_write_ez(row,m,(LCD_ZM *)charater_point);
	m--;
	for(k=1;k<=i;k++){
		lcm_write_ez(row,m,BCD2LCM(buf[k],0));
		m--;
	}
	for(k=i+1;k<len;k++){
		lcm_write_ez(row,m,(LCD_ZM *)space);
		m--;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayDateTime
//* Object              : 显示日期时间
//* Input Parameters    : row——行数
//*                       end_column——最后一位的列数
//*                       flag——数据显示标志从个位开始
//*                       依次（秒分时日月年）以1表示显示0表示不显示
//* Output Parameters   : none
//* Functions called    : 
//* 引用的全局变量      : none
//* 修改的全局变量      : none
//*----------------------------------------------------------------------------
void DisplayDateTime(u_char flag,u_char row,u_char column)
{
	u_char m=column;
	if(ClockVL)
		lcm_write_ez(row,column-1,(LCD_ZM *)charater_xing);
	else
		lcm_write_ez(row,column-1,(LCD_ZM *)space);

	if((flag & 0x20)!=0)
	{	//年
		lcm_write_ez(row,m,BCD2LCM(curTime.year,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.year,0));m++;
		lcm_write_ez(row,m,(LCD_ZM *)space);m++;
	}
	if((flag & 0x10)!=0)
	{	//月
		lcm_write_ez(row,m,BCD2LCM(curTime.month,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.month,0));m++;
		lcm_write_ez(row,m,(LCD_ZM *)space);m++;
	}
	if((flag & 0x08)!=0)
	{	//日
		lcm_write_ez(row,m,BCD2LCM(curTime.day,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.day,0));m++;
		lcm_write_ez(row,m,(LCD_ZM *)space);m++;
	}
	if((flag & 0x04)!=0)
	{	//时
		lcm_write_ez(row,m,BCD2LCM(curTime.hour,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.hour,0));m++;
	}
	if((flag & 0x02)!=0)
	{	//分
		lcm_write_ez(row,m,(LCD_ZM *)digital_);m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.minute,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.minute,0));m++;
	}
	if((flag & 0x01)!=0)
	{	//秒
		lcm_write_ez(row,m,(LCD_ZM *)digital_);m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.second,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.second,0));m++;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplaySpeedDimensoin
//* Object              : 显示常用界面：时间和速度
//* Input Parameters    : row——行
//*                       column——列
//*                       hz_pt——待写汉字字模数组指针
//* Output Parameters   : none
//* Functions called    : lcm_write_command，lcm_control，lcm_write_data
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void DisplaySpeedDimensoin()
{
	lcm_write_ez(1,9,(LCD_ZM *)charater_slash);
	lcm_write_zimu(1,6,(LCD_ZM *)Letter_k);
	lcm_write_zimu(1,7,(LCD_ZM *)Letter_m);
//	lcm_write_zimu(1,8,(LCD_ZM *)Letter_h);
	write_h();
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayNormalUI
//* Object              : 显示常用界面：时间和速度
//* Input Parameters    : row——行
//*                       column——列
//*                       hz_pt——待写汉字字模数组指针
//* Output Parameters   : none
//* Functions called    : lcm_write_command，lcm_control，lcm_write_data
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void DisplayNormalUI()
{
	last_lcd_tcb.mode = lcd_tcb.mode;
	last_lcd_tcb.ListNb = lcd_tcb.ListNb;
	last_lcd_tcb.NodeNb = lcd_tcb.NodeNb;
	last_lcd_tcb.KeepTime = lcd_tcb.KeepTime;
	
	if((PIO_PDSR & USB_S_VCC) == USB_S_VCC)
	{
		 if(last_lcd_tcb.mode != USBComm)
		 {
	 	
		 	lcd_tcb.mode = USBComm;
			lcd_tcb.KeepTime = 0;
			lcd_tcb.ListNb = 0;
			lcd_tcb.NodeNb = 0;
			lcm_clear_ram(ALL);
//			lcm_write_zimu(1,4,(LCD_ZM *)Letter_U);
//			lcm_write_zimu(1,5,(LCD_ZM *)Letter_S);
//			lcm_write_zimu(1,6,(LCD_ZM *)Letter_B);
			lcm_write_hz1(1,4,(LCD_ZM *)comm_tong);	
			lcm_write_hz1(1,5,(LCD_ZM *)comm_xun);	
		 }
		 return;
	}
	if((LastPowerOn == 0)&&(PowerOn == 1))
		lcm_initialize();
	else
		lcm_reset();
	
	if(last_lcd_tcb.mode != Normal)
	{
		lcm_clear_ram(ALL);
		lcd_tcb.mode = Normal;
		lcd_tcb.KeepTime = 0;
		lcd_tcb.ListNb = 0;
		lcd_tcb.NodeNb = 0;
	}
	
	if((CardIn == 2)&&(CurSpeed == 0))
		DisplayInteger(PartitionTable_BASE->DriverCode,0,19,8);
	else
//		DisplayDateTime(0x07,0,12);
		DisplayAlarm();
	if(CardIn)
	{
		lcm_write_ez(0,10,(LCD_ZM *)ch_sub);
		lcm_write_ez(0,9,(LCD_ZM *)ch_bracket);
	}
	else
	{
		lcm_write_ez(0,10,(LCD_ZM *)space);
		lcm_write_ez(0, 9,(LCD_ZM *)space);
	}
	if(pTable.InOSAlarmCycle)
		DisplayInteger(RoadNb,0,8,0);
	else
		lcm_write_ez(0,8,(LCD_ZM *)space);
	
	//速度
//	DisplayInteger(CurSpeed,1,2,3);
	DisplaySpeed((u_char)CurSpeed);
	DisplaySpeedDimensoin();
//	DisplayInteger(ClockType,0,8,3);

/*	//状态
	u_int z;
	int j;
	for(j=8;j>=0;j--)
	{
		z=1<<j;
		if((STATUS & z)==0)
			lcm_write_ez(1,19-j,(LCD_ZM *)digital_0);
		else
			lcm_write_ez(1,19-j,(LCD_ZM *)digital_1);
	}	
*/
#if RPM_EN	
	DisplayInteger(CurEngine,1,19,5);
#else
	if((AlarmFlag & 0x0c)!=0)
	{//显示“连续驾驶”	
		lcm_write_hz1(1,6,(LCD_ZM *)lian);
		lcm_write_hz1(1,7,(LCD_ZM *)xu);
		lcm_write_hz1(1,8,(LCD_ZM *)driver_jia);
		lcm_write_hz1(1,9,(LCD_ZM *)driver_shi);
	}
	else
	{
		//里程
/*		DisplayInteger(LastPN[3],1,18,2);
		DisplayInteger(LastPN[2],1,16,2);
		DisplayInteger(LastPN[1],1,14,2);
		DisplayInteger(LastPN[0],1,12,2);*/
		DisplayFloat(Distance,1,16,5);
		lcm_write_zimu(1,15,(LCD_ZM *)Letter_k);
		lcm_write_zimu(1,16,(LCD_ZM *)Letter_m);
	}
#endif
/*
DisplayInteger(LastPN[3],1,18,2);
DisplayInteger(LastPN[2],1,16,2);
DisplayInteger(LastPN[1],1,14,2);
DisplayInteger(LastPN[0],1,12,2);
*/	 
	
} 

//*----------------------------------------------------------------------------
//* Function Name       : DisplayEraseDataFlash
//* Object              : 显示"正在更新"
//* Input Parameters    : 
//* Output Parameters   : none
//* Functions called    : 
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------
void DisplayEraseDataFlash()
{
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)being_zheng);	
	lcm_write_hz1(1,2,(LCD_ZM *)being_zai);	
	lcm_write_hz1(1,3,(LCD_ZM *)update_geng);	
	lcm_write_hz1(1,4,(LCD_ZM *)update_xin);
	lcd_tcb.mode = Normal;
}
void DisplayOK()
{
	LCD_ZM **p;
	int i=0;
	lcm_clear_ram(ALL);
		p = (LCD_ZM **)working_ok;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);

}
void DisplayError()
{
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)error_gu);	
	lcm_write_hz1(1,2,(LCD_ZM *)error_zhang);	
}
void DisplayClockError()
{
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)time_shi);	
	lcm_write_hz1(1,2,(LCD_ZM *)time_jian);	
	lcm_write_hz1(1,3,(LCD_ZM *)error_cuo);	
	lcm_write_hz1(1,4,(LCD_ZM *)error_wu);	
}
void Display_Scan_Udisk()
{
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)scan_sao);
	lcm_write_hz1(1,2,(LCD_ZM *)scan_miao);
	lcm_write_hz1(1,3,(LCD_ZM *)udisk_u);
	lcm_write_hz1(1,4,(LCD_ZM *)udisk_pan);
}

void Display_Save()
{
	lcm_clear_ram(ALL);
	lcm_write_hz1(0,0,(LCD_ZM *)being_zheng);
	lcm_write_hz1(0,1,(LCD_ZM *)being_zai);
	lcm_write_hz1(0,2,(LCD_ZM *)save_bao);
	lcm_write_hz1(0,3,(LCD_ZM *)save_cun);
	////////////
	lcm_write_ez ( 0, 19, (LCD_ZM *)percent);
	lcm_write_ez ( 1, 0, (LCD_ZM *)process_bar1);
	lcm_write_ez ( 0, 18, (LCD_ZM *)digital_1);
	
	////////////
	
}

void Display_Fail()
{
	//lcm_clear_ram(LINE2);
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)operate_cao);
	lcm_write_hz1(1,2,(LCD_ZM *)operate_zuo);
	lcm_write_hz1(1,3,(LCD_ZM *)fail_shi);
	lcm_write_hz1(1,4,(LCD_ZM *)fail_bai);
}

void Display_udisk_full()
{
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)udisk_u);
	lcm_write_hz1(1,2,(LCD_ZM *)udisk_pan);
	lcm_write_hz1(1,3,(LCD_ZM *)udisk_kong);
	lcm_write_hz1(1,4,(LCD_ZM *)udisk_jian);
	lcm_write_hz1(1,5,(LCD_ZM *)udisk_yi);
	lcm_write_hz1(1,6,(LCD_ZM *)udisk_man);
}
void DisplayTestDoorFail()
{
	lcm_clear_ram(ALL);

	lcm_write_hz1(1,2,(LCD_ZM *)door_jian);
	lcm_write_hz1(1,3,(LCD_ZM *)door_ce);
	lcm_write_hz1(1,4,(LCD_ZM *)fail_shi);
	lcm_write_hz1(1,5,(LCD_ZM *)fail_bai);
}
void DisplayTestDoorSucc()
{
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,2,(LCD_ZM *)door_jian);
	lcm_write_hz1(1,3,(LCD_ZM *)door_ce);
	lcm_write_hz1(1,4,(LCD_ZM *)succ_cheng);
	lcm_write_hz1(1,5,(LCD_ZM *)succ_gong);
}
//add by panhui 2005-02-20, for 山东公安招标技术要求
void DisplayAlarm()
{
	if((AlarmFlag & 0x03)!=0)
	{//显示“超速”
		lcm_write_hz1(0,6,(LCD_ZM *)over_chao);
		lcm_write_hz1(0,7,(LCD_ZM *)speed_su);
		if(AlarmFlag & 0x01)
			DisplayInteger(PARAMETER_BASE->LowSpeedLimit,0,19,4);
		else if(AlarmFlag & 0x02)
			DisplayInteger(PARAMETER_BASE->HighSpeedLimit,0,19,4);
	}
	else
		DisplayDateTime(0x07,0,12);
}
#if OpenDoorDeal
void Display_TestDoor()
{
	//lcm_clear_ram(LINE2);
	lcm_clear_ram(ALL);
	lcm_write_hz1(1,1,(LCD_ZM *)door_jin);
	lcm_write_hz1(1,2,(LCD_ZM *)door_xing);
	lcm_write_hz1(1,3,(LCD_ZM *)door_zhong);
}

void Display_DoorNB()
{
	lcm_write_hz1(0,0,(LCD_ZM *)door_men);
	lcm_write_ez(0,2,(LCD_ZM *)digital_1);
	lcm_write_ez(0,3,(LCD_ZM *)space);
	lcm_write_hz1(1,0,(LCD_ZM *)door_men);
	lcm_write_ez(1,2,(LCD_ZM *)digital_2);
	lcm_write_ez(1,3,(LCD_ZM *)space);
}

void Display_Test_No_Door(u_char doorNB)
{
	u_char raw;
	if(doorNB==1)
		raw = 0;
	if(doorNB==2)
		raw = 1;
	lcm_write_hz1(raw,2,(LCD_ZM *)door_wei);
	lcm_write_hz1(raw,3,(LCD_ZM *)door_jian);
	lcm_write_hz1(raw,4,(LCD_ZM *)door_ce);
	lcm_write_hz1(raw,5,(LCD_ZM *)door_dao);
}

void Display_DoubleKeyLevel(u_char doorNB)
{
	u_char raw;
	if(doorNB==1)
		raw = 0;
	if(doorNB==2)
		raw = 1;
	lcm_write_hz1(raw,2,(LCD_ZM *)door_shuang);
	lcm_write_hz1(raw,3,(LCD_ZM *)door_jian_key);
	lcm_write_hz1(raw,4,(LCD_ZM *)door_dian);
	lcm_write_hz1(raw,5,(LCD_ZM *)door_ping);
}

void Display_SingleKeyLevel(u_char doorNB)
{
	u_char raw;
	if(doorNB==1)
		raw = 0;
	if(doorNB==2)
		raw = 1;
	lcm_write_hz1(raw,2,(LCD_ZM *)door_dan);
	lcm_write_hz1(raw,3,(LCD_ZM *)door_jian_key);
	lcm_write_hz1(raw,4,(LCD_ZM *)door_dian);
	lcm_write_hz1(raw,5,(LCD_ZM *)door_ping);
}

void Display_DoubleKeyPulse(u_char doorNB)
{
	u_char raw;
	if(doorNB==1)
		raw = 0;
	if(doorNB==2)
		raw = 1;
	lcm_write_hz1(raw,2,(LCD_ZM *)door_shuang);
	lcm_write_hz1(raw,3,(LCD_ZM *)door_jian_key);
	lcm_write_hz1(raw,4,(LCD_ZM *)door_mai);
	lcm_write_hz1(raw,5,(LCD_ZM *)door_chong);
}

void Display_SingleKeyPulse(u_char doorNB)
{
	u_char raw;
	if(doorNB==1)
		raw = 0;
	if(doorNB==2)
		raw = 1;
	lcm_write_hz1(raw,2,(LCD_ZM *)door_dan);
	lcm_write_hz1(raw,3,(LCD_ZM *)door_jian_key);
	lcm_write_hz1(raw,4,(LCD_ZM *)door_mai);
	lcm_write_hz1(raw,5,(LCD_ZM *)door_chong);
}

//*----------------------------------------------------------------------------
//* Function Name       : DoorType
//* Object              : 显示开关门状态
//* Input Parameters    : 
//* Output Parameters   : none
//* Functions called    : 
//* 引用的全局变量      :
//* 修改的全局变量      :
//*----------------------------------------------------------------------------

void DoorType()
{
	u_char raw;
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcm_clear_ram(LINE2);
	
	lcm_clear_ram(ALL);
	Display_DoorNB();
	
	switch(PARAMETER_BASE->Door1Type)
	{
		case 0x0b : Display_DoubleKeyLevel(1);  break;
		case 0x0a : Display_SingleKeyLevel(1);  break;
		case 0x09 : Display_SingleKeyLevel(1);  break;
		case 0x07 : Display_DoubleKeyPulse(1);  break;
		case 0x06 : Display_SingleKeyPulse(1);  break;
		case 0x05 : Display_SingleKeyPulse(1);  break;
		default   : Display_Test_No_Door(1);    break;
	}

	switch(PARAMETER_BASE->Door2Type)
	{
		case 0x0b : Display_DoubleKeyLevel(2);  break;
		case 0x0a : Display_SingleKeyLevel(2);  break;
		case 0x09 : Display_SingleKeyLevel(2);  break;
		case 0x07 : Display_DoubleKeyPulse(2);  break;
		case 0x06 : Display_SingleKeyPulse(2);  break;
		case 0x05 : Display_SingleKeyPulse(2);  break;
		default   : Display_Test_No_Door(2);    break;
	}

	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}
#endif