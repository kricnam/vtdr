//*----------------------------------------------------------------------------
//* File Name           : RS232.c
//* Object              : C program of RS232 communication
//*----------------------------------------------------------------------------

#include    "parts/r40807/reg_r40807.h"
#include    "parts/r40807/lib_r40807.h"
#include    "ibb3.h"
#include    "RS232.h"
#include    "sl811mheader.h"
#include    "lcd.h"
//*232通讯全局变量
UsartDesc *RS232;
u_char RSCmdrxBuf[CmdLength];
u_char RSCmdtxBuf[CmdLength];
u_char RSDatarxBuf[DataLength];
u_char LargeDataBuffer[28*1024];//[360*65];
u_int DataLengthReceived;
u_char CheckSum;
u_char SendCheckSum;
u_char Schedule_Result = 0;
u_char CloseUSART1Time = 0xff;
//u_char SetTimeFlag = 0;

extern u_char BCD2Char(u_char bcd);
extern u_char GetOverTimeRecordIn2Days(OTDR *record);

extern CLOCK curTime;
extern PartitionTable pTable;
extern u_int PulseTotalNumber;
extern u_char InRecordCycle;		//是否在记录数据过程中
extern u_char InFlashWriting;	//在FLASH写周期中
extern u_char FinishFlag;

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
int rs232_status_ready(u_int *status, u_int mask)
{
	int j;
	j=0;
	do
	{
		*status = at91_usart_get_status(RS232);
		j++;
	}while((((*status) & mask) != mask)&&(j<5000));
	
	if((j>=5000)&&(((*status)&mask)==0))
	{
		return FALSE;
	}
	return TRUE;
}

//*----------------------------------------------------------------------------
//* Function Name            : Int2BCD
//* Object              	 : 十进制数转换为BCD码
//* Input Parameters    	 : ch——待转换的十进制数
//* Output Parameters   	 : 转换后的BCD码
//* Global  Variable Quoted  : none
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void Int2BCD(u_int ch, u_char *buf)
{
	u_char d0,d1;
	u_int x;
	int i;
	x=ch;
	for(i=2;i>=0;i--)
	{
		d0=x%10;
		x=x/10;
		d1=x%10;
		x=x/10;
		buf[i]=(d1<<4)+d0;
		
	}

}
//*----------------------------------------------------------------------------
//* Function Name            : Char2BCD
//* Object              	 : 十进制数转换为BCD码
//* Input Parameters    	 : ch——待转换的十进制数
//* Output Parameters   	 : 转换后的BCD码
//* Global  Variable Quoted  : none
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
u_char Char2BCD(u_char ch)
{
	u_char bcd,d0,d1;
	d0 = ch%10;
	d1 = (ch/10)<<4;
	bcd = d1+d0;

	return(bcd);
}
//*----------------------------------------------------------------------------
//* 函数名：IncreaseTime
//* 功能：日期时间变量加一个给定的增量（该增量可正可负）
//* 输入：dt—— 日期时间变量
//*     ：inc——增量(以分钟计，可正可负)
//* 输出：已被修改的日期时间变量指针指向的数据（dt指向的数据）
//* 引用的全局变量: none
//* 修改的全局变量: none
//*----------------------------------------------------------------------------
void IncreaseTime(DateTime *dt, int inc)
{
	int t=dt->time+inc;
	int d=dt->day;
	int m=dt->month;
	int y=dt->year;
	do{
		if(t>=24*60)
		{
			d++;
			t=t-24*60;
			switch(m)
			{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				if(d>31){
					m++;
					d=d-31;
				}
				break;
			case 2:
				if((d>29)&&(y%4==0)){
					m++;
					d=d-29;
				}
				else if ((d>28)&&(y%4!=0)){
					m++;
					d=d-28;
				}
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				if(d>30){
					m++;
					d=d-30;
				}
				break;
			}
			if(m>12){
				y++;
				m=m-12;
			}
		}
		else if(t<0)
		{
			d--;
			t=t+24*60;
			switch(m)
			{
			case 2:
			case 4:
			case 6:
			case 8:
			case 9:
			case 11:
			case 1:
				if(d<1){
					m--;
					d=d+31;
				}
				break;
			case 3:
				if((d<1)&&(y%4==0)){
					m--;
					d=d+29;
				}
				else if ((d<1)&&(y%4!=0)){
					m--;
					d=d+28;
				}
				break;
			case 5:
			case 7:
			case 10:
			case 12:
				if(d<1){
					m--;
					d=d+30;
				}
				break;
			}
			if(m<1){
				y--;
				m=m+12;
			}
		}

	}while((t>=24*60)||(t<0));
	dt->time=t;
	dt->day=d;
	dt->month=m;
	dt->year=y;
}
//*----------------------------------------------------------------------------
//* 函数名：HaveTime
//* 功能：计算两时间之间相差多少分钟
//* 输入：bigtime——输入的两个时间中较大的一个，是函数的第一个参数,
//*     ：smalltime——输入的两个时间中较小的一个，是函数的第二个参数
//* 输出：正常情况(bigtime>smalltime)输出bigtime与smalltime之间相差的分钟数
//*     : 如果bigtime<smalltime,则输出"-1"
//* 引用的全局变量: none
//* 修改的全局变量: none
//*----------------------------------------------------------------------------
int HaveTime(DateTime bigtime,DateTime smalltime)
{
	int result = 0;

	if(bigtime.year < smalltime.year)
		return -1;
	else if(bigtime.year==smalltime.year)
	{
		if(bigtime.month < smalltime.month)
			return -1;
		else if(bigtime.month == smalltime.month)
		{
			if(bigtime.day < smalltime.day)
				return -1;
			else if(bigtime.day == smalltime.day)
			{
				if(bigtime.time < smalltime.time)
					return -1;
			}
		}
	}
	int timesmall = smalltime.time;
	smalltime.time = 0;
	while((bigtime.year != smalltime.year) || (bigtime.month != smalltime.month) || (bigtime.day != smalltime.day))
	{
		IncreaseTime(&smalltime,24*60);
		bigtime.time = bigtime.time+24*60;
	}

	result = bigtime.time-timesmall;
	return result;
}


//*----------------------------------------------------------------------------
//* Function Name            : PrepareTime
//* Object                   : 将BCD码形式的时间数据（年月日时分）转换成
//*                          : 十进制形式的数据
//* Input Parameters         : bcd_time——指向存放BCD码形式的时间数据的数组首地址的指针
//*                          : dt——指向存放十进制形式时间数据的结构的指针
//* Output Parameters        : none
//* Global  Variable Quoted  : none
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void PrepareTime(u_char *bcd_time,DateTime *dt)
{

	dt->year = BCD2Char(bcd_time[0]);
	dt->month = BCD2Char(bcd_time[1]);
	dt->day = BCD2Char(bcd_time[2]);
	dt->time = BCD2Char(bcd_time[3])*60+BCD2Char(bcd_time[4]);
}

//*----------------------------------------------------------------------------
//* Function Name            : WriteDataTxTime
//* Object                   : 向232接口写实时时间（用于上载实时时间、360小时累计行驶里程和2个日历天累计行驶里程）
//* Input Parameters         : cmd——命令字，用于区分是何命令，
//*                          : 若为0x02(上载实时时间),则上载“年月日时分秒”
//*                          : 若不是0x02，则上载“年月日时分”（对应360小时累计行驶里程和2个日历天累计行驶里程）
//* Output Parameters        : none
//* Global  Variable Quoted  : curTime——实时时间，本函数中将其写入232接口
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void WriteDataTxTime(u_char cmd)
{
	//发送实时时间数据块(BCD码)
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,(curTime.year));
	SendCheckSum = SendCheckSum^(curTime.year);
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,(curTime.month));
	SendCheckSum = SendCheckSum^(curTime.month);
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,(curTime.day));
	SendCheckSum = SendCheckSum^(curTime.day);
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,(curTime.hour));
	SendCheckSum = SendCheckSum^(curTime.hour);
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,(curTime.minute));
	SendCheckSum = SendCheckSum^(curTime.minute);
	if(cmd==0x02)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,(curTime.second));
		SendCheckSum = SendCheckSum^(curTime.second);
	}
}
//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_ALL_PARA
//* Object                   : 上载参数表的前256字节
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载驾驶员代码及对应的机动车驾驶证号码的应答帧
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
void UpLoad_ALL_PARA()
{
	u_short i;
	StructPara para;
	PartitionTable part;
	u_char *p;
	para = *PARAMETER_BASE;
	part = *PartitionTable_BASE;
	p = (u_char *)(&para);   

	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x14;
	RSCmdtxBuf[3] = 0x01;
	RSCmdtxBuf[4] = 0x00;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x14^0x01^0x00^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}

	para.time.year = curTime.year;
	para.time.month = curTime.month;
	para.time.day = curTime.day;
	para.time.hour = curTime.hour;
	para.time.minute = curTime.minute;
	para.time.second = curTime.second;
	
	para.DriverCode = part.DriverCode;
	for(i=0;i<20;i++)
		para.DriverLisenseCode[i] = part.DriverLisenseCode[i];
	
	for(i = 0; i < 256;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,*p);
		SendCheckSum = SendCheckSum^(*p);
		p++;
	}	

	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	
	Modify_LastUploadTime();
}


//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_DriverCode
//* Object                   : 上载驾驶员代码及对应的机动车驾驶证号码（常量），
//*                          : 待上载数据直接从记录仪数据存储器的参数表中取得，
//*                          : 命令字为0x01
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载驾驶员代码及对应的机动车驾驶证号码的应答帧
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
void UpLoad_DriverCode()
{
	u_int i;
	PartitionTable *para = PartitionTable_BASE;
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x01;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x15;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x01^0x00^0x15^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
		
	//发送12字节的上载数据块，其中1～3为驾驶员代码(由高位到低位)，
	//4～12为机动车驾驶证号码(BCD码)
	for(i = 0; i < 3;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,(u_char)((para->DriverCode) >> ((2-i)*8)));
		SendCheckSum = SendCheckSum^((u_char)((para->DriverCode) >> ((2-i)*8)));
	}
	for(i = 0; i < 18;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,((para->DriverLisenseCode)[i]));
		SendCheckSum = SendCheckSum^((para->DriverLisenseCode)[i]);
	}

	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	
	Modify_LastUploadTime();
}

//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_RealTime
//* Object                   : 上载实时时间，数据存在于全局变量curTime中，
//*                          : 命令字为0x02
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载实时时间的应答帧
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
void UpLoad_RealTime()
{
	u_int i;
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x02;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x06;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x02^0x00^0x06^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
		
	WriteDataTxTime(RSCmdrxBuf[2]);
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	
	Modify_LastUploadTime();
}

//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_TotalDistance360h
//* Object                   : 上载最近360小时内的累计行驶里程，需要将内存中
//*                          : 记录的数据挑选、判断、累加、计算后才能输出，
//*                          : 最近360小时的累计脉冲数存于临时变量Buf中，
//*                          : 计算后得出的累计里程存于临时变量disbuf中，
//*                          : 对累计里程的发送是先高字节后低字节，
//*                          : 命令字为0x03
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载最近360小时累计行驶里程的应答帧
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
/*void UpLoad_TotalDistance360h()
{
	int TimeIntervalSum,Nb;
	int i,j,TimeInterval;
	u_int curPointer;
	u_int TimeLimit;
	OTDR record;
	OTDR_start last_start;
	u_int Buf;
	StructPT spt;
	DateTime BigTime,SmallTime,BigTime1,SmallTime1;
	u_char disbuf[3];

	//关闭看门狗
#if WATCH_DOG_EN
	WD_OMR = 0x2340;
#endif	
	u_char StartTimeBuf[6];
	u_char StopTimeBuf[6];
	int offset;
	spt = pTable.RunRecord360h;

	//置初值
	Buf = 0;
	SendCheckSum=0;
	TimeIntervalSum = 0;
	curPointer = pTable.RunRecord360h.CurPoint;
	TimeLimit = 360*60;	
	last_start.dt.type = 0;
	
	do
	{
		//取出当前记录是不正确的
		if(!GetOTDR(curPointer,&(record.start), &(record.end)))
		{
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			continue;
		}		
		PrepareTime((u_char *)(&(record.end.dt.year)),&BigTime1);
		PrepareTime((u_char *)(&(record.start.dt.year)),&SmallTime1);
		TimeInterval = HaveTime(BigTime1,SmallTime1);
		if(TimeInterval<0)
		{
			//修改指针
			offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			last_start = record.start;	
			continue;
		}	

		if(last_start.dt.type==0xafaf)
			PrepareTime((u_char *)(&(last_start.dt.year)),&BigTime);
		else
			PrepareTime((u_char *)(&curTime),&BigTime);
		
		//计算当前记录与上一条记录之间的时间差	
		PrepareTime((u_char *)(&(record.end.dt.year)),&SmallTime);
		TimeInterval = HaveTime(BigTime,SmallTime);
		if(TimeInterval < 0)
		{
			//修改指针
			offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			last_start = record.start;	
			continue;
		}	
		TimeIntervalSum  += TimeInterval;
		if(TimeIntervalSum >=TimeLimit)
			break;
		
		Nb = record.end.MinuteNb;
		Buf +=  record.end.TotalDistance;
		TimeIntervalSum += Nb;
		
		if(TimeIntervalSum >=TimeLimit)
			break;
		//修改指针
		offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
		curPointer = AddPointer(&spt, offset);
		spt.CurPoint = curPointer;

		last_start = record.start;		
	}while((TimeIntervalSum <= 360*60)&&(pTable.RunRecord360h.CurPoint!=curPointer));
	
	//由脉冲数算出里程
	Buf = ComputeDistance100m(Buf);
	disbuf[0]=(u_char)(Buf>>16);
	disbuf[1]=(u_char)(Buf>>8);
	disbuf[2]=(u_char)(Buf);
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x03;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x08;
	RSCmdtxBuf[5] = 0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
		SendCheckSum = SendCheckSum^RSCmdtxBuf[i];
	}
	//发送上载数据块
	for(i = 0;i < 3;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,disbuf[i]);
		SendCheckSum = SendCheckSum^(disbuf[i]);
	}
	//发送数据读出时刻
	WriteDataTxTime(RSCmdrxBuf[2]);
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	
	Modify_LastUploadTime();
	
	//启动看门狗
#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
#endif
}*/

