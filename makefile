CC 	= gcc
CFLAG 	= -g

FILE_SYSTEM_OBJS = fileSystem.o
TEST_CASE_OBJS = 4tc.o

all: testCases

clean:
	rm artifacts/* target/* vdisk

testCases: artifacts/$(TEST_CASE_OBJS) artifacts/$(FILE_SYSTEM_OBJS)
	$(CC) $(CFLAGS) artifacts/4tc.o artifacts/$(FILE_SYSTEM_OBJS) -o target/4tc

fileSystem: artifacts/$(FILE_SYSTEM_OBJS)
	$(CC) $(CFLAGS) artifacts/fileSystem.o -o target/output

artifacts/%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

artifacts/%.o: tests/%.c
	$(CC) -c $(CFLAGS) $< -o $@
