#文件生成规则
OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj \
keyboard.obj mouse.obj


TOOLPATH 	= ..\z_tools
INCPATH		= ..\z_tools\haribote

MAKE		= $(TOOLPATH)\make.exe -r
NASK		= $(TOOLPATH)\nask.exe
CC1			= $(TOOLPATH)\cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK	= $(TOOLPATH)\gas2nask.exe -a
OBJ2BIM		= $(TOOLPATH)\obj2bim.exe
MAKEFONT	= $(TOOLPATH)\makefont.exe
BIN2OBJ		= $(TOOLPATH)\bin2obj.exe
BIM2HRB		= $(TOOLPATH)\bim2hrb.exe
RULEFILE	= $(TOOLPATH)\haribote\haribote.rul
EDIMG		= $(TOOLPATH)\edimg.exe
IMGTOL		= $(TOOLPATH)\imgtol.com
COPY		= copy
DEL			= del

default :
	$(MAKE) img

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas	ipl.bin ipl.lst
	
asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst
	
#dsctbl.gas : dsctbl.c Makefile
#	$(CC1) -o dsctbl.gas dsctbl.c

#dsctbl.nas : dsctbl.gas Makefile
#	$(GAS2NASK) dsctbl.gas dsctbl.nas
	
#dsctbl.obj : dsctbl.nas Makefile
#	$(NASK) dsctbl.nas dsctbl.obj dsctbl.lst
	
#graphic.gas : graphic.c Makefile
#	$(CC1) -o graphic.gas graphic.c
	
#graphic.nas : graphic.gas Makefile
#	$(GAS2NASK) graphic.gas graphic.nas
	
#graphic.obj : graphic.nas Makefile
#	$(NASK) graphic.nas graphic.obj bootpack.lst
	
#bootpack.gas : bootpack.c Makefile
#	$(CC1) -o bootpack.gas bootpack.c
	
#bootpack.nas : bootpack.gas Makefile
#	$(GAS2NASK) bootpack.gas bootpack.nas
	
#bootpack.obj : bootpack.nas Makefile
#	$(NASK) bootpack.nas bootpack.obj bootpack.lst
	
#naskfunc.obj : naskfunc.nas Makefile
#	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c
	
%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas
	
%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst
	
hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin
	
hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku
	
bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
	
haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys
	
helloos.img : ipl.bin haribote.sys Makefile
#一行写不下，如果一个命令写成多行，连接符只能是“\“，并且”\“后面不能有任何字符，包括空格符
	$(EDIMG)	imgin:..\z_tools\fdimg0at.tek \
	wbinimg src:ipl.bin len:512 from:0 to:0	\
	copy from:haribote.sys to:@: \
	imgout:helloos.img
	
img :
	$(MAKE) helloos.img

asm :
	$(MAKE) ipl.bin
	
run :
	$(MAKE) img
	copy helloos.img $(TOOLPATH)\qemu\fdimage0.bin
	$(MAKE) -C $(TOOLPATH)\qemu
	
clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL)	*.obj
	-$(DEL) bootpack.nas
	-$(DEL) graphic.nas
	-$(DEL) dsctbl.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
	
#注释要在一行的头位置开始写，否则会被当做命令执行
#-$(DEL) ipl.bin
#-$(DEL) ipl.lst
#-$(DEL) haribote.sys
#-$(DEL) haribote.lst
	
src_only :
	$(MAKE) clean
	-$(DEL) helloos.img