void UpLoad_TotalDistance360h()
{
	int offset,i,j,TimeInterval;
	StructPT spt;
	u_char NoDataFlag=0;                //数据区无记录标志
	u_char ReadOTDRFlag = 0;            //读记录标志
	u_char InOTDRFlag = 0;              //在一条疲劳驾驶记录中间标志
	u_char FilledNB = 0;                //60字节中已经填充的字节数
	u_char FirstRead = 1;               //读第一条记录标志
	u_int curPointer,Old_Pointer,Mid_Pointer;   //DataFlash中的指针
	OTDR cur_record,Last_Record;
	u_short hourNB = 0;                 //360小时计数器
	u_int CurRemainMinuteNB;            //当前剩余分钟数
	u_int temp;
	CLOCK StartTime;
	CLOCK current_time;
	DateTime BigTime,SmallTime;
	u_int rhr;
	u_int Buf;
	u_char disbuf[3];

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	

	spt = pTable.RunRecord360h;
	curPointer = pTable.RunRecord360h.CurPoint;
	Old_Pointer = curPointer;
	Buf = 0;
	SendCheckSum=0;

	//数据区没有合法记录
	if(GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(cur_record.start), &(cur_record.end)))
	{
		CurRemainMinuteNB = cur_record.end.MinuteNb;
		Buf += cur_record.end.TotalDistance;
		RefreshCurTime((CLOCK *)(&(cur_record.end.dt.year)),(CLOCK *)(&current_time));
		spt.CurPoint = Old_Pointer;
		
		do
		{
			//有读记录标志
			if(ReadOTDRFlag)
			{
				//没有成功读入疲劳记录，补最后65字节并结束
				if(!GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(Last_Record.start), &(Last_Record.end)))
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取剩余分钟数的数据装入内存
	//				Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
					hourNB++;
					break;
				}
				//成功读入一条疲劳驾驶记录
				else
				{
					Buf += Last_Record.end.TotalDistance;
					spt.CurPoint = Old_Pointer;
					FirstRead = 0;
					//计算两条记录之间的时间差
					PrepareTime((u_char *)(&(cur_record.start.dt.year)),&BigTime);
					PrepareTime((u_char *)(&(Last_Record.end.dt.year)),&SmallTime);
					TimeInterval = HaveTime(BigTime,SmallTime);
					//如果剩余分钟数加时间间隔大于60分钟
					if((TimeInterval+FilledNB)>=60)
					{
						ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
	//					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
	//					for(i=0;i<60;i++)
	//						Buf60Bytes[i] = 0;
						hourNB++;
						InOTDRFlag = 0;
						//更新剩余分钟数和时间
						CurRemainMinuteNB = Last_Record.end.MinuteNb;
						RefreshCurTime((CLOCK *)(&(Last_Record.end.dt.year)),&current_time);
						ReadOTDRFlag = 0;				
						continue;
					}
					//如果剩余分钟数加时间间隔小于60分钟
					else if((TimeInterval+FilledNB)<60)
					{
						if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)>=60)
						{
							ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -sizeof(OTDR_end));
							spt.CurPoint = Old_Pointer;
							offset = 0-temp;
	//						GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,Buf60Bytes);
	//						Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);			
	//						for(i=0;i<60;i++)
	//							Buf60Bytes[i] = 0;
							Old_Pointer = AddPointer(&spt, offset);
							Mid_Pointer = Old_Pointer;
							spt.CurPoint = Old_Pointer;
							hourNB++;
							InOTDRFlag = 1;
							//更新剩余分钟数和时间
							CurRemainMinuteNB = Last_Record.end.MinuteNb-temp;
							RefreshCurTime(&StartTime,&current_time);	
							ReadOTDRFlag = 0;
							continue;
						}
						else if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)<60)
						{
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
							spt.CurPoint = Old_Pointer;
	//						offset = 0-Last_Record.end.MinuteNb;
	//						j = 60-TimeInterval-FilledNB-Last_Record.end.MinuteNb;
	//						GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[j]));
							offset = 0-sizeof(OTDR_start)-Last_Record.end.MinuteNb;
							Old_Pointer = AddPointer(&spt, offset);
							spt.CurPoint = Old_Pointer;
							ReadOTDRFlag = 1;
							//获取当前记录的起始时间
							RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
							FilledNB = TimeInterval+FilledNB+Last_Record.end.MinuteNb;
							continue;
						}
					}
				}
			}
			//没有读记录标志
			if(!ReadOTDRFlag)
			{
				//当前剩余分钟数大于60
				if(CurRemainMinuteNB > 60)
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取本小时的60分钟数据存入Buf60Bytes
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
	//				GetOTDRDataFromFlash((u_short *)Old_Pointer,-60,Buf60Bytes);
					Old_Pointer = AddPointer(&spt, -60);
					Mid_Pointer = Old_Pointer;
					spt.CurPoint = Old_Pointer;
	//				Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
	//				for(i=0;i<60;i++)
	//					Buf60Bytes[i] = 0;
					hourNB++;
					InOTDRFlag = 1;
					//更新剩余分钟数和时间
					CurRemainMinuteNB -= 60;
					RefreshCurTime(&StartTime,&current_time);				
					continue;
				}
				//当前剩余分钟数小于60
				else
				{
					//置读记录标志
					ReadOTDRFlag = 1;
					if(!FirstRead)//获取当前记录的起始时间
						RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
					//补充部分数据入60字节缓冲区
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
	//				offset = 0-CurRemainMinuteNB;
	//				if(offset!=0)
	//				{
	//					temp = 60-CurRemainMinuteNB;
	//					GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[temp]));
	//				}
					Old_Pointer = AddPointer(&spt, offset-sizeof(OTDR_start));
					spt.CurPoint = Old_Pointer;
					FilledNB = CurRemainMinuteNB;
					continue;
				}
			}
		}while((hourNB<=360)&&(pTable.RunRecord360h.CurPoint!=Old_Pointer));
	}
	
	//由脉冲数算出里程
	Buf = ComputeDistance100m(Buf);
	Int2BCD(Buf, disbuf);
/*	disbuf[0]=(u_char)(Buf>>16);
	disbuf[1]=(u_char)(Buf>>8);
	disbuf[2]=(u_char)(Buf);*/
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x03;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x08;
	RSCmdtxBuf[5] = 0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
		SendCheckSum = SendCheckSum^RSCmdtxBuf[i];
	}
	//发送上载数据块
	for(i = 0;i < 3;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,disbuf[i]);
		SendCheckSum = SendCheckSum^(disbuf[i]);
	}
	//发送数据读出时刻
	WriteDataTxTime(RSCmdrxBuf[2]);
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	
	Modify_LastUploadTime();
	
	//启动看门狗
#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
#endif
}
//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_CHCO
//* Object                   : 上载车辆特征系数（常量），
//*                          : 待上载数据直接从记录仪数据存储器的参数表中取得，
//*                          : 命令字为0x04
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载车辆特征系数的应答帧
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
void UpLoad_CHCO()
{
	u_int i;
	StructPara *para = PARAMETER_BASE;
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x04;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x03;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x04^0x00^0x03^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
		
	//发送3字节的上载数据块
	for(i = 0; i < 3;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,(u_char)((para->CHCO) >> ((2-i)*8)));
		SendCheckSum = SendCheckSum^((u_char)((para->CHCO) >> ((2-i)*8)));
	}

	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	Modify_LastUploadTime();
}

//*----------------------------------------------------------------------------
//* Function Name            : ComputeTimeBeforeX
//* Object                   : 计算某一时刻之前一定时间的时刻值
//* Input Parameters         : ct——指向基础时间（年月如时分秒的结构）的指针
//*                          : dt——指向计算出的时间结果（年月如时分秒的结构）的指针
//*                          : timeinterval——时间间隔（以分钟计），
//*                          : 时间向前推为正,timeinterval不能为负
//* Output Parameters        : none
//* Global  Variable Quoted  : none
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void ComputeTimeBeforeX(CLOCK *ct,CLOCK *dt,u_int timeinterval)
{
	DateTime TimeBeforeXBuf;
		
	TimeBeforeXBuf.year = BCD2Char(ct->year);
	TimeBeforeXBuf.month = BCD2Char(ct->month);
	TimeBeforeXBuf.day = BCD2Char(ct->day);
	TimeBeforeXBuf.time = BCD2Char(ct->hour)*60+BCD2Char(ct->minute);
	
	int hour;
	hour = 0 - timeinterval;
	IncreaseTime(&TimeBeforeXBuf,hour);
	
	dt->year = Char2BCD(TimeBeforeXBuf.year);
	dt->month = Char2BCD(TimeBeforeXBuf.month);
	dt->day = Char2BCD(TimeBeforeXBuf.day);
	
	hour = (TimeBeforeXBuf.time)/60;
	dt->hour = Char2BCD((u_char)hour);
	dt->minute = Char2BCD(TimeBeforeXBuf.time-hour*60);
}

//*----------------------------------------------------------------------------
//* Function Name            : GetOneOTDRandModifyPointer
//* Object                   : 获取一条疲劳驾驶记录并修改指针
//* Input Parameters         : 
//* Output Parameters        : none
//* Global  Variable Quoted  : none
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
u_char GetOneOTDRandModifyPointer(u_int *p,u_int *old_p, OTDR_start *s, OTDR_end *e)
{
	int offset,TimeInterval;
	StructPT spt;
	DateTime BigTime,SmallTime;
	spt = pTable.RunRecord360h;
	spt.CurPoint = *p;
	
	do
	{
		//取出当前记录是不正确的
		if(!GetOTDR(*p,s,e))
		{
			offset = -1;
			*p = AddPointer(&spt, offset);
			spt.CurPoint = *p;
			continue;
		}
		*old_p = *p;
		offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+e->MinuteNb);
		*p = AddPointer(&spt, offset);
		spt.CurPoint = *p;
		//判断本条记录的起始时间是否大于结束时间
		PrepareTime((u_char *)(&(e->dt.year)),&BigTime);
		PrepareTime((u_char *)(&(s->dt.year)),&SmallTime);
		TimeInterval = HaveTime(BigTime,SmallTime);
		if(TimeInterval==-1)
			continue;
		else
			return 1;
	}while(pTable.RunRecord360h.CurPoint != *p);
	
	return 0;
}



void Write65ByteToSRAM(u_short hourNB,CLOCK *t,u_char *buf)
{
	u_char i;
	u_int start = 65*hourNB;
	//写每小时起始时间入缓冲区
	LargeDataBuffer[start] = t->year;
	LargeDataBuffer[start+1] = t->month;
	LargeDataBuffer[start+2] = t->day;
	LargeDataBuffer[start+3] = t->hour;
	LargeDataBuffer[start+4] = t->minute;
	
	//写60分钟数据入缓冲区
	for(i=0;i<60;i++)
		LargeDataBuffer[start+5+i] = buf[i];
}


