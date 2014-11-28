#include "testCase.h"
#include "fileSystem.h"

int main ()
{

	fprintf(stdout, "Size of dirBlock: %d\n", sizeof(dirBlock_t));
	fprintf(stdout, "Size of SuperBlock: %d\n", sizeof(superBlock_t));
	fprintf(stdout, "Size of fileBlock: %d\n", sizeof(fileBlock_t));
	createDisk();
	fprintf(stdout, "%s", "Created disk");
}
