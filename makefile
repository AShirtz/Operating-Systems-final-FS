CC 	= clang
CFLAG 	= -g

FILE_SYSTEM_OBJS = fileSystem.o
TEST_CASE_OBJS = \
	1tc.o \
	2tc.o \
	3tc.o \
	4tc.o \
	5tc.o \
	6tc.o \
	7tc.o \
	8tc.o \
	realtc.o

all: testCases

clean:
	rm *.o target/* vdisk

testCases: $(TEST_CASE_OBJS) $(FILE_SYSTEM_OBJS)
	$(CC) $(CFLAGS) 1tc.o $(FILE_SYSTEM_OBJS) -o target/1tc
	$(CC) $(CFLAGS) 2tc.o $(FILE_SYSTEM_OBJS) -o target/2tc
	$(CC) $(CFLAGS) 3tc.o $(FILE_SYSTEM_OBJS) -o target/3tc
	$(CC) $(CFLAGS) 4tc.o $(FILE_SYSTEM_OBJS) -o target/4tc
	$(CC) $(CFLAGS) 5tc.o $(FILE_SYSTEM_OBJS) -o target/5tc
	$(CC) $(CFLAGS) 6tc.o $(FILE_SYSTEM_OBJS) -o target/6tc
	$(CC) $(CFLAGS) 7tc.o $(FILE_SYSTEM_OBJS) -o target/7tc
	$(CC) $(CFLAGS) 8tc.o $(FILE_SYSTEM_OBJS) -o target/8tc
	$(CC) $(CFLAGS) realtc.o $(FILE_SYSTEM_OBJS) -o target/realtc

%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@
