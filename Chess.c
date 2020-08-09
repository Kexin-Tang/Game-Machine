#include<stdio.h>
#include<xil_io.h>
#include<xil_printf.h>
#include<xparameters.h>
#include "VGA_CChess.h"
#include "Win.h"

#define TFT_FRAME_ADDR	XPAR_MIG7SERIES_0_HIGHADDR-0x001fffff


int Board[10][9];
extern int key;
extern int game;



void VGA_Board_Chess(int y,int x,char *Image,int Iw)
{
	unsigned int addr,color;
	for(int i=0;i<43;i++)
	{
		for(int j=0;j<43;j++)
		{
			addr = (unsigned int)Image+(i*Iw+j)*4;
			color = Xil_In32(addr);
			if(color!=0x0) Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),color);
		}
	}
}

void VGA_Block_Chess(int y,int x,char *Image,int Iw)
{
	unsigned int addr,color;
	for(int i=0;i<43;i++)
	{
		for(int j=0;j<43;j++)
		{
			addr = (unsigned int)Image+(i*Iw+j)*4;
			color = Xil_In32(addr);
			Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),color);
		}
	}
}

void VGA_Word(int y,int x,char *Word,int Wc)
{
	unsigned int addr,display;
	for(int i=0;i<32;i++)
	{
		addr = (unsigned int)Word+i*4;
		display = Xil_In32(addr);
		for(int j=0;j<32;j++)
		{
			if(display&(Smaple_N<<j)) Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+y)+j+x),Wc);
		}
	}
}


void VGA_Piece(int CY,int CX,int P)
{
	unsigned int y,x;
	y= CY*43+21;
	x= CX*43+105+21;
	VGA_Block_Chess(y,x,&gImage_Board[(y*430+x-105)*4],430);
	if(P)
	{
		VGA_Board_Chess(y,x,gImage_Piece[P/8],43);
		VGA_Word (y+7,x+9,gWord_Piece[P],Colour[P/8]);
	}
}

void VGA_Cursor(int CY,int CX)
{
	int y,x;
	y= CY*43+21;
	x= CX*43+105+21;
	VGA_Board_Chess(y,x,gImage_Cursor,43);
}



int CChess_Reset(int *R1,int *R2,int *R3,int *R4,int *R5){

	Xil_Out32(XPAR_AXI_TFT_0_BASEADDR+0x00,TFT_FRAME_ADDR); //向AR寄存器写数据

	int addr,color;

	for(int i=0;i<480;i++){
		for(int j=0;j<640;j++){
			if(j>=105&&j<535&&i<473){
				addr = (unsigned int)gImage_Board+(i*430+j-105)*4;
				color = Xil_In32(addr);
				Xil_Out32(TFT_FRAME_ADDR+4*(1024*i+j),color);
			}
			else Xil_Out32(TFT_FRAME_ADDR+4*(1024*i+j),0x00000000);
		}
	}                            //清屏 显示空棋盘

	for(int i=0;i<10;i++){
		for(int j=0;j<9;j++){
			Board[i][j]= reset[i*9+j];
			VGA_Piece(i,j,Board[i][j]);
		}
	}                             //布局初始化 显示棋子

	VGA_Board_Chess(216,21,gImage_Piece[0],43);
	                           //显示先手

	*R1= *R2= *R3= *R4= *R5= 0;
}


