#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();
	fprintf(stdout, "Size of dirBlock_t: %d\n", sizeof(dirBlock_t));
	fprintf(stdout, "Size of superblock_t: %d\n", sizeof(superBlock_t));
	fprintf(stdout, "Size of fileBlock_t: %d\n", sizeof(fileBlock_t));
	assert(sizeof(dirBlock_t) == 1024, "dirBlock_t size test.", result);
	assert(sizeof(fileBlock_t) == 1024, "fileBlock_t size test.", result);
	assert(sizeof(superBlock_t) == 1024, "superBlock_t size test.", result);
	assert(createDisk(), "Creating vdisk file.", result);
	printTestResult(result);
	freeTestResult(result);
	return 0;
}
