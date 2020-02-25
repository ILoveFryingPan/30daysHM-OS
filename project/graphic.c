
#include "bootpack.h"

void init_palette()
{
	static unsigned char table_rgb[16 *3] = {
		0x00, 0x00, 0x00,		//0：黑
		0xff, 0x00, 0x00,		//1: 亮红
		0x00, 0xff, 0x00,		//2: 亮绿
		0xff, 0xff, 0x00,		//3：亮黄
		0x00, 0x00, 0xff,		//4: 亮蓝
		0xff, 0x00, 0xff,		//5: 亮紫
		0x00, 0xff, 0xff,		//6: 浅亮蓝
		0xff, 0xff, 0xff,		//7: 白
		0xc6, 0xc6, 0xc6,		//8: 亮灰
		0x84, 0x00, 0x00,		//9: 暗红
		0x00, 0x84, 0x00,		//10: 暗绿
		0x84, 0x84, 0x00,		//11: 暗黄
		0x00, 0x00, 0x84,		//12: 暗青
		0x84, 0x00, 0x84,		//13: 暗紫
		0x00, 0x84, 0x84,		//14: 浅暗蓝
		0x84, 0x84, 0x84		//15: 暗灰
	};
	set_palette(0, 15, table_rgb);
	return;
	
	/*C语言中的static char 语句只能用于数据，相当于汇编中的DB指令*/
	/*C语言中的static具有初始化变量的功能*/
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();		//记录中断许可标志的值
	io_cli();						//将中断许可标志置为0，禁止中断
	io_out8(0x03c8, start);			//此处的第一个参数必须是0x03c8
	for(i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);	//将值设置到调色板中，第一个参数必须是0x03c9
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);		//复原中断许可标志
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for(y = y0; y <= y1; y++) {
		for(x = x0; x <= x1; x++) {
			vram[y * xsize + x] = c;
		}
	}
	return;
}

void init_screen(char *p, int xsize, int ysize)
{
	boxfill8(p, xsize, COL8_008484, 	0, 		0, 			xsize - 1, ysize - 29);
	boxfill8(p, xsize, COL8_C6C6C6, 	0, ysize - 28, 		xsize - 1, ysize - 28);
	boxfill8(p, xsize, COL8_FFFFFF, 	0, ysize - 27, 		xsize - 1, ysize - 27);
	boxfill8(p, xsize, COL8_C6C6C6, 	0, ysize - 26, 		xsize - 1, ysize - 1);
	
	boxfill8(p, xsize, COL8_FFFFFF, 	3, ysize - 24, 		59,			ysize - 24);
	boxfill8(p, xsize, COL8_FFFFFF, 	2, ysize - 24, 		2,			ysize - 4);
	boxfill8(p, xsize, COL8_848484, 	3, ysize - 4, 		59,			ysize - 4);
	boxfill8(p, xsize, COL8_848484,		59,ysize - 23, 		59, 		ysize - 5);
	boxfill8(p, xsize, COL8_000000, 	2, 	ysize - 3, 		59, 		ysize - 3);
	boxfill8(p, xsize, COL8_000000, 	60,ysize - 24, 		60,			ysize - 3);
	
	boxfill8(p, xsize, COL8_848484,xsize - 47,ysize - 24, 	xsize - 4, ysize - 24);
	boxfill8(p, xsize, COL8_848484,xsize - 47,ysize - 23, 	xsize - 47, ysize - 4);
	boxfill8(p, xsize, COL8_FFFFFF,xsize - 47,ysize - 3, 	xsize - 4, ysize - 3);
	boxfill8(p, xsize, COL8_FFFFFF,xsize - 3, ysize - 24, 	xsize - 3, ysize - 3);
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d;		//d是数据，p是在屏幕要显示字母数据的某一行的起始地址
	for(i = 0; i <= 15; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if(d & 0x80) { p[0] = c; }
		if(d & 0x40) { p[1] = c; }
		if(d & 0x20) { p[2] = c; }
		if(d & 0x10) { p[3] = c; }
		if(d & 0x08) { p[4] = c; }
		if(d & 0x04) { p[5] = c; }
		if(d & 0x02) { p[6] = c; }
		if(d & 0x01) { p[7] = c; }
	}
	return;
}

void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	for(; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor8(char *mouse, char bc)
{
	//准备鼠标颜色指针（16 x 16）
	static char cursor[16][16] = {
		"**************..",
		"*00000000000*...",
		"*0000000000*....",
		"*000000000*.....",
		"*00000000*......",
		"*0000000*.......",
		"*0000000*.......",
		"*00000000*......",
		"*0000**000*.....",
		"*000*..*000*....",
		"*00*....*000*...",
		"*0*......*000*..",
		"**........*000*.",
		"*..........*000*",
		"............*00*",
		".............***"
	};
	int x, y;
	
	for(y = 0; y < 16; y++) {
		for(x = 0; x < 16; x++) {
			if(cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			
			if(cursor[y][x] == '0') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			
			if(cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for(y = 0; y < pysize; y++) {
		for(x = 0; x < pxsize; x++) {
			vram[(y + py0) * vxsize + px0 + x] = buf[y * bxsize + x];
		}
	}
	return;
}
