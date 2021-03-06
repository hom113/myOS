# my makefile
TOOLPATH=../tolset/z_tools/
INCPATH=../tolset/z_tools/haribote

MAKE	=$(TOOLPATH)make.exe -r
NASK	=$(TOOLPATH)nask.exe
CC1     =$(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK=$(TOOLPATH)gas2nask.exe -a
OBJ2BIM =$(TOOLPATH)obj2bim.exe
MAKEFONT=$(TOOLPATH)makefont.exe
BIN2OBJ	=$(TOOLPATH)bin2obj.exe
BIM2HRB =$(TOOLPATH)bim2hrb.exe
RULEFILE=$(TOOLPATH)haribote/haribote.rul
EDIMG	=$(TOOLPATH)edimg.exe
IMGTOL	=$(TOOLPATH)imgtol.com
COPY	=copy
DEL	    =del


default:
	$(MAKE) img

#生成bin和lst文件，方便调试
ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst


bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst
    
naskfunc.obj : naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : bootpack.obj naskfunc.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj hankaku.obj
# 3MB+64KB=3136KB    

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
    
haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys
    


#生成img镜像文件
haribote.img : ipl10.bin haribote.sys Makefile
	$(EDIMG)   imgin:../tolset/z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img
        
img:
	$(MAKE) haribote.img


#运行模拟器
run:
	$(MAKE) img
	$(COPY) haribote.img ..\tolset\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../tolset/z_tools/qemu

#当磁盘装到驱动器里时后，运行会往磁盘里装helloos.img(用模拟器忽略这个命令)
install:
	$(MAKE) img
	$(IMGTOL) w a:haribote.img

clean:
	-$(DEL) *.bin
	-$(DEL) *.lst 
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
    
src_only :
	$(MAKE) clean
	-$(DEL) haribote.img