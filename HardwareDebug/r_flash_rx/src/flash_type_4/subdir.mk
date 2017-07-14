################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
..\r_flash_rx/src/flash_type_4/r_flash_type4.c 

C_DEPS += \
./r_flash_rx/src/flash_type_4/r_flash_type4.d 

OBJS += \
./r_flash_rx/src/flash_type_4/r_flash_type4.obj 


# Each subdirectory must supply rules for building sources it contributes
r_flash_rx/src/flash_type_4/%.obj: ../r_flash_rx/src/flash_type_4/%.c
	@echo 'Scanning and building file: $<'
	@echo 'Invoking: Scanner and Compiler'
	ccrx  -MM -MP -output=dep="$(@:%.obj=%.d)" -MT="$(@:%.obj=%.obj)" -MT="$(@:%.obj=%.d)" -lang=c99   -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","D:\workspace\Flight\src\cg_src","D:\workspace\Flight\r_flash_rx\src\targets\rx23t","D:\workspace\Flight\src\SrcUser","D:\workspace\Flight\r_bsp","D:\workspace\Flight\r_config","D:\workspace\Flight\r_flash_rx","D:\workspace\Flight\r_flash_rx\src","D:\workspace\Flight\r_flash_rx\src\flash_type_1","D:\workspace\Flight\r_flash_rx\src\flash_type_2","D:\workspace\Flight\r_flash_rx\src\flash_type_3","D:\workspace\Flight\r_flash_rx\src\flash_type_4","D:\workspace\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -fpu -nologo  -define=__RX=1   "$<"
	ccrx -lang=c99 -output=obj="$(@:%.d=%.obj)"  -include="C:\PROGRA~2\Renesas\RX\2_6_0/include","D:\workspace\Flight\src\cg_src","D:\workspace\Flight\r_flash_rx\src\targets\rx23t","D:\workspace\Flight\src\SrcUser","D:\workspace\Flight\r_bsp","D:\workspace\Flight\r_config","D:\workspace\Flight\r_flash_rx","D:\workspace\Flight\r_flash_rx\src","D:\workspace\Flight\r_flash_rx\src\flash_type_1","D:\workspace\Flight\r_flash_rx\src\flash_type_2","D:\workspace\Flight\r_flash_rx\src\flash_type_3","D:\workspace\Flight\r_flash_rx\src\flash_type_4","D:\workspace\Flight\r_flash_rx\src\targets"  -debug -nomessage -isa=rxv2 -fpu -nologo  -define=__RX=1   "$<"
	@echo 'Finished scanning and building: $<'
	@echo.

