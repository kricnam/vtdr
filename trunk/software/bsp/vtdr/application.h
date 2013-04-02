#ifndef application_h
#define application_h




#define GetSpeedSelf    	0
#define POWERON_LINE_EN     0
#define StatusPol           0
#define StationStamp_EN     0
#define guizhoufile    	    0
#define guizhou             1
#define GPS                 0


#define OpenDoorDeal    	1
#define RTC8025             1     //ʱ��оƬѡ�񿪹�
#define GetSpeedStatusBy232 0     //ͨ��ں�������ͨѶ��ȡ�ٶȺ�ȫ��״̬����
//#define WATCH_DOG_EN    	1     // ���Ź�����
#define RPM_EN				0     //������ת�ٿ���
#define SectionAlarm_EN     0     //��·�α�������
#define OpenDoorAlarm		0     //������ʻ��������
#define Test                0	  //���Թ�װ
#define Status14			0     //����8��״̬
//**************************************//
#define AlarmRecord         0     //���ٺ�ƣ�ͼ�ʻ������¼
////////////////////////////////////////////////////////////////////////

//2004.03.02 panhui �人�������߸�Ϊ2Сʱ���п�����ʻ������
#if OpenDoorAlarm
#define GotoSleepTime 36000     //����ʱ�䣽120����
#else
#define GotoSleepTime 1500     //����ʱ�䣽300�룽5����
#endif

#define DoorNBLimit   6       //200ms�п����ź���Ч�ĸ�������

//�����ж����ȼ�
#define USART1_INT_PRI	4
#define IRQ0_INT_PRI	7       //USB
#define IRQ1_INT_PRI    3       //ENGINE SPEED
#define IRQ2_INT_PRI    6       //SPEED
#define TC0_INT_PRI     0       //TICK
#define Clock_PRI     	5       //CLOCK 
#define DriverCard_PRI 	5		//DRIVERCARD
#define TC2_INT_PRI     6       //��ʱ��
#define TC1_INT_PRI     5       //door��ʱ��


typedef struct
{

	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	
} CLOCK;
typedef struct
{

	int year;
	int month;
	int day;
	int time;
	
} DateTime;

typedef struct
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char type;
	unsigned char reserved;
} UpDownCLOCK;

typedef struct
{

	unsigned short type;
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	
} Record_CLOCK;

typedef struct
{
	unsigned char speed;
	unsigned long bound;
} RoadSection;
typedef struct
{

	unsigned short mark;//*�����֡���2
	unsigned char  sn[22];//��Ʒ���кš���22(24)
	unsigned long CHCO;//��������ϵ��4(28)
	unsigned char  AutoType[12];//�������͡���12(40)
	unsigned char  AutoVIN[18];//����VIN�š���18(58)
	unsigned char  AutoCode[12];//���ƺš���12(70)
	unsigned char  AutoSort[12];//���Ʒ��ࡪ��12(82)
	unsigned short  CodeColor;//������ɫ����2(84)
	unsigned long  DriverCode;//��ʻԱ���롪��4(88)
	unsigned char  DriverLisenseCode[20];//��ʻ֤���롪��20(108)
	unsigned short status_polarity;//״̬���ԡ���2(110)
	unsigned short status_mask;//����״̬����2(112)
	unsigned char  OverSpeedTimeLimit;//����ʱ�����ޣ�0��255�룩����1(113)
	unsigned char  AlarmSound;//��������ѡ��0x00-����0xFF-������1(114)
	unsigned char  LowSpeedLimit;//����·�ٶ����ޡ���1(115)
	unsigned char  HighSpeedLimit;//����·�ٶ����ޡ���1(116)
	CLOCK   time;//ʵʱʱ��BCD���ʾ����8(124)
	CLOCK   InstallTime;//��װ����BCD���ʾ����8(132)
	unsigned char  PulseNumber;//���ٴ�����ÿת�����������
	unsigned char  RPM_Pulse;
	unsigned char  Door1Type;
	unsigned char  Door2Type;
	unsigned char  reserved[112];//������Ԥ������ʱ���ã�(254)
	unsigned char  DriveHour24;//ƣ�ͼ�ʻʱ��
	unsigned char  RestHour24;
	unsigned char  DriveHour;	//ƣ�ͼ�ʻʱ������
	unsigned char  RestMinute;	//ƣ�ͼ�ʻ������Ϣʱ������
	unsigned char  wakeGPStime;
	unsigned short  IBBType;//��¼�Ǵ��롪��2(256)

	unsigned char SectionNumber;
	RoadSection section[20];
	//2004.03.23
	unsigned char  reserved1[92];//Ϊ�˱�֤��վ������·�߷ֶ����ôӵ�ַ0x200��ʼд
	unsigned char SectionNumber1;//��վ����·�߷ֶεĶ���
	RoadSection section1[20];//·�μ����ٶ�����
} StructPara;

