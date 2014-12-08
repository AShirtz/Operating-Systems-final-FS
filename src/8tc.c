#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	fileSystem_t *FS = getFS();
	mkDir("NewDir");
	mkDir("FooDir");

	giveDirListing();

	deleteDir("NewDir");

	giveDirListing();

	openFile("TestFile");
	giveDirListing();

	deleteDir("NewDir");
	giveDirListing();

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
