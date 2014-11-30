#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define BLOCK_SIZE 1024
#define MAX_OPEN_FILES 10
#define MAX_BLOCK_COUNT 8096 
#define MAX_PATH_LENGTH 100

/*
	Block Types
		blockType: 
				0 = superBlock
				1 = dirBlock
				2 = fileBlock
*/

typedef struct superBlock
{
	int		blockType;
	int 		numBlocks;
	int		rootDirBlockNum;
	char 		freeBlockBitmap[MAX_BLOCK_COUNT / 8];
} superBlock_t;

typedef struct dirBlock
{
	int		blockType;
	char 		name[16];
	int		entryCount;
	int		continuationDirBlockNum;
	char		entries[996];	//NOTE: The format for entry listings is as such
					//	name,blockType,blockNum;name,blockType,blockNum; ... 
} dirBlock_t;

typedef struct fileBlock
{
	int		blockType;
	char		name[16];
	int 		internalFileSize;
	int		continuationFileBlockNum;
	char		data[996];
} fileBlock_t;

/*
	Objects
*/

typedef struct
{
	int	curContentsPtr;
	int	startingBlockNum;

	int 	localContentsPtr;
	int	currentBlockNum;
} activeFile_t;

typedef struct
{
	int 		fd;
	activeFile_t	*file;
} fileDescriptor_t;



typedef struct
{
	char	name[16];
	int	blockType;
	int	startingBlockNum;
} directoryEntry_t;

typedef struct
{
	char			name[16];
	char			path[MAX_PATH_LENGTH];
	directoryEntry_t	*entries;
} activeDir_t;

/*
	Controllers
*/

typedef struct
{
	FILE *vdisk;
	
} diskController_t;

typedef struct
{
	fileDescriptor_t	activeFiles[MAX_OPEN_FILES];
	
} FileSystem_t;