void RefreshCurTime(CLOCK *s,CLOCK *d)
{
	d->year = s->year;
	d->month = s->month;
	d->day = s->day;
	d->hour = s->hour;
	d->minute = s->minute;
}


//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_Speed360h
//* Object                   : 上载最近360小时内的行驶速度，需要将内存中
//*                          : 记录的数据挑选、判断、累加、计算后才能输出，
//*                          : 最近360小时的行驶速度数据存于全局变量LargeDataBuffer中，
//*                          : 最先发送的数据块是速度数据对应的起始时间，
//*                          : 总数为360*60的360小时行驶速度数据被分为6块发送，
//*                          : 每块的长度是3607字节
//*                          : 对行驶速度的发送是先早时间后晚时间，
//*                          : 命令字为0x05
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机，
//*                          : RSCmdrxBuf[0]－RSCmdrxBuf[6]——命令接收寄存器,
//*                          : 用于在传送各页之间接收PC机的同步命令
//*                          : CheckSum——由PC机接收到的命令计算出的校验和
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 时间页赋值为上载最近360小时速度的应答帧，
//*                          : 数据页赋值为上载最近360小时速度的应答帧中加入页号
//*                          : RSCmdrxBuf[0]－RSCmdrxBuf[6]——命令接收寄存器,
//*                          : 每页接收到的内容都根据页号有所不同
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*                          : LargeDataBuffer——存放内存中取出的最近360小时速度数据
//*----------------------------------------------------------------------------
void UpLoad_Speed360h()
{
	int offset,i,j,TimeInterval;
	StructPT spt;
	u_char ReadOTDRFlag = 0;            //读记录标志
	u_char InOTDRFlag = 0;              //在一条疲劳驾驶记录中间标志
	u_char FilledNB = 0;                //60字节中已经填充的字节数
	u_char FirstRead = 1;               //读第一条记录标志
	u_int curPointer,Old_Pointer,Mid_Pointer;   //DataFlash中的指针
	OTDR cur_record,Last_Record;
	u_short hourNB = 0;                 //360小时计数器
	u_int CurRemainMinuteNB;            //当前剩余分钟数
	u_int temp;
	CLOCK StartTime;
	CLOCK current_time;
	u_char Buf60Bytes[60];              //60分钟数据缓冲区
	DateTime BigTime,SmallTime;
	u_int status232;
	u_int rhr;

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	

	spt = pTable.RunRecord360h;
	curPointer = pTable.RunRecord360h.CurPoint;
	Old_Pointer = curPointer;
	for(i=0;i<60;i++)
		Buf60Bytes[i] = 0;
	for(i=0;i<360*65;i++)
		LargeDataBuffer[i] = 0;

	//数据区没有合法记录
	if(GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(cur_record.start), &(cur_record.end)))
	{
		CurRemainMinuteNB = cur_record.end.MinuteNb;
		RefreshCurTime((CLOCK *)(&(cur_record.end.dt.year)),(CLOCK *)(&current_time));
		spt.CurPoint = Old_Pointer;
		
		do
		{
			//有读记录标志
			if(ReadOTDRFlag)
			{
				//没有成功读入疲劳记录，补最后65字节并结束
				if(!GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(Last_Record.start), &(Last_Record.end)))
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取剩余分钟数的数据装入内存
					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
					hourNB++;
					break;
				}
				//成功读入一条疲劳驾驶记录
				else
				{
					spt.CurPoint = Old_Pointer;
					FirstRead = 0;
					//计算两条记录之间的时间差
					PrepareTime((u_char *)(&(cur_record.start.dt.year)),&BigTime);
					PrepareTime((u_char *)(&(Last_Record.end.dt.year)),&SmallTime);
					TimeInterval = HaveTime(BigTime,SmallTime);
					//如果剩余分钟数加时间间隔大于60分钟
					if((TimeInterval+FilledNB)>=60)
					{
						ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
						Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
						for(i=0;i<60;i++)
							Buf60Bytes[i] = 0;
						hourNB++;
						InOTDRFlag = 0;
						//更新剩余分钟数和时间
						CurRemainMinuteNB = Last_Record.end.MinuteNb;
						RefreshCurTime((CLOCK *)(&(Last_Record.end.dt.year)),&current_time);
						ReadOTDRFlag = 0;				
						continue;
					}
					//如果剩余分钟数加时间间隔小于60分钟
					else if((TimeInterval+FilledNB)<60)
					{
						if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)>=60)
						{
							ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -sizeof(OTDR_end));
							spt.CurPoint = Old_Pointer;
							offset = 0-temp;
							GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,Buf60Bytes);
							Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);			
							for(i=0;i<60;i++)
								Buf60Bytes[i] = 0;
							Old_Pointer = AddPointer(&spt, offset);
							Mid_Pointer = Old_Pointer;
							spt.CurPoint = Old_Pointer;
							hourNB++;
							InOTDRFlag = 1;
							//更新剩余分钟数和时间
							CurRemainMinuteNB = Last_Record.end.MinuteNb-temp;
							RefreshCurTime(&StartTime,&current_time);	
							ReadOTDRFlag = 0;
							continue;
						}
						else if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)<60)
						{
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
							spt.CurPoint = Old_Pointer;
							offset = 0-Last_Record.end.MinuteNb;
							j = 60-TimeInterval-FilledNB-Last_Record.end.MinuteNb;
							GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[j]));
							offset = 0-sizeof(OTDR_start)-Last_Record.end.MinuteNb;
							Old_Pointer = AddPointer(&spt, offset);
							spt.CurPoint = Old_Pointer;
							ReadOTDRFlag = 1;
							//获取当前记录的起始时间
							RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
							FilledNB = TimeInterval+FilledNB+Last_Record.end.MinuteNb;
							continue;
						}
					}
				}
			}
			//没有读记录标志
			if(!ReadOTDRFlag)
			{
				//当前剩余分钟数大于60
				if(CurRemainMinuteNB > 60)
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取本小时的60分钟数据存入Buf60Bytes
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
					GetOTDRDataFromFlash((u_short *)Old_Pointer,-60,Buf60Bytes);
					Old_Pointer = AddPointer(&spt, -60);
					Mid_Pointer = Old_Pointer;
					spt.CurPoint = Old_Pointer;
					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
					for(i=0;i<60;i++)
						Buf60Bytes[i] = 0;
					hourNB++;
					InOTDRFlag = 1;
					//更新剩余分钟数和时间
					CurRemainMinuteNB -= 60;
					RefreshCurTime(&StartTime,&current_time);				
					continue;
				}
				//当前剩余分钟数小于60
				else
				{
					//置读记录标志
					ReadOTDRFlag = 1;
					if(!FirstRead)//获取当前记录的起始时间
						RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
					//补充部分数据入60字节缓冲区
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
					offset = 0-CurRemainMinuteNB;
					if(offset!=0)
					{
						temp = 60-CurRemainMinuteNB;
						GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[temp]));
					}
					Old_Pointer = AddPointer(&spt, offset-sizeof(OTDR_start));
					spt.CurPoint = Old_Pointer;
					FilledNB = CurRemainMinuteNB;
					continue;
				}
			}
		}while((hourNB<=360)&&(pTable.RunRecord360h.CurPoint!=Old_Pointer));
	}
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x05;
	RSCmdtxBuf[3] = (u_char)((hourNB*65)>>8);
	RSCmdtxBuf[4] = (u_char)(hourNB*65);
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x05^((u_char)((hourNB*65)>>8))^((u_char)(hourNB*65))^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	

	for(j=0;j<hourNB;j++)
	{
		//发送上载数据块（360h之前的时间）
		for(i = 0;i < 65;i++)
		{
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,LargeDataBuffer[j*65+i]);
			SendCheckSum = SendCheckSum^LargeDataBuffer[j*65+i];
		}
	}
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);

	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}
//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_AutoVIN
//* Object                   : 上载车辆VIN号、车牌号码、车牌分类，
//*                          : 待上载数据直接从记录仪数据存储器的参数表中取得，
//*                          : 命令字为0x06
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载车辆VIN号、车牌号码、车牌分类的应答帧
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
void UpLoad_AutoVIN()
{
	u_int i;
	StructPara *para = PARAMETER_BASE;
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x06;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x29;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x06^0x00^0x29^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	
	//发送17字节的上载数据块车辆VIN号
	for(i = 0; i < 17;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,((para->AutoVIN)[i]));
		SendCheckSum = SendCheckSum^((para->AutoVIN)[i]);
	}

	//发送12字节的上载数据块车牌号码
	for(i = 0; i < 12;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,((para->AutoCode)[i]));
		SendCheckSum = SendCheckSum^((para->AutoCode)[i]);
	}

	//发送12字节的上载数据块车牌分类
	for(i = 0; i < 12;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,((para->AutoSort)[i]));
		SendCheckSum = SendCheckSum^((para->AutoSort)[i]);
	}
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	Modify_LastUploadTime();
}



//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_DoubtPoint
//* Object                   : 上载事故疑点数据，10个疑点数据从
//*                          : 记录仪内存中的疑点数据区取出
//*                          : 命令字为0x07
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值为上载事故疑点的应答帧,
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或,
//*                          : LargeDataBuffer——使用第0到2059个字节存放
//*                          : 10个疑点数据
//*----------------------------------------------------------------------------
void UpLoad_DoubtPoint()
{
	u_int i,j;
	flash_word *source,*des;
	u_int STOPp;
	u_short temp_data;
	
	for(i=0;i<2060;i++)
		LargeDataBuffer[i]=0;
	
	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	
	//根据10个疑点的数据长度算出待传数据的起始地址（每个疑点长度为210字节）
	STOPp = pTable.DoubtPointData.CurPoint;
	//挑选出要传送的数据装入内存缓冲区
	for(i = 0;i < 10;i++)
	{
		STOPp -= 206;
		source = (flash_word *)STOPp;
		des = (flash_word *)(&(LargeDataBuffer[i*206]));
		for(j = 0;j < 103;j++)
		{
			des[j]=source[j];
		}
		//制动信号放在状态的最高位
		for(j = 3;j < 103;j++)
		{
			temp_data = (des[j])&0x8200;
			if(temp_data==0x0200)
				des[j]=((des[j])|0x8000)&0xfdff;
			if(temp_data==0x8000)
				des[j]=((des[j])|0x0200)&0x7fff;
		}
		STOPp -= 4;
		if(STOPp==pTable.DoubtPointData.BaseAddr)
			STOPp = pTable.DoubtPointData.EndAddr - 110 + 1;
	}
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x07;
	RSCmdtxBuf[3] = 0x08;
	RSCmdtxBuf[4] = 0x0c;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x07^0x08^0x0c^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
		
	//发送2060字节的疑点数据
	for(i = 0; i < 2060;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,LargeDataBuffer[i]);
		SendCheckSum = SendCheckSum^(LargeDataBuffer[i]);
	}
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	Modify_LastUploadTime();
	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}

