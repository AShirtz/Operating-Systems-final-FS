#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	fileSystem_t *FS = getFS();

	int fd = openFile("TestFile");

	activeFile_t *aF = ((fileSystem_t *) getFS())->activeFiles[fd];

	char *text = "Here is some text. ";
	
	writeInFile(fd, text, strlen(text));
	seekInFile(fd, 994);
	writeInFile(fd, text, strlen(text));

	char target[100];
	memset(target, 0, sizeof(target));
	seekInFile(fd,0);
	readInFile(fd, target, strlen(text));

	deny(strcmp(text, target), "file write file read test.", result);

	memset(target, 0, sizeof(target));

	seekInFile(fd,994);
	readInFile(fd, target, strlen(text));
	
	deny(strcmp(text, target), "file write file read test.", result);

	blockLink_t *bL = generateBlockLink(2);

	giveDirListing();

	fprintf(stdout, "%d, %d\n", aF->fileHeadLink->blockNum, aF->fileHeadLink->nextLink);
	fprintf(stdout, "%d, %d\n", bL->blockNum, bL->nextLink);

	deleteFile("TestFile");

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