typedef struct
{
	unsigned long  DriverCode;//��ʻԱ���롪��4(88)
	unsigned char  DriverLisenseCode[20];//��ʻ֤���롪��20(108)
} DRIVER;

typedef struct
{//�����ṹ

	unsigned long  BaseAddr;//��ʼ��ַ
	unsigned long  EndAddr;//�����ַ
	unsigned long  CurPoint;//��ǰָ���ַ,ָ����һ����ݿɴ�ŵ�λ��
	
} StructPT;

typedef struct
{

	unsigned short  Available;//=0��ʾû��ʹ�ô������>0��ʾ�������Ч
	StructPT DoubtPointData;
	StructPT OverSpeedRecord;
	StructPT PowerOffRunRecord;
	StructPT RunRecord360h;
	StructPT BaseData;
	StructPT OilData;
	StructPT RPMData;
	StructPT TemperatureData;
	StructPT OilPressData;
	unsigned long	 TotalDistance;
	CLOCK	 LastUploadTime;
	unsigned long	 DriverCode;//��ʻԱ����
	unsigned char   DriverLisenseCode[20];//��ʻ֤����
	unsigned char 	 InOSAlarmCycle;//���ڷ�·�α��������С���־��0xaa��վ������0x55��վ������
	unsigned long	 OSAlarmAddupDistance;//��·�α���·���ۼ�

} PartitionTable;

/* ���������Ч�ָ�λ���� */
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

/* ������ݷ���� */
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

#define PARAMETER_BASE    ((StructPara *)0x00000000)   //((StructPara *)(DATAFLASH_BASE+0x00000000))
#define PartitionTable_BASE  ((PartitionTable *)0x00001000)//((PartitionTable *)(DATAFLASH_BASE+0x00001000))
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

/*�����ɵ���ݸ�ʽ*/
typedef struct{
	unsigned char speed;//ÿ0.2���ٶ�
	unsigned char status;//ÿ0.2��8λ״̬
}DoubtData;
typedef struct{
	unsigned long 	DriverCode;
	CLOCK   StopTime;//ʵʱʱ��BCD���ʾ����6
	DoubtData data[100];//20�����20��5��100
	unsigned char 	pt;
}DoubtDataBlock;

/*����ƣ�ͼ�ʻ��ݸ�ʽ*/
//#define DriveMinuteLimit 5
//#define RestMinuteLimit 1
//#define DriveMinuteLimit 180
//#define RestMinuteLimit 20
typedef struct{
	Record_CLOCK  dt;
}OTDR_start;
typedef struct{
	Record_CLOCK  dt;
	unsigned long  TotalDistance;
	unsigned long MinuteNb;
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

/*����״̬�ֽ���ÿλ�ĺ���*/
#define DOOR	   8
#define AIRPRESS   7
#define STATION    6
#define HEADLIGHT  5
#define RIGHTLIGHT 4
#define HORN	   3
#define BREAK      2
#define LEFTLIGHT  1
#define POWERON	   0
/* ����TIMECHANGE�� */
#define YEAR_CHANGE		0
#define MONTH_CHANGE	1
#define DAY_CHANGE		2
#define HOUR_CHANGE		3
#define MINUTE_CHANGE	4
#define SECOND_CHANGE	5

/* �����־λ��ÿλ�ĺ��� */
#define SpeedPowerOff    0

typedef struct
{//��ʻ��¼
	Record_CLOCK dt;
}RecordData_start;
typedef struct
{//��ʻ��¼
	Record_CLOCK dt;
	unsigned long DistancePulse;
	unsigned long DriverCode;
}RecordData_end;

//�����ӡ����ʾ��ÿ����ƽ���ٶ�
typedef struct
{//����ʱ��

	unsigned char hour;
	unsigned char minute;
	unsigned char speed;
	unsigned long  DriverCode;
	
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
