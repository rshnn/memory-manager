# gcc (Ubuntu 4.8.5-4ubuntu2) 4.8.5 (rshnn)
CC = gcc

# -m32 	32-bit mode 
# -O0	No optimization option  
# -g 	Enable gdb debugging 
CFLAGS 		= -m32 -O0 -Wall
DEBUGGER 	= -g

SOURCE 		= memory-manager.c memory-manager.h
TARGET 		= memory-manager


# Toggle gdb debugger 
ifeq ($(DEBUG), TRUE)
CFLAGS += $(DEBUGGER)
endif

make: 
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

debug: 
	make DEBUG=TRUE

clean: 
	rm -f $(TARGET) swagmaster.swp

rebuild:
	rm -f $(TARGET)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)
