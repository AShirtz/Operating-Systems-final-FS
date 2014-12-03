#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	fileSystem_t *FSA = getFS();
	fileSystem_t *FSB = getFS();

	assert(FSA == FSB, "getFS idempotency test.", result);

	assert(*(FSA->activeDir->name) == '~', "FS initialization test", result);
	assert(FSA->superBlock->numBlocks == 100, "FS init test, super block", result);

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
