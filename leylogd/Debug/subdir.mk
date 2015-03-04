################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TMP102.cpp \
../become_daemon.cpp \
../main.cpp 

OBJS += \
./TMP102.o \
./become_daemon.o \
./main.o 

CPP_DEPS += \
./TMP102.d \
./become_daemon.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++-4.7 -I/usr/arm-linux-gnueabihf/include/c++/4.7.3 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


