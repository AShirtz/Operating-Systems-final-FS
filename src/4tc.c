#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	dirBlock_t dB;
	memset(&dB, 0, sizeof(dirBlock_t));
	dB.entryCount = 3;

	memcpy(&(dB.name), "~", 1);

	char *entryExample = "testFile,2,3;testDir,1,4;fraggleRock,1,5;";
	memcpy(dB.entries, entryExample, strlen(entryExample));

	activeDir_t *aD;
	aD = activateDirBlock(&dB);

	fprintf(stdout, "Result: %s\n", aD->name);

	assert(aD->entries[0].startingBlockNum == 3, "Entry Listing conversion test.\n", result);
	assert(aD->entries[1].startingBlockNum == 4, "Entry Listing conversion test, Cont'd.\n", result);

	assert(aD->entries[0].blockType == 2, "Entry block type test.\n", result); 
	assert(aD->entries[1].blockType == 1, "Entry block type test.\n", result); 

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
