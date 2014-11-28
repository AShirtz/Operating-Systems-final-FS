CC 	= gcc
CFLAG 	= -g

FILE_SYSTEM_OBJS = fileSystem.o
TEST_CASE_OBJS = 2tc.o

all: testCases

clean:
	rm artifacts/* target/* vdisk

testCases: artifacts/$(TEST_CASE_OBJS) artifacts/$(FILE_SYSTEM_OBJS)
	$(CC) $(CFLAGS) artifacts/2tc.o artifacts/$(FILE_SYSTEM_OBJS) -o target/2tc

fileSystem: artifacts/$(FILE_SYSTEM_OBJS)
	$(CC) $(CFLAGS) artifacts/fileSystem.o -o target/output

artifacts/%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

artifacts/%.o: tests/%.c
	$(CC) -c $(CFLAGS) $< -o $@
