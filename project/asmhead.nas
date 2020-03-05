; haribote-os
; TAB=4

BOTPAK	EQU		0x00280000		; bootpackのロード先
DSKCAC	EQU		0x00100000		; ディスクキャッシュの場所
DSKCAC0	EQU		0x00008000		; ディスクキャッシュの場所（リアルモード）

; 有关BOOT_INFO
CYLS		EQU		0X0FF0		; 设定启动区
LEDS		EQU		0X0FF1		; 小键盘LED灯
VMODE		EQU		0X0FF2		; 关于颜色数目的信息。颜色的位数。
SCRNX		EQU		0X0FF4		; 分辨率的X（screen x)
SCRNY		EQU		0X0FF6		; 分辨率的Y（screen y)
VRAM		EQU		0X0FF8		; 图像缓冲区的开始地址

	ORG		0xc200		;这个程序将要被装载到内存的什么地方呢？
	
	MOV		AL, 0X13	; VGA显卡，320x200x8位彩色
	MOV		AH, 0X00
	INT		0X10
	MOV		BYTE [VMODE], 8		; 记录画面模式
	MOV		WORD [SCRNX], 320
	MOV		WORD [SCRNY], 200
	MOV		DWORD [VRAM], 0X000A0000
	
; 用BIOS取得键盘上各种LED指示灯的状态
	MOV		AH, 0X02
	INT		0X16				; keyboard BIOS
	MOV		[LEDS], AL
;fin:
;	HLT
;	JMP		fin

;为方便程序能够正常的执行，需要对内存进行分段
;因为CPU的指定会时不时的有PIC发送过来的中断，所以也要初始化中断
;下面的是书中的正文

;PIC关闭一切中断
;	根据AT兼容机的规格，如果要初始化PIC
;	必须在CLI之前进行，否则有时会挂起
;	随后进行PIC的初始化
	MOV		AL, 0XFF
	OUT		0X21, AL		;禁止主PIC的全部中断
	NOP						;如果连续执行OUT指令，有些机种会无法正常运行。所以执行NOP，NOP是让CPU休息一个时钟长的时间
	OUT		0XA1, AL		;禁止从PIC的全部中断
	
	CLI						;禁止CPU级别的中断

;为了让CPU能够访问1MB以上的内存空间，设定A20GATE

	CALL	waitkbdout		;调用waitkbdout函数，CALL是把下一条指令放到栈里面，
							;当waitkbdout函数执行完（也就是执行到RET，RET在栈里面）的时候，
							;从栈里面取出下一条指令继续执行
	MOV		AL, 0XD1
	OUT		0X64, AL
	CALL	waitkbdout
	MOV		AL, 0XDF
	OUT		0X60, AL		;执行后，开启A20，否则为了兼容旧版，只能使用1MB内存
	CALL	waitkbdout
	
;切换到保护模式
[INSTRSET	"i486p"]		;“想要使用486指令”的叙述,下面的关键字LGDT、EAX、CR0都是属于486的
	LGDT	[GDTR0]			;设定GDT，也就是“段”
	MOV		EAX, CR0
	AND		EAX, 0X7FFFFFFF	;设bit31为0，（为了禁止分页）
	OR		EAX, 0X00000001	;设bit0为1（为了切换到保护模式）
	MOV		CR0, EAX		;执行了这条指令，就说明切换到了带保护的32模式
							;因为CPU为了快速的执行，所以利用了管道。
							;在上一条指令还在执行的时候，下一条甚至再下一条指令就开始解释了。
							;但是因为切换前解释的指令并不适用切换后的模式，
							;所以切换后的第一时间就要执行JMP指令，重新解释一遍
	JMP		pipelineflush
	
pipelineflush:
	MOV		AX, 1 * 8		;可读写的段，第8字节（32bit），以下指令是将各个段寄存器设置成该段
	MOV		DS, AX
	MOV		ES, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		SS, AX			;暂时不处理代码段寄存器CS，书中暂时没讲
	
; bootpack的转送
	MOV		ESI, bootpack	; 在该汇编文件的最后一行只有一个bootpack标签，
							; 通过编译链接后，标签bootpack指向的就是bootpack.c文件
	MOV		EDI, BOTPAK
	MOV		ECX, 512 * 1024 / 4
	CALL	memcpy			;把数据源的地址保存在源变址寄存器，把数据要保存到的地址交给目的变址寄存器
							;调用memcpy（汇编写的）函数移动数据，memcpy函数使用计数寄存器计数
							;又因为每一次移动4个字节的数据（因为是32位模式），所以要移动的总字节数要除以4
							
; 磁盘数据最终转送到它本来的位置去

	MOV		ESI, 0X7C00		;转送源
	MOV		EDI, DSKCAC		;转送目的地
	MOV		ECX, 512 / 4
	CALL	memcpy
	
; 所有剩下的
	MOV		ESI, DSKCAC0 + 512		; 转送源
	MOV		EDI, DSKCAC + 512		; 转送目的地
	MOV		ECX, 0
	MOV		CL, BYTE [CYLS]
	IMUL	ECX, 512 * 18 * 2 / 4	; ECX乘以后面的数字，2是磁头，18是单磁头一个柱面的扇区数，512是一个扇区的字节数
									; CL中保存的是要转移的柱面数
									; 所以改行指令的意思是：从柱面数变换为字节数 / 4
	SUB		ECX, 512 / 4			; 减去 IPL
	CALL	memcpy
	
; 必须由asmhead来完成的工作，至此全部完毕
; 以后就交由bootpack来完成

; bootpack的启动
	MOV		EBX, BOTPAK
	MOV		ECX, [EBX + 16]
	ADD		ECX, 3					; ECX += 3；
	SHR		ECX, 2					; SHR表示右移2位，所以结果是：ECX / 4
	JZ		skip					; 没有要转送的东西时，直接跳转到skip，否则先执行转送，然后执行skip
	MOV		ESI, [EBX + 20]			; 转送源
	ADD		ESI, EBX
	MOV		EDI, [EBX + 12]			; 转送目的地
	CALL	memcpy
	
skip:
	MOV		ESP, [EBX + 12]			; 栈初始值
	JMP		DWORD	2 * 8:0X0000001B	; 到这里就是开始执行bootpack.c文件中的内容了
	
waitkbdout:
	IN		AL, 0X64
	AND		AL, 0X02
	IN		AL, 0X60				; 空读（为了清空数据接收缓冲区中的垃圾数据）
	JNZ		waitkbdout				; AND的结果如果不是0， 就跳到waitkbdout
	RET
	
memcpy:
	MOV		EAX, [ESI]
	ADD		ESI, 4
	MOV		[EDI], EAX
	ADD		EDI, 4
	SUB		ECX, 1
	JNZ		memcpy					; 减法运算的结果如果不是0，就跳转到memcpy
	RET
	
	ALIGNB	16
GDT0:
	RESB	8						; NULL selector
	DW		0XFFFF, 0X0000, 0X9200, 0X00CF	; 可以读写的段（segment) 32bit
	DW		0XFFFF, 0X0000, 0X9A28, 0X0047	; 可以执行的段（segment）32bit （bootpack用）
	
	DW		0
	
GDTR0:
	DW		8 * 3 - 1
	DD		GDT0
	
	ALIGNB	16
	
bootpack:
