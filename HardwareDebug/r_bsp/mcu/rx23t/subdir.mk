################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
..\r_bsp/mcu/rx23t/cpu.c \
..\r_bsp/mcu/rx23t/locking.c \
..\r_bsp/mcu/rx23t/mcu_clocks.c \
..\r_bsp/mcu/rx23t/mcu_init.c \
..\r_bsp/mcu/rx23t/mcu_interrupts.c \
..\r_bsp/mcu/rx23t/mcu_locks.c 

C_DEPS += \
./r_bsp/mcu/rx23t/cpu.d \
./r_bsp/mcu/rx23t/locking.d \
./r_bsp/mcu/rx23t/mcu_clocks.d \
./r_bsp/mcu/rx23t/mcu_init.d \
./r_bsp/mcu/rx23t/mcu_interrupts.d \
./r_bsp/mcu/rx23t/mcu_locks.d 

OBJS += \
./r_bsp/mcu/rx23t/cpu.obj \
./r_bsp/mcu/rx23t/locking.obj \
./r_bsp/mcu/rx23t/mcu_clocks.obj \
./r_bsp/mcu/rx23t/mcu_init.obj \
./r_bsp/mcu/rx23t/mcu_interrupts.obj \
./r_bsp/mcu/rx23t/mcu_locks.obj 


# Each subdirectory must supply rules for building sources it contributes
r_bsp/mcu/rx23t/%.obj: ../r_bsp/mcu/rx23t/%.c
	@echo 'Scanning and building file: $<'
	@echo 'Invoking: Scanner and Compiler'
	ccrx  -MM -MP -output=dep="$(@:%.obj=%.d)" -MT="$(@:%.obj=%.obj)" -MT="$(@:%.obj=%.d)" -lang=c99   -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp\mcu\rx23t\register_access","C:\Users\Antiver\Documents\Workspace\Flight\src\cg_src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets\rx23t","C:\Users\Antiver\Documents\Workspace\Flight\src\SrcUser","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp","C:\Users\Antiver\Documents\Workspace\Flight\r_config","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_1","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_2","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_3","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_4","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -optimize=0 -fpu -nologo  -define=__RX=1   "$<"
	ccrx -lang=c99 -output=obj="$(@:%.d=%.obj)"  -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp\mcu\rx23t\register_access","C:\Users\Antiver\Documents\Workspace\Flight\src\cg_src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets\rx23t","C:\Users\Antiver\Documents\Workspace\Flight\src\SrcUser","C:\Users\Antiver\Documents\Workspace\Flight\r_bsp","C:\Users\Antiver\Documents\Workspace\Flight\r_config","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_1","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_2","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_3","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\flash_type_4","C:\Users\Antiver\Documents\Workspace\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -optimize=0 -fpu -nologo  -define=__RX=1   "$<"
	@echo 'Finished scanning and building: $<'
	@echo.

