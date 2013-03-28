#ifndef ibb3_h
#define ibb3_h


#include    "periph/stdc/std_c.h"

typedef unsigned char LCD_ZM; 

////////////////////////配置参数////////////////////////////////////////
#define GetSpeedSelf    	0       //主机自己采集速度脉冲
#define POWER_MODEL_EN    	1    	/* 使用电源模块，没有光隔 */
#define POWERON_LINE_EN     0		//电源板上电线
#define StatusPol           0       //主机给适配器送状态极性
#define StationStamp_EN     0       //站点标注标志

#define guizhoufile    	    0       //为贵州长文件名"*"
#define guizhou             1       //针对贵州特殊要求（改车牌号不能刷新数据）
#define GPS                 0       //是否打开GPS防火墙

//***以下参数在产品版本号中显示为后缀***//
#define OpenDoorDeal    	1       //包含软件开关门处理
#define RTC8025             1     //时钟芯片选择开关
#define GetSpeedStatusBy232 0     //通过串口和适配器通讯获取速度和全部状态开关
#define WATCH_DOG_EN    	1     // 看门狗开关
#define RPM_EN				0     //发动机转速开关
#define SectionAlarm_EN     0     //分路段报警开关
#define OpenDoorAlarm		0     //开门行驶报警开关
#define Test                0	  //测试工装
#define Status14			0     //超过8种状态
//**************************************//
#define AlarmRecord         0     //超速和疲劳驾驶报警记录
////////////////////////////////////////////////////////////////////////
#if RTC8025
#include	"ibb3board_rtc8025.h"
#else
#include	"ibb3board.h"
#endif

//2004.03.02 panhui 武汉公交休眠改为2小时（有开门行驶报警）
#if OpenDoorAlarm
#define GotoSleepTime 36000     //休眠时间＝120分钟
#else
#define GotoSleepTime 1500     //休眠时间＝300秒＝5分钟
#endif

#define DoorNBLimit   6       //200ms中开门信号有效的个数门限

//定义中断优先级
#define USART1_INT_PRI	4
#define IRQ0_INT_PRI	7       //USB
#define IRQ1_INT_PRI    3       //ENGINE SPEED
#define IRQ2_INT_PRI    6       //SPEED
#define TC0_INT_PRI     0       //TICK
#define Clock_PRI     	5       //CLOCK 
#define DriverCard_PRI 	5		//DRIVERCARD
#define TC2_INT_PRI     6       //计时器
#define TC1_INT_PRI     5       //door计时器


typedef struct
{//定义时钟

	u_char year;
	u_char month;
	u_char day;
	u_char hour;
	u_char minute;
	u_char second;
	
} CLOCK;
typedef struct
{//定义时钟

	int year;
	int month;
	int day;
	int time;//以分钟为单位
	
} DateTime;

typedef struct
{//定义上传和下传时间
	u_char year;
	u_char month;
	u_char day;
	u_char hour;
	u_char minute;
	u_char second;
	u_char type;
	u_char reserved;
} UpDownCLOCK;

typedef struct
{//定义时钟

	u_short type;
	u_char year;
	u_char month;
	u_char day;
	u_char hour;
	u_char minute;
	u_char second;
	
} Record_CLOCK;

