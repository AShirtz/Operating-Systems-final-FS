#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

//	diskController_t *dC = initializeDiskController();
	getFS();
	superBlock_t sBCopy;
	fprintf(stdout, "Pre read\n");
	readBlockFromDisk(0, &sBCopy);
	fprintf(stdout, "Post read\n");
	
	assert(sBCopy.numBlocks == 100, "NumBlocks test.\n", result);
	assert(sBCopy.rootDirBlockNum == 1, "RootDirIndex test.\n", result);

	dirBlock_t rDCopy;
	fprintf(stdout, "Pre read\n");
	readBlockFromDisk(1, &rDCopy);
	fprintf(stdout, "Post read\n");

	assert(rDCopy.entryCount == 0, "RootDir file count test.\n", result);
	assert(rDCopy.continuationDirBlockNum == -1, "Continuation block num test.\n", result);	

//	freeDiskController(dC);
	printTestResult(result);
	freeTestResult(result);
	return 0;
}