//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_DistanceinTwoDays
//* Object                   : 上载最近最近2个日历天内的累计行驶里程，需要将内存中
//*                          : 记录的数据挑选、判断、累加、计算后才能输出，
//*                          : 最近2个日历天的累计里程存于临时变量Buf中，
//*                          : 对累计里程的发送是先高字节后低字节，
//*                          : 命令字为0x08
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载最近360小时累计行驶里程的应答帧
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
/*void UpLoad_DistanceinTwoDays()
{
	int offset,i,j,TimeInterval;
	StructPT spt;
	u_char ReadOTDRFlag = 0;            //读记录标志
	u_char InOTDRFlag = 0;              //在一条疲劳驾驶记录中间标志
	u_char FilledNB = 0;                //60字节中已经填充的字节数
	u_char FirstRead = 1;               //读第一条记录标志
	u_int curPointer,Old_Pointer,Mid_Pointer;   //DataFlash中的指针
	OTDR cur_record,Last_Record,temp_record;
	u_short hourNB = 0;                 //360小时计数器
	u_int CurRemainMinuteNB;            //当前剩余分钟数
	u_int temp;
	CLOCK StartTime;
	CLOCK current_time,temptime;
	DateTime BigTime,SmallTime;
	u_int rhr;
	u_int Buf;
	u_char disbuf[3];
	u_char HourLimit;

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	

	spt = pTable.RunRecord360h;
	curPointer = pTable.RunRecord360h.CurPoint;
	Old_Pointer = curPointer;
	Buf = 0;
	SendCheckSum=0;

	do
	{
		if(!GetOTDR(curPointer,&(temp_record.start), &(temp_record.end)))
		{
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			continue;
		}
		break;
	}while(pTable.RunRecord360h.CurPoint!=curPointer);
	
	temptime.year = temp_record.end.dt.year;
	temptime.month = temp_record.end.dt.month;
	temptime.day = temp_record.end.dt.day;
	temptime.hour = temp_record.end.dt.hour;
	temptime.minute = temp_record.end.dt.minute;
		

	//置初值
	HourLimit = BCD2Char(temptime.hour) + 24 + 1;	//数据区没有合法记录
	if(!GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(cur_record.start), &(cur_record.end)))
	{
		RS232UploadError();
		//启动看门狗
		#if WATCH_DOG_EN
	    WD_CR = 0xc071;
	    WD_OMR = 0x2343;
		#endif
		return;
	}
	CurRemainMinuteNB = cur_record.end.MinuteNb;
	Buf += cur_record.end.TotalDistance;
	RefreshCurTime((CLOCK *)(&(cur_record.end.dt.year)),(CLOCK *)(&current_time));
	spt.CurPoint = Old_Pointer;
	
	do
	{
		//有读记录标志
		if(ReadOTDRFlag)
		{
			//没有成功读入疲劳记录，补最后65字节并结束
			if(!GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(Last_Record.start), &(Last_Record.end)))
			{
				ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
				//获取剩余分钟数的数据装入内存
//				Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
				hourNB++;
				break;
			}
			//成功读入一条疲劳驾驶记录
			else
			{
				Buf += Last_Record.end.TotalDistance;
				spt.CurPoint = Old_Pointer;
				FirstRead = 0;
				//计算两条记录之间的时间差
				PrepareTime((u_char *)(&(cur_record.start.dt.year)),&BigTime);
				PrepareTime((u_char *)(&(Last_Record.end.dt.year)),&SmallTime);
				TimeInterval = HaveTime(BigTime,SmallTime);
				//如果剩余分钟数加时间间隔大于60分钟
				if((TimeInterval+FilledNB)>=60)
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
//					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
//					for(i=0;i<60;i++)
//						Buf60Bytes[i] = 0;
					hourNB++;
					InOTDRFlag = 0;
					//更新剩余分钟数和时间
					CurRemainMinuteNB = Last_Record.end.MinuteNb;
					RefreshCurTime((CLOCK *)(&(Last_Record.end.dt.year)),&current_time);
					ReadOTDRFlag = 0;				
					continue;
				}
				//如果剩余分钟数加时间间隔小于60分钟
				else if((TimeInterval+FilledNB)<60)
				{
					if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)>=60)
					{
						ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
						temp = 60-TimeInterval-FilledNB;
						Old_Pointer = AddPointer(&spt, -sizeof(OTDR_end));
						spt.CurPoint = Old_Pointer;
						offset = 0-temp;
//						GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,Buf60Bytes);
//						Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);			
//						for(i=0;i<60;i++)
//							Buf60Bytes[i] = 0;
						Old_Pointer = AddPointer(&spt, offset);
						Mid_Pointer = Old_Pointer;
						spt.CurPoint = Old_Pointer;
						hourNB++;
						InOTDRFlag = 1;
						//更新剩余分钟数和时间
						CurRemainMinuteNB = Last_Record.end.MinuteNb-temp;
						RefreshCurTime(&StartTime,&current_time);	
						ReadOTDRFlag = 0;
						continue;
					}
					else if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)<60)
					{
						temp = 60-TimeInterval-FilledNB;
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
//						offset = 0-Last_Record.end.MinuteNb;
//						j = 60-TimeInterval-FilledNB-Last_Record.end.MinuteNb;
//						GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[j]));
						offset = 0-sizeof(OTDR_start)-Last_Record.end.MinuteNb;
						Old_Pointer = AddPointer(&spt, offset);
						spt.CurPoint = Old_Pointer;
						ReadOTDRFlag = 1;
						//获取当前记录的起始时间
						RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
						FilledNB = TimeInterval+FilledNB+Last_Record.end.MinuteNb;
						continue;
					}
				}
			}
		}
		//没有读记录标志
		if(!ReadOTDRFlag)
		{
			//当前剩余分钟数大于60
			if(CurRemainMinuteNB > 60)
			{
				ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
				//获取本小时的60分钟数据存入Buf60Bytes
				if(!InOTDRFlag)
				{
					Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
					spt.CurPoint = Old_Pointer;
				}
//				GetOTDRDataFromFlash((u_short *)Old_Pointer,-60,Buf60Bytes);
				Old_Pointer = AddPointer(&spt, -60);
				Mid_Pointer = Old_Pointer;
				spt.CurPoint = Old_Pointer;
//				Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
//				for(i=0;i<60;i++)
//					Buf60Bytes[i] = 0;
				hourNB++;
				InOTDRFlag = 1;
				//更新剩余分钟数和时间
				CurRemainMinuteNB -= 60;
				RefreshCurTime(&StartTime,&current_time);				
				continue;
			}
			//当前剩余分钟数小于60
			else
			{
				//置读记录标志
				ReadOTDRFlag = 1;
				if(!FirstRead)//获取当前记录的起始时间
					RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
				//补充部分数据入60字节缓冲区
				if(!InOTDRFlag)
				{
					Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
					spt.CurPoint = Old_Pointer;
				}
//				offset = 0-CurRemainMinuteNB;
//				if(offset!=0)
//				{
//					temp = 60-CurRemainMinuteNB;
//					GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[temp]));
//				}
				Old_Pointer = AddPointer(&spt, offset-sizeof(OTDR_start));
				spt.CurPoint = Old_Pointer;
				FilledNB = CurRemainMinuteNB;
				continue;
			}
		}
	}while((hourNB<=HourLimit)&&(pTable.RunRecord360h.CurPoint!=Old_Pointer));
	
	//由脉冲数算出里程
	Buf = ComputeDistance100m(Buf);
	Int2BCD(Buf, disbuf);
//	disbuf[0]=(u_char)(Buf>>16);
//	disbuf[1]=(u_char)(Buf>>8);
//	disbuf[2]=(u_char)(Buf);
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x03;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x08;
	RSCmdtxBuf[5] = 0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
		SendCheckSum = SendCheckSum^RSCmdtxBuf[i];
	}
	//发送上载数据块
	for(i = 0;i < 3;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,disbuf[i]);
		SendCheckSum = SendCheckSum^(disbuf[i]);
	}
	//发送数据读出时刻
	WriteDataTxTime(RSCmdrxBuf[2]);
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	
	Modify_LastUploadTime();
	
	//启动看门狗
#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
#endif
}
*/
void UpLoad_DistanceinTwoDays()
{
	int i,j,TimeInterval,TimeIntervalSum,Nb;
	u_int TimeLimit;
	u_int curPointer;
	OTDR record,temp_record;
	OTDR_start last_start;
	u_int Buf;
	StructPT spt;
	DateTime BigTime,SmallTime,BigTime1,SmallTime1;
	CLOCK temptime;

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	
	int offset;
	spt = pTable.RunRecord360h;

	//置初值
	Buf = 0;
	TimeIntervalSum = 0;
	curPointer = pTable.RunRecord360h.CurPoint;	
	last_start.dt.type = 0;
	
	do
	{
		if(!GetOTDR(curPointer,&(temp_record.start), &(temp_record.end)))
		{
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			continue;
		}
		break;
	}while(pTable.RunRecord360h.CurPoint!=curPointer);
	
	temptime.year = temp_record.end.dt.year;
	temptime.month = temp_record.end.dt.month;
	temptime.day = temp_record.end.dt.day;
	temptime.hour = temp_record.end.dt.hour;
	temptime.minute = temp_record.end.dt.minute;
		

	//置初值
	TimeLimit = (BCD2Char(temptime.hour))*60+BCD2Char(temptime.minute)+24*60;	
	do
	{
		//取出当前记录是不正确的
		if(!GetOTDR(curPointer,&(record.start), &(record.end)))
		{
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			continue;
		}
		PrepareTime((u_char *)(&(record.end.dt.year)),&BigTime1);
		PrepareTime((u_char *)(&(record.start.dt.year)),&SmallTime1);
		TimeInterval = HaveTime(BigTime1,SmallTime1);
		if(TimeInterval<0)
		{
			//修改指针
			offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			last_start = record.start;	
			continue;
		}	
			

		if(last_start.dt.type==0xafaf)
			PrepareTime((u_char *)(&(last_start.dt.year)),&BigTime);
		else
			PrepareTime((u_char *)(&temptime),&BigTime);
		
		//计算当前记录与上一条记录之间的时间差	
		PrepareTime((u_char *)(&(record.end.dt.year)),&SmallTime);
		TimeInterval = HaveTime(BigTime,SmallTime);
		if(TimeInterval < 0)
		{
			//修改指针
			offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			last_start = record.start;	
			continue;
		}	
		TimeIntervalSum  += TimeInterval;
		if(TimeIntervalSum >=TimeLimit)
			break;
		
		Nb = record.end.MinuteNb;
		Buf +=  record.end.TotalDistance;
		TimeIntervalSum += Nb;
		
		if(TimeIntervalSum >=TimeLimit)
			break;
		//修改指针
		offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
		curPointer = AddPointer(&spt, offset);
		spt.CurPoint = curPointer;

		last_start = record.start;		
	}while((TimeIntervalSum <= TimeLimit)&&(pTable.RunRecord360h.CurPoint!=curPointer));
	
	//由脉冲数算出里程
	u_char disbuf[3];
	Buf = ComputeDistance100m(Buf);
	Int2BCD(Buf, disbuf);

	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x08;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x08;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x08^0x00^0x08^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	//发送上载数据块
	for(i = 0;i < 3;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
//		at91_usart_write(RS232,(u_char)(Buf>>(8*(2-i))));
		at91_usart_write(RS232,disbuf[i]);
//		SendCheckSum = SendCheckSum^((u_char)(Buf>>(8*(2-i))));
		SendCheckSum = SendCheckSum^(disbuf[i]);
	}
	//发送数据读出时刻
	WriteDataTxTime(RSCmdrxBuf[2]);
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	Modify_LastUploadTime();
	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}


