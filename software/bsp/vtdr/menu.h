#ifndef menu_h
#define menu_h

#include "ibb3.h"
////////////////////////////////////////////////////////
//                        menu                        //
////////////////////////////////////////////////////////
#define NULL 0
//液晶显示控制块模式类型定义
#define Normal		0
#define Node 		1
#define BackLeaf 	2
#define ActLeaf 	3
#define Action		4
#define USBComm		5

//动作类型定义
#define PRINT		0
#define USB_SLAVE	1
#define USB_HOST	2
#define SHOW		3

//定义数据结构
typedef struct
{//定义菜单树的结点

	LCD_ZM * * content;//菜单项显示的内容
	short ChildrenList;//孩子结点列表序号＝－1表示为叶子结点
	short FatherList;//父结点列表序号
	short FatherNB;//父结点在列表中的序号
	void (* handler)(void);//叶子结点的处理程序＝－1表示没有处理程序
	
} MENU_NODE;

typedef struct
{
	MENU_NODE * ListPt;//结点列表首地址
	unsigned short NodeNumber;//结点个数
	
} NODE_LIST;

typedef struct
{//LCD显示当前控制块

	unsigned char mode;
	unsigned char ListNb;
	unsigned char NodeNb;
	unsigned int  KeepTime;
	
} LCDTCB;

typedef struct
{
	unsigned char type;
	unsigned char IfActionEnd;//＝0未结束；＝1动作结束
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
