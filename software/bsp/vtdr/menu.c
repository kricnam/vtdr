#include 	"menu.h"
#include 	"lcd.h"
#include 	"lcd_word_model.h"


LCDTCB lcd_tcb;
LCDTCB last_lcd_tcb;
ACTION_TCB act_tcb;

const FONT_MATRIX * content00[] = {
	display_xian,
	display_shi,
	can,
	data_shu,
	NULL
};

const FONT_MATRIX * content01[] = {
	display_xian,
	display_shi,
	record_ji,
	record_lu,
	data_shu,
	data_ju,
	NULL	
};//��ʾ��¼���

const FONT_MATRIX * content02[] = {
	print_da,
	print_yin,
	NULL
};//��ӡ

const FONT_MATRIX * content04[] = {
	data_shu,
	data_ju,
	save_bao,
	save_cun,
	to_dao,
	udisk_u,
	udisk_pan,
	NULL
};//��ݱ��浽����

const FONT_MATRIX * content03[] = {
	other_qi,
	other_ta,
	operate_cao,
	operate_zuo,
	NULL
};//�������

const FONT_MATRIX * content30[] = {
	door_jian,
	door_ce,
	door_kai,
	door_guan,
	door_men,
	NULL
};//��⿪����
const FONT_MATRIX * content31[] = {

	door_kai,
	door_guan,
	door_men,
	can,
	data_shu,
	NULL
};//�����Ų���

const FONT_MATRIX * back[] = {
	back_fan,
	back_hui,
	NULL
};//����

const FONT_MATRIX * content10[] = {
	auto_che,
	auto_pai,
	number_hao,
	code_ma,
	NULL
};//���ƺ���

const FONT_MATRIX * content11[] = {
	driver_jia,
	driver_shi,
	driver_yuan,
	code_dai,
	code_ma,
	NULL
};//��ʻԱ����

const FONT_MATRIX * content12[] = {
	//driver_jia,//myw 2003.7.14��ʻԱ
	//driver_shi,
	//driver_yuan,
	driver_jia,
	driver_shi,
	zheng,
	number_hao,
	code_ma,
	NULL
};//��ʻ֤����

const FONT_MATRIX * content13[] = {
	auto_che,
	auto_liang,
	character_te,
	character_zheng,
	xi,
	data_shu,
	NULL
};//��������ϵ��

const FONT_MATRIX * content14[] = {
	status_zhuang,
	status_tai,
	ji,
	xing,
	NULL
};//״̬����

const FONT_MATRIX * content15[] = {
	product_chan,
	product_pin,
	version_ban,
	version_ben,
	number_hao,
	NULL
};//��Ʒ�汾��

const FONT_MATRIX * content20[] = {
	every_mei,
	minute_fen,
	minute_zhong,
	//average_ping,//myw 2003.7.14ƽ��
	//average_jun,
	auto_che,
	speed_su,
	NULL
};//ÿ���ӳ���

const FONT_MATRIX * content21[] = {
	lian,
	xu,
	driver_jia,
	driver_shi,
	record_ji,
	record_lu,
	NULL
};//�����ʻ��¼

const FONT_MATRIX * content22[] = {
	addup_lei,
	addup_ji,
	all_zong,
	distance_li,
	distance_cheng,
	NULL
};//�ۼ������

const FONT_MATRIX * none[] = {
	none_wu,
	record_ji,
	record_lu,
	NULL
};//�޼�¼

const FONT_MATRIX * being_stat[] = {
	being_zheng,
	being_zai,
	stat_tong,
	stat_ji,
	NULL
};//����ͳ��

const FONT_MATRIX * working_ok[] = {
	work_gong,
	work_zuo,
	being_zheng,
	normal_chang,
	NULL
};


