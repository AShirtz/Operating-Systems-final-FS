#include <stdio.h>
#include <stdlib.h>

#define MAX_OPEN_FILES 10
#define MAX_BLOCK_COUNT 8128 

/*
	Block Types
*/

typedef struct superBlock
{
	int 		numBlocks;
	struct dirBlock	*rootDir;
	char 		freeBlockBitmap[MAX_BLOCK_COUNT / 8];
} superBlock_t;

typedef struct dirBlock
{
	char 		name[16];
	int		fileCount;
	struct dirBlock	*nextDirBlock;
	char		entries[1000];
} dirBlock_t;

typedef struct fileBlock
{
	char			name[16];
	int 			internalFileSize;
	struct fileBlock 	*nextFileBlock;
	char			data[1000];
} fileBlock_t;

/*
	Objects
*/

typedef struct
{
	int		curContentsPtr;
	fileBlock_t	*startingFileBlock;

	int 		localContentsPtr;
	fileBlock_t	*currentFileBlock;

} activeFile_t;

typedef struct
{
	int 		fd;
	activeFile_t	*file;
} fileDescriptor_t;

typedef struct
{
	fileDescriptor_t	activeFiles[MAX_OPEN_FILES];
} FileSystem_t;

