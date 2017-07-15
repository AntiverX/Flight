################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
..\r_bsp/board/rskrx23t/dbsct.c \
..\r_bsp/board/rskrx23t/hwsetup.c \
..\r_bsp/board/rskrx23t/lowlvl.c \
..\r_bsp/board/rskrx23t/lowsrc.c \
..\r_bsp/board/rskrx23t/resetprg.c \
..\r_bsp/board/rskrx23t/sbrk.c \
..\r_bsp/board/rskrx23t/vecttbl.c 

C_DEPS += \
./r_bsp/board/rskrx23t/dbsct.d \
./r_bsp/board/rskrx23t/hwsetup.d \
./r_bsp/board/rskrx23t/lowlvl.d \
./r_bsp/board/rskrx23t/lowsrc.d \
./r_bsp/board/rskrx23t/resetprg.d \
./r_bsp/board/rskrx23t/sbrk.d \
./r_bsp/board/rskrx23t/vecttbl.d 

OBJS += \
./r_bsp/board/rskrx23t/dbsct.obj \
./r_bsp/board/rskrx23t/hwsetup.obj \
./r_bsp/board/rskrx23t/lowlvl.obj \
./r_bsp/board/rskrx23t/lowsrc.obj \
./r_bsp/board/rskrx23t/resetprg.obj \
./r_bsp/board/rskrx23t/sbrk.obj \
./r_bsp/board/rskrx23t/vecttbl.obj 


# Each subdirectory must supply rules for building sources it contributes
r_bsp/board/rskrx23t/%.obj: ../r_bsp/board/rskrx23t/%.c
	@echo 'Scanning and building file: $<'
	@echo 'Invoking: Scanner and Compiler'
	ccrx  -MM -MP -output=dep="$(@:%.obj=%.d)" -MT="$(@:%.obj=%.obj)" -MT="$(@:%.obj=%.d)" -lang=c99   -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp\mcu\rx23t\register_access","C:\Users\Antiver\Documents\Workspace\Flight\src\cg_src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets\rx23t","C:\Users\Antiver\Documents\Workspace\Flight\src\SrcUser","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp","C:\Users\Antiver\Documents\Workspace\Flight\r_config","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_1","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_2","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_3","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_4","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -optimize=0 -fpu -nologo  -define=__RX=1   "$<"
	ccrx -lang=c99 -output=obj="$(@:%.d=%.obj)"  -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp\mcu\rx23t\register_access","C:\Users\Antiver\Documents\Workspace\Flight\src\cg_src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets\rx23t","C:\Users\Antiver\Documents\Workspace\Flight\src\SrcUser","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp","C:\Users\Antiver\Documents\Workspace\Flight\r_config","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_1","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_2","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_3","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_4","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -optimize=0 -fpu -nologo  -define=__RX=1   "$<"
	@echo 'Finished scanning and building: $<'
	@echo.

