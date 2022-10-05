################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/dh0.cpp \
../src/hhs_cam.cpp 

CPP_DEPS += \
./src/dh0.d \
./src/hhs_cam.d 

OBJS += \
./src/dh0.o \
./src/hhs_cam.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/opencv2 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/dh0.d ./src/dh0.o ./src/hhs_cam.d ./src/hhs_cam.o

.PHONY: clean-src

