#ifndef		DataManager_h
#define		DataManager_h

#define RoadType 0
#define OverSpeed 1

//AlarmFlag报警标志按位定义
#define OverLow       0
#define OverHigh      1
#define OverTime1     2
#define OverTime2     3
#define OpenDoorDrive 4

//定义行驶数据类型
#define START 		1
#define END			2
#define MinuteChg	3
#define SpeedChg	4
#define StatusChg	5
#define DATA_1min	6

#define NewOTData		0	//新记录
#define MergeLastData 	1	//合并上次记录

//Update4k类型
#define UpdateFlashTimes 0
#define UpdateFlashOnce	 1
/////////*******2003.10.06 panhui*********////////
#define UpdateFlashAll	 2
/////////*******2003.10.06 panhui*********////////

#define EnterHighWayTime 900 	//3分钟
#define OutOfHighWayTime 300 	//1分钟

//指针回溯
#define RecordFlagByte   16     //16个字节的预留标志

extern void SelfCheck();
extern void OverSpeedHandler();
extern void DoubtPointHandler();
extern void RunRecordHandler();
extern unsigned int ComputeDistance100m(unsigned int pulseNb);

#endif