
#include<stdio.h>
#include "bootpack.h"

//extern struct KEYBUF keybuf;
extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void init_keyboard(void);
void enable_mouse(void);

void HariMain(void)
{
	//int i;			/*变量声明：i是一个32位整数*/
	//char *p = 0xa0000;
	//p = 0x0000;
	//int xsize = 320, ysize = 200;
	
	//char *vram;			//显示内存的起始地址的指针
	//int xsize, ysize;	//显示区域的宽和高
	//short *binfo_scrnx, *binfo_scrny;		//该指针指向的地址保存的是显示区域的宽高信息
	//int *binfo_vram;						//这是个指针，指针的内容也是个指针，内容指针表示*vram
	
	struct BOOTINFO *info = (struct BOOTINFO *) 0x0ff0;
	char s[40], keybuf[32], mousebuf[128];
	int mx, my, i;
	char mcursor[256];
	
	//初始化“段”和中断以及pic,并且设置sti,使CPU能够接收外部设备的中断
	init_gdtidt();
	init_pic();
	io_sti();
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);
	
	init_keyboard();
	
	init_palette();				//设定调色板
	
	//binfo_scrnx = (short *) 0x0ff4;
	//binfo_scrny = (short *) 0x0ff6;
	//binfo_vram = (int *) 0x0ff8;
	
	//xsize = *binfo_scrnx;
	//ysize = *binfo_scrny;
	//vram = (char *) *binfo_vram;
	
	
	
	init_screen((*info).vram, info -> scrnx, info -> scrny);
	
	putfont8_asc(info -> vram, info -> scrnx, 8, 8, COL8_FFFFFF, "ABC 123");
	
	mx = (info -> scrnx - 16) / 2;
	my = (info -> scrny - 28 -16) / 2;
	
	//显示mx和my的值
	sprintf(s, "(%d, %d)", mx, my);
	putfont8_asc((*info).vram, (*info).scrnx, 8, 32, COL8_FFFFFF, s);
	
	//显示鼠标图案
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(info -> vram, info -> scrnx, 16, 16, mx, my, mcursor, 16);
	
	enable_mouse();
	
	for(;;) {
		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			//i = keybuf.data[keybuf.next_r];
			//keybuf.len--;
			//keybuf.next_r++;
			//if(keybuf.next_r == 32) {
			//	keybuf.next_r = 0;
			//}
			if(fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(info -> vram, info -> scrnx, COL8_008484, 0, 16, 15, 31);
				putfont8_asc(info -> vram, info -> scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(info -> vram, info -> scrnx, COL8_008484, 32, 16, 47, 31);
				putfont8_asc(info -> vram, info -> scrnx, 32, 16, COL8_FFFFFF, s);
			}
		}
	}
	
	//static char font_A[16] = {
	//	0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
	//	0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	//};
	
	//putfont8(info -> vram, info -> scrnx, 5, 5, COL8_FFFFFF, font_A);
	
	
	//for(i = 0xa0000; i <= 0xaffff; i++) {
	//	//write_mem8(i, 15);	/*MOV BYTE[I], 15*/
	//	//write_mem8(i, i & 0x0f);
	//	//*((char *) i) = i & 0x0f;
	//	p[i] = i & 0x0f;
	//}
	
	//boxfill8(p, 320, COL8_FF0000, 20, 20, 120, 120);
	//boxfill8(p, 320, COL8_00FF00, 70, 50, 170, 150);
	//boxfill8(p, 320, COL8_0000FF, 120, 80, 220, 180);
	
	//fin:
		//io_hlt();	/*执行naskfunc.nas里的_io_hlt*/
		//goto fin;
	for(;;) {
		io_hlt();
	}
}


#define	PORT_KEYDAT				0x0060
#define	PORT_KEYSTA				0x0064
#define	PORT_KEYCMD				0x0064
#define	KEYSTA_SEND_NOTREADY	0x02
#define	KEYCMD_WRITE_MODE		0x60
#define	KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	//等待键盘控制电路准备完毕
	for(;;) {
		if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define	KEYCMD_SENDTO_MOUSE		0xd4
#define	MOUSECMD_ENABLE			0xf4

void enable_mouse(void)
{
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;		//顺利的话，键盘控制器会返回ACK(0xfa)
}