//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_SpeedinTwoDays
//* Object                   : 上载最近2个日历天内的行驶速度，需要将内存中
//*                          : 记录的数据挑选、判断、累加、计算后才能输出，
//*                          : 最近2个日历天的行驶速度数据存于全局变量LargeDataBuffer中，
//*                          : 最先发送的数据块是速度数据对应的起始时间（放在Buf中），
//*                          : 对行驶速度的发送是先早时间后晚时间，
//*                          : 命令字为0x09
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机，
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载最近2个日历天内行驶速度数据的应答帧，
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*                          : LargeDataBuffer——存放内存中取出的最近2个日历天速度数据
//*----------------------------------------------------------------------------
/*void UpLoad_SpeedinTwoDays()
{
	int i,j,TimeInterval,TimeIntervalSum;
	int Nb;
	u_char buf[2];
	u_int TimeLimit;
	u_int curPointer,p;
	OTDR record;
	OTDR_start last_start;
	StructPT spt;
	DateTime BigTime,SmallTime;
	CLOCK TimeBefore2day;
	u_char Buf[5];

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	
	u_char StartTimeBuf[6];
	u_char StopTimeBuf[6];
	int offset;
	spt = pTable.RunRecord360h;

	//置初值
	TimeLimit = (BCD2Char(curTime.hour))*60+BCD2Char(curTime.minute)+24*60;
	j = TimeLimit-1;
	TimeIntervalSum = 0;
	curPointer = pTable.RunRecord360h.CurPoint;
	for(i = 0;i < 5;i++)
		Buf[i]=0;
		
	for(i = 0;i<TimeLimit;i++)
		LargeDataBuffer[i] = 0;	
	
	last_start.dt.type = 0;
	
	//计算出360小时之前的时间并装入缓冲区
	ComputeTimeBeforeX(&curTime,&TimeBefore2day,TimeLimit);
	Buf[0] = TimeBefore2day.year;
	Buf[1] = TimeBefore2day.month;
	Buf[2] = TimeBefore2day.day;
	Buf[3] = TimeBefore2day.hour;
	Buf[4] = TimeBefore2day.minute;
	
	do
	{
		//取出当前记录是不正确的
		if(!GetOTDR(curPointer,&(record.start), &(record.end)))
		{
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			continue;
		}
		
		if(last_start.dt.type==0xafaf)
			PrepareTime((u_char *)(&(last_start.dt.year)),&BigTime);
		else
			PrepareTime((u_char *)(&curTime),&BigTime);
		
		//计算当前记录与上一条记录之间的时间差	
		PrepareTime((u_char *)(&(record.end.dt.year)),&SmallTime);
		TimeInterval = HaveTime(BigTime,SmallTime);
		if(TimeInterval < 0)
			break;
		TimeIntervalSum  += TimeInterval;
		if(TimeIntervalSum >=TimeLimit)
			break;
		
		j -= TimeInterval;
		
		Nb = record.end.MinuteNb;
		TimeIntervalSum += Nb;
		i = 0;
		p = AddPointer(&spt, -sizeof(OTDR_end));
		while((j>=0)&&(Nb>0))
		{
			offset = -2;
			GetOTDRDataFromFlash((u_short *)p, offset,buf);
			LargeDataBuffer[j] = buf[1];
			j--;
			Nb--;
			if(Nb<=0)
				break;
			if(j>=0){
				LargeDataBuffer[j] = buf[0];
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
		offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
		curPointer = AddPointer(&spt, offset);
		spt.CurPoint = curPointer;

		last_start = record.start;		
	}while((j>0)&&(pTable.RunRecord360h.CurPoint!=curPointer));
	
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x09;
	RSCmdtxBuf[3] = (u_char)((TimeLimit+5)>>8);
	RSCmdtxBuf[4] = (u_char)(TimeLimit+5);
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
		SendCheckSum = SendCheckSum^RSCmdtxBuf[i];
	}
	//发送上载数据块
	for(i = 0;i < 5;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,Buf[i]);
		SendCheckSum = SendCheckSum^Buf[i];
	}
	for(i = 0;i < TimeLimit;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,LargeDataBuffer[i]);
		SendCheckSum = SendCheckSum^LargeDataBuffer[i];
	}
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	Modify_LastUploadTime();
	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}
*/
void UpLoad_SpeedinTwoDays()
{
	int offset,i,j,TimeInterval;
	StructPT spt;
	u_char ReadOTDRFlag = 0;            //读记录标志
	u_char InOTDRFlag = 0;              //在一条疲劳驾驶记录中间标志
	u_char FilledNB = 0;                //60字节中已经填充的字节数
	u_char FirstRead = 1;               //读第一条记录标志
	u_int curPointer,Old_Pointer,Mid_Pointer;   //DataFlash中的指针
	OTDR cur_record,Last_Record,temp_record;
	u_short hourNB = 0;                 //360小时计数器
	u_int CurRemainMinuteNB;            //当前剩余分钟数
	u_int temp;
	CLOCK StartTime;
	CLOCK current_time,temptime;
	u_char Buf60Bytes[60];              //60分钟数据缓冲区
	DateTime BigTime,SmallTime;
	u_int status232;
	u_int rhr;
	u_char HourLimit;

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	

	spt = pTable.RunRecord360h;
	curPointer = pTable.RunRecord360h.CurPoint;
	Old_Pointer = curPointer;
	for(i=0;i<60;i++)
		Buf60Bytes[i] = 0;
	for(i=0;i<48*65;i++)
		LargeDataBuffer[i]=0;

	do
	{
		if(!GetOTDR(curPointer,&(temp_record.start), &(temp_record.end)))
		{
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			continue;
		}
		break;
	}while(pTable.RunRecord360h.CurPoint!=curPointer);
	
	temptime.year = temp_record.end.dt.year;
	temptime.month = temp_record.end.dt.month;
	temptime.day = temp_record.end.dt.day;
	temptime.hour = temp_record.end.dt.hour;
	temptime.minute = temp_record.end.dt.minute;
		

	//置初值
	HourLimit = BCD2Char(temptime.hour) + 24 + 1;
	//数据区没有合法记录
	if(GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(cur_record.start), &(cur_record.end)))
	{
		CurRemainMinuteNB = cur_record.end.MinuteNb;
		RefreshCurTime((CLOCK *)(&(cur_record.end.dt.year)),(CLOCK *)(&current_time));
		spt.CurPoint = Old_Pointer;
		
		do
		{
			//有读记录标志
			if(ReadOTDRFlag)
			{
				//没有成功读入疲劳记录，补最后65字节并结束
				if(!GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(Last_Record.start), &(Last_Record.end)))
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取剩余分钟数的数据装入内存
					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
					hourNB++;
					break;
				}
				//成功读入一条疲劳驾驶记录
				else
				{
					spt.CurPoint = Old_Pointer;
					FirstRead = 0;
					//计算两条记录之间的时间差
					PrepareTime((u_char *)(&(cur_record.start.dt.year)),&BigTime);
					PrepareTime((u_char *)(&(Last_Record.end.dt.year)),&SmallTime);
					TimeInterval = HaveTime(BigTime,SmallTime);
					//如果剩余分钟数加时间间隔大于60分钟
					if((TimeInterval+FilledNB)>=60)
					{
						ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
						Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
						for(i=0;i<60;i++)
							Buf60Bytes[i] = 0;
						hourNB++;
						InOTDRFlag = 0;
						//更新剩余分钟数和时间
						CurRemainMinuteNB = Last_Record.end.MinuteNb;
						RefreshCurTime((CLOCK *)(&(Last_Record.end.dt.year)),&current_time);
						ReadOTDRFlag = 0;				
						continue;
					}
					//如果剩余分钟数加时间间隔小于60分钟
					else if((TimeInterval+FilledNB)<60)
					{
						if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)>=60)
						{
							ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -sizeof(OTDR_end));
							spt.CurPoint = Old_Pointer;
							offset = 0-temp;
							GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,Buf60Bytes);
							Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);			
							for(i=0;i<60;i++)
								Buf60Bytes[i] = 0;
							Old_Pointer = AddPointer(&spt, offset);
							Mid_Pointer = Old_Pointer;
							spt.CurPoint = Old_Pointer;
							hourNB++;
							InOTDRFlag = 1;
							//更新剩余分钟数和时间
							CurRemainMinuteNB = Last_Record.end.MinuteNb-temp;
							RefreshCurTime(&StartTime,&current_time);	
							ReadOTDRFlag = 0;
							continue;
						}
						else if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)<60)
						{
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
							spt.CurPoint = Old_Pointer;
							offset = 0-Last_Record.end.MinuteNb;
							j = 60-TimeInterval-FilledNB-Last_Record.end.MinuteNb;
							GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[j]));
							offset = 0-sizeof(OTDR_start)-Last_Record.end.MinuteNb;
							Old_Pointer = AddPointer(&spt, offset);
							spt.CurPoint = Old_Pointer;
							ReadOTDRFlag = 1;
							//获取当前记录的起始时间
							RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
							FilledNB = TimeInterval+FilledNB+Last_Record.end.MinuteNb;
							continue;
						}
					}
				}
			}
			//没有读记录标志
			if(!ReadOTDRFlag)
			{
				//当前剩余分钟数大于60
				if(CurRemainMinuteNB > 60)
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取本小时的60分钟数据存入Buf60Bytes
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
					GetOTDRDataFromFlash((u_short *)Old_Pointer,-60,Buf60Bytes);
					Old_Pointer = AddPointer(&spt, -60);
					Mid_Pointer = Old_Pointer;
					spt.CurPoint = Old_Pointer;
					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
					for(i=0;i<60;i++)
						Buf60Bytes[i] = 0;
					hourNB++;
					InOTDRFlag = 1;
					//更新剩余分钟数和时间
					CurRemainMinuteNB -= 60;
					RefreshCurTime(&StartTime,&current_time);				
					continue;
				}
				//当前剩余分钟数小于60
				else
				{
					//置读记录标志
					ReadOTDRFlag = 1;
					if(!FirstRead)//获取当前记录的起始时间
						RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
					//补充部分数据入60字节缓冲区
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
					offset = 0-CurRemainMinuteNB;
					if(offset!=0)
					{
						temp = 60-CurRemainMinuteNB;
						GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[temp]));
					}
					Old_Pointer = AddPointer(&spt, offset-sizeof(OTDR_start));
					spt.CurPoint = Old_Pointer;
					FilledNB = CurRemainMinuteNB;
					continue;
				}
			}
		}while((hourNB<=HourLimit)&&(pTable.RunRecord360h.CurPoint!=Old_Pointer));
	}
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x09;
	RSCmdtxBuf[3] = (u_char)((hourNB*65)>>8);
	RSCmdtxBuf[4] = (u_char)(hourNB*65);
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x09^((u_char)((hourNB*65)>>8))^((u_char)(hourNB*65))^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}


	for(j=0;j<hourNB;j++)
	{
		//发送上载数据块（360h之前的时间）
		for(i = 0;i < 65;i++)
		{
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,LargeDataBuffer[j*65+i]);
			SendCheckSum = SendCheckSum^LargeDataBuffer[j*65+i];
		}
		

	}
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);

	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}
//*----------------------------------------------------------------------------
//* Function Name            : TransmitOneOTDR
//* Object                   : 传送一条疲劳驾驶记录
//* Input Parameters         : record——指向疲劳驾驶记录的指针
//*                          : nb——记录的序号
//* Output Parameters        : none
//* Global  Variable Quoted  : SendCheckSum——校验和，得到计算结果后传送给PC机
//* Global  Variable Modified: SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
void TransmitOneOTDR(OTDR *record,u_char nb)
{
	u_char i;
	for(i=0;i<18;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,record[nb].end.driver.DriverLisenseCode[i]);
		SendCheckSum = SendCheckSum^record[nb].end.driver.DriverLisenseCode[i];
	}
	
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].start.dt.year);
	SendCheckSum = SendCheckSum^record[nb].start.dt.year;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].start.dt.month);
	SendCheckSum = SendCheckSum^record[nb].start.dt.month;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].start.dt.day);
	SendCheckSum = SendCheckSum^record[nb].start.dt.day;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].start.dt.hour);
	SendCheckSum = SendCheckSum^record[nb].start.dt.hour;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].start.dt.minute);
	SendCheckSum = SendCheckSum^record[nb].start.dt.minute;
	
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].end.dt.year);
	SendCheckSum = SendCheckSum^record[nb].end.dt.year;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].end.dt.month);
	SendCheckSum = SendCheckSum^record[nb].end.dt.month;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].end.dt.day);
	SendCheckSum = SendCheckSum^record[nb].end.dt.day;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].end.dt.hour);
	SendCheckSum = SendCheckSum^record[nb].end.dt.hour;
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,record[nb].end.dt.minute);
	SendCheckSum = SendCheckSum^record[nb].end.dt.minute;
}
//*----------------------------------------------------------------------------
//* Function Name            : UpLoad_OverThreeHours
//* Object                   : 上载最近2个日历天内同一驾驶员连续驾驶时间超过3小时
//*                          : 的所有记录数据，
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器,
//*                          : 赋值后通过232接口将其中的值（应答帧）传送给PC机
//*                          : SendCheckSum——校验和，得到计算结果后传送给PC机，
//* Global  Variable Modified: RSCmdtxBuf[0]－RSCmdtxBuf[5]——命令传送寄存器，
//*                          : 赋值为上载最近2个日历天内同一驾驶员连续驾驶时间超过3小时的应答帧，
//*                          : SendCheckSum——校验和，将传送的数据按字节进行异或
//*----------------------------------------------------------------------------
void UpLoad_OverThreeHours()
{
	u_int i;
	u_char recordnb;
	u_short length;
	OTDR record[16];
	
	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	
	recordnb = GetOverTimeRecordIn2Days(record);
	length = recordnb*28;
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x11;
	RSCmdtxBuf[3] = (u_char)(length>>8);
	RSCmdtxBuf[4] = (u_char)length;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x11^((u_char)(length>>8))^((u_char)length)^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	//发送上载数据块
	if(recordnb!=0)
	{
		for(i=0;i<recordnb;i++)
		{
			TransmitOneOTDR(&(record[i]),i);
		}
	}
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	Modify_LastUploadTime();
	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}


//*----------------------------------------------------------------------------
//* Function Name            : Set_DriverCode
//* Object                   : 设置驾驶员代码、驾驶证号码
//*                          : 命令字为0x81(RSCmdrxBuf[2])
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdrxBuf[2]——从PC机接收到的命令字，应为0x81
//*                          : DataLengthReceived——从PC机接收到的数据长度，应为0x15，
//*                          : 本程序中用来判断数据长度是否正确
//*                          : RSDatarxBuf——从PC机接收到的驾驶员代码、驾驶证号码数据
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void Set_DriverCode()
{
	u_int i=0;
	u_int new_drivercode;
	StructPara para;
	u_char CorrespondCmdWord;
	CorrespondCmdWord = RSCmdrxBuf[2];
	
	//判断数据块长度是否为21字节
	if(DataLengthReceived == 0x15)
	{
		//判断接收到的时间是否处于合法数据范围
		
		//发送设置正确应答帧
		RS232SetSuccess(CorrespondCmdWord);

		//首先将DATAFLASH的第一个4K区的分区表和参数表拷贝的内存中
		para = *PARAMETER_BASE;
	
		//计算新的驾驶员代码
		new_drivercode = RSDatarxBuf[0]*65536+RSDatarxBuf[1]*256 +RSDatarxBuf[2];
			
		while(para.DriverLisenseCode[i]==RSDatarxBuf[i+3])
			i++;		
		if((i < 18)||(para.DriverCode != new_drivercode))
		{
			//将需要修改的数据写入内存中已保存的4k中的相应位置
			para.DriverCode = new_drivercode;
			for(i = 0;i < 18;i++)
				para.DriverLisenseCode[i]=RSDatarxBuf[i+3];
			//将内存中改好的参数表Copy回DATAFLASH的First4k中
			WriteParameterTable(&para);
		}		
	}
	else//数据块长度错误，返回PC机通讯错误应答帧
		RS232SetError();
}

