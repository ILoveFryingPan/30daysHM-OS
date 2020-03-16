
#include<stdio.h>
#include "bootpack.h"

//extern struct KEYBUF keybuf;
extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

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
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMEN_ADDR;
	
	
	//初始化“段”和中断以及pic,并且设置sti,使CPU能够接收外部设备的中断
	init_gdtidt();
	init_pic();
	io_sti();			//IDT/PIC的初始化已经完成， 于是开放CPU的中断
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	io_out8(PIC0_IMR, 0xf9);		//开放PIC1和键盘中断（11111001）
	io_out8(PIC1_IMR, 0xef);		//开放鼠标中断（11101111）
	
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
	
	enable_mouse(&mdec);
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	i = memtotal / (1024 * 1024);
	memman_init(memman);
	memman_free_4k(memman, 0x00001000, 0x0009e000);	//0x00001000 - 0x0009efff
	memman_free_4k(memman, 0x00400000, memtotal - 0x00400000);
	sprintf(s, "memory %dMB    free : %dKB", i, memman_total(memman) / 1024);
	putfont8_asc(info -> vram, info -> scrnx, 0, 48, COL8_FFFFFF, s);
	
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
				if(mouse_decode(&mdec, i) != 0) {
					//鼠标的三个字节都齐了，显示出来
					sprintf(s, "%02X %02X %02X", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
					boxfill8(info -> vram, info -> scrnx, COL8_008484, 32, 16, 32 + 26 * 8 - 1, 31);
					putfont8_asc(info -> vram, info -> scrnx, 32, 16, COL8_FFFFFF, s);
					
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					putfont8_asc(info -> vram, info -> scrnx, 112, 16, COL8_FFFFFF, s);
					boxfill8(info -> vram, info -> scrnx, COL8_008484, mx, my, mx + 15, my + 15);	//隐藏鼠标
					mx += mdec.x;
					my += mdec.y;
					if(mx < 0) {
						mx = 0;
					}
					if(my < 0) {
						my = 0;
					}
					if(mx > info -> scrnx - 16) {
						mx = info -> scrnx - 16;
					}
					if(my > info -> scrny - 16) {
						my = info -> scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(info -> vram, info -> scrnx, COL8_008484, 0, 0, 79, 15);		//隐藏坐标
					putfont8_asc(info -> vram, info -> scrnx, 0, 0, COL8_FFFFFF, s);		//显示坐标
					putblock8_8(info -> vram, info -> scrnx, 16, 16, mx, my, mcursor, 16);	//描画鼠标
				}
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
