################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf *.sni *.dbg *.lst *.lis *.lpp *.map *.x *.lbp src/*.src src/*.p src/*.pp src/*.mrc *.bls *.libelf *.lbk *.abs *.fsy

# All of the sources participating in the build are defined here
-include sources.mk
-include r_bsp/board/rskrx23t/subdir.mk
-include r_bsp/mcu/all/subdir.mk
-include r_bsp/mcu/rx23t/subdir.mk
-include r_flash_rx/src/flash_type_1/subdir.mk
-include r_flash_rx/src/flash_type_2/subdir.mk
-include r_flash_rx/src/flash_type_3/subdir.mk
-include r_flash_rx/src/flash_type_4/subdir.mk
-include r_flash_rx/src/subdir.mk
-include src/subdir.mk
-include src/SrcUser/subdir.mk
-include src/cg_src/subdir.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_UPPER_DEPS)),)
-include $(CC_UPPER_DEPS)
endif
ifneq ($(strip $(C++_UPPER_DEPS)),)
-include $(C++_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_UPPER_DEPS)),)
-include $(CXX_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_UPPER_DEPS)),)
-include $(CPP_UPPER_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CP_DEPS)),)
-include $(CP_DEPS)
endif
ifneq ($(strip $(SRC_UPPER_DEPS)),)
-include $(SRC_UPPER_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(SRC_DEPS)),)
-include $(SRC_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(CP_UPPER_DEPS)),)
-include $(CP_UPPER_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(OBJ_DEPS)),)
-include $(OBJ_DEPS)
endif
ifneq ($(strip $(ABS_DEPS)),)
-include $(ABS_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(O_DEPS)),)
-include $(O_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

ifneq ($(strip $(SRC_UPPER_SRCS)),)
ALL_ASMS += $(SRC_UPPER_SRCS) 
endif
ifneq ($(strip $(SRC_SRCS)),)
ALL_ASMS += $(SRC_SRCS) 
endif
ifneq ($(strip $(S_UPPER_SRCS)),)
ALL_ASMS += $(S_UPPER_SRCS) 
endif
ifneq ($(strip $(S_SRCS)),)
ALL_ASMS += $(S_SRCS) 
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
CONVERTOR_OUTPUT_OUTPUTS += \
Flgiht.mot \

LINKER_OUTPUT_OUTPUTS += \
Flgiht.abs \

LIBRARY_GENERATOR_OUTPUTTYPE_OUTPUTS += \
Flgiht.lib \

# All Target
all: Flgiht.mot
	@echo 'Build complete.'

# Tool invocations
Flgiht.mot: $(LINKER_OUTPUT_OUTPUTS)
	@echo 'Invoking: Converter'
	rlink   "C:\Users\Antiver\Documents\Workspace\Flight\HardwareDebug\Flgiht.abs" -form=stype  -output="C:\Users\Antiver\Documents\Workspace\Flight\HardwareDebug\Flgiht.mot" -nomessage
	@echo 'Finished building target:'
	@echo.
	$(MAKE) --no-print-directory post-build

Flgiht.abs: $(OBJS) $(LIBRARY_GENERATOR_OUTPUTTYPE_OUTPUTS) $(ALL_ASMS) 
	@echo 'Invoking: Linker'
	@echo 'Building target:'
	rlink  -subcommand="LinkerSubCommand.tmp"  -library="C:\Users\Antiver\DOCUME~1\WORKSP~1\Flight\HARDWA~2\Flgiht.lib" -noprelink -list="Flgiht.map" -show -nooptimize -debug -output="C:\Users\Antiver\Documents\Workspace\Flight\HardwareDebug\Flgiht.abs" -rom=D=R,PFRAM=RPFRAM,D_1=R_1,D_2=R_2 -nocompress -vect=_undefined_interrupt_source_isr -nomessage -nologo
	@echo 'Finished building:'
	@echo.

Flgiht.lib:  
	@echo 'Invoking: Standard Library'
	lbgrx -lang=c99 -head=runtime,stdio,stdlib,string,new -isa=rxv2 -fpu -output="Flgiht.lib"
	@echo 'Finished building:'
	@echo.

# Other Targets
clean:
ifneq ($(strip $(OBJS)),)
	-$(file > clean_OBJS.tmp , $(OBJS))
	-@ xargs -s 6000 -t -a clean_OBJS.tmp $(RM)
	-@$(RM) clean_OBJS.tmp
endif
ifneq ($(strip $(C_DEPS)),)
	-$(file > clean_C_DEPS.tmp , $(C_DEPS))
	-@ xargs -s 6000 -t -a clean_C_DEPS.tmp $(RM)
	-@$(RM) clean_C_DEPS.tmp
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
	-$(file > clean_C_UPPER_DEPS.tmp , $(C_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_C_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_C_UPPER_DEPS.tmp
endif
ifneq ($(strip $(CPP_DEPS)),)
	-$(file > clean_CPP_DEPS.tmp , $(CPP_DEPS))
	-@ xargs -s 6000 -t -a clean_CPP_DEPS.tmp $(RM)
	-@$(RM) clean_CPP_DEPS.tmp
endif
ifneq ($(strip $(CPP_UPPER_DEPS)),)
	-$(file > clean_CPP_UPPER_DEPS.tmp , $(CPP_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_CPP_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_CPP_UPPER_DEPS.tmp
endif
ifneq ($(strip $(C++_DEPS)),)
	-$(file > clean_C++_DEPS.tmp , $(C++_DEPS))
	-@ xargs -s 6000 -t -a clean_C++_DEPS.tmp $(RM)
	-@$(RM) clean_C++_DEPS.tmp
endif
ifneq ($(strip $(C++_UPPER_DEPS)),)
	-$(file > clean_C++_UPPER_DEPS.tmp , $(C++_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_C++_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_C++_UPPER_DEPS.tmp
endif
ifneq ($(strip $(CP_DEPS)),)
	-$(file > clean_CP_DEPS.tmp , $(CP_DEPS))
	-@ xargs -s 6000 -t -a clean_CP_DEPS.tmp $(RM)
	-@$(RM) clean_CP_DEPS.tmp
endif
ifneq ($(strip $(CP_UPPER_DEPS)),)
	-$(file > clean_CP_UPPER_DEPS.tmp , $(CP_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_CP_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_CP_UPPER_DEPS.tmp
endif
ifneq ($(strip $(CXX_DEPS)),)
	-$(file > clean_CXX_DEPS.tmp , $(CXX_DEPS))
	-@ xargs -s 6000 -t -a clean_CXX_DEPS.tmp $(RM)
	-@$(RM) clean_CXX_DEPS.tmp
endif
ifneq ($(strip $(CXX_UPPER_DEPS)),)
	-$(file > clean_CXX_UPPER_DEPS.tmp , $(CXX_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_CXX_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_CXX_UPPER_DEPS.tmp
endif
ifneq ($(strip $(CC_DEPS)),)
	-$(file > clean_CC_DEPS.tmp , $(CC_DEPS))
	-@ xargs -s 6000 -t -a clean_CC_DEPS.tmp $(RM)
	-@$(RM) clean_CC_DEPS.tmp
endif
ifneq ($(strip $(CC_UPPER_DEPS)),)
	-$(file > clean_CC_UPPER_DEPS.tmp , $(CC_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_CC_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_CC_UPPER_DEPS.tmp
endif
ifneq ($(strip $(ASM_PRNS)),)
	-$(file > clean_ASM_PRNS.tmp , $(ASM_PRNS))
	-@ xargs -s 6000 -t -a clean_ASM_PRNS.tmp $(RM)
	-@$(RM) clean_ASM_PRNS.tmp
endif
ifneq ($(strip $(OBJ_DEPS)),)
	-$(file > clean_OBJ_DEPS.tmp , $(OBJ_DEPS))
	-@ xargs -s 6000 -t -a clean_OBJ_DEPS.tmp $(RM)
	-@$(RM) clean_OBJ_DEPS.tmp
endif
ifneq ($(strip $(ABS_DEPS)),)
	-$(file > clean_ABS_DEPS.tmp , $(ABS_DEPS))
	-@ xargs -s 6000 -t -a clean_ABS_DEPS.tmp $(RM)
	-@$(RM) clean_ABS_DEPS.tmp
endif
ifneq ($(strip $(O_DEPS)),)
	-$(file > clean_O_DEPS.tmp , $(O_DEPS))
	-@ xargs -s 6000 -t -a clean_O_DEPS.tmp $(RM)
	-@$(RM) clean_O_DEPS.tmp
endif
ifneq ($(strip $(S_DEPS)),)
	-$(file > clean_S_DEPS.tmp , $(S_DEPS))
	-@ xargs -s 6000 -t -a clean_S_DEPS.tmp $(RM)
	-@$(RM) clean_S_DEPS.tmp
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
	-$(file > clean_S_UPPER_DEPS.tmp , $(S_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_S_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_S_UPPER_DEPS.tmp
endif
ifneq ($(strip $(SRC_DEPS)),)
	-$(file > clean_SRC_DEPS.tmp , $(SRC_DEPS))
	-@ xargs -s 6000 -t -a clean_SRC_DEPS.tmp $(RM)
	-@$(RM) clean_SRC_DEPS.tmp
endif
ifneq ($(strip $(SRC_UPPER_DEPS)),)
	-$(file > clean_SRC_UPPER_DEPS.tmp , $(SRC_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_SRC_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_SRC_UPPER_DEPS.tmp
endif
ifneq ($(strip $(ASM_DEPS)),)
	-$(file > clean_ASM_DEPS.tmp , $(ASM_DEPS))
	-@ xargs -s 6000 -t -a clean_ASM_DEPS.tmp $(RM)
	-@$(RM) clean_ASM_DEPS.tmp
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
	-$(file > clean_ASM_UPPER_DEPS.tmp , $(ASM_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_ASM_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_ASM_UPPER_DEPS.tmp
endif
	-$(RM) $(LIBRARY_GENERATOR_OUTPUTTYPE_OUTPUTS) $(CONVERTOR_OUTPUT_OUTPUTS) 
	-rm -rf "r_bsp/board/rskrx23t"/*.obj "r_bsp/board/rskrx23t"/*.d "r_bsp/board/rskrx23t"/*.src "r_bsp/board/rskrx23t"/*.p "r_bsp/board/rskrx23t"/*.pp "r_bsp/board/rskrx23t"/*.mrc "r_bsp/board/rskrx23t"/*.prn "r_bsp/board/rskrx23t"/*.asm "r_bsp/board/rskrx23t"/*.lst
	-rm -rf "r_bsp/mcu/all"/*.obj "r_bsp/mcu/all"/*.d "r_bsp/mcu/all"/*.src "r_bsp/mcu/all"/*.p "r_bsp/mcu/all"/*.pp "r_bsp/mcu/all"/*.mrc "r_bsp/mcu/all"/*.prn "r_bsp/mcu/all"/*.asm "r_bsp/mcu/all"/*.lst
	-rm -rf "r_bsp/mcu/rx23t"/*.obj "r_bsp/mcu/rx23t"/*.d "r_bsp/mcu/rx23t"/*.src "r_bsp/mcu/rx23t"/*.p "r_bsp/mcu/rx23t"/*.pp "r_bsp/mcu/rx23t"/*.mrc "r_bsp/mcu/rx23t"/*.prn "r_bsp/mcu/rx23t"/*.asm "r_bsp/mcu/rx23t"/*.lst
	-rm -rf "r_flash_rx/src/flash_type_1"/*.obj "r_flash_rx/src/flash_type_1"/*.d "r_flash_rx/src/flash_type_1"/*.src "r_flash_rx/src/flash_type_1"/*.p "r_flash_rx/src/flash_type_1"/*.pp "r_flash_rx/src/flash_type_1"/*.mrc "r_flash_rx/src/flash_type_1"/*.prn "r_flash_rx/src/flash_type_1"/*.asm "r_flash_rx/src/flash_type_1"/*.lst
	-rm -rf "r_flash_rx/src/flash_type_2"/*.obj "r_flash_rx/src/flash_type_2"/*.d "r_flash_rx/src/flash_type_2"/*.src "r_flash_rx/src/flash_type_2"/*.p "r_flash_rx/src/flash_type_2"/*.pp "r_flash_rx/src/flash_type_2"/*.mrc "r_flash_rx/src/flash_type_2"/*.prn "r_flash_rx/src/flash_type_2"/*.asm "r_flash_rx/src/flash_type_2"/*.lst
	-rm -rf "r_flash_rx/src/flash_type_3"/*.obj "r_flash_rx/src/flash_type_3"/*.d "r_flash_rx/src/flash_type_3"/*.src "r_flash_rx/src/flash_type_3"/*.p "r_flash_rx/src/flash_type_3"/*.pp "r_flash_rx/src/flash_type_3"/*.mrc "r_flash_rx/src/flash_type_3"/*.prn "r_flash_rx/src/flash_type_3"/*.asm "r_flash_rx/src/flash_type_3"/*.lst
	-rm -rf "r_flash_rx/src/flash_type_4"/*.obj "r_flash_rx/src/flash_type_4"/*.d "r_flash_rx/src/flash_type_4"/*.src "r_flash_rx/src/flash_type_4"/*.p "r_flash_rx/src/flash_type_4"/*.pp "r_flash_rx/src/flash_type_4"/*.mrc "r_flash_rx/src/flash_type_4"/*.prn "r_flash_rx/src/flash_type_4"/*.asm "r_flash_rx/src/flash_type_4"/*.lst
	-rm -rf "r_flash_rx/src"/*.obj "r_flash_rx/src"/*.d "r_flash_rx/src"/*.src "r_flash_rx/src"/*.p "r_flash_rx/src"/*.pp "r_flash_rx/src"/*.mrc "r_flash_rx/src"/*.prn "r_flash_rx/src"/*.asm "r_flash_rx/src"/*.lst
	-rm -rf "src"/*.obj "src"/*.d "src"/*.src "src"/*.p "src"/*.pp "src"/*.mrc "src"/*.prn "src"/*.asm "src"/*.lst
	-rm -rf "src/SrcUser"/*.obj "src/SrcUser"/*.d "src/SrcUser"/*.src "src/SrcUser"/*.p "src/SrcUser"/*.pp "src/SrcUser"/*.mrc "src/SrcUser"/*.prn "src/SrcUser"/*.asm "src/SrcUser"/*.lst
	-rm -rf "src/cg_src"/*.obj "src/cg_src"/*.d "src/cg_src"/*.src "src/cg_src"/*.p "src/cg_src"/*.pp "src/cg_src"/*.mrc "src/cg_src"/*.prn "src/cg_src"/*.asm "src/cg_src"/*.lst
	@echo.
	@echo 'Clean complete.'

post-build:
	@echo.
	-C:\Renesas\E2_STU~1\DEBUGC~1\RENESA~1.EXE Flgiht.abs Flgiht.x 
	@echo.

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
