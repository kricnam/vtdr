#ifndef menu_h
#define menu_h

#include "ibb3.h"
////////////////////////////////////////////////////////
//                        menu                        //
////////////////////////////////////////////////////////
#define NULL 0
//Һ����ʾ���ƿ�ģʽ���Ͷ���
#define Normal		0
#define Node 		1
#define BackLeaf 	2
#define ActLeaf 	3
#define Action		4
#define USBComm		5

//�������Ͷ���
#define PRINT		0
#define USB_SLAVE	1
#define USB_HOST	2
#define SHOW		3

//�������ݽṹ
typedef struct
{//����˵����Ľ��

	LCD_ZM * * content;//�˵�����ʾ������
	short ChildrenList;//���ӽ���б���ţ���1��ʾΪҶ�ӽ��
	short FatherList;//������б����
	short FatherNB;//��������б��е����
	void (* handler)(void);//Ҷ�ӽ��Ĵ�����򣽣�1��ʾû�д������
	
} MENU_NODE;

typedef struct
{
	MENU_NODE * ListPt;//����б��׵�ַ
	unsigned short NodeNumber;//������
	
} NODE_LIST;

typedef struct
{//LCD��ʾ��ǰ���ƿ�

	unsigned char mode;
	unsigned char ListNb;
	unsigned char NodeNb;
	unsigned int  KeepTime;
	
} LCDTCB;

typedef struct
{
	unsigned char type;
	unsigned char IfActionEnd;//��0δ��������1��������
	unsigned char LineNumber;
	unsigned char CurLine;
} ACTION_TCB;

extern void SaveDatatoUdisk();
extern void PrintAllData();
extern void DisplayAutoCode();
extern void DisplayDriverNumber();
extern void DisplayDriverCode();
extern void Displaywheel();
extern void DisplayStatusPolarity();
extern LCD_ZM *AutoCodeHZ2LCM(unsigned short data);
extern LCD_ZM *ASCII2LCM(unsigned char data);
extern void OKKeyHandler();
extern void SelectKeyHandler();
extern void DisplayNormalUI();
extern LCD_ZM *BCD2LCM(unsigned char data, unsigned char type);
extern void DisplayErrorCard();
extern void Display2DayOTDR();
extern void Display15MinAverageSpeed();
extern void WriteDataToUDiskMenu();
extern void DisplayProductVersion();
extern void DisplayTotalDistance();
extern void DisplayInteger(u_int integer,u_char row,u_char end_column,u_char len);
extern void DisplayAlarm();
#endif /* menu_h */