void CChess_Move(int *CX,int *CY,int *TX,int *TY,int *Side,int *Step,int *Win){
	int SX,SY;
	int EX,EY;
	int PX,PY;
	int dX,dY,L;
	int Barrier;
	int ENS,ENE;
	unsigned int color,addr;

	if(*Win) return 0;

	*Step=!(*Step);
	if(*Step){
		*TY = *CY;
		*TX = *CX;
	}

	else{
		SY = *TY;
		SX = *TX;
		EY = *CY;
		EX = *CX;

		//---Prepare Part---
		dY = (EY>SY)? (EY-SY): (SY-EY);
		dX = (EX>SX)? (EX-SX): (SX-EX);
		L  = (dX>dY)? dX: dY;

		PX = EX; PY = EY; Barrier = 0;
		for(int i=1;i<L;i++){
			PY = (PY>SY)? (PY-1): (PY<SY? PY+1: PY);
			PX = (PX>SX)? (PX-1): (PX<SX? PX+1: PX);
			if(Board[PY][PX]) Barrier++;
		}

		//---Decision Part---
		int WE = 0;
		ENS = ((Board[SY][SX]/8)==*Side) && (Board[SY][SX]!=0);
		ENE = ((Board[EY][EX]/8)!=*Side) || (Board[EY][EX]==0);
		if(ENS&& ENE&& L) switch(Board[SY][SX]%8){

			case 1:{	WE =((dX==0|| dY==0)&&
							 (Board[EY][EX]==9-*Side*8&& Barrier==0))||
							((dX+dY==1)&&
							 (EX<=5&& EX>=3)&& (EY<=2|| EY>=7));	break;}

			case 2:{	WE = (dX+dY==2&& L==1)&&
   							 (EX<=5&& EX>=3)&& (EY<=2|| EY>=7);		break;}

			case 3:{	WE = (dX+dY==4&& L==2)&&
			     			 (Barrier==0) &&
				  			 (EY<=*Side*5+4)&& (EY>=*Side*5)  ;		break;}

			case 4:{	WE = (dX+dY==3&& L==2) &&
			     			 (Barrier==0);							break;}

			case 5:{	WE = (dX==0|| dY==0) &&
							 (Barrier==0);							break;}

			case 6:{	WE = (dX==0|| dY==0) &&
			     			((Board[EY][EX]!=0&& Barrier==1)||
							 (Board[EY][EX]==0&& Barrier==0)) ;		break;}

			case 7:{	WE = (dX+dY==1) && ((!*Side)?
			     			 ((SY<5&& EY>SY)|| (SY>4&& EY>=SY)):
			    			 ((SY>4&& EY<SY)|| (SY<5&& EY<=SY)));	break;}

			default:{   WE = 0;										break;}
		}
		else			WE = 0;

		//---Movement Part---
		if(WE){
			*Win = (Board[EY][EX]%8==1);

			Board[EY][EX] = Board[SY][SX];
			Board[SY][SX] = 0;

			VGA_Piece(EY,EX,Board[EY][EX]);
			VGA_Piece(SY,SX,0);

			if(*Win){
				for(int i=0;i<59;i++)
				{
					for(int j=0;j<153;j++)
					{
						addr = (unsigned int)gImage_Win[*Side]+(i*153+j)*4;
						color = Xil_In32(addr);
						if(color!=0x0) Xil_Out32(TFT_FRAME_ADDR+4*(1024*(i+212)+j+245),color);
					}
				}
				return 0;
			}

			*Side = !*Side;
			VGA_Board_Chess(216,21,gImage_Piece[*Side],43);

		}
	}
}


int CChess(){
	int CursorX,CursorY;
	int TempX,TempY;
	int Side,Step,Win;

	CChess_Reset(&CursorX,&CursorY,&Side,&Step,&Win);
	VGA_Cursor(CursorY,CursorX);

	while(game == 2)
	{
		switch(key)
		{
			case 0x2D: {VGA_Piece(CursorY,CursorX,Board[CursorY][CursorX]);

					    CursorY= (CursorY==9)? 0: CursorY+1;
						VGA_Cursor(CursorY,CursorX);
						if(Step) VGA_Cursor(TempY,TempX);
						key=0;		break;}
				//"down"

			case 0x3D: {VGA_Piece(CursorY,CursorX,Board[CursorY][CursorX]);

					    CursorY= (CursorY==0)? 9: CursorY-1;
						VGA_Cursor(CursorY,CursorX);
						if(Step) VGA_Cursor(TempY,TempX);
						key=0;		break;}
				//"up"

			case 0x2B: {VGA_Piece(CursorY,CursorX,Board[CursorY][CursorX]);

					        CursorX= (CursorX==8)? 0: CursorX+1;
						VGA_Cursor(CursorY,CursorX);
						if(Step) VGA_Cursor(TempY,TempX);
						key=0;		break;}
				//"right"

			case 0x2E: {VGA_Piece(CursorY,CursorX,Board[CursorY][CursorX]);

					        CursorX= (CursorX==0)? 8: CursorX-1;
						VGA_Cursor(CursorY,CursorX);
						if(Step) VGA_Cursor(TempY,TempX);
						key=0;		break;}
				//"left"

			case 0x0d: {CChess_Move (&CursorX,&CursorY,&TempX,&TempY,&Side,&Step,&Win);
						VGA_Cursor(CursorY,CursorX);
						key=0;		break;}
				//"确定"

			case 0x0B:	{key=0;	CChess_Reset(&CursorX,&CursorY,&Side,&Step,&Win);
							VGA_Cursor(CursorY,CursorX);break;}
			default: {break;}
		}
	}
}
