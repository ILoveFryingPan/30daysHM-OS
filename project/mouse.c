
#include "bootpack.h"

void enable_mouse(struct MOUSE_DEC *mdec)
{
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec -> phase = 0;
	return;		//顺利的话，键盘控制器会返回ACK(0xfa)
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if(0 == mdec -> phase) {
		if(0xfa == dat) {		//等待鼠标的0xfa的状态
			mdec -> phase = 1;
		}
		return 0;
	}
	if(1 == mdec -> phase) {
		if((dat & 0xc8) == 0x08) {	//因为第一个字节高四位的值为0~3，低八位的值为8,9,a,c,通过if判断，如果为true，则满足条件
			mdec -> buf[0] = dat;		//等待鼠标的第一个字节
			mdec -> phase = 2;
		}
		return 0;
	}
	if(2 == mdec -> phase) {
		mdec -> buf[1] = dat;		//等待鼠标的第二个字节
		mdec -> phase = 3;
		return 0;
	}
	if(3 == mdec -> phase) {
		mdec -> buf[2] = dat;		//等待鼠标的第三个字节
		mdec -> phase = 1;
		mdec -> btn = mdec -> buf[0] & 0x07;	//经过与运算后，btn可能值为1,2,4，分别代表左点击，又点击，滚轮点击
		mdec -> x = mdec -> buf[1];
		mdec -> y = mdec -> buf[2];
		if((mdec -> buf[0] & 0x10) != 0) {
			mdec -> x |= 0xffffff00;
		}
		if((mdec -> buf[0] & 0x20) != 0) {
			mdec -> y |= 0xffffff00;
		}
		mdec -> y = -mdec -> y;					//鼠标的y方向与画面符号相反
		return 1;
	}
	return -1;
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
