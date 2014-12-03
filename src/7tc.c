#include "testCase.h"
#include "fileSystem.h"

int main ()
{
	testResult_t *result = initializeTestResult();

	

	printTestResult(result);
	freeTestResult(result);
	return 0;
}
