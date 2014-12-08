#include "fileSystem.h"

int	openFile(char *name);
void	closeFile(int fd);
bool	deleteFile(char *name);
int	writeInFile(int fd, char *outBuf, int len);
int	readInFile(int fd, char *inBuf, int len);
int	seekInFile(int fd, int offset);
bool	mkDir(char *name);
void	giveDirListing();
bool	deleteDir(char *name);

fileSystem_t 	*initializeFS();
fileSystem_t 	*getFS();
void		freeFS();
int		findEmptyBlock();
void		markBlockUsed(int blockNum);
void		markBlockFree(int blockNum);

FILE 	*createDisk();
void 	formatDisk(diskController_t *dC);

diskController_t 	*initializeDiskController();
void 			freeDiskController(diskController_t *dC);
int 			writeBlockToDisk(int blockNum, void *blockPtr);
int 			readBlockFromDisk(int blockNum, void *blockPtr);
int			writeDirect(char *outBuf, int len, int blockNum, int contentsPtr);
int			readDirect(char *inBuf, int len, int blockNum, int contentsPtr);

activeDir_t 		*activateDirBlock(dirBlock_t *dB);
directoryEntry_t 	*convertEntryListingForDirBlock(char *entryListing);

bool addDirectoryEntry(activeDir_t *aD, char *name, int blockType, int startingBlockNum);
bool makeDirectory(activeDir_t *aD, char *name);

dirBlock_t 	*convertActiveDirToBlock(activeDir_t *aD);
char 		*serializeEntryListing(activeDir_t *aD);

void 	freeActiveDirectory(activeDir_t *aD);
void 	freeDirBlock(dirBlock_t *dB);

activeFile_t 	*openFileInternal(activeDir_t *aD, char *name);
bool		createFile(activeDir_t *aD, char *name); 
activeFile_t	*activateFileBlock(int blockNum, fileBlock_t *fB);
blockLink_t	*generateBlockLink(int blockNum);

void	freeBlockLink(blockLink_t *bL);
void 	freeActiveFile(activeFile_t *aF);

fileSystem_t *FS;
bool FSInitialized = false;

