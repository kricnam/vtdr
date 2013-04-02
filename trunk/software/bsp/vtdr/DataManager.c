//*----------------------------------------------------------------------------
//*      ���ݴ����ӳ���
//*----------------------------------------------------------------------------
//* File Name           : DataManager.c
//* Object              : ��¼�ǲɼ����ٶ�״̬���ݵĴ���ͱ���
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
extern u_char TimeChange;	//ʱ��仯��־
extern u_int AddupSpeed;
extern u_short SpeedNb;
extern PartitionTable pTable;
extern StructPara Parameter;
extern u_int PulseTotalNumber;	/*������ʻ��������*/
extern u_short STATUS;			/*16��״̬*/
extern u_int PulseNB_In1Sec;     //ÿ0.2������ǰ��1�����ۼ��ٶ�������
extern u_char PowerOn;
extern u_int CurEngine;
extern u_short DriveMinuteLimit;       //ƣ�ͼ�ʻ��ʻʱ������
extern u_short RestMinuteLimit;        //ƣ�ͼ�ʻ������Ϣʱ������
extern u_int CurPulse; 
 
DoubtDataBlock ddb;			//��ǰ�ɵ����ݿ�
u_char InRecordCycle=0;		//�Ƿ��ڼ�¼���ݹ�����
u_char InFlashWriting=0;	//��FLASHд������
u_int Tick02NB;				//����ͣ��֮��0.2s�ĸ���
OTDR_end otdrEND;			//ƣ�ͼ�ʻ��¼����������
u_int AddupSpeed = 0;		//1�����ٶ��ۼ�
u_short SpeedNb = 0;		//1�����ٶ�ֵ����
u_char PowerOnTime=0;		//�ϵ����ʱ��
u_char OTRecordType=0;		//����ƣ�ͼ�ʻ��¼����
u_int LastDistance;			//�ϴ�ƣ�ͼ�ʻ��¼�ۼ����
u_char STATUS1min;			//1����״̬��
DRIVER CurrentDriver;		//��ǰ˾��
DRIVER RecordDriver;		//��¼˾��
Record_CLOCK PowerOnDT;     //�ϵ�����ʱ��
RecordData_end rec_end;
u_char FinishFlag=0;

extern u_char LargeDataBuffer[];
u_short *DoubtData_4k = (u_short *)(&(LargeDataBuffer[12*1024]));//[2*1024];
u_char *OTDR_4k = &(LargeDataBuffer[16*1024]);//[4*1024];
u_short *BaseData_4k = (u_short *)(&(LargeDataBuffer[20*1024]));//[2*1024];

