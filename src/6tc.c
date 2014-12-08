#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	fileSystem_t *FS = getFS();

	FS->superBlock->blockBitmap[0] = 0b11111111;
	FS->superBlock->blockBitmap[1] = 0b11111111;
	FS->superBlock->blockBitmap[2] = 0b00001101;
	
	assert(findEmptyBlock() == 17, "empty block find test.", result);

	assert(FS->activeDir->entryCount == 0, "add entry test begin", result);

	activeFile_t * aF = openFileInternal(FS->activeDir, "TestFile");

	assert(FS->activeDir->entryCount == 1, "add entry test begin", result);

	directoryEntry_t dE = FS->activeDir->entries[0];
	fprintf(stdout, "%s, %d, %d\n", dE.name, dE.blockType, dE.startingBlockNum);
	fprintf(stdout, "%s, %d\n", aF->name, aF->curContentsPtr);

	fileBlock_t *fB;
	fB = malloc(sizeof(fileBlock_t));
	readBlockFromDisk(17, fB);
	fprintf(stdout, "%d\n", fB->blockType);

	assert(fB->blockType == 3, "read blockType", result);

	fprintf(stdout, "%s\n", fB->name);

	assert(findEmptyBlock() == 20, "bitmap affect test.", result);

	markBlockFree(17);
	assert(findEmptyBlock() == 17, "bitmap affect test.", result);
	memset(fB, 0, sizeof(fileBlock_t));
	readBlockFromDisk(17, fB);
	fprintf(stdout, "%d\n", fB->blockType);

	assert(fB->blockType == 0, "free block zero set test.", result);

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