const MENU_NODE list0[6]={
	{
		(FONT_MATRIX **)content00,
		1,
		-1,
		-1,
		NULL
	},
	{
		(FONT_MATRIX **)content01,
		2,
		-1,
		-1,
		NULL
	},
	{
		(FONT_MATRIX **)content02,
		-1,
		-1,
		-1,
		PrintAllData
	},
	{
		(FONT_MATRIX **)content04,
		-1,
		-1,
		-1,
		SaveDatatoUdisk
	},
	{
		(FONT_MATRIX **)content03,
		3,
		-1,
		-1,
		NULL
	},
	{
		(FONT_MATRIX **)back,
		-1,
		-1,
		-1,
		NULL
	}
};
const MENU_NODE list1[7]={
	{
		(FONT_MATRIX **)content10,
		-1,
		0,
		0,
		DisplayAutoCode
	},
	{
		(FONT_MATRIX **)content11,
		-1,
		0,
		0,
		DisplayDriverNumber
	},
	{
		(FONT_MATRIX **)content12,
		-1,
		0,
		0,
		DisplayDriverCode
	},
	{
		(FONT_MATRIX **)content13,
		-1,
		0,
		0,
		Displaywheel
	},
	{
		(FONT_MATRIX **)content14,
		-1,
		0,
		0,
		DisplayStatusPolarity
	},
	{
		(FONT_MATRIX **)content15,
		-1,
		0,
		0,
		DisplayProductVersion
	},
	{
		(FONT_MATRIX **)back,
		-1,
		0,
		0,
		NULL
	}

};
const MENU_NODE list2[4]={
	{
		(FONT_MATRIX **)content20,
		-1,
		0,
		1,
		Display15MinAverageSpeed
	},
	{
		(FONT_MATRIX **)content21,
		-1,
		0,
		1,
		Display2DayOTDR
	},
	{
		(FONT_MATRIX **)content22,
		-1,
		0,
		1,
		DisplayTotalDistance
	},
	{
		(FONT_MATRIX **)back,
		-1,
		0,
		1,
		NULL
	}
};
#if OpenDoorDeal
const MENU_NODE list3[3]={
	{
		(FONT_MATRIX **)content30,
		-1,
		0,
		3,
		JudgeDoorType
	},
	{
		(FONT_MATRIX **)content31,
		-1,
		0,
		3,
		DoorType
	},
	{
		(FONT_MATRIX **)back,
		-1,
		0,
		3,
		NULL
	}
};
#else
const MENU_NODE list3[3]={
	{
		(FONT_MATRIX **)content30,
		-1,
		0,
		3,
		NULL
	},
	{
		(FONT_MATRIX **)content31,
		-1,
		0,
		3,
		NULL
	},
	{
		(FONT_MATRIX **)back,
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
//* Object              : ������ݵ����̣�������ɺ󷵻ص�ͨ������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
//*----------------------------------------------------------------------------
void SaveDatatoUdisk()
{
   usb_host_init();
   Udisk_handler();

}
//*----------------------------------------------------------------------------
//* Function Name       : PrintAllData
//* Object              : ��ӡ������ݣ�������ɺ󷵻ص�ͨ������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
//*----------------------------------------------------------------------------
void PrintAllData()
{
	lcd_clear(line2);
	lcm_write_hz1(1,1,(FONT_MATRIX *)being_zheng);
	lcm_write_hz1(1,2,(FONT_MATRIX *)being_zai);
	lcm_write_hz1(1,3,(FONT_MATRIX *)print_da);
	lcm_write_hz1(1,4,(FONT_MATRIX *)print_yin);

	#if GetSpeedStatusBy232
	PIO_SODR = SPEED;//ʹ�ܴ�ӡ���ͨѶ��
	#endif
	
	Printer();//��ӡ
	
	#if GetSpeedStatusBy232
	PIO_CODR = SPEED;//��ֹ��ӡ���ͨѶ��
	at91_usart_open(RS2320,US_ASYNC_MODE,BAUDS4800,0);
	#endif
	
	DisplayNormalUI();
}
//*----------------------------------------------------------------------------
//* Function Name       : WriteDataToUDiskMenu
//* Object              : ������ݵ�U�̣�������ɺ󷵻ص�ͨ������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
//*----------------------------------------------------------------------------
void WriteDataToUDiskMenu()
{
	//at91_pio_write ( &PIO_DESC, (1<<8), PIO_CLEAR_OUT );
	//delayms(10);
	//at91_pio_write ( &PIO_DESC, (1<<8), PIO_SET_OUT );//��λSL811
	//delayms(10);

//	sl811write(0x0f,0x80);
	int i;
	//�ȴ�
	lcd_clear(0);
	lcm_write_hz1(1,1,(FONT_MATRIX *)scan_sao);
	lcm_write_hz1(1,2,(FONT_MATRIX *)scan_miao);
	lcm_write_hz1(1,3,(FONT_MATRIX *)udisk_u);
	lcm_write_hz1(1,4,(FONT_MATRIX *)udisk_pan);

	if(Scan_UDisk())		
	{	
		lcd_clear(line2);
		lcm_write_hz1(1,1,(FONT_MATRIX *)being_zheng);
		lcm_write_hz1(1,2,(FONT_MATRIX *)being_zai);
		lcm_write_hz1(1,3,(FONT_MATRIX *)save_bao);
		lcm_write_hz1(1,4,(FONT_MATRIX *)save_cun);
		/////////////
		//���ж�
//		OS_ENTER_CRITICAL();
	
		if(!WriteDataToUDisk())
		{
			//ʧ��
			lcd_clear(line2);
			lcm_write_hz1(1,1,(FONT_MATRIX *)operate_cao);
			lcm_write_hz1(1,2,(FONT_MATRIX *)operate_zuo);
			lcm_write_hz1(1,3,(FONT_MATRIX *)fail_shi);
			lcm_write_hz1(1,4,(FONT_MATRIX *)fail_bai);
			for(i=0;i<100000;i++);
		}
				
		//���ж�
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
//* Object              : ��ʾ���ƺ���
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : act_tcb
//*----------------------------------------------------------------------------
void DisplayAutoCode()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcd_clear(line2);

	StructPara *para = PARAMETER_BASE;
	unsigned char j=0,col=0,type=0;//type��0���֣���1���֣�2��ĸ
	unsigned char buf=para->AutoCode[0];
	unsigned short hz;
	while((buf!='\0')&&(col<20)&&(j<12))
	{
		if(buf>127)
		{//���ִ���
			hz = buf;
			hz = hz<<8;
			j++;
			buf=para->AutoCode[j];
			hz = hz+buf;
			if((col&1)==1)//���������
				col++;
			lcm_write_hz1(1,col/2,AutoCodeHZ2LCM(hz));
			col+=2;
			type = 0;
		}
		else
		{//��ĸ������
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
//* Object              : ��ʾ˾����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : act_tcb
//*----------------------------------------------------------------------------
void DisplayDriverNumber()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcd_clear(line2);

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
//* Object              : ��ʾ˾���ʻ֤����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : act_tcb
//*----------------------------------------------------------------------------
void DisplayDriverCode()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcd_clear(line2);

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
//* Object              : ��ʾ����ϵ��
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : act_tcb
//*----------------------------------------------------------------------------
void Displaywheel()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcd_clear(line2);
	
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
//* Object              : ��ʾ״̬����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : act_tcb
//*----------------------------------------------------------------------------
void DisplayStatusPolarity()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcd_clear(line2);
	
	StructPara *para = PARAMETER_BASE;
	unsigned short s=para->status_polarity;
	unsigned short z;
	int j;
	for(j=15;j>=0;j--)
	{
		z=1<<j;
		if((s & z)==0)
			lcm_write_ez(1,19-j,(FONT_MATRIX *)digital_0);
		else
			lcm_write_ez(1,19-j,(FONT_MATRIX *)digital_1);
	}
	
	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : DisplayProductVersion
//* Object              : ��ʾ��Ʒ�汾��
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : act_tcb
//*----------------------------------------------------------------------------
void DisplayProductVersion()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcd_clear(line2);
	
	u_char col=0;
	lcm_write_ez(1,col,(FONT_MATRIX *)digital_0);
	lcm_write_ez(1,col+1,(FONT_MATRIX *)digital_6);
	lcm_write_ez(1,col+2,(FONT_MATRIX *)charater_point);
	lcm_write_ez(1,col+3,(FONT_MATRIX *)digital_0);
	lcm_write_ez(1,col+4,(FONT_MATRIX *)digital_8);
	lcm_write_ez(1,col+5,(FONT_MATRIX *)charater_point);
	lcm_write_ez(1,col+6,(FONT_MATRIX *)digital_2);
	lcm_write_ez(1,col+7,(FONT_MATRIX *)digital_8);
	lcm_write_ez(1,col+8,(FONT_MATRIX *)charater_point);
	lcm_write_ez(1,col+9,(FONT_MATRIX *)digital_1);
#if	guizhou
	lcm_write_ez(1,col+10,(FONT_MATRIX *)charater_xing);
#else
	lcm_write_ez(1,col+10,(FONT_MATRIX *)charater_point);
#endif
	
#if OpenDoorDeal//����������Ŵ���
	lcm_write_ez(1,col+11,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+11,(FONT_MATRIX *)digital_0);
#endif
#if RTC8025//ʱ��оƬѡ�񿪹�
	lcm_write_ez(1,col+12,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+12,(FONT_MATRIX *)digital_0);
#endif
#if GetSpeedStatusBy232     //ͨ��ں�������ͨѶ��ȡ�ٶȺ�ȫ��״̬����
	lcm_write_ez(1,col+13,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+13,(FONT_MATRIX *)digital_0);
#endif

#if WATCH_DOG_EN          // ���Ź�����
	lcm_write_ez(1,col+14,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+14,(FONT_MATRIX *)digital_0);
#endif
#if RPM_EN			     //������ת�ٿ���
	lcm_write_ez(1,col+15,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+15,(FONT_MATRIX *)digital_0);
#endif
#if SectionAlarm_EN        //��·�α�������
	lcm_write_ez(1,col+16,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+16,(FONT_MATRIX *)digital_0);
#endif
#if OpenDoorAlarm	     //������ʻ��������
	lcm_write_ez(1,col+17,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+17,(FONT_MATRIX *)digital_0);
#endif
#if Test        	  //���Թ�װ
	lcm_write_ez(1,col+18,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+18,(FONT_MATRIX *)digital_0);
#endif
#if Status14    //����8��״̬
	lcm_write_ez(1,col+19,(FONT_MATRIX *)digital_1);
#else
	lcm_write_ez(1,col+19,(FONT_MATRIX *)digital_0);
#endif
	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : Get15MinuteSpeed
//* Object              : 15�����ڵ�ʱ����ٶ�
//* Input Parameters    : 
//* Output Parameters   : ͣ��ʱ�� ʱ,��,�ٶ�(3*15)
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
	
	//�ó�ֵ
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
		//ȡ����ǰ��¼�ǲ���ȷ��
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
			
		//���㵱ǰ��¼����һ����¼֮���ʱ���	
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
		//�޸�ָ��
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
//* Object              : ��ʾͣ��ʱ����ǰ��15����ƽ���ٶ�
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void Display15MinAverageSpeed()
{//��¼ѭ����ʾ
	u_char i;
	LCD_ZM ** p;
	PrintSpeed *pt;
	pt=(PrintSpeed *)OTDR_Array;
	act_tcb.type = SHOW;
//	act_tcb.LineNumber = 15;
//myw 2003.7.14
		lcm_write_zimu(0,9,(FONT_MATRIX *)Letter_k);
		lcm_write_zimu(0,10,(FONT_MATRIX *)Letter_m);
		lcm_write_ez(0,13,(FONT_MATRIX *)charater_slash);
		lcm_write_zimu(0,12,(FONT_MATRIX *)Letter_h);
		
//
	if(act_tcb.CurLine == 0)//ȡ��¼
	{
		//��ʾ������ͳ�ơ�
		lcd_clear(line2);
		int i=0;
		p = (FONT_MATRIX **)being_stat;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
		act_tcb.LineNumber = Get15MinAverageSpeed(pt);
	}
	
	if(act_tcb.LineNumber == 0)
	{//��ʾ���޼�¼��
		lcd_clear(line2);
		int i=0;
		p = (FONT_MATRIX **)none;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
	}
	else{	 

		act_tcb.CurLine ++;
		if(act_tcb.CurLine>15)
			act_tcb.CurLine = 1;
			
		lcd_clear(line2);
		u_char col=0;
		u_char row=1;
		//ʱ�䣺ʱ
		lcm_write_ez(row,col,BCD2LCM(pt[act_tcb.CurLine-1].hour,1));
		lcm_write_ez(row,col+1,BCD2LCM(pt[act_tcb.CurLine-1].hour,0));
		lcm_write_ez(row,col+2,(LCD_ZM *)digital_);
		//��
		lcm_write_ez(row,col+3,BCD2LCM(pt[act_tcb.CurLine-1].minute,1));
		lcm_write_ez(row,col+4,BCD2LCM(pt[act_tcb.CurLine-1].minute,0));
		//�ٶ�
		DisplayInteger(pt[act_tcb.CurLine-1].speed,row,col+8,3);
		
		/*lcm_write_zimu(row,8,(LCD_ZM *)Letter_k);//myw 2003.7.14
		lcm_write_zimu(row,9,(LCD_ZM *)Letter_m);
		lcm_write_ez(row,12,(LCD_ZM *)charater_slash);
		lcm_write_zimu(row,11,(LCD_ZM *)Letter_h);
		*/
//˾���� 
		DisplayInteger(pt[act_tcb.CurLine-1].DriverCode,row,19,0);
	}
	

}
//*----------------------------------------------------------------------------
//* Function Name       : Display2DayOTDR
//* Object              : ��ʾ2�������쳬��3Сʱ������ʻ��¼
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void Display2DayOTDR()
{//��¼ѭ����ʾ
	FONT_MATRIX ** p;
	act_tcb.type = SHOW;
	if(act_tcb.CurLine == 0)//ȡ��¼
	{
		//��ʾ������ͳ�ơ�
		lcm_clear_ram(LINE2);
		int i=0;
		p = (FONT_MATRIX **)being_stat;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
		act_tcb.LineNumber = GetOverTimeRecordIn2Days(OTDR_Array);
	}

	if(act_tcb.LineNumber == 0)
	{//��ʾ���޼�¼��
		lcm_clear_ram(LINE2);
		int i=0;
		p = (FONT_MATRIX **)none;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);
	}
	else{	 
		act_tcb.CurLine ++;
		if(act_tcb.CurLine>act_tcb.LineNumber)
			act_tcb.CurLine = 1;
		
		lcd_clear(lineall);
		lcm_write_ez(0,0,BCD2LCM(Char2BCD(act_tcb.CurLine),1));
		lcm_write_ez(0,1,BCD2LCM(Char2BCD(act_tcb.CurLine),0));
		lcm_write_ez(0,2,(LCD_ZM *)digital_);
		u_char col=4;
		u_char row=0;
		//��ʼʱ�䣺��
		lcm_write_ez(row,col,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.year,1));
		lcm_write_ez(row,col+1,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.year,0));
		//��
		lcm_write_ez(row,col+3,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.month,1));
		lcm_write_ez(row,col+4,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.month,0));
		//��
		lcm_write_ez(row,col+6,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.day,1));
		lcm_write_ez(row,col+7,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.day,0));
		//ʱ
		lcm_write_ez(row,col+9,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.hour,1));
		lcm_write_ez(row,col+10,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.hour,0));
		lcm_write_ez(row,col+11,(LCD_ZM *)digital_);
		//��
		lcm_write_ez(row,col+12,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.minute,1));
		lcm_write_ez(row,col+13,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].start.dt.minute,0));

		row = 1;
		//����ʱ�䣺��
		lcm_write_ez(row,col,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.year,1));
		lcm_write_ez(row,col+1,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.year,0));
		//��
		lcm_write_ez(row,col+3,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.month,1));
		lcm_write_ez(row,col+4,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.month,0));
		//��
		lcm_write_ez(row,col+6,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.day,1));
		lcm_write_ez(row,col+7,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.day,0));
		//ʱ
		lcm_write_ez(row,col+9,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.hour,1));
		lcm_write_ez(row,col+10,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.hour,0));
		lcm_write_ez(row,col+11,(LCD_ZM *)digital_);
		//��
		lcm_write_ez(row,col+12,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.minute,1));
		lcm_write_ez(row,col+13,BCD2LCM(OTDR_Array[act_tcb.CurLine-1].end.dt.minute,0));
			
		
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayTotalDistance
//* Object              : ��ʾ�����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void DisplayTotalDistance()
{
	act_tcb.type = SHOW;
	act_tcb.LineNumber = 1;
	if(act_tcb.CurLine == act_tcb.LineNumber)
		return;
	
	lcd_clear(line2);
	
	//�ۼ������
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
	lcm_write_ez(1,m,(FONT_MATRIX *)charater_point);
	m--;
	u_char k;
	for(k=1;k<=i;k++){
		lcm_write_ez(1,m,BCD2LCM(buf[k],0));
		m--;
	}
	
	lcm_write_hz1(1,8,(FONT_MATRIX *)km_gong);
	lcm_write_hz1(1,9,(FONT_MATRIX *)distance_li);

	act_tcb.IfActionEnd = 1;
	act_tcb.CurLine = 1;
}
//*----------------------------------------------------------------------------
//* Function Name       : SelectKeyHandler
//* Object              : ��ʾ��ǰ�˵����
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void DisplayCurrentNode()
{
	FONT_MATRIX ** p1;
	FONT_MATRIX ** p2;

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
	
	//��ʾָʾ��־
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
//* Object              : ��ѡ�񡱼������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
		case Node://��ǰ����޸�Ϊͬ����һ�����
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
//* Object              : ��ȷ�ϡ��������
//* Input Parameters    : none
//* Output Parameters   : none
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void OKKeyHandler()
{
	short TemplistNb;

	lcd_tcb.KeepTime = 0;
	switch(lcd_tcb.mode)
	{
		case Node://�л�����һ�����ӽ��
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
		case BackLeaf://�л���������ͨ��ģʽ
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
		case ActLeaf://�л�������ģʽ
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
//* Object              : ASCII��ת��Ϊ��ģָ��
//* Input Parameters    : data������ת������ĸ
//* Output Parameters   : ��ģָ��
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
FONT_MATRIX *ASCII2LCM(unsigned char data)
{
	FONT_MATRIX *ret;
	
	switch(data)
	{
		case 48:
			ret=(FONT_MATRIX *)digital_0;break;
		case 49:
			ret=(FONT_MATRIX *)digital_1;break;
		case 50:
			ret=(FONT_MATRIX *)digital_2;break;
		case 51:
			ret=(FONT_MATRIX *)digital_3;break;
		case 52:
			ret=(FONT_MATRIX *)digital_4;break;
		case 53:
			ret=(FONT_MATRIX *)digital_5;break;
		case 54:
			ret=(FONT_MATRIX *)digital_6;break;
		case 55:
			ret=(FONT_MATRIX *)digital_7;break;
		case 56:
			ret=(FONT_MATRIX *)digital_8;break;
		case 57:
			ret=(FONT_MATRIX *)digital_9;break;
		
		case 65:
			ret=(FONT_MATRIX *)Letter_A;break;
		case 66:
			ret=(FONT_MATRIX *)Letter_B;break;
		case 67:
			ret=(FONT_MATRIX *)Letter_C;break;
		case 68:
			ret=(FONT_MATRIX *)Letter_D;break;
		case 69:
			ret=(FONT_MATRIX *)Letter_E;break;
		case 70:
			ret=(FONT_MATRIX *)Letter_F;break;
		case 71:
			ret=(FONT_MATRIX *)Letter_G;break;
		case 72:
			ret=(FONT_MATRIX *)Letter_H;break;
		case 73:
			ret=(FONT_MATRIX *)Letter_I;break;
		case 74:
			ret=(FONT_MATRIX *)Letter_J;break;
		case 75:
			ret=(FONT_MATRIX *)Letter_K;break;
		case 76:
			ret=(FONT_MATRIX *)Letter_L;break;
		case 77:
			ret=(FONT_MATRIX *)Letter_M;break;
		case 78:
			ret=(FONT_MATRIX *)Letter_N;break;
		case 79:
			ret=(FONT_MATRIX *)Letter_O;break;
		case 80:
			ret=(FONT_MATRIX *)Letter_P;break;
		case 81:
			ret=(FONT_MATRIX *)Letter_Q;break;
		case 82:
			ret=(FONT_MATRIX *)Letter_R;break;
		case 83:
			ret=(FONT_MATRIX *)Letter_S;break;
		case 84:
			ret=(FONT_MATRIX *)Letter_T;break;
		case 85:
			ret=(FONT_MATRIX *)Letter_U;break;
		case 86:
			ret=(FONT_MATRIX *)Letter_V;break;
		case 87:
			ret=(FONT_MATRIX *)Letter_W;break;
		case 88:
			ret=(FONT_MATRIX *)Letter_X;break;
		case 89:
			ret=(FONT_MATRIX *)Letter_Y;break;
		case 90:
			ret=(FONT_MATRIX *)Letter_Z;break;
			
		case 97:
			ret=(FONT_MATRIX *)Letter_a;break;
		case 98:
			ret=(FONT_MATRIX *)Letter_b;break;
		case 99:
			ret=(FONT_MATRIX *)Letter_c;break;
		case 100:
			ret=(FONT_MATRIX *)Letter_d;break;
		case 101:
			ret=(FONT_MATRIX *)Letter_e;break;
		case 102:
			ret=(FONT_MATRIX *)Letter_f;break;
		case 103:
			ret=(FONT_MATRIX *)Letter_g;break;
		case 104:
			ret=(FONT_MATRIX *)Letter_h;break;
		case 105:
			ret=(FONT_MATRIX *)Letter_i;break;
		case 106:
			ret=(FONT_MATRIX *)Letter_j;break;
		case 107:
			ret=(FONT_MATRIX *)Letter_k;break;
		case 108:
			ret=(FONT_MATRIX *)Letter_l;break;
		case 109:
			ret=(FONT_MATRIX *)Letter_m;break;
		case 110:
			ret=(FONT_MATRIX *)Letter_n;break;
		case 111:
			ret=(FONT_MATRIX *)Letter_o;break;
		case 112:
			ret=(FONT_MATRIX *)Letter_p;break;
		case 113:
			ret=(FONT_MATRIX *)Letter_q;break;
		case 114:
			ret=(FONT_MATRIX *)Letter_r;break;
		case 115:
			ret=(FONT_MATRIX *)Letter_s;break;
		case 116:
			ret=(FONT_MATRIX *)Letter_t;break;
		case 117:
			ret=(FONT_MATRIX *)Letter_u;break;
		case 118:
			ret=(FONT_MATRIX *)Letter_v;break;
		case 119:
			ret=(FONT_MATRIX *)Letter_w;break;
		case 120:
			ret=(FONT_MATRIX *)Letter_x;break;
		case 121:
			ret=(FONT_MATRIX *)Letter_y;break;
		case 122:
			ret=(FONT_MATRIX *)Letter_z;break;
		default:
			ret=(FONT_MATRIX *)space;break;
	}
	return ret;
}
//*----------------------------------------------------------------------------
//* Function Name       : AutoCodeHZ2LCM
//* Object              : ���ƺ��еĺ���ת��Ϊ��ģָ��
//* Input Parameters    : data������ת���ĺ���
//* Output Parameters   : ��ģָ��
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
FONT_MATRIX *AutoCodeHZ2LCM(unsigned short data)
{
	FONT_MATRIX *ret;


	switch(data)
	{
		case 0xbea9:
			ret = (FONT_MATRIX *)ch_jing1;//{"��"}
			break; 
		case 0xbba6:
			ret = (FONT_MATRIX *)ch_hu;//{"��"}
			break; 
		case 0xbdf2:
			ret = (FONT_MATRIX *)ch_jin1;//{"��"}
			break; 
		case 0xcbd5:
			ret = (FONT_MATRIX *)ch_su;//{"��"}
			break; 
		case 0xcdee:
			ret = (FONT_MATRIX *)ch_wan;//{"��"}
			break; 
		case 0xb8d3:
			ret = (FONT_MATRIX *)ch_gan4;//{"��"}
			break; 
		case 0xc3f6:
			ret = (FONT_MATRIX *)ch_min;//{"��"}
			break; 
		case 0xc2b3:
			ret = (FONT_MATRIX *)ch_lu;//{"³"}
			break; 
		case 0xd5e3:
			ret = (FONT_MATRIX *)ch_zhe;//{"��"}
			break; 
		case 0xbdfa:
			ret = (FONT_MATRIX *)ch_jin4;//{"��"}
			break; 
		case 0xbcbd:
			ret = (FONT_MATRIX *)ch_ji4;//{"��"}
			break; 
		case 0xd4a5:
			ret = (FONT_MATRIX *)ch_yu4;//{"ԥ"}
			break; 
		case 0xc3c9:
			ret = (FONT_MATRIX *)ch_meng;//{"��"}
			break; 
		case 0xd0c2:
			ret = (FONT_MATRIX *)ch_xin;//{"��"}
			break; 
		case 0xc4fe:
			ret = (FONT_MATRIX *)ch_ning;//{"��"}
			break; 
		case 0xc1c9:
			ret = (FONT_MATRIX *)ch_liao;//{"��"}
			break; 
		case 0xbada:
			ret = (FONT_MATRIX *)ch_hei;//{"��"}
			break; 
		case 0xbcaa:
			ret = (FONT_MATRIX *)ch_ji2;//{"��"}
			break; 
		case 0xcfe6:
			ret = (FONT_MATRIX *)ch_xiang;//{"��"}
			break; 
		case 0xb6f5:
			ret = (FONT_MATRIX *)ch_e;//{"��"}
			break; 
		case 0xb9f0:
			ret = (FONT_MATRIX *)ch_gui;//{"��"}
			break; 
		case 0xd4c1:
			ret = (FONT_MATRIX *)ch_yue;//{"��"}
			break; 
		case 0xc7ed:
			ret = (FONT_MATRIX *)ch_qiong;//{"��"}
			break; 
		case 0xb2d8:
			ret = (FONT_MATRIX *)ch_zang;//{"��"}
			break; 
		case 0xc9c2:
			ret = (FONT_MATRIX *)ch_shan;//{"��"}
			break; 
		case 0xb8ca:
			ret = (FONT_MATRIX *)ch_gan1;//{"��"}
			break; 
		case 0xc7e0:
			ret = (FONT_MATRIX *)ch_qing;//{"��"}
			break; 
		case 0xb4a8:
			ret = (FONT_MATRIX *)ch_chuan;//{"��"}
			break; 
		case 0xc7ad:
			ret = (FONT_MATRIX *)ch_qian;//{"ǭ"}
			break; 
		case 0xd4c6:
			ret = (FONT_MATRIX *)ch_yun;//{"��"}
			break; 
		case 0xbaa3:
			ret = (FONT_MATRIX *)ch_hai3;//{"��"}
			break; 
		case 0xcca8:
			ret = (FONT_MATRIX *)ch_tai;//{"̨"}
			break; 
		case 0xd3e5:
			ret = (FONT_MATRIX *)ch_yu2;//{"��"}
			break; 
		case 0xb8db:
			ret = (FONT_MATRIX *)ch_gang;//{"��"}
			break; 
		case 0xb0c4:
			ret = (FONT_MATRIX *)ch_ao;//{"��"}
			break; 
		case 0xcab9:
			ret = (FONT_MATRIX *)ch_shi;//{"ʹ"}
			break; 
		case 0xbcd7:
			ret = (FONT_MATRIX *)ch_jia;//{"��"}
			break; 
		case 0xd2d2:
			ret = (FONT_MATRIX *)ch_yi;//{"��"}
			break; 
		case 0xb1fb:
			ret = (FONT_MATRIX *)ch_bing;//{"��"}
			break; 
		case 0xb6a1:
			ret = (FONT_MATRIX *)ch_ding;//{"��"}
			break; 
		case 0xceec:
			ret = (FONT_MATRIX *)ch_wu4;//{"��"}
			break; 
		case 0xbcba:
			ret = (FONT_MATRIX *)ch_ji3;//{"��"}
			break; 
		case 0xb8fd:
			ret = (FONT_MATRIX *)ch_geng;//{"��"}
			break; 
		case 0xd0c1:
			ret = (FONT_MATRIX *)ch_xin1;//{"��"}
			break; 
		case 0xd7d3:
			ret = (FONT_MATRIX *)ch_zi;//{"��"}
			break; 
		case 0xb3f3:
			ret = (FONT_MATRIX *)ch_chou;//{"��"}
			break; 
		case 0xd2fa:
			ret = (FONT_MATRIX *)ch_yin;//{"��"}
			break; 
		case 0xc3ae:
			ret = (FONT_MATRIX *)ch_mou;//{"î"}
			break; 
		case 0xb3bd:
			ret = (FONT_MATRIX *)ch_chen;//{"��"}
			break; 
		case 0xcee7:
			ret = (FONT_MATRIX *)ch_wu3;//{"��"}
			break; 
		case 0xceb4:
			ret = (FONT_MATRIX *)ch_wei;//{"δ"}
			break; 
		case 0xc9ea:
			ret = (FONT_MATRIX *)ch_shen;//{"��"}
			break; 
		case 0xd3cf:
			ret = (FONT_MATRIX *)ch_you;//{"��"}
			break; 
		case 0xbaa5:
			ret = (FONT_MATRIX *)ch_hai4;//{"��"}
			break; 
		case 0xc8c9:
			ret = (FONT_MATRIX *)ch_ren;//{"��"}
			break; 
		case 0xbeaf:
			ret = (FONT_MATRIX *)ch_jing3;//{"��"}
			break; 
		case 0xb9f3:
			ret = (FONT_MATRIX *)ch_gui4;//{"��"}
			break;
		case 0xc1ec:
			ret = (FONT_MATRIX *)ch_ling;//{"��"}
			break;
		case 0xd1a7:
			ret = (FONT_MATRIX *)ch_xue;//{"ѧ"}
			break;
		case 0xcad4:
			ret = (FONT_MATRIX *)ch_shi_try;//{"��"}
			break;
		case 0xbeb3:
			ret = (FONT_MATRIX *)ch_jing;//{"��"}
			break;
		default:
			ret = (FONT_MATRIX *)space;
			break;
	}
	return ret;
}
//*----------------------------------------------------------------------------
//* Function Name       : BCD2LCM
//* Object              : BCD��ʽ�����ת��Ϊ��ģָ��
//* Input Parameters    : data������ת����BCD��ʽ���
//*                       type����0������λ��1������λ
//* Output Parameters   : ��ģָ��
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
FONT_MATRIX *BCD2LCM(u_char data, u_char type)
{
	u_char temp;
	FONT_MATRIX *ret;
	if(type)
	{//����λ
		temp = (data & 0xf0) >> 4;
	}
	else
		temp = data & 0x0f;
	
	switch(temp)
	{
		case 0:
			ret = (FONT_MATRIX *)digital_0;break;
		case 1:
			ret = (FONT_MATRIX *)digital_1;break;
		case 2:
			ret = (FONT_MATRIX *)digital_2;break;
		case 3:
			ret = (FONT_MATRIX *)digital_3;break;
		case 4:
			ret = (FONT_MATRIX *)digital_4;break;
		case 5:
			ret = (FONT_MATRIX *)digital_5;break;
		case 6:
			ret = (FONT_MATRIX *)digital_6;break;
		case 7:
			ret = (FONT_MATRIX *)digital_7;break;
		case 8:
			ret = (FONT_MATRIX *)digital_8;break;
		case 9:
			ret = (FONT_MATRIX *)digital_9;break;
		default:
			ret = 0;break;
	}
	return ret;
} 
//*----------------------------------------------------------------------------
//* Function Name       : DisplayInteger
//* Object              : ��ʾ����
//* Input Parameters    : row��������
//*                       end_column�������һλ������
//*                       Integer��������ʾ������
//*                       len������ʾ��ݵ������
//* Output Parameters   : none
//* Functions called    : 
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
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
		lcm_write_ez(row,m,(FONT_MATRIX *)space);
		m--;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayFloat
//* Object              : ��ʾһλС��ĸ�����
//* Input Parameters    : row��������
//*                       end_column�������һλ������
//*                       Float��������ʾ������
//*                       len������ʾ��ݵ������
//* Output Parameters   : none
//* Functions called    : 
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
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
	lcm_write_ez(row,m,(FONT_MATRIX *)charater_point);
	m--;
	for(k=1;k<=i;k++){
		lcm_write_ez(row,m,BCD2LCM(buf[k],0));
		m--;
	}
	for(k=i+1;k<len;k++){
		lcm_write_ez(row,m,(FONT_MATRIX *)space);
		m--;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayDateTime
//* Object              : ��ʾ����ʱ��
//* Input Parameters    : row��������
//*                       end_column�������һλ������
//*                       flag���������ʾ��־�Ӹ�λ��ʼ
//*                       ���Σ����ʱ�����꣩��1��ʾ��ʾ0��ʾ����ʾ
//* Output Parameters   : none
//* Functions called    : 
//* ���õ�ȫ�ֱ���      : none
//* �޸ĵ�ȫ�ֱ���      : none
//*----------------------------------------------------------------------------
void DisplayDateTime(u_char flag,u_char row,u_char column)
{
	u_char m=column;
	if(ClockVL)
		lcm_write_ez(row,column-1,(FONT_MATRIX *)charater_xing);
	else
		lcm_write_ez(row,column-1,(FONT_MATRIX *)space);

	if((flag & 0x20)!=0)
	{	//��
		lcm_write_ez(row,m,BCD2LCM(curTime.year,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.year,0));m++;
		lcm_write_ez(row,m,(FONT_MATRIX *)space);m++;
	}
	if((flag & 0x10)!=0)
	{	//��
		lcm_write_ez(row,m,BCD2LCM(curTime.month,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.month,0));m++;
		lcm_write_ez(row,m,(FONT_MATRIX *)space);m++;
	}
	if((flag & 0x08)!=0)
	{	//��
		lcm_write_ez(row,m,BCD2LCM(curTime.day,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.day,0));m++;
		lcm_write_ez(row,m,(FONT_MATRIX *)space);m++;
	}
	if((flag & 0x04)!=0)
	{	//ʱ
		lcm_write_ez(row,m,BCD2LCM(curTime.hour,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.hour,0));m++;
	}
	if((flag & 0x02)!=0)
	{	//��
		lcm_write_ez(row,m,(LCD_ZM *)digital_);m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.minute,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.minute,0));m++;
	}
	if((flag & 0x01)!=0)
	{	//��
		lcm_write_ez(row,m,(LCD_ZM *)digital_);m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.second,1));m++;
		lcm_write_ez(row,m,BCD2LCM(curTime.second,0));m++;
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplaySpeedDimensoin
//* Object              : ��ʾ���ý��棺ʱ����ٶ�
//* Input Parameters    : row������
//*                       column������
//*                       hz_pt������д������ģ����ָ��
//* Output Parameters   : none
//* Functions called    : lcm_write_command��lcm_control��lcm_write_data
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void DisplaySpeedDimensoin()
{
	lcm_write_ez(1,9,(FONT_MATRIX *)charater_slash);
	lcm_write_zimu(1,6,(FONT_MATRIX *)Letter_k);
	lcm_write_zimu(1,7,(FONT_MATRIX *)Letter_m);
//	lcm_write_zimu(1,8,(FONT_MATRIX *)Letter_h);
	write_h();
}
//*----------------------------------------------------------------------------
//* Function Name       : DisplayNormalUI
//* Object              :
//* Input Parameters    :
//*
//*
//* Output Parameters   : none
//* Functions called    :
//*      :
//*      :
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
			lcm_write_hz1(1,4,(FONT_MATRIX *)comm_tong);
			lcm_write_hz1(1,5,(FONT_MATRIX *)comm_xun);
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
		lcm_write_ez(0,10,(FONT_MATRIX *)ch_sub);
		lcm_write_ez(0,9,(FONT_MATRIX *)ch_bracket);
	}
	else
	{
		lcm_write_ez(0,10,(FONT_MATRIX *)space);
		lcm_write_ez(0, 9,(FONT_MATRIX *)space);
	}
	if(pTable.InOSAlarmCycle)
		DisplayInteger(RoadNb,0,8,0);
	else
		lcm_write_ez(0,8,(FONT_MATRIX *)space);
	
	//�ٶ�
//	DisplayInteger(CurSpeed,1,2,3);
	DisplaySpeed((u_char)CurSpeed);
	DisplaySpeedDimensoin();
//	DisplayInteger(ClockType,0,8,3);

/*	//״̬
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
	{//��ʾ�������ʻ��	
		lcm_write_hz1(1,6,(FONT_MATRIX *)lian);
		lcm_write_hz1(1,7,(FONT_MATRIX *)xu);
		lcm_write_hz1(1,8,(FONT_MATRIX *)driver_jia);
		lcm_write_hz1(1,9,(FONT_MATRIX *)driver_shi);
	}
	else
	{
		//���
/*		DisplayInteger(LastPN[3],1,18,2);
		DisplayInteger(LastPN[2],1,16,2);
		DisplayInteger(LastPN[1],1,14,2);
		DisplayInteger(LastPN[0],1,12,2);*/
		DisplayFloat(Distance,1,16,5);
		lcm_write_zimu(1,15,(FONT_MATRIX *)Letter_k);
		lcm_write_zimu(1,16,(FONT_MATRIX *)Letter_m);
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
//* Object              : ��ʾ"���ڸ���"
//* Input Parameters    : 
//* Output Parameters   : none
//* Functions called    : 
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
//*----------------------------------------------------------------------------
void DisplayEraseDataFlash()
{
	lcd_clear(lineall);
	lcm_write_hz1(1,1,(FONT_MATRIX *)being_zheng);
	lcm_write_hz1(1,2,(FONT_MATRIX *)being_zai);
	lcm_write_hz1(1,3,(FONT_MATRIX *)update_geng);
	lcm_write_hz1(1,4,(FONT_MATRIX *)update_xin);
	lcd_tcb.mode = Normal;
}
void DisplayOK()
{
	FONT_MATRIX **p;
	int i=0;
	lcd_clear(lineall);
		p = (FONT_MATRIX **)working_ok;
		do{
			lcm_write_hz1(1,i,p[i]);
			i++;
		}while(p[i]!=NULL);

}
void DisplayError()
{
	lcd_clear(lineall);
	lcm_write_hz1(1,1,(FONT_MATRIX *)error_gu);
	lcm_write_hz1(1,2,(FONT_MATRIX *)error_zhang);
}
void DisplayClockError()
{
	lcd_clear(lineall);
	lcm_write_hz1(1,1,(FONT_MATRIX *)time_shi);
	lcm_write_hz1(1,2,(FONT_MATRIX *)time_jian);
	lcm_write_hz1(1,3,(FONT_MATRIX *)error_cuo);
	lcm_write_hz1(1,4,(FONT_MATRIX *)error_wu);
}
void Display_Scan_Udisk()
{
	lcd_clear(lineall);
	lcm_write_hz1(1,1,(FONT_MATRIX *)scan_sao);
	lcm_write_hz1(1,2,(FONT_MATRIX *)scan_miao);
	lcm_write_hz1(1,3,(FONT_MATRIX *)udisk_u);
	lcm_write_hz1(1,4,(FONT_MATRIX *)udisk_pan);
}

void Display_Save()
{
	lcd_clear(lineall);
	lcm_write_hz1(0,0,(FONT_MATRIX *)being_zheng);
	lcm_write_hz1(0,1,(FONT_MATRIX *)being_zai);
	lcm_write_hz1(0,2,(FONT_MATRIX *)save_bao);
	lcm_write_hz1(0,3,(FONT_MATRIX *)save_cun);
	////////////
	lcm_write_ez ( 0, 19, (FONT_MATRIX *)percent);
	lcm_write_ez ( 1, 0, (FONT_MATRIX *)process_bar1);
	lcm_write_ez ( 0, 18, (FONT_MATRIX *)digital_1);
	
	////////////
	
}

void Display_Fail()
{
	//lcm_clear_ram(LINE2);
	lcd_clear(lineall);
	lcm_write_hz1(1,1,(FONT_MATRIX *)operate_cao);
	lcm_write_hz1(1,2,(FONT_MATRIX *)operate_zuo);
	lcm_write_hz1(1,3,(FONT_MATRIX *)fail_shi);
	lcm_write_hz1(1,4,(FONT_MATRIX *)fail_bai);
}

void Display_udisk_full()
{
	lcd_clear(lineall);
	lcm_write_hz1(1,1,(FONT_MATRIX *)udisk_u);
	lcm_write_hz1(1,2,(FONT_MATRIX *)udisk_pan);
	lcm_write_hz1(1,3,(FONT_MATRIX *)udisk_kong);
	lcm_write_hz1(1,4,(FONT_MATRIX *)udisk_jian);
	lcm_write_hz1(1,5,(FONT_MATRIX *)udisk_yi);
	lcm_write_hz1(1,6,(FONT_MATRIX *)udisk_man);
}
void DisplayTestDoorFail()
{
	lcd_clear(lineall);

	lcm_write_hz1(1,2,(FONT_MATRIX *)door_jian);
	lcm_write_hz1(1,3,(FONT_MATRIX *)door_ce);
	lcm_write_hz1(1,4,(FONT_MATRIX *)fail_shi);
	lcm_write_hz1(1,5,(FONT_MATRIX *)fail_bai);
}
void DisplayTestDoorSucc()
{
	lcd_clear(lineall);
	lcm_write_hz1(1,2,(FONT_MATRIX *)door_jian);
	lcm_write_hz1(1,3,(FONT_MATRIX *)door_ce);
	lcm_write_hz1(1,4,(FONT_MATRIX *)succ_cheng);
	lcm_write_hz1(1,5,(FONT_MATRIX *)succ_gong);
}
//add by panhui 2005-02-20, for ɽ�������б꼼��Ҫ��
void DisplayAlarm()
{
	if((AlarmFlag & 0x03)!=0)
	{//��ʾ�����١�
		lcm_write_hz1(0,6,(FONT_MATRIX *)over_chao);
		lcm_write_hz1(0,7,(FONT_MATRIX *)speed_su);
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
//* Object              : ��ʾ������״̬
//* Input Parameters    : 
//* Output Parameters   : none
//* Functions called    : 
//* ���õ�ȫ�ֱ���      :
//* �޸ĵ�ȫ�ֱ���      :
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