//*----------------------------------------------------------------------------
//* Function Name            : Set_AutoVIN
//* Object                   : 设置车辆VIN号、车牌号码、车牌分类，
//*                          : 如果新车牌号与记录仪内存中原有车牌号不同，
//*                          : 就刷新记录仪内部除参数表和分区表外的数据存储区，并
//*                          : 将分区表初始化，
//*                          : 命令字为0x82(RSCmdrxBuf[2])
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdrxBuf[2]——从PC机接收到的命令字，应为0x82
//*                          : DataLengthReceived——从PC机接收到的数据长度，应为0x29，
//*                          : 本程序中用来判断数据长度是否正确
//*                          : RSDatarxBuf——从PC机接收到的车辆VIN号、车牌号码、
//*                          : 车牌分类数据
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void Set_AutoVIN()
{
	u_int i,j,k,sector_addr;
	StructPara para;
	u_char CorrespondCmdWord;
	CorrespondCmdWord = RSCmdrxBuf[2];
	i = 0;
	j = 0;
	k = 0;
	
	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	
	//判断数据块长度是否为41字节
	if(DataLengthReceived == 0x29)
	{
		//判断接收到的时间是否处于合法数据范围
		
		//发送设置正确应答帧
		RS232SetSuccess(CorrespondCmdWord);

		//首先将DATAFLASH的第一个4K区的分区表和参数表拷贝的内存中
		para = *PARAMETER_BASE;
	
		while((para.AutoVIN[i]==RSDatarxBuf[i])&&(i<18))
			i++;		
		while((para.AutoCode[j]==RSDatarxBuf[j+17])&&(j<12))
			j++;		
		while((para.AutoSort[k]==RSDatarxBuf[k+29])&&(k<12))
			k++;		
		if((i < 17)||(j < 12)||(k < 12))
		{
			//将需要修改的数据写入内存中已保存的4k中的相应位置
			for(i = 0;i < 17;i++)
				para.AutoVIN[i]=RSDatarxBuf[i];
			for(i = 0;i < 18;i++)
				para.AutoCode[i]=RSDatarxBuf[i+17];
			for(i = 0;i < 18;i++)
				para.AutoSort[i]=RSDatarxBuf[i+29];
			if(j<12)
			{
				para.Door1Type = 0xff;
				para.Door2Type = 0xff;
			}
			//将内存中改好的参数表Copy回DATAFLASH的First4k中
			WriteParameterTable(&para);
		}
		if(j < 12)	
		{
			DisplayEraseDataFlash();			
			for(i=2;i<=255;i++)
			{
				sector_addr = i<<12;
				flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr);
//				if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr))
//					return(0);
			}
			InitializeTable(1,0,1);
			PulseTotalNumber = 0;//当前里程清零
			//清除标志	
			InRecordCycle=0;	//是否在记录数据过程中
			InFlashWriting=0;	//在FLASH写周期中
			FinishFlag=0;

			lcm_clear_ram(ALL);
			DisplayNormalUI();	
		}	
	}
	else//数据块长度错误，返回PC机通讯错误应答帧
		RS232SetError();
	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif

/*		//判断接收到的车辆VIN号是否处于合法数据范围
		for(i = 0;i < 17;i++)
		{
			LowFourBits[i] = (RSDatarxBuf[i]) & 0x0f;
			HighFourBits[i] = ((RSDatarxBuf[i]) & 0xf0) >> 4;
			if((LowFourBits[i] > 9) || (HighFourBits[i] > 9))
			{
				RS232SetError();
				return;
			}
		}
*/		
}


//*----------------------------------------------------------------------------
//* Function Name            : Set_RealTime
//* Object                   : 设置实时时间
//*                          : 命令字为0xc2
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdrxBuf[2]——从PC机接收到的命令字，应为0x82
//*                          : DataLengthReceived——从PC机接收到的数据长度，应为0x06，
//*                          : 本程序中用来判断数据长度是否正确
//*                          : RSDatarxBuf——从PC机接收到的实时时间
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void Set_RealTime()
{
	u_int i,j;
	flash_word *p;
	StructPara para;
	u_char CorrespondCmdWord;
	CorrespondCmdWord = RSCmdrxBuf[2];
	CLOCK ClockSet;
	
	//判断数据块长度是否为6字节
	if(DataLengthReceived == 0x06)
	{
		//判断接收到的时间是否处于合法数据范围
	    ClockSet.year = RSDatarxBuf[0];
	    ClockSet.month = RSDatarxBuf[1];
	    ClockSet.day = RSDatarxBuf[2];
	    ClockSet.hour = RSDatarxBuf[3];
	    ClockSet.minute = RSDatarxBuf[4];
	    ClockSet.second = RSDatarxBuf[5];   
		if(!IsCorrectCLOCK(&ClockSet))
		{
			RS232SetError();
			return;
		}
		//发送设置正确应答帧
		RS232SetSuccess(CorrespondCmdWord);

		//首先将DATAFLASH的第一个4K区的分区表和参数表拷贝的内存中
		para = *PARAMETER_BASE;
	
		//将需要修改的数据（realtime）写入内存中已保存的4k中的相应位置
		para.time = ClockSet;
		
		//将内存中改好的参数表Copy回DATAFLASH的First4k中
		WriteParameterTable(&para);
			
		//向时钟芯片设置时间
		SetCurrentDateTime(&ClockSet);
//		SetTimeFlag = 1;
			
	}
	else//数据块长度错误，返回PC机通讯错误应答帧
		RS232SetError();
}


//*----------------------------------------------------------------------------
//* Function Name            : Set_CHCO
//* Object                   : 设置车辆特征系数
//*                          : 命令字为0xc3
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdrxBuf[2]——从PC机接收到的命令字，应为0xc3
//*                          : DataLengthReceived——从PC机接收到的数据长度，应为0x03，
//*                          : 本程序中用来判断数据长度是否正确
//*                          : RSDatarxBuf——从PC机接收到的车辆特征系数
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void Set_CHCO()
{
	StructPara para;
	u_int new_chco;
	u_char CorrespondCmdWord;
	CorrespondCmdWord = RSCmdrxBuf[2];
	
	//判断数据块长度是否为3字节
	if(DataLengthReceived == 0x03)
	{
		//发送设置正确应答帧
		RS232SetSuccess(CorrespondCmdWord);
		
		//首先将DATAFLASH的第一个4K区的分区表和参数表拷贝的内存中
		para = *PARAMETER_BASE;
		
		//计算新的车轮系数
		new_chco = RSDatarxBuf[0]*65536+RSDatarxBuf[1]*256 +RSDatarxBuf[2];
		
		//判断参数表是否有变化//参数表有变化
		if(new_chco!=para.CHCO)
		{
			para.CHCO = new_chco;
			WriteParameterTable(&para);
		}
	}
	else//数据块长度错误，返回PC机通讯错误应答帧
		RS232SetError();
}


//*----------------------------------------------------------------------------
//* Function Name            : Set_ALL_PARA
//* Object                   : 设置记录仪的所有参数
//*                          : 命令字为0x83(RSCmdrxBuf[2])
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdrxBuf[2]——从PC机接收到的命令字，应为0x83
//*                          : DataLengthReceived——从PC机接收到的数据长度，应为0xff，
//*                          : 本程序中用来判断数据长度是否正确
//*                          : RSDatarxBuf——从PC机接收到的参数表信息
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void Set_ALL_PARA()
{
	u_int i,j,k,sector_addr;
	StructPara para;
	u_int new_chco;
	u_short new_CodeColor;
	u_int new_DriverCode;
	CLOCK ClockSet;
	u_char CorrespondCmdWord;
	CorrespondCmdWord = RSCmdrxBuf[2];
	
	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	
	//判断数据块长度是否为256字节
	if((DataLengthReceived == 0x100)&&(RSDatarxBuf[0]==0xaa)&&(RSDatarxBuf[1]==0x30))
	{
		//判断接收到的时间是否处于合法数据范围
		
		//首先将DATAFLASH的第一个4K区的分区表和参数表拷贝的内存中
		para = *PARAMETER_BASE;
	
		for(i=0;i<22;i++)
			para.sn[i] = RSDatarxBuf[i+2];

		//计算新的车轮系数
		new_chco = RSDatarxBuf[26]*65536+RSDatarxBuf[25]*256 +RSDatarxBuf[24];
		if(new_chco!=para.CHCO)
			para.CHCO = new_chco;	
					
		for(i=0;i<12;i++)
			para.AutoType[i] = RSDatarxBuf[i+28];

		i = 0;
		j = 0;
		k = 0;
		while((para.AutoVIN[i]==RSDatarxBuf[i+40])&&(i<18))    
			i++;		
		while((para.AutoCode[j]==RSDatarxBuf[j+58])&&(j<12))   
			j++;		
		while((para.AutoSort[k]==RSDatarxBuf[k+70])&&(k<12))   
			k++;		
		if((i < 18)||(j < 12)||(k < 12))   
		{
			//将需要修改的数据写入内存中已保存的4k中的相应位置
			for(i = 0;i < 18;i++)
				para.AutoVIN[i]=RSDatarxBuf[i+40];
			for(i = 0;i < 12;i++)
				para.AutoCode[i]=RSDatarxBuf[i+58];
			for(i = 0;i < 12;i++)
				para.AutoSort[i]=RSDatarxBuf[i+70];
			if(j<12)
			{
				para.Door1Type = 0xff;
				para.Door2Type = 0xff;
			}
		}
			
		new_CodeColor = RSDatarxBuf[83]*256 +RSDatarxBuf[82];   //?
		if(new_CodeColor != para.CodeColor)
			para.CodeColor = new_CodeColor;
/*	
		new_DriverCode = RSDatarxBuf[87]*65536+RSDatarxBuf[85]*256 +RSDatarxBuf[84];  //?
		if(new_DriverCode != para.DriverCode)
			para.DriverCode = new_DriverCode;
		
		for(i=0;i<20;i++)
			para.DriverLisenseCode[i] = RSDatarxBuf[i+88];
*/
		para.status_polarity = (RSDatarxBuf[109]<<8) + RSDatarxBuf[108];
		para.status_mask = (RSDatarxBuf[111]<<8) + RSDatarxBuf[110];
		para.OverSpeedTimeLimit = RSDatarxBuf[112];
		para.AlarmSound = RSDatarxBuf[113];
		para.LowSpeedLimit = RSDatarxBuf[114];
		para.HighSpeedLimit = RSDatarxBuf[115];	
		
        ClockSet.year = RSDatarxBuf[116];
        ClockSet.month = RSDatarxBuf[117];
        ClockSet.day = RSDatarxBuf[118];
        ClockSet.hour = RSDatarxBuf[119];
        ClockSet.minute = RSDatarxBuf[120];
        ClockSet.second = RSDatarxBuf[121];   
		if(!IsCorrectCLOCK(&ClockSet))
		{
			RS232SetError();
			return;
		}			
		para.time = ClockSet;
		
		if(j<12)
			para.InstallTime = ClockSet;
				
		para.PulseNumber = RSDatarxBuf[132];  
		para.RPM_Pulse = RSDatarxBuf[133];  
		
		para.IBBType = RSDatarxBuf[255]*256 + RSDatarxBuf[254];  
		
		//将内存中改好的参数表Copy回DATAFLASH的First4k中
		WriteParameterTable(&para);
				
		//向时钟芯片设置时间
		SetCurrentDateTime(&ClockSet);

		if(j < 12)	
		{
			DisplayEraseDataFlash();			
			for(i=2;i<=255;i++)
			{
				sector_addr = i<<12;
				flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr);
//				if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr))
//					return(0);
			}
			InitializeTable(1,0,1);
			PulseTotalNumber = 0;//当前里程清零
			//清除标志	
			InRecordCycle=0;	//是否在记录数据过程中
			InFlashWriting=0;	//在FLASH写周期中
			FinishFlag=0;

			lcm_clear_ram(ALL);
			DisplayNormalUI();	
		}	
		//发送设置正确应答帧
		RS232SetSuccess(CorrespondCmdWord);

	}
	else//数据块长度错误，返回PC机通讯错误应答帧
		RS232SetError();
	//启动看门狗
	#if WATCH_DOG_EN
        WD_CR = 0xc071;
        WD_OMR = 0x2343;
	#endif

