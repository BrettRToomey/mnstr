CC=clang
TARGET=mnstr

LFLAGS=-lhpdf

all: debug

debug: $(TARGET)
release: $(TARGET)

$(TARGET):
	$(CC) src/main.c $(LFLAGS) -o $(TARGET)

monsterGen:
	$(CC) util/monsters.c -o monsterGen

clean:
	- rm -f $(TARGET) monsterGen

.PHONY: all clean debug release 
