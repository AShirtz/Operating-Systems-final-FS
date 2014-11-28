CC 	= gcc
CFLAG 	= -g

FILE_SYSTEM_OBJS = \
	fileSystem.o 

all: fileSystem testCase

clean:
	rm artifacts/* target/* vdisk

testCase:
	$(CC) $(CFLAGS) tests/testCase1.c -o target/compileTest 

fileSystem: artifacts/$(FILE_SYSTEM_OBJS)
	$(CC) $(CFLAGS) artifacts/$(FILE_SYSTEM_OBJS) -o target/output

artifacts/%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@