/*		//判断接收到的车辆VIN号是否处于合法数据范围
		for(i = 0;i < 17;i++)
		{
			LowFourBits[i] = (RSDatarxBuf[i]) & 0x0f;
			HighFourBits[i] = ((RSDatarxBuf[i]) & 0xf0) >> 4;
			if((LowFourBits[i] > 9) || (HighFourBits[i] > 9))
			{
				RS232SetError();
				return;
			}
		}
*/		
}

//*----------------------------------------------------------------------------
//* Function Name            : RS232SetError
//* Object                   : 记录仪向PC机返回设置通讯错误命令帧
//*                          : 命令字为0xfb
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf——待传送的设置通讯错误应答帧
//*                          : SendCheckSum——待传送的校验和
//* Global  Variable Modified: RSCmdtxBuf——赋值为设置通讯错误应答帧
//*                          : SendCheckSum——将待传送的应答帧按字节异或
//*----------------------------------------------------------------------------
void RS232SetError()
{
	u_int i;
	
	//向PC机返回设置通讯错误应答帧
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0xfb;
	RSCmdtxBuf[3] = 0x00;
	SendCheckSum = 0x55^0x7a^0xfb^0x00;	
	for(i = 0; i < 4;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);									
}

//*----------------------------------------------------------------------------
//* Function Name            : RS232UploadError
//* Object                   : 记录仪向PC机返回上载通讯错误命令帧
//*                          : 命令字为0xfa
//* Input Parameters         : none
//* Output Parameters        : none
//* Global  Variable Quoted  : RSCmdtxBuf——待传送的上载通讯错误应答帧
//*                          : SendCheckSum——待传送的校验和
//* Global  Variable Modified: RSCmdtxBuf——赋值为上载通讯错误应答帧
//*                          : SendCheckSum——将待传送的应答帧按字节异或
//*----------------------------------------------------------------------------
void RS232UploadError()
{
	u_int i;

	//向PC机返回上载通讯错误应答帧
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0xfa;
	RSCmdtxBuf[3] = 0x00;
	SendCheckSum = 0x55^0x7a^0xfa^0x00;	
	for(i = 0; i < 4;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);									
}

//*----------------------------------------------------------------------------
//* Function Name            : RS232SetSuccess
//* Object                   : 记录仪向PC机返回设置正确应答帧
//* Input Parameters         : CorrespondCmdWord——从PC机接收到的与设置命令相对应的命令字
//* Output Parameters        : none
//* Global  Variable Quoted  : CorrespondCmdWord——从PC机接收到的与设置命令相对应的命令字
//*                          : RSCmdtxBuf——待传送的设置通讯正确应答帧
//*                          : SendCheckSum——待传送的校验和
//* Global  Variable Modified: RSCmdtxBuf——赋值为设置通讯正确应答帧
//*                          : SendCheckSum——将待传送的应答帧按字节异或
//*----------------------------------------------------------------------------
void RS232SetSuccess(u_char CorrespondCmdWord)
{
	u_int i;

	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = CorrespondCmdWord;
	RSCmdtxBuf[3] = 0x00;
	RSCmdtxBuf[4] = 0x00;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^CorrespondCmdWord^0x00^0x00^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
}

//*----------------------------------------------------------------------------
//* Function Name            : Write4kDataToFlash
//* Object                   : 将内存中的4k参数写入FLASH的一个4K中，
//*                          : 如果是写第0个4k（参数表）而且其中的车牌号发生了变化，
//*                          : 就将数据存储器的2－255个4k全部擦除，同时更新分区表
//* Input Parameters         : PageNb——待写入的4k在数据存储器中的页号（0或1）
//* Output Parameters        : "1"——写入成功
//*                          : "0"——写入失败
//* Global  Variable Quoted  : ep2_bufr——USB设备接收到的一个4k
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
int Write4kDataToFlash(u_char PageNb)
{
	u_int i = 0;
	u_int j = 0;
	u_short *Buffer;
	StructPara old_para,new_para;
	
	flash_word *sector_addr = (flash_word *)(DATAFLASH_BASE + 0x1000*PageNb);
	flash_word *word_addr = (flash_word *)(DATAFLASH_BASE + 0x1000*PageNb);

	if(PageNb==0)
		old_para = *PARAMETER_BASE;
	//擦除FLASH
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,(flash_word *)sector_addr))
		return(0);
	
	//内存中的一个4k数据重新写回FLASH
	Buffer = (u_short *)ep2_bufr;
	for(i = 0;i < 2048;i++ )
	{
		if(!flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,word_addr,*Buffer))
			return ( FALSE ) ;
		word_addr++;
		Buffer++;
	}
	
	if(PageNb==0)
	{
	    CLOCK clockset;
	    clockset=PARAMETER_BASE->time;
		SetCurrentDateTime(&clockset);
		
		new_para = *PARAMETER_BASE;
		
		while((old_para.AutoCode[j]==new_para.AutoCode[j])&&(j<12))
			j++;
		
		//如果车牌号码有变化，就擦除DATAFLASH的2－255分区
		if(j < 12)	
		{
			DisplayEraseDataFlash();
			for(i=2;i<=255;i++)
			{
				WD_CR = 0xc071;//reload watchdog
				sector_addr = (flash_word *)(i<<12);
				flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr);
//				if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr))
//					return(0);
			}
			InitializeTable(1,0,1);
			PulseTotalNumber = 0;//当前里程清零	
			//清除标志	
			InRecordCycle=0;	//是否在记录数据过程中
			InFlashWriting=0;	//在FLASH写周期中
			FinishFlag=0;


			lcm_clear_ram(ALL);
			DisplayNormalUI();	
		}	
	}		
	return(1);
}		
//*----------------------------------------------------------------------------
//* Function Name            : Modify_LastUploadTime
//* Object                   : 将内存中的4k参数写入FLASH的一个4K中，
//*                          : 如果是写第0个4k（参数表）而且其中的车牌号发生了变化，
//*                          : 就将数据存储器的2－255个4k全部擦除，同时更新分区表
//* Input Parameters         : PageNb——待写入的4k在数据存储器中的页号（0或1）
//* Output Parameters        : "1"——写入成功
//*                          : "0"——写入失败
//* Global  Variable Quoted  : ep2_bufr——USB设备接收到的一个4k
//* Global  Variable Modified: none
//*----------------------------------------------------------------------------
void Modify_LastUploadTime()
{
	PartitionTable p;
	p = *PartitionTable_BASE;
	
	p.LastUploadTime = curTime;
	
    WritePartitionTable(&p);
}

void UpLoad_Speed360h_wayon()
{
	int offset,i,j,k,TimeInterval;
	StructPT spt;
	u_char ReadOTDRFlag = 0;            //读记录标志
	u_char InOTDRFlag = 0;              //在一条疲劳驾驶记录中间标志
	u_char FilledNB = 0;                //60字节中已经填充的字节数
	u_char FirstRead = 1;               //读第一条记录标志
	u_int curPointer,Old_Pointer,Mid_Pointer;   //DataFlash中的指针
	OTDR cur_record,Last_Record;
	u_short hourNB = 0;                 //360小时计数器
	u_int CurRemainMinuteNB;            //当前剩余分钟数
	u_int temp;
	CLOCK StartTime;
	CLOCK current_time;
	u_char Buf60Bytes[60];              //60分钟数据缓冲区
	DateTime BigTime,SmallTime;
	u_int status232;
	u_int rhr;

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	

	spt = pTable.RunRecord360h;
	curPointer = pTable.RunRecord360h.CurPoint;
	Old_Pointer = curPointer;
	for(i=0;i<60;i++)
		Buf60Bytes[i] = 0;
	
	//数据区没有合法记录
	if(GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(cur_record.start), &(cur_record.end)))
	{
		CurRemainMinuteNB = cur_record.end.MinuteNb;
		RefreshCurTime((CLOCK *)(&(cur_record.end.dt.year)),(CLOCK *)(&current_time));
		spt.CurPoint = Old_Pointer;
		
		do
		{
			//有读记录标志
			if(ReadOTDRFlag)
			{
				//没有成功读入疲劳记录，补最后65字节并结束
				if(!GetOneOTDRandModifyPointer(&(curPointer), &(Old_Pointer), &(Last_Record.start), &(Last_Record.end)))
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取剩余分钟数的数据装入内存
					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
					hourNB++;
					break;
				}
				//成功读入一条疲劳驾驶记录
				else
				{
					spt.CurPoint = Old_Pointer;
					FirstRead = 0;
					//计算两条记录之间的时间差
					PrepareTime((u_char *)(&(cur_record.start.dt.year)),&BigTime);
					PrepareTime((u_char *)(&(Last_Record.end.dt.year)),&SmallTime);
					TimeInterval = HaveTime(BigTime,SmallTime);
					//如果剩余分钟数加时间间隔大于60分钟
					if((TimeInterval+FilledNB)>=60)
					{
						ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
						Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
						for(i=0;i<60;i++)
							Buf60Bytes[i] = 0;
						hourNB++;
						InOTDRFlag = 0;
						//更新剩余分钟数和时间
						CurRemainMinuteNB = Last_Record.end.MinuteNb;
						RefreshCurTime((CLOCK *)(&(Last_Record.end.dt.year)),&current_time);
						ReadOTDRFlag = 0;				
						continue;
					}
					//如果剩余分钟数加时间间隔小于60分钟
					else if((TimeInterval+FilledNB)<60)
					{
						if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)>=60)
						{
							ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -sizeof(OTDR_end));
							spt.CurPoint = Old_Pointer;
							offset = 0-temp;
							GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,Buf60Bytes);
							Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);			
							for(i=0;i<60;i++)
								Buf60Bytes[i] = 0;
							Old_Pointer = AddPointer(&spt, offset);
							Mid_Pointer = Old_Pointer;
							spt.CurPoint = Old_Pointer;
							hourNB++;
							InOTDRFlag = 1;
							//更新剩余分钟数和时间
							CurRemainMinuteNB = Last_Record.end.MinuteNb-temp;
							RefreshCurTime(&StartTime,&current_time);	
							ReadOTDRFlag = 0;
							continue;
						}
						else if((TimeInterval+FilledNB+Last_Record.end.MinuteNb)<60)
						{
							temp = 60-TimeInterval-FilledNB;
							Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
							spt.CurPoint = Old_Pointer;
							offset = 0-Last_Record.end.MinuteNb;
							j = 60-TimeInterval-FilledNB-Last_Record.end.MinuteNb;
							GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[j]));
							offset = 0-sizeof(OTDR_start)-Last_Record.end.MinuteNb;
							Old_Pointer = AddPointer(&spt, offset);
							spt.CurPoint = Old_Pointer;
							ReadOTDRFlag = 1;
							//获取当前记录的起始时间
							RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
							FilledNB = TimeInterval+FilledNB+Last_Record.end.MinuteNb;
							continue;
						}
					}
				}
			}
			//没有读记录标志
			if(!ReadOTDRFlag)
			{
				//当前剩余分钟数大于60
				if(CurRemainMinuteNB > 60)
				{
					ComputeTimeBeforeX(&current_time,&StartTime,60);  //计算本小时的起始时间
					//获取本小时的60分钟数据存入Buf60Bytes
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
					GetOTDRDataFromFlash((u_short *)Old_Pointer,-60,Buf60Bytes);
					Old_Pointer = AddPointer(&spt, -60);
					Mid_Pointer = Old_Pointer;
					spt.CurPoint = Old_Pointer;
					Write65ByteToSRAM(hourNB,&StartTime,Buf60Bytes);
					for(i=0;i<60;i++)
						Buf60Bytes[i] = 0;
					hourNB++;
					InOTDRFlag = 1;
					//更新剩余分钟数和时间
					CurRemainMinuteNB -= 60;
					RefreshCurTime(&StartTime,&current_time);				
					continue;
				}
				//当前剩余分钟数小于60
				else
				{
					//置读记录标志
					ReadOTDRFlag = 1;
					if(!FirstRead)//获取当前记录的起始时间
						RefreshCurTime((CLOCK *)(&(Last_Record.start.dt.year)),(CLOCK *)(&(cur_record.start.dt.year)));
					//补充部分数据入60字节缓冲区
					if(!InOTDRFlag)
					{
						Old_Pointer = AddPointer(&spt, -(sizeof(OTDR_end)));
						spt.CurPoint = Old_Pointer;
					}
					offset = 0-CurRemainMinuteNB;
					if(offset!=0)
					{
						temp = 60-CurRemainMinuteNB;
						GetOTDRDataFromFlash((u_short *)Old_Pointer,offset,&(Buf60Bytes[temp]));
					}
					Old_Pointer = AddPointer(&spt, offset-sizeof(OTDR_start));
					spt.CurPoint = Old_Pointer;
					FilledNB = CurRemainMinuteNB;
					continue;
				}
			}
		}while((hourNB<=360)&&(pTable.RunRecord360h.CurPoint!=Old_Pointer));
	}
	
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x12;
	RSCmdtxBuf[3] = (u_char)(hourNB>>8);
	RSCmdtxBuf[4] = (u_char)hourNB;
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0x55^0x7a^0x12^((u_char)(hourNB>>8))^((u_char)hourNB)^0x00;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
	}
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);

	if(!rs232_status_ready(&status232, 0x01))
	{
		RS232UploadError();
		//启动看门狗
		#if WATCH_DOG_EN
	    WD_CR = 0xc071;
	    WD_OMR = 0x2343;
		#endif
		return;
	}
	//读取PC机的应答帧
	for(i = 0;i < 3;i++)
	{
		if(!rs232_status_ready(&status232, 0x01))
		{
			RS232UploadError();
			//启动看门狗
			#if WATCH_DOG_EN
		    WD_CR = 0xc071;
		    WD_OMR = 0x2343;
			#endif
			return;
		}
		
		at91_usart_read(RS232,&rhr);
		RSCmdrxBuf[i] = (char)rhr;
	}
	
	//判断PC机的应答帧是否正确
	if((RSCmdrxBuf[0]!=0xaa)||(RSCmdrxBuf[1]!=0x75)||(RSCmdrxBuf[2]!=0x12))
	{
		RS232UploadError();
		//启动看门狗
		#if WATCH_DOG_EN
	    WD_CR = 0xc071;
	    WD_OMR = 0x2343;
		#endif
		return;
	}

	for(j=0;j<hourNB;j++)
	{
		//首先发送应答帧的起始字头和命令字
		RSCmdtxBuf[0] = 0x55;
		RSCmdtxBuf[1] = 0x7a;
		RSCmdtxBuf[2] = 0x12;
		RSCmdtxBuf[3] = (u_char)(hourNB>>8);
		RSCmdtxBuf[4] = (u_char)hourNB;
		RSCmdtxBuf[5] = 0x00;
		SendCheckSum = 0x55^0x7a^0x12^((u_char)(hourNB>>8))^((u_char)hourNB)^0x00;
		for(i = 0; i < 6;i++)
		{
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,RSCmdtxBuf[i]);
		}

		//发送上载数据块（360h之前的时间）
		for(i = 0;i < 65;i++)
		{
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,LargeDataBuffer[j*65+i]);
			SendCheckSum = SendCheckSum^LargeDataBuffer[j*65+i];
		}
		
		//发送校验和
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,SendCheckSum);
		
		//读取PC机的应答帧
		for(i = 0;i < 3;i++)
		{
			if(!rs232_status_ready(&status232, 0x01))
			{
				RS232UploadError();
				//启动看门狗
				#if WATCH_DOG_EN
			    WD_CR = 0xc071;
			    WD_OMR = 0x2343;
				#endif
				return;
			}
			
			at91_usart_read(RS232,&rhr);
			RSCmdrxBuf[i] = (char)rhr;
		}
		
		//判断PC机的应答帧是否正确
		if((RSCmdrxBuf[0]==0xaa)&&(RSCmdrxBuf[1]==0x75)&&(RSCmdrxBuf[2]==0x12))
			continue;
		else if((RSCmdrxBuf[0]==0xaa)&&(RSCmdrxBuf[1]==0x75)&&(RSCmdrxBuf[2]==0xfa))
		{
			j--;
			continue;
		}
		else
		{
			RS232UploadError();
			//启动看门狗
			#if WATCH_DOG_EN
		    WD_CR = 0xc071;
		    WD_OMR = 0x2343;
			#endif
			return;
		}
	}

	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}

