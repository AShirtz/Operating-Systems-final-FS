#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ERROR_LOG_LEN 1000

typedef struct 
{
	int totalTests;
	int failedTests;

	char *errorLog;
	int curErrorLogIndex;
} testResult_t;

void initializeTestResult (testResult_t *result)
{
	memset(result, 0, sizeof(testResult_t));
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
		fprintf(stdout, " ~~~~~ TEST FAILED ~~~~~ \n");
	}
	
	fprintf(stdout, "Total Tests: %d \nTests Succeeded: %d \n Tests Failed: %d\n", result->totalTests, result->totalTests - result->failedTests, result->failedTests);
	
	if (result->failedTests)
	{
		fprintf(stdout, "Error Log: \n %s", result->errorLog);
	}
}
