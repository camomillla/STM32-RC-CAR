################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/NimaLTD_Driver/ATC/atc.c 

OBJS += \
./Middlewares/Third_Party/NimaLTD_Driver/ATC/atc.o 

C_DEPS += \
./Middlewares/Third_Party/NimaLTD_Driver/ATC/atc.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/NimaLTD_Driver/ATC/%.o Middlewares/Third_Party/NimaLTD_Driver/ATC/%.su Middlewares/Third_Party/NimaLTD_Driver/ATC/%.cyclo: ../Middlewares/Third_Party/NimaLTD_Driver/ATC/%.c Middlewares/Third_Party/NimaLTD_Driver/ATC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../I-CUBE-ATC -I../Middlewares/Third_Party/NimaLTD_Driver/ATC -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-NimaLTD_Driver-2f-ATC

clean-Middlewares-2f-Third_Party-2f-NimaLTD_Driver-2f-ATC:
	-$(RM) ./Middlewares/Third_Party/NimaLTD_Driver/ATC/atc.cyclo ./Middlewares/Third_Party/NimaLTD_Driver/ATC/atc.d ./Middlewares/Third_Party/NimaLTD_Driver/ATC/atc.o ./Middlewares/Third_Party/NimaLTD_Driver/ATC/atc.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-NimaLTD_Driver-2f-ATC

