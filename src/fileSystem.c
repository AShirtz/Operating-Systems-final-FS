#include "fileSystem.h"

FILE *createDisk();
diskController_t *initializeDiskController();

diskController_t *initializeDiskController()
{
	diskController_t *diskCont;
	diskCont = malloc(sizeof(diskCont));
	diskCont->vdisk = createDisk();
	return diskCont;
}

FILE *createDisk ()
{
	FILE *vdisk;
	vdisk = fopen("vdisk", "w+");
	system("dd if=/dev/zero of=vdisk bs=1024 count=8128");
	return vdisk;
}

/*TODO:
	1. Mount Disk
		dd a big space of all zero's
		Each 1024 bytes is a block
		The first block is the Super block
	2. Write	
*/