void UpLoad_SpeedinTwoDays_wayon()
{
	int i,j,TimeInterval,TimeIntervalSum;
	int Nb;
	u_char buf[2];
	u_int TimeLimit;
	u_int curPointer,p;
	OTDR record,temp_record;
	OTDR_start last_start;
	StructPT spt;
	DateTime BigTime,SmallTime;
	CLOCK TimeBefore2day,temptime;
	u_char Buf[5];

	//关闭看门狗
	#if WATCH_DOG_EN
	WD_OMR = 0x2340;
	#endif	
	u_char StartTimeBuf[6];
	u_char StopTimeBuf[6];
	int offset;
	temp_record.end.dt.year = 0xff;
	temp_record.end.dt.month = 0xff;
	temp_record.end.dt.day = 0xff;
	temp_record.end.dt.hour = 0xff;
	temp_record.end.dt.minute = 0xff;
	spt = pTable.RunRecord360h;
	curPointer = pTable.RunRecord360h.CurPoint;
	TimeIntervalSum = 0;
	for(i = 0;i < 5;i++)
		Buf[i]=0;
		
	for(i = 0;i<48*60;i++)
		LargeDataBuffer[i] = 0;	

	do
	{
		if(!GetOTDR(curPointer,&(temp_record.start), &(temp_record.end)))
		{
			offset = -1;
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
			continue;
		}
		break;
	}while(pTable.RunRecord360h.CurPoint!=curPointer);
	
	if(temp_record.end.dt.month!=0xff)
	{
		temptime.year = temp_record.end.dt.year;
		temptime.month = temp_record.end.dt.month;
		temptime.day = temp_record.end.dt.day;
		temptime.hour = temp_record.end.dt.hour;
		temptime.minute = temp_record.end.dt.minute;	

		//置初值
		TimeLimit = (BCD2Char(temptime.hour))*60+BCD2Char(temptime.minute)+24*60;
		j = TimeLimit-1;
		
		last_start.dt.type = 0;
		
		//计算出360小时之前的时间并装入缓冲区
		ComputeTimeBeforeX(&temptime,&TimeBefore2day,TimeLimit);
		Buf[0] = TimeBefore2day.year;
		Buf[1] = TimeBefore2day.month;
		Buf[2] = TimeBefore2day.day;
		Buf[3] = TimeBefore2day.hour;
		Buf[4] = TimeBefore2day.minute;
		
		do
		{
			//取出当前记录是不正确的
			if(!GetOTDR(curPointer,&(record.start), &(record.end)))
			{
				offset = -1;
				curPointer = AddPointer(&spt, offset);
				spt.CurPoint = curPointer;
				continue;
			}
			
			if(last_start.dt.type==0xafaf)
				PrepareTime((u_char *)(&(last_start.dt.year)),&BigTime);
			else
				PrepareTime((u_char *)(&temptime),&BigTime);
			
			//计算当前记录与上一条记录之间的时间差	
			PrepareTime((u_char *)(&(record.end.dt.year)),&SmallTime);
			TimeInterval = HaveTime(BigTime,SmallTime);
			if(TimeInterval < 0)
				break;
			if(last_start.dt.type!=0xafaf)
				TimeLimit -= TimeInterval;
			TimeIntervalSum  += TimeInterval;
			if(TimeIntervalSum >=TimeLimit)
				break;
			
			j -= TimeInterval;
			
			Nb = record.end.MinuteNb;
			TimeIntervalSum += Nb;
			i = 0;
			p = AddPointer(&spt, -sizeof(OTDR_end));
			while((j>=0)&&(Nb>0))
			{
				offset = -2;
				GetOTDRDataFromFlash((u_short *)p, offset,buf);
				LargeDataBuffer[j] = buf[1];
				j--;
				Nb--;
				if(Nb<=0)
					break;
				if(j>=0){
					LargeDataBuffer[j] = buf[0];
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
			offset = 0 - (sizeof(OTDR_start)+sizeof(OTDR_end)+record.end.MinuteNb);
			curPointer = AddPointer(&spt, offset);
			spt.CurPoint = curPointer;
	
			last_start = record.start;		
		}while((j>0)&&(pTable.RunRecord360h.CurPoint!=curPointer));
	}
	//数据区无数据
	else
	{
		TimeLimit = (BCD2Char(curTime.hour))*60+BCD2Char(curTime.minute)+24*60;
		ComputeTimeBeforeX(&curTime,&TimeBefore2day,TimeLimit);
		Buf[0] = TimeBefore2day.year;
		Buf[1] = TimeBefore2day.month;
		Buf[2] = TimeBefore2day.day;
		Buf[3] = TimeBefore2day.hour;
		Buf[4] = TimeBefore2day.minute;
	}
		
	//首先发送应答帧的起始字头和命令字
	RSCmdtxBuf[0] = 0x55;
	RSCmdtxBuf[1] = 0x7a;
	RSCmdtxBuf[2] = 0x13;
	RSCmdtxBuf[3] = (u_char)((TimeLimit+5)>>8);
	RSCmdtxBuf[4] = (u_char)(TimeLimit+5);
	RSCmdtxBuf[5] = 0x00;
	SendCheckSum = 0;
	for(i = 0; i < 6;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,RSCmdtxBuf[i]);
		SendCheckSum = SendCheckSum^RSCmdtxBuf[i];
	}
	//发送上载数据块
	for(i = 0;i < 5;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,Buf[i]);
		SendCheckSum = SendCheckSum^Buf[i];
	}
	for(i = 0;i < TimeLimit;i++)
	{
		while((at91_usart_get_status(RS232) & 0x02) != 0x02);
		at91_usart_write(RS232,LargeDataBuffer[i]);
		SendCheckSum = SendCheckSum^LargeDataBuffer[i];
	}
	
	//发送校验和
	while((at91_usart_get_status(RS232) & 0x02) != 0x02);
	at91_usart_write(RS232,SendCheckSum);
	Modify_LastUploadTime();
	//启动看门狗
	#if WATCH_DOG_EN
    WD_CR = 0xc071;
    WD_OMR = 0x2343;
	#endif
}

void Reply_Schedule(void)
{
	u_char i;
	u_char sendchecksum = 0;
	switch(Schedule_Result)
	{
		//查询到达成功
		case 1:
			RSCmdrxBuf[0] = 0x55;
			RSCmdrxBuf[1] = 0x7a;
			for(i = 0;i < 16;i++)
			{
				while((at91_usart_get_status(RS232) & 0x02) != 0x02);
				at91_usart_write(RS232,RSCmdrxBuf[i]);
				sendchecksum = sendchecksum^RSCmdrxBuf[i];
			}
			//发送校验和
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,sendchecksum);
		break;
		//查询到达失败
		case 2 : 
			RSCmdrxBuf[0] = 0x55;
			RSCmdrxBuf[1] = 0x7a;
			RSCmdrxBuf[2] = 0xce;
			for(i = 0;i < 16;i++)
			{
				while((at91_usart_get_status(RS232) & 0x02) != 0x02);
				at91_usart_write(RS232,RSCmdrxBuf[i]);
				sendchecksum = sendchecksum^RSCmdrxBuf[i];
			}
			//发送校验和
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,sendchecksum);
		break;
		//调度成功
		case 3 :
			RSCmdrxBuf[0] = 0x55;
			RSCmdrxBuf[1] = 0x7a;
			for(i = 0;i < 16;i++)
			{
				while((at91_usart_get_status(RS232) & 0x02) != 0x02);
				at91_usart_write(RS232,RSCmdrxBuf[i]);
				sendchecksum = sendchecksum^RSCmdrxBuf[i];
			}
			//发送校验和
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,sendchecksum);
		break;
		//调度失败
		case 4 :
			RSCmdrxBuf[0] = 0x55;
			RSCmdrxBuf[1] = 0x7a;
			RSCmdrxBuf[2] = 0xbd;
			for(i = 0;i < 16;i++)
			{
				while((at91_usart_get_status(RS232) & 0x02) != 0x02);
				at91_usart_write(RS232,RSCmdrxBuf[i]);
				sendchecksum = sendchecksum^RSCmdrxBuf[i];
			}
			//发送校验和
			while((at91_usart_get_status(RS232) & 0x02) != 0x02);
			at91_usart_write(RS232,sendchecksum);
		break;
	}
	Schedule_Result = 0;
	//禁止USART1的RXRDY中断
	RS232->usart_base->US_CR = 0x20;
	RS232->usart_base->US_IDR = 0x01;
	CloseUSART1Time=0;

}