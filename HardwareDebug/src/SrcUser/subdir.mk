################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
..\src/SrcUser/ctrl_basic.c \
..\src/SrcUser/ctrl_beep.c \
..\src/SrcUser/ctrl_control.c \
..\src/SrcUser/ctrl_debug.c \
..\src/SrcUser/ctrl_drone_status.c \
..\src/SrcUser/ctrl_encoder.c \
..\src/SrcUser/ctrl_flash.c \
..\src/SrcUser/ctrl_fmu.c \
..\src/SrcUser/ctrl_key.c \
..\src/SrcUser/ctrl_led.c \
..\src/SrcUser/ctrl_map.c \
..\src/SrcUser/ctrl_pid.c \
..\src/SrcUser/ctrl_serial_graph.c \
..\src/SrcUser/ctrl_smp.c \
..\src/SrcUser/ctrl_two_sta_dev.c \
..\src/SrcUser/ctrl_usart.c 

C_DEPS += \
./src/SrcUser/ctrl_basic.d \
./src/SrcUser/ctrl_beep.d \
./src/SrcUser/ctrl_control.d \
./src/SrcUser/ctrl_debug.d \
./src/SrcUser/ctrl_drone_status.d \
./src/SrcUser/ctrl_encoder.d \
./src/SrcUser/ctrl_flash.d \
./src/SrcUser/ctrl_fmu.d \
./src/SrcUser/ctrl_key.d \
./src/SrcUser/ctrl_led.d \
./src/SrcUser/ctrl_map.d \
./src/SrcUser/ctrl_pid.d \
./src/SrcUser/ctrl_serial_graph.d \
./src/SrcUser/ctrl_smp.d \
./src/SrcUser/ctrl_two_sta_dev.d \
./src/SrcUser/ctrl_usart.d 

OBJS += \
./src/SrcUser/ctrl_basic.obj \
./src/SrcUser/ctrl_beep.obj \
./src/SrcUser/ctrl_control.obj \
./src/SrcUser/ctrl_debug.obj \
./src/SrcUser/ctrl_drone_status.obj \
./src/SrcUser/ctrl_encoder.obj \
./src/SrcUser/ctrl_flash.obj \
./src/SrcUser/ctrl_fmu.obj \
./src/SrcUser/ctrl_key.obj \
./src/SrcUser/ctrl_led.obj \
./src/SrcUser/ctrl_map.obj \
./src/SrcUser/ctrl_pid.obj \
./src/SrcUser/ctrl_serial_graph.obj \
./src/SrcUser/ctrl_smp.obj \
./src/SrcUser/ctrl_two_sta_dev.obj \
./src/SrcUser/ctrl_usart.obj 


# Each subdirectory must supply rules for building sources it contributes
src/SrcUser/%.obj: ../src/SrcUser/%.c
	@echo 'Scanning and building file: $<'
	@echo 'Invoking: Scanner and Compiler'
	ccrx  -MM -MP -output=dep="$(@:%.obj=%.d)" -MT="$(@:%.obj=%.obj)" -MT="$(@:%.obj=%.d)" -lang=c99   -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","G:\UAV\myproject\Flight\r_bsp\mcu\rx23t\register_access","G:\UAV\myproject\Flight\src\cg_src","G:\UAV\myproject\Flight\r_flash_rx\src\targets\rx23t","G:\UAV\myproject\Flight\src\SrcUser","G:\UAV\myproject\Flight\r_bsp","G:\UAV\myproject\Flight\r_config","G:\UAV\myproject\Flight\r_flash_rx","G:\UAV\myproject\Flight\r_flash_rx\src","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_1","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_2","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_3","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_4","G:\UAV\myproject\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -fpu -nologo  -define=__RX=1   "$<"
	ccrx -lang=c99 -output=obj="$(@:%.d=%.obj)"  -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","G:\UAV\myproject\Flight\r_bsp\mcu\rx23t\register_access","G:\UAV\myproject\Flight\src\cg_src","G:\UAV\myproject\Flight\r_flash_rx\src\targets\rx23t","G:\UAV\myproject\Flight\src\SrcUser","G:\UAV\myproject\Flight\r_bsp","G:\UAV\myproject\Flight\r_config","G:\UAV\myproject\Flight\r_flash_rx","G:\UAV\myproject\Flight\r_flash_rx\src","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_1","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_2","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_3","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_4","G:\UAV\myproject\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -fpu -nologo  -define=__RX=1   "$<"
	@echo 'Finished scanning and building: $<'
	@echo.

