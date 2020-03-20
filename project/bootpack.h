
/* graphic.c */
#define COL8_000000		0		//黑
#define	COL8_FF0000		1		//亮红
#define	COL8_00FF00		2		//亮绿
#define	COL8_FFFF00		3		//亮黄
#define	COL8_0000FF		4		//亮蓝
#define	COL8_FF00FF		5		//亮紫
#define	COL8_00FFFF		6		//浅亮蓝
#define COL8_FFFFFF		7		//白
#define COL8_C6C6C6		8		//亮灰
#define COL8_840000		9		//暗红
#define COL8_008400		10		//暗绿
#define COL8_848400		11		//暗黄
#define COL8_000084		12		//暗青
#define COL8_840084		13		//暗紫
#define COL8_008484		14		//浅暗蓝
#define COL8_848484		15		//暗灰

/*告诉C编译器，有一个函数在别的文件里*/

//C语言函数的声明
//调色板设定函数1，生成颜色数组
void init_palette(void);
//调色板设定函数2，将颜色数组设置到内存中
void set_palette(int start, int end, unsigned char *rgb);
//界面绘制，对界面(x0, y0)/(x1, y1)范围内的界面设置成颜色编号为c的颜色，xsize是界面横向总宽度
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
//使用上面的函数绘制界面
void init_screen(char *p, int xsize, int ysize);
//绘制8x16大小的字符，x,y是字符的起始坐标点，c是颜色，font是代表字符的16字节数据，每一位代表一种颜色
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
//绘制字符串
void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
//生成鼠标的颜色数组
void init_mouse_cursor8(char *mouse, char bc);
//将鼠标的图案画到界面中
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
int px0, int py0, char *buf, int bxsize);

//void write_mem8(int addr, int data);

/*是函数声明却不用{}，而用；，这表示的意思是：函数是在别的文件中，你自己找一下吧！*/

/*naskfunc.nas */
//函数的具体实现由汇编语言实现
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
//下面两个函数分别是把“段”和中断使用汇编写到指定的位置
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
int load_cr0(void);
void store_cr0(int cr0);
unsigned int memtest_sub(unsigned int start, unsigned int end);

/*dsctbl.c */
//保存段信息的结构体
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

//保存中断信息的结构体
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

//调用“段”和中断的初始化函数初始化这两部分的内存
void init_gdtidt(void);
//初始化内存“段”的函数，把数据写到某一块内存中
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
//初始化中断的函数， 把数据写到某一块内存中
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

#define	ADR_IDT			0x0026f800
#define	LIMIT_IDT		0x000007ff
#define	ADR_GDT			0x00270000
#define	LIMIT_GDT		0x0000ffff
#define	ADR_BOTPAK		0x00280000
#define	LIMIT_BOTPAK	0x0007ffff
#define	AR_DATA32_RW	0x4092
#define	AR_CODE32_ER	0x409a
#define	AR_INTGATE32	0x008e



//该结构体保存的是显示区长宽、启动区长度，启动区起始地址等数据
//因为这些数据通过汇编已经写到了内存中，所以下面的代码只是把对应内存中的地址给了该结构体
/* asmhead.nas */
struct BOOTINFO {	//0x0ff0~0x0fff
	char cyls;		//启动区读取硬盘读到何处为止
	char leds;		//启动时键盘LED的状态
	char vmode;		//显卡模式为多少位彩色
	char reserve;
	short scrnx, scrny;	//画面分辨率
	char *vram;
};
#define ADR_BOOTINFO		0x00000ff0

/* int.c*/
struct KEYBUF {
	unsigned char data[32];
	int next_r, next_w, len;
};

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1
#define	PORT_KEYDAT		0x0060
void init_pic(void);

/* fifo.c*/
struct FIFO8 {
	unsigned char *buf;		//缓冲区的起始指针
	int p;					//下一个写入地址next_w
	int q;					//下一个读出地址next_r
	int size;				//缓冲区的总字节数
	int free;				//缓冲区没有数据的字节数
	int flags;				//是否充满缓冲区的标志
};

#define FLAGS_OVERRUN	0x0001
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

/* keyboard.c */
#define	PORT_KEYDAT				0x0060
#define	PORT_KEYSTA				0x0064
#define	PORT_KEYCMD				0x0064
#define	KEYSTA_SEND_NOTREADY	0x02
#define	KEYCMD_WRITE_MODE		0x60
#define	KBC_MODE				0x47
#define	KEYCMD_SENDTO_MOUSE		0xd4
#define	MOUSECMD_ENABLE			0xf4

void wait_KBC_sendready(void);
void init_keyboard(void);

/* mouse.c */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};

void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);


/* memory.c */
#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000
#define MEMMAN_FREES		4090		//大约是32KB
#define MEMMEN_ADDR			0x003c0000

struct FREEINFO {
	//可用信息
	unsigned int addr, size;
};

struct MEMMAN {
	//内存管理
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free [MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
unsigned int memman_total(struct MEMMAN *man);
void memman_init(struct MEMMAN *man);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);


/* sheet.c */
#define MAX_SHEETS		256

struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
};

struct SHTCTL {
	unsigned char *vram;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

//初始化图层结构体
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
//创建一个新的图层
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
//设置图层的基本数据
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
//调整图层的高度
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height);
//刷新所有的图层
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1);
//上下左右移动图层
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0);
//释放已使用图层的内存的函数
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht);
//在限制范围内刷新界面
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1);
