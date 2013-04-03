#ifndef		DataManager_h
#define		DataManager_h

#define RoadType 0
#define OverSpeed 1

//AlarmFlag������־��λ����
#define OverLow       0
#define OverHigh      1
#define OverTime1     2
#define OverTime2     3
#define OpenDoorDrive 4

//������ʻ�������
#define START 		1
#define END			2
#define MinuteChg	3
#define SpeedChg	4
#define StatusChg	5
#define DATA_1min	6

#define NewOTData		0	//�¼�¼
#define MergeLastData 	1	//�ϲ��ϴμ�¼

//Update4k����
#define UpdateFlashTimes 0
#define UpdateFlashOnce	 1
/////////*******2003.10.06 panhui*********////////
#define UpdateFlashAll	 2
/////////*******2003.10.06 panhui*********////////

#define EnterHighWayTime 900 	//3����
#define OutOfHighWayTime 300 	//1����

//ָ�����
#define RecordFlagByte   16     //16���ֽڵ�Ԥ����־

extern void SelfCheck();
extern void OverSpeedHandler();
extern void DoubtPointHandler();
extern void RunRecordHandler();
extern unsigned long ComputeDistance100m(unsigned long pulseNb);

#endif
