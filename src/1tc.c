#include "testCase.h"

/*
	This test case is to see if the test framework is working,
	it does not deal with the file system at all, just normal
	C calls.
*/

int main ()
{
	testResult_t *result = initializeTestResult();
	int i;
	for (i = 0; i < 10; i++) { }
	assert( i == 10, "For loop.", result);
	deny( i == 12, "For loop, should error.", result);
	
	printTestResult(result);

	freeTestResult(result);
	return 0;
}
