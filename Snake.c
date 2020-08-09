#include<stdio.h>
#include<stdlib.h>
#include<xparameters.h>
#include<xil_printf.h>
#include<xil_io.h>

//extern int dir;
extern int key;
extern int posedge;
extern int game;
extern int Random;

int my_rand;
int Board[16][16];
int Body[256][2];

int Drection;

#define NK 0x00000000
#define NW 0x00FFFFFF
#define NY 0x00FFFF00
#define NR 0x00FF0000

#define TFT_FRAME_ADDR XPAR_MIG7SERIES_0_HIGHADDR-0x001fffff
//#define XPAR_AXI_TFT_0_BASEADDR 0x44A10000

void VGA_Board_Snake(int y,int x,u32 color){
	for(int i=0;i<25;i++){
		for(int j=0;j<25;j++){
			Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),color);
		}
	}
}

//========================
//		游戏初始化
//========================
int Snake_Reset(int Board[16][16],int Body[256][2],int *H,int *T,int *L,int *D,int *F,int *R){

	Xil_Out32(XPAR_AXI_TFT_0_BASEADDR+0x00,TFT_FRAME_ADDR);		//向AR寄存器写数据
	for(int i=0;i<480;i++){
		for(int j=0;j<640;j++){
			if((j<=79)||(j>=560))
			{
				Xil_Out32(TFT_FRAME_ADDR+4*(1024*i+j),0x55555555);
			}
			else
				Xil_Out32(TFT_FRAME_ADDR+4*(1024*i+j),NK);
		}
	}

	for(int i=0;i<16;i++)
	{
		for(int j=0;j<16;j++)
		{
			Board[i][j]= 0;
		}
	}

	Body[0][1] =Body[0][0] =4;
	*H= *T= 0;
	*D= 1;
	*R= 0;
	*L= 1;
	Board[4][4] =1;

	VGA_Board_Snake(4*30,4*30+80,NW);

	while(1)
	{
		my_rand=rand()%256;
		if(Board[my_rand/16][my_rand%16] == 0)
			break;
	}


	Board[my_rand/16][my_rand%16] = 2;

	VGA_Board_Snake((my_rand/16)*30,(my_rand%16)*30+80,NY);

}

//========================
//		蛇的移动
//========================
int Snake_Move (int Board[16][16],int Body[256][2],int *H,int *T,int *L,int *D,int *F,int *R)
{
	int _NY,_NX;

	if(*L==256) return 0;


	switch(*D){
		case 1:{_NY= Body[*H][1]+ 1;  _NX= Body[*H][0]+ 0;	break;}

		case 2:{_NY= Body[*H][1]- 1;  _NX= Body[*H][0]+ 0;	break;}

		case 3:{_NY= Body[*H][1]+ 0;  _NX= Body[*H][0]+ 1;	break;}

		case 4:{_NY= Body[*H][1]+ 0;  _NX= Body[*H][0]- 1;	break;}

		//default: return 0;
	}


	if(*R= (Board[_NY][_NX]==1&& (_NY!=Body[*T][1]|| _NX!=Body[*T][0]))||      //eat self
		   (_NY<0|| _NY>15|| _NX<0|| _NX>15)|| *R)                             //hit wall
	{
		Board[Body[*H][1]][Body[*H][0]] = 3;
		VGA_Board_Snake((Body[*H][1])*30,(Body[*H][0])*30+80,NR);
		Random++;
	}

	else switch(Board[_NY][_NX]){
		case 2:{                          //food eating
			Board[_NY][_NX] = 1;

			VGA_Board_Snake(_NY*30,_NX*30+80,NW);

			*H= (*H==255)? 0: *H+1;
			Body[*H][1]= _NY;
			Body[*H][0]= _NX;
			*L=*L+1;


			while(1)
			{
				my_rand=rand()%256;
				if(!Board[my_rand/16][my_rand%16])
					break;
			}
			Board[my_rand/16][my_rand%16] = 2;

			VGA_Board_Snake((my_rand/16)*30,(my_rand%16)*30+80,NY);

			break;
		}

		default:{                          //normal movement
			Board[_NY][_NX] = 1;
			Board[Body[*T][1]][Body[*T][0]] = 0;

			VGA_Board_Snake(_NY*30,_NX*30+80,NW);
			VGA_Board_Snake(Body[*T][1]*30,Body[*T][0]*30+80,NK);

			*H= (*H==255)? 0: *H+1;
			*T= (*H==255)? 0: *T+1;
			Body[*H][1]= _NY;
			Body[*H][0]= _NX;

			break;
		}
	}
}

//========================
//		游戏主函数
//========================
int Snake()
{
	int Head,Tail;
	int Food,Length;
	int Re;

	srand(654);

	Snake_Reset(Board,Body,&Head,&Tail,&Length,&Drection,&Food,&Re);
	Init();

	while(game == 1)
	{

		if(posedge)
		{
			posedge = 0;
			Snake_Move (Board,Body,&Head,&Tail,&Length,&Drection,&Food,&Re);
		}

		switch(key)
		{
			case 0x2D: {Drection = (Drection==2)? 2: 1;	key=0;	break;}
								//"down"
			case 0X3D: {Drection = (Drection==1)? 1: 2;	key=0;	break;}
								//"up"
			case 0X2B: {Drection = (Drection==4)? 4: 3;	key=0;	break;}
								//"right"
			case 0X2E: {Drection = (Drection==3)? 3: 4;	key=0;	break;}
								//"left"
			case 0x0B:	{key=0;	Snake_Reset(Board,Body,&Head,&Tail,&Length,&Drection,&Food,&Re); Random++;	break;}
			default: {break;}
		}
	}


}
