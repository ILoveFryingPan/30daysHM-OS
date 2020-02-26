
#include "bootpack.h"

void init_pic(void)
{
	/*PIC的初始化*/
	io_out8(PIC0_IMR, 0xff);		//禁止所有中断
	io_out8(PIC1_IMR, 0xff);		//禁止所有中断
	
	io_out8(PIC0_ICW1, 0x11);		//边沿触发模式 （edge trigger mode)
	io_out8(PIC0_ICW2, 0x20);		//IRQ0-7由INT20-27接收
	io_out8(PIC0_ICW3, 1 << 2);		//PIC1由IRQ2连接
	io_out8(PIC0_ICW4, 0x01);		//无缓冲区模式
	
	io_out8(PIC1_ICW1, 0x11);		//边沿触发模式 （edge trigger mode)
	io_out8(PIC1_ICW2, 0x28);		//IRQ8-15由INT28-2f接收
	io_out8(PIC1_ICW3, 2);			//PIC1由IRQ2连接
	io_out8(PIC1_ICW4, 0x01);		//无缓冲区模式
	
	io_out8(PIC0_IMR, 0xfb);		//11111011 PIC1以外全部禁止
	io_out8(PIC1_IMR, 0xff);		//11111111禁止所有中断
	
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

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
/*来自PS/2鼠标的中断*/
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	//通知PIC1  IRQ-12的受理已经完成
	io_out8(PIC0_OCW2, 0x62);	//通知PIC0  IRQ-02的受理已经完成
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
	
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	//boxfill8(binfo -> vram, binfo -> scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	//putfont8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	//for (;;) {
	//	io_hlt();
	//}
}

void inthandler27(int *esp)
/**
对于一部分机种，随着PIC的初始化，会产生一次IRQ7中断，如果不对该中断处理程序执行STI（设置中断标志），操作系统的启动会失败
*/
{
	io_out8(PIC0_OCW2, 0x67);	//具体看7-1章
	return;
}