typedef struct
{
	u_char speed;
	u_int bound;
} RoadSection;
typedef struct
{

	u_short mark;//*特征字——2
	u_char  sn[22];//产品序列号——22(24)
	u_int CHCO;//车辆特征系数——4(28)
	u_char  AutoType[12];//车辆类型——12(40)
	u_char  AutoVIN[18];//车辆VIN号——18(58)
	u_char  AutoCode[12];//车牌号——12(70)
	u_char  AutoSort[12];//车牌分类——12(82)
	u_short  CodeColor;//车牌颜色——2(84)
	u_int  DriverCode;//驾驶员代码——4(88)
	u_char  DriverLisenseCode[20];//驾驶证号码——20(108)
	u_short status_polarity;//状态极性——2(110)
	u_short status_mask;//在用状态——2(112)
	u_char  OverSpeedTimeLimit;//超速时间门限（0－255秒）——1(113)
	u_char  AlarmSound;//声音报警选择（0x00-无声；0xFF-有声）——1(114)
	u_char  LowSpeedLimit;//低速路速度门限——1(115)
	u_char  HighSpeedLimit;//高速路速度门限——1(116)
	CLOCK   time;//实时时钟BCD码表示——8(124)
	CLOCK   InstallTime;//初装日期BCD码表示——8(132)
	u_char  PulseNumber;//车速传感器每转产生的脉冲数
	u_char  RPM_Pulse;
	u_char  Door1Type;
	u_char  Door2Type;
	u_char  reserved[112];//××（预留，暂时不用）(254)
	u_char  DriveHour24;//疲劳驾驶时间
	u_char  RestHour24;
	u_char  DriveHour;	//疲劳驾驶时间门限
	u_char  RestMinute;	//疲劳驾驶最少休息时间门限
	u_char  wakeGPStime;
	u_short  IBBType;//记录仪代码——2(256)
					//（0x0A＝IBB-100A； 0x0C＝IBB-2C；0x30=IBB-3）
	u_char SectionNumber;
	RoadSection section[20];
	//2004.03.23
	u_char  reserved1[92];//为了保证副站发车的路线分段设置从地址0x200开始写
	u_char SectionNumber1;//副站发车路线分段的段数
	RoadSection section1[20];//路段及其速度限制
} StructPara;

typedef struct
{
	u_int  DriverCode;//驾驶员代码——4(88)
	u_char  DriverLisenseCode[20];//驾驶证号码——20(108)
} DRIVER;

typedef struct
{//分区表结构

	u_int  BaseAddr;//起始地址
	u_int  EndAddr;//结束地址
	u_int  CurPoint;//当前指针地址,指向下一个数据可存放的位置
	
} StructPT;

typedef struct
{

	u_short  Available;//=0表示没有使用此数据区；>0表示数据区有效
	StructPT DoubtPointData;
	StructPT OverSpeedRecord;
	StructPT PowerOffRunRecord;
	StructPT RunRecord360h;
	StructPT BaseData;
	StructPT OilData;
	StructPT RPMData;
	StructPT TemperatureData;
	StructPT OilPressData;
	u_int	 TotalDistance;
	CLOCK	 LastUploadTime;
	u_int	 DriverCode;//驾驶员代码
	u_char   DriverLisenseCode[20];//驾驶证号码
	u_char 	 InOSAlarmCycle;//“在分路段报警周期中”标志（0xaa主站发车，0x55副站发车）
	u_int	 OSAlarmAddupDistance;//分路段报警路程累计

} PartitionTable;

/* 定义分区有效字各位含义 */
#define DOUBTPOINTDATA  	0
#define OVERSPEEDRECORD    	1
#define POWEROFFRUNRECORD   2
#define RUNRECORD360h 		3
#define BASEDATA 			4
#define OILDATA  			5
#define RPMDATA  			6
#define TEMPERATUREDATA  	7
#define OILPRESSDATA 		8
#define STATUS14DATA        9
/*  */
#define DoubtDataBlockSize 210

/* 定义数据分区表 */
#define DOUBTPOINTDATA_EN  		1

#if AlarmRecord
#define OVERSPEEDRECORD_EN    	1
#else
#define OVERSPEEDRECORD_EN    	0
#endif

#define POWEROFFRUNRECORD_EN   	0	
#define RUNRECORD360h_EN 		1
#define BASEDATA_EN 			1
#define OILDATA_EN  			0

#if RPM_EN
#define RPMDATA_EN  			1
#else
#define RPMDATA_EN				0
#endif

#if Status14
#define Status14DATA_EN  			1
#else
#define Status14DATA_EN				0
#endif

#define TEMPERATUREDATA_EN  	0
#define OILPRESSDATA_EN 		0

#define PartitionTableFlag  (DOUBTPOINTDATA_EN << DOUBTPOINTDATA)|(OVERSPEEDRECORD_EN << OVERSPEEDRECORD)|(RUNRECORD360h_EN << RUNRECORD360h)|(BASEDATA_EN << BASEDATA)|(RPMDATA_EN << RPMDATA)|(Status14DATA_EN << STATUS14DATA)

