/*
 * 		Button:	XPAR_GPIO_0_BASEADDR 					0x40010000
 * 		Int:	XPAR_INTC_0_BASEADDR 					0x41200000
 * 		Timer:	XPAR_AXI_TIMER_0_BASEADDR 				0x41C00000
 */
//============================================================
//			regs	P239				地址
//
//			ISR				XPAR_INTC_0_BASEADDR+0x00	(中断状态寄存器，读1有中断)
//			IER				XPAR_INTC_0_BASEADDR+0X08	(中断使能寄存器，写1开中断)
//			IAR				XPAR_INTC_0_BASEADDR+0X0C	(中断响应寄存器，写1清中断)
//			MER				XPAR_INTC_0_BASEADDR+0X1C	(0x03开启硬件中断和总中断)
//============================================================
//=================================================================
//			GPIO_reg					地址
//			IER				XPAR_GPIO_0_BASEADDR+0X128
//			ISR				XPAR_GPIO_0_BASEADDR+0X120
//			GIE				XPAR_GPIO_0_BASEADDR+0X11C
//=================================================================
//=================================================================
//			Timer_reg					地址
//			TCSR			XPAR_AXI_TIMER_0_BASEADDR+0X00
//			TLR				XPAR_AXI_TIMER_0_BASEADDR+0X04
//			TCR				XPAR_AXI_TIMER_0_BASEADDR+0X08
//=================================================================
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<xparameters.h>
#include<xil_printf.h>
#include<xil_io.h>

#define VALUE 50000000

#define GPIO_DATA1 0x0
#define GPIO_TRI1 0x4
#define GPIO_DATA2 0x8
#define GPIO_TRI2 0xc
#define GPIO_IER 0x128
#define GPIO_ISR 0x120
#define GPIO_GIE 0x11c

#define INT_ISR 0
#define INT_IER 8
#define INT_IAR 12
#define INT_MER 28

//int dir=0;				//记录蛇的方向
extern int posedge;			//记录定时器中断
extern int key;


//========================
//	初始化相应寄存器
//========================
void Init()
{
	//Xil_Out16(XPAR_GPIO_0_BASEADDR+0x04,0xffff);			//设置按键为输入

	//配置GPIO中断
	//Xil_Out32(XPAR_GPIO_0_BASEADDR+0x128,0x1);				//设置GPIO的IER中bit0==1，开启通道1中断
	//Xil_Out32(XPAR_GPIO_0_BASEADDR+0x11c,0x80000000);		//设置GPIO的GIE中bit31=1，允许GPIO输出中断

	//配置Timer中断
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR)&0x77f);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+0x04,VALUE);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR)|0x20);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR&0x7df)|(0xd2));

}

/*
//========================
//		总中断
//========================
void My_Handler()
{
	int flag = Xil_In32(XPAR_MICROBLAZE_0_AXI_INTC_BASEADDR+0x00);

	if((flag&0x01) == 0x01)				//即Timer发生的中断
		Timer_Handler();
	//else if((flag&0x02) == 0x02)		//即Button发生的中断
	//	Button_Handler();
	else if ((flag%0x80)==0x80)
		KeyBoard_Handler();
	else
		;

	Xil_Out32(XPAR_MICROBLAZE_0_AXI_INTC_BASEADDR+0x0c,flag);		//清除中断标志
}
*/

/*
//========================
//		按键中断
//========================
void Button_Handler()
{
	int KEY = Xil_In8(XPAR_GPIO_0_BASEADDR+0x00)&0xff;		//读取按键键值
	switch(KEY)
	{
		//ok
		case 0x01:
		{
			delay();
			if((Xil_In8(XPAR_GPIO_0_BASEADDR+0x00)&0xff) == 0x01)
				xil_printf("ok\n");
			dir=0;
			break;
		}

		//up
		case 0x02:
		{
			delay();
			if((Xil_In8(XPAR_GPIO_0_BASEADDR+0x00)&0xff) == 0x02)
				dir=1;
			break;
		}

		//left
		case 0x04:
		{
			delay();
			if((Xil_In8(XPAR_GPIO_0_BASEADDR+0x00)&0xff) == 0x04)
				dir=2;
			break;
		}

		//right
		case 0x08:
		{
			delay();
			if((Xil_In8(XPAR_GPIO_0_BASEADDR+0x00)&0xff) == 0x08)
				dir=3;
			break;
		}

		//down
		case 0x10:
		{
			delay();
			if((Xil_In8(XPAR_GPIO_0_BASEADDR+0x00)&0xff) == 0x10)
				dir=4;
			break;
		}

		default:
		{
			dir=0;
			break;
		}
	}
	Xil_Out32(XPAR_GPIO_0_BASEADDR+0x120,Xil_In8(XPAR_GPIO_0_BASEADDR+0x120));
}
*/

/*
void KeyBoard_Handler()
{
	int N=0xf7;
	int y,x;

	for(int i=0;i<100;i++);                                 //防抖
	y = Xil_In8(0x40030000+GPIO_DATA1)& 0xf;

	for(x=0;x<4;x++)
	{
		Xil_Out8(0x40030000+GPIO_DATA2,N>>x);
		if((Xil_In8(0x40030000+GPIO_DATA1)& 0xf)!= 0xf) break;
	}
	key = y+(x<<4);          //取按键值
	xil_printf("key=%x\n",key);

	Xil_Out8(0x40030000+GPIO_DATA2,0x0);
	while((Xil_In8(0x40030000+GPIO_DATA1)& 0xf)!= 0xf);      //等待按键松开

	for(int i=0;i<100;i++);
	Xil_Out32(0x40030000+GPIO_ISR,Xil_In32(0x40030000+GPIO_ISR));
}
*/


/*
//=========================
//		定时器中断
//=========================
void Timer_Handler()
{
	posedge=1;
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR));
}
*/


//=========================
//			延时
//==========================
void delay()
{
	int i,j;
	for(i=10000;i>0;i--)
		for(j=1000;j>0;j--)
			;
}
