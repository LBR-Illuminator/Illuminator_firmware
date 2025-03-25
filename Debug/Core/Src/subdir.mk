################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app_comms_handler.c \
../Core/Src/app_sys_coordinator.c \
../Core/Src/freertos.c \
../Core/Src/main.c 

OBJS += \
./Core/Src/app_comms_handler.o \
./Core/Src/app_sys_coordinator.o \
./Core/Src/freertos.o \
./Core/Src/main.o 

C_DEPS += \
./Core/Src/app_comms_handler.d \
./Core/Src/app_sys_coordinator.d \
./Core/Src/freertos.d \
./Core/Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L432xx -c -I../Core/Inc -I../Drivers/HAL/Inc -I../Drivers/VAL/Inc -I../I-CUBE-EE -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/NimaLTD_Driver/EE -I../Middlewares/Third_Party/lwjson/src/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app_comms_handler.cyclo ./Core/Src/app_comms_handler.d ./Core/Src/app_comms_handler.o ./Core/Src/app_comms_handler.su ./Core/Src/app_sys_coordinator.cyclo ./Core/Src/app_sys_coordinator.d ./Core/Src/app_sys_coordinator.o ./Core/Src/app_sys_coordinator.su ./Core/Src/freertos.cyclo ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su

.PHONY: clean-Core-2f-Src