//*----------------------------------------------------------------------------
//* Function Name       : Task_GetData
//* Object              : �Լ����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : �Ƚ������ַ����Ƿ���ͬ
//* Input Parameters    : ���Ƚϵ������ַ���str1,str2,����length
//* Output Parameters   : 1�����ַ�����ͬ��0�����ַ�����ͬ
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : д������
//* Input Parameters    : none
//* Output Parameters   : �Ƿ�ɹ�
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int WriteParameterTable(StructPara *para)
{
	flash_word *data;
	flash_word *p;
	int i,size;

	//����4k
	p = (flash_word *)PARAMETER_BASE;
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,p))
		return(FALSE);
	
	
	//��������д��FLASH
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
//* Object              : д������
//* Input Parameters    : none
//* Output Parameters   : �Ƿ�ɹ�
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int WritePartitionTable(PartitionTable *ptt)
{
	flash_word *data;
	flash_word *p;
	int i,size;

	//����4k
	p = (flash_word *)PartitionTable_BASE;
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,p))
		return(FALSE);
	
	//��������д��FLASH
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
//* Object              : ��ʼ��������
//* Input Parameters    : none
//* Output Parameters   : д�������Ƿ�ɹ�
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
	/////////////��ָֹ���ܷ�/////////////////////////////

	//��������д��FLASH
	return (1);
	

}
//*----------------------------------------------------------------------------
//* Function Name       : InitializeTable
//* Object              : ��ʼ��������
//* Input Parameters    : u_char parti�����Ƿ����»ָ�������
//                        u_char para�����Ƿ����»ָ�������
//                        u_char change_set�����Ƿ��������ó��ƺţ�
//                                             �����ۼ����������
//* Output Parameters   : ��ʼ��������Ͳ������Ƿ�ɹ�
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int InitializeTable(u_char parti,u_char para,u_char change_set)
{
	int i;

	//��ȡ������	
	Parameter = *PARAMETER_BASE;
	if(para)
	{
		Parameter.mark=0x30aa;//*�����֡���2
		Parameter.IBBType=0x3000;//��¼�Ǵ��롪��2
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
			pTable.DriverCode = 0;//��ʻԱ����
			for( i = 0; i < 20; i++)
				pTable.DriverLisenseCode[i] = 0;//��ʻ֤����
			pTable.InOSAlarmCycle = 0;//���ڷ�·�α��������С���־
			pTable.OSAlarmAddupDistance = 0;//��·�α���·���ۼ�
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
		/////////////��ָֹ���ܷ�/////////////////////////////
		if(change_set)
		{
			pTable.TotalDistance = 0;
			pTable.DriverCode = 0;//��ʻԱ����
			for( i = 0; i < 20; i++)
				pTable.DriverLisenseCode[i] = 0;//��ʻ֤����
			pTable.InOSAlarmCycle = 0;//���ڷ�·�α��������С���־
			pTable.OSAlarmAddupDistance = 0;//��·�α���·���ۼ�
		}
		if( !WritePartitionTable(&pTable) )
			return(0);
	
	}
	
	return (1);
}
//*----------------------------------------------------------------------------
//* Function Name       : UpdateParameterPartition
//* Object              : ���²���������
//* Input Parameters    : none
//* Output Parameters   : �²����������Ƿ�ɹ�
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int UpdateParameterPartition()
{
	u_int sector_addr;
	sector_addr = 0;
	flash_word *data;
	flash_word *p;
	int i,size;
	
	//����FLASH
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE, (flash_word *)sector_addr))
		return(0);
	//ˢ�²�����
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
	
	//ˢ�·�����
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
//* Object              : FLASH��4K�����ڴ棬������ǰ4k������������ȷ���Ƿ�
//                        ��ָ��֮ǰ������д��FLASH
//* Input Parameters    : p������ǰ����ָ��
//*                       Buffer�����ڴ��д����µ�4k�������׵�ַ
//*                       type��������
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int Update4k(u_short *p,u_short *Buffer,u_char type)
{
	u_short i;
	flash_word *sector_addr;
	sector_addr = (flash_word *)((u_int)p & 0xfffff000);
	//4k�����ڴ�
	for(i=0;i<2*1024;i++)
		Buffer[i] = sector_addr[i];

	if(type == UpdateFlashOnce)
		return(1); 
		
	//����FLASH
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,sector_addr))
		return(0);
	//ָ��֮ǰ����������д��FLASH
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
//* Object              : дһ���ɵ����ݵ�FLASH��
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : curTime
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int WriteDoubtDataToFlash()
{
	u_short i; 
	u_short *p4k;
	p4k = (u_short *)(pTable.DoubtPointData.CurPoint);
	if((InFlashWriting&(1<<DOUBTPOINTDATA))==0)
	{//���û�п�ʼFLASHд�����ȶ��뵱ǰ���ɵ�4K���ݳ�פ�ڴ�
		InFlashWriting |= 1<<DOUBTPOINTDATA;//����ɵ�����Ϊ����дFLASH��
		if(!Update4k(p4k,DoubtData_4k,UpdateFlashTimes))
			return (0);
	}
	
	//д��ǰ��ͣ���ɵ㵽FLASH��
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
		
		//�鿴ָ���Ƿ�Ҫת������һ��4k������
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
		
		//�鿴ָ���Ƿ�Ҫת������һ��4k������
		if(p4k != (u_short *)((u_int)p&0xff000))
		{
			if(!Update4k(p,DoubtData_4k, UpdateFlashTimes))
				return (0);
			p4k = (u_short *)((u_int)p&0xff000);
		}
	}
	
	//�����ɵ�����ָ��
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
//* Object              : BCD��ת��Ϊʮ������
//* Input Parameters    : bcd������дת����BCD��
//* Output Parameters   : ת�����ʮ������
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : �ж��Ƿ�������������
//* Input Parameters    : time��������µ�ʱ��ָ��
//*                       end������һ��ƣ�ͼ�¼����������ָ��
//* Output Parameters   : 0�������ǣ�1������
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : �ж�ʱ����
//* Input Parameters    : time��������µ�ʱ��ָ��
//*                       end������һ��ƣ�ͼ�¼����������ָ��
//* Output Parameters   : ʱ����ֵ�����ӣ���������20����ʱ����FF
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : ����ָ������ƫ����
//* Input Parameters    : pt������������ṹ
//*                       inc�����ۼ�ֵ
//* Output Parameters   : �����Ľ��
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : ��ȡƣ�ͼ�ʻ���������ݲ��Ҹ������ݻ�����
//* Input Parameters    : none
//* Output Parameters   : none
//* Global Parameters   : otdrEND,InFlashWriting
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int Get_otdrEND(OTDR_start *start,OTDR_end *end)
{
	if((InFlashWriting&(1<<RUNRECORD360h))==0)
	{//���û�п�ʼдFLASH�����ȶ������һ�����ݵĽ���������
		return(GetOTDR( pTable.RunRecord360h.CurPoint, start, end ));
	}
	else{
		return(2);
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : Write4kToFlashOTDR
//* Object              : �Ȳ�����д4k�ڴ����ݵ�FLASH��
//* Input Parameters    : p������ǰ����ָ��
//*                       Buffer�����ڴ��д����µ�4k�������׵�ַ
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int Write4kToFlashOTDR(u_short *p,u_short *Buffer)
{
	//����FLASH
	if(!flash_sst39_erase_sector((flash_word *)DATAFLASH_BASE,(flash_word *)p))
		return (0);

	return(Write4kToFlash(p,Buffer));
}
//*----------------------------------------------------------------------------
//* Function Name       : Write4kToFlash
//* Object              : д4k�ڴ����ݵ�FLASH��
//* Input Parameters    : p������ǰ����ָ��
//*                       Buffer�����ڴ��д����µ�4k�������׵�ַ
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int Write4kToFlash(u_short *p,u_short *Buffer)
{
	u_short *inside_p,*flash_p;
	u_short data;
	inside_p = (u_short *)((u_int)p & 0x00fff);//4k�ڵ�ַָ��
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
//* Object              : дOTDR���ݵ�OTDR�ڴ����������Ҫ������һ4k
//* Input Parameters    : buf������д��������׵�ַ
//*                       len�������ݳ���
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
		{//�л�����һ��4k
			//���µ�ַָ��
			p=(u_char *)(OTDR_4k);
			last_pos = pTable.RunRecord360h.CurPoint&0xfffff000;
			pos = (pTable.RunRecord360h.CurPoint&0xff000) + 0x01000;
			if(pos>(pTable.RunRecord360h.EndAddr&0xff000))
				pos = pTable.RunRecord360h.BaseAddr;
			else
				pos += (u_int)DATAFLASH_BASE; 
			pTable.RunRecord360h.CurPoint = pos;
		
			//д��ǰ4k��������FLASH��
			Write4kToFlashOTDR((u_short *)last_pos,(u_short *)OTDR_4k);
			//���»�����
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
//* Object              : ����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int WriteZeroToOTDREndData(u_char zeroNB)
{
	u_int pt;
	u_char i,buf[20];
	int inc;
	
	if((InFlashWriting&(1<<RUNRECORD360h))==0)
	{//���û�п�ʼдFLASH�����ȶ������һ�����ݵĽ���������
		InFlashWriting |= 1<<RUNRECORD360h;//�������Ϊ����дFLASH��
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
//* Object              : дһ��ƽ���ٶ����ݵ�FLASH��
//* Input Parameters    : V������д���ƽ���ٶ�ֵ
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
	{//�л�����һ��4k
		//���µ�ַָ��
		p=(u_char *)(OTDR_4k);
		last_pos = pTable.RunRecord360h.CurPoint&0xfffff000;
		pos = (pTable.RunRecord360h.CurPoint&0xff000) + 0x01000;
		if(pos>(pTable.RunRecord360h.EndAddr&0xff000))
			pos = pTable.RunRecord360h.BaseAddr;
		else
			pos += (u_int)DATAFLASH_BASE; 
		pTable.RunRecord360h.CurPoint = pos;
	
		//д��ǰ4k��������FLASH��
		Write4kToFlashOTDR((u_short *)last_pos,(u_short *)OTDR_4k);
		//���»�����
		Update4k((u_short *)pos,(u_short *)OTDR_4k,UpdateFlashOnce);
	}
	else
		pTable.RunRecord360h.CurPoint++;	 

}
//*----------------------------------------------------------------------------
//* Function Name       : WriteOTDRStartData
//* Object              : 360Сʱƣ�ͼ�ʻ������ʼ������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : curTime
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int WriteOTDRStartData()
{
	u_char *p;
	u_int pos;
	u_char i,buf[8];
	int inc;
	
	if((InFlashWriting&(1<<RUNRECORD360h))==0)
	{//���û�п�ʼдFLASH�����ȶ������һ�����ݵĽ���������
		InFlashWriting |= 1<<RUNRECORD360h;//�������Ϊ����дFLASH��
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
//* Object              : 360Сʱƣ�ͼ�ʻ������ʼ������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : curTime
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : �ж��Ƿ���һ���ɵ�����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : Tick02NB,ddb
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              :360Сʱƽ���ٶ���ʻ��¼��ͳ��ƣ�ͼ�ʻ
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : CurSpeed,curTime
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void RunRecord360Handler()
{
	u_char AverageV; 
	u_char flag=InRecordCycle&(1<<RUNRECORD360h);
	u_char space=0xff;
	int succ;
	
	
	//��¼��ǰ�ɵ�����
	ddb.data[ddb.pt].speed = (u_char)CurSpeed;
	ddb.data[ddb.pt].status = CurStatus;
	ddb.pt++;
	if(ddb.pt>99)
		ddb.pt = 0;
		
	if(flag==0)//û�м�¼�����У�speed==0��
	{
//		if((CurSpeed > 0)&&(LastSPE > 0))
		if((CurSpeed - DeltaSpeed==0)&&(CurSpeed>0))
		{//��¼��ʼ��
			InRecordCycle |= 1<<RUNRECORD360h;//�޸ļ�¼���ݱ�־
			
			Tick02NB = 0;

			//�ж��Ƿ�������һ����¼�������ʱ���Ƿ񲻳���20����
			OTDR_start CurOTDR_start;
			OTDR_end CurOTDR_end;
			succ=Get_otdrEND(&CurOTDR_start,&CurOTDR_end);
			if(succ==1)//��FLASH�ж�����һ����¼
				otdrEND = CurOTDR_end;
				
			//����������¼��˾��������ȣ��Ҳ���δ֪˾��	
			if(succ && (otdrEND.driver.DriverCode != 0) && 
				(otdrEND.driver.DriverCode==RecordDriver.DriverCode))	
				space = JudgeTimeSpace(&curTime,&otdrEND);
				
			//һ����¼�ڣ�һ���ϵ磩���������ٶ����ߵ�˾������Ϊδ֪˾��
			if((succ == 2) && (otdrEND.driver.DriverCode == 0)&&
				(RecordDriver.DriverCode ==0 ))
				space = JudgeTimeSpace(&curTime,&otdrEND);
				
			//һ����¼�ڣ�һ���ϵ磩���������ٶ�����,ǰ��Ϊδ֪˾�������Ϊ��֪
			if((succ == 2) && (otdrEND.driver.DriverCode == 0)&&
			(RecordDriver.DriverCode !=0 )&&(PowerOnDT.type==0xefef))
				space = JudgeTimeSpace(&curTime,&otdrEND);
				
			//ʱ�������޶�ʱ���ڣ��ϲ���һ����¼
			if((space<=RestMinuteLimit)&&(PowerOnDT.type!=0))
			{
				OTRecordType = MergeLastData;
				LastDistance = otdrEND.TotalDistance;
				//��space���㵽endָ��λ��
				WriteZeroToOTDREndData(space);
				otdrEND.MinuteNb += space; 
			}
			else//�¼�¼
			{
				if((succ == 2) && (otdrEND.driver.DriverCode == 0)&&
					(RecordDriver.DriverCode !=0 )&&(PowerOnDT.type==0xefef))
				{
					//�޸�ǰһ����¼��˾��
					//ModifyDriverToLastOTDR();
					otdrEND.driver = RecordDriver;
					 
				}
				//��ʼдһ���¼�¼
				WriteOTDRStartData();
				OTRecordType = NewOTData;
				otdrEND.MinuteNb = 0;
			}
			//��otdrEND���ݽṹ��ֵ
			otdrEND.TotalDistance = PulseTotalNumber;
			
		}
	}
	else//���ڼ�¼���ݣ�speed>0��
	{
		//*��¼1����ƽ���ٶ�
		AddupSpeed +=CurSpeed;
		SpeedNb ++;
		if(TimeChange & (0x01<<MINUTE_CHANGE))//��һ����
		{
			AverageV=AddupSpeed/SpeedNb;
			AddupSpeed = 0;
			SpeedNb = 0;
			otdrEND.MinuteNb ++;
			WriteAverageSpeed(AverageV);
		}
		
		Tick02NB ++;
		
		//*��¼ͣ��������
		if(((CurSpeed == 0)&&(PulseNB_In1Sec == 0))||((CurSpeed == 0)&&(!PowerOn)))
		{
			if(IsDoubtPointData())
				WriteDoubtDataToFlash();//дһ��ͣ���ɵ����ݵ�����flash��
			InRecordCycle &= ~(1<<RUNRECORD360h);//�޸ļ�¼���ݱ�־
			//��¼�������ݵ�
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
//* Object              :����ǰ��¼360Сʱƽ���ٶ���ʻ��¼��ͳ��ƣ�ͼ�ʻ�ڴ�
//                       ���浽FLASH��
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void FinishOTDRToFlash()
{
	u_int p;
	otdrEND.driver = RecordDriver;
	if((InRecordCycle&(1<<RUNRECORD360h))!=0)
	{
		//��¼�������ݵ�
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
//* Object              : ÿ����ʻ��¼����ʱ����¼�м�¼��δ֪˾�����������忨��
//						  �����֪˾���ļ�¼�޸���˾������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
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
	
	//�ɵ��¼
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
		i++;//2003.11.11,panhui(��������)
	}while((cmp1>0)&&(cmp2>=0)&&(i<97));
	
	//360Сʱƽ���ٶȼ�¼
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteBaseDataToFlash
//* Object              : ����ʻ��¼д������FLASH
//* Input Parameters    : buf�������ݻ�����ָ��
//                        len����u_short�����ݳ���
//						  type������������
//* Output Parameters   : if data write TRUE or FALSE
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
int WriteBaseDataToFlash(u_short *buf,u_char len,u_char type)
{

	u_char i;
	u_int pos,UpdatePos;
	
	if((InFlashWriting&(1<<BASEDATA))==0)
	{//���û�п�ʼдFLASH
		if(type!=START)
			return(0);
		InFlashWriting |= 1<<BASEDATA;//�������Ϊ����дFLASH��
		pos = pTable.BaseData.CurPoint;
		if(!Update4k((u_short *)pos,BaseData_4k,UpdateFlashTimes))
			return(0);

		/////////*******2003.10.06 panhui*********////////
		UpdatePos = pos + RecordFlagByte*2;
		if(UpdatePos > pTable.BaseData.EndAddr)
			UpdatePos = pTable.BaseData.BaseAddr + (UpdatePos-pTable.BaseData.EndAddr)-1;
		if((pos&0xff000)!=(UpdatePos&0xff000))
		{//��ҳ
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
		{//��Ҫ������һ4k
			if(pos>pTable.BaseData.EndAddr)
				pos = pTable.BaseData.BaseAddr;
			if(!Update4k((u_short *)pos,BaseData_4k,UpdateFlashTimes))
				return(0);
			
		}*/
		if(((pos&0xff000)!=(UpdatePos&0xff000))&&((UpdatePos&0x00fff)==0))
		{//��Ҫ������һ4k
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
		
		//�޸Ŀ��ܵġ�δ֪˾�����ɵ��360ƽ���ٶȼ�¼
		if(PowerOnDT.type==0xefef)
			ModifyUnknownDriver();
		
		PowerOnDT.type = 0;

	}
	return(1);

}
//*----------------------------------------------------------------------------
//* Function Name       : BaseDataHandler
//* Object              : ������ϸ���ݴ������¼
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : PowerOnTime,STATUS1min,TimeChange,InRecordCycle,curTime
//* �޸ĵ�ȫ�ֱ���      : PowerOnTime,STATUS1min,InRecordCycle   
//*----------------------------------------------------------------------------
void BaseDataHandler()
{
	int i,pt;
	u_short data;
	RecordData_start rec_start;

	if((InRecordCycle&(1<<BASEDATA))==0)
	{//���û�п�ʼ��¼������ʻ����
		
//		if((PowerOn&&(PowerOnTime>=5))||((CurSpeed - DeltaSpeed==0)&&(CurSpeed>0)))
		if(CurSpeed>0)
		{//�ϵ�ʱ�̣���¼һ�����
			PowerOnTime = 0;
			PulseTotalNumber = 0;//���	����
			
			InRecordCycle |= 1<<BASEDATA;//��ǡ���ʼ��¼���ݡ�
			STATUS1min = 0;//2003.10.23,panhui
			
			//׼����ʼ����
			rec_start.dt.type = 0xFEFE;
			rec_start.dt.year = curTime.year;
			rec_start.dt.month = curTime.month;
			rec_start.dt.day = curTime.day;
			rec_start.dt.hour = curTime.hour;
			rec_start.dt.minute = curTime.minute;
			rec_start.dt.second = curTime.second;
			
			PowerOnDT = rec_start.dt;//��ʼ���ϵ�ʱ��ͱ�־
			if(CurrentDriver.DriverCode == 0)
				PowerOnDT.type = 0xefef;
			
			//�жϵ�ǰָ���Ƿ�Ϊż������Ϊ��������һ���ֽ�
			if((pTable.BaseData.CurPoint&1)!=0)
			{
				pTable.BaseData.CurPoint++;
				if(pTable.BaseData.CurPoint>pTable.BaseData.EndAddr)
					pTable.BaseData.CurPoint=pTable.BaseData.BaseAddr;
			}
			
			//��ʻ��¼���д��dataflash
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
	{//���ڼ�¼��ʻ����
		if((CurSpeed == 0)&&(!PowerOn))
		{//�ٶ�Ϊ��ʱ�ϵ磬��¼һ��������
			InRecordCycle &= ~(1<<BASEDATA);//��ǡ�������¼���ݡ�
			//׼����������
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
		{//����˾������¼һ��������
			//׼����������
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
			
			//��ʻ��¼������д��dataflash
			WriteBaseDataToFlash((u_short *)(&rec_end),(sizeof(RecordData_end))/2,END);
			
			//׼����ʼ����
			rec_start.dt.type = 0xFEFE;
			rec_start.dt.year = curTime.year;
			rec_start.dt.month = curTime.month;
			rec_start.dt.day = curTime.day;
			rec_start.dt.hour = curTime.hour;
			rec_start.dt.minute = curTime.minute;
			rec_start.dt.second = curTime.second;
			PowerOnDT = rec_start.dt;//��ʼ���ϵ�ʱ��ͱ�־
			
			//��ʻ��¼���д��dataflash
			WriteBaseDataToFlash((u_short *)(&rec_start),(sizeof(RecordData_start))/2,START);
			RecordDriver = CurrentDriver;
			PulseTotalNumber = 0;

		}
		else
		{//1���Ӽ�¼һ���ٶ�״̬����
			STATUS1min |= CurStatus;
			if(TimeChange & (0x01<<SECOND_CHANGE))//��һ����
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
//* Object              : �������м�¼
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : PowerOnTime,STATUS1min,TimeChange,InRecordCycle,curTime
//* �޸ĵ�ȫ�ֱ���      : PowerOnTime,STATUS1min,InRecordCycle   
//*----------------------------------------------------------------------------
void FinishAllRecord()
{
	if(FinishFlag)
	{
		//��ʻ��¼������д��dataflash
		WriteBaseDataToFlash((u_short *)(&rec_end),(sizeof(RecordData_end))/2,END);
		RecordDriver = CurrentDriver;//�ָ���¼˾��
		FinishFlag = 0;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : GetOTDRDataFromFlash
//* Object              : �Ӹ����ĵ�ַ��ȡinc���ȵ�ƣ�ͼ�ʻ����
//*                       ����ŵ�BUF��
//* Input Parameters    : p���������ĵ�ַ
//*                       inc�������ݳ��ȣ����ݷ���ȷ�������ݵķ���
//*                       ע�⣺inc����Ϊ������ż��
//*                       buf����������
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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

	//���ָ��Ϊż��
	if(((u_int)p&1)==0)
	{
		for(i=0;i<inc/2;i++){
			buf[2*i] = (u_char)(*p);
			buf[2*i+1] = (u_char)((*p)>>8);
			p++;
			if((u_int)p > pTable.RunRecord360h.EndAddr)
				p = (u_short *)pTable.RunRecord360h.BaseAddr;
		}
		//���incΪ����
		if((inc&1)==1)
		{
//			p=(u_short *)((u_int)p-1);
			data = *p;
			buf[inc-1] = (u_char)data;
			if((u_int)p > pTable.RunRecord360h.EndAddr)
				p = (u_short *)pTable.RunRecord360h.BaseAddr;
		}
	}
	//���ָ��Ϊ����
	else{
		//���incΪż��
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
		//���incΪ����
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
//* Object              : �ж�ʱ�������Ƿ���ȷ
//* Input Parameters    : ʱ��ָ��
//* Output Parameters   : �Ƿ���һ����ȷ��ʱ������
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : �ж�ʱ�������Ƿ���ȷ
//* Input Parameters    : ʱ��ָ��
//* Output Parameters   : �Ƿ���һ����ȷ��ʱ������
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : ��ȡ��ǰ��ƣ�ͼ�ʻ��¼
//* Input Parameters    : p������ǰָ��;
//*                       s����ƣ�ͼ�ʻ��¼��ʼ����;
//*                       e����ƣ�ͼ�ʻ��¼��������;
//* Output Parameters   : �Ƿ���һ��ƣ�ͼ�ʻ��¼
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
//* Object              : ��ȡ��ǰ˾���������������ڵ�ƣ�ͼ�ʻ��¼
//* Input Parameters    : ��¼ƣ�ͼ�ʻ��¼������ָ�루�����С��15����
//* Output Parameters   : ƣ�ͼ�ʻ��¼�ĸ���
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
//*----------------------------------------------------------------------------
u_char GetOverTimeRecordIn2Days(OTDR *record)
{
	OTDR_start CurOTDR_start;
	OTDR_end CurOTDR_end;
	StructPT spt;
	u_char next=0;//=0�������¿�ʼһ���¼�¼
				  //=1�����ȴ���һ����¼
	
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
		//��ȡ��ǰ��ƣ�ͼ�ʻ��¼
		if(!GetOTDR( p, &CurOTDR_start, &CurOTDR_end))
		{
			offset = -1;
			p = AddPointer(&spt, offset);
			spt.CurPoint = p;
			bytes++;
			if((bytes>=4096)||(p == pTable.RunRecord360h.CurPoint))
			{//��Ҳû�м�¼��
				if(record[number].end.MinuteNb>DriveMinuteLimit)
					number++;
				break;
			}
			continue;
		}
		//�����µ�ָ��ֵ
		offset = 0 - (sizeof(CurOTDR_end)+sizeof(CurOTDR_start)+CurOTDR_end.MinuteNb);
		p = AddPointer(&spt, offset);
		spt.CurPoint = p;
				
		//�Ƚ�˾�������Ƿ����
		if(PartitionTable_BASE->DriverCode!=CurOTDR_end.driver.DriverCode)
			continue;
			
		if(timeflag == 0)
		{//��¼����2������������
			time.year = CurOTDR_end.dt.year;
			time.month = CurOTDR_end.dt.month;
			time.day = CurOTDR_end.dt.day;
			time.hour = CurOTDR_end.dt.hour;
			time.minute = CurOTDR_end.dt.minute;
			time.second = CurOTDR_end.dt.second;
			timeflag = 1;
		}

		
		//�Ƚ�ʱ���Ƿ�����Ҫ���Լ��Ƿ���������������
		if((time.year!=CurOTDR_end.dt.year)||(time.month!=CurOTDR_end.dt.month)||(time.day!=CurOTDR_end.dt.day))
		{//�������ͬһ������
			//�ж��Ƿ���ǰһ�������죿
			if(!IfOneAfterAotherDay(&time, &CurOTDR_end))
			{
				if(record[number].end.MinuteNb>DriveMinuteLimit)
					number++;
				break;
			}
		}
		
		//ͳ��ƣ�ͼ�ʻ
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
			{//�ϲ���¼
				addup = record[number].end.MinuteNb+space+CurOTDR_end.MinuteNb;
				record[number].end.MinuteNb = addup;
				addup = record[number].end.TotalDistance+CurOTDR_end.TotalDistance;
				record[number].end.TotalDistance = addup;
				record[number].start = CurOTDR_start;
			}
			else
			{//ȷ���Ƿ���һ��Ϊƣ�ͼ�ʻ��¼������¼���μ�¼
				if(record[number].end.MinuteNb>DriveMinuteLimit)
					number++;
				record[number].start = CurOTDR_start;
				record[number].end = CurOTDR_end;
			}
		}
		
	
	}while((bytes<4096)&&(p != pTable.RunRecord360h.CurPoint));//��δ����ƣ�ͼ�ʻ��¼

	return (number);
}
//*----------------------------------------------------------------------------
//* Function Name       : ComputeDistance100m
//* Object              : �����������ͳ���ϵ���������������λΪ����
//* Input Parameters    : pulseNb�������������
//* Output Parameters   : ��������0-5804009��
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
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
//* Object              : �Ƚ�����ʱ��Ĵ�С
//* Input Parameters    : dt1����ʱ��1��dt2����ʱ��2
//* Output Parameters   : 1 ���� dt1 > dt2;
//                       -1 ���� dt1 < dt2;
//                        0 ���� dt1 = dt2;
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
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
//* Object              : ����ָ�붨λ
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
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
	//���ҵ�ǰָ��λ���Ƿ���ڱ�־
	for(i=0;i<RecordFlagByte;i++)
	{
		if(*p!=0xff)
			break;
		if((u_int)p==pTable.BaseData.EndAddr)
			p=(u_char *)pTable.BaseData.BaseAddr;
		else
			p++;
	}
	if(i==RecordFlagByte)//��־���ڣ�����ָ�붨λ��
		return;
		
	//���¶�λ�������ݵ�ָ��
	do 
	{
		if((*p == 0xff)&&(((u_int)p&1)==0))
		{//�����ҵ���־
			i=0;
			do
			{
				if((u_int)p == pTable.BaseData.EndAddr)
					p = (u_char *)pTable.BaseData.BaseAddr;
				else
					p++;
				i++;
			}while((*p==0xff)&&(i<RecordFlagByte));
			if(i == RecordFlagByte)//�ҵ�����ָ��λ��
			{
				//�鿴��־֮ǰ�Ƿ��н�����־AEAE
				//1)�ж��Ƿ�����Ѿ�д��aeae��־�ķ������ϵ��¼
				DataPoint = (u_int)p;
				DataPoint -= (RecordFlagByte+2);
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				f1 = *((u_char *)DataPoint);
				f2 = *((u_char *)(DataPoint+1));
				if((f1==0xae)&&(f2==0xae))
				{//�������ϵ��¼����ָ�����
					pTable.BaseData.CurPoint = (u_int)p;
					pTable.BaseData.CurPoint = AddPointer(&(pTable.BaseData),-2);
					update = 1;
					break;
				}
				
				//2)�ж��Ƿ���������ϵ��¼
				DataPoint = (u_int)p;
				DataPoint -= RecordFlagByte*2;
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				f1 = *((u_char *)DataPoint);
				f2 = *((u_char *)(DataPoint+1));
					
				//�����־��ʼλ��
				/////////*******2003.10.06 panhui*********////////
				DataPoint = (u_int)p;
				DataPoint -= RecordFlagByte;
				if(DataPoint<pTable.BaseData.BaseAddr)
					DataPoint = pTable.BaseData.EndAddr - (pTable.BaseData.BaseAddr - DataPoint)+1;
				if((f1==0xae)&&(f2==0xae))//������¼�����ݣ���ָ�����
					pTable.BaseData.CurPoint = DataPoint;
				else
				{//�������ϵ�,δ���ü�д������־AEAE
					//д���ݽ�����־
					data = 0xaeae;
					flash_sst39_write_flash((flash_word *)DATAFLASH_BASE,(flash_word *)DataPoint,data);
					//���·�����
					pTable.BaseData.CurPoint = (u_int)p;
				}
				/////////*******2003.10.06 panhui*********////////
				update = 1;
				break;
			}
		}
		else
		{//δ�ҵ���־�������ƶ�ָ��
			if((u_int)p == pTable.BaseData.EndAddr)
				p = (u_char *)pTable.BaseData.BaseAddr;
			else
				p++;
		}
	}while((u_int)p != DataPoint);
	
	if(update)
	{
		//���¶�λ�ɵ����ݵ�ָ��
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
			//////////�ػʵ�3̨��//////////////////////////////
			LastDT = CurDT;
				
		}while(DataPoint != pTable.DoubtPointData.CurPoint);	
		
		pTable.DoubtPointData.CurPoint = DataPoint;
		
		WritePartitionTable(&pTable);
	}
}