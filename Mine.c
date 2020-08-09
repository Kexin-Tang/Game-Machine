#include<stdio.h>
#include<xil_io.h>
#include<xil_printf.h>
#include<xparameters.h>
#include"VGA_Mine.h"
#include"Mark.h"


#define TFT_FRAME_ADDR	XPAR_MIG7SERIES_0_HIGHADDR-0x001fffff

int Board[16][16];
int Mine[16][16];
int Die,Win,Num,Rest;
int Random=344;

extern int key,game;

//===========================================
void VGA_Board_MSweep(int y,int x,int M){
	unsigned int addr,color;
	for(int i=0;i<30;i++){
		for(int j=0;j<30;j++){
			if(i==0||i==30||j==0||j==30) Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),0x00000000);
			else if(M){
				addr = (unsigned int)gImage_MS_block+(i*30+j)*4;
				color = Xil_In32(addr);
				Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),color);
			}
			else Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),0x00cdcdcd);
		}
	}
}

void VGA_Word_MSweep(int y,int x,int P,int M){
	unsigned int addr,display;
	if(M==0&&P==0) return;
	for(int i=0;i<32;i++){
		addr = (unsigned int)gWord_Number[P]+i*4;
		display = Xil_In32(addr);
		for(int j=0;j<32;j++){
			if(display&(0x00000001<<j)) Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),0x000088ff);
		}
	}
}


void VGA_Piece_MSweep(int CY,int CX,int P){
	int y,x;
	y= CY*30;
	x= CX*30+80;
	VGA_Board_MSweep(y,x,(P>9));
	if(P<9) VGA_Word_MSweep (y-1,x-1,P,0);
	else if(P==0xf)
		for(int i=0;i<30;i++){
		for(int j=0;j<30;j++){
			Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),0x00ff0000);
		}
	}
	else if(P==0xc) VGA_Word_MSweep (y-1,x-1,10,0);
}

void VGA_Cursor_MSweep(int CY,int CX){
	int y,x;
	y= CY*30;
	x= CX*30+80;
	for(int i=0;i<30;i++){
		for(int j=0;j<30;j++){
			if(i<2||i>27||j<2||j>27) Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),0xffff0000);
		}
	}
}


//扫雷初始化
int MSweep_Reset(int *R1,int *R2,int *R3,int *R4,int *R5,int *R6){
	int N;


	for(int i=0;i<480;i++)
		for(int j=0;j<640;j++)
		{
			Xil_Out32(TFT_FRAME_ADDR+4*(1024*i+j),0x00000000);
		}


	for(int i=0;i<16;i++)
		for(int j=0;j<16;j++){
			Board[i][j]= 0xE;
			VGA_Piece_MSweep(i,j,0xE);
			Mine[i][j]= 0;
		}

	*R1= *R2= *R3= *R4= 0;
	*R5= 40;
	*R6= 256;

	VGA_Cursor_MSweep(0,0);

	for(int i=0;i<40;i++){
		while(1){N = rand(Random)%255; if(!Mine[N/16][N%16]) break;}
		Mine[N/16][N%16]= 1;
	}

	//显示剩余的标记数
	int color,addr;
	for(int i=0;i<40;i++)
		{
			for(int j=0;j<80;j++)
			{
				addr = (unsigned int)gImage_Mark+(i*80+j)*4;
				color = Xil_In32(addr);
				Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+220)+j),color);
			}
		}
	VGA_Word_MSweep(232,16,(*R5)/10,1);
	VGA_Word_MSweep(232,32,(*R5)%10,1);
}

//标记地雷
int MSweep_Mark(int X,int Y,int D,int W,int *Num){
	if(D==1|| W==1) return 0;

	switch(Board[Y][X]){
		case 0xe:{
			if(Num==0) return 0;
			Board[Y][X]= 0xc;
			VGA_Piece_MSweep(Y,X,0xc);
			(*Num)--;
			break;
		}
		case 0xc:{
			Board[Y][X]= 0xe;
			VGA_Piece_MSweep(Y,X,0xe);
			(*Num)++;
			break;
		}
		default: return 0;
	}


	int color,addr;
	for(int i=0;i<40;i++)
		{
			for(int j=0;j<80;j++)
			{
				addr = (unsigned int)gImage_Mark+(i*80+j)*4;
				color = Xil_In32(addr);
				Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+220)+j),color);
			}
		}
	VGA_Word_MSweep(232,16,(*Num)/10,1);
	VGA_Word_MSweep(232,32,(*Num)%10,1);
}


