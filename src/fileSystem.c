#include "fileSystem.h"

int main ()
{

	fprintf(stdout, "Size of dirBlock: %d\n", sizeof(dirBlock_t));
	fprintf(stdout, "Size of SuperBlock: %d\n", sizeof(superBlock_t));
	fprintf(stdout, "Size of fileBlock: %d\n", sizeof(fileBlock_t));
}

/*TODO:
	1. Mount Disk
		dd a big space of all zero's
		Each 1024 bytes is a block
		The first block is the Super block
	2. Write	
*/