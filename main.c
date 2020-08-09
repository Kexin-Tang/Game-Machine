/*
 * 		Button:	XPAR_GPIO_0_BASEADDR 					0x40010000
 * 		Int:	XPAR_INTC_0_BASEADDR 					0x41200000
 * 		Timer:	XPAR_AXI_TIMER_0_BASEADDR 				0x41C00000
 */
//============================================================
//			regs	P239				��ַ
//
//			ISR				XPAR_INTC_0_BASEADDR+0x00	(�ж�״̬�Ĵ�������1���ж�)
//			IER				XPAR_INTC_0_BASEADDR+0X08	(�ж�ʹ�ܼĴ�����д1���ж�)
//			IAR				XPAR_INTC_0_BASEADDR+0X0C	(�ж���Ӧ�Ĵ�����д1���ж�)
//			MER				XPAR_INTC_0_BASEADDR+0X1C	(0x03����Ӳ���жϺ����ж�)
//============================================================
//=================================================================
//			GPIO_reg					��ַ
//			IER				XPAR_GPIO_0_BASEADDR+0X128
//			ISR				XPAR_GPIO_0_BASEADDR+0X120
//			GIE				XPAR_GPIO_0_BASEADDR+0X11C
//=================================================================
//=================================================================
//			Timer_reg					��ַ
//			TCSR			XPAR_AXI_TIMER_0_BASEADDR+0X00
//			TLR				XPAR_AXI_TIMER_0_BASEADDR+0X04
//			TCR				XPAR_AXI_TIMER_0_BASEADDR+0X08
//=================================================================
#include<xparameters.h>
#include<stdio.h>
#include<xil_io.h>
#include<xil_printf.h>
#include "VGA_main.h"

#define TFT_FRAME_ADDR	XPAR_MIG7SERIES_0_HIGHADDR-0x001fffff

#define INT_ISR 0
#define INT_IER 8
#define INT_IAR 12
#define INT_MER 28

#define GPIO_DATA1 0x0
#define GPIO_TRI1 0x4
#define GPIO_DATA2 0x8
#define GPIO_TRI2 0xc
#define GPIO_IER 0x128
#define GPIO_ISR 0x120
#define GPIO_GIE 0x11c

#define TIME_SR 0x0
#define TIME_LR 0x4
#define TIME_RESET 50000000

int key = 0;
int posedge = 0;
int game = 0;

void KeyBoard_Handler();
void Timer_Handler();
void My_Handler()__attribute__((interrupt_handler));

int Snake ();
int getkey();

void wallpaper()
{
	int i,j,addr,color;
	for(i=0;i<480;i++)
		for(j=0;j<640;j++)
		{
			addr = (unsigned int)gImage_main+(i*640+j)*4;
			color = Xil_In32(addr);
			Xil_Out32(TFT_FRAME_ADDR+4*(1024*i+j),color);
		}

}


int main()
{
	int i,j,addr,color;

	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+GPIO_TRI1,0xf);
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+GPIO_TRI2,0x0);
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+GPIO_DATA2,0x0);
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+GPIO_IER ,0x1);
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+GPIO_GIE ,0x80000000);
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+GPIO_ISR,Xil_In32(XPAR_AXI_GPIO_0_BASEADDR+GPIO_ISR));

	Xil_Out32(XPAR_INTC_0_BASEADDR+INT_IER,0x81);
	Xil_Out32(XPAR_INTC_0_BASEADDR+INT_MER,0x3);

	microblaze_enable_interrupts();

	Xil_Out32(XPAR_AXI_TFT_0_BASEADDR+0x00,TFT_FRAME_ADDR);

	wallpaper();

	while(1)
	{
		switch(game)
		{
		case 1:{Snake();	wallpaper();	break;}
		//̰����
		case 2:{CChess();	wallpaper();	break;}
		//�й�����
		case 3:{MSweep();	wallpaper();	break;}
		//ɨ��
		default: break;
		}
	}
}

//========================
//		���ж�
//========================
void My_Handler()
{
	int flag = Xil_In32(XPAR_MICROBLAZE_0_AXI_INTC_BASEADDR+0x00);

	if((flag&0x01) == 0x01)				//��Timer�������ж�
		Timer_Handler();
	//else if((flag&0x02) == 0x02)		//��Button�������ж�
	//	Button_Handler();
	else if ((flag&0x80)==0x80)
		KeyBoard_Handler();
	else
		;

	Xil_Out32(XPAR_MICROBLAZE_0_AXI_INTC_BASEADDR+0x0c,flag);		//����жϱ�־
}

void KeyBoard_Handler()
{
	int N=0xf7;
	int y,x;

	for(int i=0;i<100;i++);                                 //����
	y = Xil_In8(0x40030000+GPIO_DATA1)& 0xf;

	for(x=0;x<4;x++)
	{
		Xil_Out8(0x40030000+GPIO_DATA2,N>>x);
		if((Xil_In8(0x40030000+GPIO_DATA1)& 0xf)!= 0xf) break;
	}
	key = y+(x<<4);          //ȡ����ֵ
	xil_printf("key=%x\n",key);

	Xil_Out8(0x40030000+GPIO_DATA2,0x0);
	while((Xil_In8(0x40030000+GPIO_DATA1)& 0xf)!= 0xf);      //�ȴ������ɿ�

	for(int i=0;i<100;i++);
	Xil_Out32(0x40030000+GPIO_ISR,Xil_In32(0x40030000+GPIO_ISR));

	switch(key)
	{
	case 0x37:	{game = 1; key=0;	break;}
	case 0x27:	{game = 2; key=0;	break;}
	case 0x17:	{game = 3; key=0;	break;}
	case 0x07:	{game = 4; key=0;	break;}
	default:	break;
	}
}



//=========================
//		��ʱ���ж�
//=========================
void Timer_Handler()
{
	posedge=1;
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR));
}

