#include "fileSystem.h"

fileSystem_t 	*initializeFS();
fileSystem_t 	*getFS();
void		freeFS();
int		findEmptyBlock();
void		markBlockUsed(int blockNum);
void		freeBlock(int blockNum);	//TODO: not done,

FILE 	*createDisk();
void 	formatDisk(diskController_t *dC);

diskController_t 	*initializeDiskController();
void 			freeDiskController(diskController_t *dC);
int 			writeBlockToDisk(int blockNum, void *blockPtr);
int 			readBlockFromDisk(int blockNum, void *blockPtr);
int			writeDirect(char *outBuf, int len, int blockNum, int contentsPtr);
int			readDirect(char *inBuf, int len, int blockNum, int contentsPtr);

activeDir_t 		*activateDirBlock(dirBlock_t *dB);
directoryEntry_t 	*convertEntryListingForDirBlock(dirBlock_t *dB);

void addDirectoryEntry(activeDir_t *aD, char *name, int blockType, int startingBlockNum);
void makeDirectory(activeDir_t *aD, char *name);
void changeDirectory(activeDir_t *aD, char *name);

dirBlock_t 	*convertActiveDirToBlock(activeDir_t *aD);
char 		*serializeEntryListing(activeDir_t *aD);

void 	freeActiveDirectory(activeDir_t *aD);
void 	freeDirBlock(dirBlock_t *dB);

activeFile_t 	*openFile(activeDir_t *aD, char *name);
void		createFile(activeDir_t *aD, char *name); 
activeFile_t	*activateFileBlock(fileBlock_t *fB);

void 	freeActiveFile(activeFile_t *aF);

fileSystem_t *FS;
bool FSInitialized = false;

void freeFS()
{
	fileSystem_t *FS = getFS();
	freeDiskController(FS->diskCont);
	freeActiveDirectory(FS->activeDir);
	free(FS->superBlock);
	free(FS);
}

fileSystem_t *getFS()
{
	if(!FSInitialized) { FS = initializeFS(); }
	return FS;
}

fileSystem_t *initializeFS()
{
	//This method does some reading by hand, this is because readFromDisk and writeToDisk call getFS
	fileSystem_t *FS;
	FS = malloc(sizeof(fileSystem_t));
	FS->diskCont = initializeDiskController();

	dirBlock_t *dB;
	dB = malloc(sizeof(dirBlock_t));
	fseek(FS->diskCont->vdisk, (1 * BLOCK_SIZE), SEEK_SET);
	fread(dB, BLOCK_SIZE, 1, FS->diskCont->vdisk);

	FS->activeDir = activateDirBlock(dB);

	FS->superBlock = malloc(sizeof(superBlock_t));
	fseek(FS->diskCont->vdisk, 0, SEEK_SET);
	fread(FS->superBlock, BLOCK_SIZE, 1, FS->diskCont->vdisk);

	freeDirBlock(dB);

	FSInitialized = true;
	return FS;
}

int findEmptyBlock()
{
	superBlock_t *sB = ((fileSystem_t *) getFS())->superBlock;
	int firstFree = -1;
	int i;
	for (i = 0; i < sB->numBlocks; i++)
	{
		if (sB->freeBlockBitmap[i] != (char) 0xFF)
		{
			firstFree = 8 * i;
			char counter = 1;
			while (sB->freeBlockBitmap[i] & counter)
			{
				counter = counter << 1;
				firstFree++;
			}
			break;
		}
	}
	return firstFree;
}

void markBlockUsed(int blockNum)
{
	char localBitmap;
	localBitmap = ((fileSystem_t *) getFS())->superBlock->freeBlockBitmap[blockNum / 8];

	char c = 0b1;
	c = c << (blockNum % 8);
	((fileSystem_t *) getFS())->superBlock->freeBlockBitmap[blockNum / 8] = localBitmap | c;
}

void freeBlock(int blockNum)
{
	
}

activeFile_t *openFile(activeDir_t *aD, char *name)
{
	//Check to see if the file in question is in the current directory
	bool inCurDir = false;
	int i;
	for (i = 0; i < aD->entryCount; i++)
	{
		if (!strcmp(name, aD->entries[i].name))
		{
			inCurDir = true;
			break;
		}
	}
	//If not found, create said file
	//Create file ensures that the recursive call to openFile will find the file in the current dir
	if (!inCurDir) 
	{
		createFile(aD, name);
		return openFile(aD, name);
	}
	//Else read fileBlock and activate
	fileBlock_t *fB;
	fB = malloc(sizeof(fileBlock_t));
	readBlockFromDisk(aD->entries[i].startingBlockNum, fB);

	activeFile_t *result;
	result = activateFileBlock(fB);

	return result;
}

activeFile_t *activateFileBlock(fileBlock_t *fB)
{
	activeFile_t *result;
	result = malloc(sizeof(activeFile_t));
	result->curContentsPtr = 0;
	result->fileHeadBlock = fB;
	return result;
}

void createFile(activeDir_t *aD, char *name)
{
	fileBlock_t *fB;
	fB = malloc(sizeof(fileBlock_t));
	memcpy(fB->name, name, NAME_SIZE);
	fB->blockType = 3;
	fB->continuationFileBlockNum = -1;

	int firstFreeBlock = findEmptyBlock();
	writeBlockToDisk(firstFreeBlock, fB);
	free(fB);
	markBlockUsed(firstFreeBlock);

	addDirectoryEntry(aD, name, 3, firstFreeBlock);
}

