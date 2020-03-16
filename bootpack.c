/*  */
#include<stdio.h>

#define COL8_000000 	0	/*黑*/
#define COL8_FF0000		1	/*亮红*/
#define COL8_00FF00		2	/*亮绿*/
#define COL8_FFFF00 	3	/*亮黄*/
#define	COL8_0000FF		4	/*亮蓝*/
#define COL8_FF00FF		5	/*亮紫*/
#define COL8_00FFFF		6	/*浅亮蓝*/
#define COL8_FFFFFF		7	/*白*/
#define	COL8_C6C6C6		8	/*亮灰*/
#define	COL8_840000		9	/*暗红*/
#define COL8_008400		10	/*暗绿*/
#define COL8_848400		11	/*暗黄*/
#define COL8_000084		12	/*暗蓝*/
#define COL8_840084		13	/*暗紫*/
#define COL8_008484		14	/*浅暗蓝*/
#define COL8_848484		15	/*暗灰*/

void io_hlt(void);
void io_cli(void);
void io_out8(int port,int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);
void init_palette(void);
void set_palette(int start,int end,unsigned char *rgb);
void init_screen(char* vram,int xsize,int ysize);
void putfont8(char *vram,int xsize,int x,int y,char c,char *font);
void putfont8_asc(char *vram,int xsize,int x,int y,char c,unsigned char *s);
void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,
                 int px0,int py0,char *buf,int bxsize);
void init_mouse_cursor8(char *mouse,char bc);
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd,unsigned int limit,int base,int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd,int offset,int selector,int ar);
				 
struct BOOTINFO{
	char cyls,leds,vmode,reserve;
	short scrnx,scrny;
	char *vram;
};
/*
static char font_A[16]={
	0x00,0x18,0x18,0x18,0x18,0x24,0x24,0x24,
	0x24,0x7e,0x42,0x42,0x42,0xe7,0x00,0x00
	
};
*/

struct SEGMENT_DESCRIPTOR{
	short limit_low,base_low;
	char base_mid,access_right;
	char limit_high,base_high;
};

struct GATE_DESCRIPTOR{
	short offset_low,selector;
	char dw_count,access_right;
	short offset_high;
};



void HariMain(void)
{
	struct 	BOOTINFO *binfo=(struct BOOTINFO *)0x0ff0;
//	char *s;/*存储要显示的变量*/
	init_palette();/*设定调色板*/
	init_screen(binfo->vram,binfo->scrnx,binfo->scrny);/*初始化屏幕*/
	
	putfont8_asc(binfo->vram,binfo->scrnx,8,8,COL8_FFFFFF,"jdsa,DDD");
	putfont8_asc(binfo->vram,binfo->scrnx,31,31,COL8_000000,"hello,hom!Welcome to your home.");
	
	//sprintf(s,"scrnx=%x",0x100A);/*显示打印变量*/
	//putfont8_asc(binfo->vram,binfo->scrnx,31,47,COL8_000000,s);/*显示打印变量*/
	char my_mouse[256];
	init_mouse_cursor8(my_mouse,COL8_848484);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,100,100,my_mouse,16);
	for(;;){
		io_hlt(); /*调用naskfunc.nas中的_io_hlt */
	}
}

void init_gdtidt(void){
	struct SEGMENT_DESCRIPTOR *gdt =(struct SEGMENT_DESCRIPTOR *)0x00270000;
	struct GATE_DESCRIPTOR    *idt =(struct GATE_DESCRIPTOR    *)0x0026f800;
	int i;
	/* GDT的初始化*/
	for(i=0;i<8192;i++){
		set_segmdesc(gdt+i,0,0,0);
	}
	set_segmdesc(gdt+1,0xffffffff,0x00000000,0x4092);
	set_segmdesc(gdt+2,0x0007ffff,0x00280000,0x409a);
	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd,unsigned int limit,int base,int ar)
{/* limit:段的字节数-1，base:基址,ar:访问权限*/
	if (limit >0xfffff){
		ar |=0x8000;
		limit/=0x1000;
	}
	sd->limit_low	=limit & 0xffff;
	sd->base_low	=base & 0xffff;
	sd->base_mid	=(base >> 16)& 0xff;
	sd->access_right=ar&0xff;
	sd->limit_high	=((limit >>16) & 0x0f) | ((ar>>8)&0xf0);
	sd->base_high	=(base>>24)&0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd,int offset,int selector,int ar){
	
	gd->offset_low	=offset & 0xffff;
	gd->selector	=selector;
	gd->dw_count	=(ar>>8) & 0xff;
	gd->access_right=ar & 0xff;
	gd->offset_high	=(offset >> 16) & 0xffff;
	return;
}

