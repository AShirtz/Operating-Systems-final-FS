#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	getFS();	

	deleteFile("File to Delete");
	deleteDir("Dir to Delete");

	int newFd = openFile("NewFile");

	char *text = "Here is some text.";
	int writeLen = writeInFile(newFd, text, strlen(text));
	assert(writeLen == strlen(text), "File write return value test.", result);
	seekInFile(newFd, 0);
	char target[100];
	memset(target, 0, sizeof(target));
	int readLen = readInFile(newFd, target, strlen(text));
	assert(readLen == strlen(text), "File write return value test.", result);
	deny(strcmp(target, text), "Read + write test.", result);

	int fd = openFile("File to Delete");

	seekInFile(newFd, FILE_BLOCK_DATA_SIZE - 2);
	writeLen = writeInFile(newFd, text, strlen(text));
	seekInFile(newFd, FILE_BLOCK_DATA_SIZE - 2);
	readLen = readInFile(newFd, target, strlen(text));
	assert(readLen == strlen(text), "Non-contiguous allocation read+write test.", result);
	deny(strcmp(target, text), "Read + write test.", result);

	mkDir("Dir to Delete");

	giveDirListing();

	deleteFile("File to Delete");
	mkDir("NewDir");
	giveDirListing();

	deleteDir("Dir to Delete");

	giveDirListing();

	seekInFile(newFd, 0);

	int i;
	for (i = 0; i < 150; i++)
	{
		writeInFile(newFd, text, strlen(text));
		seekInFile(newFd, (i * FILE_BLOCK_DATA_SIZE) - 2);
	}

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
