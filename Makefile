.SUFFIXES : .c .o

CC = gcc
CXX = g++

INC = \include
OUT = \out
LIBS = -lwiringPi -lwiringPiDev -lpthread -lncurses
CFLAGS = -w -I$(INC)
CPPFLAGS = -w -O2 -Wall -std=c++11 -I$(INC)

SRCS_SSD = ssd1306/ssd1306.cpp
SRCS_I2C = I2C.c
SRCS = 	main.cpp \
 		joypad_task.cpp \
 		motor_clk_task.cpp \
 		control.cpp

OBJS = $(SRCS:.cpp=.o)
OBJS += $(SRCS_SSD:.cpp=.o)
OBJS += $(SRCS_I2C:.c=.o)

TARGET = step_motor

all : $(TARGET)
$(TARGET):$(OBJS)
	@echo "***********************************************"
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)
	@echo "***********************************************"
	@echo "$(TARGET) Created Successfully."
dep :
	gccmakedep $(INC) $(SRCS)

clean:
	rm -f $(OBJS) $(TARGET)
	echo "clean!!"