void init_mouse_cursor8(char *mouse,char bc){
/*准备鼠标指针（16*16）*/	
	static char cursor[16][16]={
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOO***OOO*.....",
		"*OO*...*OOO*....",
		"*O*.....*OOO*...",
		".........*OOO*..",
		"..........*OOO*.",
		"...........*OO*.",
		"............***."
	};
	int x,y;
	for(y=0;y<16;y++){
		for(x=0;x<16;x++){
			if(cursor[y][x]=='*')mouse[y*16+x]=COL8_000000;
			if(cursor[y][x]=='O')mouse[y*16+x]=COL8_FFFFFF;
			if(cursor[y][x]=='.')mouse[y*16+x]=bc;
		}
		
	}
	return;
}

void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,
                 int px0,int py0,char *buf,int bxsize){
	int x,y;
	for(y=0;y<pysize;y++)
		for(x=0;x<pxsize;x++)
			vram[(py0+y)*vxsize+px0+x]=buf[y*16+x];
	return;
	
}

void putfont8(char *vram,int xsize,int x,int y,char c,char *font){
	int i;
	char d;
	char *p;
	for(i=0;i<16;i++){
		p=vram+(y+i)*xsize+x;
		d=font[i];
		if((d & 0x80)!=0)p[0]=c;
		if((d & 0x40)!=0)p[1]=c;
		if((d & 0x20)!=0)p[2]=c;
		if((d & 0x10)!=0)p[3]=c;
		if((d & 0x08)!=0)p[4]=c;
		if((d & 0x04)!=0)p[5]=c;
		if((d & 0x02)!=0)p[6]=c;
		if((d & 0x01)!=0)p[7]=c;
		
	}
	return;
}

void putfont8_asc(char *vram,int xsize,int x,int y,char c,unsigned char *s){
	extern char hankaku[4096];
	for(;*s!=0x00;s++){
		putfont8(vram,xsize,x,y,c,hankaku+*s*16);
		x+=8;
	}
	return;
	
}

void init_palette(void)
{
static unsigned char table_rgb[16*3]=
{
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */

};
set_palette(0,15,table_rgb);
return;	
	
	
}

void set_palette(int start,int end,unsigned char *rgb)
{
	
	int i,eflags;
	eflags=io_load_eflags();
	io_cli();
	io_out8(0x03c8,start);
	for(i=start;i<=end;i++){
	io_out8(0x03c9,rgb[0]/4);
	io_out8(0x03c9,rgb[1]/4);
	io_out8(0x03c9,rgb[2]/4);
	rgb+=3;
	}
	io_store_eflags(eflags);
	return;
}

void boxfill8(unsigned char*vram,int xsize,unsigned char c,int x0,int y0,int x1,int y1)
{
	int x,y;
	for(y=y0;y<=y1;y++)
	{
		for(x=x0;x<=x1;x++)vram[y*xsize+x]=c;
		
	}
	return;
	
}

void init_screen(char* vram,int xsize,int ysize)
{


	boxfill8(vram,xsize,COL8_848484,0,			   0,	xsize-1,ysize-29);
	boxfill8(vram,xsize,COL8_C6C6C6,0,		ysize-28,	xsize-1,ysize-28);
	boxfill8(vram,xsize,COL8_FFFFFF,0,		ysize-27,	xsize-1,ysize-27);
	boxfill8(vram,xsize,COL8_C6C6C6,0,		ysize-26,	xsize-1,ysize-1);
	boxfill8(vram,xsize,COL8_FFFFFF,3,		ysize-24,		 59,ysize-24);
	boxfill8(vram,xsize,COL8_FFFFFF,2,		ysize-24,	      2,ysize-4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
	
	return;
}