void freeFS()
{
	fileSystem_t *FS = getFS();
	freeDiskController(FS->diskCont);
	freeActiveDirectory(FS->activeDir);
	int i;
	for (i = 0; i < 8; i++)
	{
		if (FS->activeFilesBitmap & (1 << i))
		{
			freeActiveFile(FS->activeFiles[i]);
		}
	}
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
	memset(FS, 0, sizeof(fileSystem_t));
	FS->diskCont = initializeDiskController();

	dirBlock_t *dB;
	dB = malloc(sizeof(dirBlock_t));
	memset(dB, 0, sizeof(dirBlock_t));
	fseek(FS->diskCont->vdisk, (1 * BLOCK_SIZE), SEEK_SET);
	fread(dB, BLOCK_SIZE, 1, FS->diskCont->vdisk);

	FS->activeDir = activateDirBlock(dB);

	FS->superBlock = malloc(sizeof(superBlock_t));
	memset(FS->superBlock, 0, sizeof(superBlock_t));
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
	for (i = 0; i < (sB->numBlocks / 8); i++)
	{
		if (sB->blockBitmap[i] != (char) 0xFF)
		{
			firstFree = 8 * i;
			char counter = 1;
			while (sB->blockBitmap[i] & counter)
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
	localBitmap = ((fileSystem_t *) getFS())->superBlock->blockBitmap[blockNum / 8];

	char c = 1 << (blockNum % 8);
	((fileSystem_t *) getFS())->superBlock->blockBitmap[blockNum / 8] = localBitmap | c;
}

void markBlockFree(int blockNum)
{
	char localBitmap = ((fileSystem_t *) getFS())->superBlock->blockBitmap[blockNum / 8];
	char c = 1 << (blockNum % 8);
	((fileSystem_t *) getFS())->superBlock->blockBitmap[blockNum / 8] = localBitmap ^ c;
}

int openFile(char *name)
{
	//Sanitizing input
	char validName[NAME_SIZE];
	memset(validName, 0, NAME_SIZE);
	memcpy(validName, name, strlen(name));

	if (getFS()->activeFilesBitmap == (char) 0xFF)
	{
		fprintf(stdout, "Cannot open file %s, too many files open.\n", validName);
		return -1;
	}
	//TODO: make sure that this file isn't already open, if so just return that fd
	int activeFileIndex = 0;
	char c = 1;
	while (getFS()->activeFilesBitmap & c)
	{
		c = c << 1;
		activeFileIndex++;
	}

	getFS()->activeFiles[activeFileIndex] = openFileInternal(getFS()->activeDir, validName);
	getFS()->activeFilesBitmap = getFS()->activeFilesBitmap | c;
	return activeFileIndex;
}

void closeFile(int fd)
{
	if (!(getFS()->activeFilesBitmap & (1 << fd)))
	{
		//TODO: error out, file not opened
		fprintf(stdout, "File descriptor not valid.\n");
		return;
	}

	freeActiveFile(getFS()->activeFiles[fd]);
	getFS()->activeFiles[fd] = NULL;
	getFS()->activeFilesBitmap = (getFS()->activeFilesBitmap ^ (1 << fd));
}

void removeDirectoryListing (char *name)
{
	directoryEntry_t *dE = getFS()->activeDir->entries;	

	if (!strcmp(dE->name, name))
	{
		getFS()->activeDir->entries = dE->nextEntry;
	} else {
		while (dE->nextEntry != NULL)
		{
			if (!strcmp(dE->nextEntry->name, name))
			{
				dE->nextEntry = dE->nextEntry->nextEntry;
				break;
			} else {
				dE = dE->nextEntry;
			}
		}
	}
}

bool deleteFile(char *name)
{
	directoryEntry_t *dE = getFS()->activeDir->entries;
	if (dE == NULL)
	{
		fprintf(stdout, "Delete file named %s failed: no such file.\n", name);
		return false;
	}
	bool inCurDir = false;
	if (strcmp(dE->name, name))
	{
		while (dE->nextEntry != NULL)
		{
			dE = dE->nextEntry;
			if (!strcmp(dE->name, name))
			{
				inCurDir = true;
				break;
			}
		}
	} else { inCurDir = true; }

	if (!inCurDir)
	{
		fprintf(stdout, "Delete file named %s failed: no such file.\n", name);
		return false;
	}

	blockLink_t *bL = generateBlockLink(dE->startingBlockNum);
	removeDirectoryListing(name);
	blockLink_t *bLcpy = bL;
	fileBlock_t nullBlock;
	memset(&nullBlock, 0, sizeof(fileBlock_t));
	writeBlockToDisk(bL->blockNum, &nullBlock);
	markBlockFree(bL->blockNum);

	while (bL->nextLink != 0)
	{
		bL = bL->nextLink;
		writeBlockToDisk(bL->blockNum, &nullBlock);
		markBlockFree(bL->blockNum);
	}
	freeBlockLink(bLcpy);
	return true;
}

void giveDirListing()
{
	fprintf(stdout, "%s\n", getFS()->activeDir->name);
	directoryEntry_t *dE = getFS()->activeDir->entries;
	while (dE != NULL)
	{
		fprintf(stdout, "	%s : %s\n", (dE->blockType == 2) ? "Dir " : "File", dE->name);
		dE = dE->nextEntry;
	}
	fprintf(stdout, "\n");
}

bool createContFileBlock(activeFile_t *aF)
{
	int firstFreeBlock = findEmptyBlock();
	if (firstFreeBlock == -1)
	{
		fprintf(stdout, "Cannot allocate block for file, disk full.\n");
		return false;
	}

	fileBlock_t *fB;
	fB = malloc(sizeof(fileBlock_t));
	memset(fB, 0, sizeof(fileBlock_t));
	memcpy(fB->name, aF->name, NAME_SIZE);	
	fB->blockType = 3;
	
	blockLink_t *bL = aF->fileHeadLink;
	while (bL->nextLink != 0) { bL = bL->nextLink; }
	blockLink_t *newBL = malloc(sizeof(blockLink_t));
	memset(newBL, 0, sizeof(blockLink_t));
	bL->nextLink = newBL;

	writeBlockToDisk(firstFreeBlock, fB);
	markBlockUsed(firstFreeBlock);
	writeDirect(&firstFreeBlock, sizeof(int), bL->blockNum, offsetof(fileBlock_t, contBlockNum));
	newBL->blockNum = firstFreeBlock;

	free(fB);
	return true;
}

int getBlockNumForCurContPtr(activeFile_t *aF)
{
	blockLink_t *bL = aF->fileHeadLink;
	int contPtr = aF->curContentsPtr;
	while (contPtr >= FILE_BLOCK_DATA_SIZE)
	{
		if (bL->nextLink == 0)
		{
			if (!createContFileBlock(aF))
			{
				fprintf(stdout, "Cannot allocate, disk full.\n");
				return -1;
			}
		}
		bL = bL->nextLink;
		contPtr -= FILE_BLOCK_DATA_SIZE;
	}
	return bL->blockNum;
}

int writeInFile(int fd, char *outBuf, int len)
{
	//Check to see if the fd is an active file
	if (!(getFS()->activeFilesBitmap & (1 << fd)))
	{
		fprintf(stdout, "File descriptor invalid.\n");
		return -1;
	}

	activeFile_t *aF;
	aF = getFS()->activeFiles[fd];

	int blockNum = getBlockNumForCurContPtr(aF);
	if (blockNum == -1) 
	{
		fprintf(stdout, "Write failed.\n");
		return -1;
	}
	if ((aF->curContentsPtr % FILE_BLOCK_DATA_SIZE) + len > FILE_BLOCK_DATA_SIZE)
	{
		int toWrite = FILE_BLOCK_DATA_SIZE - (aF->curContentsPtr % FILE_BLOCK_DATA_SIZE);
		int writeLen = writeDirect(outBuf, toWrite, blockNum, (aF->curContentsPtr % FILE_BLOCK_DATA_SIZE) + offsetof(fileBlock_t, data));
		aF->curContentsPtr += writeLen;
		writeLen += writeInFile(fd, (outBuf + writeLen), (len - writeLen));
		return writeLen;
	}
	else 
	{
		int writeLen = writeDirect(outBuf, len, blockNum, (aF->curContentsPtr % FILE_BLOCK_DATA_SIZE) + offsetof(fileBlock_t, data));
		aF->curContentsPtr += writeLen;
		return writeLen;
	}
}

int readInFile(int fd, char *inBuf, int len)
{

	if (!(getFS()->activeFilesBitmap & (1 << fd)))
	{
		fprintf(stdout, "File descriptor invalid.\n");
		return -1;
	}

	activeFile_t *aF;
	aF = getFS()->activeFiles[fd];
	
	int blockNum = getBlockNumForCurContPtr(aF);
	if (blockNum == -1)
	{
		fprintf(stdout, "Read failed.\n");
		return -1;
	}
	if ((aF->curContentsPtr % FILE_BLOCK_DATA_SIZE) + len > FILE_BLOCK_DATA_SIZE)
	{
		int toRead = FILE_BLOCK_DATA_SIZE - (aF->curContentsPtr % FILE_BLOCK_DATA_SIZE);
		int readLen = readDirect(inBuf, toRead, blockNum, (aF->curContentsPtr % FILE_BLOCK_DATA_SIZE) + offsetof(fileBlock_t, data));
		aF->curContentsPtr += readLen;
		readLen += readInFile(fd, (inBuf + readLen), (len - readLen));
		return readLen;
	}
	int readLen = readDirect(inBuf, len, blockNum, (aF->curContentsPtr % FILE_BLOCK_DATA_SIZE) + offsetof(fileBlock_t, data));
	aF->curContentsPtr += readLen;
	return readLen;
}

int seekInFile(int fd, int offset)
{
	if (!(getFS()->activeFilesBitmap & (1 << fd)))
	{
		fprintf(stdout, "File descriptor invalid.\n");
		return -1;
	}
	int reset = getFS()->activeFiles[fd]->curContentsPtr;
	getFS()->activeFiles[fd]->curContentsPtr = offset;
	int i = getBlockNumForCurContPtr(getFS()->activeFiles[fd]);
	if (i == -1)
	{
		fprintf(stdout, "Seek failed.\n");
		getFS()->activeFiles[fd]->curContentsPtr = reset;
		return -1;
	}
	return getFS()->activeFiles[fd]->curContentsPtr;
}

bool mkDir(char *name)
{
	//Validate name
	int i;
	for (i = 0; i < strlen(name); i++)
	{
		if (name[i] == ',' || name[i] == ';' || name[i] == '/')
		{
			return false;
		}
	}

	char validName[NAME_SIZE];
	memset(validName, 0, sizeof(validName));
	memcpy(validName, name, strlen(name));

	return makeDirectory(getFS()->activeDir, validName);
}

activeFile_t *openFileInternal(activeDir_t *aD, char *name)
{
	//Check to see if the file in question is in the current directory
	//This leaves dE as the correct entry for the file in question
	bool inCurDir = false;
	directoryEntry_t *dE = aD->entries;
	if (dE != NULL)
	{
		if (!strcmp(dE->name, name))
		{
			inCurDir = true;
		} else {
			while (dE->nextEntry != NULL)
			{
				dE = dE->nextEntry;
				if (!strcmp(dE->name, name))
				{
					inCurDir = true;
					break;
				}
			}
		}
	}

	//If not found, create said file
	//Create file ensures that the recursive call to openFile will find the file in the current dir
	if (!inCurDir) 
	{
		createFile(aD, name);
		return openFileInternal(aD, name);
	}
	//Else read fileBlock and activate
	fileBlock_t *fB;
	fB = malloc(sizeof(fileBlock_t));
	memset(fB, 0, sizeof(fileBlock_t));
	readBlockFromDisk(dE->startingBlockNum, fB);

	activeFile_t *result;
	result = activateFileBlock(dE->startingBlockNum, fB);

	return result;
}

activeFile_t *activateFileBlock(int blockNum, fileBlock_t *fB)
{
	activeFile_t *result;
	result = malloc(sizeof(activeFile_t));
	memset(result, 0, sizeof(activeFile_t));
	memcpy(result->name, fB->name, NAME_SIZE);
	result->fileHeadLink = generateBlockLink(blockNum);
	return result;
}

blockLink_t *generateBlockLink(int blockNum)
{
	blockLink_t *result;
	result = malloc(sizeof(blockLink_t));
	memset(result, 0, sizeof(blockLink_t));
	result->blockNum = blockNum;

	fileBlock_t fB;
	memset(&fB, 0, sizeof(fileBlock_t));
	readBlockFromDisk(blockNum, &fB);
	if (fB.contBlockNum != 0) { result->nextLink = generateBlockLink(fB.contBlockNum); }
	return result;
}

bool createFile(activeDir_t *aD, char *name)
{
	int firstFreeBlock = findEmptyBlock();
	bool result = addDirectoryEntry(aD, name, 3, firstFreeBlock);
	if (firstFreeBlock == -1 || !result)
	{
		return false;
	}

	fileBlock_t *fB;
	fB = malloc(sizeof(fileBlock_t));
	memset(fB, 0, sizeof(fileBlock_t));
	memcpy(fB->name, name, NAME_SIZE);
	fB->blockType = 3;

	writeBlockToDisk(firstFreeBlock, fB);
	free(fB);
	markBlockUsed(firstFreeBlock);

	return true;
}

void freeBlockLink(blockLink_t *bL)
{
	if (bL->nextLink != NULL) { freeBlockLink(bL->nextLink); }
	free(bL);
}

void freeActiveFile(activeFile_t *aF)
{
	freeBlockLink(aF->fileHeadLink);
	free(aF);
}

activeDir_t *activateDirBlock(dirBlock_t *dB)
{
	activeDir_t *result;
	result = (activeDir_t *) malloc(sizeof(activeDir_t));
	memset(result, 0, sizeof(activeDir_t));
	memcpy(result->name, dB->name, NAME_SIZE);
	result->entryCount = dB->entryCount;
	result->entries = convertEntryListingForDirBlock(dB->entries);
	return result;
}

directoryEntry_t *convertEntryListingForDirBlock(char *entryListing)
{
	if (strlen(entryListing) == 0) { return NULL; }
	directoryEntry_t *dE;
	dE = malloc(sizeof(directoryEntry_t));
	memset(dE, 0, sizeof(directoryEntry_t));

	char *entryStr = strtok(entryListing, ';');

	char *name = strtok(entryStr, ',');
	char *blockType = strtok(NULL, ',');
	char *blockNum = strtok(NULL, ',');

	memcpy(dE->name, name, strlen(name));
	dE->blockType = atoi(blockType);
	dE->startingBlockNum = atoi(blockNum);

	dE->nextEntry = convertEntryListingForDirBlock((entryListing + strlen(entryStr) + 1));

	return dE;
}

bool makeDirectory(activeDir_t *aD, char *name)
{
	int firstBlockFree = findEmptyBlock();
	bool result = addDirectoryEntry(aD, name, 2, firstBlockFree);
	if (firstBlockFree == -1 || !result)
	{
		return false;
	}

	dirBlock_t *dB;
	dB = malloc(sizeof(dirBlock_t));
	memset(dB, 0, sizeof(dirBlock_t));
	memcpy(dB->name, name, strlen(name));
	dB->blockType = 2;

	writeBlockToDisk(firstBlockFree, dB);
	markBlockUsed(firstBlockFree);
	free(dB);

	return true;
}

bool deleteDir(char *name)
{
	directoryEntry_t *dE = getFS()->activeDir->entries;
	if (dE == NULL)
	{
		fprintf(stdout, "Delete directory %s failed: no such directory.\n", name);
		return false;
	}
	bool inCurDir = false;
	if (strcmp(dE->name, name))
	{
		while (dE->nextEntry != NULL)
		{
			dE = dE->nextEntry;
			if (!strcmp(dE->name, name))
			{
				inCurDir = true;
				break;
			}
		}
	} else { inCurDir = true; }

	if (!inCurDir)
	{
		fprintf(stdout, "Delete directory %s failed: no such directory.\n", name);
		return false;
	}

	removeDirectoryListing(name);
	dirBlock_t nullBlock;
	memset(&nullBlock, 0, sizeof(dirBlock_t));
	writeBlockToDisk(dE->startingBlockNum, &nullBlock);
	markBlockFree(dE->startingBlockNum);
	return true;
}

bool addDirectoryEntry(activeDir_t *aD, char *name, int blockType, int startingBlockNum)
{
	if(aD->entryCount >= MAX_ENTRY_COUNT) { return false; }

	directoryEntry_t *newEntry;
	newEntry = malloc(sizeof(directoryEntry_t));
	memset(newEntry, 0, sizeof(directoryEntry_t));

	memcpy(newEntry->name, name, strlen(name));
	newEntry->blockType = blockType;
	newEntry->startingBlockNum = startingBlockNum;

	directoryEntry_t *dE = aD->entries;

	if (dE != NULL)
	{
		while (dE->nextEntry != NULL) { dE = dE->nextEntry; }
		dE->nextEntry = newEntry;
	} else {
		aD->entries = newEntry;
	}
	aD->entryCount++;
	return true;
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
	memset(dB, 0, sizeof(dirBlock_t));
	dB->blockType = 2;
	memcpy(dB->name, aD->name, strlen(aD->name));
	dB->entryCount = aD->entryCount;
	char *entryListing = serializeEntryListing(aD);
	memcpy(dB->entries, entryListing, strlen(entryListing));
	free(entryListing);
	return dB;
}

char *serializeEntryListing(activeDir_t *aD)
{
	char *result;
	result = malloc(996 * sizeof(char));
	memset(result, 0, (996 * sizeof(char)));
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
	memset(diskCont, 0, sizeof(diskController_t));
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
	memset(sB, 0, sizeof(superBlock_t));
	sB->blockType = 1;
	sB->numBlocks = 104;	//Just make it a multiple of 8, this is for the bitmapping to work
	sB->rootDirBlockNum = 1;
	sB->blockBitmap[0] = 3;

	fseek(dC->vdisk, 0, SEEK_SET);
	fwrite(sB, BLOCK_SIZE, 1, dC->vdisk);

	dirBlock_t *rD;
	rD = malloc(sizeof(dirBlock_t));
	memset(rD, 0, sizeof(dirBlock_t));
	rD->blockType = 2;
	memcpy(rD->name, "~", 1);
	rD->entryCount = 0;
	
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
	int writeLen = fwrite(outBuf, 1, len, dC->vdisk);
	fflush(dC->vdisk);
	return writeLen;
}

int readDirect(char *inBuf, int len, int blockNum, int contentsPtr)
{
	diskController_t *dC = getFS()->diskCont;
	fseek(dC->vdisk, (blockNum * BLOCK_SIZE) + contentsPtr, SEEK_SET);
	int readLen = fread(inBuf, 1, len, dC->vdisk);
	return readLen;

}
