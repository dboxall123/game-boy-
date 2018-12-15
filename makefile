#OBJS specifies which files to compile as part of the project 
OBJS = main.c init.c mmu.c cpu.c gpu.c
#CC specifies which compiler we're using 
CC = gcc 
#INCLUDE_PATHS specifies the additional include paths we'll need 
INCLUDE_PATHS = -IC:\minGW\include\SDL2 
#LIBRARY_PATHS specifies the additional library paths we'll need 
LIBRARY_PATHS = -LC:\minGW\lib 
#COMPILER_FLAGS specifies the additional compilation options we're using 
# -w suppresses all warnings 
# -Wl,-subsystem,windows gets rid of the console window 
COMPILER_FLAGS = #-Wl,-subsystem,windows , -O, -g
#LINKER_FLAGS specifies the libraries we're linking against 
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
#OBJ_NAME specifies the name of our exectuable 
OBJ_NAME = emu
#This is the target that compiles our executable 
all : $(OBJS) 
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)