void 	freeActiveFile(activeFile_t *aF)
{
	free(aF->fileHeadBlock);
	free(aF);
}

activeDir_t *activateDirBlock(dirBlock_t *dB)
{
	activeDir_t *result;
	result = (activeDir_t *) malloc(sizeof(activeDir_t));
	memcpy(result->name, dB->name, NAME_SIZE);
	result->entryCount = dB->entryCount;
	result->entries = convertEntryListingForDirBlock(dB);
//TODO: continutation pointer needs to be addressed
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

void makeDirectory(activeDir_t *aD, char *name)
{
	dirBlock_t *dB;
	dB = malloc(sizeof(dirBlock_t));
	memcpy(dB->name, name, strlen(name));
	dB->continuationDirBlockNum = -1;
	dB->blockType = 2;

	int firstBlockFree = findEmptyBlock();
	writeBlockToDisk(firstBlockFree, dB);
	markBlockUsed(firstBlockFree);
	free(dB);

	addDirectoryEntry(aD, name, 2, firstBlockFree);
}

void addDirectoryEntry(activeDir_t *aD, char *name, int blockType, int startingBlockNum)
{
	//Increase size of aD->entries by 1 
	directoryEntry_t *entries;
	entries = malloc(((aD->entryCount + 1) * sizeof(directoryEntry_t)) + 1);
	memcpy(entries, aD->entries, (aD->entryCount * sizeof(directoryEntry_t)));
	free(aD->entries);
	aD->entries = entries;

	directoryEntry_t newEntry;
	memset(&newEntry, 0, sizeof(directoryEntry_t));
	memcpy(&(newEntry.name), name, strlen(name));
	newEntry.blockType = blockType;
	newEntry.startingBlockNum = startingBlockNum;

	memcpy((aD->entries + aD->entryCount++), &newEntry, sizeof(directoryEntry_t));
}

void freeActiveDirectory(activeDir_t *aD)
{
	free(aD->entries);
	free(aD);
}

dirBlock_t *convertActiveDirToBlock(activeDir_t *aD)
{
	dirBlock_t *dB;
	dB = malloc(sizeof(dirBlock_t));
	dB->blockType = 2;
	memcpy(dB->name, aD->name, strlen(aD->name));
	dB->entryCount = aD->entryCount;
	char *entryListing = serializeEntryListing(aD);
	memcpy(dB->entries, entryListing, strlen(entryListing));
	free(entryListing);
//TODO: Continuation pointer needs to be addressed	
	return dB;
}

char *serializeEntryListing(activeDir_t *aD)
{
	char *result;
	result = malloc(996 * sizeof(char));
	int curIndex = 0;
	int i;
	for (i = 0; i < aD->entryCount; i++)
	{
		sprintf((result + curIndex), "%s,%d,%d,;", aD->entries[i].name, aD->entries[i].blockType, aD->entries[i].startingBlockNum);
		curIndex = strlen(result);
	}
	return result;
}

void freeDirBlock(dirBlock_t *dB)
{
	free(dB);
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
	fclose(dC->vdisk);
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
	//This method does some writing by hand, this is because readFromDisk and writeToDisk call getFS
	superBlock_t *sB;
	sB = malloc(sizeof(superBlock_t));
	sB->blockType = 1;
	sB->numBlocks = 100;
	sB->rootDirBlockNum = 1;
	sB->freeBlockBitmap[0] = 3;

	fseek(dC->vdisk, 0, SEEK_SET);
	fwrite(sB, BLOCK_SIZE, 1, dC->vdisk);

	dirBlock_t *rD;
	rD = malloc(sizeof(dirBlock_t));
	rD->blockType = 2;
	memcpy(rD->name, "~", 1);
	rD->entryCount = 0;
	rD->continuationDirBlockNum = -1;
	
	fseek(dC->vdisk, 1 * BLOCK_SIZE, SEEK_SET);
	fwrite(rD, BLOCK_SIZE, 1, dC->vdisk);

	free(sB);
	free(rD);
}

int readBlockFromDisk (int blockNum, void *blockPtr)
{
	diskController_t *dC = ((fileSystem_t *) getFS())->diskCont;
	fseek(dC->vdisk, (blockNum * BLOCK_SIZE), SEEK_SET);
	int readLen = fread(blockPtr, BLOCK_SIZE, 1, dC->vdisk);
	return readLen;
}

int writeBlockToDisk (int blockNum, void *blockPtr)
{
	diskController_t *dC = ((fileSystem_t *) getFS())->diskCont;
	fseek(dC->vdisk, (blockNum * BLOCK_SIZE), SEEK_SET);
	int writeLen = fwrite(blockPtr, BLOCK_SIZE, 1, dC->vdisk);
	fflush(dC->vdisk);
	return writeLen;
}

int writeDirect(char *outBuf, int len, int blockNum, int contentsPtr)
{
	diskController_t *dC = getFS()->diskCont;
	fseek(dC->vdisk, (blockNum * BLOCK_SIZE) + contentsPtr, SEEK_SET);
	int writeLen = fwrite(outBuf, len, 1, dC->vdisk);
	fflush(dC->vdisk);
	return writeLen;
}

int readDirect(char *inBuf, int len, int blockNum, int contentsPtr)
{
	diskController_t *dC = getFS()->diskCont;
	fseek(dC->vdisk, (blockNum * BLOCK_SIZE) + contentsPtr, SEEK_SET);
	int readLen = fread(inBuf, len, 1, dC->disk);
	return readLen;

}
