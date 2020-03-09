
#include "bootpack.h"

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

//struct KEYBUF keybuf;
struct FIFO8 keyfifo;

void inthandler21(int *esp)
/*来自PS/2键盘的中断*/
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);		//通知PIC“IRQ-01已经受理完毕”
	data = io_in8(PORT_KEYDAT);
	//if(keybuf.len < 32) {
	//	keybuf.data[keybuf.next_w] = data;
	//	keybuf.next_w++;
	//	keybuf.len++;
	//	if(keybuf.next_w == 32) {
	//		keybuf.next_w = 0;
	//	}
	//}
	fifo8_put(&keyfifo, data);
	return;
	
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	//unsigned char data, s[4];
	//io_out8(PIC0_OCW2, 0x61);		//通知PIC“IRQ-01已经受理完毕”
	//data = io_in8(PORT_KEYDAT);
	
	//sprintf(s, "%02X", data);
	
	//boxfill8(binfo -> vram, binfo -> scrnx, COL8_008484, 0, 16, 15, 31);
	//putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
	
	//return;
}