#define PARAMETER_BASE ((StructPara *)(DATAFLASH_BASE+0x00000000))
#define PartitionTable_BASE ((PartitionTable *)(DATAFLASH_BASE+0x00001000))
#define DPD_BASE      DATAFLASH_BASE+0x02000
#define DPD_END       DATAFLASH_BASE+0x06fff
#define RR360H_BASE   DATAFLASH_BASE+0x07000
#define RR360H_END    DATAFLASH_BASE+0x0cfff
#define BASEDATA_BASE DATAFLASH_BASE+0x0d000
#define BASEDATA_END  DATAFLASH_BASE+0xfffff
#if AlarmRecord
#define BASEDATA_BASE DATAFLASH_BASE+0x0d000
#define BASEDATA_END  DATAFLASH_BASE+0xfdfff
#define ALARMDATA_BASE DATAFLASH_BASE+0xfe000
#define ALARMDATA_END  DATAFLASH_BASE+0xfffff
#else
#define BASEDATA_BASE DATAFLASH_BASE+0x0d000
#define BASEDATA_END  DATAFLASH_BASE+0xfffff
#endif

//2004.07.23
#define DOWNLOADTIME_BASE DATAFLASH_BASE+0x00400
#define DOWNLOADTIME_END DATAFLASH_BASE+0x007f4
#define UPLOADTIME_BASE DATAFLASH_BASE+0x00800
#define UPLOADTIME_END DATAFLASH_BASE+0x00bf4
#define SETCHCOTIME_BASE DATAFLASH_BASE+0x00c00
#define SETCHCOTIME_END DATAFLASH_BASE+0x00ff4
//2004.07.23

#define DoubtDataSpace 110 

/*定义疑点数据格式*/
typedef struct{
	u_char speed;//每0.2秒速度
	u_char status;//每0.2秒8位状态
}DoubtData;
typedef struct{
	u_int 	DriverCode;
	CLOCK   StopTime;//实时时钟BCD码表示——6
	DoubtData data[100];//20秒数据20×5＝100
	u_char 	pt;
}DoubtDataBlock;

/*定义疲劳驾驶数据格式*/
//#define DriveMinuteLimit 5
//#define RestMinuteLimit 1
//#define DriveMinuteLimit 180
//#define RestMinuteLimit 20
typedef struct{
	Record_CLOCK  dt;
}OTDR_start;
typedef struct{
	Record_CLOCK  dt;
	u_int  TotalDistance;
	u_int MinuteNb;
	DRIVER driver;
}OTDR_end;
typedef struct{
	OTDR_start start;
	OTDR_end end;
}OTDR;

typedef struct{
	Record_CLOCK  dt;
	DRIVER driver;
}AnAlarmData;

/*定义状态字节中每位的含义*/
#define DOOR	   8
#define AIRPRESS   7
#define STATION    6
#define HEADLIGHT  5
#define RIGHTLIGHT 4
#define HORN	   3
#define BREAK      2
#define LEFTLIGHT  1
#define POWERON	   0
/* 定义TIMECHANGE中 */
#define YEAR_CHANGE		0
#define MONTH_CHANGE	1
#define DAY_CHANGE		2
#define HOUR_CHANGE		3
#define MINUTE_CHANGE	4
#define SECOND_CHANGE	5

/* 定义标志位中每位的含义 */
#define SpeedPowerOff    0

typedef struct
{//行驶记录
	Record_CLOCK dt;
}RecordData_start;
typedef struct
{//行驶记录
	Record_CLOCK dt;
	u_int DistancePulse;
	u_int DriverCode;
}RecordData_end;

//定义打印及显示的每分钟平均速度
typedef struct
{//定义时钟

	u_char hour;
	u_char minute;
	u_char speed;
	u_int  DriverCode;
	
} PrintSpeed;


#define MAX_BUFF 2048
#define RECVHEADSIZE 4
#define SENDHEADSIZE 8
#define SENDFILEBUFF 1000
#define SENDFILESUM	 2

//232
#define BAUDS4800 427
#define BAUDS9600 213
#define BAUDS38400 53
#define DataLength  256
#define CmdLength   17


#endif