//刷新VGA显示
int MSweep_Sweep(int X,int Y){
	int i,j,n=0;

	if(Die==1|| Board[Y][X]!=0Xe|| Win==1) return 0;

	for(i=(Y==0?Y:Y-1);i<=(Y==15?Y:Y+1);i++){
		for(j=(X==0?X:X-1);j<=(X==15?X:X+1);j++){
			if(j!=X|| i!=Y) n= n+Mine[i][j];
		}
	}
	Board[Y][X]=n;
	VGA_Piece_MSweep(Y,X,n);
	Rest--;
	Win =(Rest==25);

	if(n==0){
		if(Y!=15) MSweep_Sweep(X,Y+1);
		if(Y!=0 ) MSweep_Sweep(X,Y-1);
		if(X!=15) MSweep_Sweep(X+1,Y);
		if(X!=0 ) MSweep_Sweep(X-1,Y);
		if(Y!=15&& X!=15) MSweep_Sweep(X+1,Y+1);
		if(Y!=0 && X!=15) MSweep_Sweep(X+1,Y-1);
		if(Y!=15&& X!=0 ) MSweep_Sweep(X-1,Y+1);
		if(Y!=0 && X!=0 ) MSweep_Sweep(X-1,Y-1);
	}
}


//扫雷主函数
int MSweep(){
	int CursorX,CursorY;

	MSweep_Reset(&CursorX,&CursorY,&Die,&Win,&Num,&Rest);

	while(game == 3)
	{
		switch(key){
			case 0x2D: {key=0; VGA_Piece_MSweep(CursorY,CursorX,Board[CursorY][CursorX]);
					  CursorY= (CursorY==15)? 0: CursorY+1;
					  VGA_Cursor_MSweep(CursorY,CursorX);		break;}
				//"down"
			case 0x3D: {key=0; VGA_Piece_MSweep(CursorY,CursorX,Board[CursorY][CursorX]);
			          CursorY= (CursorY==0)? 15: CursorY-1;
			          VGA_Cursor_MSweep(CursorY,CursorX);		break;}
				//"up"
			case 0x2B: {key=0; VGA_Piece_MSweep(CursorY,CursorX,Board[CursorY][CursorX]);
		              CursorX= (CursorX==15)? 0: CursorX+1;
		              VGA_Cursor_MSweep(CursorY,CursorX);		break;}
				//"right"
			case 0x2E: {key=0; VGA_Piece_MSweep(CursorY,CursorX,Board[CursorY][CursorX]);
			          CursorX= (CursorX==0)? 15: CursorX-1;
			          VGA_Cursor_MSweep(CursorY,CursorX);		break;}
				//"left"
			case 0x0D: {key=0; if(!Win) if(Die= Die|| Mine[CursorY][CursorX]){
					 	Board[CursorY][CursorX]=0xf;
					 	VGA_Piece_MSweep(CursorY,CursorX,0xf);	delay(); MSweep_Reset(&CursorX,&CursorY,&Die,&Win,&Num,&Rest);	Random++;
					 	break;
						}
					  MSweep_Sweep(CursorX,CursorY);	VGA_Cursor_MSweep(CursorY,CursorX);		break;}
				//"确定"
			case 0x0E:{key=0; MSweep_Mark (CursorX,CursorY,Die,Win,&Num);	VGA_Cursor_MSweep(CursorY,CursorX);		break;}
				//"M"

			case 0x0B:	{key=0;	MSweep_Reset(&CursorX,&CursorY,&Die,&Win,&Num,&Rest); Random++;	break;}
			default: {break;}
		}
	}
}


