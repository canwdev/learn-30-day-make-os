# 变量
TOOLPATH = ..\z_tools


# 默认行为~

default :
	$(TOOLPATH)\make.exe img

# 文件生成规则

ipl.bin : ipl.nas Makefile
	$(TOOLPATH)\nask.exe ipl.nas ipl.bin ipl.lst

haribote.sys : haribote.nas Makefile
	$(TOOLPATH)\nask.exe haribote.nas haribote.sys haribote.lst

haribote.img : ipl.bin haribote.sys Makefile
	$(TOOLPATH)\edimg.exe  imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img

# 指令

asm :
	$(TOOLPATH)\make.exe -r ipl.bin

img :
	$(TOOLPATH)\make.exe -r haribote.img

run :
	$(TOOLPATH)\make.exe img
	copy haribote.img $(TOOLPATH)\qemu\fdimage0.bin
	$(TOOLPATH)\make.exe -C $(TOOLPATH)\qemu

install :
	$(TOOLPATH)\make.exe img
	$(TOOLPATH)\imgtol.com w a: haribote.img

clean :
	-del ipl.bin
	-del ipl.lst
	-del haribote.sys
	-del haribote.lst

src_only :
	$(TOOLPATH)\make.exe clean
	-del haribote.img