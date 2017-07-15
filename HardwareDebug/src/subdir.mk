################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
..\src/Flgiht.c 

C_DEPS += \
./src/Flgiht.d 

OBJS += \
./src/Flgiht.obj 


# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.c
	@echo 'Scanning and building file: $<'
	@echo 'Invoking: Scanner and Compiler'
	ccrx  -MM -MP -output=dep="$(@:%.obj=%.d)" -MT="$(@:%.obj=%.obj)" -MT="$(@:%.obj=%.d)" -lang=c99   -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","G:\UAV\myproject\Flight\r_bsp\mcu\rx23t\register_access","G:\UAV\myproject\Flight\src\cg_src","G:\UAV\myproject\Flight\r_flash_rx\src\targets\rx23t","G:\UAV\myproject\Flight\src\SrcUser","G:\UAV\myproject\Flight\r_bsp","G:\UAV\myproject\Flight\r_config","G:\UAV\myproject\Flight\r_flash_rx","G:\UAV\myproject\Flight\r_flash_rx\src","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_1","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_2","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_3","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_4","G:\UAV\myproject\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -optimize=0 -fpu -nologo  -define=__RX=1   "$<"
	ccrx -lang=c99 -output=obj="$(@:%.d=%.obj)"  -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","G:\UAV\myproject\Flight\r_bsp\mcu\rx23t\register_access","G:\UAV\myproject\Flight\src\cg_src","G:\UAV\myproject\Flight\r_flash_rx\src\targets\rx23t","G:\UAV\myproject\Flight\src\SrcUser","G:\UAV\myproject\Flight\r_bsp","G:\UAV\myproject\Flight\r_config","G:\UAV\myproject\Flight\r_flash_rx","G:\UAV\myproject\Flight\r_flash_rx\src","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_1","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_2","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_3","G:\UAV\myproject\Flight\r_flash_rx\src\flash_type_4","G:\UAV\myproject\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -optimize=0 -fpu -nologo  -define=__RX=1   "$<"
	@echo 'Finished scanning and building: $<'
	@echo.

