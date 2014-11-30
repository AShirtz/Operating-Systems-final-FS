#include "fileSystem.h"

FILE *createDisk();
void formatDisk(diskController_t *dC);

diskController_t *initializeDiskController();
void freeDiskController(diskController_t *dC);
int writeBlockToDisk(diskController_t *dC, int blockNum, void *blockPtr);
int readBlockFromDisk(diskController_t *dC, int blockNum, void *blockPtr);

activeDir_t *activateDirBlock(dirBlock_t *dB);
directoryEntry_t *convertEntryListingForDirBlock(dirBlock_t *dB);

activeDir_t *activateDirBlock(dirBlock_t *dB)
{
	activeDir_t *result;
	result = (activeDir_t *) malloc(sizeof(activeDir_t));
	memcpy(result->name, dB->name, 16);
	result->entries = convertEntryListingForDirBlock(dB);
	return result;
}

directoryEntry_t *convertEntryListingForDirBlock(dirBlock_t *dB)
{
	directoryEntry_t *entries;
	entries = malloc((dB->entryCount * sizeof(directoryEntry_t)) + 1);

	int i;

	char *entriesList[dB->entryCount];
	entriesList[0] = strtok(dB->entries, ";");
	for (i = 1; i < dB->entryCount; i++)
	{
		entriesList[i] = strtok(NULL, ";");
	}

	for (i = 0; i < dB->entryCount; i++)
	{
		directoryEntry_t result;
		memset(&result, 0, sizeof(directoryEntry_t));

		char *name = strtok(entriesList[i], ",");
		memcpy(&(result.name), name, strlen(name));

		result.blockType = atoi(strtok(NULL, ","));
		result.startingBlockNum = atoi(strtok(NULL, ","));

		memcpy(&(entries[i]), &result, sizeof(directoryEntry_t));
	}
	return entries;
}

diskController_t *initializeDiskController()
{
	diskController_t *diskCont;
	diskCont = malloc(sizeof(diskController_t));
	diskCont->vdisk = createDisk();
	formatDisk(diskCont);
	return diskCont;
}

void freeDiskController (diskController_t *dC)
{
	free(dC);
}

FILE *createDisk ()
{
	FILE *vdisk;
	system("dd if=/dev/zero of=vdisk bs=1024 count=100");
	vdisk = fopen("vdisk", "r+");
	return vdisk;
}

void formatDisk (diskController_t *dC)
{
	superBlock_t sB;
	memset(&sB, 0, BLOCK_SIZE);
	sB.blockType = 0;
	sB.numBlocks = 100;
	sB.rootDirBlockNum = 1;
	sB.freeBlockBitmap[0] = 3;

	writeBlockToDisk (dC, 0, &sB);

	dirBlock_t rD;
	memset(&rD, 0, BLOCK_SIZE);
	rD.blockType = 1;
	memcpy(rD.name, "~", 1);
	rD.entryCount = 0;
	rD.continuationDirBlockNum = -1;
	
	writeBlockToDisk(dC, 1, &rD);
}

int readBlockFromDisk (diskController_t *dC, int blockNum, void *blockPtr)
{
	fseek(dC->vdisk, (blockNum * BLOCK_SIZE), SEEK_SET);
	int readLen = fread(blockPtr, BLOCK_SIZE, 1, dC->vdisk);
	return readLen;
}

int writeBlockToDisk (diskController_t *dC, int blockNum, void *blockPtr)
{
	fseek(dC->vdisk, (blockNum * BLOCK_SIZE), SEEK_SET);
	int writeLen = fwrite(blockPtr, BLOCK_SIZE, 1, dC->vdisk);
	fflush(dC->vdisk);
	return writeLen;
}
