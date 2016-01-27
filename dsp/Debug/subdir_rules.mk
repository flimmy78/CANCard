################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
CANmd.obj: ../CANmd.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="CANmd.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

CanCardcfg.cmd: ../CanCard.tcf
	@echo 'Building file: $<'
	@echo 'Invoking: TConf'
	"C:/ti/xdctools_3_23_03_53/tconf" -b -Dconfig.importPath="C:/ti/bios_5_41_13_42/packages;" "$<"
	@echo 'Finished building: $<'
	@echo ' '

CanCardcfg.s??: CanCardcfg.cmd
CanCardcfg_c.c: CanCardcfg.cmd
CanCardcfg.h: CanCardcfg.cmd
CanCardcfg.h??: CanCardcfg.cmd
CanCard.cdb: CanCardcfg.cmd

CanCardcfg.obj: ./CanCardcfg.s?? $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="CanCardcfg.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

CanCardcfg_c.obj: ./CanCardcfg_c.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="CanCardcfg_c.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

ISAmd.obj: ../ISAmd.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="ISAmd.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Uartmd.obj: ../Uartmd.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="Uartmd.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

circ.obj: ../circ.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="circ.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


