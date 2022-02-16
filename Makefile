CC=g++
FILES=*.c *.cpp
OPTS=
TARGET=opengl-snake
# INCLUDES= -I~/src/libraries/include/
LIBS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl

run:
	./$(TARGET)

all:
	$(CC) $(FILES) $(OPTS) -o $(TARGET) $(LIBS)
	
