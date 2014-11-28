#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ERROR_LOG_LEN 1000

typedef struct 
{
	int totalTests;
	int failedTests;

	char errorLog[ERROR_LOG_LEN];
	int curErrorLogIndex;
} testResult_t;

testResult_t *initializeTestResult ()
{
	testResult_t *result;
	result = (testResult_t *) malloc(sizeof(testResult_t));
	return result;
}

void freeTestResult (testResult_t *result)
{
	free(result);
}

void assert (bool exprResult, char *msg, testResult_t *result)
{
	result->totalTests++;
	if (!exprResult)
	{
		result->failedTests++;
		snprintf((result->errorLog + result->curErrorLogIndex), strlen(msg) + 1, "%s\n", msg);
		result->curErrorLogIndex += strlen(msg) + 1;
	}
}

void deny (bool exprResult, char *msg, testResult_t *result)
{
	assert(!exprResult, msg, result);
}

void printTestResult (testResult_t *result)
{
	if (result->failedTests)
	{
		fprintf(stdout, " ~~~~~ TEST FAILED ~~~~~~~~ \n");
	} else {
		fprintf(stdout, " ~~~~~ TEST SUCCEEDED ~~~~~ \n");
	}
	
	fprintf(stdout, "  Total Tests: %d \n  Tests Succeeded: %d \n  Tests Failed: %d\n", result->totalTests, result->totalTests - result->failedTests, result->failedTests);
	
	if (result->failedTests)
	{
		fprintf(stdout, "Error Log: \n %s\n", result->errorLog);
	}
}
