#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	fileSystem_t *FS = getFS();

	FS->superBlock->freeBlockBitmap[0] = 0b11111111;
	FS->superBlock->freeBlockBitmap[1] = 0b11111111;
	FS->superBlock->freeBlockBitmap[2] = 0b00001101;
	
	assert(findEmptyBlock() == 17, "empty block find test.", result);

	assert(FS->activeDir->entryCount == 0, "add entry test begin", result);

	activeFile_t * aF = openFile(FS->activeDir, "TestFile");

	assert(FS->activeDir->entryCount == 1, "add entry test begin", result);

	directoryEntry_t dE = FS->activeDir->entries[0];
	fprintf(stdout, "%s, %d, %d\n", dE.name, dE.blockType, dE.startingBlockNum);
	fprintf(stdout, "%s, %d\n", aF->fileHeadBlock->name, aF->curContentsPtr);

	fileBlock_t *fB;
	fB = malloc(sizeof(fileBlock_t));
	readBlockFromDisk(17, fB);

	fprintf(stdout, "%s\n", fB->name);

	assert(findEmptyBlock() == 20, "bitmap affect test.", result);

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
