#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	dirBlock_t *dB;
	dB = malloc(sizeof(dirBlock_t));
	dB->entryCount = 3;

	memcpy(&(dB->name), "~", 1);

	char *entryExample = "testFile,3,3;testDir,2,4;fraggleRock,3,5;";
	memcpy(dB->entries, entryExample, strlen(entryExample));

	activeDir_t *aD;
	aD = activateDirBlock(dB);

	assert(aD->entryCount == 3, "Entry count test.\n", result);
	assert(aD->entries[0].startingBlockNum == 3, "Entry Listing conversion test.\n", result);
	assert(aD->entries[1].startingBlockNum == 4, "Entry Listing conversion test, Cont'd.\n", result);

	assert(aD->entries[0].blockType == 3, "Entry block type test.\n", result); 
	assert(aD->entries[1].blockType == 2, "Entry block type test.\n", result); 

	free(dB);

	dB = malloc(sizeof(dirBlock_t));

	dB = convertActiveDirToBlock(aD);

	assert(dB->entryCount == 3, "Entry count test.\n", result);
	assert(dB->blockType == 2, "blockType test\n", result);
	
	fprintf(stdout, "Result: %s, %s\n", dB->name, dB->entries);

	freeActiveDirectory(aD);
	freeDirBlock(dB);

	

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
