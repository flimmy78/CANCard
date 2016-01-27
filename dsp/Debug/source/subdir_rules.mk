################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
source/DSP281x_GlobalVariableDefs.obj: ../source/DSP281x_GlobalVariableDefs.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.0/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv5/tools/compiler/c2000_6.1.0/include" --include_path="D:/ccs5_prj/Debug" --include_path="D:/ccs5_prj/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/bios/include" --include_path="C:/ti/bios_5_41_13_42/packages/ti/rtdx/include/c2000" --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="source/DSP281x_GlobalVariableDefs.pp" --obj_directory="source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


