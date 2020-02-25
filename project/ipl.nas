; hello-os
; TAB=4

CYLS	EQU		10		; EQU相当于C语言中的预定义#define命令，意思是：CYLS = 10

	ORG		0x7c00		; 指明程序的装载地址
	
; 以下的记述用于标准FAT12格式的软盘

	JMP		entry
	DB		0x90

; 以下这段是标准FAT32格式软盘专用的代码

	;DB	0XEB, 0X4E, 0X90
	DB	"HELLOIPL"		; 启动区的名称可以是任意字符串（8字节）
	DW	512				; 每个扇区（sector）的大小（必须为512字节）
	DB	1				; 簇（cluster）的大小（必须为1个扇区）
	DW	1				; FAT的起始位置（一般从第一个扇区开始）
	DB	2				; FAT的个数（必须为2）
	DW	224				; 根目录的大小（一般设成224项）
	DW	2880			; 该磁盘的大小（必须是2880扇区）
	DB	0xf0			; 磁盘的种类（必须是0xf0)
	DW	9				; FAT的长度（必须是9扇区）
	DW	18				; 1个磁道（track）有几个扇区（必须是18）
	DW	2				; 磁头数（必须是2）
	DD	0				; 不使用分区，必须是0
	DD	2880			; 重写一次磁盘大小
	DB	0, 0, 0x29		; 意义不明，固定
	DD	0xffffffff		; （可能是）卷标号码
	DB	"HELLO-OS   "	; 磁盘的名称（11字节）
	DB	"FAT12   "		; 磁盘格式名称（8字节）
	RESB	18			; 先空出18字节
	
; 程序主题

;	DB	0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
;	DB	0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
;	DB	0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
;	DB	0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
;	DB	0xee, 0xf4, 0xeb, 0xfd

entry:
	MOV		AX, 0		; 初始化寄存器
	MOV		SS, AX
	MOV		SP, 0X7C00
	MOV		DS, AX
	
	MOV		SI, msg
	MOV		CL, 1
	
putloop:
	MOV		AL, [SI]
	ADD		SI, 1		; 给si加1
	CMP		AL, 0
	
	;JE		fin
	JE		judge
	MOV		AH, 0X0E	; 显示一个文字
	MOV		BX, 15		; 指定字符颜色
	INT		0X10		; 调用显卡BIOS
	JMP		putloop
	
judge:
	CMP		CL, 1
	JE		read2
	CMP		CL, 2
	JE		fin
	CMP		CL, 3
	JE		nextipl
	
nextipl:
	MOV		[0x0ff0],CH		; IPLがどこまで読んだのかをメモ
	JMP		0xc200
	
read2:
	MOV		AX, 0X0820
	MOV		ES, AX
	MOV		CH, 0		; 柱面0
	MOV		CL, 2		; 扇区2（扇区数从1开始）
	MOV		DH, 0		; 磁头0
	
readloop:
	MOV		SI, 0
	
retry:
	
	MOV		AH, 0X02	; AH=0x02 : 读盘
	MOV		AL, 1		; (要读取的扇区的数量）1个扇区
	MOV		BX, 0
	MOV		DL, 0X00	; A驱动器
	INT		0X13		; 调用磁盘BIOS
	JNC		next		; 没出错的话跳转到fin
	ADD		SI, 1		; 往SI加1
	CMP		SI, 5		; 比较SI与5
	JAE		error		; SI >= 5时，跳转到error
	MOV		AH, 0X00	
	MOV		DL, 0X00	; A驱动器
	INT		0X13		; 重置驱动器
	JMP		retry
	
next:
	MOV		AX, ES		; 把内存地址后移200
	ADD		AX, 0X020
	MOV		ES, AX		; 因为没有ADD ES， 0x20指令，所以这里稍微绕个弯
	ADD		CL, 1		; 往CL里加1
	CMP		CL, 18		; 比较CL与18
	JBE		readloop	; 如果CL <= 18， 跳转至readloop
	MOV		CL, 1		; 扇区初始化
	ADD		DH, 1		; 磁头加1
	CMP		DH, 2
	JB		readloop	; 如果DH < 2，则跳转到readloop
	MOV		DH, 0
	ADD		CH, 1		; 磁盘的柱面加1
	CMP		CH, CYLS
	JB		readloop	; 如果CH < CYLS，则跳转到readloop
	JMP		msg2		; 如果把18个扇区都加载到内存，则输出成功语句，hello，world
	
error:
	MOV		SI, errormsg
	MOV		CL, 2
	JMP		putloop
	
msg2:
	MOV		SI, msg
	MOV		CL, 3
	JMP		putloop
	
fin:
	HLT					; 让CPU停止， 等待指令
	JMP		fin			; 无线循环
	
; 信息显示部分

errormsg:
	DB		0x0a, 0x0a		; 2个换行
	DB		"load error"
	DB		0x0a			; 换行
	DB		0

msg:
	DB	0x0a, 0x0a		; 2个换行
	DB	"hello, world"
	DB	0x0a			; 换行
	DB	0
	
	RESB	0x7dfe-$		; 填写0x00,直到 0x001fe
	DB	0x55, 0xaa
	
; 以下是启动区以外部分的输出

;	DB	0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
;	RESB	4600
;	DB	0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
;	RESB	1469432